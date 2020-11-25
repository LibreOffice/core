/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <algorithm>
#include <cassert>

#include <osl/diagnose.h>
#include <osl/process.h>
#include <sal/log.hxx>
#include "SysShExec.hxx"
#include <osl/file.hxx>
#include <sal/macros.h>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/system/SystemShellExecuteException.hpp>
#include <com/sun/star/system/SystemShellExecuteFlags.hpp>
#include <com/sun/star/uri/UriReferenceFactory.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <o3tl/char16_t2wchar_t.hxx>
#include <o3tl/runtimetooustring.hxx>
#include <rtl/uri.hxx>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>
#include <Shlobj.h>
#include <Shobjidl.h>
#include <objbase.h>

#include <systools/win32/comtools.hxx>

using com::sun::star::uno::Reference;
using com::sun::star::uno::RuntimeException;
using com::sun::star::uno::Sequence;
using com::sun::star::lang::XServiceInfo;
using com::sun::star::lang::IllegalArgumentException;
using com::sun::star::system::XSystemShellExecute;
using com::sun::star::system::SystemShellExecuteException;

using namespace ::com::sun::star::system::SystemShellExecuteFlags;
using namespace cppu;

#define SYSSHEXEC_IMPL_NAME  "com.sun.star.sys.shell.SystemShellExecute"

namespace
{
    Sequence< OUString > SysShExec_getSupportedServiceNames()
    {
        Sequence< OUString > aRet { "com.sun.star.system.SystemShellExecute" };
        return aRet;
    }

    /* This is the error table that defines the mapping between OS error
    codes and errno values */

    struct errentry {
        unsigned long oscode;   /* OS return value */
        int errnocode;          /* System V error code */
    };

    struct errentry errtable[] = {
        {  ERROR_SUCCESS,                osl_File_E_None     },  /* 0 */
        {  ERROR_INVALID_FUNCTION,       osl_File_E_INVAL    },  /* 1 */
        {  ERROR_FILE_NOT_FOUND,         osl_File_E_NOENT    },  /* 2 */
        {  ERROR_PATH_NOT_FOUND,         osl_File_E_NOENT    },  /* 3 */
        {  ERROR_TOO_MANY_OPEN_FILES,    osl_File_E_MFILE    },  /* 4 */
        {  ERROR_ACCESS_DENIED,          osl_File_E_ACCES    },  /* 5 */
        {  ERROR_INVALID_HANDLE,         osl_File_E_BADF     },  /* 6 */
        {  ERROR_ARENA_TRASHED,          osl_File_E_NOMEM    },  /* 7 */
        {  ERROR_NOT_ENOUGH_MEMORY,      osl_File_E_NOMEM    },  /* 8 */
        {  ERROR_INVALID_BLOCK,          osl_File_E_NOMEM    },  /* 9 */
        {  ERROR_BAD_ENVIRONMENT,        osl_File_E_2BIG     },  /* 10 */
        {  ERROR_BAD_FORMAT,             osl_File_E_NOEXEC   },  /* 11 */
        {  ERROR_INVALID_ACCESS,         osl_File_E_INVAL    },  /* 12 */
        {  ERROR_INVALID_DATA,           osl_File_E_INVAL    },  /* 13 */
        {  ERROR_INVALID_DRIVE,          osl_File_E_NOENT    },  /* 15 */
        {  ERROR_CURRENT_DIRECTORY,      osl_File_E_ACCES    },  /* 16 */
        {  ERROR_NOT_SAME_DEVICE,        osl_File_E_XDEV     },  /* 17 */
        {  ERROR_NO_MORE_FILES,          osl_File_E_NOENT    },  /* 18 */
        {  ERROR_LOCK_VIOLATION,         osl_File_E_ACCES    },  /* 33 */
        {  ERROR_BAD_NETPATH,            osl_File_E_NOENT    },  /* 53 */
        {  ERROR_NETWORK_ACCESS_DENIED,  osl_File_E_ACCES    },  /* 65 */
        {  ERROR_BAD_NET_NAME,           osl_File_E_NOENT    },  /* 67 */
        {  ERROR_FILE_EXISTS,            osl_File_E_EXIST    },  /* 80 */
        {  ERROR_CANNOT_MAKE,            osl_File_E_ACCES    },  /* 82 */
        {  ERROR_FAIL_I24,               osl_File_E_ACCES    },  /* 83 */
        {  ERROR_INVALID_PARAMETER,      osl_File_E_INVAL    },  /* 87 */
        {  ERROR_NO_PROC_SLOTS,          osl_File_E_AGAIN    },  /* 89 */
        {  ERROR_DRIVE_LOCKED,           osl_File_E_ACCES    },  /* 108 */
        {  ERROR_BROKEN_PIPE,            osl_File_E_PIPE     },  /* 109 */
        {  ERROR_DISK_FULL,              osl_File_E_NOSPC    },  /* 112 */
        {  ERROR_INVALID_TARGET_HANDLE,  osl_File_E_BADF     },  /* 114 */
        {  ERROR_INVALID_HANDLE,         osl_File_E_INVAL    },  /* 124 */
        {  ERROR_WAIT_NO_CHILDREN,       osl_File_E_CHILD    },  /* 128 */
        {  ERROR_CHILD_NOT_COMPLETE,     osl_File_E_CHILD    },  /* 129 */
        {  ERROR_DIRECT_ACCESS_HANDLE,   osl_File_E_BADF     },  /* 130 */
        {  ERROR_NEGATIVE_SEEK,          osl_File_E_INVAL    },  /* 131 */
        {  ERROR_SEEK_ON_DEVICE,         osl_File_E_ACCES    },  /* 132 */
        {  ERROR_DIR_NOT_EMPTY,          osl_File_E_NOTEMPTY },  /* 145 */
        {  ERROR_NOT_LOCKED,             osl_File_E_ACCES    },  /* 158 */
        {  ERROR_BAD_PATHNAME,           osl_File_E_NOENT    },  /* 161 */
        {  ERROR_MAX_THRDS_REACHED,      osl_File_E_AGAIN    },  /* 164 */
        {  ERROR_LOCK_FAILED,            osl_File_E_ACCES    },  /* 167 */
        {  ERROR_ALREADY_EXISTS,         osl_File_E_EXIST    },  /* 183 */
        {  ERROR_FILENAME_EXCED_RANGE,   osl_File_E_NOENT    },  /* 206 */
        {  ERROR_NESTING_NOT_ALLOWED,    osl_File_E_AGAIN    },  /* 215 */
        {  ERROR_NOT_ENOUGH_QUOTA,       osl_File_E_NOMEM    }    /* 1816 */
    };

    /* size of the table */
    #define ERRTABLESIZE (SAL_N_ELEMENTS(errtable))

    /* The following two constants must be the minimum and maximum
    values in the (contiguous) range of osl_File_E_xec Failure errors. */
    #define MIN_EXEC_ERROR ERROR_INVALID_STARTING_CODESEG
    #define MAX_EXEC_ERROR ERROR_INFLOOP_IN_RELOC_CHAIN

    /* These are the low and high value in the range of errors that are
    access violations */
    #define MIN_EACCES_RANGE ERROR_WRITE_PROTECT
    #define MAX_EACCES_RANGE ERROR_SHARING_BUFFER_EXCEEDED


    /*******************************************************************************/

    oslFileError _mapError( DWORD dwError )
    {
        unsigned i;

        /* check the table for the OS error code */
        for ( i = 0; i < ERRTABLESIZE; ++i )
        {
            if ( dwError == errtable[i].oscode )
                return static_cast<oslFileError>(errtable[i].errnocode);
        }

        /* The error code wasn't in the table.  We check for a range of */
        /* osl_File_E_ACCES errors or exec failure errors (ENOEXEC).  Otherwise   */
        /* osl_File_E_INVAL is returned.                                          */

        if ( dwError >= MIN_EACCES_RANGE && dwError <= MAX_EACCES_RANGE)
            return osl_File_E_ACCES;
        else if ( dwError >= MIN_EXEC_ERROR && dwError <= MAX_EXEC_ERROR)
            return osl_File_E_NOEXEC;
        else
            return osl_File_E_INVAL;
    }

    #define MapError( oserror ) _mapError( oserror )

    #define E_UNKNOWN_EXEC_ERROR -1


    bool is_system_path(const OUString& path_or_uri)
    {
        OUString url;
        osl::FileBase::RC rc = osl::FileBase::getFileURLFromSystemPath(path_or_uri, url);
        return (rc == osl::FileBase::E_None);
    }


    // trying to identify a jump mark


    const OUString    JUMP_MARK_HTM(".htm#");
    const OUString    JUMP_MARK_HTML(".html#");
    const sal_Unicode HASH_MARK      = '#';

    bool has_jump_mark(const OUString& system_path, sal_Int32* jmp_mark_start = nullptr)
    {
        sal_Int32 jmp_mark = std::max<int>(
            system_path.lastIndexOf(JUMP_MARK_HTM),
            system_path.lastIndexOf(JUMP_MARK_HTML));

        if (jmp_mark_start)
            *jmp_mark_start = jmp_mark;

        return (jmp_mark > -1);
    }


    bool is_existing_file(const OUString& file_name)
    {
        OSL_ASSERT(is_system_path(file_name));

        bool exist = false;

        OUString file_url;
        osl::FileBase::RC rc = osl::FileBase::getFileURLFromSystemPath(file_name, file_url);

        if (osl::FileBase::E_None == rc)
        {
            osl::DirectoryItem dir_item;
            rc = osl::DirectoryItem::get(file_url, dir_item);
            exist = (osl::FileBase::E_None == rc);
        }
        return exist;
    }


    // Jump marks in file urls are illegal.


    void remove_jump_mark(OUString* p_command)
    {
        OSL_PRECOND(p_command, "invalid parameter");

        sal_Int32 pos;
        if (has_jump_mark(*p_command, &pos))
        {
            const sal_Unicode* p_jmp_mark = p_command->getStr() + pos;
            while (*p_jmp_mark && (*p_jmp_mark != HASH_MARK))
                p_jmp_mark++;

            *p_command = OUString(p_command->getStr(), p_jmp_mark - p_command->getStr());
        }
    }

}

CSysShExec::CSysShExec( const Reference< css::uno::XComponentContext >& xContext ) :
    WeakComponentImplHelper< XSystemShellExecute, XServiceInfo >( m_aMutex ),
    m_xContext(xContext)
{
    /*
     * As this service is declared thread-affine, it is ensured to be called from a
     * dedicated thread, so initialize COM here.
     *
     * We need COM to be initialized for STA, but osl thread get initialized for MTA.
     * Once this changed, we can remove the uninitialize call.
     */
    CoUninitialize();
    CoInitialize( nullptr );
}

namespace
{
bool checkExtension(OUString const & extension, OUString const & blacklist) {
    assert(!extension.isEmpty());
    for (sal_Int32 i = 0; i != -1;) {
        OUString tok = blacklist.getToken(0, ';', i);
        tok.startsWith(".", &tok);
        if (extension.equalsIgnoreAsciiCase(tok)) {
            return false;
        }
    }
    return true;
}

// This callback checks if the found window is the specified process's top-level window,
// and activates the first found such window.
BOOL CALLBACK FindAndActivateProcWnd(HWND hwnd, LPARAM lParam)
{
    if (!IsWindowVisible(hwnd))
        return TRUE; // continue enumeration
    if (GetWindow(hwnd, GW_OWNER)) // not a top-level window
        return TRUE; // continue enumeration
    const DWORD nParamProcId = static_cast<DWORD>(lParam);
    assert(nParamProcId != 0);
    DWORD nWndProcId = 0;
    (void)GetWindowThreadProcessId(hwnd, &nWndProcId);
    if (nWndProcId != nParamProcId)
        return TRUE; // continue enumeration

    // Found it! Bring it to front
    if (IsIconic(hwnd))
    {
        ShowWindow(hwnd, SW_RESTORE);
    }
    SetForegroundWindow(hwnd);
    SetActiveWindow(hwnd);
    return FALSE; // stop enumeration
}
}

void SAL_CALL CSysShExec::execute( const OUString& aCommand, const OUString& aParameter, sal_Int32 nFlags )
{
    // parameter checking
    if (0 == aCommand.getLength())
        throw IllegalArgumentException(
            "Empty command",
            static_cast< XSystemShellExecute* >( this ),
            1 );

    if ((nFlags & ~(NO_SYSTEM_ERROR_MESSAGE | URIS_ONLY)) != 0)
        throw IllegalArgumentException(
            "Invalid Flags specified",
            static_cast< XSystemShellExecute* >( this ),
            3 );

    OUString preprocessed_command(aCommand);
    if ((nFlags & URIS_ONLY) != 0)
    {
        css::uno::Reference< css::uri::XUriReference > uri(
            css::uri::UriReferenceFactory::create(m_xContext)->parse(aCommand));
        if (!(uri.is() && uri->isAbsolute()))
        {
            throw css::lang::IllegalArgumentException(
                "XSystemShellExecute.execute URIS_ONLY with"
                         " non-absolute URI reference "
                 + aCommand,
                static_cast< cppu::OWeakObject * >(this), 0);
        }
        if (uri->getScheme().equalsIgnoreAsciiCase("file")) {
            // ShellExecuteExW appears to ignore the fragment of a file URL anyway, so remove it:
            uri->clearFragment();
            preprocessed_command = uri->getUriReference();
            OUString pathname;
            auto const e1
                = osl::FileBase::getSystemPathFromFileURL(uri->getUriReference(), pathname);
            if (e1 != osl::FileBase::E_None) {
                throw css::lang::IllegalArgumentException(
                    ("XSystemShellExecute.execute, getSystemPathFromFileURL <" + aCommand
                     + "> failed with " + OUString::number(e1)),
                    {}, 0);
            }
            const int MAX_LONG_PATH = 32767; // max longpath on WinNT
            if (pathname.getLength() >= MAX_LONG_PATH)
            {
                throw css::lang::IllegalArgumentException(
                    "XSystemShellExecute.execute, path <" + pathname + "> too long", {}, 0);
            }
            wchar_t path[MAX_LONG_PATH];
            wcscpy_s(path, o3tl::toW(pathname.getStr()));
            for (int i = 0;; ++i) {
                // tdf#130216: normalize c:\path\to\something\..\else into c:\path\to\else
                if (PathResolve(path, nullptr, PRF_VERIFYEXISTS | PRF_REQUIREABSOLUTE) == 0)
                {
                    throw css::lang::IllegalArgumentException(
                        "XSystemShellExecute.execute, PathResolve(" + OUString(o3tl::toU(path))
                            + ") failed",
                        {}, 0);
                }
                SHFILEINFOW info;
                if (SHGetFileInfoW(path, 0, &info, sizeof info, SHGFI_EXETYPE) != 0)
                {
                    throw css::lang::IllegalArgumentException(
                        "XSystemShellExecute.execute, cannot process <" + aCommand + ">", {}, 0);
                }
                if (SHGetFileInfoW(path, 0, &info, sizeof info, SHGFI_ATTRIBUTES) == 0)
                {
                    throw css::lang::IllegalArgumentException(
                        "XSystemShellExecute.execute, SHGetFileInfoW(" + OUString(o3tl::toU(path)) + ") failed", {},
                        0);
                }
                if ((info.dwAttributes & SFGAO_LINK) == 0) {
                    break;
                }
                sal::systools::COMReference<IShellLinkW> link;
                auto e2 = CoCreateInstance(
                    CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER, IID_IShellLinkW,
                    reinterpret_cast<LPVOID *>(&link));
                if (FAILED(e2)) {
                    throw css::lang::IllegalArgumentException(
                        ("XSystemShellExecute.execute, CoCreateInstance failed with "
                         + OUString::number(e2)),
                        {}, 0);
                }
                sal::systools::COMReference<IPersistFile> file;
                try {
                    file = link.QueryInterface<IPersistFile>(IID_IPersistFile);
                } catch(sal::systools::ComError & e3) {
                    throw css::lang::IllegalArgumentException(
                        ("XSystemShellExecute.execute, QueryInterface failed with: "
                         + o3tl::runtimeToOUString(e3.what())),
                        {}, 0);
                }
                e2 = file->Load(path, STGM_READ);
                if (FAILED(e2)) {
                    throw css::lang::IllegalArgumentException(
                        ("XSystemShellExecute.execute, IPersistFile.Load failed with "
                         + OUString::number(e2)),
                        {}, 0);
                }
                e2 = link->Resolve(nullptr, SLR_UPDATE | SLR_NO_UI);
                if (FAILED(e2)) {
                    throw css::lang::IllegalArgumentException(
                        ("XSystemShellExecute.execute, IShellLink.Resolve failed with "
                         + OUString::number(e2)),
                        {}, 0);
                }
                WIN32_FIND_DATAW wfd;
                e2 = link->GetPath(path, SAL_N_ELEMENTS(path), &wfd, SLGP_RAWPATH);
                if (FAILED(e2)) {
                    throw css::lang::IllegalArgumentException(
                        ("XSystemShellExecute.execute, IShellLink.GetPath failed with "
                         + OUString::number(e2)),
                        {}, 0);
                }
                // Fail at some arbitrary nesting depth, to avoid an infinite loop:
                if (i == 30) {
                    throw css::lang::IllegalArgumentException(
                        "XSystemShellExecute.execute, link depth exceeded for <" + aCommand + ">",
                        {}, 0);
                }
            }
            pathname = o3tl::toU(path);
            auto const n = pathname.lastIndexOf('.');
            if (n > pathname.lastIndexOf('\\')) {
                auto const ext = pathname.copy(n + 1);
                OUString env;
                if (osl_getEnvironment(OUString("PATHEXT").pData, &env.pData) != osl_Process_E_None)
                {
                    SAL_INFO("shell", "osl_getEnvironment(PATHEXT) failed");
                }
                if (!(checkExtension(ext, env)
                      && checkExtension(
                          ext,
                          ".COM;.EXE;.BAT;.CMD;.VBS;.VBE;.JS;.JSE;.WSF;.WSH;.MSC;.PY;.CLASS;.JAR")))
                {
                    throw css::lang::IllegalArgumentException(
                        "XSystemShellExecute.execute, cannot process <" + aCommand + ">", {}, 0);
                }
            }
        }
    }

    /*  #i4789#; jump mark detection on system paths
        if the given command is a system path (not http or
        other uri schemes) and seems to have a jump mark
        and names no existing file (remember the jump mark
        sign '#' is a valid file name character we remove
        the jump mark, else ShellExecuteEx fails */
    if (is_system_path(preprocessed_command))
    {
        if (has_jump_mark(preprocessed_command) && !is_existing_file(preprocessed_command))
            remove_jump_mark(&preprocessed_command);
    }
    /* Convert file uris to system paths */
    else
    {
        OUString aSystemPath;
        if (::osl::FileBase::E_None == ::osl::FileBase::getSystemPathFromFileURL(preprocessed_command, aSystemPath))
            preprocessed_command = aSystemPath;
        else if (preprocessed_command.startsWithIgnoreAsciiCase("file:"))
            //I use ToIUri conversion instead of the translateToExternal method of the css.uri.ExternalUriReferenceTranslator
            //UNO service, because the translateToExternal method only supports characters covered by the current Windows code page.
            preprocessed_command = rtl::Uri::decode(preprocessed_command, rtl_UriDecodeToIuri, RTL_TEXTENCODING_UTF8);
    }

    SHELLEXECUTEINFOW sei;
    ZeroMemory(&sei, sizeof( sei));

    sei.cbSize       = sizeof(sei);
    sei.lpFile       = o3tl::toW(preprocessed_command.getStr());
    sei.lpParameters = o3tl::toW(aParameter.getStr());
    sei.nShow        = SW_SHOWNORMAL;
    sei.fMask = SEE_MASK_NOCLOSEPROCESS; // we need sei.hProcess

    if (NO_SYSTEM_ERROR_MESSAGE & nFlags)
        sei.fMask |= SEE_MASK_FLAG_NO_UI;

    SetLastError( 0 );

    bool bRet = ShellExecuteExW(&sei);

    if (!bRet && (nFlags & NO_SYSTEM_ERROR_MESSAGE))
    {
        // ShellExecuteEx fails to set an error code
        // we return osl_File_E_INVAL
        sal_Int32 psxErr = GetLastError();
        if (ERROR_SUCCESS == psxErr)
            psxErr = E_UNKNOWN_EXEC_ERROR;
        else
            psxErr = MapError(psxErr);

        throw SystemShellExecuteException(
            "Error executing command",
            static_cast< XSystemShellExecute* >(this),
            psxErr);
    }
    else
    {
        // Get Permission make changes to the Window of the created Process
        const DWORD procId = GetProcessId(sei.hProcess);
        if (procId != 0)
        {
            AllowSetForegroundWindow(procId);
            WaitForInputIdle(sei.hProcess, 1000); // so that main window is created; imperfect
            EnumWindows(FindAndActivateProcWnd, static_cast<LPARAM>(procId));
        }
    }

    // Close the handle for the created childprocess when we are done
    CloseHandle(sei.hProcess);
}

// XServiceInfo

OUString SAL_CALL CSysShExec::getImplementationName(  )
{
    return SYSSHEXEC_IMPL_NAME;
}

sal_Bool SAL_CALL CSysShExec::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

Sequence< OUString > SAL_CALL CSysShExec::getSupportedServiceNames(  )
{
    return SysShExec_getSupportedServiceNames();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
