/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
|*    Beschreibung
|*    Ersterstellung    MM 15.05.91
|*    Letzte Aenderung  MM 15.05.91
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
|*    Beschreibung
|*    Ersterstellung    MM 18.07.91
|*    Letzte Aenderung  MM 18.07.91
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
ERRTYPE RscEnum::GetNumber( const RSCINST & rInst, sal_Int32 * pNumber ){
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
|*    Beschreibung
|*    Ersterstellung    MM 15.01.92
|*    Letzte Aenderung  MM 15.01.92
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
