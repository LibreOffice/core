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

#include <cstdlib>
#include <cstdio>
#include <cstring>

#include <rscconst.hxx>
#include <rscall.h>
#include <rschash.hxx>
#include <tools/resid.hxx>

RscConst::RscConst( Atom nId, sal_uInt32 nTypeId )
    : RscTop( nId, nTypeId )
{
    pVarArray = nullptr;
    nEntries = 0;
}

RscConst::~RscConst()
{
    if( pVarArray )
        rtl_freeMemory( static_cast<void *>(pVarArray) );
}

RSCCLASS_TYPE RscConst::GetClassType() const
{
    return RSCCLASS_CONST;
}

ERRTYPE RscConst::SetConstant( Atom nVarName, sal_Int32 lValue )
{
    if( pVarArray )
        pVarArray = static_cast<VarEle *>(rtl_reallocateMemory( static_cast<void *>(pVarArray),
                                                     ((nEntries +1) * sizeof( VarEle )) ));
    else
        pVarArray = static_cast<VarEle *>(rtl_allocateMemory( ((nEntries +1) * sizeof( VarEle )) ));
    pVarArray[ nEntries ].nId     = nVarName;
    pVarArray[ nEntries ].lValue  = lValue;
    nEntries++;

    return ERR_OK;
}

bool RscConst::GetConstValue( Atom nConst, sal_Int32 * pValue ) const
{
    sal_uInt32 i = 0;

    for( i = 0; i < nEntries; i++ )
    {
        if( pVarArray[ i ].nId == nConst )
        {
            *pValue = pVarArray[ i ].lValue;
            return true;
        }
    }
    return false;
}

bool RscConst::GetValueConst( sal_Int32 lValue, Atom * pConst ) const
{
    sal_uInt32 i = 0;

    for( i = 0; i < nEntries; i++ )
    {
        if( pVarArray[ i ].lValue == lValue )
        {
            *pConst = pVarArray[ i ].nId;
            return true;
        }
    }
    return false;
}

sal_uInt32 RscConst::GetConstPos( Atom nConst )
{
    sal_uInt32 i = 0;

    for( i = 0; i < nEntries; i++ )
    {
        if( pVarArray[ i ].nId == nConst )
            return i;
    }

    return nEntries;
}

RscEnum::RscEnum( Atom nId, sal_uInt32 nTypeId )
    : RscConst( nId, nTypeId )
{
    nSize = ALIGNED_SIZE( sizeof( RscEnumInst ) );
}

ERRTYPE RscEnum::SetConst( const RSCINST & rInst, Atom nConst, sal_Int32 /*nVal*/ )
{
    sal_uInt32 i = 0;

    if( nEntries != (i = GetConstPos( nConst )) )
    {
        reinterpret_cast<RscEnumInst *>(rInst.pData)->nValue = i;
        reinterpret_cast<RscEnumInst *>(rInst.pData)->bDflt = false;
        return ERR_OK;
    }

    return ERR_RSCENUM;
}

ERRTYPE RscEnum::SetNumber( const RSCINST & rInst, sal_Int32 lValue )
{
    sal_uInt32  i = 0;

    for( i = 0; i < nEntries; i++ )
    {
        if( (sal_Int32)pVarArray[ i ].lValue == lValue )
            return SetConst( rInst, pVarArray[ i ].nId, lValue );
    }

    return ERR_RSCENUM;
}

ERRTYPE RscEnum::GetConst( const RSCINST & rInst, Atom * pH )
{
    *pH = pVarArray[ reinterpret_cast<RscEnumInst *>(rInst.pData)->nValue ].nId;
    return ERR_OK;
}

ERRTYPE RscEnum::GetNumber( const RSCINST & rInst, sal_Int32 * pNumber ){
    *pNumber = pVarArray[ reinterpret_cast<RscEnumInst *>(rInst.pData)->nValue ].lValue;
    return ERR_OK;
}

RSCINST RscEnum::Create( RSCINST * pInst, const RSCINST & rDflt, bool bOwnClass )
{
    RSCINST aInst;

    if( !pInst )
    {
        aInst.pClass = this;
        aInst.pData = static_cast<CLASS_DATA>(
                      rtl_allocateMemory( sizeof( RscEnumInst ) ));
    }
    else
        aInst = *pInst;

    if( !bOwnClass && rDflt.IsInst() )
        bOwnClass = rDflt.pClass->InHierarchy( this );

    if( bOwnClass )
        memmove( aInst.pData, rDflt.pData, Size() );
    else
    {
        reinterpret_cast<RscEnumInst *>(aInst.pData)->nValue = 0;
        reinterpret_cast<RscEnumInst *>(aInst.pData)->bDflt = true;
    }

    return aInst;
}

bool RscEnum::IsValueDefault( const RSCINST & rInst, CLASS_DATA pDef )
{
    return pDef && (reinterpret_cast<RscEnumInst*>(rInst.pData)->nValue == reinterpret_cast<RscEnumInst*>(pDef)->nValue );
}

void RscEnum::WriteSrc( const RSCINST & rInst, FILE * fOutput,
                        RscTypCont *, sal_uInt32, const char * )
{
    fprintf( fOutput, "%s",
             pHS->getString( pVarArray[ reinterpret_cast<RscEnumInst *>(rInst.pData)->nValue ].nId ).getStr() );
}

ERRTYPE RscEnum::WriteRc( const RSCINST & rInst, RscWriteRc & aMem,
                          RscTypCont *, sal_uInt32, bool )
{
    aMem.Put( (sal_Int32)pVarArray[ reinterpret_cast<RscEnumInst *>(rInst.pData)->nValue ].lValue );
    return ERR_OK;
}

RscLangEnum::RscLangEnum()
    : RscEnum( pHS->getID( "LangEnum" ), RSC_NOTYPE ),
      mnLangId( 0x400 )
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
