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

#include <rscrange.hxx>

RscRange::RscRange( Atom nId, sal_uInt32 nTypeId )
                        : RscTop( nId, nTypeId )
{
    nMin = nMax = 0;
    nSize = ALIGNED_SIZE( sizeof( RscRangeInst ) );
}

RSCCLASS_TYPE RscRange::GetClassType() const
{
    return RSCCLASS_NUMBER;
}

ERRTYPE RscRange::SetRange( sal_Int32 nMinimum, sal_Int32 nMaximum )
{
    if( nMinimum > nMaximum )
    {
        nMin = nMaximum;
        nMax = nMinimum;
    }
    else
    {
        nMax = nMaximum;
        nMin = nMinimum;
    };

    return( ERR_OK );
}

sal_Bool RscRange::IsValueDefault( const RSCINST & rInst, CLASS_DATA pDef )
{
    if( pDef )
    {
        if( ((RscRangeInst*)rInst.pData)->nValue ==
          ((RscRangeInst*)pDef)->nValue )
        {
            return sal_True;
        }
    }

    return sal_False;
}

ERRTYPE RscRange::SetNumber( const RSCINST & rInst, sal_Int32 nValue )
{
    if( nMax < nValue || nMin > nValue )
        return( ERR_RSCRANGE_OUTDEFSET );
    ((RscRangeInst *)rInst.pData)->nValue = (sal_uInt16)( nValue - nMin );
    ((RscRangeInst *)rInst.pData)->bDflt = sal_False;
    return( ERR_OK );
}

ERRTYPE RscRange::GetNumber( const RSCINST & rInst, sal_Int32 * pN )
{
    *pN = ((RscRangeInst *)rInst.pData)->nValue + nMin;
    return( ERR_OK );
}

RSCINST RscRange::Create( RSCINST * pInst, const RSCINST & rDflt,
                            sal_Bool bOwnClass )
{
    RSCINST aInst;

    if( !pInst )
    {
        aInst.pClass = this;
        aInst.pData = (CLASS_DATA)
                      rtl_allocateMemory( sizeof( RscRangeInst ) );
    }
    else
        aInst = *pInst;
    if( !bOwnClass && rDflt.IsInst() )
        bOwnClass = rDflt.pClass->InHierarchy( this );

    if( bOwnClass )
        memmove( aInst.pData, rDflt.pData, sizeof( RscRangeInst ) );
    else
    {
        if( 0L >= nMin && 0L <= nMax )
            ((RscRangeInst *)aInst.pData)->nValue = (sal_uInt16)(0L - nMin);
        else
            ((RscRangeInst *)aInst.pData)->nValue = 0;
        ((RscRangeInst *)aInst.pData)->bDflt = sal_True;
    }

    return( aInst );
}

void RscRange::WriteSrc( const RSCINST & rInst, FILE * fOutput,
                         RscTypCont *, sal_uInt32, const char * )
{
    fprintf( fOutput, "%ld", long( ((RscRangeInst *)rInst.pData)->nValue + nMin ) );
}

ERRTYPE RscRange::WriteRc( const RSCINST & rInst, RscWriteRc & aMem,
                           RscTypCont *, sal_uInt32, sal_Bool )
{
    if( nMin >= 0 )
    {
        sal_uInt16 n;
        n = (sal_uInt16)(((RscRangeInst *)rInst.pData)->nValue + nMin);
        aMem.Put( n );
    }
    else
    {
        sal_Int16 n;
        n = (sal_Int16)(((RscRangeInst *)rInst.pData)->nValue + nMin);
        aMem.Put( n );
    }

    return( ERR_OK );
}

RscLongRange::RscLongRange( Atom nId, sal_uInt32 nTypeId )
                        : RscTop( nId, nTypeId )
{
    nMin = nMax = 0;
    nSize = ALIGNED_SIZE( sizeof( RscLongRangeInst ) );
}

RSCCLASS_TYPE RscLongRange::GetClassType() const
{
    return RSCCLASS_NUMBER;
}

ERRTYPE RscLongRange::SetRange( sal_Int32 nMinimum, sal_Int32 nMaximum )
{
    if( nMinimum > nMaximum )
    {
        nMin = nMaximum;
        nMax = nMinimum;
    }
    else
    {
        nMax = nMaximum;
        nMin = nMinimum;
    };

    return( ERR_OK );
}

sal_Bool RscLongRange::IsValueDefault( const RSCINST & rInst, CLASS_DATA pDef )
{
    if( pDef )
        return 0 == memcmp( &((RscLongRangeInst*)rInst.pData)->nValue,
                            &((RscLongRangeInst*)pDef)->nValue,
                            sizeof( sal_Int32 ) );

    return sal_False;
}

ERRTYPE RscLongRange::SetNumber( const RSCINST & rInst, sal_Int32 nValue )
{
    if( nMax < nValue || nMin > nValue )
        return( ERR_RSCRANGE_OUTDEFSET );
    void * pData = &((RscLongRangeInst*)rInst.pData)->nValue;
    memcpy( pData, &nValue, sizeof( sal_Int32 ) );
    ((RscLongRangeInst *)rInst.pData)->bDflt = sal_False;
    return( ERR_OK );
}

ERRTYPE RscLongRange::GetNumber( const RSCINST & rInst, sal_Int32 * pN )
{
    memmove( pN, &((RscLongRangeInst*)rInst.pData)->nValue,
             sizeof( sal_Int32 ) );
    return( ERR_OK );
}

RSCINST RscLongRange::Create( RSCINST * pInst, const RSCINST & rDflt,
                              sal_Bool bOwnClass )
{
    RSCINST aInst;

    if( !pInst )
    {
        aInst.pClass = this;
        aInst.pData = (CLASS_DATA)
                      rtl_allocateMemory( sizeof( RscLongRangeInst ) );
    }
    else
        aInst = *pInst;
    if( !bOwnClass && rDflt.IsInst() )
        bOwnClass = rDflt.pClass->InHierarchy( this );

    if( bOwnClass )
        memmove( aInst.pData, rDflt.pData, sizeof( RscLongRangeInst ) );
    else
    {
        sal_Int32   lDflt;
        if( 0L >= nMin && 0L <= nMax )
            lDflt = 0;
        else
            lDflt = nMin;
        void * pData = &((RscLongRangeInst*)aInst.pData)->nValue;
        memcpy( pData, &lDflt, sizeof( sal_Int32 ) );
        ((RscLongRangeInst *)aInst.pData)->bDflt = sal_True;
    }

    return( aInst );
}

void RscLongRange::WriteSrc( const RSCINST & rInst, FILE * fOutput,
                         RscTypCont *, sal_uInt32, const char * )
{
    sal_Int32 lVal;
    GetNumber( rInst, &lVal );
    fprintf( fOutput, "%d", static_cast<int>(lVal) );
}

ERRTYPE RscLongRange::WriteRc( const RSCINST & rInst, RscWriteRc & aMem,
                               RscTypCont *, sal_uInt32, sal_Bool )
{
    sal_Int32 lVal;

    GetNumber( rInst, &lVal );
    aMem.Put( (sal_Int32)lVal );

    return( ERR_OK );
}

RscLongEnumRange::RscLongEnumRange( Atom nId, sal_uInt32 nTypeId )
                        : RscLongRange( nId, nTypeId )
{
}

ERRTYPE RscLongEnumRange::SetConst( const RSCINST & rInst, Atom /*nConst*/,
                                    sal_Int32 nValue )
{
    return SetNumber( rInst, nValue );
}

RscIdRange::RscIdRange( Atom nId, sal_uInt32 nTypeId )
            : RscTop( nId, nTypeId )
{
    nSize = ALIGNED_SIZE( sizeof( RscId ) );
    nMin = nMax = 0;
}

RSCCLASS_TYPE RscIdRange::GetClassType() const
{
    return RSCCLASS_NUMBER;
}

RSCINST RscIdRange::Create( RSCINST * pInst, const RSCINST & rDflt, sal_Bool bOwnClass ){
    RSCINST aInst;
    RscId * pClassData;

    if( !pInst ){
        aInst.pClass = this;
        aInst.pData = (CLASS_DATA)rtl_allocateMemory( sizeof( RscId ) );
    }
    else
        aInst = *pInst;


    if( !bOwnClass && rDflt.IsInst() )
        bOwnClass = rDflt.pClass->InHierarchy( this );

    pClassData = (RscId *)aInst.pData;

    pClassData->Create();
    if( bOwnClass )
        *pClassData = *(RscId *)rDflt.pData;
    else{
            *pClassData = RscId();
        if( 0 >= nMin && 0 <= nMax )
            *pClassData = RscId( (sal_Int32)0 );
        else
            *pClassData = RscId( nMin );
        //cUnused wird fuer Defaultkennung verwendet
        ((RscId *)aInst.pData)->aExp.cUnused = sal_True;
    }

    return( aInst );
}

void RscIdRange :: Destroy( const RSCINST & rInst ){
    ((RscId *)rInst.pData)->Destroy();
}

sal_Bool RscIdRange::IsValueDefault( const RSCINST & rInst, CLASS_DATA pDef ){
    if( pDef ){
        if( ((RscId*)rInst.pData)->aExp.IsNumber()
          && ((RscId*)pDef)->aExp.IsNumber() )
        {
            if( ((RscId*)rInst.pData)->GetNumber() ==
              ((RscId*)pDef)->GetNumber() )
            {
                return sal_True;
            }
        }
    }

    return sal_False;
}

ERRTYPE RscIdRange::SetNumber( const RSCINST & rInst, sal_Int32 nValue )
{
    if( nMax < nValue || nMin > nValue )
        return( ERR_RSCRANGE_OUTDEFSET );

    *(RscId *)rInst.pData = RscId( nValue );
    ((RscId *)rInst.pData)->aExp.cUnused = sal_False;
    return( ERR_OK );
}

ERRTYPE RscIdRange::GetNumber( const RSCINST & rInst, sal_Int32 * plValue ){
    *plValue = ((RscId *)rInst.pData)->GetNumber();
    return( ERR_OK );
}

ERRTYPE RscIdRange::SetRef( const RSCINST & rInst, const RscId & rRscId ){
    ERRTYPE aError;
    if( rRscId.IsId() ){
        aError = SetNumber( rInst, rRscId );
        if( aError.IsOk() ){
            *(RscId *)rInst.pData = rRscId;
            ((RscId *)rInst.pData)->aExp.cUnused = sal_False;
        }
    }
    else
        aError = ERR_RSCRANGE_OUTDEFSET;

    return( aError );
}

ERRTYPE RscIdRange::GetRef( const RSCINST & rInst, RscId * pRscId ){
    *pRscId = *(RscId *)rInst.pData;

    return( ERR_OK );
}

void RscIdRange::WriteSrc( const RSCINST & rInst, FILE * fOutput,
                           RscTypCont *, sal_uInt32, const char * )
{
    fprintf( fOutput, "%s", ((RscId *)rInst.pData)->GetName().getStr() );
}

ERRTYPE RscIdRange::WriteRc( const RSCINST & rInst, RscWriteRc & aMem,
                             RscTypCont *, sal_uInt32, sal_Bool )
{
    sal_Int32 lVal = ((RscId*)rInst.pData)->GetNumber();

    aMem.Put( (sal_Int32)lVal );

    return( ERR_OK );
}

sal_Bool RscIdRange::IsConsistent( const RSCINST & rInst )
{
    long nValue = ((RscId *)rInst.pData)->GetNumber();
    if( (nMax >= nValue) && (nMin <= nValue) )
        return sal_True;
    else {
        return sal_False;
    }
}

RscBool::RscBool( Atom nId, sal_uInt32 nTypeId )
        : RscRange( nId, nTypeId )
{
    RscRange::SetRange( 0, 1 );
}

RSCCLASS_TYPE  RscBool::GetClassType() const
{
    return RSCCLASS_BOOL;
}

void RscBool::WriteSrc( const RSCINST & rInst, FILE * fOutput,
                        RscTypCont *, sal_uInt32, const char * )
{
    sal_Int32 l;

    GetNumber( rInst, &l );
    if( l )
        fprintf( fOutput, "TRUE" );
    else
        fprintf( fOutput, "FALSE" );
}

RscBreakRange :: RscBreakRange( Atom nId, sal_uInt32 nTypeId )
                        : RscRange( nId, nTypeId )
{
    nOutRange = 0xFFFFFFFF;
}

ERRTYPE RscBreakRange::SetNumber( const RSCINST & rInst, sal_Int32 nValue ){
    if( nValue == nOutRange )
        return( ERR_RSCRANGE_OUTDEFSET );
    else
        return( RscRange::SetNumber( rInst, nValue ) );
}

RSCINST RscBreakRange::Create( RSCINST * pInst, const RSCINST & rDflt,
                               sal_Bool bOwnClass )
{
    RSCINST aInst;
    sal_Int32   l;

    aInst = RscRange::Create( pInst, rDflt, bOwnClass );

    GetNumber( aInst, &l );
    if( l == nOutRange )
        ((RscRangeInst *)aInst.pData)->nValue++;

    return( aInst );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
