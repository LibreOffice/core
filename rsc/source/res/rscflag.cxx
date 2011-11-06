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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <rscflag.hxx>

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
RscFlag::RscFlag( Atom nId, sal_uInt32 nTypeId )
            : RscConst( nId, nTypeId )
{}

/*************************************************************************
|*
|*    RscFlag::Size()
|*
|*    Beschreibung      Die Groe�e der Instanzdaten richtet sich nach
|*                      der Anzahl der Flags
|*    Ersterstellung    MM 03.04.91
|*    Letzte Aenderung  MM 03.04.91
|*
*************************************************************************/
sal_uInt32 RscFlag::Size()
{
    return( ALIGNED_SIZE( sizeof( RscFlagInst ) *
            ( 1 + (nEntries -1) / (sizeof( sal_uInt32 ) * 8) ) ) );
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
ERRTYPE RscFlag::SetNotConst( const RSCINST & rInst, Atom nConst )
{
    sal_uInt32 i = 0, nFlag = 0;

    if( nEntries != (i = GetConstPos( nConst )) ){
        nFlag = 1 << (i % (sizeof( sal_uInt32 ) * 8) );
        i = i / (sizeof( sal_uInt32 ) * 8);
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
ERRTYPE RscFlag::SetConst( const RSCINST & rInst, Atom nConst, sal_Int32 /*nVal*/ )
{
    sal_uInt32 i = 0, nFlag = 0;

    if( nEntries != (i = GetConstPos( nConst )) ){
        nFlag = 1 << (i % (sizeof( sal_uInt32 ) * 8) );
        i = i / (sizeof( sal_uInt32 ) * 8);
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
        aInst.pData = (CLASS_DATA) rtl_allocateMemory( Size() );
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
RSCINST RscFlag::Create( RSCINST * pInst, const RSCINST & rDflt, sal_Bool bOwnClass )
{
    RSCINST aInst = CreateBasic( pInst );
    sal_uInt32  i = 0;

    if( !bOwnClass && rDflt.IsInst() )
        bOwnClass = rDflt.pClass->InHierarchy( this );

    if( bOwnClass )
        memmove( aInst.pData, rDflt.pData, Size() );
    else
    {
        for( i = 0; i < Size() / sizeof( RscFlagInst ); i++ )
        {
            ((RscFlagInst *)aInst.pData)[ i ].nFlags = 0;
            ((RscFlagInst *)aInst.pData)[ i ].nDfltFlags = 0xFFFFFFFF;
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
                               sal_Bool bOwnClass, Atom nConstId )
{
    RSCINST aInst = CreateBasic( pInst );
    sal_uInt32 i = 0, nFlag = 0;

    if( !bOwnClass && rDfltI.IsInst() )
        bOwnClass = rDfltI.pClass->InHierarchy( this );

    if( nEntries != (i = GetConstPos( nConstId )) ){
        nFlag = 1 << (i % (sizeof( sal_uInt32 ) * 8) );
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
    sal_uInt32 i = 0;

    for( i = 0; i < Size() / sizeof( RscFlagInst ); i++ )
        ((RscFlagInst *)rInst.pData)[ i ].nDfltFlags = 0xFFFFFFFF;
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

/*************************************************************************
|*
|*    RscFlag::IsValueDefault()
|*
|*    Beschreibung
|*    Ersterstellung    MM 25.04.91
|*    Letzte Aenderung  MM 25.04.91
|*
*************************************************************************/
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
    sal_uInt32 i = 0;

    if( pDef ){
        sal_uInt32  Flag = 0, nIndex = 0;

        Flag = 1;
        for( i = 0; i < nEntries; i++ ){
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

/*************************************************************************
|*
|*    RscFlag::IsSet()
|*
|*    Beschreibung
|*    Ersterstellung    MM 10.04.91
|*    Letzte Aenderung  MM 10.04.91
|*
*************************************************************************/
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

/*************************************************************************
|*
|*    RscClient::RscClient()
|*
|*    Beschreibung
|*    Ersterstellung    MM 08.04.91
|*    Letzte Aenderung  MM 08.04.91
|*
*************************************************************************/
RscClient::RscClient( Atom nId, sal_uInt32 nTypeId, RscFlag * pClass,
                       Atom nConstantId )
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
                          RscTypCont *, sal_uInt32, const char * )
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

