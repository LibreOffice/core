/*************************************************************************
 *
 *  $RCSfile: dbg_lay.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:20 $
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

#ifndef _DBG_LAY_HXX
#define _DBG_LAY_HXX

#define PROT_FILE_INIT  0x00000000
#define PROT_INIT       0x00000001
#define PROT_MAKEALL    0x00000002
#define PROT_MOVE_FWD   0x00000004
#define PROT_MOVE_BWD   0x00000008
#define PROT_GROW       0x00000010
#define PROT_SHRINK     0x00000020
#define PROT_GROW_TST   0x00000040
#define PROT_SHRINK_TST 0x00000080
#define PROT_SIZE       0x00000100
#define PROT_PRTAREA    0x00000200
#define PROT_POS        0x00000400
#define PROT_ADJUSTN    0x00000800
#define PROT_SECTION    0x00001000
#define PROT_CUT        0x00002000
#define PROT_PASTE      0x00004000
#define PROT_LEAF       0x00008000
#define PROT_TESTFORMAT 0x00010000
#define PROT_FRMCHANGES 0x00020000


#define ACT_START           1
#define ACT_END             2
#define ACT_CREATE_MASTER   3
#define ACT_CREATE_FOLLOW   4
#define ACT_DEL_MASTER      5
#define ACT_DEL_FOLLOW      6
#define ACT_MERGE           7
#define ACT_NEXT_SECT       8
#define ACT_PREV_SECT       9

#ifndef PRODUCT

#include "swtypes.hxx"

class SwImplProtocol;
class SwFrm;
class SwImplEnterLeave;

class SwProtocol
{
    static ULONG nRecord;
    static SwImplProtocol* pImpl;
    static BOOL Start() { return 0 != ( PROT_INIT & nRecord ); }
public:
    static ULONG Record() { return nRecord; }
    static void SetRecord( ULONG nNew ) { nRecord = nNew; }
    static BOOL Record( ULONG nFunc ) { return 0 != (( nFunc | PROT_INIT ) & nRecord); }
    static void Record( const SwFrm* pFrm, ULONG nFunction, ULONG nAction, void* pParam );
    static void Init();
    static void Stop();
};

class SwEnterLeave
{
    SwImplEnterLeave* pImpl;
    void Ctor( const SwFrm* pFrm, ULONG nFunc, ULONG nAct, void* pPar );
    void Dtor();
public:
    SwEnterLeave( const SwFrm* pFrm, ULONG nFunc, ULONG nAct, void* pPar )
        { if( SwProtocol::Record( nFunc ) ) Ctor( pFrm, nFunc, nAct, pPar ); else pImpl = NULL; }
    ~SwEnterLeave() { if( pImpl ) Dtor(); }
};

#define PROTOCOL( pFrm, nFunc, nAct, pPar ) {   if( SwProtocol::Record( nFunc ) )\
                                                    SwProtocol::Record( pFrm, nFunc, nAct, pPar ); }
#define PROTOCOL_INIT SwProtocol::Init();
#define PROTOCOL_STOP SwProtocol::Stop();
#define PROTOCOL_ENTER( pFrm, nFunc, nAct, pPar ) SwEnterLeave aEnter( pFrm, nFunc, nAct, pPar );

#else

#define PROTOCOL( pFrm, nFunc, nAct, pPar )
#define PROTOCOL_INIT
#define PROTOCOL_STOP
#define PROTOCOL_ENTER( pFrm, nFunc, nAct, pPar )

#endif

#endif
