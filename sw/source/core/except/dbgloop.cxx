/*************************************************************************
 *
 *  $RCSfile: dbgloop.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:19 $
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
    for( USHORT i = 0; i < DBG_MAX_STACK; ++i )
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


