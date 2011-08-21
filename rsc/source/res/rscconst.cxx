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
#include <cstdlib>
#include <cstdio>
#include <cstring>

// Solar Definitionen
#include <tools/solar.h>

// Programmabhaengige Includes.
#include <rscconst.hxx>
#include <rscall.h>
#include <rschash.hxx>
#include <tools/resid.hxx>

/****************** C O D E **********************************************/
/****************** R s c C o n s t **************************************/
/*************************************************************************
|*
|*    RscConst::RscConst()
|*
*************************************************************************/
RscConst::RscConst( Atom nId, sal_uInt32 nTypeId )
    : RscTop( nId, nTypeId )
{
    pVarArray = NULL;
    nEntries = 0;
}

/*************************************************************************
|*
|*    RscConst::~RscConst()
|*
*************************************************************************/
RscConst::~RscConst()
{
    if( pVarArray )
        rtl_freeMemory( (void *)pVarArray );
}

/*************************************************************************
|*
|*    RscConst::GetClassType()
|*
*************************************************************************/
RSCCLASS_TYPE RscConst::GetClassType() const
{
    return RSCCLASS_CONST;
}

/*************************************************************************
|*
|*    RscConst::SetConstance()
|*
*************************************************************************/
ERRTYPE RscConst::SetConstant( Atom nVarName, sal_Int32 lValue ){
    if( pVarArray )
        pVarArray = (VarEle *)
            rtl_reallocateMemory( (void *)pVarArray,
                ((nEntries +1) * sizeof( VarEle )) );
    else
        pVarArray = (VarEle *)
            rtl_allocateMemory( ((nEntries +1) * sizeof( VarEle )) );
    pVarArray[ nEntries ].nId     = nVarName;
    pVarArray[ nEntries ].lValue  = lValue;
    nEntries++;

    return( ERR_OK );
}

/*************************************************************************
|*
|*    RscConst::GetConstance()
|*
*************************************************************************/
Atom RscConst::GetConstant( sal_uInt32 nPos ){
     if( nPos < nEntries )
        return pVarArray[ nPos ].nId;
    return( InvalidAtom );
}

/*************************************************************************
|*
|*    RscConst::GetConstValue()
|*
*************************************************************************/
sal_Bool RscConst::GetConstValue( Atom nConst, sal_Int32 * pValue ) const
{
    sal_uInt32 i = 0;

    for( i = 0; i < nEntries; i++ )
        if( pVarArray[ i ].nId == nConst )
        {
            *pValue = pVarArray[ i ].lValue;
            return sal_True;
        }
    return sal_False;
}

/*************************************************************************
|*
|*    RscConst::GetValueConst()
|*
*************************************************************************/
sal_Bool RscConst::GetValueConst( sal_Int32 lValue, Atom * pConst ) const
{
    sal_uInt32 i = 0;

    for( i = 0; i < nEntries; i++ )
        if( pVarArray[ i ].lValue == lValue )
        {
            *pConst = pVarArray[ i ].nId;
            return sal_True;
        }
    return sal_False;
}

/*************************************************************************
|*
|*    RscConst::GetConstPos()
|*
|*    Beschreibung      Sucht die Position der Konstanten
|*                      Return = nEntries, nicht gefunden
|*                      Return = Position im Feld
|*
*************************************************************************/
sal_uInt32 RscConst::GetConstPos( Atom nConst )
{
    sal_uInt32 i = 0;

    for( i = 0; i < nEntries; i++ )
    {
        if( pVarArray[ i ].nId == nConst )
            return( i );
    }

    return( nEntries );
}

/*************************************************************************
|*
|*    RscEnum::WriteSyntax()
|*
*************************************************************************/
void RscConst::WriteSyntax( FILE * fOutput, RscTypCont * pTC )
{
    RscTop::WriteSyntax( fOutput, pTC );

    sal_uInt32 i = 0;
    // Wenn eine Variable Maskierung hat, dann Maskenfeld
    fprintf( fOutput, "\t" );
    for( i = 0; i < nEntries; i++ )
    {
        fprintf( fOutput, "%s, ", pHS->getString( pVarArray[ i ].nId ).getStr() );
        if( 3 == (i % 4) && i < sal_uInt32(nEntries -1) )
            fprintf( fOutput, "\n\t" );
    };
    fprintf( fOutput, "\n" );
}

//==================================================================
void RscConst::WriteRcAccess
(
    FILE * fOutput,
    RscTypCont * /*pTC*/,
    const char * pName
)
{
    fprintf( fOutput, "\t\tSet%s( %s( ", pName, pHS->getString( GetId() ).getStr() );
    fprintf( fOutput, "*(short*)(pResData+nOffset) ) );\n" );
    fprintf( fOutput, "\t\tnOffset += sizeof( short );\n" );
}

/****************** R s c E n u m ****************************************/
/*************************************************************************
|*
|*    RscEnum::RscEnum()
|*
*************************************************************************/
RscEnum::RscEnum( Atom nId, sal_uInt32 nTypeId )
            : RscConst( nId, nTypeId )
{
    nSize = ALIGNED_SIZE( sizeof( RscEnumInst ) );
}

/*************************************************************************
|*
|*    RscEnum::SetConst()
|*
*************************************************************************/
ERRTYPE RscEnum::SetConst( const RSCINST & rInst, Atom nConst, sal_Int32 /*nVal*/ )
{
    sal_uInt32 i = 0;

    if( nEntries != (i = GetConstPos( nConst )) )
    {
        ((RscEnumInst *)rInst.pData)->nValue = i;
        ((RscEnumInst *)rInst.pData)->bDflt = sal_False;
        return( ERR_OK );
    };

    return( ERR_RSCENUM );
}

/*************************************************************************
|*
|*    RscEnum::SetNumber()
|*
*************************************************************************/
ERRTYPE RscEnum::SetNumber( const RSCINST & rInst, sal_Int32 lValue )
{
    sal_uInt32  i = 0;

    for( i = 0; i < nEntries; i++ ){
        if( (sal_Int32)pVarArray[ i ].lValue == lValue )
            return( SetConst( rInst, pVarArray[ i ].nId, lValue ) );
    };

    return( ERR_RSCENUM );
}

/*************************************************************************
|*
|*    RscEnum::GetConst()
|*
*************************************************************************/
ERRTYPE RscEnum::GetConst( const RSCINST & rInst, Atom * pH ){
    *pH = pVarArray[ ((RscEnumInst *)rInst.pData)->nValue ].nId;
    return( ERR_OK );
}

/*************************************************************************
|*
|*    RscEnum::GetNumber()
|*
*************************************************************************/
ERRTYPE RscEnum::GetNumber( const RSCINST & rInst, sal_Int32 * pNumber ){
    *pNumber = pVarArray[ ((RscEnumInst *)rInst.pData)->nValue ].lValue;
    return( ERR_OK );
}

/*************************************************************************
|*
|*    RscEnum::Create()
|*
*************************************************************************/
RSCINST RscEnum::Create( RSCINST * pInst, const RSCINST & rDflt, sal_Bool bOwnClass ){
    RSCINST aInst;

    if( !pInst ){
        aInst.pClass = this;
        aInst.pData = (CLASS_DATA)
                      rtl_allocateMemory( sizeof( RscEnumInst ) );
    }
    else
        aInst = *pInst;
    if( !bOwnClass && rDflt.IsInst() )
        bOwnClass = rDflt.pClass->InHierarchy( this );

    if( bOwnClass )
        memmove( aInst.pData, rDflt.pData, Size() );
    else{
        ((RscEnumInst *)aInst.pData)->nValue = 0;
        ((RscEnumInst *)aInst.pData)->bDflt = sal_True;
    }

    return( aInst );
}

/*************************************************************************
|*
|*    RscEnum::IsValueDefault()
|*
*************************************************************************/
sal_Bool RscEnum::IsValueDefault( const RSCINST & rInst, CLASS_DATA pDef ){
    if( pDef ){
        if( ((RscEnumInst*)rInst.pData)->nValue ==
          ((RscEnumInst*)pDef)->nValue )
        {
            return sal_True;
        }
    }

    return sal_False;
}

/*************************************************************************
|*
|*    RscEnum::WriteSrc()
|*
*************************************************************************/
void RscEnum::WriteSrc( const RSCINST & rInst, FILE * fOutput,
                         RscTypCont *, sal_uInt32, const char * )
{
    fprintf( fOutput, "%s", pHS->getString(
             pVarArray[ ((RscEnumInst *)rInst.pData)->nValue ].nId ).getStr() );
}

/*************************************************************************
|*
|*    RscEnum::WriteRc()
|*
*************************************************************************/
ERRTYPE RscEnum::WriteRc( const RSCINST & rInst, RscWriteRc & aMem,
                          RscTypCont *, sal_uInt32, sal_Bool )
{
    aMem.Put( (sal_Int32)pVarArray[ ((RscEnumInst *)rInst.pData)->nValue ].lValue );
    return( ERR_OK );
}

RscLangEnum::RscLangEnum()
        : RscEnum( pHS->getID( "LangEnum" ), RSC_NOTYPE ),
          mnLangId( 0x400 )
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
