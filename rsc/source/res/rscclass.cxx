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

#include <rscdb.hxx>
#include <rscclass.hxx>

#include <tools/rcid.h>
#include <tools/rc.h>

RscClass::RscClass( Atom nId, sal_uInt32 nTypeId, RscTop * pSuperCl )
    : RscTop( nId, nTypeId, pSuperCl )
{
    nEntries = 0;
    pVarTypeList = NULL;
    nSuperSize = RscTop::Size();
    nSize = nSuperSize + ALIGNED_SIZE( sizeof( RscClassInst ) );
}

void RscClass::Pre_dtor()
{
    sal_uInt32  i;

    RscTop::Pre_dtor();

    for( i = 0; i < nEntries; i++ )
    {
        if( pVarTypeList[ i ].pDefault )
        {
            pVarTypeList[ i ].pClass->Destroy(
                         RSCINST( pVarTypeList[ i ].pClass,
                                  pVarTypeList[ i ].pDefault ) );
            rtl_freeMemory( pVarTypeList[ i ].pDefault );
            pVarTypeList[ i ].pDefault = NULL;
        };
    };
}

RscClass::~RscClass()
{
    if( pVarTypeList )
        rtl_freeMemory( (void *)pVarTypeList );
}

RSCCLASS_TYPE RscClass::GetClassType() const
{
    return RSCCLASS_COMPLEX;
}

RSCINST RscClass::GetInstData
(
    CLASS_DATA pData,
    sal_uInt32 nEle,
    sal_Bool bGetCopy
)
{
    RSCINST aInst;

    aInst.pClass = pVarTypeList[ nEle ].pClass;
    if( pData )
    {
        if( VAR_NODATAINST & pVarTypeList[ nEle ].nVarType )
        {
            RSCINST aTmpI;

            aTmpI.pClass = this;
            aTmpI.pData = pData;
            if( bGetCopy )
                aInst.pData = GetCopyVar(
                                  aTmpI,
                                  pVarTypeList[ nEle ].nDataBaseName
                              ).pData;
            else
                aInst.pData = GetVariable(
                                  aTmpI,
                                  pVarTypeList[ nEle ].nDataBaseName,
                                  RSCINST()
                              ).pData;
        }
        else if( VAR_POINTER & pVarTypeList[ nEle ].nVarType )
        {
            if( VAR_EXTENDABLE & pVarTypeList[ nEle ].nVarType )
                aInst = *(RSCINST *)
                              (pData + pVarTypeList[ nEle ].nOffset);
            else
                aInst.pData = *(CLASS_DATA *)
                              (pData + pVarTypeList[ nEle ].nOffset);
        }
        else
            aInst.pData = pData + pVarTypeList[ nEle ].nOffset;
    };
    return( aInst );
}

CLASS_DATA RscClass::GetDfltData( sal_uInt32 nEle )
{
    if( pVarTypeList[ nEle ].pDefault )
        return pVarTypeList[ nEle ].pDefault;

    return pVarTypeList[ nEle ].pClass->GetDefault().pData;
}

void RscClass::SetVarDflt( CLASS_DATA pData, sal_uInt32 nEle, sal_Bool bSet )
{
    RscClassInst * pClass;

    pClass = (RscClassInst *)(pData + nSuperSize );
    if( bSet )
        pClass->nVarDflt |= ((sal_uLong)1 << nEle);
    else
        pClass->nVarDflt &= ~((sal_uLong)1 << nEle);
}

sal_Bool RscClass::IsDflt( CLASS_DATA pData, sal_uInt32 nEle )
{
    RscClassInst *  pClass;
    sal_Bool            bRet;

    pClass = (RscClassInst *)(pData + nSuperSize );
    if( pClass->nVarDflt & ((sal_uLong)1 << nEle) )
        bRet = sal_True;
    else
        bRet = sal_False;
    return bRet;
}

RSCINST RscClass::Create
(
    RSCINST * pInst,
    const RSCINST & rDflt,
    sal_Bool bOwnClass
)
{
    sal_uInt32  i;
    RSCINST aInst;
    RSCINST aMemInst, aDfltI;

    if( !pInst )
    {
        aInst.pClass = this;
        aInst.pData = (CLASS_DATA) rtl_allocateMemory( Size() );
    }
    else
        aInst = *pInst;
    if( !bOwnClass && rDflt.IsInst() )
        bOwnClass = rDflt.pClass->InHierarchy( this );

    RscTop::Create( &aInst, rDflt, bOwnClass );

    if( bOwnClass )
        ((RscClassInst *)(aInst.pData + nSuperSize))->nVarDflt =
            ((RscClassInst *)(rDflt.pData + nSuperSize))->nVarDflt;
    else
        ((RscClassInst *)(aInst.pData + nSuperSize))->nVarDflt = ~((sal_uLong)0);

    for( i = 0; i < nEntries; i++ )
    {
        aDfltI = GetInstData( bOwnClass ? rDflt.pData : NULL, i, sal_True );

        if( (VAR_POINTER & pVarTypeList[ i ].nVarType)
          && !(VAR_NODATAINST & pVarTypeList[ i ].nVarType) )
        {
            CLASS_DATA  * ppData;
            if( VAR_EXTENDABLE & pVarTypeList[ i ].nVarType )
            {
                RSCINST * pInstance = (RSCINST *)
                        (aInst.pData + pVarTypeList[ i ].nOffset );
                pInstance->pClass = pVarTypeList[ i ].pClass;
                ppData = &pInstance->pData;
            }
            else
                ppData = (CLASS_DATA* )
                        (aInst.pData + pVarTypeList[ i ].nOffset );
            *ppData = NULL;
            if( aDfltI.IsInst() )
            {
                aMemInst = pVarTypeList[ i ].pClass->Create( NULL, aDfltI );
                *ppData = aMemInst.pData;
            };
        }
        else
        {
            aMemInst = GetInstData( aInst.pData, i, sal_True );
            aMemInst = aMemInst.pClass->Create( &aMemInst, aDfltI );
        };
    }

    return( aInst );
}

void RscClass::Destroy( const RSCINST & rInst )
{
    sal_uInt32  i;

    RscTop::Destroy( rInst );

    for( i = 0; i < nEntries; i++ )
    {
        if( !(pVarTypeList[ i ].nVarType & VAR_NODATAINST) )
        {
            RSCINST aTmpI;

            aTmpI = GetInstData( rInst.pData, i, sal_True );
            if( aTmpI.IsInst() )
            {
                // Objekt loeschen
                aTmpI.pClass->Destroy( aTmpI );
                if( pVarTypeList[ i ].nVarType & VAR_POINTER )
                {
                    // Speicher freigeben
                    rtl_freeMemory( aTmpI.pData );
                };
            };
        }
    };
}

ERRTYPE RscClass::SetVariable
(
    Atom nVarName,
    RscTop * pClass,
    RSCINST * pDflt,
    RSCVAR nVarType,
    sal_uInt32 nMask,
    Atom nDataBaseName
)
{
    if( pVarTypeList )
        pVarTypeList = (VARTYPE_STRUCT *)
                 rtl_reallocateMemory( (void *)pVarTypeList,
                 ((nEntries +1) * sizeof( VARTYPE_STRUCT )) );
    else
        pVarTypeList = (VARTYPE_STRUCT *)
            rtl_allocateMemory( ((nEntries +1)
                            * sizeof( VARTYPE_STRUCT )) );

    pVarTypeList[ nEntries ].nVarName       = nVarName;
    pVarTypeList[ nEntries ].nMask          = nMask;
    pVarTypeList[ nEntries ].pClass         = pClass;
    pVarTypeList[ nEntries ].nOffset        = nSize;
    pVarTypeList[ nEntries ].nDataBaseName  = nDataBaseName;
    if( pDflt )
        pVarTypeList[ nEntries ].pDefault = pDflt->pData;
    else
        pVarTypeList[ nEntries ].pDefault = NULL;

    pVarTypeList[ nEntries ].nVarType = ~VAR_POINTER & nVarType;
    if( pClass->Size() > 10 || (nVarType & VAR_EXTENDABLE) )
        pVarTypeList[ nEntries ].nVarType |= VAR_POINTER;

    if( !(pVarTypeList[ nEntries ].nVarType & VAR_NODATAINST) )
    {
        if( pVarTypeList[ nEntries ].nVarType & VAR_POINTER )
        {
            if( pVarTypeList[ nEntries ].nVarType & VAR_EXTENDABLE )
                nSize += sizeof( RSCINST );
            else
                nSize += sizeof( CLASS_DATA );
        }
        else
            nSize += pClass->Size();
    }

    nEntries++;
    if( nEntries > (sizeof( sal_uLong ) * 8) )
    {
        // Bereich fuer Default zu klein
        RscExit( 16 );
    };
    return( ERR_OK );
}

void RscClass::EnumVariables( void * pData, VarEnumCallbackProc pProc )
{
    sal_uInt32 i;

    RscTop::EnumVariables( pData, pProc );
    for( i = 0; i < nEntries; i ++ )
    {
        if( !(pVarTypeList[ i ].nVarType & VAR_NOENUM) )
            (*pProc)( pData, pVarTypeList[ i ].pClass->GetClassType(),
                        pVarTypeList[ i ].nVarName );
    }
}

RSCINST RscClass::GetVariable
(
    const RSCINST & rInst,
    Atom nVarName,
    const RSCINST & rInitInst,
    sal_Bool bInitDflt,
    RscTop * pCreateClass
)
{
    sal_uInt32  i = 0;
    RSCINST aTmpI;

    while( i < nEntries && pVarTypeList[ i ].nVarName != nVarName )
        i++;
    if( i < nEntries )
    {
        if( VAR_NODATAINST & pVarTypeList[ i ].nVarType )
        {
            aTmpI = GetVariable( rInst,
                                 pVarTypeList[ i ].nDataBaseName,
                                 RSCINST() );
            aTmpI.pClass = pVarTypeList[ i ].pClass;
        }
        else
        {
            // Default Instanz generieren
            RSCINST aDefInst = rInitInst;
            if( !aDefInst.IsInst() && bInitDflt )
            {
                // mit dem Variablen-Default besetzen
                aDefInst.pData  = pVarTypeList[ i ].pDefault;
                aDefInst.pClass = pVarTypeList[ i ].pClass;
            }

            aTmpI = GetInstData( rInst.pData, i );
            if( aTmpI.IsInst() )
            {
                if( aDefInst.IsInst() )
                {
                    aTmpI.pClass->Destroy( aTmpI );
                    aTmpI.pClass->Create( &aTmpI, aDefInst );
                }
            }
            else
            { // Wird ueber Zeiger angegeben
                if( VAR_EXTENDABLE & pVarTypeList[ i ].nVarType )
                {
                    RSCINST * pInst = (RSCINST *)
                            (rInst.pData + pVarTypeList[ i ].nOffset );
                    if( pCreateClass && pCreateClass->InHierarchy( aTmpI.pClass ) )
                        *pInst = pCreateClass->Create( NULL, aDefInst );
                    else
                        *pInst = aTmpI.pClass->Create( NULL, aDefInst );
                    aTmpI = *pInst;
                }
                else
                {
                    CLASS_DATA  * ppData
                        = (CLASS_DATA *)(rInst.pData + pVarTypeList[ i ].nOffset);
                    aTmpI = aTmpI.pClass->Create( NULL, aDefInst );
                    *ppData = aTmpI.pData;
                }
            }
        };
        // auf nicht Default setzen
        SetVarDflt( rInst.pData, i, sal_False );
        return( aTmpI );
    };

    return( RscTop::GetVariable( rInst, nVarName, rInitInst,
                                bInitDflt, pCreateClass ) );
}

RSCINST RscClass::GetCopyVar
(
    const RSCINST & rInst,
    Atom nVarName
)
{
    sal_uInt32  i = 0;
    RSCINST aVarI;

    while( i < nEntries && pVarTypeList[ i ].nVarName != nVarName )
        i++;

    if( i < nEntries )
    {
        if( VAR_NODATAINST & pVarTypeList[ i ].nVarType )
        {
            aVarI = GetCopyVar( rInst, pVarTypeList[ i ].nDataBaseName );
            aVarI.pClass = pVarTypeList[ i ].pClass;
        }
        else
        {
            if( IsDflt( rInst.pData, i ) )
            {
                // mit Variablen Default initialiaieren
                aVarI = GetVariable( rInst, nVarName, RSCINST(), sal_True );
                SetVarDflt( rInst.pData, i, sal_True );
            }
            else
                aVarI = GetInstData( rInst.pData, i, sal_True );

        };
        return aVarI ;
    };

    return RscTop::GetCopyVar( rInst, nVarName );
}

sal_Bool RscClass::IsConsistent( const RSCINST & rInst )
{
    sal_uInt32  i = 0;
    RSCINST aTmpI;
    sal_Bool    bRet;

    bRet = RscTop::IsConsistent( rInst );

    for( i = 0; i < nEntries; i++ )
    {
        if( !(VAR_NODATAINST & pVarTypeList[ i ].nVarType) )
        {
            aTmpI = GetInstData( rInst.pData, i, sal_True );

            if( aTmpI.IsInst() )
                if( ! aTmpI.pClass->IsConsistent( aTmpI ) )
                    bRet = sal_False;
        }
    };

    return( bRet );
}

void RscClass::SetToDefault( const RSCINST & rInst )
{
    sal_uInt32  i;
    RSCINST aTmpI;
    RscClassInst *  pClass;

    pClass = (RscClassInst *)(rInst.pData + nSuperSize );

    for( i = 0; i < nEntries; i++ )
    {
        // Variablen ohne eigenen Speicher werden vom "Datenserver"
        // auf Default gesetzt
        if( !(VAR_NODATAINST & pVarTypeList[ i ].nVarType) )
        {
            aTmpI = GetInstData( rInst.pData, i, sal_True );
            if( aTmpI.IsInst() )
                aTmpI.pClass->SetToDefault( aTmpI );
        }
    }
    pClass->nVarDflt = ~((sal_uLong)0); // alles auf Default

    RscTop::SetToDefault( rInst );
}

sal_Bool RscClass::IsDefault( const RSCINST & rInst )
{
    sal_uInt32  i;
    RSCINST aTmpI;

    for( i = 0; i < nEntries; i++ )
    {
        // Variablen ohne eigenen Speicher werden vom "Datenserver"
        // auf Default untersucht
        if( !(VAR_NODATAINST & pVarTypeList[ i ].nVarType) )
            if( !IsDflt( rInst.pData, i ) )
                return( sal_False );
    };

    return( RscTop::IsDefault( rInst ) );
}

RSCINST RscClass::GetDefault( Atom nVarId )
{
    sal_uInt32  i;

    i = 0;
    while( i < nEntries && pVarTypeList[ i ].nVarName != nVarId )
        i++;
    if( i < nEntries )
    {
        RSCINST aTmpI;

        aTmpI.pClass = pVarTypeList[ i ].pClass;
        aTmpI.pData  = GetDfltData( i );
        return( aTmpI );
    };

    return( RscTop::GetDefault( nVarId ) );
}

sal_Bool RscClass::IsValueDflt( CLASS_DATA pData, sal_uInt32 nEle )
{
    RSCINST aTmpI;

    aTmpI = GetInstData( pData, nEle, sal_True );

    if( aTmpI.IsInst() )
    {
        if( VAR_SVDYNAMIC & pVarTypeList[ nEle ].nVarType )
            return sal_False;

        if( aTmpI.pClass == pVarTypeList[ nEle ].pClass )
            //sie haben auch die gleiche Klasse
            return aTmpI.pClass->IsValueDefault( aTmpI, GetDfltData( nEle ) );
        else
            return sal_False;
    }
    return sal_True;
}

sal_Bool RscClass::IsValueDefault( const RSCINST & rInst, CLASS_DATA pDef )
{
    RSCINST aTmpI;
    RSCINST aDfltI;

    if( !RscTop::IsValueDefault( rInst, pDef ) )
        return sal_False;

    if( pDef )
    {
        for( sal_uInt32 i = 0; i < nEntries; i++ )
        {
            aTmpI = GetInstData( rInst.pData, i, sal_True );
            if( aTmpI.IsInst() )
            {
                if( aTmpI.pClass != pVarTypeList[ i ].pClass )
                    //sie haben nicht die gleiche Klasse
                    return sal_False;

                aDfltI = GetInstData( pDef, i, sal_True );
                if( !aDfltI.IsInst() )
                    aDfltI.pData = GetDfltData( i );

                if( !aTmpI.pClass->IsValueDefault( aTmpI, aDfltI.pData ) )
                    return sal_False;
            }
        }
    }
    else
        return sal_False;

    return sal_True;
}

void RscClass::SetDefault( const RSCINST & rInst, Atom nVarName )
{
    sal_uInt32  i = 0;
    RSCINST aTmpI;

    while( i < nEntries && pVarTypeList[ i ].nVarName != nVarName )
        i++;

    if( i < nEntries )
    {
        aTmpI = GetInstData( rInst.pData, i, sal_True );
        if( aTmpI.IsInst() )
        {
            aTmpI.pClass->Destroy( aTmpI );
            aTmpI.pClass->Create( &aTmpI, RSCINST() );
            SetVarDflt( rInst.pData, i, sal_True );
        }
    }
    else //In Superklasse nach Variable suchen
        RscTop::SetDefault( rInst, nVarName );

}


void RscClass::WriteSrc
(
    const RSCINST & rInst,
    FILE * fOutput,
    RscTypCont * pTC,
    sal_uInt32 nTab,
    const char * pVarName
)
{
    sal_uInt32  i = 0, n = 0;
    RSCINST aTmpI;

    RscTop::WriteSrc( rInst, fOutput, pTC, nTab, pVarName );

    for( i = 0; i < nEntries; i++ )
    {
        if( !(VAR_HIDDEN & pVarTypeList[ i ].nVarType) )
        {
            // Hack wegen Position und Dimensiuon
            if( nRsc_XYMAPMODEId == pVarTypeList[ i ].nVarName
              || nRsc_WHMAPMODEId == pVarTypeList[ i ].nVarName )
            {
                if( !IsDflt( rInst.pData, i )     //MapUnit
                  || !IsDflt( rInst.pData, i+1 )  //X, Width
                  || !IsDflt( rInst.pData, i+2 ) )//Y, Height
                {// ein Wert ist nicht Default
                    for( n = 0; n < nTab; n++ )
                        fputc( '\t', fOutput );
                    if( nRsc_XYMAPMODEId == pVarTypeList[ i ].nVarName )
                        fprintf( fOutput, "Pos = " );
                    else
                        fprintf( fOutput, "Size = " );

                    if( !IsDflt( rInst.pData, i ) )
                    {
                        aTmpI = GetInstData( rInst.pData, i, sal_True );
                        aTmpI.pClass->WriteSrcHeader(
                              aTmpI, fOutput, pTC, nTab, RscId(), pVarName );
                    }

                    fprintf( fOutput, "( " );
                    aTmpI = GetInstData( rInst.pData, i+1, sal_True );
                    if( !aTmpI.IsInst() )
                        aTmpI.pData = GetDfltData( i+1 );
                    aTmpI.pClass->WriteSrcHeader(
                              aTmpI, fOutput, pTC, nTab, RscId(), pVarName );

                    fprintf( fOutput, ", " );
                    aTmpI = GetInstData( rInst.pData, i+2, sal_True );
                    if( !aTmpI.IsInst() )
                        aTmpI.pData = GetDfltData( i+2 );
                    aTmpI.pClass->WriteSrcHeader(
                              aTmpI, fOutput, pTC, nTab, RscId(), pVarName );
                    fprintf( fOutput, " );\n" );
                }
                i += 2; //_X, _Y oder _Widht, Height ueberlesen
            }
            else if( !IsDflt( rInst.pData, i )
              && !IsValueDflt( rInst.pData, i ) )
            {
                aTmpI = GetInstData( rInst.pData, i, sal_True );

                if( aTmpI.IsInst() )
                {
                    const char * pName = pHS->getString( pVarTypeList[ i ].nVarName ).getStr();

                    for( n = 0; n < nTab; n++ )
                        fputc( '\t', fOutput );
                    fprintf( fOutput, "%s", pName );
                    fprintf( fOutput, " = " );
                    aTmpI.pClass->WriteSrcHeader(
                              aTmpI, fOutput, pTC, nTab, RscId(), pName );
                    fprintf( fOutput, ";\n" );
                }
            };
        };
    };

    return;
}

sal_Int32 RscClass::GetCorrectValues
(
    const RSCINST & rInst,
    sal_uInt32 nVarPos,
    sal_uInt32 nTupelIdx,
    RscTypCont * pTC
)
{
    sal_Int32 nLang = 0;
    sal_Int32 nBaseValue;

    // Basiswert holen
    RSCINST aTmpI = GetInstData( rInst.pData, nVarPos, sal_True );
    aTmpI.pClass->GetNumber( aTmpI, &nBaseValue );

    // Sprach Delta holen
    aTmpI = rInst.pClass->GetVariable( rInst, nRsc_DELTALANG, RSCINST() );
    if( aTmpI.IsInst() )
    {
        RscWriteRc aMem;
        aTmpI.pClass->WriteRc( aTmpI, aMem, pTC, 0, sal_False );
        nLang = (sal_Int32)aMem.GetShort( nTupelIdx * sizeof(sal_uInt16) );
    }

    return nLang + nBaseValue;
}

ERRTYPE RscClass::WriteInstRc
(
    const RSCINST & rInst,
    RscWriteRc & rMem,
    RscTypCont * pTC,
    sal_uInt32 nDeep,
    sal_Bool bExtra
)
{
    sal_uInt32 i = 0;
    ERRTYPE aError;
    RSCINST aTmpI;
    sal_uInt32  nMaskOff = 0;// Offset um Maskenfeld zu addressieren

    // Wenn eine Variable Maskierung hat, dann Maskenfeld
    for( i = 0; i < nEntries; i++ )
    {
        if( pVarTypeList[ i ].nMask )
        {
            nMaskOff = rMem.Size();
            rMem.Put( sal_uInt32(0) );
            break;
        }
    };

    for( i = 0; i < nEntries && aError.IsOk(); i++ )
    {
        if( !((VAR_NODATAINST | VAR_NORC) & pVarTypeList[ i ].nVarType ))
        {
            if( pVarTypeList[ i ].nMask )
            {
                if( !IsDflt( rInst.pData, i ) )
                {
                    if( nRsc_X == pVarTypeList[ i ].nVarName )
                    {
                        sal_Int32 nVal = GetCorrectValues( rInst, i, 0, pTC );
                        rMem.Put( nVal );
                    }
                    else if( nRsc_Y == pVarTypeList[ i ].nVarName )
                    {
                        sal_Int32 nVal = GetCorrectValues( rInst, i, 1, pTC );
                        rMem.Put( nVal );
                    }
                    else if( nRsc_WIDTH == pVarTypeList[ i ].nVarName )
                    {
                        sal_Int32 nVal = GetCorrectValues( rInst, i, 2, pTC );
                        rMem.Put( nVal );
                    }
                    else if( nRsc_HEIGHT == pVarTypeList[ i ].nVarName )
                    {
                        sal_Int32 nVal = GetCorrectValues( rInst, i, 3, pTC );
                        rMem.Put( nVal );
                    }
                    else
                    {
                        aTmpI = GetInstData( rInst.pData, i, sal_True );
                        // Nur an Variable Extradata bExtra nicht auf sal_False
                        // setzen
                        aError = aTmpI.pClass->
                            WriteRcHeader( aTmpI, rMem, pTC,
                                        RscId(), nDeep,
                                        (nRsc_EXTRADATA
                                        == pVarTypeList[ i ].nVarName)
                                        ? bExtra : sal_False );
                    }
                    sal_uInt32 nMask = rMem.GetLong( nMaskOff );
                    nMask |= pVarTypeList[ i ].nMask;
                    rMem.PutAt( nMaskOff, nMask );
                }
            }
            else{
                if( IsDflt( rInst.pData, i ) )
                {
                    aTmpI.pClass = pVarTypeList[ i ].pClass;
                    aTmpI.pData  = GetDfltData( i );
                }
                else
                    aTmpI = GetInstData( rInst.pData, i, sal_True );
                // Nur an Variable Extradata bExtra nicht auf sal_False
                // setzen
                aError = aTmpI.pClass->
                            WriteRcHeader( aTmpI, rMem, pTC,
                                        RscId(), nDeep,
                                        (nRsc_EXTRADATA
                                        == pVarTypeList[ i ].nVarName)
                                        ? bExtra : sal_False );
            }
        }
    }

    return( aError );
}

ERRTYPE RscClass::WriteRc
(
    const RSCINST & rInst,
    RscWriteRc & rMem,
    RscTypCont * pTC,
    sal_uInt32 nDeep,
    sal_Bool bExtra
)
{
    ERRTYPE aError;

    aError = RscTop::WriteRc( rInst, rMem, pTC, nDeep, bExtra );
    if( aError.IsOk() )
        aError = WriteInstRc( rInst, rMem, pTC, nDeep, bExtra );

    return( aError );
}

RscSysDepend::RscSysDepend( Atom nId, sal_uInt32 nTypeId, RscTop * pSuper )
            : RscClass( nId, nTypeId, pSuper )
{}

ERRTYPE RscSysDepend::WriteSysDependRc( const RSCINST & rInst, RscWriteRc & rMem,
                RscTypCont * pTC, sal_uInt32 nDeep, sal_Bool bExtra, sal_Bool bFirst )
{
    ERRTYPE     aError;
    RSCINST     aFileName;

    //Instanz mit dem Dateinamen "FILENAME" holen
    aFileName = RscClass::GetCopyVar( rInst, pHS->getID( "FILE", true ) );
    if( aFileName.IsInst() )
    {
        RscWriteRc aTmpMem;
        aError = aFileName.pClass->WriteRcHeader( aFileName, aTmpMem, pTC,
                                                  RscId(), nDeep, bExtra );
        // Obsolete - need changes in VCL
        rMem.Put( sal_uInt32(0) );

        // Identifier schreiben
        sal_uInt32  nId = 0xFFFFFFFF;
        if( aTmpMem.Size() && pTC && (*aTmpMem.GetUTF8( 0 ) != '\0') )
        {
            nId = pTC->PutSysName( rInst.pClass->GetTypId(),
                                   aTmpMem.GetUTF8( 0 ),
                                   0, 0, bFirst );
        }
        rMem.Put( nId );
        aError = aFileName.pClass->WriteRcHeader( aFileName, rMem, pTC,
                                                  RscId(), nDeep, bExtra );
    }
    else
        aError = ERR_ERROR;

    return( aError );
}

ERRTYPE RscSysDepend::WriteRc( const RSCINST & rInst, RscWriteRc & rMem,
                            RscTypCont * pTC, sal_uInt32 nDeep, sal_Bool bExtra )
{
    ERRTYPE     aError = RscClass::WriteRc( rInst, rMem, pTC, nDeep, bExtra );

    if( this == rInst.pClass )
        // nur wenn es eigen Klasse ist
        aError = WriteSysDependRc( rInst, rMem, pTC, nDeep, bExtra );
    return aError;
}

RscTupel::RscTupel( Atom nId, sal_uInt32 nTypeId, RscTop * pSuper )
    : RscClass( nId, nTypeId, pSuper )
{}

RSCINST RscTupel::GetTupelVar( const RSCINST & rInst, sal_uInt32 nPos,
                                 const RSCINST & rInitInst )
{
    if( nPos >= nEntries )
    {
        return RSCINST();
    }
    else
        return GetVariable( rInst, pVarTypeList[ nPos ].nVarName, rInitInst );
}

void RscTupel::WriteSrc( const RSCINST & rInst, FILE * fOutput,
                         RscTypCont * pTC, sal_uInt32 nTab,
                         const char * pVarName )
{
    sal_uInt32  i = 0;
    RSCINST aTmpI;

    RscTop::WriteSrc( rInst, fOutput, pTC, nTab, pVarName );

    fprintf( fOutput, "< " );
    for( i = 0; i < nEntries; i++ )
    {
        if( !(VAR_HIDDEN & pVarTypeList[ i ].nVarType) )
        {
            if( !IsDflt( rInst.pData, i )
              && !IsValueDflt( rInst.pData, i ) )
            {
                aTmpI = GetInstData( rInst.pData, i, sal_True );

                if( aTmpI.IsInst() )
                    aTmpI.pClass->WriteSrcHeader(
                              aTmpI, fOutput, pTC, nTab, RscId(), pVarName );
                else
                    fprintf( fOutput, "Default" );
            }
            else
                fprintf( fOutput, "Default" );
            fprintf( fOutput, "; " );
        };
    };
    fprintf( fOutput, ">" );

    return;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
