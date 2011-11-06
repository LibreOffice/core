/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

