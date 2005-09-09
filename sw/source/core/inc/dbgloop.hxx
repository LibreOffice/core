/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dbgloop.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 03:44:10 $
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

#ifndef _DBGLOOP_HXX
#define _DBGLOOP_HXX

#ifndef PRODUCT

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif

class SvStream;

#define DBG_MAX_STACK   20      // Verschachtelungstiefe
#define DBG_MAX_LOOP  1000      // das Abbruchkriterium

class DbgLoopStack
{
    USHORT aCount[DBG_MAX_STACK];
    USHORT nPtr;
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
