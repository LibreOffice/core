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

RscLongRange::RscLongRange( Atom nId, RESOURCE_TYPE nTypeId )
    : RscTop( nId, nTypeId )
    , nMin(0), nMax(0)
{
}

void RscLongRange::SetRange( sal_Int32 nMinimum, sal_Int32 nMaximum )
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
    }
}

bool RscLongRange::IsValueDefault( const RSCINST & rInst, CLASS_DATA pDef )
{
    if( pDef )
        return 0 == memcmp( &reinterpret_cast<RscLongRangeInst*>(rInst.pData)->nValue,
                            &reinterpret_cast<RscLongRangeInst*>(pDef)->nValue,
                            sizeof( sal_Int32 ) );

    return false;
}

ERRTYPE RscLongRange::SetNumber( const RSCINST & rInst, sal_Int32 nValue )
{
    if( nMax < nValue || nMin > nValue )
        return ERR_RSCRANGE_OUTDEFSET;

    void * pData = &reinterpret_cast<RscLongRangeInst*>(rInst.pData)->nValue;
    memcpy( pData, &nValue, sizeof( sal_Int32 ) );
    reinterpret_cast<RscLongRangeInst *>(rInst.pData)->bDflt = false;
    return ERR_OK;
}

ERRTYPE RscLongRange::GetNumber( const RSCINST & rInst, sal_Int32 * pN )
{
    memmove( pN, &reinterpret_cast<RscLongRangeInst*>(rInst.pData)->nValue,
             sizeof( sal_Int32 ) );
    return ERR_OK;
}

RSCINST RscLongRange::Create( RSCINST * pInst, const RSCINST & rDflt,
                              bool bOwnClass )
{
    RSCINST aInst;

    if( !pInst )
    {
        aInst.pClass = this;
        aInst.pData = static_cast<CLASS_DATA>(rtl_allocateMemory( sizeof( RscLongRangeInst ) ));
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
        if( 0 >= nMin && 0 <= nMax )
            lDflt = 0;
        else
            lDflt = nMin;

        void * pData = &reinterpret_cast<RscLongRangeInst*>(aInst.pData)->nValue;
        memcpy( pData, &lDflt, sizeof( sal_Int32 ) );
        reinterpret_cast<RscLongRangeInst *>(aInst.pData)->bDflt = true;
    }

    return aInst;
}

void RscLongRange::WriteSrc( const RSCINST & rInst, FILE * fOutput,
                             RscTypCont *, sal_uInt32, const char * )
{
    sal_Int32 lVal;
    GetNumber( rInst, &lVal );
    fprintf( fOutput, "%d", static_cast<int>(lVal) );
}

ERRTYPE RscLongRange::WriteRc( const RSCINST & rInst, RscWriteRc & aMem,
                               RscTypCont *, sal_uInt32 )
{
    sal_Int32 lVal;

    GetNumber( rInst, &lVal );
    aMem.Put( lVal );

    return ERR_OK;
}

RscLongEnumRange::RscLongEnumRange( Atom nId, RESOURCE_TYPE nTypeId )
    : RscLongRange( nId, nTypeId )
{
}

ERRTYPE RscLongEnumRange::SetConst( const RSCINST & rInst, Atom /*nConst*/,
                                    sal_Int32 nValue )
{
    return SetNumber( rInst, nValue );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
