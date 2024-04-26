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
#include <mutex>

#include <o3tl/runtimetooustring.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <sal/log.hxx>
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
#include <vector>
#include <osl/process.h>
#include <rtl/strbuf.hxx>
#include <o3tl/lru_map.hxx>
#include "file_url.hxx"

namespace
{
struct FrameData
{
    const char* file = nullptr;
    void* addr;
    ptrdiff_t offset;
    OString info;
    bool handled = false;
};

typedef o3tl::lru_map<void*, OString> FrameCache;
std::mutex frameCacheMutex;
FrameCache frameCache( 256 );

void process_file_addr2line( const char* file, std::vector<FrameData>& frameData )
{
    if(access( file, R_OK ) != 0)
        return; // cannot read info from the binary file anyway
    OUString binary(u"addr2line"_ustr);
    OUString dummy;
#if defined __clang__
    // llvm-addr2line is faster than addr2line
    if(osl::detail::find_in_PATH(u"llvm-addr2line"_ustr, dummy))
        binary = "llvm-addr2line";
#endif
    if(!osl::detail::find_in_PATH(binary, dummy))
        return; // Will not work, avoid warnings from osl process code.
    OUString arg1(u"-Cfe"_ustr);
    OUString arg2 = OUString::fromUtf8(file);
    std::vector<OUString> addrs;
    std::vector<rtl_uString*> args;
    args.reserve(frameData.size() + 2);
    args.push_back( arg1.pData );
    args.push_back( arg2.pData );
    for( FrameData& frame : frameData )
    {
        if( frame.file != nullptr && strcmp( file, frame.file ) == 0 )
        {
            addrs.push_back("0x" + OUString::number(frame.offset, 16));
            args.push_back(addrs.back().pData);
            frame.handled = true;
        }
    }

    oslProcess aProcess;
    oslFileHandle pOut = nullptr;
    oslFileHandle pErr = nullptr;
    oslSecurity pSecurity = osl_getCurrentSecurity();
    oslProcessError eErr = osl_executeProcess_WithRedirectedIO(
        binary.pData, args.data(), args.size(), osl_Process_SEARCHPATH | osl_Process_HIDDEN, pSecurity, nullptr,
        nullptr, 0, &aProcess, nullptr, &pOut, &pErr);
    osl_freeSecurityHandle(pSecurity);

    if (eErr != osl_Process_E_None)
    {
        SAL_WARN("sal.osl", binary << " call to resolve " << file << " symbols failed");
        return;
    }

    OStringBuffer outputBuffer;
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
            outputBuffer.append(buffer, bytesRead);
            if (err != osl_File_E_None && err != osl_File_E_AGAIN)
                break;
        }
        osl_closeFile(pOut);
    }
    if(pErr)
        osl_closeFile(pErr);
    eErr = osl_joinProcess(aProcess);
    osl_freeProcessHandle(aProcess);

    OString output = outputBuffer.makeStringAndClear();
    std::vector<OString> lines;
    sal_Int32 outputPos = 0;
    while(outputPos < output.getLength())
    {
        sal_Int32 end1 = output.indexOf('\n', outputPos);
        if(end1 < 0)
            break;
        sal_Int32 end2 = output.indexOf('\n', end1 + 1);
        if(end2 < 0)
            end2 = output.getLength();
        lines.push_back(output.copy( outputPos, end1 - outputPos ));
        lines.push_back(output.copy( end1 + 1, end2 - end1 - 1 ));
        outputPos = end2 + 1;
    }
    if(lines.size() != addrs.size() * 2)
    {
        SAL_WARN("sal.osl", "failed to parse " << binary << " call output to resolve " << file << " symbols ");
        return; // addr2line problem?
    }
    size_t linesPos = 0;
    for( FrameData& frame : frameData )
    {
        if( frame.file != nullptr && strcmp( file, frame.file ) == 0 )
        {
            // There should be two lines, first function name and second source file information.
            // If each of them starts with ??, it is invalid/unknown.
            OString function = lines[linesPos];
            OString source = lines[linesPos+1];
            linesPos += 2;
            if(function.isEmpty() || function.startsWith("??"))
            {
                // Cache that the address cannot be resolved.
                std::lock_guard guard(frameCacheMutex);
                frameCache.insert( { frame.addr, "" } );
            }
            else
            {
                if( source.startsWith("??"))
                    frame.info = function + " in " + file;
                else
                    frame.info = function + " at " + source;
                std::lock_guard guard(frameCacheMutex);
                frameCache.insert( { frame.addr, frame.info } );
            }
        }
    }
}

} // namespace

OUString sal::backtrace_to_string(BacktraceState* backtraceState)
{
    // Collect frames for each binary and process each binary in one addr2line
    // call for better performance.
    std::vector< FrameData > frameData;
    frameData.resize(backtraceState->nDepth);
    for (int i = 0; i != backtraceState->nDepth; ++i)
    {
        Dl_info dli;
        void* addr = backtraceState->buffer[i];
        std::unique_lock guard(frameCacheMutex);
        auto it = frameCache.find(addr);
        bool found = it != frameCache.end();
        guard.unlock();
        if( found )
        {
            frameData[ i ].info = it->second;
            frameData[ i ].handled = true;
        }
        else if (dladdr(addr, &dli) != 0)
        {
            if (dli.dli_fname && dli.dli_fbase)
            {
                frameData[ i ].file = dli.dli_fname;
                frameData[ i ].addr = addr;
                frameData[ i ].offset = reinterpret_cast<ptrdiff_t>(addr) - reinterpret_cast<ptrdiff_t>(dli.dli_fbase);
            }
        }
    }
    for (int i = 0; i != backtraceState->nDepth; ++i)
    {
        if(frameData[ i ].file != nullptr && !frameData[ i ].handled)
            process_file_addr2line( frameData[ i ].file, frameData );
    }
    OUStringBuffer b3;
    std::unique_ptr<char*, decltype(free)*> b2{ nullptr, free };
    bool fallbackInitDone = false;
    for (int i = 0; i != backtraceState->nDepth; ++i)
    {
        if (i != 0)
            b3.append("\n");
        b3.append( "#" + OUString::number( i ) + " " );
        if(!frameData[i].info.isEmpty())
            b3.append(o3tl::runtimeToOUString(frameData[i].info.getStr()));
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
    if (!b2) {
        return OUString();
    }
    OUStringBuffer b3;
    for (int i = 0; i != backtraceState->nDepth; ++i) {
        if (i != 0) {
            b3.append("\n");
        }
        b3.append( "#" + OUString::number( i ) + " " );
        b3.append(o3tl::runtimeToOUString(b2.get()[i]));
    }
    return b3.makeStringAndClear();
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
