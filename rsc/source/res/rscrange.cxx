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
/****************** I N C L U D E S **************************************/

// C and C++ Includes.
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Solar Definitionen
#include <tools/solar.h>

// Programmabh�ngige Includes.
#include <rscrange.hxx>

/****************** D E F I N E S ****************************************/
/****************** C O D E **********************************************/
/****************** R s c R a n g e **************************************/
/*************************************************************************
|*
|*    RscRange::RscRange()
|*
*************************************************************************/
RscRange::RscRange( Atom nId, sal_uInt32 nTypeId )
                        : RscTop( nId, nTypeId )
{
    nMin = nMax = 0;
    nSize = ALIGNED_SIZE( sizeof( RscRangeInst ) );
}

/*************************************************************************
|*
|*    RscRange::GetClassType()
|*
*************************************************************************/
RSCCLASS_TYPE RscRange::GetClassType() const
{
    return RSCCLASS_NUMBER;
}

/*************************************************************************
|*
|*    RscRange::SetRange()
|*
*************************************************************************/
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

/*************************************************************************
|*
|*    RscRange::IsValueDefault()
|*
*************************************************************************/
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

/*************************************************************************
|*
|*    RscRange::SetNumber()
|*
*************************************************************************/
ERRTYPE RscRange::SetNumber( const RSCINST & rInst, sal_Int32 nValue )
{
    if( nMax < nValue || nMin > nValue )
        return( ERR_RSCRANGE_OUTDEFSET );
    ((RscRangeInst *)rInst.pData)->nValue = (sal_uInt16)( nValue - nMin );
    ((RscRangeInst *)rInst.pData)->bDflt = sal_False;
    return( ERR_OK );
}

/*************************************************************************
|*
|*    RscRange::GetNumber()
|*
*************************************************************************/
ERRTYPE RscRange::GetNumber( const RSCINST & rInst, sal_Int32 * pN )
{
    *pN = ((RscRangeInst *)rInst.pData)->nValue + nMin;
    return( ERR_OK );
}

/*************************************************************************
|*
|*    RscRange::Create()
|*
*************************************************************************/
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

/*************************************************************************
|*
|*    RscRange::WriteSrc()
|*
*************************************************************************/
void RscRange::WriteSrc( const RSCINST & rInst, FILE * fOutput,
                         RscTypCont *, sal_uInt32, const char * )
{
    fprintf( fOutput, "%ld", long( ((RscRangeInst *)rInst.pData)->nValue + nMin ) );
}

/*************************************************************************
|*
|*    RscRange::WriteRc()
|*
*************************************************************************/
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

//=======================================================================
void RscRange::WriteRcAccess
(
    FILE * fOutput,
    RscTypCont * /*pTC*/,
    const char * pName
)
{
    fprintf( fOutput, "\t\tSet%s( ", pName );
    if( nMin >= 0 )
        fprintf( fOutput, "*(sal_uInt32 *)(pResData+nOffset) );\n" );
    else
        fprintf( fOutput, "*(sal_Int32 *)(pResData+nOffset) );\n" );
    fprintf( fOutput, "\t\tnOffset += sizeof( sal_uInt32 );\n" );
}

/****************** R s c L o n g R a n g e ******************************/
/*************************************************************************
|*
|*    RscLongRange::RscLongRange()
|*
*************************************************************************/
RscLongRange::RscLongRange( Atom nId, sal_uInt32 nTypeId )
                        : RscTop( nId, nTypeId )
{
    nMin = nMax = 0;
    nSize = ALIGNED_SIZE( sizeof( RscLongRangeInst ) );
}

/*************************************************************************
|*
|*    RscLongRange::GetClassType()
|*
*************************************************************************/
RSCCLASS_TYPE RscLongRange::GetClassType() const
{
    return RSCCLASS_NUMBER;
}

/*************************************************************************
|*
|*    RscLongRange::SetRange()
|*
*************************************************************************/
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

/*************************************************************************
|*
|*    RscLongRange::IsValueDefault()
|*
*************************************************************************/
sal_Bool RscLongRange::IsValueDefault( const RSCINST & rInst, CLASS_DATA pDef )
{
    if( pDef )
        return 0 == memcmp( &((RscLongRangeInst*)rInst.pData)->nValue,
                            &((RscLongRangeInst*)pDef)->nValue,
                            sizeof( sal_Int32 ) );

    return sal_False;
}

/*************************************************************************
|*
|*    RscLongRange::SetNumber()
|*
*************************************************************************/
ERRTYPE RscLongRange::SetNumber( const RSCINST & rInst, sal_Int32 nValue )
{
    if( nMax < nValue || nMin > nValue )
        return( ERR_RSCRANGE_OUTDEFSET );
    void * pData = &((RscLongRangeInst*)rInst.pData)->nValue;
    memmove( pData, &nValue, sizeof( sal_Int32 ) );
    ((RscLongRangeInst *)rInst.pData)->bDflt = sal_False;
    return( ERR_OK );
}

/*************************************************************************
|*
|*    RscLongRange::GetNumber()
|*
*************************************************************************/
ERRTYPE RscLongRange::GetNumber( const RSCINST & rInst, sal_Int32 * pN )
{
    memmove( pN, &((RscLongRangeInst*)rInst.pData)->nValue,
             sizeof( sal_Int32 ) );
    return( ERR_OK );
}

/*************************************************************************
|*
|*    RscLongRange::Create()
|*
*************************************************************************/
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
        memmove( pData, &lDflt, sizeof( sal_Int32 ) );
        ((RscLongRangeInst *)aInst.pData)->bDflt = sal_True;
    }

    return( aInst );
}

/*************************************************************************
|*
|*    RscLongRange::WriteSrc()
|*
*************************************************************************/
void RscLongRange::WriteSrc( const RSCINST & rInst, FILE * fOutput,
                         RscTypCont *, sal_uInt32, const char * )
{
    sal_Int32 lVal;
    GetNumber( rInst, &lVal );
    fprintf( fOutput, "%d", static_cast<int>(lVal) );
}

/*************************************************************************
|*
|*    RscLongRange::WriteRc()
|*
*************************************************************************/
ERRTYPE RscLongRange::WriteRc( const RSCINST & rInst, RscWriteRc & aMem,
                               RscTypCont *, sal_uInt32, sal_Bool )
{
    sal_Int32 lVal;

    GetNumber( rInst, &lVal );
    aMem.Put( (sal_Int32)lVal );

    return( ERR_OK );
}

//=======================================================================
void RscLongRange::WriteRcAccess
(
    FILE * fOutput,
    RscTypCont * /*pTC*/,
    const char * pName
)
{
    fprintf( fOutput, "\t\tSet%s( ", pName );
    fprintf( fOutput, "GetLong( pResData+nOffset ) );\n" );
    fprintf( fOutput, "\t\tnOffset += sizeof( sal_Int32 );\n" );
}

/****************** R s c L o n g E n u m R a n g e *********************/
/*************************************************************************
|*    RscLongEnumRange::RscLongEnumRange()
|*
|*    Beschreibung
*************************************************************************/
RscLongEnumRange::RscLongEnumRange( Atom nId, sal_uInt32 nTypeId )
                        : RscLongRange( nId, nTypeId )
{
}

/*************************************************************************
|*    RscLongEnumRange::SetConst()
|*
|*    Beschreibung
*************************************************************************/
ERRTYPE RscLongEnumRange::SetConst( const RSCINST & rInst, Atom /*nConst*/,
                                    sal_Int32 nValue )
{
    return SetNumber( rInst, nValue );
}

/****************** R s c I d R a n g e **********************************/
/*************************************************************************
|*
|*    RscIdRange::RscIdRange()
|*
*************************************************************************/
RscIdRange::RscIdRange( Atom nId, sal_uInt32 nTypeId )
            : RscTop( nId, nTypeId )
{
    nSize = ALIGNED_SIZE( sizeof( RscId ) );
    nMin = nMax = 0;
}

/*************************************************************************
|*
|*    RscIdRange::RscIdRange()
|*
*************************************************************************/
RSCCLASS_TYPE RscIdRange::GetClassType() const
{
    return RSCCLASS_NUMBER;
}

/*************************************************************************
|*
|*    RscIdRange::Create()
|*
*************************************************************************/
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

/*************************************************************************
|*
|*    RscIdRange::Destroy()
|*
*************************************************************************/
void RscIdRange :: Destroy( const RSCINST & rInst ){
    ((RscId *)rInst.pData)->Destroy();
}

/*************************************************************************
|*
|*    RscIdRange::IsValueDefault()
|*
*************************************************************************/
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

/*************************************************************************
|*
|*    RscIdRange::SetNumber()
|*
*************************************************************************/
ERRTYPE RscIdRange::SetNumber( const RSCINST & rInst, sal_Int32 nValue )
{
    if( nMax < nValue || nMin > nValue )
        return( ERR_RSCRANGE_OUTDEFSET );

    *(RscId *)rInst.pData = RscId( nValue );
    ((RscId *)rInst.pData)->aExp.cUnused = sal_False;
    return( ERR_OK );
}

/*************************************************************************
|*
|*    RscIdRange::GetNumber()
|*
*************************************************************************/
ERRTYPE RscIdRange::GetNumber( const RSCINST & rInst, sal_Int32 * plValue ){
    *plValue = ((RscId *)rInst.pData)->GetNumber();
    return( ERR_OK );
}

/*************************************************************************
|*
|*    RscIdRange::SetRef()
|*
*************************************************************************/
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

/*************************************************************************
|*
|*    RscIdRange::GetRef()
|*
*************************************************************************/
ERRTYPE RscIdRange::GetRef( const RSCINST & rInst, RscId * pRscId ){
    *pRscId = *(RscId *)rInst.pData;

    return( ERR_OK );
}

/*************************************************************************
|*
|*    RscIdRange::WriteSrc()
|*
*************************************************************************/
void RscIdRange::WriteSrc( const RSCINST & rInst, FILE * fOutput,
                           RscTypCont *, sal_uInt32, const char * )
{
    fprintf( fOutput, "%s", ((RscId *)rInst.pData)->GetName().GetBuffer() );
}

/*************************************************************************
|*
|*    RscIdRange::WriteRc()
|*
*************************************************************************/
ERRTYPE RscIdRange::WriteRc( const RSCINST & rInst, RscWriteRc & aMem,
                             RscTypCont *, sal_uInt32, sal_Bool )
{
    sal_Int32 lVal = ((RscId*)rInst.pData)->GetNumber();

    aMem.Put( (sal_Int32)lVal );

    return( ERR_OK );
}

//=======================================================================
void RscIdRange::WriteRcAccess
(
    FILE * fOutput,
    RscTypCont * /*pTC*/,
    const char * pName
)
{
    fprintf( fOutput, "\t\tSet%s( ", pName );
    fprintf( fOutput, "GetLong( pResData+nOffset ) );\n" );
    fprintf( fOutput, "\t\tnOffset += sizeof( sal_Int32 );\n" );
}

/*************************************************************************
|*
|*    RscIdRange::IsConsistent()
|*
*************************************************************************/
sal_Bool RscIdRange::IsConsistent( const RSCINST & rInst )
{
    long nValue = ((RscId *)rInst.pData)->GetNumber();
    if( (nMax >= nValue) && (nMin <= nValue) )
        return sal_True;
    else {
        return sal_False;
    }
}

/****************** R s c B o o l ****************************************/
/*************************************************************************
|*
|*    RscBool::RscBool()
|*
*************************************************************************/
RscBool::RscBool( Atom nId, sal_uInt32 nTypeId )
        : RscRange( nId, nTypeId )
{
    RscRange::SetRange( 0, 1 );
}

/*************************************************************************
|*
|*    RscBool::GetClassType()
|*
*************************************************************************/
RSCCLASS_TYPE  RscBool::GetClassType() const
{
    return RSCCLASS_BOOL;
}

/*************************************************************************
|*
|*    RscBool::WriteSrc()
|*
*************************************************************************/
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

//=======================================================================
void RscBool::WriteRcAccess
(
    FILE * fOutput,
    RscTypCont * /*pTC*/,
    const char * pName
)
{
    fprintf( fOutput, "\t\tSet%s( ", pName );
    fprintf( fOutput, "(sal_Bool)*(short *)(pResData+nOffset) );\n" );
    fprintf( fOutput, "\t\tnOffset += sizeof( short );\n" );
}

/****************** R s c B r e a k R a n g e ****************************/
/*************************************************************************
|*
|*    RscBreakRange::SetNumber()
|*
*************************************************************************/
RscBreakRange :: RscBreakRange( Atom nId, sal_uInt32 nTypeId )
                        : RscRange( nId, nTypeId )
{
    nOutRange = 0xFFFFFFFF;
}

/*************************************************************************
|*
|*    RscBreakRange::SetNumber()
|*
*************************************************************************/
ERRTYPE RscBreakRange::SetNumber( const RSCINST & rInst, sal_Int32 nValue ){
    if( nValue == nOutRange )
        return( ERR_RSCRANGE_OUTDEFSET );
    else
        return( RscRange::SetNumber( rInst, nValue ) );
}

/*************************************************************************
|*
|*    RscBreakRange::Create()
|*
*************************************************************************/
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
