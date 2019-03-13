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

#ifndef WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN
# include <windows.h>
# undef WIN32_LEAN_AND_MEAN
#endif

#include "file-impl.hxx"
#include "procimpl.hxx"
#include <rtl/alloc.h>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include "secimpl.hxx"
#include <osl/file.hxx>
#include <o3tl/char16_t2wchar_t.hxx>

#include <vector>
#include <algorithm>

namespace /* private */
{
    /* Function object that compares two strings that are
       expected to be environment variables in the form
       "name=value". Only the 'name' part will be compared.
       The comparison is in upper case and returns true
       if the first of both strings is less than the
       second one. */
    struct less_environment_variable
    {
        bool operator() (const OUString& lhs, const OUString& rhs) const
        {
            OSL_ENSURE((lhs.indexOf(L'=') > -1) &&
                        (rhs.indexOf(L'=') > -1),
                        "Malformed environment variable");

            // Windows compares environment variables uppercase
            // so we do it, too
            return (rtl_ustr_compare_WithLength(
                lhs.toAsciiUpperCase().pData->buffer,
                lhs.indexOf(L'='),
                rhs.toAsciiUpperCase().pData->buffer,
                rhs.indexOf(L'=')) < 0);
        }
    };

    /* Function object used by for_each algorithm to
       calculate the sum of the length of all strings
       in a string container. */
    class sum_of_string_lengths
    {
    public:

        sum_of_string_lengths() : sum_(0) {}

        void operator() (const OUString& string)
        {
            OSL_ASSERT(string.getLength());

            // always include the terminating '\0'
            if (string.getLength())
                sum_ += string.getLength() + 1;
        }

        operator size_t () const
        {
            return sum_;
        }
    private:
        size_t sum_;
    };

    size_t calc_sum_of_string_lengths(const std::vector<OUString>& string_cont)
    {
        return std::for_each(
            string_cont.begin(), string_cont.end(), sum_of_string_lengths());
    }

    void read_environment(/*out*/ std::vector<OUString>* environment)
    {
        // GetEnvironmentStrings returns a sorted list, Windows
        // sorts environment variables upper case
        LPWSTR env = GetEnvironmentStringsW();
        LPWSTR p   = env;

        while (size_t l = wcslen(p))
        {
            environment->push_back(o3tl::toU(p));
            p += l + 1;
        }
        FreeEnvironmentStringsW(env);

        // it is apparently possible that the environment is not completely
        // sorted; Cygwin may append entries, which breaks the equal_range
        std::stable_sort(environment->begin(), environment->end(),
            less_environment_variable());
    }

    /* the environment list must be sorted, new values
    should either replace existing ones or should be
    added to the list, environment variables will
    be handled case-insensitive */
    bool create_merged_environment(
        rtl_uString* env_vars[],
        sal_uInt32 env_vars_count,
        /*in|out*/ std::vector<OUString>* merged_env)
    {
        OSL_ASSERT(env_vars && env_vars_count > 0 && merged_env);

        read_environment(merged_env);

        for (sal_uInt32 i = 0; i < env_vars_count; i++)
        {
            OUString env_var = OUString(env_vars[i]);

            if (env_var.getLength() == 0)
                return false;

            auto iter_pair = std::equal_range(
                merged_env->begin(),
                merged_env->end(),
                env_var,
                less_environment_variable());

            if (env_var.indexOf(L'=') == -1)
            {
                merged_env->erase(iter_pair.first, iter_pair.second);
            }
            else
            {
                if (iter_pair.first != iter_pair.second) // found
                    *iter_pair.first = env_var;
                else // not found
                    merged_env->insert(iter_pair.first, env_var);
            }
        }
        return true;
    }

    /* Create a merged environment */
    bool setup_process_environment(
        rtl_uString* environment_vars[],
        sal_uInt32 n_environment_vars,
        /*in|out*/ std::vector<sal_Unicode>& environment)
    {
        std::vector<OUString> merged_env;
        if (!create_merged_environment(environment_vars, n_environment_vars, &merged_env))
            return false;

        // allocate enough space for the '\0'-separated environment strings and
        // a final '\0'
        environment.resize(calc_sum_of_string_lengths(merged_env) + 1);

        sal_uInt32 pos = 0;
        for (auto& envv : merged_env)
        {
            OSL_ASSERT(envv.getLength());

            sal_uInt32 n = envv.getLength() + 1; // copy the final '\0', too
            memcpy(&environment[pos], envv.getStr(), n * sizeof(sal_Unicode));
            pos += n;
        }
        environment[pos] = 0; // append a final '\0'

        return true;
    }

    /*  In contrast to the Win32 API function CreatePipe with
        this function the caller is able to determine separately
        which handle of the pipe is inheritable. */
    bool create_pipe(
        PHANDLE p_read_pipe,
        bool    b_read_pipe_inheritable,
        PHANDLE p_write_pipe,
        bool    b_write_pipe_inheritable,
        LPVOID  p_security_descriptor = nullptr,
        DWORD   pipe_size = 0)
    {
        SECURITY_ATTRIBUTES sa;
        sa.nLength              = sizeof(SECURITY_ATTRIBUTES);
        sa.lpSecurityDescriptor = p_security_descriptor;
        sa.bInheritHandle       = b_read_pipe_inheritable || b_write_pipe_inheritable;

        BOOL   bRet  = FALSE;
        HANDLE hTemp = nullptr;

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

    // Add a quote sign to the start and the end of a string
    // if not already present
    OUString quote_string(const OUString& string)
    {
        OUStringBuffer quoted;
        if (string.indexOf(L'"') != 0)
            quoted.append('"');

        quoted.append(string);

        if (string.lastIndexOf(L'"') != (string.getLength() - 1))
            quoted.append('"');

        return quoted.makeStringAndClear();
    }

    // The parameter path must be a system path. If it is longer than 260 characters
    // then it is shortened using the GetShortPathName function. This function only
    // works if the path exists. Because "path" can be the path to an executable, it
    // may not have the file extension ".exe". However, if the file on disk has the
    // ".exe" extension, then the function will fail. In this case a second attempt
    // is started by adding the parameter "extension" to "path".
    OUString getShortPath(OUString const & path, OUString const & extension)
    {
        OUString ret(path);
        if (path.getLength() > 260)
        {
            std::vector<sal_Unicode> vec(path.getLength() + 1);
            //GetShortPathNameW only works if the file can be found!
            const DWORD len = GetShortPathNameW(
                o3tl::toW(path.getStr()), o3tl::toW(&vec[0]), path.getLength() + 1);

            if (!len && GetLastError() == ERROR_FILE_NOT_FOUND
                && extension.getLength())
            {
                const OUString extPath(path + extension);
                std::vector<sal_Unicode> vec2(
                    extPath.getLength() + 1);
                const DWORD len2 = GetShortPathNameW(
                    o3tl::toW(extPath.getStr()), o3tl::toW(&vec2[0]), extPath.getLength() + 1);
                ret = OUString(&vec2[0], len2);
            }
            else
            {
                ret = OUString(&vec[0], len);
            }
        }
        return ret;
    }

    // Returns the system path of the executable which can either
    // be provided via the strImageName parameter or as first
    // element of the strArguments list.
    // The returned path will be quoted if it contains spaces.
    OUString get_executable_path(
        rtl_uString* image_name,
        rtl_uString* cmdline_args[],
        sal_uInt32 n_cmdline_args,
        bool search_path)
    {
        OUString exe_name;

        if (image_name)
            exe_name = image_name;
        else if (n_cmdline_args)
            exe_name = OUString(cmdline_args[0]);

        OUString exe_url = exe_name;
        if (search_path)
            osl_searchFileURL(exe_name.pData, nullptr, &exe_url.pData);

        OUString exe_path;
        if (osl::FileBase::E_None != osl::FileBase::getSystemPathFromFileURL(exe_url, exe_path))
            return OUString();

        exe_path = getShortPath(exe_path, ".exe");

        if (exe_path.indexOf(' ') != -1)
            exe_path = quote_string(exe_path);

        return exe_path;
    }

    OUString get_file_extension(const OUString& file_name)
    {
        // Quoted file name
        if ((file_name.indexOf(L'"') == 0) && (file_name.lastIndexOf(L'"') == (file_name.getLength() - 1)))
        {
            sal_Int32 index = file_name.lastIndexOf('.');
            if ((index != -1) && ((index + 2) < file_name.getLength()))
                return file_name.copy(index + 1, file_name.getLength() - (index + 2));
        }
        // Unquoted file name
        else
        {
            sal_Int32 index = file_name.lastIndexOf('.');
            if ((index != -1) && ((index + 1) < file_name.getLength()))
                return file_name.copy(index + 1);
        }
        return OUString();
    }

    bool is_batch_file(const OUString& file_name)
    {
        OUString ext = get_file_extension(file_name);
        return (ext.equalsIgnoreAsciiCase("bat") ||
                ext.equalsIgnoreAsciiCase("cmd") ||
                ext.equalsIgnoreAsciiCase("btm"));
    }

    const OUString ENV_COMSPEC ("COMSPEC");
    OUString get_batch_processor()
    {
        OUString comspec;
        osl_getEnvironment(ENV_COMSPEC.pData, &comspec.pData);

        OSL_ASSERT(comspec.getLength());

        /* check if comspec path contains blanks and quote it if any */
        if (comspec.indexOf(' ') != -1)
            comspec = quote_string(comspec);

        return comspec;
    }

} // namespace private

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
        nullptr, nullptr, nullptr );
}

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
    OUString exe_path = get_executable_path(
        ustrImageName, ustrArguments, nArguments, (Options & osl_Process_SEARCHPATH) != 0);

    if (0 == exe_path.getLength())
        return osl_Process_E_NotFound;

    if (pProcess == nullptr)
        return osl_Process_E_InvalidError;

    DWORD flags = NORMAL_PRIORITY_CLASS;
    OUStringBuffer command_line;

    if (is_batch_file(exe_path))
    {
        OUString batch_processor = get_batch_processor();

        if (batch_processor.getLength())
        {
            /* cmd.exe does not work without a console window */
            if (!(Options & osl_Process_WAIT) || (Options & osl_Process_DETACHED))
                flags |= CREATE_NEW_CONSOLE;

            command_line.append(batch_processor);
            command_line.append(" /c ");
        }
        else
            // should we return here in case of error?
            return osl_Process_E_Unknown;
    }

    command_line.append(exe_path);

    /* Add remaining arguments to command line. If ustrImageName is nullptr
       the first parameter is the name of the executable so we have to
       start at 1 instead of 0 */
    for (sal_uInt32 n = (nullptr != ustrImageName) ? 0 : 1; n < nArguments; n++)
    {
        command_line.append(" ");

        /* Quote arguments containing blanks */
        if (OUString(ustrArguments[n]).indexOf(' ') != -1)
            command_line.append(quote_string(ustrArguments[n]));
        else
            command_line.append(ustrArguments[n]);
    }

    std::vector<sal_Unicode> environment;
    LPVOID p_environment = nullptr;

    if (nEnvironmentVars && ustrEnvironmentVars)
    {
        if (!setup_process_environment(
                ustrEnvironmentVars, nEnvironmentVars, environment))
            return osl_Process_E_InvalidError;

        flags |= CREATE_UNICODE_ENVIRONMENT;
        p_environment = &environment[0];
    }

    OUString cwd;
    if (ustrDirectory && ustrDirectory->length && (osl::FileBase::E_None != osl::FileBase::getSystemPathFromFileURL(ustrDirectory, cwd)))
           return osl_Process_E_InvalidError;

    LPCWSTR p_cwd = (cwd.getLength()) ? o3tl::toW(cwd.getStr()) : nullptr;

    if ((Options & osl_Process_DETACHED) && !(flags & CREATE_NEW_CONSOLE))
        flags |= DETACHED_PROCESS;

    STARTUPINFOW startup_info;
    memset(&startup_info, 0, sizeof(startup_info));

    startup_info.cb        = sizeof(startup_info);
    startup_info.dwFlags   = STARTF_USESHOWWINDOW;
    startup_info.lpDesktop = const_cast<LPWSTR>(L"");

    /* Create pipes for redirected IO */
    HANDLE hInputRead  = nullptr;
    HANDLE hInputWrite = nullptr;
    if (pProcessInputWrite && create_pipe(&hInputRead, true, &hInputWrite, false))
        startup_info.hStdInput = hInputRead;

    HANDLE hOutputRead  = nullptr;
    HANDLE hOutputWrite = nullptr;
    if (pProcessOutputRead && create_pipe(&hOutputRead, false, &hOutputWrite, true))
        startup_info.hStdOutput = hOutputWrite;

    HANDLE hErrorRead  = nullptr;
    HANDLE hErrorWrite = nullptr;
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
            flags |= CREATE_NO_WINDOW; // ignored for non-console
                                       // applications; ignored on
                                       // Win9x
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

    OUString cmdline = command_line.makeStringAndClear();
    PROCESS_INFORMATION process_info;
    BOOL bRet = FALSE;

    if ((Security != nullptr) && (static_cast<oslSecurityImpl*>(Security)->m_hToken != nullptr))
    {
        bRet = CreateProcessAsUserW(
            static_cast<oslSecurityImpl*>(Security)->m_hToken,
            nullptr, const_cast<LPWSTR>(o3tl::toW(cmdline.getStr())), nullptr,  nullptr,
            b_inherit_handles, flags, p_environment, p_cwd,
            &startup_info, &process_info);
    }
    else
    {
        bRet = CreateProcessW(
            nullptr, const_cast<LPWSTR>(o3tl::toW(cmdline.getStr())), nullptr,  nullptr,
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

        oslProcessImpl* pProcImpl = static_cast<oslProcessImpl*>(
            malloc(sizeof(oslProcessImpl)));

        if (pProcImpl != nullptr)
        {
            pProcImpl->m_hProcess  = process_info.hProcess;
            pProcImpl->m_IdProcess = process_info.dwProcessId;

            *pProcess = static_cast<oslProcess>(pProcImpl);

            if (Options & osl_Process_WAIT)
                WaitForSingleObject(pProcImpl->m_hProcess, INFINITE);

            if (pProcessInputWrite)
                *pProcessInputWrite = osl_createFileHandleFromOSHandle(hInputWrite, osl_File_OpenFlag_Write);

            if (pProcessOutputRead)
                *pProcessOutputRead = osl_createFileHandleFromOSHandle(hOutputRead, osl_File_OpenFlag_Read);

            if (pProcessErrorRead)
                *pProcessErrorRead = osl_createFileHandleFromOSHandle(hErrorRead, osl_File_OpenFlag_Read);

            return osl_Process_E_None;
        }
    }

    /* if an error occurred we have to close the server side pipe ends too */

    if (hInputWrite)
        CloseHandle(hInputWrite);

    if (hOutputRead)
        CloseHandle(hOutputRead);

    if (hErrorRead)
        CloseHandle(hErrorRead);

    return osl_Process_E_Unknown;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
