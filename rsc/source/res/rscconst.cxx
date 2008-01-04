/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: rscconst.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: obo $ $Date: 2008-01-04 15:59:46 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
#ifndef _RSCCONST_HXX
#include <rscconst.hxx>
#endif
#include <rscall.h>
#include <rschash.hxx>
#include <tools/resid.hxx>

/****************** C O D E **********************************************/
/****************** R s c C o n s t **************************************/
/*************************************************************************
|*
|*    RscConst::RscConst()
|*
|*    Beschreibung
|*    Ersterstellung    MM 03.05.91
|*    Letzte Aenderung  MM 03.05.91
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
|*    Beschreibung
|*    Ersterstellung    MM 03.05.91
|*    Letzte Aenderung  MM 03.05.91
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
|*    Beschreibung
|*    Ersterstellung    MM 03.05.91
|*    Letzte Aenderung  MM 03.05.91
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
|*    Beschreibung
|*    Ersterstellung    MM 03.04.91
|*    Letzte Aenderung  MM 03.04.91
|*
*************************************************************************/
ERRTYPE RscConst::SetConstant( Atom nVarName, INT32 lValue ){
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
|*    Beschreibung
|*    Ersterstellung    MM 15.05.91
|*    Letzte Aenderung  MM 15.05.91
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
|*    Beschreibung
|*    Ersterstellung    MM 15.05.91
|*    Letzte Aenderung  MM 15.05.91
|*
*************************************************************************/
BOOL RscConst::GetConstValue( Atom nConst, INT32 * pValue ) const
{
    sal_uInt32 i = 0;

    for( i = 0; i < nEntries; i++ )
        if( pVarArray[ i ].nId == nConst )
        {
            *pValue = pVarArray[ i ].lValue;
            return TRUE;
        }
    return FALSE;
}

/*************************************************************************
|*
|*    RscConst::GetValueConst()
|*
|*    Beschreibung
|*    Ersterstellung    MM 15.05.91
|*    Letzte Aenderung  MM 15.05.91
|*
*************************************************************************/
BOOL RscConst::GetValueConst( INT32 lValue, Atom * pConst ) const
{
    sal_uInt32 i = 0;

    for( i = 0; i < nEntries; i++ )
        if( pVarArray[ i ].lValue == lValue )
        {
            *pConst = pVarArray[ i ].nId;
            return TRUE;
        }
    return FALSE;
}

/*************************************************************************
|*
|*    RscConst::GetConstPos()
|*
|*    Beschreibung      Sucht die Position der Konstanten
|*                      Return = nEntries, nicht gefunden
|*                      Return = Position im Feld
|*    Ersterstellung    MM 03.04.91
|*    Letzte Aenderung  MM 03.04.91
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
|*    Beschreibung
|*    Ersterstellung    MM 29.05.91
|*    Letzte Aenderung  MM 29.05.91
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
|*    Beschreibung
|*    Ersterstellung    MM 03.04.91
|*    Letzte Aenderung  MM 03.04.91
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
|*    Beschreibung
|*    Ersterstellung    MM 03.04.91
|*    Letzte Aenderung  MM 03.04.91
|*
*************************************************************************/
ERRTYPE RscEnum::SetConst( const RSCINST & rInst, Atom nConst, INT32 /*nVal*/ )
{
    sal_uInt32 i = 0;

    if( nEntries != (i = GetConstPos( nConst )) )
    {
        ((RscEnumInst *)rInst.pData)->nValue = i;
        ((RscEnumInst *)rInst.pData)->bDflt = FALSE;
        return( ERR_OK );
    };

    return( ERR_RSCENUM );
}

/*************************************************************************
|*
|*    RscEnum::SetNumber()
|*
|*    Beschreibung
|*    Ersterstellung    MM 18.07.91
|*    Letzte Aenderung  MM 18.07.91
|*
*************************************************************************/
ERRTYPE RscEnum::SetNumber( const RSCINST & rInst, INT32 lValue )
{
    sal_uInt32  i = 0;

    for( i = 0; i < nEntries; i++ ){
        if( (INT32)pVarArray[ i ].lValue == lValue )
            return( SetConst( rInst, pVarArray[ i ].nId, lValue ) );
    };

    return( ERR_RSCENUM );
}

/*************************************************************************
|*
|*    RscEnum::GetConst()
|*
|*    Beschreibung
|*    Ersterstellung    MM 03.04.91
|*    Letzte Aenderung  MM 03.04.91
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
|*    Beschreibung
|*    Ersterstellung    MM 16.09.91
|*    Letzte Aenderung  MM 16.09.91
|*
*************************************************************************/
ERRTYPE RscEnum::GetNumber( const RSCINST & rInst, INT32 * pNumber ){
    *pNumber = pVarArray[ ((RscEnumInst *)rInst.pData)->nValue ].lValue;
    return( ERR_OK );
}

/*************************************************************************
|*
|*    RscEnum::Create()
|*
|*    Beschreibung
|*    Ersterstellung    MM 03.04.91
|*    Letzte Aenderung  MM 03.04.91
|*
*************************************************************************/
RSCINST RscEnum::Create( RSCINST * pInst, const RSCINST & rDflt, BOOL bOwnClass ){
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
        ((RscEnumInst *)aInst.pData)->bDflt = TRUE;
    }

    return( aInst );
}

/*************************************************************************
|*
|*    RscEnum::IsValueDefault()
|*
|*    Beschreibung
|*    Ersterstellung    MM 15.01.92
|*    Letzte Aenderung  MM 15.01.92
|*
*************************************************************************/
BOOL RscEnum::IsValueDefault( const RSCINST & rInst, CLASS_DATA pDef ){
    if( pDef ){
        if( ((RscEnumInst*)rInst.pData)->nValue ==
          ((RscEnumInst*)pDef)->nValue )
        {
            return TRUE;
        }
    }

    return FALSE;
}

/*************************************************************************
|*
|*    RscEnum::WriteSrc()
|*
|*    Beschreibung
|*    Ersterstellung    MM 08.04.91
|*    Letzte Aenderung  MM 08.04.91
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
|*    Beschreibung
|*    Ersterstellung    MM 15.04.91
|*    Letzte Aenderung  MM 15.04.91
|*
*************************************************************************/
ERRTYPE RscEnum::WriteRc( const RSCINST & rInst, RscWriteRc & aMem,
                          RscTypCont *, sal_uInt32, BOOL )
{
    aMem.Put( (INT32)pVarArray[ ((RscEnumInst *)rInst.pData)->nValue ].lValue );
    return( ERR_OK );
}

RscLangEnum::RscLangEnum()
        : RscEnum( pHS->getID( "LangEnum" ), RSC_NOTYPE ),
          mnLangId( 0x400 )
{
}
