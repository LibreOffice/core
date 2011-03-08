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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_rsc.hxx"
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include <tools/rc.h>
#include <tools/list.hxx>
#include <rscerror.h>
#include <rsctools.hxx>
#include <rscclass.hxx>
#include <rsccont.hxx>
#include <rsctree.hxx>
#include <rscdb.hxx>
#include <rscdef.hxx>
#include <rscpar.hxx>

#include "rsclex.hxx"

/************** V a r i a b l e n ****************************************/
ObjectStack                     S;
RscTop *                        pCurClass;
sal_uInt32                      nCurMask;
char                            szErrBuf[ 100 ];

/************** H i l f s F u n k t i o n e n ****************************/
RSCINST GetVarInst( const RSCINST & rInst, const char * pVarName )
{
    RSCINST aInst;

    aInst = rInst.pClass->GetVariable( rInst, pHS->getID( pVarName ),
                                       RSCINST() );

    if( !aInst.pData )
        pTC->pEH->Error( ERR_NOVARIABLENAME, rInst.pClass, RscId() );

    return( aInst );
}

void SetNumber( const RSCINST & rInst, const char * pVarName, INT32 lValue )
{
    RSCINST aInst;

    aInst = GetVarInst( rInst, pVarName );

    if( aInst.pData ){
        ERRTYPE aError;
        aError = aInst.pClass->SetNumber( aInst, lValue );

        if( aError.IsError() )
            pTC->pEH->Error( aError, aInst.pClass, RscId() );
    }
}

void SetConst( const RSCINST & rInst, const char * pVarName,
               Atom nValueId, INT32 nVal )
{
    RSCINST aInst;

    aInst = GetVarInst( rInst, pVarName );
    if( aInst.pData )
    {
        ERRTYPE aError;
        aError = aInst.pClass->SetConst( aInst, nValueId, nVal );

        if( aError.IsError() )
            pTC->pEH->Error( aError, aInst.pClass, RscId() );
    }
}

void SetString( const RSCINST & rInst, const char * pVarName, const char * pStr )
{
    RSCINST aInst;

    aInst = GetVarInst( rInst, pVarName );
    if( aInst.pData ){
        ERRTYPE aError;
        aError = aInst.pClass->SetString( aInst, pStr );

        if( aError.IsError() )
            pTC->pEH->Error( aError, aInst.pClass, RscId() );
    }
}

RscId MakeRscId( RscExpType aExpType )
{
    if( !aExpType.IsNothing() ){
        INT32       lValue;

        if( !aExpType.Evaluate( &lValue ) )
            pTC->pEH->Error( ERR_ZERODIVISION, NULL, RscId() );
        if( lValue < 1 || lValue > (INT32)0x7FFF )
        {
            pTC->pEH->Error( ERR_IDRANGE, NULL, RscId(),
                             ByteString::CreateFromInt32( lValue ).GetBuffer() );
        }

        if( aExpType.IsDefinition() )
            return RscId( aExpType.aExp.pDef );
        else
            return RscId( lValue );
    }
    return RscId();
}

BOOL DoClassHeader( RSCHEADER * pHeader, BOOL bMember )
{
    RSCINST aCopyInst;
    RscId aName1 = MakeRscId( pHeader->nName1 );
    RscId aName2 = MakeRscId( pHeader->nName2 );

    if( pHeader->pRefClass )
        aCopyInst.pClass = pHeader->pRefClass;
    else
        aCopyInst.pClass = pHeader->pClass;

    if( TYPE_COPY == pHeader->nTyp )
    {
        ObjNode * pCopyObj = aCopyInst.pClass->GetObjNode( aName2 );

        if( !pCopyObj )
        {
            ByteString aMsg( pHS->getString( aCopyInst.pClass->GetId() ) );
            aMsg += ' ';
            aMsg += aName2.GetName();
            pTC->pEH->Error( ERR_NOCOPYOBJ, pHeader->pClass, aName1,
                             aMsg.GetBuffer() );
        }
        else
            aCopyInst.pData = pCopyObj->GetRscObj();
    }

    if( bMember )
    {
        // Angabe von Superklassen oder abgeleiteten Klassen ist jetzt erlaubt
        if( S.Top().pClass->InHierarchy( pHeader->pClass )
          ||  pHeader->pClass->InHierarchy( S.Top().pClass) )
        {
            if( aCopyInst.IsInst() )
            {
                RSCINST aTmpI( S.Top() );
                aTmpI.pClass->Destroy( aTmpI );
                aTmpI.pClass->Create( &aTmpI, aCopyInst );
            };
        }
        else
            pTC->pEH->Error( ERR_FALSETYPE, S.Top().pClass, aName1,
                             pHS->getString( pHeader->pClass->GetId() ) );
    }
    else
    {
        if( S.IsEmpty() )
        {
            if( (INT32)aName1 < 256 )
                pTC->pEH->Error( WRN_GLOBALID, pHeader->pClass, aName1 );

            if( aCopyInst.IsInst() )
                S.Push( pHeader->pClass->Create( NULL, aCopyInst ) );
            else
                S.Push( pHeader->pClass->Create( NULL, RSCINST() ) );

            ObjNode * pNode = new ObjNode( aName1, S.Top().pData,
                                           pFI->GetFileIndex() );
            pTC->pEH->StdOut( ".", RscVerbosityVerbose );

            if( !aName1.IsId() )
                pTC->pEH->Error( ERR_IDEXPECTED, pHeader->pClass, aName1 );
            else if( !pHeader->pClass->PutObjNode( pNode ) )
                pTC->pEH->Error( ERR_DOUBLEID, pHeader->pClass, aName1 );
        }
        else
        {
            RSCINST aTmpI;
            ERRTYPE aError;

            if( (INT32)aName1 >= 256 && aName1.IsId() )
                pTC->pEH->Error( WRN_LOCALID, pHeader->pClass, aName1 );
            aError = S.Top().pClass->GetElement( S.Top(), aName1,
                                                 pHeader->pClass, aCopyInst, &aTmpI );

            if( aError.IsWarning() )
                pTC->pEH->Error( aError, pHeader->pClass, aName1 );
            else if( aError.IsError() )
            {
                if( ERR_CONT_INVALIDTYPE == aError )
                    pTC->pEH->Error( aError, S.Top().pClass, aName1,
                                     pHS->getString( pHeader->pClass->GetId() ) );
                else
                    pTC->pEH->Error( aError, S.Top().pClass, aName1 );
                S.Top().pClass->GetElement( S.Top(), RscId(),
                                            pHeader->pClass, RSCINST(), &aTmpI );

                if( !aTmpI.IsInst() )
                    return( FALSE );
            }
            S.Push( aTmpI );
        };
    };
    if( TYPE_REF == pHeader->nTyp )
    {
        ERRTYPE aError;

        aError = S.Top().pClass->SetRef( S.Top(), aName2 );
        pTC->pEH->Error( aError, S.Top().pClass, aName1 );
    }

    return( TRUE );
}

RSCINST GetFirstTupelEle( const RSCINST & rTop )
{ // Aufwaertskompatible, Tupel probieren
    RSCINST aInst;
    ERRTYPE aErr;

    aErr = rTop.pClass->GetElement( rTop, RscId(), NULL, RSCINST(), &aInst );
    if( !aErr.IsError() )
        aInst = aInst.pClass->GetTupelVar( aInst, 0, RSCINST() );
    return aInst;
}

/************** Y a c c   C o d e ****************************************/
//#define YYDEBUG 1

#define TYPE_Atom             0
#define TYPE_RESID                1

#ifdef UNX
#define YYMAXDEPTH              2000
#else
#ifdef W30
#define YYMAXDEPTH              300
#else
#define YYMAXDEPTH              800
#endif
#endif

#if defined _MSC_VER
#pragma warning(push, 1)
#pragma warning(disable:4129 4273 4701)
#endif
#include "yyrscyacc.cxx"
#if defined _MSC_VER
#pragma warning(pop)
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
