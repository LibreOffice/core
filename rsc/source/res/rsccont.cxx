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

#include <rsccont.hxx>

#include <tools/rcid.h>

void ENTRY_STRUCT::Destroy()
{
    aName.Destroy();
    if( aInst.IsInst() )
    {
        aInst.pClass->Destroy( aInst );
        rtl_freeMemory( aInst.pData );
    }
}

RscBaseCont::RscBaseCont( Atom nId, sal_uInt32 nTypeId, RscTop * pSuper,
                          bool bNoIdent )
    : RscTop( nId, nTypeId, pSuper )
    , nSize( 0 )
{
    pTypeClass = nullptr;
    pTypeClass1 = nullptr;
    bNoId = bNoIdent;
    nOffInstData = RscTop::Size();
    nSize = nOffInstData + ALIGNED_SIZE( sizeof( RscBaseContInst ) );
}

RscBaseCont::~RscBaseCont()
{
}

RSCCLASS_TYPE RscBaseCont::GetClassType() const
{
    return RSCCLASS_COMPLEX;
}

void RscBaseCont::DestroyElements( RscBaseContInst * pClassData )
{
    if( pClassData->nEntries )
    {
        for (sal_uInt32 i = 0; i < pClassData->nEntries; i++ )
        {
            pClassData->pEntries[ i ].Destroy();
        }
        rtl_freeMemory( pClassData->pEntries );
        pClassData->pEntries = nullptr;
        pClassData->nEntries = 0;
    }
}

RSCINST RscBaseCont::Create( RSCINST * pInst, const RSCINST & rDflt,
                             bool bOwnClass )
{
    RSCINST aInst;
    RscBaseContInst * pClassData;

    if( !pInst )
    {
        aInst.pClass = this;
        aInst.pData = static_cast<CLASS_DATA>(rtl_allocateMemory( Size() ));
    }
    else
        aInst = *pInst;

    if( !bOwnClass && rDflt.IsInst() )
        bOwnClass = rDflt.pClass->InHierarchy( this );

    RscTop::Create( &aInst, rDflt, bOwnClass );

    pClassData = reinterpret_cast<RscBaseContInst *>(aInst.pData + nOffInstData);
    pClassData->nEntries = 0;
    pClassData->pEntries = nullptr;
    pClassData->bDflt = true;

    if( bOwnClass )
    {
        RscBaseContInst *   pDfltClassData;
        RSCINST         aDfltI;

        pDfltClassData = reinterpret_cast<RscBaseContInst *>(rDflt.pData + nOffInstData);

        if( 0 != pDfltClassData->nEntries )
        {
            *pClassData = *pDfltClassData;
            pClassData->pEntries =
                static_cast<ENTRY_STRUCT *>(rtl_allocateMemory( sizeof( ENTRY_STRUCT )
                                                    * pClassData->nEntries ));
            for (sal_uInt32 i = 0; i < pClassData->nEntries; i++ )
            {
                pClassData->pEntries[ i ].Create();
                pClassData->pEntries[ i ].aName =
                                    pDfltClassData->pEntries[ i ].aName;
                aDfltI = pDfltClassData->pEntries[ i ].aInst;
                pClassData->pEntries[ i ].aInst =
                                    aDfltI.pClass->Create( nullptr, aDfltI );
            }
        }
    }

    return aInst;
}

void RscBaseCont::Destroy( const RSCINST & rInst )
{
    RscBaseContInst * pClassData;

    RscTop::Destroy( rInst);

    pClassData = reinterpret_cast<RscBaseContInst *>(rInst.pData + nOffInstData);
    DestroyElements( pClassData );
}

RSCINST RscBaseCont::SearchElePos( const RSCINST & rInst, const RscId & rEleName,
                                   RscTop * pClass, sal_uInt32 nPos )
{
    RscBaseContInst * pClassData;

    pClassData = reinterpret_cast<RscBaseContInst *>(rInst.pData + nOffInstData);
    if( !pClass )
        pClass = pTypeClass;

    if( rEleName.IsId() )
    {
        for (sal_uInt32 i = nPos; i < pClassData->nEntries; i++ )
        {
            if( pClassData->pEntries[ i ].aName == rEleName &&
                pClassData->pEntries[ i ].aInst.pClass == pClass )
            {
                return pClassData->pEntries[ i ].aInst;
            }
        }
    }
    return RSCINST();
}

RSCINST RscBaseCont::SearchEle( const RSCINST & rInst, const RscId & rEleName,
                                RscTop * pClass )
{
    return SearchElePos( rInst, rEleName, pClass, 0 );
}

ERRTYPE RscBaseCont::GetElement( const RSCINST & rInst, const RscId & rEleName,
                                 RscTop * pCreateClass,
                                 const RSCINST & rCreateInst, RSCINST * pGetInst )
{
    RscBaseContInst * pClassData;
    RSCINST           aTmpI;
    ERRTYPE           aError;

    if( !bNoId && !rEleName.IsId() )
        aError = WRN_CONT_NOID;

    pClassData = reinterpret_cast<RscBaseContInst *>(rInst.pData + nOffInstData);

    if( pCreateClass )
    {
        if( !pCreateClass->InHierarchy( pTypeClass ) )
        {
            if( pTypeClass1 )
            {
                if( !pCreateClass->InHierarchy( pTypeClass1 ) )
                {
                    return ERR_CONT_INVALIDTYPE;
                }
            }
            else
            {
                return ERR_CONT_INVALIDTYPE;
            }
        }
    }
    else
        pCreateClass = pTypeClass;

    pClassData->bDflt = false;

    if( !bNoId )
        aTmpI = SearchEle( rInst, rEleName, pCreateClass );
    // entry found
    if( aTmpI.IsInst() )
    {
        aError = WRN_CONT_DOUBLEID;
        if( rCreateInst.IsInst() )
        {
            aTmpI.pClass->Destroy( aTmpI );
            aTmpI.pClass->Create( &aTmpI, rCreateInst );
        }
    }
    else
    {
        if( pClassData->pEntries )
        {
            pClassData->pEntries =
                static_cast<ENTRY_STRUCT *>(rtl_reallocateMemory( pClassData->pEntries,
                         sizeof( ENTRY_STRUCT ) * (pClassData->nEntries +1) ));
        }
        else
        {
            pClassData->pEntries =
                static_cast<ENTRY_STRUCT *>(rtl_allocateMemory( sizeof( ENTRY_STRUCT )
                                                * (pClassData->nEntries +1) ));
        }

        pClassData->pEntries[ pClassData->nEntries ].Create();
        pClassData->pEntries[ pClassData->nEntries ].aName = rEleName;

        if( rCreateInst.IsInst() )
        {
            // initialize instance with CreateInst data
            pClassData->pEntries[ pClassData->nEntries ].aInst =
                pCreateClass->Create( nullptr, rCreateInst );
        }
        else
        {
            pClassData->pEntries[ pClassData->nEntries ].aInst =
                pCreateClass->Create( nullptr, RSCINST() );
        }

        pClassData->nEntries++;
        aTmpI = pClassData->pEntries[ pClassData->nEntries -1 ].aInst;
    }

    *pGetInst = aTmpI;
    return aError;
}

sal_uInt32 RscBaseCont::GetCount( const RSCINST & rInst )
{
    RscBaseContInst * pClassData;

    pClassData = reinterpret_cast<RscBaseContInst *>(rInst.pData + nOffInstData);
    return pClassData->nEntries;
}

RSCINST RscBaseCont::GetPosEle( const RSCINST & rInst, sal_uInt32 nPos )
{
    RscBaseContInst * pClassData;

    pClassData = reinterpret_cast<RscBaseContInst *>(rInst.pData + nOffInstData);

    if( nPos < pClassData->nEntries )
        return pClassData->pEntries[ nPos ].aInst;
    return RSCINST();
}

ERRTYPE RscBaseCont::MovePosEle( const RSCINST & rInst, sal_uInt32 nDestPos,
                                 sal_uInt32 nSourcePos )
{
    ERRTYPE aError;
    RscBaseContInst * pClassData;

    pClassData = reinterpret_cast<RscBaseContInst *>(rInst.pData + nOffInstData);

    if( (nDestPos < pClassData->nEntries) && (nSourcePos < pClassData->nEntries) )
    {
        ENTRY_STRUCT aEntry;
        int nInc = 1;
        sal_uInt32 i = 0;

        // mark source
        aEntry = pClassData->pEntries[ nSourcePos ];
        // guess direction of the for-loop
        if( nDestPos < nSourcePos )
            nInc = -1;

        for( i = nSourcePos; i != nDestPos; i += nInc )
            pClassData->pEntries[ i ] = pClassData->pEntries[ i + nInc ];

        // assign source to target
        pClassData->pEntries[ nDestPos ] = aEntry;
    }
    else
        aError = ERR_RSCCONT;

    return aError;
}

ERRTYPE RscBaseCont::SetPosRscId( const RSCINST & rInst, sal_uInt32 nPos,
                                  const RscId & rId )
{
    RscBaseContInst * pClassData;
    RSCINST           aTmpI;
    ERRTYPE           aError;

    pClassData = reinterpret_cast<RscBaseContInst *>(rInst.pData + nOffInstData);

    if( nPos < pClassData->nEntries )
    {
        if( ! (rId == pClassData->pEntries[ nPos ].aName) )
            aTmpI = SearchEle( rInst, rId,
                               pClassData->pEntries[ nPos ].aInst.pClass );
        if( !aTmpI.IsInst() )
            pClassData->pEntries[ nPos ].aName = rId;
        else
            aError = ERR_RSCCONT;
    }
    else
        aError = ERR_RSCCONT;

    return aError;
}

SUBINFO_STRUCT RscBaseCont::GetInfoEle( const RSCINST & rInst, sal_uInt32 nPos )
{
    RscBaseContInst * pClassData;
    SUBINFO_STRUCT  aInfo;

    pClassData = reinterpret_cast<RscBaseContInst *>(rInst.pData + nOffInstData);

    if( nPos < pClassData->nEntries )
    {
        aInfo.aId = pClassData->pEntries[ nPos ].aName;
        aInfo.nPos = nPos;
        aInfo.pClass = pClassData->pEntries[ nPos ].aInst.pClass;
    }
    return aInfo;
}

ERRTYPE RscBaseCont::SetString( const RSCINST & rInst, const char * pStr )
{
    RscBaseContInst * pClassData;
    RSCINST aTmpI;
    ERRTYPE aError;
    char    *pTmpStr;

    pClassData = reinterpret_cast<RscBaseContInst *>(rInst.pData + nOffInstData);

    // otherwise infinite recursion is possible
    if( RSC_NOTYPE == pTypeClass->GetTypId() )
    {
        aError = GetElement( rInst, RscId(), pTypeClass, RSCINST(), &aTmpI );
        aError = aTmpI.pClass->GetString( aTmpI, &pTmpStr );

        if( aError.IsOk() )
            aError = aTmpI.pClass->SetString( aTmpI, pStr );
        else
        {
            aError.Clear();
            DeletePos( rInst, pClassData->nEntries -1 );
            aError = GetElement( rInst, RscId(), pTypeClass1, RSCINST(), &aTmpI );
            aError = aTmpI.pClass->GetString( aTmpI, &pTmpStr );
            if( aError.IsOk() )
                aError = aTmpI.pClass->SetString( aTmpI, pStr );
        }

        if( aError.IsError() )
            DeletePos( rInst, pClassData->nEntries -1 );
    }
    else
        aError = ERR_UNKNOWN_METHOD;

    return aError;
}

ERRTYPE RscBaseCont::SetNumber( const RSCINST & rInst, sal_Int32 lValue )
{
    RscBaseContInst * pClassData;
    RSCINST aTmpI;
    ERRTYPE aError;
    sal_Int32   lNumber;

    pClassData = reinterpret_cast<RscBaseContInst *>(rInst.pData + nOffInstData);

    // otherwise infinite recursion is possible
    if( RSC_NOTYPE == pTypeClass->GetTypId() )
    {
        aError = GetElement( rInst, RscId(), pTypeClass, RSCINST(), &aTmpI );
        aError = aTmpI.pClass->GetNumber( aTmpI, &lNumber );

        if( aError.IsOk() )
            aError = aTmpI.pClass->SetNumber( aTmpI, lValue );
        else
        {
            aError.Clear();
            DeletePos( rInst, pClassData->nEntries -1 );
            aError = GetElement( rInst, RscId(), pTypeClass1, RSCINST(), &aTmpI );
            aError = aTmpI.pClass->GetNumber( aTmpI, &lNumber );
            if( aError.IsOk() )
                aError = aTmpI.pClass->SetNumber( aTmpI, lValue );
        }
        if( aError.IsError() )
            DeletePos( rInst, pClassData->nEntries -1 );
    }
    else
        aError = ERR_UNKNOWN_METHOD;

    return aError;
}

ERRTYPE RscBaseCont::SetBool( const RSCINST & rInst,
                              bool bValue)
{
    RscBaseContInst * pClassData;
    RSCINST aTmpI;
    ERRTYPE aError;
    bool    bBool;

    pClassData = reinterpret_cast<RscBaseContInst *>(rInst.pData + nOffInstData);

    // otherwise infinite recursion is possible
    if( RSC_NOTYPE == pTypeClass->GetTypId() )
    {
        aError = GetElement( rInst, RscId(), pTypeClass, RSCINST(), &aTmpI );
        aError = aTmpI.pClass->GetBool( aTmpI, &bBool );

        if( aError.IsOk() )
            aError = aTmpI.pClass->SetBool( aTmpI, bValue );
        else
        {
            aError.Clear();
            DeletePos( rInst, pClassData->nEntries -1 );
            aError = GetElement( rInst, RscId(), pTypeClass1, RSCINST(), &aTmpI );
            aError = aTmpI.pClass->GetBool( aTmpI, &bBool );
            if( aError.IsOk() )
                aError = aTmpI.pClass->SetBool( aTmpI, bValue );
        }

        if( aError.IsError() )
            DeletePos( rInst, pClassData->nEntries -1 );
    }
    else
        aError = ERR_UNKNOWN_METHOD;

    return aError;
}

ERRTYPE RscBaseCont::SetConst( const RSCINST & rInst,
                               Atom nValueId,
                               sal_Int32 lValue)
{
    RscBaseContInst * pClassData;
    RSCINST aTmpI;
    ERRTYPE aError;
    Atom    nConst;

    pClassData = reinterpret_cast<RscBaseContInst *>(rInst.pData + nOffInstData);

    // otherwise infinite recursion is possible
    if( RSC_NOTYPE == pTypeClass->GetTypId() )
    {
        aError = GetElement( rInst, RscId(), pTypeClass, RSCINST(), &aTmpI );
        aError = aTmpI.pClass->GetConst( aTmpI, &nConst );

        if( aError.IsOk() )
            aError = aTmpI.pClass->SetConst( aTmpI, nValueId, lValue );
        else
        {
            aError.Clear();
            DeletePos( rInst, pClassData->nEntries -1 );
            aError = GetElement( rInst, RscId(), pTypeClass1, RSCINST(), &aTmpI );
            aError = aTmpI.pClass->GetConst( aTmpI, &nConst );
            if( aError.IsOk() )
                aError = aTmpI.pClass->SetConst( aTmpI, nValueId, lValue );
        }

        if( aError.IsError() )
            DeletePos( rInst, pClassData->nEntries -1 );
    }
    else
        aError = ERR_UNKNOWN_METHOD;

    return aError;
}

ERRTYPE RscBaseCont::SetRef( const RSCINST & rInst, const RscId & rRefId )
{
    RscBaseContInst * pClassData;
    RSCINST aTmpI;
    ERRTYPE aError;
    RscId   aId;

    pClassData = reinterpret_cast<RscBaseContInst *>(rInst.pData + nOffInstData);

    // otherwise infinite recursion is possible
    if( RSC_NOTYPE == pTypeClass->GetTypId() )
    {
        aError = GetElement( rInst, RscId(), pTypeClass, RSCINST(), &aTmpI );
        aError = aTmpI.pClass->GetRef( aTmpI, &aId );

        if( aError.IsOk() )
            aError = aTmpI.pClass->SetRef( aTmpI, rRefId );
        else
        {
            aError.Clear();
            DeletePos( rInst, pClassData->nEntries -1 );
            aError = GetElement( rInst, RscId(), pTypeClass1, RSCINST(), &aTmpI );
            aError = aTmpI.pClass->GetRef( aTmpI, &aId );
            if( aError.IsOk() )
                aError = aTmpI.pClass->SetNumber( aTmpI, rRefId );
        }

        if( aError.IsError() )
            DeletePos( rInst, pClassData->nEntries -1 );
    }
    else
        aError = ERR_UNKNOWN_METHOD;

    return aError;
}

bool RscBaseCont::IsConsistent( const RSCINST & rInst )
{
    sal_uInt32  i = 0;
    RscBaseContInst * pClassData;
    bool    bRet;

    bRet = RscTop::IsConsistent( rInst );

    pClassData = reinterpret_cast<RscBaseContInst *>(rInst.pData + nOffInstData);

    // check for duplicate Id and keep order
    // complexity = n^2 / 2
    for( i = 0; i < pClassData->nEntries; i++ )
    {
        if( !bNoId )
        {
            if( (sal_Int32)pClassData->pEntries[ i ].aName > 0x7FFF ||
                (sal_Int32)pClassData->pEntries[ i ].aName < 1 )
            {
                bRet = false;
            }
            else if( SearchElePos( rInst, pClassData->pEntries[ i ].aName,
                                   pClassData->pEntries[ i ].aInst.pClass, i +1 ).IsInst() )
            {
                 bRet = false;
            }
        }
        if( ! pClassData->pEntries[ i ].aInst.pClass->
            IsConsistent( pClassData->pEntries[ i ].aInst ) )
        {
            bRet = false;
        }
    }

    return bRet;
}

void RscBaseCont::SetToDefault( const RSCINST & rInst )
{
    sal_uInt32  i = 0;
    RscBaseContInst * pClassData;

    pClassData = reinterpret_cast<RscBaseContInst *>(rInst.pData + nOffInstData);

    for( i = 0; i < pClassData->nEntries; i++ )
    {
        pClassData->pEntries[ i ].aInst.pClass->
                  SetToDefault( pClassData->pEntries[ i ].aInst );
    }

    RscTop::SetToDefault( rInst );
}

bool RscBaseCont::IsDefault( const RSCINST & rInst )
{
    sal_uInt32  i = 0;
    RscBaseContInst * pClassData;

    pClassData = reinterpret_cast<RscBaseContInst *>(rInst.pData + nOffInstData);

    if( !pClassData->bDflt )
        return false;

    for( i = 0; i < pClassData->nEntries; i++ )
    {
        if( ! pClassData->pEntries[ i ].aInst.pClass->
            IsDefault( pClassData->pEntries[ i ].aInst ) )
        {
            return false;
        }
    }

    return RscTop::IsDefault( rInst );
}

bool RscBaseCont::IsValueDefault( const RSCINST & rInst, CLASS_DATA pDef )
{
    RscBaseContInst * pClassData;

    if( !RscTop::IsValueDefault( rInst, pDef ) )
        return false;

    pClassData = reinterpret_cast<RscBaseContInst *>(rInst.pData + nOffInstData);

    if( pClassData->nEntries )
        return false;
    else
        return true;
}

void RscBaseCont::Delete( const RSCINST & rInst, RscTop * pClass,
                          const RscId & rId )
{
    sal_uInt32  i = 0;
    RscBaseContInst * pClassData;

    pClassData = reinterpret_cast<RscBaseContInst *>(rInst.pData + nOffInstData);
    if( !pClass )
        pClass = pTypeClass;

    for( i = 0; i < pClassData->nEntries; i++ )
    {
        if( pClassData->pEntries[ i ].aName == rId )
        {
            if( pClassData->pEntries[ i ].aInst.pClass == pClass || !pClass )
            {
                DeletePos( rInst, i );
                return;
            }
        }
    }

}

void RscBaseCont::DeletePos( const RSCINST & rInst, sal_uInt32 nPos )
{
    RscBaseContInst *   pClassData;

    pClassData = reinterpret_cast<RscBaseContInst *>(rInst.pData + nOffInstData);

    if( nPos < pClassData->nEntries )
    {
        if( 1 == pClassData->nEntries )
            DestroyElements( pClassData );
        else
        {
            pClassData->pEntries[ nPos ].Destroy();
            pClassData->nEntries--;

            for (sal_uInt32 i = nPos; i < pClassData->nEntries; i++ )
                pClassData->pEntries[ i ] = pClassData->pEntries[ i + 1 ];

        }
    }
}

void RscBaseCont::ContWriteSrc( const RSCINST & rInst, FILE * fOutput,
                                RscTypCont * pTC, sal_uInt32 nTab,
                                const char * pVarName )
{
    sal_uInt32  i = 0, t = 0;
    RscBaseContInst * pClassData;

    pClassData = reinterpret_cast<RscBaseContInst *>(rInst.pData + nOffInstData);

    for( i = 0; i < pClassData->nEntries; i++ )
    {
        for( t = 0; t < nTab; t++ )
            fputc( '\t', fOutput );

        pClassData->pEntries[ i ].aInst.pClass->
                  WriteSrcHeader( pClassData->pEntries[ i ].aInst,
                                  fOutput, pTC, nTab,
                                  pClassData->pEntries[ i ].aName, pVarName );
        fprintf( fOutput, ";\n" );
    }
}

ERRTYPE RscBaseCont::ContWriteRc( const RSCINST & rInst, RscWriteRc & rMem,
                                  RscTypCont * pTC, sal_uInt32 nDeep, bool bExtra )
{
    RscBaseContInst * pClassData;
    ERRTYPE       aError;

    if( bExtra || bNoId )
    { // only write sub resources when bExtra == true
        pClassData = reinterpret_cast<RscBaseContInst *>(rInst.pData + nOffInstData);

        for (sal_uInt32 i = 0; i < pClassData->nEntries && aError.IsOk(); i++ )
        {
            aError = pClassData->pEntries[ i ].aInst.pClass->
                         WriteRcHeader( pClassData->pEntries[ i ].aInst,
                                        rMem, pTC,
                                        pClassData->pEntries[ i ].aName,
                                        nDeep, bExtra );
        }
    }

    return aError ;
}

void RscBaseCont::WriteSrc( const RSCINST & rInst, FILE * fOutput,
                            RscTypCont * pTC, sal_uInt32 nTab,
                            const char * pVarName )
{
    RscTop::WriteSrc( rInst, fOutput, pTC, nTab, pVarName );
    ContWriteSrc( rInst, fOutput, pTC, nTab, pVarName );
}

ERRTYPE RscBaseCont::WriteRc( const RSCINST & rInst, RscWriteRc & rMem,
                              RscTypCont * pTC, sal_uInt32 nDeep, bool bExtra )
{
    ERRTYPE       aError;

    aError = RscTop::WriteRc( rInst, rMem, pTC, nDeep, bExtra );
    if( aError.IsOk() )
        aError = ContWriteRc( rInst, rMem, pTC, nDeep, bExtra );

    return aError;
}

RscContWriteSrc::RscContWriteSrc( Atom nId, sal_uInt32 nTypeId,
                                  RscTop * pSuper, bool bNoIdent )
    : RscBaseCont( nId, nTypeId, pSuper, bNoIdent )
{
}

void RscContWriteSrc::WriteSrc( const RSCINST & rInst, FILE * fOutput,
                                RscTypCont * pTC, sal_uInt32 nTab,
                                const char * pVarName )
{
    sal_uInt32  i;

    RscTop::WriteSrc( rInst, fOutput, pTC, nTab, pVarName );

    fprintf( fOutput, "\n" );
    for( i = 0; i < nTab; i++ )
        fputc( '\t', fOutput );

    fprintf( fOutput, "{\n" );

    ContWriteSrc( rInst, fOutput, pTC, nTab +1, pVarName );

    for( i = 0; i < nTab; i++ )
        fputc( '\t', fOutput );

    fprintf( fOutput, "}" );
}

RscCont::RscCont( Atom nId, sal_uInt32 nTypeId, RscTop * pSuper, bool bNoIdent )
    : RscContWriteSrc( nId, nTypeId, pSuper, bNoIdent )
{
}

ERRTYPE RscCont::WriteRc( const RSCINST & rInst, RscWriteRc & rMem,
                                 RscTypCont * pTC, sal_uInt32 nDeep, bool bExtra )
{
    RscBaseContInst * pClassData;
    ERRTYPE aError;

    aError = RscTop::WriteRc( rInst, rMem, pTC, nDeep, bExtra );

    pClassData = reinterpret_cast<RscBaseContInst *>(rInst.pData + nOffInstData);

    rMem.Put( pClassData->nEntries );

    if( aError.IsOk() )
        aError = ContWriteRc( rInst, rMem, pTC, nDeep, bExtra );

    return aError;
}

RscContExtraData::RscContExtraData( Atom nId, sal_uInt32 nTypeId,
                                    RscTop * pSuper, bool bNoIdent )
    : RscContWriteSrc( nId, nTypeId, pSuper, bNoIdent )
{
}

ERRTYPE RscContExtraData::WriteRc( const RSCINST & rInst, RscWriteRc & rMem,
                                   RscTypCont * pTC, sal_uInt32 nDeep, bool bExtra )
{
    ERRTYPE aError;

    if( bExtra )
        aError = RscContWriteSrc::WriteRc( rInst, rMem, pTC, nDeep, bExtra );
    else
        aError = RscTop::WriteRc( rInst, rMem, pTC, nDeep, bExtra );

    return aError;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
