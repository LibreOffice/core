/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include <osl/diagnose.h>
#include "SysShExec.hxx"
#include <osl/file.hxx>
#include <sal/macros.h>

#include <com/sun/star/system/SystemShellExecuteFlags.hpp>
#include <com/sun/star/uri/UriReferenceFactory.hpp>

#define WIN32_LEAN_AND_MEAN
#if defined _MSC_VER
#pragma warning(push, 1)
#endif
#include <windows.h>
#include <shellapi.h>
#include <objbase.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif

//------------------------------------------------------------------------
// namespace directives
//------------------------------------------------------------------------

using com::sun::star::uno::Reference;
using com::sun::star::uno::RuntimeException;
using com::sun::star::uno::Sequence;
using com::sun::star::uno::XInterface;
using com::sun::star::lang::EventObject;
using com::sun::star::lang::XServiceInfo;
using com::sun::star::lang::IllegalArgumentException;
using rtl::OUString;
using osl::Mutex;
using com::sun::star::system::XSystemShellExecute;
using com::sun::star::system::SystemShellExecuteException;

using namespace ::com::sun::star::system::SystemShellExecuteFlags;
using namespace cppu;

#define SYSSHEXEC_IMPL_NAME  "com.sun.star.sys.shell.SystemShellExecute"

//------------------------------------------------------------------------
// helper functions
//------------------------------------------------------------------------

namespace // private
{
    namespace css = com::sun::star;

    Sequence< OUString > SAL_CALL SysShExec_getSupportedServiceNames()
    {
        Sequence< OUString > aRet(1);
        aRet[0] = OUString("com.sun.star.sys.shell.SystemShellExecute");
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
                return (oslFileError)errtable[i].errnocode;
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

    //-----------------------------------------

    bool is_system_path(const OUString& path_or_uri)
    {
        OUString url;
        osl::FileBase::RC rc = osl::FileBase::getFileURLFromSystemPath(path_or_uri, url);
        return (rc == osl::FileBase::E_None);
    }

    //-----------------------------------------
    // trying to identify a jump mark
    //-----------------------------------------

    const OUString    JUMP_MARK_HTM(".htm#");
    const OUString    JUMP_MARK_HTML(".html#");
    const sal_Unicode HASH_MARK      = (sal_Unicode)'#';

    bool has_jump_mark(const OUString& system_path, sal_Int32* jmp_mark_start = NULL)
    {
        sal_Int32 jmp_mark = std::max<int>(
            system_path.lastIndexOf(JUMP_MARK_HTM),
            system_path.lastIndexOf(JUMP_MARK_HTML));

        if (jmp_mark_start)
            *jmp_mark_start = jmp_mark;

        return (jmp_mark > -1);
    }

    //-----------------------------------------

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

    //-------------------------------------------------
    // Jump marks in file urls are illegal.
    //-------------------------------------------------

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

} // end namespace

//-----------------------------------------------------------------------------------------

CSysShExec::CSysShExec( const Reference< css::uno::XComponentContext >& xContext ) :
    WeakComponentImplHelper2< XSystemShellExecute, XServiceInfo >( m_aMutex ),
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
    CoInitialize( NULL );
}

//-------------------------------------------------

void SAL_CALL CSysShExec::execute( const OUString& aCommand, const OUString& aParameter, sal_Int32 nFlags )
        throw (IllegalArgumentException, SystemShellExecuteException, RuntimeException)
{
    // parameter checking
    if (0 == aCommand.getLength())
        throw IllegalArgumentException(
            OUString("Empty command"),
            static_cast< XSystemShellExecute* >( this ),
            1 );

    if ((nFlags & ~(NO_SYSTEM_ERROR_MESSAGE | URIS_ONLY)) != 0)
        throw IllegalArgumentException(
            OUString("Invalid Flags specified"),
            static_cast< XSystemShellExecute* >( this ),
            3 );

    if ((nFlags & URIS_ONLY) != 0)
    {
        css::uno::Reference< css::uri::XUriReference > uri(
            css::uri::UriReferenceFactory::create(m_xContext)->parse(aCommand));
        if (!(uri.is() && uri->isAbsolute()))
        {
            throw css::lang::IllegalArgumentException(
                (rtl::OUString(
                        "XSystemShellExecute.execute URIS_ONLY with"
                        " non-absolute URI reference ")
                 + aCommand),
                static_cast< cppu::OWeakObject * >(this), 0);
        }
    }

    /*  #i4789#; jump mark detection on system paths
        if the given command is a system path (not http or
        other uri schemes) and seems to have a jump mark
        and names no existing file (remeber the jump mark
        sign '#' is a valid file name character we remove
        the jump mark, else ShellExecuteEx fails */
    OUString preprocessed_command(aCommand);
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
    }

    SHELLEXECUTEINFOW sei;
    ZeroMemory(&sei, sizeof( sei));

    sei.cbSize       = sizeof(sei);
    sei.lpFile       = reinterpret_cast<LPCWSTR>(preprocessed_command.getStr());
    sei.lpParameters = reinterpret_cast<LPCWSTR>(aParameter.getStr());
    sei.nShow        = SW_SHOWNORMAL;

    if (NO_SYSTEM_ERROR_MESSAGE & nFlags)
        sei.fMask = SEE_MASK_FLAG_NO_UI;

    SetLastError( 0 );

    sal_Bool bRet = ShellExecuteExW(&sei) ? sal_True : sal_False;

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
            OUString("Error executing command"),
            static_cast< XSystemShellExecute* >(this),
            psxErr);
    }
}

// -------------------------------------------------
// XServiceInfo
// -------------------------------------------------

OUString SAL_CALL CSysShExec::getImplementationName(  )
    throw( RuntimeException )
{
    return OUString(SYSSHEXEC_IMPL_NAME );
}

// -------------------------------------------------
//  XServiceInfo
// -------------------------------------------------

sal_Bool SAL_CALL CSysShExec::supportsService( const OUString& ServiceName )
    throw( RuntimeException )
{
    Sequence < OUString > SupportedServicesNames = SysShExec_getSupportedServiceNames();

    for ( sal_Int32 n = SupportedServicesNames.getLength(); n--; )
        if (SupportedServicesNames[n].compareTo(ServiceName) == 0)
            return sal_True;

    return sal_False;
}

// -------------------------------------------------
//  XServiceInfo
// -------------------------------------------------

Sequence< OUString > SAL_CALL CSysShExec::getSupportedServiceNames(  )
    throw( RuntimeException )
{
    return SysShExec_getSupportedServiceNames();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
