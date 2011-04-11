/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#define PROT_SNAPSHOT   0x00040000

#define ACT_START           1
#define ACT_END             2
#define ACT_CREATE_MASTER   3
#define ACT_CREATE_FOLLOW   4
#define ACT_DEL_MASTER      5
#define ACT_DEL_FOLLOW      6
#define ACT_MERGE           7
#define ACT_NEXT_SECT       8
#define ACT_PREV_SECT       9

#define SNAP_LOWER       0x00000001
#define SNAP_FLYFRAMES   0x00000002
#define SNAP_TABLECONT   0x00000004

#if OSL_DEBUG_LEVEL > 1

#include "swtypes.hxx"

class SwImplProtocol;
class SwFrm;
class SwImplEnterLeave;

class SwProtocol
{
    static sal_uLong nRecord;
    static SwImplProtocol* pImpl;
    static sal_Bool Start() { return 0 != ( PROT_INIT & nRecord ); }
public:
    static sal_uLong Record() { return nRecord; }
    static void SetRecord( sal_uLong nNew ) { nRecord = nNew; }
    static sal_Bool Record( sal_uLong nFunc ) { return 0 != (( nFunc | PROT_INIT ) & nRecord); }
    static void Record( const SwFrm* pFrm, sal_uLong nFunction, sal_uLong nAction, void* pParam );
    static void Init();
    static void Stop();
    static void SnapShot( const SwFrm* pFrm, sal_uLong nFlags );
    static void GetVar( const sal_uInt16 nNo, long& rVar );
};

class SwEnterLeave
{
    SwImplEnterLeave* pImpl;
    void Ctor( const SwFrm* pFrm, sal_uLong nFunc, sal_uLong nAct, void* pPar );
    void Dtor();
public:
    SwEnterLeave( const SwFrm* pFrm, sal_uLong nFunc, sal_uLong nAct, void* pPar )
        { if( SwProtocol::Record( nFunc ) ) Ctor( pFrm, nFunc, nAct, pPar ); else pImpl = NULL; }
    ~SwEnterLeave() { if( pImpl ) Dtor(); }
};

#define PROTOCOL( pFrm, nFunc, nAct, pPar ) {   if( SwProtocol::Record( nFunc ) )\
                                                    SwProtocol::Record( pFrm, nFunc, nAct, pPar ); }
#define PROTOCOL_INIT SwProtocol::Init();
#define PROTOCOL_STOP SwProtocol::Stop();
#define PROTOCOL_ENTER( pFrm, nFunc, nAct, pPar ) SwEnterLeave aEnter( pFrm, nFunc, nAct, pPar );
#define PROTOCOL_SNAPSHOT( pFrm, nFlags ) SwProtocol::SnapShot( pFrm, nFlags );
#define GET_VARIABLE( nNo, nVar ) SwProtocol::GetVar( nNo, nVar );

#else

#define PROTOCOL( pFrm, nFunc, nAct, pPar )
#define PROTOCOL_INIT
#define PROTOCOL_STOP
#define PROTOCOL_ENTER( pFrm, nFunc, nAct, pPar )
#define PROTOCOL_SNAPSHOT( pFrm, nFlags )
#define GET_VARIABLE( nNo, nVar )

#endif

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
