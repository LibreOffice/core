/*************************************************************************
 *
 *  $RCSfile: rscflag.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:42:56 $
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

    $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/rsc/source/res/rscflag.cxx,v 1.1.1.1 2000-09-18 16:42:56 hr Exp $

*************************************************************************/
/****************** I N C L U D E S **************************************/

// C and C++ Includes.
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Programmabh„ngige Includes.
#ifndef _RSCFLAG_HXX
#include <rscflag.hxx>
#endif

/****************** C O D E **********************************************/
/****************** R s c F l a g ****************************************/
/*************************************************************************
|*
|*    RscFlag::RscFlag()
|*
|*    Beschreibung
|*    Ersterstellung    MM 03.04.91
|*    Letzte Aenderung  MM 03.04.91
|*
*************************************************************************/
RscFlag::RscFlag( HASHID nId, USHORT nTypeId, BOOL bUSHORT_ )
            : RscConst( nId, nTypeId )
            , bUSHORT( bUSHORT_ )
{}

/*************************************************************************
|*
|*    RscFlag::Size()
|*
|*    Beschreibung      Die Groeáe der Instanzdaten richtet sich nach
|*                      der Anzahl der Flags
|*    Ersterstellung    MM 03.04.91
|*    Letzte Aenderung  MM 03.04.91
|*
*************************************************************************/
USHORT RscFlag::Size()
{
    return( ALIGNED_SIZE( sizeof( RscFlagInst ) *
            ( 1 + (nEntries -1) / (sizeof( USHORT ) * 8) ) ) );
}

/*************************************************************************
|*
|*    RscFlag::SetNotConst()
|*
|*    Beschreibung
|*    Ersterstellung    MM 03.04.91
|*    Letzte Aenderung  MM 03.04.91
|*
*************************************************************************/
ERRTYPE RscFlag::SetNotConst( const RSCINST & rInst, HASHID nConst ){
    USHORT i, nFlag;

    if( nEntries != (i = GetConstPos( nConst )) ){
        nFlag = 1 << (i % (sizeof( USHORT ) * 8) );
        i = i / (sizeof( USHORT ) * 8);
        ((RscFlagInst *)rInst.pData)[ i ].nFlags     &= ~nFlag;
        ((RscFlagInst *)rInst.pData)[ i ].nDfltFlags &= ~nFlag;
        return( ERR_OK );
    };

    return( ERR_RSCFLAG );
}

/*************************************************************************
|*
|*    RscFlag::SetConst()
|*
|*    Beschreibung
|*    Ersterstellung    MM 03.04.91
|*    Letzte Aenderung  MM 03.04.91
|*
*************************************************************************/
ERRTYPE RscFlag::SetConst( const RSCINST & rInst, HASHID nConst, long nVal )
{
    USHORT i, nFlag;

    if( nEntries != (i = GetConstPos( nConst )) ){
        nFlag = 1 << (i % (sizeof( USHORT ) * 8) );
        i = i / (sizeof( USHORT ) * 8);
        ((RscFlagInst *)rInst.pData)[ i ].nFlags     |= nFlag;
        ((RscFlagInst *)rInst.pData)[ i ].nDfltFlags &= ~nFlag;
        return( ERR_OK );
    };

    return( ERR_RSCFLAG );
}

/*************************************************************************
|*
|*    RscFlag::CreateBasic()
|*
|*    Beschreibung
|*    Ersterstellung    MM 16.01.92
|*    Letzte Aenderung  MM 16.01.92
|*
*************************************************************************/
RSCINST RscFlag::CreateBasic( RSCINST * pInst )
{
    RSCINST aInst;

    if( !pInst ){
        aInst.pClass = this;
        aInst.pData = (CLASS_DATA) RscMem::Malloc( Size() );
    }
    else
        aInst = *pInst;

    return( aInst );
}

/*************************************************************************
|*
|*    RscFlag::Create()
|*
|*    Beschreibung
|*    Ersterstellung    MM 03.04.91
|*    Letzte Aenderung  MM 16.01.92
|*
*************************************************************************/
RSCINST RscFlag::Create( RSCINST * pInst, const RSCINST & rDflt, BOOL bOwnClass ){
    RSCINST aInst = CreateBasic( pInst );
    USHORT  i;

    if( !bOwnClass && rDflt.IsInst() )
        bOwnClass = rDflt.pClass->InHierarchy( this );

    if( bOwnClass )
        memmove( aInst.pData, rDflt.pData, Size() );
    else
    {
        for( i = 0; i < Size() / sizeof( RscFlagInst ); i++ )
        {
            ((RscFlagInst *)aInst.pData)[ i ].nFlags = 0;
            ((RscFlagInst *)aInst.pData)[ i ].nDfltFlags = 0xFFFF;
        }
    };

    return( aInst );
}

/*************************************************************************
|*
|*    RscFlag::CreateClient()
|*
|*    Beschreibung
|*    Ersterstellung    MM 16.01.92
|*    Letzte Aenderung  MM 16.01.92
|*
*************************************************************************/
RSCINST RscFlag::CreateClient( RSCINST * pInst, const RSCINST & rDfltI,
                               BOOL bOwnClass, HASHID nConstId )
{
    RSCINST aInst = CreateBasic( pInst );
    USHORT i, nFlag;

    if( !bOwnClass && rDfltI.IsInst() )
        bOwnClass = rDfltI.pClass->InHierarchy( this );

    if( nEntries != (i = GetConstPos( nConstId )) ){
        nFlag = 1 << (i % (sizeof( USHORT ) * 8) );
        i = i / (sizeof( USHORT ) * 8);
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

/*************************************************************************
|*
|*    RscFlag::SetToDefault()
|*
|*    Beschreibung
|*    Ersterstellung    MM 03.04.91
|*    Letzte Aenderung  MM 03.04.91
|*
*************************************************************************/
void RscFlag::SetToDefault( const RSCINST & rInst )
{
    USHORT i;

    for( i = 0; i < Size() / sizeof( RscFlagInst ); i++ )
        ((RscFlagInst *)rInst.pData)[ i ].nDfltFlags = 0xFFFF;
}

/*************************************************************************
|*
|*    RscFlag::IsDlft()
|*
|*    Beschreibung
|*    Ersterstellung    MM 03.04.91
|*    Letzte Aenderung  MM 03.04.91
|*
*************************************************************************/
BOOL RscFlag::IsDefault( const RSCINST & rInst )
{
    USHORT i;

    for( i = 0; i < Size() / sizeof( RscFlagInst ); i++ )
        if( ((RscFlagInst *)rInst.pData)[ i ].nDfltFlags != 0xFFFF )
            return( FALSE );
    return( TRUE );
}

BOOL RscFlag::IsDefault( const RSCINST & rInst, HASHID nConstId )
{
    USHORT i, nFlag;

    if( nEntries != (i = GetConstPos( nConstId )) ){
        nFlag = 1 << (i % (sizeof( USHORT ) * 8) );
        i = i / (sizeof( USHORT ) * 8);
        if( ((RscFlagInst *)rInst.pData)[ i ].nDfltFlags & nFlag )
            return( TRUE );
        else
            return( FALSE );
    };
    return( TRUE );
}

/*************************************************************************
|*
|*    RscFlag::IsValueDefault()
|*
|*    Beschreibung
|*    Ersterstellung    MM 25.04.91
|*    Letzte Aenderung  MM 25.04.91
|*
*************************************************************************/
BOOL RscFlag::IsValueDefault( const RSCINST & rInst, CLASS_DATA pDef,
                              HASHID nConstId )
{
    USHORT i, nFlag;

    if( nEntries != (i = GetConstPos( nConstId )) ){
        nFlag = 1 << (i % (sizeof( USHORT ) * 8) );
        i = i / (sizeof( USHORT ) * 8);

        if( pDef ){
            if( (((RscFlagInst *)rInst.pData)[ i ].nFlags & nFlag)
              == (((RscFlagInst *)pDef)[ i ].nFlags & nFlag) )
            {
                return TRUE;
            }
        }
    };

    return FALSE;
}

BOOL RscFlag::IsValueDefault( const RSCINST & rInst, CLASS_DATA pDef ){
    USHORT i;

    if( pDef ){
        USHORT  Flag, nIndex;

        Flag = 1;
        for( i = 0; i < nEntries; i++ ){
            nIndex = i / (sizeof( USHORT ) * 8);
            if( (((RscFlagInst *)rInst.pData)[ nIndex ].nFlags & Flag)
              != (((RscFlagInst *)pDef)[ nIndex ].nFlags & Flag)  )
            {
                return FALSE;
            }
            Flag <<= 1;
            if( !Flag )
                Flag = 1;
        };
    }
    else
        return FALSE;

    return TRUE;
}

/*************************************************************************
|*
|*    RscFlag::IsSet()
|*
|*    Beschreibung
|*    Ersterstellung    MM 10.04.91
|*    Letzte Aenderung  MM 10.04.91
|*
*************************************************************************/
BOOL RscFlag::IsSet( const RSCINST & rInst, HASHID nConstId ){
    USHORT i, nFlag;

    if( nEntries != (i = GetConstPos( nConstId )) ){
        nFlag = 1 << (i % (sizeof( USHORT ) * 8) );
        i = i / (sizeof( USHORT ) * 8);
        if( ((RscFlagInst *)rInst.pData)[ i ].nFlags & nFlag )
            return( TRUE );
        else
            return( FALSE );
    };
    return( TRUE );
}

/*************************************************************************
|*
|*    RscFlag::WriteSrc()
|*
|*    Beschreibung
|*    Ersterstellung    MM 08.04.91
|*    Letzte Aenderung  MM 08.04.91
|*
*************************************************************************/
void RscFlag::WriteSrc( const RSCINST & rInst, FILE * fOutput,
                        RscTypCont *, USHORT, const char * )
{
    USHORT  i, Flag, nIndex;
    BOOL    bComma = FALSE;

    Flag = 1;
    for( i = 0; i < nEntries; i++ ){
        nIndex = i / (sizeof( USHORT ) * 8);
        if( !( ((RscFlagInst *)rInst.pData)[ nIndex ].nDfltFlags & Flag) ){
            if( bComma )
                fprintf( fOutput, ", " );
            if( ((RscFlagInst *)rInst.pData)[ nIndex ].nFlags & Flag )
                fprintf( fOutput, "%s", pHS->Get( pVarArray[ i ].nId ) );
            else{
                fprintf( fOutput, "not " );
                fprintf( fOutput, "%s", pHS->Get( pVarArray[ i ].nId ) );
            }
            bComma = TRUE;
        }
        Flag <<= 1;
        if( !Flag )
            Flag = 1;
    };
}

/*************************************************************************
|*
|*    RscFlag::WriteRc()
|*
|*    Beschreibung
|*    Ersterstellung    MM 15.04.91
|*    Letzte Aenderung  MM 15.04.91
|*
*************************************************************************/
ERRTYPE RscFlag::WriteRc( const RSCINST & rInst, RscWriteRc & aMem,
                          RscTypCont *, USHORT, BOOL )
{
    INT32   lVal = 0;
    USHORT  i, Flag, nIndex;

    Flag = 1;
    for( i = 0; i < nEntries; i++ ){
        nIndex = i / (sizeof( USHORT ) * 8);
        if( ((RscFlagInst *)rInst.pData)[ nIndex ].nFlags & Flag )
            lVal |= pVarArray[ i ].lValue;

        Flag <<= 1;
        if( !Flag )
            Flag = 1;
    };

    if( bUSHORT )
        aMem.Put( (USHORT)lVal );
    else
        aMem.Put( (INT32)lVal );
    return( ERR_OK );
}

/*************************************************************************
|*
|*    RscClient::RscClient()
|*
|*    Beschreibung
|*    Ersterstellung    MM 08.04.91
|*    Letzte Aenderung  MM 08.04.91
|*
*************************************************************************/
RscClient::RscClient( HASHID nId, USHORT nTypeId, RscFlag * pClass,
                       HASHID nConstantId )
           : RscTop ( nId, nTypeId )
{
   pRefClass = pClass;
   nConstId = nConstantId;
}

/*************************************************************************
|*
|*    RscClient::GetClassType()
|*
|*    Beschreibung
|*    Ersterstellung    MM 08.04.91
|*    Letzte Aenderung  MM 08.04.91
|*
*************************************************************************/
RSCCLASS_TYPE RscClient::GetClassType() const
{
    return RSCCLASS_BOOL;
}

/*************************************************************************
|*
|*    RscClient::WriteSrc()
|*
|*    Beschreibung
|*    Ersterstellung    MM 08.04.91
|*    Letzte Aenderung  MM 08.04.91
|*
*************************************************************************/
void RscClient::WriteSrc( const RSCINST & rInst, FILE * fOutput,
                          RscTypCont *, USHORT, const char * )
{
    if( pRefClass->IsSet( rInst, nConstId ) )
        fprintf( fOutput, "TRUE" );
    else
        fprintf( fOutput, "FALSE" );
}

/*************************************************************************
|*
|*    RscClient::Create()
|*
|*    Beschreibung
|*    Ersterstellung    MM 08.04.91
|*    Letzte Aenderung  MM 08.04.91
|*
*************************************************************************/
RSCINST RscClient::Create( RSCINST * pInst, const RSCINST & rDflt,
                           BOOL bOwnClass )
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

