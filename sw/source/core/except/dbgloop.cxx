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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

#ifdef DBG_UTIL

#include <tools/stream.hxx>
#include "dbgloop.hxx"
#include "errhdl.hxx"

DbgLoopStack DbgLoop::aDbgLoopStack;

/*************************************************************************
 *                      class DbgLoopStack
 *************************************************************************/

DbgLoopStack::DbgLoopStack()
{
    Reset();
}

void DbgLoopStack::Reset()
{
    nPtr = 0;
    pDbg = 0;
    for( sal_uInt16 i = 0; i < DBG_MAX_STACK; ++i )
        aCount[i] = 0;
}

/*************************************************************************
 *                       DbgLoopStack::Push()
 *************************************************************************/

void DbgLoopStack::Push( const void *pThis )
{
    // Wir muessen irgendwie mitbekommen, wann die erste Stackposition
    // resettet werden soll, z.B. wenn wir einen Nullpointer uebergeben
    if( !nPtr && ( pDbg != pThis || !pThis ) )
    {
        aCount[1] = 0;
        pDbg = pThis;
    }

    ++nPtr;
    if( DBG_MAX_STACK > nPtr )
    {
        // Wenn eine loop entdeckt wird, wird der counter wieder zurueckgesetzt.
        ASSERT( DBG_MAX_LOOP > aCount[nPtr], "DbgLoopStack::Push: loop detected" );
        if( DBG_MAX_LOOP > aCount[nPtr] )
            ++(aCount[nPtr]);
        else
            aCount[nPtr] = 0;
    }
}

/*************************************************************************
 *                       DbgLoopStack::Pop()
 *************************************************************************/

void DbgLoopStack::Pop()
{
    if( DBG_MAX_STACK > nPtr )
    {
        ASSERT( nPtr, "DbgLoopStack::Pop: can't pop the stack" );

        ASSERT( aCount[nPtr], "DbgLoopStack::Pop: can't dec the count" );
        if( DBG_MAX_STACK > nPtr + 1 )
            aCount[nPtr + 1] = 0;
    }
    --nPtr;
}

/*************************************************************************
 *                       DbgLoopStack::Print()
 *************************************************************************/

void DbgLoopStack::Print( SvStream &rOS ) const
{
    rOS << "POS: " << nPtr << '\n';
    sal_uInt16 i;
    for( i = 0; i < DBG_MAX_STACK; ++i )
        rOS << i << " ";
    rOS << '\n';
    for( i = 0; i < DBG_MAX_STACK; ++i )
        rOS << aCount[i] << " ";
    rOS << '\n';
}

#ifdef STAND_ALONE
// compile with: cl /AL /DSTAND_ALONE dbgloop.cxx

/*************************************************************************
 *                          main()
 *************************************************************************/

#include <stdlib.h>

void AssertFail( const char *pErr, const char *pFile, sal_uInt16 nLine )
{
    cout << pErr << '\n';
    PrintLoopStack( cout );
    exit(0);
}

class Test
{
public:
        void Run() const;
};

void Test::Run() const
{
    cout << "---" << '\n';
    for( sal_uInt16 i = 0; i < 10; ++i )
    {
        cout << "i" << i;
        DBG_LOOP;
        PrintLoopStack( cout );
        for( sal_uInt16 j = 0; j < 10; ++j )
        {
            cout << " j" << j;
            DBG_LOOP;
            PrintLoopStack( cout );
        }
        cout << '\n';
    }
    PrintLoopStack( cout );
}

int main()
{
    // unterschiedliche Instanzen waehlen wg. pDbg != pThis
    Test aTest1;
    aTest1.Run();
    Test aTest2;
    aTest2.Run();
    return 0;
}
#endif

#endif // DBG_UTIL

