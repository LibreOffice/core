/*************************************************************************
 *
 *  $RCSfile: procimpl.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-09-29 14:40:32 $
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

#define UNICODE
#define _UNICODE

#ifndef WIN32_LEAN_AND_MEAN
#   define WIN32_LEAN_AND_MEAN
#   include <windows.h>
#   include <tchar.h>
#   undef WIN32_LEAN_AND_MEAN
#endif

#ifndef _OSL_PROCIMPL_H_
#include "procimpl.h"
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef _OSL_SECURITYIMPL_H_
#include "secimpl.h"
#endif

#ifndef INCLUDED_SAL_INTERNAL_ALLOCATOR_HXX
#include "internal/allocator.hxx"
#endif

#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif

#include <list>
#include <vector>
#include <algorithm>
#include <string>

//#################################################
extern "C" oslFileHandle SAL_CALL osl_createFileHandleFromOSHandle( HANDLE hFile );

//#################################################
const sal_Unicode NAME_VALUE_SEPARATOR = TEXT('=');
const sal_Char* SPACE = " ";
const rtl::OUString ENV_COMSPEC = rtl::OUString::createFromAscii("COMSPEC");
const rtl::OUString QUOTE = rtl::OUString::createFromAscii("\"");

namespace /* private */
{
    //#################################################
    typedef std::list<rtl::OUString, sal::Allocator<rtl::OUString> > string_container_t;
    typedef string_container_t::iterator string_container_iterator_t;
    typedef string_container_t::const_iterator string_container_const_iterator_t;
    typedef std::pair<string_container_iterator_t, string_container_iterator_t> iterator_pair_t;
    typedef std::vector<sal_Unicode, sal::Allocator<sal_Unicode> > environment_container_t;

    //#################################################
    /* Function object that compares two strings that are
       expected to be environment variables in the form
       "name=value". Only the 'name' part will be compared.
       The comparison is in upper case and returns true
       if the first of both strings is less than the
       second one. */
    struct less_environment_variable :
        public std::binary_function<rtl::OUString, rtl::OUString, bool>
    {
        bool operator() (const rtl::OUString& lhs, const rtl::OUString& rhs) const
        {
            OSL_ENSURE((lhs.indexOf(NAME_VALUE_SEPARATOR) > -1) && \
                        (rhs.indexOf(NAME_VALUE_SEPARATOR) > -1), \
                        "Malformed environment variable");

            // Windows compares environment variables uppercase
            // so we do it, too
            return (rtl_ustr_compare_WithLength(
                lhs.toAsciiUpperCase().pData->buffer,
                lhs.indexOf(NAME_VALUE_SEPARATOR),
                rhs.toAsciiUpperCase().pData->buffer,
                rhs.indexOf(NAME_VALUE_SEPARATOR)) < 0);
        }
    };

    //#################################################
    /* Function object used by for_each algorithm to
       calculate the sum of the length of all strings
       in a string container. */
    class sum_of_string_lengths
    {
    public:
        //--------------------------------
        sum_of_string_lengths() : sum_(0) {}

        //--------------------------------
        void operator() (const rtl::OUString& string)
        {
            OSL_ASSERT(string.getLength());

            // always include the terminating '\0'
            if (string.getLength())
                sum_ += string.getLength() + 1;
        }

        //--------------------------------
        operator size_t () const
        {
            return sum_;
        }
    private:
        size_t sum_;
    };

    //#################################################
    inline size_t calc_sum_of_string_lengths(const string_container_t& string_cont)
    {
        return std::for_each(
            string_cont.begin(), string_cont.end(), sum_of_string_lengths());
    }

    //#################################################
    void read_environment(/*out*/ string_container_t* environment)
    {
        // GetEnvironmentStrings returns a sorted list, Windows
        // sorts environment variables upper case
        LPTSTR env = reinterpret_cast<LPTSTR>(GetEnvironmentStrings());
        LPTSTR p   = env;

        while (size_t l = _tcslen(p))
        {
            environment->push_back(p);
            p += l + 1;
        }
        FreeEnvironmentStrings(env);
    }

    //#################################################
    /* the environment list must be sorted, new values
    should either replace existing ones or should be
    added to the list, environment variables will
    be handled case-insensitive */
    bool create_merged_environment(
        rtl_uString* env_vars[],
        sal_uInt32 env_vars_count,
        /*in|out*/ string_container_t* merged_env)
    {
        OSL_ASSERT(env_vars && env_vars_count > 0 && merged_env);

        read_environment(merged_env);

        for (sal_uInt32 i = 0; i < env_vars_count; i++)
        {
            rtl::OUString env_var = rtl::OUString(env_vars[i]);

            if ((env_var.getLength() == 0) ||
                (env_var.indexOf(NAME_VALUE_SEPARATOR) == -1))
                return false;

            iterator_pair_t iter_pair = std::equal_range(
                merged_env->begin(),
                merged_env->end(),
                env_var,
                less_environment_variable());

            if (iter_pair.first != iter_pair.second) // found
                *iter_pair.first = env_var;
            else // not found
                merged_env->insert(iter_pair.first, env_var);
        }
        return true;
    }

    //#################################################
    /* Create a merged environment */
    bool setup_process_environment(
        rtl_uString* environment_vars[],
        sal_uInt32 n_environment_vars,
        /*in|out*/ environment_container_t& environment)
    {
        string_container_t merged_env;
        if (!create_merged_environment(environment_vars, n_environment_vars, &merged_env))
            return false;

        // reserve enough space for the '\0'-separated environment strings and
        // a final '\0'
        environment.reserve(calc_sum_of_string_lengths(merged_env) + 1);

        string_container_const_iterator_t iter = merged_env.begin();
        string_container_const_iterator_t iter_end = merged_env.end();

        sal_uInt32 pos = 0;
        for (/**/; iter != iter_end; ++iter)
        {
            rtl::OUString envv = *iter;

            OSL_ASSERT(envv.getLength());

            sal_uInt32 n = envv.getLength() + 1; // copy the final '\0', too
            rtl_copyMemory(
                reinterpret_cast<void*>(&environment[pos]),
                reinterpret_cast<const void*>(envv.getStr()),
                n * sizeof(sal_Unicode));
            pos += n;
        }
        environment[pos] = 0; // append a final '\0'

        return true;
    }

    //##########################################################
    /*  In contrast to the Win32 API function CreatePipe with
        this function the caller is able to determine separately
        which handle of the pipe is inheritable. */
    bool create_pipe(
        PHANDLE p_read_pipe,
        bool    b_read_pipe_inheritable,
        PHANDLE p_write_pipe,
        bool    b_write_pipe_inheritable,
        LPVOID  p_security_descriptor = NULL,
        DWORD   pipe_size = 0)
    {
        SECURITY_ATTRIBUTES sa;
        sa.nLength              = sizeof(SECURITY_ATTRIBUTES);
        sa.lpSecurityDescriptor = p_security_descriptor;
        sa.bInheritHandle       = b_read_pipe_inheritable || b_write_pipe_inheritable;

        BOOL   bRet  = FALSE;
        HANDLE hTemp = NULL;

        if (!b_read_pipe_inheritable && b_write_pipe_inheritable)
        {
            bRet = CreatePipe(&hTemp, p_write_pipe, &sa, pipe_size);

            if (bRet && !DuplicateHandle(GetCurrentProcess(), hTemp,
                            GetCurrentProcess(), p_read_pipe, 0, FALSE,
                            DUPLICATE_CLOSE_SOURCE | DUPLICATE_SAME_ACCESS))
            {
                CloseHandle(hTemp);
                CloseHandle(*p_read_pipe);
                return false;
            }
        }
        else if (b_read_pipe_inheritable && !b_write_pipe_inheritable)
        {
            bRet = CreatePipe(p_read_pipe, &hTemp, &sa, pipe_size);

            if (bRet && !DuplicateHandle(GetCurrentProcess(), hTemp,
                            GetCurrentProcess(), p_write_pipe, 0, FALSE,
                            DUPLICATE_CLOSE_SOURCE | DUPLICATE_SAME_ACCESS))
            {
                CloseHandle(hTemp);
                CloseHandle(*p_write_pipe);
                return false;
            }
        }
        else
        {
            bRet = CreatePipe(p_read_pipe, p_write_pipe, &sa, pipe_size);
        }
        return bRet;
    }

    //#########################################################
    // Add a quote sign to the start and the end of a string
    // if not already present
    rtl::OUString quote_string(const rtl::OUString& string)
    {
        rtl::OUStringBuffer quoted;
        if (string.indexOf(QUOTE) != 0)
            quoted.append(QUOTE);

        quoted.append(string);

        if (string.lastIndexOf(QUOTE) != (string.getLength() - 1))
            quoted.append(QUOTE);

        return quoted.makeStringAndClear();
    }

    //##########################################################
    // Returns the system path of the executable which can either
    // be provided via the strImageName parameter or as first
    // element of the strArguments list.
    // The returned path will be quoted if it contains spaces.
    rtl::OUString get_executable_path(
        rtl_uString* image_name,
        rtl_uString* cmdline_args[],
        sal_uInt32 n_cmdline_args,
        bool search_path)
    {
        rtl::OUString exe_name;

        if (image_name)
            exe_name = image_name;
        else if (n_cmdline_args)
            exe_name = rtl::OUString(cmdline_args[0]);

        rtl::OUString exe_url = exe_name;
        if (search_path)
            osl_searchFileURL(exe_name.pData, NULL, &exe_url.pData);

        rtl::OUString exe_path;
        if (osl_File_E_None != osl::FileBase::getSystemPathFromFileURL(exe_url, exe_path))
            return rtl::OUString();

        if (exe_path.indexOf(' ') != -1)
            exe_path = quote_string(exe_path);

        return exe_path;
    }

    //##########################################################
    rtl::OUString get_file_extension(const rtl::OUString& file_name)
    {
        sal_Int32 index = file_name.lastIndexOf('.');
        if ((index != -1) && ((index + 1) < file_name.getLength()))
            return file_name.copy(index + 1);

        return rtl::OUString();
    }

    //##########################################################
    bool is_batch_file(const rtl::OUString& file_name)
    {
        rtl::OUString ext = get_file_extension(file_name);
        return (ext.equalsIgnoreAsciiCaseAscii("bat") ||
                ext.equalsIgnoreAsciiCaseAscii("cmd") ||
                ext.equalsIgnoreAsciiCaseAscii("btm"));
    }

    //##########################################################
    rtl::OUString get_batch_processor()
    {
        rtl::OUString comspec;
        osl_getEnvironment(ENV_COMSPEC.pData, &comspec.pData);

        OSL_ASSERT(comspec.getLength());

        /* check if comspec path contains blanks and quote it if any */
        if (comspec.indexOf(' ') != -1)
            comspec = quote_string(comspec);

        return comspec;
    }

} // namespace private


//#################################################
oslProcessError SAL_CALL osl_executeProcess(
    rtl_uString *strImageName,
    rtl_uString *strArguments[],
    sal_uInt32   nArguments,
    oslProcessOption Options,
    oslSecurity Security,
    rtl_uString *strDirectory,
    rtl_uString *strEnvironmentVars[],
    sal_uInt32   nEnvironmentVars,
    oslProcess *pProcess
)
{
    return osl_executeProcess_WithRedirectedIO(
        strImageName,
        strArguments,
        nArguments,
        Options,
        Security,
        strDirectory,
        strEnvironmentVars,
        nEnvironmentVars,
        pProcess,
        NULL, NULL, NULL );
}

//#################################################
oslProcessError SAL_CALL osl_executeProcess_WithRedirectedIO(
    rtl_uString *ustrImageName,
    rtl_uString *ustrArguments[],
    sal_uInt32   nArguments,
    oslProcessOption Options,
    oslSecurity Security,
    rtl_uString *ustrDirectory,
    rtl_uString *ustrEnvironmentVars[],
    sal_uInt32 nEnvironmentVars,
    oslProcess *pProcess,
    oslFileHandle *pProcessInputWrite,
    oslFileHandle *pProcessOutputRead,
    oslFileHandle *pProcessErrorRead)
{
    rtl::OUString exe_path = get_executable_path(
        ustrImageName, ustrArguments, nArguments, (Options & osl_Process_SEARCHPATH));

    if (0 == exe_path.getLength())
        return osl_Process_E_NotFound;

    if (pProcess == NULL)
        return osl_Process_E_InvalidError;

    DWORD flags = NORMAL_PRIORITY_CLASS;
    rtl::OUStringBuffer command_line;

    if (is_batch_file(exe_path))
    {
        rtl::OUString batch_processor = get_batch_processor();

        if (batch_processor.getLength())
        {
            /* cmd.exe does not work without a console window */
            if (!(Options & osl_Process_WAIT) || (Options & osl_Process_DETACHED))
                flags |= CREATE_NEW_CONSOLE;

            command_line.append(batch_processor);
            command_line.appendAscii(" /c ");
        }
        else
            // should we return here in case of error?
            return osl_Process_E_Unknown;
    }

    command_line.append(exe_path);

    /* Add remaining arguments to command line. If ustrImageName is NULL
       the first parameter is the name of the executable so we have to
       start at 1 instead of 0 */
    for (sal_uInt32 n = (NULL != ustrImageName) ? 0 : 1; n < nArguments; n++)
    {
        command_line.appendAscii(SPACE);

        /* Quote arguments containing blanks */
        if (rtl::OUString(ustrArguments[n]).indexOf(' ') != -1)
            command_line.append(quote_string(ustrArguments[n]));
        else
            command_line.append(ustrArguments[n]);
    }

    environment_container_t environment;
    LPVOID p_environment = NULL;

    if (nEnvironmentVars && ustrEnvironmentVars)
    {
        if (!setup_process_environment(
                ustrEnvironmentVars, nEnvironmentVars, environment))
            return osl_Process_E_InvalidError;

        flags |= CREATE_UNICODE_ENVIRONMENT;
        p_environment = &environment[0];
    }

    rtl::OUString cwd;
    if (ustrDirectory && ustrDirectory->length && (osl_File_E_None != osl::FileBase::getSystemPathFromFileURL(ustrDirectory, cwd)))
           return osl_Process_E_InvalidError;

    LPCWSTR p_cwd = (cwd.getLength()) ? cwd.getStr() : NULL;

    if ((Options & osl_Process_DETACHED) && !(flags & CREATE_NEW_CONSOLE))
        flags |= DETACHED_PROCESS;

    STARTUPINFO startup_info;
    memset(&startup_info, 0, sizeof(STARTUPINFO));

    startup_info.cb        = sizeof(STARTUPINFO);
    startup_info.dwFlags   = STARTF_USESHOWWINDOW;
    startup_info.lpDesktop = L"";

    /* Create pipes for redirected IO */
    HANDLE hInputRead  = NULL;
    HANDLE hInputWrite = NULL;
    if (pProcessInputWrite && create_pipe(&hInputRead, true, &hInputWrite, false))
        startup_info.hStdInput = hInputRead;

    HANDLE hOutputRead  = NULL;
    HANDLE hOutputWrite = NULL;
    if (pProcessOutputRead && create_pipe(&hOutputRead, false, &hOutputWrite, true))
        startup_info.hStdOutput = hOutputWrite;

    HANDLE hErrorRead  = NULL;
    HANDLE hErrorWrite = NULL;
    if (pProcessErrorRead && create_pipe(&hErrorRead, false, &hErrorWrite, true))
        startup_info.hStdError = hErrorWrite;

    bool b_inherit_handles = false;
    if (pProcessInputWrite || pProcessOutputRead || pProcessErrorRead)
    {
        startup_info.dwFlags |= STARTF_USESTDHANDLES;
        b_inherit_handles      = true;
    }

    switch(Options & (osl_Process_NORMAL | osl_Process_HIDDEN | osl_Process_MINIMIZED | osl_Process_MAXIMIZED | osl_Process_FULLSCREEN))
    {
        case osl_Process_HIDDEN:
            startup_info.wShowWindow = SW_HIDE;
            break;

        case osl_Process_MINIMIZED:
            startup_info.wShowWindow = SW_MINIMIZE;
            break;

        case osl_Process_MAXIMIZED:
        case osl_Process_FULLSCREEN:
            startup_info.wShowWindow = SW_MAXIMIZE;
            break;

        default:
            startup_info.wShowWindow = SW_NORMAL;
    }

    rtl::OUString cmdline = command_line.makeStringAndClear();
    PROCESS_INFORMATION process_info;
    BOOL bRet = FALSE;

    if ((Security != NULL) && (((oslSecurityImpl*)Security)->m_hToken != NULL))
    {
        bRet = CreateProcessAsUser(
            ((oslSecurityImpl*)Security)->m_hToken,
            NULL, const_cast<sal_Unicode*>(cmdline.getStr()), NULL,  NULL,
            b_inherit_handles, flags, p_environment, p_cwd,
            &startup_info, &process_info);
    }
    else
    {
        bRet = CreateProcess(
            NULL, const_cast<sal_Unicode*>(cmdline.getStr()), NULL,  NULL,
            b_inherit_handles, flags, p_environment, p_cwd,
            &startup_info, &process_info);
    }

    /* Now we can close the pipe ends that are used by the child process */

    if (hInputRead)
        CloseHandle(hInputRead);

    if (hOutputWrite)
        CloseHandle(hOutputWrite);

    if (hErrorWrite)
        CloseHandle(hErrorWrite);

    if (bRet)
    {
        CloseHandle(process_info.hThread);

        oslProcessImpl* pProcImpl = reinterpret_cast<oslProcessImpl*>(
            rtl_allocateMemory(sizeof(oslProcessImpl)));

        if (pProcImpl != NULL)
        {
            pProcImpl->m_hProcess  = process_info.hProcess;
            pProcImpl->m_IdProcess = process_info.dwProcessId;

            *pProcess = (oslProcess)pProcImpl;

            if (Options & osl_Process_WAIT)
                WaitForSingleObject(pProcImpl->m_hProcess, INFINITE);

            if (pProcessInputWrite)
                *pProcessInputWrite = osl_createFileHandleFromOSHandle(hInputWrite);

            if (pProcessOutputRead)
                *pProcessOutputRead = osl_createFileHandleFromOSHandle(hOutputRead);

            if (pProcessErrorRead)
                *pProcessErrorRead = osl_createFileHandleFromOSHandle(hErrorRead);

            return osl_Process_E_None;
        }
    }

    /* if an error occured we have to close the server side pipe ends too */

    if (hInputWrite)
        CloseHandle(hInputWrite);

    if (hOutputRead)
        CloseHandle(hOutputRead);

    if (hErrorRead)
        CloseHandle(hErrorRead);

    return osl_Process_E_Unknown;
}
