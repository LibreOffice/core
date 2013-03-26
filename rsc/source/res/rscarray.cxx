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


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <rscconst.hxx>
#include <rscarray.hxx>
#include <rscdb.hxx>

RscInstNode::RscInstNode( sal_uInt32 nId )
{
    nTypeId = nId;
}

RscInstNode::~RscInstNode()
{
    if( aInst.IsInst() )
    {
        aInst.pClass->Destroy( aInst );
        rtl_freeMemory( aInst.pData );
    }
}

sal_uInt32 RscInstNode::GetId() const
{
    return nTypeId;
}

RscArray::RscArray( Atom nId, sal_uInt32 nTypeId, RscTop * pSuper, RscEnum * pTypeCl )
        : RscTop( nId, nTypeId, pSuper )
{
    pTypeClass = pTypeCl;
    nOffInstData = RscTop::Size();
    nSize = nOffInstData + ALIGNED_SIZE( sizeof( RscArrayInst ) );
}

RscArray::~RscArray()
{
}

RSCCLASS_TYPE RscArray::GetClassType() const
{
    return RSCCLASS_ENUMARRAY;
}

RscTop * RscArray::GetTypeClass() const
{
    return pTypeClass;
}

static RscInstNode * Create( RscInstNode * pNode )
{
    RscInstNode * pRetNode = NULL;

    if( pNode )
    {
        pRetNode = new RscInstNode( pNode->GetId() );
        pRetNode->aInst = pNode->aInst.pClass->Create( NULL, pNode->aInst );
        RscInstNode * pTmpNode = Create(pNode->Left());
        if (pTmpNode)
            pRetNode->Insert( pTmpNode );
        if( (pTmpNode = Create( pNode->Right() )) != NULL )
            pRetNode->Insert( pTmpNode );
    }

    return pRetNode;
}

RSCINST RscArray::Create( RSCINST * pInst, const RSCINST & rDflt,
                          sal_Bool bOwnClass )
{
    RSCINST aInst;
    RscArrayInst *  pClassData;

    if( !pInst )
    {
        aInst.pClass = this;
        aInst.pData = (CLASS_DATA) rtl_allocateMemory( Size() );
    }
    else
        aInst = *pInst;
    if( !bOwnClass && rDflt.IsInst() )
        bOwnClass = rDflt.pClass->InHierarchy( this );

    RscTop::Create( &aInst, rDflt, bOwnClass );

    pClassData          = (RscArrayInst *)(aInst.pData + nOffInstData);
    pClassData->pNode   = NULL;
    if( bOwnClass )
    {
        RscArrayInst *   pDfltClassData;

        pDfltClassData = (RscArrayInst *)(rDflt.pData + nOffInstData);

        pClassData->pNode = ::Create( pDfltClassData->pNode );
    }
    return( aInst );
}

static void Destroy( RscInstNode * pNode )
{
    if( pNode )
    {
        Destroy( pNode->Left() );
        Destroy( pNode->Right() );
        delete pNode;
    }
}

void RscArray::Destroy( const RSCINST & rInst )
{
    RscArrayInst *  pClassData;

    RscTop::Destroy( rInst );

    pClassData = (RscArrayInst *)(rInst.pData + nOffInstData);

    //Baum rekursiv loeschen
    ::Destroy( pClassData->pNode );
}

ERRTYPE RscArray::GetValueEle
(
    const RSCINST & rInst,
    sal_Int32 lValue,
    RscTop * pCreateClass,
    RSCINST * pGetInst
)
{
    RscArrayInst *  pClassData;
    RscInstNode *   pNode;

    pClassData = (RscArrayInst *)(rInst.pData + nOffInstData);

    ERRTYPE     aError;

    Atom  nId;
    if( !pTypeClass->GetValueConst( sal_uInt32(lValue), &nId ) )
    { // nicht gefunden
        return ERR_ARRAY_INVALIDINDEX;
    }

    if( pClassData->pNode )
        pNode = pClassData->pNode->Search( sal_uInt32(lValue) );
    else
        pNode = NULL;

    if( !pNode )
    {
        pNode = new RscInstNode( sal_uInt32(lValue) );
        if( pCreateClass && GetSuperClass()->InHierarchy( pCreateClass ) )
            pNode->aInst = pCreateClass->Create( NULL, rInst );
        else
            pNode->aInst = GetSuperClass()->Create( NULL, rInst );
        pNode->aInst.pClass->SetToDefault( pNode->aInst );
        if( pClassData->pNode )
            pClassData->pNode->Insert( pNode );
        else
            pClassData->pNode = pNode;
    }

    *pGetInst = pNode->aInst;
    return aError;
}

ERRTYPE RscArray::GetArrayEle
(
    const RSCINST & rInst,
    Atom nId,
    RscTop * pCreateClass,
    RSCINST * pGetInst
)
{
    sal_Int32  lValue;
    if( !pTypeClass->GetConstValue( nId, &lValue ) )
    { // nicht gefunden
        return ERR_ARRAY_INVALIDINDEX;
    }

    return GetValueEle( rInst, lValue, pCreateClass, pGetInst );
}

static sal_Bool IsConsistent( RscInstNode * pNode )
{
    sal_Bool bRet = sal_True;

    if( pNode )
    {
        bRet = pNode->aInst.pClass->IsConsistent( pNode->aInst );
        if( !IsConsistent( pNode->Left() ) )
            bRet = sal_False;
        if( !IsConsistent( pNode->Right() ) )
            bRet = sal_False;
    }
    return bRet;
}

sal_Bool RscArray::IsConsistent( const RSCINST & rInst )
{
    RscArrayInst * pClassData;
    sal_Bool    bRet;

    bRet = RscTop::IsConsistent( rInst );

    pClassData = (RscArrayInst *)(rInst.pData + nOffInstData);
    if( !::IsConsistent( pClassData->pNode ) )
        bRet = sal_False;

    return( bRet );
}

static void SetToDefault( RscInstNode * pNode )
{
    if( pNode )
    {
        pNode->aInst.pClass->SetToDefault( pNode->aInst );
        SetToDefault( pNode->Left() );
        SetToDefault( pNode->Right() );
    }
}

void RscArray::SetToDefault( const RSCINST & rInst )
{
    RscArrayInst * pClassData;

    pClassData = (RscArrayInst *)(rInst.pData + nOffInstData);

    ::SetToDefault( pClassData->pNode );

    RscTop::SetToDefault( rInst );
}

static sal_Bool IsDefault( RscInstNode * pNode )
{
    sal_Bool bRet = sal_True;

    if( pNode )
    {
        bRet = pNode->aInst.pClass->IsDefault( pNode->aInst );
        if( bRet )
            bRet = IsDefault( pNode->Left() );
        if( bRet )
            bRet = IsDefault( pNode->Right() );
    }
    return bRet;
}

sal_Bool RscArray::IsDefault( const RSCINST & rInst )
{
    RscArrayInst * pClassData;

    pClassData = (RscArrayInst *)(rInst.pData + nOffInstData);

    sal_Bool bRet = ::IsDefault( pClassData->pNode );

    if( bRet )
        bRet = RscTop::IsDefault( rInst );
    return bRet;
}

static sal_Bool IsValueDefault( RscInstNode * pNode, CLASS_DATA pDef )
{
    sal_Bool bRet = sal_True;

    if( pNode )
    {
        bRet = pNode->aInst.pClass->IsValueDefault( pNode->aInst, pDef );
        if( bRet )
            bRet = IsValueDefault( pNode->Left(), pDef );
        if( bRet )
            bRet = IsValueDefault( pNode->Right(), pDef );
    }
    return bRet;
}

sal_Bool RscArray::IsValueDefault( const RSCINST & rInst, CLASS_DATA pDef )
{
    sal_Bool bRet = RscTop::IsValueDefault( rInst, pDef );

    if( bRet )
    {
        RscArrayInst * pClassData = (RscArrayInst *)(rInst.pData + nOffInstData);

        bRet = ::IsValueDefault( pClassData->pNode, pDef );
    }
    return bRet;
}

void RscArray::WriteSrcHeader( const RSCINST & rInst, FILE * fOutput,
                               RscTypCont * pTC, sal_uInt32 nTab,
                               const RscId & aId, const char * pVarName )
{
    RscArrayInst * pClassData;

    pClassData = (RscArrayInst *)(rInst.pData + nOffInstData);

    if( pTC->IsSrsDefault() )
    { // nur einen Wert schreiben
        RscInstNode *   pNode = NULL;
        if( pClassData->pNode )
        {
            std::vector< sal_uInt32 >::const_iterator it;
            for( it = pTC->GetFallbacks().begin(); !pNode && it != pTC->GetFallbacks().end(); ++it )
                pNode = pClassData->pNode->Search( *it );
        }

        if( pNode )
        {
            if( pNode->aInst.pClass->IsDefault( pNode->aInst ) )
                fprintf( fOutput, "Default" );
            else
                pNode->aInst.pClass->WriteSrcHeader(
                                        pNode->aInst, fOutput,
                                        pTC, nTab, aId, pVarName );
            return;
        }
    }

    if( IsDefault( rInst ) )
        fprintf( fOutput, "Default" );
    else
    {
        RSCINST aSuper( GetSuperClass(), rInst.pData );
        aSuper.pClass->WriteSrcHeader( aSuper, fOutput, pTC,
                                        nTab, aId, pVarName );
    }
    if( !pTC->IsSrsDefault() )
        WriteSrc( rInst, fOutput, pTC, nTab, pVarName );
}

static void WriteSrc( RscInstNode * pNode, FILE * fOutput, RscTypCont * pTC,
                         sal_uInt32 nTab, const char * pVarName,
                         CLASS_DATA pDfltData, RscConst * pTypeClass )
{
    if( pNode )
    {
        WriteSrc( pNode->Left(), fOutput, pTC, nTab, pVarName,
                    pDfltData, pTypeClass );
        if( !pNode->aInst.pClass->IsValueDefault( pNode->aInst, pDfltData ) )
        {
            fprintf( fOutput, ";\n" );
            for( sal_uInt32 n = 0; n < nTab; n++ )
                fputc( '\t', fOutput );

            Atom  nIdxId;
            pTypeClass->GetValueConst( pNode->GetId(), &nIdxId );
            fprintf( fOutput, "%s[ %s ] = ", pVarName, pHS->getString( nIdxId ).getStr() );
            pNode->aInst.pClass->WriteSrcHeader( pNode->aInst, fOutput, pTC,
                                                nTab, RscId(), pVarName );
        }
        WriteSrc( pNode->Right(), fOutput, pTC, nTab, pVarName,
                    pDfltData, pTypeClass );
    }
}

void RscArray::WriteSrcArray( const RSCINST & rInst, FILE * fOutput,
                             RscTypCont * pTC, sal_uInt32 nTab,
                             const char * pVarName )
{
    RscArrayInst * pClassData;

    pClassData = (RscArrayInst *)(rInst.pData + nOffInstData);

    ::WriteSrc( pClassData->pNode, fOutput, pTC, nTab, pVarName,
                rInst.pData, pTypeClass );
};

void RscArray::WriteSrc( const RSCINST & rInst, FILE * fOutput,
                         RscTypCont * pTC, sal_uInt32 nTab,
                         const char * pVarName )
{
    WriteSrcArray( rInst, fOutput, pTC, nTab, pVarName );
}

ERRTYPE RscArray::WriteRc( const RSCINST & rInst, RscWriteRc & rMem,
                            RscTypCont * pTC, sal_uInt32 nDeep, sal_Bool bExtra )
{
    ERRTYPE aError;
    RscArrayInst * pClassData;
    RscInstNode *   pNode = NULL;

    pClassData = (RscArrayInst *)(rInst.pData + nOffInstData);

    if( pClassData->pNode )
    {
#if OSL_DEBUG_LEVEL > 2
        fprintf( stderr, "RscArray::WriteRc: Fallback " );
#endif
        std::vector< sal_uInt32 >::const_iterator it;
        for( it = pTC->GetFallbacks().begin(); !pNode && it != pTC->GetFallbacks().end(); ++it )
        {
            pNode = pClassData->pNode->Search( *it );
#if OSL_DEBUG_LEVEL > 2
            fprintf( stderr, " 0x%hx", *it );
#endif
        }
#if OSL_DEBUG_LEVEL > 2
            fprintf( stderr, "\n" );
#endif
    }

    if( pNode )
        aError = pNode->aInst.pClass->WriteRc( pNode->aInst, rMem, pTC,
                                                nDeep, bExtra );
    else
        aError = RscTop::WriteRc( rInst, rMem, pTC, nDeep, bExtra );

    return aError;
}

RscClassArray::RscClassArray( Atom nId, sal_uInt32 nTypeId, RscTop * pSuper,
                              RscEnum * pTypeCl )
    : RscArray( nId, nTypeId, pSuper, pTypeCl )
{
}

RscClassArray::~RscClassArray()
{
}

void RscClassArray::WriteSrcHeader( const RSCINST & rInst, FILE * fOutput,
                                    RscTypCont * pTC, sal_uInt32 nTab,
                                    const RscId & aId, const char * pName )
{
    RscArray::WriteSrcHeader( rInst, fOutput, pTC, nTab, aId, pName );
}

void RscClassArray::WriteSrc( const RSCINST & rInst, FILE * fOutput,
                             RscTypCont * pTC, sal_uInt32 nTab,
                                 const char * pVarName )
{
    RscArray::WriteSrc( rInst, fOutput, pTC, nTab, pVarName );
}

ERRTYPE RscClassArray::WriteRcHeader( const RSCINST & rInst, RscWriteRc & aMem,
                                       RscTypCont * pTC, const RscId & aId,
                                       sal_uInt32 nDeep, sal_Bool bExtra )
{
    // Eigenen Typ schreiben
    return GetSuperClass()->WriteRcHeader( rInst, aMem, pTC, aId,
                                        nDeep, bExtra );
}

RscLangArray::RscLangArray( Atom nId, sal_uInt32 nTypeId, RscTop * pSuper,
                          RscEnum * pTypeCl )
    : RscArray( nId, nTypeId, pSuper, pTypeCl )
{
}

RSCCLASS_TYPE RscLangArray::GetClassType() const
{
    if( GetSuperClass() )
        return GetSuperClass()->GetClassType();
    else
        return RscArray::GetClassType();

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
