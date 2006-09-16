/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dbgloop.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 21:09:10 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

#ifdef PRODUCT
#error Wer fummelt denn an den makefiles rum?
#endif

#ifndef _STREAM_HXX //autogen
#include <tools/stream.hxx>
#endif
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
    for( USHORT i = 0; i < DBG_MAX_STACK; ++i )
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
    USHORT i;
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

void AssertFail( const char *pErr, const char *pFile, USHORT nLine )
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
    for( USHORT i = 0; i < 10; ++i )
    {
        cout << "i" << i;
        DBG_LOOP;
        PrintLoopStack( cout );
        for( USHORT j = 0; j < 10; ++j )
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


