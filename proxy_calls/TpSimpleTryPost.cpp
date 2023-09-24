#include <windows.h>
#include <stdio.h>


#define ALLOC_ON_CODE _Pragma("section(\".text\")") __declspec(allocate(".text"))


ALLOC_ON_CODE unsigned char CallbackStub[] = {

    0x48, 0x89, 0xd3,            // mov rbx, rdx
    0x48, 0x8b, 0x03,            // mov rax, QWORD PTR[rbx]
    0x48, 0x8b, 0x4b, 0x08,      // mov rcx, QWORD PTR[rbx + 0x8]
    0xff, 0xe0                   // jmp rax

};


typedef struct _LOADLIBRARY_ARGS {
    UINT_PTR pLoadLibraryA;
    LPCSTR lpLibFileName;
} LOADLIBRARY_ARGS, *PLOADLIBRARY_ARGS;


int main() {

    LOADLIBRARY_ARGS loadLibraryArgs = { 0 };
    loadLibraryArgs.pLoadLibraryA = (UINT_PTR)GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA");
    loadLibraryArgs.lpLibFileName = "user32.dll";

    // NOTE: The TrySubmitThreadpoolCallback API is located in kernel32.dll and is directed to TpSimpleTryPost in ntdll.dll.
    // TrySubmitThreadpoolCallback((PTP_SIMPLE_CALLBACK)(unsigned char*)CallbackStub, &loadLibraryArgs, 0)

    typedef NTSTATUS(NTAPI* TPSIMPLETRYPOST)(_In_ PTP_SIMPLE_CALLBACK Callback, _Inout_opt_ PVOID Context, _In_opt_ PTP_CALLBACK_ENVIRON CallbackEnviron);
    FARPROC pTpSimpleTryPost = GetProcAddress(GetModuleHandleA("ntdll.dll"), "TpSimpleTryPost");

    ((TPSIMPLETRYPOST)pTpSimpleTryPost)((PTP_SIMPLE_CALLBACK)(unsigned char*)CallbackStub, &loadLibraryArgs, 0);

    WaitForSingleObject(GetCurrentProcess(), 1000);

    printf("user32.dll Address: %p\n", GetModuleHandleA("user32.dll"));

    return 0;

}