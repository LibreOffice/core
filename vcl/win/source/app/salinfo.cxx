/*************************************************************************
 *
 *  $RCSfile: salinfo.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:05:49 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef ENABLEUNICODE

#define VCL_NEED_BASETSD

#include <tools/presys.h>
#include <windows.h>
#include <imagehlp.h>
#include <tools/postsys.h>

#include <salsys.hxx>

/* !!! UNICODE !!! */
XubString SalSystem::GetSummarySystemInfos( ULONG nFlags )
{
    return XubString();
}

#else

#define VCL_NEED_BASETSD

#include <tools/presys.h>
#include <windows.h>
#include <imagehlp.h>
#include <tools/postsys.h>

#include <stdio.h>
#include <tools/string.hxx>
#include <salsys.hxx>

// Wegen Stacktrace-Generierung
#pragma optimize ("", off)

// #include <tlhelp32.h>
// ToolHelp32
#define MAX_MODULE_NAME32 255
#define TH32CS_SNAPMODULE   0x00000008

typedef struct tagMODULEENTRY32
{
    DWORD   dwSize;
    DWORD   th32ModuleID;       // This module
    DWORD   th32ProcessID;      // owning process
    DWORD   GlblcntUsage;       // Global usage count on the module
    DWORD   ProccntUsage;       // Module usage count in th32ProcessID's context
    BYTE  * modBaseAddr;        // Base address of module in th32ProcessID's context
    DWORD   modBaseSize;        // Size in bytes of module starting at modBaseAddr
    HMODULE hModule;            // The hModule of this module in th32ProcessID's context
    char    szModule[MAX_MODULE_NAME32 + 1];
    char    szExePath[MAX_PATH];
} MODULEENTRY32;
typedef MODULEENTRY32 *  PMODULEENTRY32;
typedef MODULEENTRY32 *  LPMODULEENTRY32;

// PSAPI functions - Windows NT only
typedef struct _MODULEINFO {
    LPVOID lpBaseOfDll;
    DWORD SizeOfImage;
    LPVOID EntryPoint;
} MODULEINFO, *LPMODULEINFO;



// PSAPI
typedef BOOL (WINAPI *ENUMPROCESSMODULESPROC)( HANDLE hProcess, HMODULE* lphModule, DWORD cb, LPDWORD lpcbNeeded );
typedef BOOL (WINAPI *GETMODULEINFORMATIONPROC)( HANDLE hProcess, HMODULE hModule, LPMODULEINFO lpmodinfo, DWORD cb );
typedef DWORD (WINAPI *GETMODULEBASENAMEAPROC)( HANDLE hProcess, HMODULE hModule, LPSTR lpBaseName, DWORD nSize );
typedef DWORD (WINAPI *GETMODULEFILENAMEEXAPROC)( HANDLE hProcess, HMODULE hModule, LPSTR lpFilename, DWORD nSize );
typedef HANDLE (WINAPI *CREATESNAPSHOTPROC)(DWORD dwFlags, DWORD th32ProcessID);
typedef BOOL (WINAPI *MODULE32FIRSTPROC)( HANDLE hSnapshot, LPMODULEENTRY32 lpme );
typedef BOOL (WINAPI *MODULE32NEXTPROC)( HANDLE hSnapshot, LPMODULEENTRY32 lpme );


// ImageHlp
typedef int (__stdcall *STACKWALKPROC) ( DWORD, HANDLE, HANDLE, LPSTACKFRAME, PVOID, PREAD_PROCESS_MEMORY_ROUTINE,PFUNCTION_TABLE_ACCESS_ROUTINE, PGET_MODULE_BASE_ROUTINE, PTRANSLATE_ADDRESS_ROUTINE );
typedef LPVOID (__stdcall *SYMFUNCTIONTABLEACCESSPROC)( HANDLE, DWORD );
typedef DWORD (__stdcall *SYMGETMODULEBASEPROC)( HANDLE, DWORD );
typedef DWORD (__stdcall *SYMSETOPTIONSPROC ) (DWORD dwSymOptions);
typedef int (__stdcall *SYMINITIALIZEPROC ) ( HANDLE, LPSTR, int );
typedef int (__stdcall *SYMCLEANUPPROC)( HANDLE );
typedef DWORD (__stdcall WINAPI *UNDECORATESYMBOLNAMEPROC)( PCSTR, PSTR, DWORD, DWORD );
typedef DWORD (__stdcall *SYMLOADMODULEPROC) ( HANDLE, HANDLE, LPSTR, LPSTR, DWORD, DWORD );
typedef int (__stdcall *SYMGETSYMFROMADDR)( HANDLE, DWORD, PDWORD, PIMAGEHLP_SYMBOL );


struct ModuleInfo
{
    struct ModuleInfo*          pNext;

    char                        szModBaseName[MAX_PATH];
    char                        szModFileName[MAX_PATH];
    unsigned long               nBaseAddress;
    unsigned long               nSize;
//  unsigned long               nEntryPoint;
//  HANDLE                      nHandle;
//  PIMAGE_DEBUG_INFORMATION    pDebugInfos;
};

struct ModuleInfo* FindModuleContainingAddress( struct ModuleInfo* pStart, void* pAddr );



struct SystemInfos
{
    DWORD   nCurrentProcessId;
    HANDLE  hCurrentProcess;
    DWORD   nCurrentThreadId;
    HANDLE  hCurrentThread;

    ModuleInfo* pModInfos;

    String  aStack;
    String  aModules;
    String  aSystemVersion;
    String  aCPUType;
    String  aMemoryInfo;
    String  aLocalVolumes;
    String  aSystemDirs;
    String  aMouseInfo;

    SystemInfos()
    {
        nCurrentProcessId = 0;
        hCurrentProcess = 0;
        hCurrentThread = 0;
        pModInfos = NULL;
    }
};

void DebugThread( SystemInfos* pSysInfos );


typedef struct _Thread
{
    DWORD dwThreadId;
    HANDLE hThread;
} Thread;


struct ModuleInfo*  WNT_CreateModuleInfos();

String ImplCreateToken( const String& rToken )
{
    String aToken( '<' );
    aToken += rToken;
    aToken += ' ';
    while ( aToken.Len() < 25 )
        aToken += '-';
    aToken += '>';
    return aToken;
}



String WNT_CreateModulePath( struct ModuleInfo* pModInfos )
{
    String aPath;
    struct ModuleInfo* pM = pModInfos;
    while ( pM )
    {
        String aTmpPath = pM->szModFileName;
        USHORT n = aTmpPath.SearchBackward( '\\' );
        if ( n != STRING_NOTFOUND )
            aTmpPath.Erase( n );
        aTmpPath += ';';
        aTmpPath.ToLower();
        if ( aPath.Search( aTmpPath ) == STRING_NOTFOUND  )
            aPath += aTmpPath;

        pM = pM->pNext;
    }
    return aPath;
}


struct ModuleInfo*  WNT_CreateModuleInfos()
{
    struct ModuleInfo* pModInfos = NULL;

    OSVERSIONINFO aOSVersion;
    aOSVersion.dwOSVersionInfoSize = sizeof( OSVERSIONINFO );
    if ( GetVersionEx( &aOSVersion ) )
    {
        if ( aOSVersion.dwPlatformId == VER_PLATFORM_WIN32_NT )
        {
            HINSTANCE hPSAPILib = LoadLibrary( "PSAPI.DLL" );
            if( hPSAPILib )
            {
                ENUMPROCESSMODULESPROC _fpEnumProcessModules = (ENUMPROCESSMODULESPROC) GetProcAddress( hPSAPILib, "EnumProcessModules" );
                GETMODULEINFORMATIONPROC _fpGetModuleInformation = (GETMODULEINFORMATIONPROC) GetProcAddress( hPSAPILib, "GetModuleInformation" );
                GETMODULEBASENAMEAPROC _fpGetModuleBaseNameA = (GETMODULEBASENAMEAPROC) GetProcAddress( hPSAPILib, "GetModuleBaseNameA" );
                GETMODULEFILENAMEEXAPROC _fpGetModuleFileNameExA = (GETMODULEFILENAMEEXAPROC) GetProcAddress( hPSAPILib, "GetModuleFileNameExA" );

                HANDLE hProcess = GetCurrentProcess();
                HMODULE hMods[1024];
                DWORD cbNeeded;
                if( _fpEnumProcessModules( hProcess, hMods, sizeof(hMods), &cbNeeded ) )
                {
                    int nMods = cbNeeded / sizeof( HMODULE );
                    int nArrSz = nMods * sizeof( struct ModuleInfo );
                    pModInfos = (struct ModuleInfo*) malloc( nArrSz );
                    memset( pModInfos, 0, nArrSz );

                    for ( int i = 0; i < nMods; i++ )
                    {
                        pModInfos[i].pNext = 0;
                        if ( i )
                            pModInfos[i-1].pNext = &pModInfos[i];

                        _fpGetModuleBaseNameA( hProcess, hMods[i], pModInfos[i].szModBaseName, sizeof( pModInfos[i].szModBaseName ) );
                        _fpGetModuleFileNameExA( hProcess, hMods[i], pModInfos[i].szModFileName, sizeof( pModInfos[i].szModFileName ) );

                        MODULEINFO aInf;
                        if ( _fpGetModuleInformation( hProcess, hMods[i], &aInf, sizeof( aInf ) ) )
                        {
                            pModInfos[i].nBaseAddress = (unsigned long) aInf.lpBaseOfDll;
                            pModInfos[i].nSize = aInf.SizeOfImage;
        //                  pModInfos[i].nEntryPoint = (unsigned long) aInf.EntryPoint;
                        }
                    }
                }
                FreeLibrary( hPSAPILib );
            }
        }
        else if ( aOSVersion.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS )
        {
            HINSTANCE hToolHelpLib = LoadLibrary( "KERNEL32.DLL" );
            if( hToolHelpLib )
            {
                CREATESNAPSHOTPROC _fpCreateSnapshot = (CREATESNAPSHOTPROC) GetProcAddress( hToolHelpLib, "CreateToolhelp32Snapshot" );
                MODULE32FIRSTPROC _fpModule32First = (MODULE32FIRSTPROC) GetProcAddress( hToolHelpLib, "Module32First" );
                MODULE32NEXTPROC _fpModule32Next = (MODULE32NEXTPROC) GetProcAddress( hToolHelpLib, "Module32Next" );

                HANDLE hSnap = _fpCreateSnapshot( TH32CS_SNAPMODULE, 0 );
                if ( hSnap )
                {
                    MODULEENTRY32 aMod32Entry;
                    ZeroMemory( &aMod32Entry, sizeof( MODULEENTRY32 ) );
                    aMod32Entry.dwSize = sizeof( MODULEENTRY32 );
                    int nMods = 0;
                    BOOL bMod = _fpModule32First( hSnap, &aMod32Entry );
                    while ( bMod )
                    {
                        nMods++;
                        bMod = _fpModule32Next( hSnap, &aMod32Entry );
                    }

                    int nArrSz = nMods * sizeof( struct ModuleInfo );
                    pModInfos = (struct ModuleInfo*) malloc( nArrSz );
                    memset( pModInfos, 0, nArrSz );

                    int nMod = 0;
                    bMod = _fpModule32First( hSnap, &aMod32Entry );
                    while ( bMod )
                    {
                        pModInfos[nMod].pNext = 0;
                        if ( nMod )
                            pModInfos[nMod-1].pNext = &pModInfos[nMod];

                        strcpy( pModInfos[nMod].szModBaseName, aMod32Entry.szModule );
                        strcpy( pModInfos[nMod].szModFileName, aMod32Entry.szExePath );
                        pModInfos[nMod].nBaseAddress = (unsigned long) aMod32Entry.modBaseAddr;
                        pModInfos[nMod].nSize = aMod32Entry.modBaseSize;
//                      pModInfos[nMod].nEntryPoint = 0xFFFFFFFF;

                        bMod = _fpModule32Next( hSnap, &aMod32Entry );
                        nMod++;
                    }

                    CloseHandle( hSnap );
                }

                FreeLibrary( hToolHelpLib );
            }
        }
    }
    return pModInfos;
}

BOOL WNT_GetLogicalAddress( PVOID addr, PTSTR szModule, DWORD len, DWORD& section, DWORD& offset )
{
    MEMORY_BASIC_INFORMATION mbi;

    if( VirtualQuery( addr, &mbi, sizeof(mbi) ) && mbi.AllocationBase )
    {
        DWORD hMod = (DWORD)mbi.AllocationBase;

        if ( !GetModuleFileName( (HMODULE)hMod, szModule, len ) )
            return FALSE;

        // Point to the DOS header in memory
        PIMAGE_DOS_HEADER pDosHdr = (PIMAGE_DOS_HEADER)hMod;

        // From the DOS header, find the NT (PE) header
        PIMAGE_NT_HEADERS pNtHdr = (PIMAGE_NT_HEADERS)(hMod + pDosHdr->e_lfanew);

        PIMAGE_SECTION_HEADER pSection = IMAGE_FIRST_SECTION( pNtHdr );

        DWORD rva = (DWORD)addr - hMod; // RVA is offset from module load address

        // Iterate through the section table, looking for the one that encompasses
        // the linear address.
        for (   unsigned i = 0;
                i < pNtHdr->FileHeader.NumberOfSections;
                i++, pSection++ )
        {
            DWORD sectionStart = pSection->VirtualAddress;
            DWORD sectionEnd = sectionStart
                        + max(pSection->SizeOfRawData, pSection->Misc.VirtualSize);

            // Is the address in this section???
            if ( (rva >= sectionStart) && (rva <= sectionEnd) )
            {
                // Yes, address is in the section.  Calculate section and offset,
                // and store in the "section" & "offset" params, which were
                // passed by reference.
                section = i+1;
                offset = rva - sectionStart;
                return TRUE;
            }
        }
    }
    return FALSE;
}


struct ModuleInfo* FindModuleContainingAddress( struct ModuleInfo* pStart, void* pAddr )
{
    struct ModuleInfo* pRet = NULL;
    struct ModuleInfo* pM = pStart;
    unsigned long nAddr = (unsigned long) pAddr;

    while ( pM && !pRet )
    {
        if ( ( nAddr >= pM->nBaseAddress ) &&
             ( nAddr < ( pM->nBaseAddress + pM->nSize ) ) )
        {
            pRet = pM;
        }
        else
        {
            pM = pM->pNext;
        }
    }
    return pRet;
}

String _OLD_GetStackInfo()
{
    // Try an other way...
    String aTmpStack;
    ModuleInfo* pMods = WNT_CreateModuleInfos();

    ULONG* pBP;
    __asm mov pBP, ebp;
    char buffer[1024];

    for ( int i = 0; i < 15; i++ )
    {
        ULONG nIP = pBP[1];
        sprintf( buffer, "[%.2u] IP=%.8lx", i, nIP );
        aTmpStack += buffer;

        ModuleInfo* pI = FindModuleContainingAddress( pMods, (void*)nIP );
        if ( pI )
        {
            sprintf( buffer, " (Rel=%.8lx) [%s, Base=%.8lx, Path=%s]", nIP-pI->nBaseAddress-0x1000, pI->szModBaseName, pI->nBaseAddress, pI->szModFileName );
            aTmpStack += buffer;
        }

        if ( !pI || (pBP[0] & 3) || (ULONG)pBP > pBP[0] )
        {
            aTmpStack += "\nError!\n";
            break;
        }

        aTmpStack += '\n';
        pBP = (ULONG*) pBP[0];
    }

    // Modul-Infos zerstoeren...
    return aTmpStack;
}

String SalSystem::GetSummarySystemInfos( ULONG nFlags)
{
    SystemInfos aSysInfos;
    HANDLE nCurrentThreadPseudo = GetCurrentThread();
    HANDLE nCurrentProcessPseudo = GetCurrentProcess();
    aSysInfos.nCurrentThreadId = GetCurrentThreadId();
    DuplicateHandle( nCurrentProcessPseudo, nCurrentThreadPseudo, nCurrentProcessPseudo,
                        &aSysInfos.hCurrentThread, PROCESS_ALL_ACCESS, TRUE, 0 );
    aSysInfos.nCurrentProcessId = GetCurrentProcessId();
    DuplicateHandle( nCurrentProcessPseudo, nCurrentProcessPseudo, nCurrentProcessPseudo,
                        &aSysInfos.hCurrentProcess, PROCESS_ALL_ACCESS, TRUE, 0 );

    if ( nFlags & SALSYSTEM_GETSYSTEMINFO_STACK )
    {
        DWORD nDebugThreadId;
        HANDLE hDebugThread = CreateThread(
                                NULL, 16000,
                                (LPTHREAD_START_ROUTINE)DebugThread, &aSysInfos,
                                0, &nDebugThreadId );

        WaitForSingleObject( hDebugThread, INFINITE );
        CloseHandle( hDebugThread );
    }
    if ( nFlags & SALSYSTEM_GETSYSTEMINFO_MODULES )
    {
        aSysInfos.aModules = "<Modules>\n";
        char buffer[1024];
        if ( !aSysInfos.pModInfos )
            aSysInfos.pModInfos = WNT_CreateModuleInfos();
        struct ModuleInfo* pM = aSysInfos.pModInfos;
        while ( pM )
        {
            aSysInfos.aModules += "  <Module name=\"";
            aSysInfos.aModules += pM->szModBaseName;
            aSysInfos.aModules += "\" path=\"";
            aSysInfos.aModules += pM->szModFileName;
            aSysInfos.aModules += "\" >\n";
            aSysInfos.aModules += "    <ModuleInfo name=\"BASE\" value=\"";
            sprintf( buffer, "%.8lx", pM->nBaseAddress );
            aSysInfos.aModules += buffer;
            aSysInfos.aModules += "\" />\n";
            aSysInfos.aModules += "    <ModuleInfo name=\"Size\" value=\"";
            aSysInfos.aModules += pM->nSize;
            aSysInfos.aModules += "\" />\n  </Module>\n";
            pM = pM->pNext;
        }
        aSysInfos.aModules += "</Modules>";
    }
    if ( nFlags & SALSYSTEM_GETSYSTEMINFO_SYSTEMVERSION )
    {
        aSysInfos.aSystemVersion = "<System name=\"";
        OSVERSIONINFO aVersionInfos;
        memset(&aVersionInfos, 0, sizeof( OSVERSIONINFO ) );
        aVersionInfos.dwOSVersionInfoSize = sizeof( OSVERSIONINFO );
        GetVersionEx( &aVersionInfos );
        if ( aVersionInfos.dwPlatformId == VER_PLATFORM_WIN32s )
            aSysInfos.aSystemVersion += "Microsoft Win32s";
        else if ( aVersionInfos.dwPlatformId == VER_PLATFORM_WIN32_NT )
            aSysInfos.aSystemVersion += "Microsoft Windows NT";
        else if ( aVersionInfos.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS )
        {
            if ( !aVersionInfos.dwMinorVersion )
                aSysInfos.aSystemVersion += "Microsoft Windows 95";
            else
                aSysInfos.aSystemVersion += "Microsoft Windows 98";
        }
        else
            aSysInfos.aSystemVersion += "Unknown Windows";
        aSysInfos.aSystemVersion += "\" version=\"";
        aSysInfos.aSystemVersion += aVersionInfos.dwMajorVersion;
        aSysInfos.aSystemVersion += '.';
        aSysInfos.aSystemVersion += aVersionInfos.dwMinorVersion;
        aSysInfos.aSystemVersion += "\" build=\"";
        aSysInfos.aSystemVersion += aVersionInfos.dwBuildNumber&0xFFFF;
        aSysInfos.aSystemVersion += "\" />";

        // aSysInfos.aSystemVersion += aVersionInfos.szCSDVersion;
        /*
            Under both Windows NT and Windows 95, you can get the
            language information in the FileVersionInfo of User.exe by
            calling GetFileVersionInfo, and then
            VerQueryValue (on \\VarFileInfo\\Translation") on the
            VersionInfo block of the operating system's User.exe.
            NOTE: This method is the most reliable. It works well under
            both Windows NT and Windows 95. This method also works for
            Windows 3.1
        */
//      DWORD nDefInputLanguage;
//      if ( SystemParametersInfo( SPI_GETDEFAULTINPUTLANG, 0, &nDefInputLanguage, 0 ) )
//      {
//          aSysInfos.aSystemVersion += " default input language = ";
//          aSysInfos.aSystemVersion += nDefInputLanguage;
//      }
    }
    if ( nFlags & SALSYSTEM_GETSYSTEMINFO_CPUTYPE )
    {
        SYSTEM_INFO aSystemInfo;
        memset( &aSystemInfo, 0, sizeof( SYSTEM_INFO ) );
        GetSystemInfo( &aSystemInfo );
        aSysInfos.aCPUType = "<CPU count=\"";
        aSysInfos.aCPUType += aSystemInfo.dwNumberOfProcessors;
        aSysInfos.aCPUType += "\" type=\"";
        if ( aSystemInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL )
            aSysInfos.aCPUType += "X86";
        else if ( aSystemInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL )
            aSysInfos.aCPUType += "MIPS";
        else if ( aSystemInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL )
            aSysInfos.aCPUType += "ALPHA";
        else if ( aSystemInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL )
            aSysInfos.aCPUType += "PPC";
        else
            aSysInfos.aCPUType += "unknown";
        aSysInfos.aCPUType += "\"></CPU>";
    }
    if ( nFlags & SALSYSTEM_GETSYSTEMINFO_MEMORYINFO )
    {
        MEMORYSTATUS aMemStatus;
        memset( &aMemStatus, 0, sizeof( MEMORYSTATUS ) );
        GlobalMemoryStatus( &aMemStatus );
        aSysInfos.aMemoryInfo = "<Memory>\n";
        aSysInfos.aMemoryInfo += "  <MemoryType name=\"Physical\" total=\"";
        aSysInfos.aMemoryInfo += (aMemStatus.dwTotalPhys+512)/1024/1024+1;
        aSysInfos.aMemoryInfo += " MB\" free=\"";
        aSysInfos.aMemoryInfo += (aMemStatus.dwAvailPhys+512)/1024/1024+1;
        aSysInfos.aMemoryInfo += " MB\" />\n";
        aSysInfos.aMemoryInfo += "  <MemoryType name=\"Swap\" total=\"";
        aSysInfos.aMemoryInfo += (aMemStatus.dwTotalPageFile+512)/1024/1024+1;
        aSysInfos.aMemoryInfo += " MB\" free=\"";
        aSysInfos.aMemoryInfo += (aMemStatus.dwAvailPageFile+512)/1024/1024+1;
        aSysInfos.aMemoryInfo += " MB\" />\n";
        aSysInfos.aMemoryInfo += "  <MemoryType name=\"Virtual\" total=\"";
        aSysInfos.aMemoryInfo += (aMemStatus.dwTotalVirtual+512)/1024/1024+1;
        aSysInfos.aMemoryInfo += " MB\" free=\"";
        aSysInfos.aMemoryInfo += (aMemStatus.dwAvailVirtual+512)/1024/1024+1;
        aSysInfos.aMemoryInfo += " MB\" />\n";
        aSysInfos.aMemoryInfo += "</Memory>";
    }
    if ( nFlags & SALSYSTEM_GETSYSTEMINFO_LOCALVOLUMES )
    {
        aSysInfos.aLocalVolumes = "<LocalVolumes>\n";
        char aDriveStrings[4096];
        GetLogicalDriveStrings( 4096, aDriveStrings );
        LPTSTR pDriveStr = aDriveStrings;
        while ( *pDriveStr )
        {
            UINT nType = GetDriveType( pDriveStr );
            if ( nType != DRIVE_REMOTE )
            {
                aSysInfos.aLocalVolumes += "  <LocalVolume type=\"";

                switch ( nType )
                {
                    case DRIVE_REMOVABLE:   aSysInfos.aLocalVolumes += "Removable"; break;
                    case DRIVE_FIXED:       aSysInfos.aLocalVolumes += "Fixed"; break;
                    case DRIVE_REMOTE:      aSysInfos.aLocalVolumes += "Remote";    break;
                    case DRIVE_CDROM:       aSysInfos.aLocalVolumes += "CD-ROM";    break;
                    case DRIVE_RAMDISK:     aSysInfos.aLocalVolumes += "RAM disk";  break;
                    default:                aSysInfos.aLocalVolumes += "Unkown";
                }
                aSysInfos.aLocalVolumes += "\" path=\"";
                aSysInfos.aLocalVolumes += pDriveStr;
                aSysInfos.aLocalVolumes += "\"";
                if( nType == DRIVE_FIXED )
                {
                    DWORD nSectorsPerCluster;
                    DWORD nBytesPerSector;
                    DWORD nNumberFreeClusters;
                    DWORD nNumberTotalClusters;
                    if( GetDiskFreeSpace( pDriveStr, &nSectorsPerCluster, &nBytesPerSector, &nNumberFreeClusters, &nNumberTotalClusters ) )
                    {
                        DWORD nUnitsPerCluster = nSectorsPerCluster*nBytesPerSector;
                        DWORD nUnitDivi = 1;
                        String aUnit = "bytes";
                        if( ( nUnitsPerCluster % 1024 ) == 0 )
                        {
                            aUnit = "KB";
                            nUnitsPerCluster /= 1024;
                        }
                        else if( ( nUnitsPerCluster % 512 ) == 0 )
                        {
                            aUnit = "KB";
                            nUnitDivi = 1024;
                        }
                        DWORD nFree = nNumberFreeClusters*nUnitsPerCluster/nUnitDivi;
                        aSysInfos.aLocalVolumes += " free=\"";
                        aSysInfos.aLocalVolumes += nFree;
                        aSysInfos.aLocalVolumes += ' ';
                        aSysInfos.aLocalVolumes += aUnit;
                        aSysInfos.aLocalVolumes += "\"";
                    }
                }
                aSysInfos.aLocalVolumes += " />\n";
            }
            while ( *pDriveStr )
                pDriveStr++;
            pDriveStr++;
        }
        aSysInfos.aLocalVolumes += "</LocalVolumes>";
    }
    if ( nFlags & SALSYSTEM_GETSYSTEMINFO_SYSTEMDIRS )
    {
        aSysInfos.aSystemDirs = "<SystemDirs>\n";
        char buffer[ MAX_PATH ];
        aSysInfos.aSystemDirs += "  <SystemDir envname=\"Windows\" path=\"";
        if( GetWindowsDirectory( buffer, MAX_PATH ) )
            aSysInfos.aSystemDirs += buffer;
        aSysInfos.aSystemDirs += "\" />\n";

        aSysInfos.aSystemDirs += "  <SystemDir envname=\"System\" path=\"";
        if( GetSystemDirectory( buffer, MAX_PATH ) )
            aSysInfos.aSystemDirs += buffer;
        aSysInfos.aSystemDirs += "\" />\n";

        aSysInfos.aSystemDirs += "  <SystemDir envname=\"Current\" path=\"";
        if( GetCurrentDirectory( MAX_PATH, buffer ) )
            aSysInfos.aSystemDirs += buffer;
        aSysInfos.aSystemDirs += "\" />\n";

        aSysInfos.aSystemDirs += "  <SystemDir envname=\"Temp\" path=\"";
        if( GetTempPath( MAX_PATH, buffer ) )
            aSysInfos.aSystemDirs += buffer;
        aSysInfos.aSystemDirs += "\" />\n";
        aSysInfos.aSystemDirs += "</SystemDirs>";

    }
    if ( nFlags & SALSYSTEM_GETSYSTEMINFO_MOUSEINFO )
    {
        aSysInfos.aMouseInfo = "<Mouse ";
        int nRet = GetSystemMetrics( SM_CMOUSEBUTTONS );
        if ( nRet )
        {
            aSysInfos.aMouseInfo += "buttons=\"";
            aSysInfos.aMouseInfo += nRet;
            aSysInfos.aMouseInfo += "\" description=\"";
            if( GetSystemMetrics( 75 /*SM_MOUSEWHEELPRESENT - missing in SDK from MSVC4.2 */ ) )
                aSysInfos.aMouseInfo += "wheel mouse";
            else
                aSysInfos.aMouseInfo += "standard mouse";
            aSysInfos.aMouseInfo += "\"";
        }
        else
        {
            aSysInfos.aMouseInfo += "description=\"Not installed.\"";
        }
        aSysInfos.aMouseInfo += " />";
    }

    CloseHandle( aSysInfos.hCurrentThread );
    CloseHandle( aSysInfos.hCurrentProcess );

    String aInfos;

    aInfos += aSysInfos.aSystemVersion;
    aInfos += "\n\n";

    aInfos += aSysInfos.aCPUType;
    aInfos += "\n\n";

    aInfos += aSysInfos.aMouseInfo;
    aInfos += "\n\n";

    aInfos += aSysInfos.aMemoryInfo;
    aInfos += "\n\n";

    aInfos += aSysInfos.aStack;
    aInfos += "\n\n";

    aInfos += aSysInfos.aModules;
    aInfos += "\n\n";

    aInfos += aSysInfos.aLocalVolumes;
    aInfos += "\n\n";

    aInfos += aSysInfos.aSystemDirs;
    aInfos += "\n\n";

    return aInfos;
}




void DebugThread( SystemInfos* pSysInfos )
{
    HINSTANCE hImageHelpLib = LoadLibrary( "IMAGEHLP.DLL" );
    if( hImageHelpLib )
    {
        if ( SuspendThread( pSysInfos->hCurrentThread ) != 0xFFFFFFFF )
        {
            STACKFRAME aStackFrame;
            memset( &aStackFrame, 0, sizeof( aStackFrame ) );

            CONTEXT aContext;
            memset( &aContext, 0, sizeof( aContext ) );
            aContext.ContextFlags = CONTEXT_FULL;
            if ( GetThreadContext( pSysInfos->hCurrentThread, &aContext ) )
            {
                STACKWALKPROC _fpStackWalk = (STACKWALKPROC) GetProcAddress( hImageHelpLib, "StackWalk" );
                SYMFUNCTIONTABLEACCESSPROC _fpSymFunctionTableAccess = (SYMFUNCTIONTABLEACCESSPROC) GetProcAddress( hImageHelpLib, "SymFunctionTableAccess" );
                SYMGETMODULEBASEPROC _fpSymGetModuleBase = (SYMGETMODULEBASEPROC) GetProcAddress( hImageHelpLib, "SymGetModuleBase" );
                SYMSETOPTIONSPROC _fpSymSetOptionsProc = (SYMSETOPTIONSPROC) GetProcAddress( hImageHelpLib, "SymGetOptions" );
                SYMINITIALIZEPROC _fpSymInitializeProc = (SYMINITIALIZEPROC) GetProcAddress( hImageHelpLib, "SymInitialize" );
                SYMCLEANUPPROC _fpSymCleanup = (SYMCLEANUPPROC) GetProcAddress( hImageHelpLib, "SymCleanup" );
                UNDECORATESYMBOLNAMEPROC _fpUndecorateSymbolName = (UNDECORATESYMBOLNAMEPROC) GetProcAddress( hImageHelpLib, "UnDecorateSymbolName" );
                SYMLOADMODULEPROC _fpSymLoadModule = ( SYMLOADMODULEPROC) GetProcAddress( hImageHelpLib, "SymLoadModule" );
                SYMGETSYMFROMADDR _fpSymGetSymFromAddr = ( SYMGETSYMFROMADDR ) GetProcAddress( hImageHelpLib, "SymGetSymFromAddr" );

                if ( !pSysInfos->pModInfos )
                    pSysInfos->pModInfos = WNT_CreateModuleInfos();

                _fpSymSetOptionsProc( SYMOPT_DEFERRED_LOADS );

                char buffer[1024];

                // Initialize the imagehlp symbol handler
                BOOL bAutoLoad = FALSE;
//              String aPath = WNT_CreateModulePath( pSysInfos->pModInfos );
//              USHORT nLen = aPath.Len();
//              memcpy( buffer, aPath.GetStr(), nLen );
//              buffer[nLen] = 0;
//              BOOL bSymbols = _fpSymInitializeProc( pSysInfos->hCurrentProcess, NULL, bAutoLoad );
                // Path funktioniert nicht, also lade ich unten alle von Hand!
                BOOL bSymbols = _fpSymInitializeProc( pSysInfos->hCurrentProcess, NULL, bAutoLoad );

                // Load symbol modules for the current process
                if ( bSymbols && !bAutoLoad )
                {
                    // LoadModuleSymbols( pSysInfos->nCurrentProcessId, pSysInfos->hCurrentProcess );
                    struct ModuleInfo* pM = pSysInfos->pModInfos;
                    char buffer1[1024];
                    char buffer2[1024];

                    while ( pM )
                    {
                        strcpy( buffer1, pM->szModFileName );
                        strcpy( buffer2, pM->szModBaseName );

                        BOOL bDone = _fpSymLoadModule( pSysInfos->hCurrentProcess, 0, buffer1, buffer2, pM->nBaseAddress, pM->nSize );

                        pM = pM->pNext;
                    }
                }

                // Initialize the STACKFRAME structure for the first call.  This is only
                // necessary for Intel CPUs, and isn't mentioned in the documentation.
                aStackFrame.AddrPC.Offset       = aContext.Eip;
                aStackFrame.AddrPC.Mode         = AddrModeFlat;
                aStackFrame.AddrStack.Offset    = aContext.Esp;
                aStackFrame.AddrStack.Mode      = AddrModeFlat;
                aStackFrame.AddrFrame.Offset    = aContext.Ebp;
                aStackFrame.AddrFrame.Mode      = AddrModeFlat;


                pSysInfos->aStack = "<Stack type=\"WIN32\" >\n";

                for ( int nS = 0; nS < 20; nS++ )
                {
                    SetLastError( 0 );
                    BOOL bStack = _fpStackWalk( IMAGE_FILE_MACHINE_I386,
                                                pSysInfos->hCurrentProcess,
                                                pSysInfos->hCurrentThread,
                                                &aStackFrame,
                                                &aContext,
                                                NULL, // ReadProcessMemory,
                                                _fpSymFunctionTableAccess,
                                                _fpSymGetModuleBase,
                                                NULL );

                    if ( !bStack || !aStackFrame.AddrReturn.Offset || !aStackFrame.AddrFrame.Offset )
                        break;

                    pSysInfos->aStack += "  <StackInfo pos=\"";
                    pSysInfos->aStack += (USHORT)nS;
                    pSysInfos->aStack += "\" ip=\"";
                    ULONG nIP = aStackFrame.AddrReturn.Offset;
                    sprintf( buffer, "%.8lx", nIP );
                    pSysInfos->aStack += buffer;
                    pSysInfos->aStack += "\"";

                    TCHAR aModuleFileName[MAX_PATH];
                    DWORD section, offset;
                    if ( WNT_GetLogicalAddress( (void*)nIP, aModuleFileName, MAX_PATH, section, offset ) )
                    {
                        pSysInfos->aStack += " rel=\"";
                        sprintf( buffer, "%.8lx", offset );
                        pSysInfos->aStack += buffer;
                        pSysInfos->aStack += "\" file=\"";
                        char* pModName = strrchr( aModuleFileName, '\\' );
                        pSysInfos->aStack += pModName ? (pModName+1) : aModuleFileName;
                        pSysInfos->aStack += "\"";
                    }
                    else
                    {
                        pSysInfos->aStack += " rel=\"ERROR\"";
                        break;
                    }

                    if ( bSymbols )
                    {

                        BYTE symbolBuffer[ sizeof(IMAGEHLP_SYMBOL) + 512 ];
                        PIMAGEHLP_SYMBOL pSymbol = (PIMAGEHLP_SYMBOL)symbolBuffer;
                        memset( symbolBuffer, 0, sizeof(symbolBuffer) );
                        pSymbol->SizeOfStruct = sizeof(symbolBuffer);
                        pSymbol->MaxNameLength = sizeof(symbolBuffer) - sizeof(IMAGEHLP_SYMBOL) + 1;

                        DWORD symDisplacement = 0;  // Displacement of the input address,
                                                    // relative to the start of the symbol

                        if ( _fpSymGetSymFromAddr( pSysInfos->hCurrentProcess, aStackFrame.AddrReturn.Offset,
                                                &symDisplacement, pSymbol ) )
                        {
                            pSysInfos->aStack += " ordinal=\"";
                            _fpUndecorateSymbolName( pSymbol->Name, buffer, 1024, UNDNAME_NAME_ONLY );
                            pSysInfos->aStack += buffer;
                            pSysInfos->aStack += "\"";
                            // aStackLine += '<';
                            // _fpUndecorateSymbolName( pSymbol->Name, buffer, 1024, UNDNAME_COMPLETE );
                            // aStackLine += buffer;
                            // aStackLine += '>';
                        }
                        else    // No symbol found.  Print out the logical address instead.
                        {
                            pSysInfos->aStack += " ordinal=\"???\"";
                        }
                    }

                    pSysInfos->aStack += " />\n";
                }
                pSysInfos->aStack += "</Stack>";
                _fpSymCleanup( pSysInfos->hCurrentProcess );
            }
            ResumeThread( pSysInfos->hCurrentThread );
        }
        FreeLibrary( hImageHelpLib );
    }
}

#endif
