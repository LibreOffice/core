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

#include <rscflag.hxx>

RscFlag::RscFlag( Atom nId, sal_uInt32 nTypeId )
            : RscConst( nId, nTypeId )
{}

sal_uInt32 RscFlag::Size()
{
    return( ALIGNED_SIZE( sizeof( RscFlagInst ) *
            ( 1 + (nEntries -1) / (sizeof( sal_uInt32 ) * 8) ) ) );
}

ERRTYPE RscFlag::SetNotConst( const RSCINST & rInst, Atom nConst )
{
    sal_uInt32 i = 0;

    if( nEntries != (i = GetConstPos( nConst )) ){
        sal_uInt32 nFlag = 1 << (i % (sizeof( sal_uInt32 ) * 8) );
        i = i / (sizeof( sal_uInt32 ) * 8);
        ((RscFlagInst *)rInst.pData)[ i ].nFlags     &= ~nFlag;
        ((RscFlagInst *)rInst.pData)[ i ].nDfltFlags &= ~nFlag;
        return( ERR_OK );
    };

    return( ERR_RSCFLAG );
}

ERRTYPE RscFlag::SetConst( const RSCINST & rInst, Atom nConst, sal_Int32 /*nVal*/ )
{
    sal_uInt32 i = 0;

    if( nEntries != (i = GetConstPos( nConst )) ){
        sal_uInt32 nFlag = 1 << (i % (sizeof( sal_uInt32 ) * 8) );
        i = i / (sizeof( sal_uInt32 ) * 8);
        ((RscFlagInst *)rInst.pData)[ i ].nFlags     |= nFlag;
        ((RscFlagInst *)rInst.pData)[ i ].nDfltFlags &= ~nFlag;
        return( ERR_OK );
    };

    return( ERR_RSCFLAG );
}

RSCINST RscFlag::CreateBasic( RSCINST * pInst )
{
    RSCINST aInst;

    if( !pInst ){
        aInst.pClass = this;
        aInst.pData = (CLASS_DATA) rtl_allocateMemory( Size() );
    }
    else
        aInst = *pInst;

    return( aInst );
}

RSCINST RscFlag::Create( RSCINST * pInst, const RSCINST & rDflt, sal_Bool bOwnClass )
{
    RSCINST aInst = CreateBasic( pInst );

    if( !bOwnClass && rDflt.IsInst() )
        bOwnClass = rDflt.pClass->InHierarchy( this );

    if( bOwnClass )
        memmove( aInst.pData, rDflt.pData, Size() );
    else
    {
        for( sal_uInt32 i = 0; i < Size() / sizeof( RscFlagInst ); i++ )
        {
            ((RscFlagInst *)aInst.pData)[ i ].nFlags = 0;
            ((RscFlagInst *)aInst.pData)[ i ].nDfltFlags = 0xFFFFFFFF;
        }
    };

    return( aInst );
}

RSCINST RscFlag::CreateClient( RSCINST * pInst, const RSCINST & rDfltI,
                               sal_Bool bOwnClass, Atom nConstId )
{
    RSCINST aInst = CreateBasic( pInst );
    sal_uInt32 i = 0;

    if( !bOwnClass && rDfltI.IsInst() )
        bOwnClass = rDfltI.pClass->InHierarchy( this );

    if( nEntries != (i = GetConstPos( nConstId )) )
    {
        sal_uInt32 nFlag = 1 << (i % (sizeof( sal_uInt32 ) * 8) );
        i = i / (sizeof( sal_uInt32 ) * 8);
        if( bOwnClass ){
            ((RscFlagInst *)aInst.pData)[ i ].nFlags &=
            ~nFlag | ((RscFlagInst *)rDfltI.pData)[ i ].nFlags;
            ((RscFlagInst *)aInst.pData)[ i ].nDfltFlags &=
            ~nFlag | ((RscFlagInst *)rDfltI.pData)[ i ].nDfltFlags;
        }
        else{
            ((RscFlagInst *)aInst.pData)[ i ].nFlags &= ~nFlag;
            ((RscFlagInst *)aInst.pData)[ i ].nDfltFlags |= nFlag;
        }
    }

    return( aInst );
}

void RscFlag::SetToDefault( const RSCINST & rInst )
{
    sal_uInt32 i = 0;

    for( i = 0; i < Size() / sizeof( RscFlagInst ); i++ )
        ((RscFlagInst *)rInst.pData)[ i ].nDfltFlags = 0xFFFFFFFF;
}

sal_Bool RscFlag::IsDefault( const RSCINST & rInst )
{
    sal_uInt32 i = 0;

    for( i = 0; i < Size() / sizeof( RscFlagInst ); i++ )
        if( ((RscFlagInst *)rInst.pData)[ i ].nDfltFlags != 0xFFFFFFFF )
            return( sal_False );
    return( sal_True );
}

sal_Bool RscFlag::IsDefault( const RSCINST & rInst, Atom nConstId )
{
    sal_uInt32 i = 0, nFlag = 0;

    if( nEntries != (i = GetConstPos( nConstId )) ){
        nFlag = 1 << (i % (sizeof( sal_uInt32 ) * 8) );
        i = i / (sizeof( sal_uInt32 ) * 8);
        if( ((RscFlagInst *)rInst.pData)[ i ].nDfltFlags & nFlag )
            return( sal_True );
        else
            return( sal_False );
    };
    return( sal_True );
}

sal_Bool RscFlag::IsValueDefault( const RSCINST & rInst, CLASS_DATA pDef,
                              Atom nConstId )
{
    sal_uInt32 i = 0, nFlag = 0;

    if( nEntries != (i = GetConstPos( nConstId )) ){
        nFlag = 1 << (i % (sizeof( sal_uInt32 ) * 8) );
        i = i / (sizeof( sal_uInt32 ) * 8);

        if( pDef ){
            if( (((RscFlagInst *)rInst.pData)[ i ].nFlags & nFlag)
              == (((RscFlagInst *)pDef)[ i ].nFlags & nFlag) )
            {
                return sal_True;
            }
        }
    };

    return sal_False;
}

sal_Bool RscFlag::IsValueDefault( const RSCINST & rInst, CLASS_DATA pDef )
{
    if( pDef ){
        sal_uInt32  Flag = 0, nIndex = 0;

        Flag = 1;
        for( sal_uInt32 i = 0; i < nEntries; i++ ){
            nIndex = i / (sizeof( sal_uInt32 ) * 8);
            if( (((RscFlagInst *)rInst.pData)[ nIndex ].nFlags & Flag)
              != (((RscFlagInst *)pDef)[ nIndex ].nFlags & Flag)  )
            {
                return sal_False;
            }
            Flag <<= 1;
            if( !Flag )
                Flag = 1;
        };
    }
    else
        return sal_False;

    return sal_True;
}

sal_Bool RscFlag::IsSet( const RSCINST & rInst, Atom nConstId )
{
    sal_uInt32 i = 0, nFlag = 0;

    if( nEntries != (i = GetConstPos( nConstId )) ){
        nFlag = 1 << (i % (sizeof( sal_uInt32 ) * 8) );
        i = i / (sizeof( sal_uInt32 ) * 8);
        if( ((RscFlagInst *)rInst.pData)[ i ].nFlags & nFlag )
            return( sal_True );
        else
            return( sal_False );
    };
    return( sal_True );
}

void RscFlag::WriteSrc( const RSCINST & rInst, FILE * fOutput,
                        RscTypCont *, sal_uInt32, const char * )
{
    sal_uInt32  i = 0, Flag = 0, nIndex = 0;
    sal_Bool    bComma = sal_False;

    Flag = 1;
    for( i = 0; i < nEntries; i++ ){
        nIndex = i / (sizeof( sal_uInt32 ) * 8);
        if( !( ((RscFlagInst *)rInst.pData)[ nIndex ].nDfltFlags & Flag) ){
            if( bComma )
                fprintf( fOutput, ", " );
            if( ((RscFlagInst *)rInst.pData)[ nIndex ].nFlags & Flag )
                fprintf( fOutput, "%s", pHS->getString( pVarArray[ i ].nId ).getStr() );
            else{
                fprintf( fOutput, "not " );
                fprintf( fOutput, "%s", pHS->getString( pVarArray[ i ].nId ).getStr() );
            }
            bComma = sal_True;
        }
        Flag <<= 1;
        if( !Flag )
            Flag = 1;
    };
}

ERRTYPE RscFlag::WriteRc( const RSCINST & rInst, RscWriteRc & aMem,
                          RscTypCont *, sal_uInt32, sal_Bool )
{
    sal_Int32   lVal = 0;
    sal_uInt32  i = 0, Flag = 0, nIndex = 0;

    Flag = 1;
    for( i = 0; i < nEntries; i++ ){
        nIndex = i / (sizeof( sal_uInt32 ) * 8);
        if( ((RscFlagInst *)rInst.pData)[ nIndex ].nFlags & Flag )
            lVal |= pVarArray[ i ].lValue;

        Flag <<= 1;
        if( !Flag )
            Flag = 1;
    };

    aMem.Put( (sal_Int32)lVal );
    return( ERR_OK );
}

RscClient::RscClient( Atom nId, sal_uInt32 nTypeId, RscFlag * pClass,
                       Atom nConstantId )
           : RscTop ( nId, nTypeId )
{
   pRefClass = pClass;
   nConstId = nConstantId;
}

RSCCLASS_TYPE RscClient::GetClassType() const
{
    return RSCCLASS_BOOL;
}

void RscClient::WriteSrc( const RSCINST & rInst, FILE * fOutput,
                          RscTypCont *, sal_uInt32, const char * )
{
    if( pRefClass->IsSet( rInst, nConstId ) )
        fprintf( fOutput, "TRUE" );
    else
        fprintf( fOutput, "FALSE" );
}

RSCINST RscClient::Create( RSCINST * pInst, const RSCINST & rDflt,
                           sal_Bool bOwnClass )
{
    RSCINST aTmpI, aDfltI;

    if( pInst ){
        aTmpI.pClass = pRefClass;
        aTmpI.pData  = pInst->pData;
    }

    if( !bOwnClass && rDflt.IsInst() ){
        bOwnClass = rDflt.pClass->InHierarchy( this );
        if( bOwnClass ){
            aDfltI.pClass = pRefClass;
            aDfltI.pData = rDflt.pData;
        }
    }

    if( pInst )
        aTmpI = pRefClass->CreateClient( &aTmpI, aDfltI,
                                         bOwnClass, nConstId );
    else
        aTmpI = pRefClass->CreateClient( NULL, aDfltI,
                                         bOwnClass, nConstId );
    aTmpI.pClass = this;

    return( aTmpI );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
