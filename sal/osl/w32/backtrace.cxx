/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include "backtraceasstring.hxx"

#include <windows.h>
#include <process.h>
#include <iostream>
#define OPTIONAL
#include <DbgHelp.h>

#include <rtl/ustrbuf.hxx>
#include <memory>

OUString osl::detail::backtraceAsString(int maxNoStackFramesToDisplay)
{
    OUStringBuffer aBuf;

    HANDLE hProcess = GetCurrentProcess();
    SymInitialize( hProcess, nullptr, true );

    std::unique_ptr<void*[]> aStack(new void*[ maxNoStackFramesToDisplay ]);
    sal_uInt32 nFrames = CaptureStackBackTrace( 0, maxNoStackFramesToDisplay, aStack.get(), nullptr );

    SYMBOL_INFO  * pSymbol;
    pSymbol = static_cast<SYMBOL_INFO *>(calloc( sizeof( SYMBOL_INFO ) + 1024 * sizeof( char ), 1 ));
    pSymbol->MaxNameLen = 1024 - 1;
    pSymbol->SizeOfStruct = sizeof( SYMBOL_INFO );

    for( sal_uInt32 i = 0; i < nFrames; i++ )
    {
        SymFromAddr( hProcess, reinterpret_cast<DWORD64>(aStack[ i ]), nullptr, pSymbol );
        aBuf.append( (sal_Int32)(nFrames - i - 1) );
        aBuf.append( ": " );
        aBuf.appendAscii( pSymbol->Name );
        aBuf.append( " - 0x" );
        aBuf.append( (sal_Int64)pSymbol->Address, 16 );
        aBuf.append( "\n" );
    }

    free( pSymbol );

    return aBuf.makeStringAndClear();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
