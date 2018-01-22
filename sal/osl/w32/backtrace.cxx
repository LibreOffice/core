/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <limits>
#include <memory>

#if !defined WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <process.h>
#include <iostream>
#define OPTIONAL
#include <DbgHelp.h>

#include <rtl/ustrbuf.hxx>
#include <sal/backtrace.hxx>

#include <backtraceasstring.hxx>

namespace {

template<typename T> T clampToULONG(T n) {
    auto const maxUlong = std::numeric_limits<ULONG>::max();
    return n > maxUlong ? static_cast<T>(maxUlong) : n;
}

}

OUString osl::detail::backtraceAsString(sal_uInt32 maxDepth)
{
    assert(maxDepth != 0);
    maxDepth = clampToULONG(maxDepth);

    OUStringBuffer aBuf;

    HANDLE hProcess = GetCurrentProcess();
    SymInitialize( hProcess, nullptr, true );

    std::unique_ptr<void*[]> aStack(new void*[ maxDepth ]);
    // https://msdn.microsoft.com/en-us/library/windows/desktop/bb204633.aspx
    // "CaptureStackBackTrace function" claims that you "can capture up to
    // MAXUSHORT frames", and on Windows Server 2003 and Windows XP it even
    // "must be less than 63", but assume that a too large input value is
    // clamped internally, instead of resulting in an error:
    sal_uInt32 nFrames = CaptureStackBackTrace( 0, static_cast<ULONG>(maxDepth), aStack.get(), nullptr );

    SYMBOL_INFO  * pSymbol;
    pSymbol = static_cast<SYMBOL_INFO *>(calloc( sizeof( SYMBOL_INFO ) + 1024 * sizeof( char ), 1 ));
    pSymbol->MaxNameLen = 1024 - 1;
    pSymbol->SizeOfStruct = sizeof( SYMBOL_INFO );

    for( sal_uInt32 i = 0; i < nFrames; i++ )
    {
        SymFromAddr( hProcess, reinterpret_cast<DWORD64>(aStack[ i ]), nullptr, pSymbol );
        aBuf.append( static_cast<sal_Int32>(nFrames - i - 1) );
        aBuf.append( ": " );
        aBuf.appendAscii( pSymbol->Name );
        aBuf.append( " - 0x" );
        aBuf.append( static_cast<sal_Int64>(pSymbol->Address), 16 );
        aBuf.append( "\n" );
    }

    free( pSymbol );

    return aBuf.makeStringAndClear();
}

std::unique_ptr<sal::BacktraceState> sal::backtrace_get(sal_uInt32 maxDepth)
{
    assert(maxDepth != 0);
    maxDepth = clampToULONG(maxDepth);

    HANDLE hProcess = GetCurrentProcess();
    SymInitialize( hProcess, nullptr, true );

    auto pStack = new void *[maxDepth];
    // https://msdn.microsoft.com/en-us/library/windows/desktop/bb204633.aspx
    // "CaptureStackBackTrace function" claims that you "can capture up to
    // MAXUSHORT frames", and on Windows Server 2003 and Windows XP it even
    // "must be less than 63", but assume that a too large input value is
    // clamped internally, instead of resulting in an error:
    int nFrames = CaptureStackBackTrace( 0, static_cast<ULONG>(maxDepth), pStack, nullptr );

    return std::unique_ptr<BacktraceState>(new BacktraceState{ pStack, nFrames });
}

OUString sal::backtrace_to_string(BacktraceState* backtraceState)
{
    OUStringBuffer aBuf;

    SYMBOL_INFO  * pSymbol;
    pSymbol = static_cast<SYMBOL_INFO *>(calloc( sizeof( SYMBOL_INFO ) + 1024 * sizeof( char ), 1 ));
    pSymbol->MaxNameLen = 1024 - 1;
    pSymbol->SizeOfStruct = sizeof( SYMBOL_INFO );
    HANDLE hProcess = GetCurrentProcess();

    auto nFrames = backtraceState->nDepth;
    for( int i = 0; i < nFrames; i++ )
    {
        SymFromAddr( hProcess, reinterpret_cast<DWORD64>(backtraceState->buffer[ i ]), nullptr, pSymbol );
        aBuf.append( static_cast<sal_Int32>(nFrames - i - 1) );
        aBuf.append( ": " );
        aBuf.appendAscii( pSymbol->Name );
        aBuf.append( " - 0x" );
        aBuf.append( static_cast<sal_Int64>(pSymbol->Address), 16 );
        aBuf.append( "\n" );
    }

    free( pSymbol );

    return aBuf.makeStringAndClear();
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
