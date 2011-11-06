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



#ifndef _DBGLOOP_HXX
#define _DBGLOOP_HXX

#ifdef DBG_UTIL

#include <tools/solar.h>

class SvStream;

#define DBG_MAX_STACK   20      // Verschachtelungstiefe
#define DBG_MAX_LOOP  1000      // das Abbruchkriterium

class DbgLoopStack
{
    sal_uInt16 aCount[DBG_MAX_STACK];
    sal_uInt16 nPtr;
    const void *pDbg;
    void Reset();
public:
    DbgLoopStack();
    void Push( const void *pThis );
    void Pop();
    void Print( SvStream &rOS ) const; //$ ostream
};

class DbgLoop
{
    friend inline void PrintLoopStack( SvStream &rOS ); //$ ostream
    static DbgLoopStack aDbgLoopStack;
public:
    inline DbgLoop( const void *pThis ) { aDbgLoopStack.Push( pThis ); }
    inline ~DbgLoop() { aDbgLoopStack.Pop(); }
};

inline void PrintLoopStack( SvStream &rOS ) //$ ostream
{
    DbgLoop::aDbgLoopStack.Print( rOS );
}

#define DBG_LOOP    DbgLoop aDbgLoop( (const void*)this );
#define DBG_LOOP_RESET  DbgLoop aDbgLoop( 0 );

#else

#define DBG_LOOP
#define DBG_LOOP_RESET

#endif

#endif
