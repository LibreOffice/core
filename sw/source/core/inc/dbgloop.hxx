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
