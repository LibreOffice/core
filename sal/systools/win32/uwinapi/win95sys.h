#pragma once

//Kernel32 objects

#define K32OBJ_SEMAPHORE            0x1
#define K32OBJ_EVENT                0x2
#define K32OBJ_MUTEX                0x3
#define K32OBJ_CRITICAL_SECTION     0x4
#define K32OBJ_PROCESS              0x5
#define K32OBJ_THREAD               0x6
#define K32OBJ_FILE                 0x7
#define K32OBJ_CHANGE               0x8
#define K32OBJ_CONSOLE              0x9
#define K32OBJ_SCREEN_BUFFER        0xA
#define K32OBJ_MEM_MAPPED_FILE      0xB
#define K32OBJ_SERIAL               0xC
#define K32OBJ_DEVICE_IOCTL         0xD
#define K32OBJ_PIPE                 0xE
#define K32OBJ_MAILSLOT             0xF
#define K32OBJ_TOOLHELP_SNAPSHOT    0x10
#define K32OBJ_SOCKET               0x11


//Process Database flags

#define fDebugSingle        0x00000001
#define fCreateProcessEvent 0x00000002
#define fExitProcessEvent   0x00000004
#define fWin16Process       0x00000008
#define fDosProcess         0x00000010
#define fConsoleProcess     0x00000020
#define fFileApisAreOem     0x00000040
#define fNukeProcess        0x00000080
#define fServiceProcess     0x00000100
#define fLoginScriptHack    0x00000800


//Thread Database flags

#define fCreateThreadEvent      0x00000001
#define fCancelExceptionAbort   0x00000002
#define fOnTempStack            0x00000004
#define fGrowableStack          0x00000008
#define fDelaySingleStep        0x00000010
#define fOpenExeAsImmovableFile 0x00000020
#define fCreateSuspended        0x00000040
#define fStackOverflow          0x00000080
#define fNestedCleanAPCs        0x00000100
#define fWasOemNowAnsi          0x00000200
#define fOKToSetThreadOem       0x00000400


#pragma pack(1)


//MODREF and IMTE structures

typedef struct _MODREF {
    struct _MODREF *pNextModRef;    // 00h
    DWORD       un1;        // 04h
    DWORD       un2;        // 08h
    DWORD       un3;        // 0Ch
    WORD        mteIndex;   // 10h
    WORD        un4;        // 12h
    DWORD       un5;        // 14h
    PVOID       ppdb;       // 18h Pointer to process database
    DWORD       un6;        // 1Ch
    DWORD       un7;        // 20h
    DWORD       un8;        // 24h
} MODREF, *PMODREF;

typedef struct _IMTE {
    DWORD       un1;            // 00h
    PIMAGE_NT_HEADERS   pNTHdr; // 04h
    DWORD       un2;            // 08h
    PSTR        pszFileName;    // 0Ch
    PSTR        pszModName;     // 10h
    WORD        cbFileName;     // 14h
    WORD        cbModName;      // 16h
    DWORD       un3;            // 18h
    DWORD       cSections;      // 1Ch
    DWORD       un5;            // 20h
    DWORD       baseAddress;    // 24h
    WORD        hModule16;      // 28h
    WORD        cUsage;         // 2Ah
    DWORD       un7;            // 2Ch
    PSTR        pszFileName2;   // 30h
    WORD        cbFileName2;    // 34h
    DWORD       pszModName2;    // 36h
    WORD        cbModName2;     // 3Ah
} IMTE, *PIMTE;


//Process Database structure

typedef struct _ENVIRONMENT_DATABASE {
PSTR    pszEnvironment;     // 00h Pointer to Environment
DWORD   un1;                // 04h
PSTR    pszCmdLine;         // 08h Pointer to command line
PSTR    pszCurrDirectory;   // 0Ch Pointer to current directory
LPSTARTUPINFOA pStartupInfo;// 10h Pointer to STARTUPINFOA struct
HANDLE  hStdIn;             // 14h Standard Input
HANDLE  hStdOut;            // 18h Standard Output
HANDLE  hStdErr;            // 1Ch Standard Error
DWORD   un2;                // 20h
DWORD   InheritConsole;     // 24h
DWORD   BreakType;          // 28h
DWORD   BreakSem;           // 2Ch
DWORD   BreakEvent;         // 30h
DWORD   BreakThreadID;      // 34h
DWORD   BreakHandlers;      // 38h
} ENVIRONMENT_DATABASE, *PENVIRONMENT_DATABASE;

typedef struct _KERNEL_OBJECT {
DWORD   Type;           // 00h KERNEL32 object type (5)
DWORD   cReference;     // 04h Number of references to process
} KERNEL_OBJECT, *PKERNEL_OBJECT;

typedef struct _HANDLE_TABLE_ENTRY {
    DWORD   flags;      // Valid flags depend on what type of object this is
    PKERNEL_OBJECT   pObject;   // Pointer to the object that the handle refers to
} HANDLE_TABLE_ENTRY, *PHANDLE_TABLE_ENTRY;

typedef struct _HANDLE_TABLE {
    DWORD   cEntries;               // Max number of handles in table
    HANDLE_TABLE_ENTRY array[1];    // An array (number is given by cEntries)
} HANDLE_TABLE, *PHANDLE_TABLE;


typedef struct _PROCESS_DATABASE {
DWORD   Type;           // 00h KERNEL32 object type (5)
DWORD   cReference;     // 04h Number of references to process
DWORD   un1;            // 08h
DWORD   someEvent;      // 0Ch An event object (What's it used for???)
DWORD   TerminationStatus;  // 10h Returned by GetExitCodeProcess
DWORD   un2;            // 14h
DWORD   DefaultHeap;        // 18h Address of the process heap
DWORD   MemoryContext;      // 1Ch pointer to the process's context
DWORD   flags;          // 20h
                // 0x00000001 - fDebugSingle
                // 0x00000002 - fCreateProcessEvent
                // 0x00000004 - fExitProcessEvent
                // 0x00000008 - fWin16Process
                // 0x00000010 - fDosProcess
                // 0x00000020 - fConsoleProcess
                // 0x00000040 - fFileApisAreOem
                // 0x00000080 - fNukeProcess
                // 0x00000100 - fServiceProcess
                // 0x00000800 - fLoginScriptHack
DWORD   pPSP;           // 24h Linear address of PSP?
WORD    PSPSelector;        // 28h
WORD    MTEIndex;       // 2Ah
WORD    cThreads;       // 2Ch
WORD    cNotTermThreads;    // 2Eh
WORD    un3;            // 30h
WORD    cRing0Threads;      // 32h number of ring 0 threads
HANDLE  HeapHandle;     // 34h Heap to allocate handle tables out of
                //     This seems to always be the KERNEL32 heap
HTASK   W16TDB;         // 38h Win16 Task Database selector
DWORD   MemMapFiles;        // 3Ch memory mapped file list (?)
PENVIRONMENT_DATABASE pEDB; // 40h Pointer to Environment Database
PHANDLE_TABLE pHandleTable; // 44h Pointer to process handle table
struct _PROCESS_DATABASE *ParentPDB;   // 48h Parent process database
PMODREF MODREFlist;     // 4Ch Module reference list
DWORD   ThreadList;     // 50h Threads in this process
DWORD   DebuggeeCB;     // 54h Debuggee Context block?
DWORD   LocalHeapFreeHead;  // 58h Head of free list in process heap
DWORD   InitialRing0ID;     // 5Ch
CRITICAL_SECTION    crst;   // 60h
DWORD   un4[3];         // 78h
DWORD   pConsole;       // 84h Pointer to console for process
DWORD   tlsInUseBits1;      // 88h  // Represents TLS indices 0 - 31
DWORD   tlsInUseBits2;      // 8Ch  // Represents TLS indices 32 - 63
DWORD   ProcessDWORD;       // 90h
struct _PROCESS_DATABASE *ProcessGroup;    // 94h
DWORD   pExeMODREF;     // 98h pointer to EXE's MODREF
DWORD   TopExcFilter;       // 9Ch Top Exception Filter?
DWORD   BasePriority;       // A0h Base scheduling priority for process
DWORD   HeapOwnList;        // A4h Head of the list of process heaps
DWORD   HeapHandleBlockList;// A8h Pointer to head of heap handle block list
DWORD   pSomeHeapPtr;       // ACh normally zero, but can a pointer to a
                // moveable handle block in the heap
DWORD   pConsoleProvider;   // B0h Process that owns the console we're using?
WORD    EnvironSelector;    // B4h Selector containing process environment
WORD    ErrorMode;      // B6H SetErrorMode value (also thunks to Win16)
DWORD   pevtLoadFinished;   // B8h Pointer to event LoadFinished?
WORD    UTState;        // BCh
} PROCESS_DATABASE, *PPROCESS_DATABASE;


//TIB (Thread Information Block) structure

typedef struct _SEH_record {
    struct _SEH_record *pNext;
    FARPROC     pfnHandler;
} SEH_record, *PSEH_record;

// This is semi-documented in the NTDDK.H file from the NT DDK
typedef struct _TIB {
PSEH_record pvExcept;       // 00h Head of exception record list
PVOID   pvStackUserTop;     // 04h Top of user stack
PVOID   pvStackUserBase;    // 08h Base of user stack
WORD    pvTDB;          // 0Ch TDB
WORD    pvThunksSS;     // 0Eh SS selector used for thunking to 16 bits
DWORD   SelmanList;     // 10h
PVOID   pvArbitrary;        // 14h Available for application use
struct _tib *ptibSelf;      // 18h Linear address of TIB structure
WORD    TIBFlags;       // 1Ch
WORD    Win16MutexCount;    // 1Eh
DWORD   DebugContext;       // 20h
DWORD   pCurrentPriority;   // 24h
DWORD   pvQueue;        // 28h Message Queue selector
PVOID  *pvTLSArray;     // 2Ch Thread Local Storage array
} TIB, *PTIB;


//TDBX structure

typedef struct _TDBX {
    DWORD   ptdb;       // 00h  // PTHREAD_DATABASE
    DWORD   ppdb;       // 04h  // PPROCESDS_DATABASE
    DWORD   ContextHandle;  // 08h
    DWORD   un1;        // 0Ch
    DWORD   TimeOutHandle;  // 10h
    DWORD   WakeParam;      // 14h
    DWORD   BlockHandle;    // 18h
    DWORD   BlockState;     // 1Ch
    DWORD   SuspendCount;   // 20h
    DWORD   SuspendHandle;  // 24h
    DWORD   MustCompleteCount;  // 28h
    DWORD   WaitExFlags;    // 2Ch
                // 0x00000001 - WAITEXBIT
                // 0x00000002 - WAITACKBIT
                // 0x00000004 - SUSPEND_APC_PENDING
                // 0x00000008 - SUSPEND_TERMINATED
                // 0x00000010 - BLOCKED_FOR_TERMINATION
                // 0x00000020 - EMULATE_NPX
                // 0x00000040 - WIN32_NPX
                // 0x00000080 - EXTENDED_HANDLES
                // 0x00000100 - FROZEN
                // 0x00000200 - DONT_FREEZE
                // 0x00000400 - DONT_UNFREEZE
                // 0x00000800 - DONT_TRACE
                // 0x00001000 - STOP_TRACING
                // 0x00002000 - WAITING_FOR_CRST_SAFE
                // 0x00004000 - CRST_SAFE
                // 0x00040000 - BLOCK_TERMINATE_APC
    DWORD   SyncWaitCount;  // 30h
    DWORD   QueuedSyncFuncs;    // 34h
    DWORD   UserAPCList;    // 38h
    DWORD   KernAPCList;    // 3Ch
    DWORD   pPMPSPSelector; // 40h
    DWORD   BlockedOnID;    // 44h
    DWORD   un2[7];     // 48h
    DWORD   TraceRefData;   // 64h
    DWORD   TraceCallBack;  // 68h
    DWORD   TraceEventHandle;   // 6Ch
    WORD    TraceOutLastCS; // 70h
    WORD    K16TDB;     // 72h
    WORD    K16PDB;     // 74h
    WORD    DosPDBSeg;      // 76h
    WORD    ExceptionCount; // 78h
} TDBX, *PTDBX;


//Thread Database structure

typedef struct _THREAD_DATABASE {
DWORD   Type;           // 00h
DWORD   cReference;     // 04h
PPROCESS_DATABASE pProcess; // 08h
DWORD   someEvent;      // 0Ch An event object (What's it used for???)
DWORD   pvExcept;       // 10h This field through field 3CH is a TIB
                //      structure (see TIB.H)
DWORD   TopOfStack;     // 14h
DWORD   StackLow;       // 18h
WORD    W16TDB;         // 1Ch
WORD    StackSelector16;    // 1Eh Used when thunking down to 16 bits
DWORD   SelmanList;     // 20h
DWORD   UserPointer;        // 24h
PTIB    pTIB;           // 28h
WORD    TIBFlags;       // 2Ch  TIBF_WIN32 = 1, TIBF_TRAP = 2
WORD    Win16MutexCount;    // 2Eh
DWORD   DebugContext;       // 30h
PDWORD  pCurrentPriority;   // 34h
DWORD   MessageQueue;       // 38h
DWORD   pTLSArray;      // 3Ch
PPROCESS_DATABASE pProcess2;// 40h Another copy of the thread's process???
DWORD   Flags;          // 44h
                // 0x00000001 - fCreateThreadEvent
                // 0x00000002 - fCancelExceptionAbort
                // 0x00000004 - fOnTempStack
                // 0x00000008 - fGrowableStack
                // 0x00000010 - fDelaySingleStep
                // 0x00000020 - fOpenExeAsImmovableFile
                // 0x00000040 - fCreateSuspended
                // 0x00000080 - fStackOverflow
                // 0x00000100 - fNestedCleanAPCs
                // 0x00000200 - fWasOemNowAnsi
                // 0x00000400 - fOKToSetThreadOem
DWORD   TerminationStatus;  // 48h Returned by GetExitCodeThread
WORD    TIBSelector;        // 4Ch
WORD    EmulatorSelector;   // 4Eh
DWORD   cHandles;       // 50h
DWORD   WaitNodeList;       // 54h
DWORD   un4;            // 58h
DWORD   Ring0Thread;        // 5Ch
PTDBX   pTDBX;          // 60
DWORD   StackBase;      // 64h
DWORD   TerminationStack;   // 68h
DWORD   EmulatorData;       // 6Ch
DWORD   GetLastErrorCode;   // 70h
DWORD   DebuggerCB;     // 74h
DWORD   DebuggerThread;     // 78h
PCONTEXT    ThreadContext;  // 7Ch  // register context defined in WINNT.H
DWORD   Except16List;       // 80h
DWORD   ThunkConnect;       // 84h
DWORD   NegStackBase;       // 88h
DWORD   CurrentSS;      // 8Ch
DWORD   SSTable;        // 90h
DWORD   ThunkSS16;      // 94h
DWORD   TLSArray[64];       // 98h
DWORD   DeltaPriority;      // 198h

// The retail version breaks off somewhere around here.
// All the remaining fields are most likely only in the debug version

DWORD   un5[7];         // 19Ch
DWORD   pCreateData16;      // 1B8h
DWORD   APISuspendCount;    // 1BCh # of times SuspendThread has been called
DWORD   un6;            // 1C0h
DWORD   WOWChain;       // 1C4h
WORD    wSSBig;         // 1C8h
WORD    un7;            // 1CAh
DWORD   lp16SwitchRec;      // 1CCh
DWORD   un8[6];         // 1D0h
DWORD   pSomeCritSect1;     // 1E8h
DWORD   pWin16Mutex;        // 1ECh
DWORD   pWin32Mutex;        // 1F0h
DWORD   pSomeCritSect2;     // 1F4h
DWORD   un9;            // 1F8h
DWORD   ripString;      // 1FCh
DWORD   LastTlsSetValueEIP[64]; // 200h (parallel to TlsArray, contains EIP
                //  where TLS value was last set from)
} THREAD_DATABASE, *PTHREAD_DATABASE;

