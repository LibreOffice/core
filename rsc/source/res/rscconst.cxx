/*************************************************************************
 *
 *  $RCSfile: rscconst.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:42:55 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
/************************************************************************

    Source Code Control System - Header

    $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/rsc/source/res/rscconst.cxx,v 1.1.1.1 2000-09-18 16:42:55 hr Exp $

**************************************************************************/
/****************** I N C L U D E S **************************************/

// C and C++ Includes.
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Solar Definitionen
#include <tools/solar.h>

// Programmabh„ngige Includes.
#ifndef _RSCCONST_HXX
#include <rscconst.hxx>
#endif

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
RscConst::RscConst( HASHID nId, USHORT nTypeId )
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
        RscMem::Free( (void *)pVarArray );
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
ERRTYPE RscConst::SetConstant( HASHID nVarName, INT32 lValue ){
    if( pVarArray )
        pVarArray = (VarEle *)
            RscMem::Realloc( (void *)pVarArray,
                (USHORT)((nEntries +1) * sizeof( VarEle )) );
    else
        pVarArray = (VarEle *)
            RscMem::Malloc( (USHORT)((nEntries +1) * sizeof( VarEle )) );
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
HASHID RscConst::GetConstant( USHORT nPos ){
     if( nPos < nEntries )
        return pVarArray[ nPos ].nId;
    return( HASH_NONAME );
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
BOOL RscConst::GetConstValue( HASHID nConst, INT32 * pValue ) const
{
    USHORT i;

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
BOOL RscConst::GetValueConst( INT32 lValue, HASHID * pConst ) const
{
    USHORT i;

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
USHORT RscConst::GetConstPos( HASHID nConst )
{
    USHORT i;

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

    USHORT i;
    // Wenn eine Variable Maskierung hat, dann Maskenfeld
    fprintf( fOutput, "\t" );
    for( i = 0; i < nEntries; i++ )
    {
        fprintf( fOutput, "%s, ", pHS->Get( pVarArray[ i ].nId ) );
        if( 3 == (i % 4) && i < (USHORT)(nEntries -1) )
            fprintf( fOutput, "\n\t" );
    };
    fprintf( fOutput, "\n" );
}

//==================================================================
void RscConst::WriteRcAccess
(
    FILE * fOutput,
    RscTypCont * pTC,
    const char * pName
)
{
    fprintf( fOutput, "\t\tSet%s( %s( ", pName, pHS->Get( GetId() ) );
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
RscEnum::RscEnum( HASHID nId, USHORT nTypeId, BOOL bUSHORT_ )
            : RscConst( nId, nTypeId )
            , bUSHORT( bUSHORT_ )
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
ERRTYPE RscEnum::SetConst( const RSCINST & rInst, HASHID nConst, long nVal )
{
    USHORT i;

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
ERRTYPE RscEnum::SetNumber( const RSCINST & rInst, INT32 lValue ){
    USHORT i;

    for( i = 0; i < nEntries; i++ ){
        if( (long)pVarArray[ i ].lValue == lValue )
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
ERRTYPE RscEnum::GetConst( const RSCINST & rInst, HASHID * pH ){
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
ERRTYPE RscEnum::GetNumber( const RSCINST & rInst, long * pNumber ){
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
                      RscMem::Malloc( sizeof( RscEnumInst ) );
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
                         RscTypCont *, USHORT, const char * )
{
    fprintf( fOutput, "%s", pHS->Get(
             pVarArray[ ((RscEnumInst *)rInst.pData)->nValue ].nId ) );
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
                          RscTypCont *, USHORT, BOOL )
{
    if( bUSHORT )
        aMem.Put( (USHORT)pVarArray[ ((RscEnumInst *)rInst.pData)->nValue ].lValue );
    else
        aMem.Put( (INT32)pVarArray[ ((RscEnumInst *)rInst.pData)->nValue ].lValue );
    return( ERR_OK );
}


