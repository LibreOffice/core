/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <cassert>
#include <cstdlib>
#include <limits>
#include <memory>

#include <o3tl/runtimetooustring.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <sal/backtrace.hxx>

#include "backtrace.h"
#include <backtraceasstring.hxx>

OUString osl::detail::backtraceAsString(sal_uInt32 maxDepth) {
    std::unique_ptr<sal::BacktraceState> backtrace = sal::backtrace_get( maxDepth );
    return sal::backtrace_to_string( backtrace.get());
}

std::unique_ptr<sal::BacktraceState> sal::backtrace_get(sal_uInt32 maxDepth)
{
    assert(maxDepth != 0);
    auto const maxInt = static_cast<unsigned int>(
        std::numeric_limits<int>::max());
    if (maxDepth > maxInt) {
        maxDepth = static_cast<sal_uInt32>(maxInt);
    }
    auto b1 = new void *[maxDepth];
    int n = backtrace(b1, static_cast<int>(maxDepth));
    return std::unique_ptr<BacktraceState>(new BacktraceState{ b1, n });
}

#if OSL_DEBUG_LEVEL > 0 && (defined LINUX || defined MACOSX || defined FREEBSD || defined NETBSD || defined OPENBSD || defined(DRAGONFLY))
// The backtrace_symbols() function is unreliable, it requires -rdynamic and even then it cannot resolve names
// of many functions, such as those with hidden ELF visibility. Libunwind doesn't resolve names for me either,
// boost::stacktrace doesn't work properly, the best result I've found is addr2line. Using addr2line is relatively
// slow, but I don't find that to be a big problem for printing of backtraces. Feel free to improve if needed
// (e.g. the calls could be grouped by the binary).
#include <dlfcn.h>
#include <unistd.h>
#include <osl/process.h>
#include <rtl/strbuf.hxx>
#include "file_url.hxx"

static OString symbol_addr2line_info(const char* file, ptrdiff_t offset)
{
    OUString binary("addr2line");
    OUString dummy;
    if(!osl::detail::find_in_PATH(binary, dummy) || access( file, R_OK ) != 0)
        return OString(); // Will not work, avoid warnings from osl process code.
    OUString arg1("-Cfe");
    OUString arg2 = OUString::fromUtf8(file);
    OUString arg3 = "0x" + OUString::number(offset, 16);
    rtl_uString* args[] = { arg1.pData, arg2.pData, arg3.pData };
    sal_Int32 nArgs = 3;

    oslProcess aProcess;
    oslFileHandle pOut = nullptr;
    oslFileHandle pErr = nullptr;
    oslSecurity pSecurity = osl_getCurrentSecurity();
    oslProcessError eErr = osl_executeProcess_WithRedirectedIO(
        binary.pData, args, nArgs, osl_Process_SEARCHPATH | osl_Process_HIDDEN, pSecurity, nullptr,
        nullptr, 0, &aProcess, nullptr, &pOut, &pErr);
    osl_freeSecurityHandle(pSecurity);

    if (eErr != osl_Process_E_None)
        return OString();

    OStringBuffer result;
    if (pOut)
    {
        const sal_uInt64 BUF_SIZE = 1024;
        char buffer[BUF_SIZE];
        while (true)
        {
            sal_uInt64 bytesRead = 0;
            while(osl_readFile(pErr, buffer, BUF_SIZE, &bytesRead) == osl_File_E_None
                && bytesRead != 0)
                ; // discard possible stderr output
            oslFileError err = osl_readFile(pOut, buffer, BUF_SIZE, &bytesRead);
            if(bytesRead == 0 && err == osl_File_E_None)
                break;
            result.append(buffer, bytesRead);
            if (err != osl_File_E_None && err != osl_File_E_AGAIN)
                break;
        }
        osl_closeFile(pOut);
    }
    if(pErr)
        osl_closeFile(pErr);
    eErr = osl_joinProcess(aProcess);
    osl_freeProcessHandle(aProcess);
    return result.makeStringAndClear();
}

static OString symbol_addr2line(void* addr)
{
    Dl_info dli;
    ptrdiff_t offset;
    if (dladdr(addr, &dli) != 0)
    {
        if (dli.dli_fname && dli.dli_fbase)
        {
            offset = reinterpret_cast<ptrdiff_t>(addr) - reinterpret_cast<ptrdiff_t>(dli.dli_fbase);
            OString info = symbol_addr2line_info(dli.dli_fname, offset);
            // There should be two lines, first function name and second source file information.
            // If each of them starts with ??, it is invalid/unknown.
            if(!info.isEmpty() && !info.startsWith("??"))
            {
                sal_Int32 end1 = info.indexOf('\n');
                if(end1 > 0)
                {
                    OString function = info.copy( 0, end1 );
                    sal_Int32 end2 = info.indexOf('\n', end1 + 1);
                    OString source = info.copy( end1 + 1, end2 > 0 ? end2 - end1 - 1 : info.getLength() - end1 );
                    if( source.startsWith("??"))
                        return function + " in " + dli.dli_fname;
                    else
                        return function + " at " + source;
                }
            }
        }
    }
    return OString();
}

OUString sal::backtrace_to_string(BacktraceState* backtraceState)
{
    OUStringBuffer b3;
    std::unique_ptr<char*, decltype(free)*> b2{ nullptr, free };
    bool fallbackInitDone = false;
    for (int i = 0; i != backtraceState->nDepth; ++i)
    {
        if (i != 0)
            b3.append("\n");
        OString info = symbol_addr2line(backtraceState->buffer[i]);
        if(!info.isEmpty())
            b3.append(o3tl::runtimeToOUString(info.getStr()));
        else
        {
            if(!fallbackInitDone)
            {
                b2 = std::unique_ptr<char*, decltype(free)*>
                    {backtrace_symbols(backtraceState->buffer, backtraceState->nDepth), free};
                fallbackInitDone = true;
            }
            if(b2)
                b3.append(o3tl::runtimeToOUString(b2.get()[i]));
            else
                b3.append("??");
        }
    }
    return b3.makeStringAndClear();
}

#else

OUString sal::backtrace_to_string(BacktraceState* backtraceState)
{
    std::unique_ptr<char*, decltype(free)*> b2{backtrace_symbols(backtraceState->buffer, backtraceState->nDepth), free};
    if (b2.get() == nullptr) {
        return OUString();
    }
    OUStringBuffer b3;
    for (int i = 0; i != backtraceState->nDepth; ++i) {
        if (i != 0) {
            b3.append("\n");
        }
        b3.append(o3tl::runtimeToOUString(b2.get()[i]));
    }
    return b3.makeStringAndClear();
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
