/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include <stdio.h>
#include <string.h>
#include <rsctop.hxx>

RscTop::RscTop( Atom nId, sal_uInt32 nTypIdent, RscTop * pSuperCl )
    : RefNode( nId )
    , pSuperClass( pSuperCl )
    , nTypId( nTypIdent )
{
    pRefClass = this;
    if( pSuperClass )
        SetCallPar( pSuperClass->aCallPar1, pSuperClass->aCallPar2,
                    pSuperClass->aCallParType );
}

void RscTop::SetCallPar(const OString& rPar1, const OString& rPar2,
    const OString& rParType)
{
    aCallPar1 = rPar1;
    aCallPar2 = rPar2;
    aCallParType = rParType;
}

RSCINST const & RscTop::GetDefault()
{
    if( !aDfltInst.IsInst() )
        aDfltInst = this->Create( nullptr, RSCINST() );
    return aDfltInst;
}

void RscTop::Pre_dtor()
{
    if( aDfltInst.IsInst() )
    {
        aDfltInst.pClass->Destroy( aDfltInst );
        rtl_freeMemory( aDfltInst.pData );
        aDfltInst = RSCINST();
    };
}

RscTop * RscTop::GetTypeClass() const
{
    if( pSuperClass )
        return pSuperClass->GetTypeClass();
    else
        return nullptr;
}

sal_uInt32 RscTop::Size()
{
    if( pSuperClass )
        return pSuperClass->Size();
    else
        return 0;
}

ERRTYPE RscTop::GetRef( const RSCINST & rInst, RscId * pRscId )
{
    if( pSuperClass )
        return pSuperClass->GetRef( rInst, pRscId );
    else
        return ERR_UNKNOWN_METHOD;
}

bool RscTop::InHierarchy( RscTop * pClass )
{
    if( this == pClass )
        return true;
    if( pSuperClass )
        return pSuperClass->InHierarchy( pClass );
    return false;
}

ERRTYPE RscTop::SetVariable( Atom nVarName, RscTop * pClass,
                     RSCINST * pDflt, RSCVAR nVarType, sal_uInt32 nMask,
                     Atom nDataBaseName )
{
    if( pSuperClass )
        return pSuperClass->SetVariable( nVarName, pClass, pDflt,
                                         nVarType, nMask, nDataBaseName );
    else
        return ERR_UNKNOWN_METHOD;
}

ERRTYPE RscTop::SetVariable( Atom nVarName, RscTop * pClass,
                     RSCINST * pDflt, RSCVAR nVarType, SfxStyleItem nMask,
                     Atom nDataBaseName )
{
    if( pSuperClass )
        return pSuperClass->SetVariable( nVarName, pClass, pDflt,
                                         nVarType, nMask, nDataBaseName );
    else
        return ERR_UNKNOWN_METHOD;
}

ERRTYPE RscTop::SetVariable( Atom nVarName, RscTop * pClass,
                     RSCINST * pDflt, RSCVAR nVarType, SfxSlotInfo nMask,
                     Atom nDataBaseName )
{
    if( pSuperClass )
        return pSuperClass->SetVariable( nVarName, pClass, pDflt,
                                         nVarType, nMask, nDataBaseName );
    else
        return ERR_UNKNOWN_METHOD;
}

void RscTop::EnumVariables( void * pData, VarEnumCallbackProc pProc )
{
    if( pSuperClass )
        pSuperClass->EnumVariables( pData, pProc );
}

RSCINST RscTop::GetVariable( const RSCINST & rInst,
                             Atom nVarName,
                             const RSCINST & rInitInst,
                             bool bInitDflt,
                             RscTop * pCreateClass)
{
    if( pSuperClass )
        return pSuperClass->GetVariable( rInst, nVarName, rInitInst, bInitDflt, pCreateClass );
    else
        return RSCINST();
}

RSCINST RscTop::GetCopyVar( const RSCINST & rInst, Atom nVarName )
{
    if( pSuperClass )
        return pSuperClass->GetCopyVar( rInst, nVarName );
    else
        return RSCINST();
}

RSCINST RscTop::GetTupelVar( const RSCINST & rInst, sal_uInt32 nPos,
                            const RSCINST & rInitInst )
{
    if( pSuperClass )
        return pSuperClass->GetTupelVar( rInst, nPos, rInitInst );
    else
        return RSCINST();
}

ERRTYPE RscTop::GetElement( const RSCINST & rInst, const RscId & rEleName,
                    RscTop *pCreateClass, const RSCINST & rCreateInst,
                    RSCINST * pGetInst )
{
    if( pSuperClass )
        return pSuperClass-> GetElement( rInst, rEleName,
                                         pCreateClass, rCreateInst,
                                         pGetInst );
    else
        return ERR_UNKNOWN_METHOD;
}

ERRTYPE RscTop::GetArrayEle( const RSCINST & rInst,
                             Atom nId,
                             RscTop * pCreateClass,
                             RSCINST * pGetInst)
{
    if( pSuperClass )
        return pSuperClass->GetArrayEle( rInst, nId, pCreateClass, pGetInst );
    else
        return ERR_UNKNOWN_METHOD;
}

ERRTYPE RscTop::GetValueEle( const RSCINST & rInst,
                             sal_Int32 lValue,
                             RscTop * pCreateClass,
                             RSCINST * pGetInst)
{
    if( pSuperClass )
        return pSuperClass->GetValueEle( rInst, lValue, pCreateClass, pGetInst );
    else
        return ERR_UNKNOWN_METHOD;
}

RSCINST RscTop::SearchEle( const RSCINST & rInst, const RscId & rEleName,
                           RscTop * pClass )
{
    if( pSuperClass )
        return pSuperClass->SearchEle( rInst, rEleName, pClass );
    else
        return RSCINST();
}

RSCINST RscTop::GetPosEle( const RSCINST & rInst, sal_uInt32 nPos )
{
    if( pSuperClass )
        return pSuperClass->GetPosEle( rInst, nPos );
    else
        return RSCINST();
}

ERRTYPE RscTop::MovePosEle( const RSCINST & rInst, sal_uInt32 nDestPos,
                            sal_uInt32 nSourcePos )
{
    if( pSuperClass )
        return pSuperClass->MovePosEle( rInst, nDestPos, nSourcePos );
    else
        return ERR_UNKNOWN_METHOD;
}

ERRTYPE RscTop::SetPosRscId( const RSCINST & rInst, sal_uInt32 nPos,
                     const RscId & rRscId )
{
    if( pSuperClass )
        return pSuperClass->SetPosRscId( rInst, nPos, rRscId );
    else
        return ERR_UNKNOWN_METHOD;
}

SUBINFO_STRUCT RscTop::GetInfoEle( const RSCINST & rInst, sal_uInt32 nPos )
{
    if( pSuperClass )
        return pSuperClass->GetInfoEle( rInst, nPos );
    else
        return SUBINFO_STRUCT();
}

sal_uInt32  RscTop::GetCount( const RSCINST & rInst )
{
    if( pSuperClass )
        return pSuperClass->GetCount( rInst );
    else
        return 0;
}

ERRTYPE RscTop::SetNumber( const RSCINST & rInst, sal_Int32 lValue )
{
    if( pSuperClass )
        return pSuperClass->SetNumber( rInst, lValue );
    else
        return ERR_UNKNOWN_METHOD;
}

ERRTYPE RscTop::SetBool( const RSCINST & rInst, bool bValue )
{
    if( pSuperClass )
        return pSuperClass->SetBool( rInst, bValue );
    else
        return ERR_UNKNOWN_METHOD;
}

ERRTYPE RscTop::SetConst( const RSCINST & rInst, Atom nId, sal_Int32 nVal )
{
    if( pSuperClass )
        return pSuperClass->SetConst( rInst, nId, nVal );
    else
        return ERR_UNKNOWN_METHOD;
}

ERRTYPE RscTop::SetNotConst( const RSCINST & rInst, Atom nId )
{
    if( pSuperClass )
        return pSuperClass->SetNotConst( rInst, nId );
    else
        return ERR_UNKNOWN_METHOD;
}

ERRTYPE RscTop::SetString( const RSCINST & rInst, const char * pStr )
{
    if( pSuperClass )
        return pSuperClass->SetString( rInst, pStr );
    else
        return ERR_UNKNOWN_METHOD;
}

ERRTYPE RscTop::GetNumber( const RSCINST & rInst, sal_Int32 * pN )
{
    if( pSuperClass )
        return pSuperClass->GetNumber( rInst, pN );
    else
        return ERR_UNKNOWN_METHOD;
}

ERRTYPE RscTop::GetBool( const RSCINST & rInst, bool * pB )
{
    if( pSuperClass )
        return pSuperClass->GetBool( rInst, pB );
    else
        return ERR_UNKNOWN_METHOD;
}

ERRTYPE RscTop::GetConst( const RSCINST & rInst, Atom * pH )
{
    if( pSuperClass )
        return pSuperClass->GetConst( rInst, pH );
    else
        return ERR_UNKNOWN_METHOD;
}

ERRTYPE RscTop::GetString( const RSCINST & rInst, char ** ppStr )
{
    if( pSuperClass )
        return pSuperClass->GetString( rInst, ppStr );
    else
        return ERR_UNKNOWN_METHOD;
}

RSCINST RscTop::Create( RSCINST * pInst, const RSCINST & rDefInst, bool bOwnRange )
{
    if( pSuperClass )
        return pSuperClass->Create( pInst, rDefInst, bOwnRange );
    else
    {
        if( pInst )
            return *pInst;
        return RSCINST();
    }
}

void    RscTop::Destroy( const RSCINST & rInst )
{
    if( pSuperClass )
        pSuperClass->Destroy( rInst );
}

bool RscTop::IsConsistent( const RSCINST & rInst )
{
    if( pSuperClass )
        return pSuperClass->IsConsistent( rInst );
    else
        return true;
}

void    RscTop::SetToDefault( const RSCINST & rInst )
{
    if( pSuperClass )
        pSuperClass->SetToDefault( rInst );
}

bool    RscTop::IsDefault( const RSCINST & rInst )
{
    if( pSuperClass )
        return pSuperClass->IsDefault( rInst );
    else
        return true;
}

bool    RscTop::IsValueDefault( const RSCINST & rInst, CLASS_DATA pDef )
{
    if( pSuperClass )
        return pSuperClass->IsValueDefault( rInst, pDef );
    else
        return true;
}

void    RscTop::SetDefault( const RSCINST & rInst, Atom nVarId )
{
    if( pSuperClass )
        pSuperClass->SetDefault( rInst, nVarId );
}

RSCINST RscTop::GetDefault( Atom nVarId )
{
    if( pSuperClass )
        return pSuperClass->GetDefault( nVarId );
    else
        return RSCINST();
}

void    RscTop::Delete( const RSCINST & rInst, RscTop * pClass,
                        const RscId & rId )
{
    if( pSuperClass )
        pSuperClass->Delete( rInst, pClass, rId );
}

void    RscTop::DeletePos( const RSCINST & rInst, sal_uInt32 nPos )
{
    if( pSuperClass )
        pSuperClass->DeletePos( rInst, nPos );
}

ERRTYPE RscTop::SetRef( const RSCINST & rInst, const RscId & rRefId )
{
    if( pSuperClass )
        return pSuperClass->SetRef( rInst, rRefId );
    else
        return ERR_UNKNOWN_METHOD;
}

void RscTop::WriteSrcHeader( const RSCINST & rInst, FILE * fOutput,
                             RscTypCont * pTC, sal_uInt32 nTab,
                             const RscId & rId, const char * pVarName )
{
    if( pSuperClass )
        pSuperClass->WriteSrcHeader( rInst, fOutput, pTC, nTab, rId, pVarName );
    else
        rInst.pClass->WriteSrc( rInst, fOutput, pTC, nTab, pVarName );
}

void RscTop::WriteSrc( const RSCINST & rInst, FILE * fOutput,
                       RscTypCont * pTC, sal_uInt32 nTab, const char * pVarName )
{
    if( pSuperClass )
        pSuperClass->WriteSrc( rInst, fOutput, pTC, nTab, pVarName );
}

ERRTYPE RscTop::WriteRcHeader( const RSCINST & rInst, RscWriteRc & rMem,
                               RscTypCont * pTC, const RscId & rId,
                               sal_uInt32 nDeep, bool bExtra )
{
    if( pSuperClass )
        return pSuperClass->WriteRcHeader( rInst, rMem, pTC, rId, nDeep, bExtra );
    else
        return rInst.pClass->WriteRc( rInst, rMem, pTC, nDeep, bExtra );
}

ERRTYPE RscTop::WriteRc( const RSCINST & rInst, RscWriteRc & rMem,
                         RscTypCont * pTC, sal_uInt32 nDeep, bool bExtra )
{
    if( pSuperClass )
        return pSuperClass->WriteRc( rInst, rMem, pTC, nDeep, bExtra );
    else
        return ERR_OK;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
