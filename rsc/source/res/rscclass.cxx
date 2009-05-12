/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: rscclass.cxx,v $
 * $Revision: 1.10 $
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

// Programmabhaengige Includes.
#include <rscdb.hxx>
#include <rscclass.hxx>

#include <tools/fsys.hxx>
#include <tools/rcid.h>
#include <tools/rc.h>

/****************** C O D E **********************************************/
/****************** R s c C l a s s **************************************/
/*************************************************************************
|*
|*    RscClass::RscClass()
|*
|*    Beschreibung
|*    Ersterstellung    MM 25.05.91
|*    Letzte Aenderung  MM 25.05.91
|*
*************************************************************************/
RscClass::RscClass( Atom nId, sal_uInt32 nTypeId, RscTop * pSuperCl )
    : RscTop( nId, nTypeId, pSuperCl )
{
    nEntries = 0;
    pVarTypeList = NULL;
    nSuperSize = RscTop::Size();
    nSize = nSuperSize + ALIGNED_SIZE( sizeof( RscClassInst ) );
}

/*************************************************************************
|*
|*    RscClass::Pre_dtor()
|*
|*    Beschreibung
|*    Ersterstellung    MM 25.05.91
|*    Letzte Aenderung  MM 25.05.91
|*
*************************************************************************/
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

/*************************************************************************
|*
|*    RscClass::~RscClass()
|*
|*    Beschreibung
|*    Ersterstellung    MM 25.05.91
|*    Letzte Aenderung  MM 25.05.91
|*
*************************************************************************/
RscClass::~RscClass()
{
    if( pVarTypeList )
        rtl_freeMemory( (void *)pVarTypeList );
}

/*************************************************************************
|*
|*    RscClass::GetClassType()
|*
|*    Beschreibung
|*    Ersterstellung    MM 25.05.91
|*    Letzte Aenderung  MM 25.05.91
|*
*************************************************************************/
RSCCLASS_TYPE RscClass::GetClassType() const
{
    return RSCCLASS_COMPLEX;
}

/*************************************************************************
|*
|*    RscClass::GetInstData()
|*
|*    Beschreibung
|*    Ersterstellung    MM 15.04.91
|*    Letzte Aenderung  MM 15.04.91
|*
*************************************************************************/
RSCINST RscClass::GetInstData
(
    CLASS_DATA pData,
    sal_uInt32 nEle,
    BOOL bGetCopy
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

/*************************************************************************
|*
|*    RscClass::GetInstDflt()
|*
|*    Beschreibung
|*
*************************************************************************/
CLASS_DATA RscClass::GetDfltData( sal_uInt32 nEle )
{
    if( pVarTypeList[ nEle ].pDefault )
        return pVarTypeList[ nEle ].pDefault;

    return pVarTypeList[ nEle ].pClass->GetDefault().pData;
}

/*************************************************************************
|*
|*    RscClass::SetVarDflt()
|*
|*    Beschreibung
|*    Ersterstellung    MM 22.07.91
|*    Letzte Aenderung  MM 22.07.91
|*
*************************************************************************/
void RscClass::SetVarDflt( CLASS_DATA pData, sal_uInt32 nEle, BOOL bSet )
{
    RscClassInst * pClass;

    pClass = (RscClassInst *)(pData + nSuperSize );
    if( bSet )
        pClass->nVarDflt |= ((ULONG)1 << nEle);
    else
        pClass->nVarDflt &= ~((ULONG)1 << nEle);
}

/*************************************************************************
|*
|*    RscClass::IsDflt()
|*
|*    Beschreibung
|*    Ersterstellung    MM 22.07.91
|*    Letzte Aenderung  MM 08.01.92
|*
*************************************************************************/
BOOL RscClass::IsDflt( CLASS_DATA pData, sal_uInt32 nEle )
{
    RscClassInst *  pClass;
    BOOL            bRet;

    pClass = (RscClassInst *)(pData + nSuperSize );
    if( pClass->nVarDflt & ((ULONG)1 << nEle) )
        bRet = TRUE;
    else
        bRet = FALSE;
/*  {
        //Variablenname ist Default
        RSCINST aTmpI;

        aTmpI = GetInstData( pData, nEle, TRUE );
        if( aTmpI.IsInst() && !aTmpI.pClass->IsDefault( aTmpI ) )
            bRet = FALSE;
    }
*/
    return bRet;
}

/*************************************************************************
|*
|*    RscClass::Create()
|*
|*    Beschreibung
|*    Ersterstellung    MM 03.04.91
|*    Letzte Aenderung  MM 03.04.91
|*
*************************************************************************/
RSCINST RscClass::Create
(
    RSCINST * pInst,
    const RSCINST & rDflt,
    BOOL bOwnClass
)
{
    sal_uInt32  i;
    CLASS_DATA  * ppData;
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
        ((RscClassInst *)(aInst.pData + nSuperSize))->nVarDflt = ~((ULONG)0);

    for( i = 0; i < nEntries; i++ )
    {
        aDfltI = GetInstData( bOwnClass ? rDflt.pData : NULL, i, TRUE );

        if( (VAR_POINTER & pVarTypeList[ i ].nVarType)
          && !(VAR_NODATAINST & pVarTypeList[ i ].nVarType) )
        {
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
            aMemInst = GetInstData( aInst.pData, i, TRUE );
            aMemInst = aMemInst.pClass->Create( &aMemInst, aDfltI );
        };
    }

    return( aInst );
}

/*************************************************************************
|*
|*    RscClass::Destroy()
|*
|*    Beschreibung
|*
*************************************************************************/
void RscClass::Destroy( const RSCINST & rInst )
{
    sal_uInt32  i;

    RscTop::Destroy( rInst );

    for( i = 0; i < nEntries; i++ )
    {
        if( !(pVarTypeList[ i ].nVarType & VAR_NODATAINST) )
        {
            RSCINST aTmpI;

            aTmpI = GetInstData( rInst.pData, i, TRUE );
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

/*************************************************************************
|*
|*    RscClass::SetVariable()
|*
|*    Beschreibung
|*
*************************************************************************/
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
    if( nEntries > (sizeof( ULONG ) * 8) )
    {
        // Bereich fuer Default zu klein
        RscExit( 16 );
    };
    return( ERR_OK );
}

/*************************************************************************
|*
|*    RscClass::EnumVariable()
|*
|*    Beschreibung
|*
*************************************************************************/
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

/*************************************************************************
|*
|*    RscClass::GetVariable()
|*
|*    Beschreibung
|*
*************************************************************************/
RSCINST RscClass::GetVariable
(
    const RSCINST & rInst,
    Atom nVarName,
    const RSCINST & rInitInst,
    BOOL bInitDflt,
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
        SetVarDflt( rInst.pData, i, FALSE );
        return( aTmpI );
    };

    return( RscTop::GetVariable( rInst, nVarName, rInitInst,
                                bInitDflt, pCreateClass ) );
}

/*************************************************************************
|*
|*    RscClass::GetCopyVar()
|*
|*    Beschreibung
|*
*************************************************************************/
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
                aVarI = GetVariable( rInst, nVarName, RSCINST(), TRUE );
                SetVarDflt( rInst.pData, i, TRUE );
            }
            else
                aVarI = GetInstData( rInst.pData, i, TRUE );

        };
        return aVarI ;
    };

    return RscTop::GetCopyVar( rInst, nVarName );
}

/*************************************************************************
|*
|*    RscClass::IsConsistent()
|*
|*    Beschreibung
|*
*************************************************************************/
BOOL RscClass::IsConsistent( const RSCINST & rInst, RscInconsList * pList )
{
    sal_uInt32  i = 0;
    RSCINST aTmpI;
    BOOL    bRet;

    bRet = RscTop::IsConsistent( rInst, pList );

    for( i = 0; i < nEntries; i++ )
    {
        if( !(VAR_NODATAINST & pVarTypeList[ i ].nVarType) )
        {
            aTmpI = GetInstData( rInst.pData, i, TRUE );

            if( aTmpI.IsInst() )
                if( ! aTmpI.pClass->IsConsistent( aTmpI, pList ) )
                    bRet = FALSE;
        }
    };

    return( bRet );
}

/*************************************************************************
|*
|*    RscClass::SetToDefault()
|*
|*    Beschreibung
|*
*************************************************************************/
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
            aTmpI = GetInstData( rInst.pData, i, TRUE );
            if( aTmpI.IsInst() )
                aTmpI.pClass->SetToDefault( aTmpI );
        }
    }
    pClass->nVarDflt = ~((ULONG)0); // alles auf Default

    RscTop::SetToDefault( rInst );
}

/*************************************************************************
|*
|*    RscClass::IsDefault()
|*
|*    Beschreibung
|*
*************************************************************************/
BOOL RscClass::IsDefault( const RSCINST & rInst )
{
    sal_uInt32  i;
    RSCINST aTmpI;

    for( i = 0; i < nEntries; i++ )
    {
        // Variablen ohne eigenen Speicher werden vom "Datenserver"
        // auf Default untersucht
        if( !(VAR_NODATAINST & pVarTypeList[ i ].nVarType) )
            if( !IsDflt( rInst.pData, i ) )
                return( FALSE );
    };

    return( RscTop::IsDefault( rInst ) );
}

/*************************************************************************
|*
|*    RscClass::GetDefault()
|*
|*    Beschreibung
|*
*************************************************************************/
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

/*************************************************************************
|*
|*    RscClass::IsValueDflt()
|*
|*    Beschreibung
|*
*************************************************************************/
BOOL RscClass::IsValueDflt( CLASS_DATA pData, sal_uInt32 nEle )
{
    RSCINST aTmpI;

    aTmpI = GetInstData( pData, nEle, TRUE );

    if( aTmpI.IsInst() )
    {
        if( VAR_SVDYNAMIC & pVarTypeList[ nEle ].nVarType )
            return FALSE;

        if( aTmpI.pClass == pVarTypeList[ nEle ].pClass )
            //sie haben auch die gleiche Klasse
            return aTmpI.pClass->IsValueDefault( aTmpI, GetDfltData( nEle ) );
        else
            return FALSE;
    }
    return TRUE;
}

/*************************************************************************
|*
|*    RscClass::IsValueDefault()
|*
|*    Beschreibung
|*
*************************************************************************/
BOOL RscClass::IsValueDefault( const RSCINST & rInst, CLASS_DATA pDef )
{
    sal_uInt32  i = 0;
    RSCINST aTmpI;
    RSCINST aDfltI;

    if( !RscTop::IsValueDefault( rInst, pDef ) )
        return FALSE;

    if( pDef )
    {
        for( i = 0; i < nEntries; i++ )
        {
            aTmpI = GetInstData( rInst.pData, i, TRUE );
            if( aTmpI.IsInst() )
            {
                if( aTmpI.pClass != pVarTypeList[ i ].pClass )
                    //sie haben nicht die gleiche Klasse
                    return FALSE;

                aDfltI = GetInstData( pDef, i, TRUE );
                if( !aDfltI.IsInst() )
                    aDfltI.pData = GetDfltData( i );

                if( !aTmpI.pClass->IsValueDefault( aTmpI, aDfltI.pData ) )
                    return FALSE;
            }
        }
    }
    else
        return FALSE;

    return TRUE;
}

/*************************************************************************
|*
|*    RscClass::SetDefault()
|*
|*    Beschreibung
|*
*************************************************************************/
void RscClass::SetDefault( const RSCINST & rInst, Atom nVarName )
{
    sal_uInt32  i = 0;
    RSCINST aTmpI;

    while( i < nEntries && pVarTypeList[ i ].nVarName != nVarName )
        i++;

    if( i < nEntries )
    {
        aTmpI = GetInstData( rInst.pData, i, TRUE );
        if( aTmpI.IsInst() )
        {
            aTmpI.pClass->Destroy( aTmpI );
            aTmpI.pClass->Create( &aTmpI, RSCINST() );
            SetVarDflt( rInst.pData, i, TRUE );
        }
    }
    else //In Superklasse nach Variable suchen
        RscTop::SetDefault( rInst, nVarName );

}


/*************************************************************************
|*
|*    RscClass::WriteSrc()
|*
|*    Beschreibung
|*
*************************************************************************/
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
                        aTmpI = GetInstData( rInst.pData, i, TRUE );
                        aTmpI.pClass->WriteSrcHeader(
                              aTmpI, fOutput, pTC, nTab, RscId(), pVarName );
                    }

                    fprintf( fOutput, "( " );
                    aTmpI = GetInstData( rInst.pData, i+1, TRUE );
                    if( !aTmpI.IsInst() )
                        aTmpI.pData = GetDfltData( i+1 );
                    aTmpI.pClass->WriteSrcHeader(
                              aTmpI, fOutput, pTC, nTab, RscId(), pVarName );

                    fprintf( fOutput, ", " );
                    aTmpI = GetInstData( rInst.pData, i+2, TRUE );
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
                aTmpI = GetInstData( rInst.pData, i, TRUE );

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

/*************************************************************************
|*
|*    RscClass::WriteInstRc()
|*
|*    Beschreibung
|*
*************************************************************************/
INT32 RscClass::GetCorrectValues
(
    const RSCINST & rInst,
    sal_uInt32 nVarPos,
    sal_uInt32 nTupelIdx,
    RscTypCont * pTC
)
{
    INT32 nLang = 0;
    INT32 nBaseValue;

    // Basiswert holen
    RSCINST aTmpI = GetInstData( rInst.pData, nVarPos, TRUE );
    aTmpI.pClass->GetNumber( aTmpI, &nBaseValue );

    // Sprach Delta holen
    aTmpI = rInst.pClass->GetVariable( rInst, nRsc_DELTALANG, RSCINST() );
    if( aTmpI.IsInst() )
    {
        RscWriteRc aMem;
        aTmpI.pClass->WriteRc( aTmpI, aMem, pTC, 0, FALSE );
        nLang = (INT32)aMem.GetShort( nTupelIdx * sizeof(sal_uInt16) );
    }

    return nLang + nBaseValue;
}

ERRTYPE RscClass::WriteInstRc
(
    const RSCINST & rInst,
    RscWriteRc & rMem,
    RscTypCont * pTC,
    sal_uInt32 nDeep,
    BOOL bExtra
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
                        INT32 nVal = GetCorrectValues( rInst, i, 0, pTC );
                        rMem.Put( nVal );
                    }
                    else if( nRsc_Y == pVarTypeList[ i ].nVarName )
                    {
                        INT32 nVal = GetCorrectValues( rInst, i, 1, pTC );
                        rMem.Put( nVal );
                    }
                    else if( nRsc_WIDTH == pVarTypeList[ i ].nVarName )
                    {
                        INT32 nVal = GetCorrectValues( rInst, i, 2, pTC );
                        rMem.Put( nVal );
                    }
                    else if( nRsc_HEIGHT == pVarTypeList[ i ].nVarName )
                    {
                        INT32 nVal = GetCorrectValues( rInst, i, 3, pTC );
                        rMem.Put( nVal );
                    }
                    else
                    {
                        aTmpI = GetInstData( rInst.pData, i, TRUE );
                        // Nur an Variable Extradata bExtra nicht auf FALSE
                        // setzen
                        aError = aTmpI.pClass->
                            WriteRcHeader( aTmpI, rMem, pTC,
                                        RscId(), nDeep,
                                        (nRsc_EXTRADATA
                                        == pVarTypeList[ i ].nVarName)
                                        ? bExtra : FALSE );
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
                    aTmpI = GetInstData( rInst.pData, i, TRUE );
                // Nur an Variable Extradata bExtra nicht auf FALSE
                // setzen
                aError = aTmpI.pClass->
                            WriteRcHeader( aTmpI, rMem, pTC,
                                        RscId(), nDeep,
                                        (nRsc_EXTRADATA
                                        == pVarTypeList[ i ].nVarName)
                                        ? bExtra : FALSE );
            }
        }
    }

    return( aError );
}

/*************************************************************************
|*
|*    RscClass::WriteRc()
|*
|*    Beschreibung
|*
*************************************************************************/
ERRTYPE RscClass::WriteRc
(
    const RSCINST & rInst,
    RscWriteRc & rMem,
    RscTypCont * pTC,
    sal_uInt32 nDeep,
    BOOL bExtra
)
{
    ERRTYPE aError;

    aError = RscTop::WriteRc( rInst, rMem, pTC, nDeep, bExtra );
    if( aError.IsOk() )
        aError = WriteInstRc( rInst, rMem, pTC, nDeep, bExtra );

    return( aError );
}

/*************************************************************************
|*
|*    RscClass::WriteSyntax()
|*
|*    Beschreibung
|*
*************************************************************************/
void RscClass::WriteSyntax( FILE * fOutput, RscTypCont * pTC )
{
    RscTop::WriteSyntax( fOutput, pTC );

    sal_uInt32 i;
    // Wenn eine Variable Maskierung hat, dann Maskenfeld
    fprintf( fOutput, "\t//%s\n", pHS->getString( GetId() ).getStr() );
    for( i = 0; i < nEntries; i++ )
    {
        fprintf( fOutput, "\t%s", pHS->getString( pVarTypeList[ i ].nVarName ).getStr() );
        sal_uInt32 n = strlen( pHS->getString( pVarTypeList[ i ].nVarName ).getStr() );
        while( n < 20 )
        {
            putc( ' ', fOutput );
            n++;
        }
        fprintf( fOutput, " = %s;\n",
                pHS->getString( pVarTypeList[ i ].pClass->GetId() ).getStr() );
    };
}

//==================================================================
void RscClass::WriteRcAccess
(
    FILE * fOutput,
    RscTypCont * /*pTC*/,
    const char * pName
)
{
    fprintf( fOutput, "\t\tSet%s( %s ", pName, pHS->getString( GetId() ).getStr() );
    fprintf( fOutput, "%s ", aCallPar2.GetBuffer() );
    fprintf( fOutput, "ResId( (RSHEADER_TYPE*)(pResData+nOffset) ) ) );\n" );
    fprintf( fOutput, "\t\tnOffset += GetObjSizeRes( (RSHEADER_TYPE*)(pResData+nOffset) );\n" );
}

//==================================================================
void RscClass::WriteRcCtor( FILE * fOutput, RscTypCont * pTC )
{
    if( GetId() != InvalidAtom )
    {
        // Konstruktor
        fprintf( fOutput, "%s::%s%s bFreeResource )",
                pHS->getString( GetId() ).getStr(),
                pHS->getString( GetId() ).getStr(),
                aCallParType.GetBuffer() );
        if( GetSuperClass() )
        {
            // Superaufruf
            fprintf( fOutput, "\n\t: %s", pHS->getString( GetSuperClass()->GetId() ).getStr() );
            fprintf( fOutput, "%s", GetSuperClass()->aCallPar1.GetBuffer() );
            fprintf( fOutput, " rResId.SetRT2( 0x%lx ) )",
                     sal::static_int_cast< unsigned long >(GetTypId()) );
        }
        fprintf( fOutput, "\n{\n" );
        fprintf( fOutput, "\tsal_uInt32\tnObjMask;\n" );
        fprintf( fOutput, "\tsal_uInt32\tnOffset = 0;\n" );
        fprintf( fOutput, "\tBYTE *\tpResData;\n\n" );
        fprintf( fOutput, "\tpResData = (BYTE *)GetClassRes();\n\n" );
        fprintf( fOutput, "\tnObjMask = *(sal_uInt32*)pResData;\n" );
        fprintf( fOutput, "\tnOffset += 4;\n\n" );

        for( sal_uInt32 i = 0; i < nEntries; i++ )
        {
            if( !((VAR_NODATAINST | VAR_NORC) & pVarTypeList[ i ].nVarType ))
            {
                fprintf( fOutput, "\tif( nObjMask & 0x%lx )\n\t{\n",
                         sal::static_int_cast< unsigned long >(
                             pVarTypeList[ i ].nMask) );

                pVarTypeList[ i ].pClass->WriteRcAccess( fOutput, pTC,
                                    pHS->getString( pVarTypeList[ i ].nVarName ).getStr() );

                fprintf( fOutput, "\t}\n" );
            }
        }
        fprintf( fOutput, "\tIncrementRes( nOffset );\n" );
        fprintf( fOutput, "\tif( bFreeResource )\n" );
        fprintf( fOutput, "\t\tFreeResource();\n" );
        fprintf( fOutput, "}\n\n" );
    }
}

/*************************************************************************
|*
|*    RscSysDepend::RscSysDepend()
|*
|*    Beschreibung
|*
*************************************************************************/
RscSysDepend::RscSysDepend( Atom nId, sal_uInt32 nTypeId, RscTop * pSuper )
            : RscClass( nId, nTypeId, pSuper )
{}

/*************************************************************************
|*
|*    RscSysDepend::WriteRc()
|*
|*    Beschreibung
|*
*************************************************************************/
ERRTYPE RscSysDepend::WriteSysDependRc( const RSCINST & rInst, RscWriteRc & rMem,
                RscTypCont * pTC, sal_uInt32 nDeep, BOOL bExtra, BOOL bFirst )
{
    sal_uInt32  nId = 0xFFFFFFFF;
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

/*************************************************************************
|*
|*    RscSysDepend::WriteRc()
|*
|*    Beschreibung
|*
*************************************************************************/
ERRTYPE RscSysDepend::WriteRc( const RSCINST & rInst, RscWriteRc & rMem,
                            RscTypCont * pTC, sal_uInt32 nDeep, BOOL bExtra )
{
    ERRTYPE     aError = RscClass::WriteRc( rInst, rMem, pTC, nDeep, bExtra );

    if( this == rInst.pClass )
        // nur wenn es eigen Klasse ist
        aError = WriteSysDependRc( rInst, rMem, pTC, nDeep, bExtra );
    return aError;
}

/*************************************************************************
|*
|*    RscFirstSysDepend::RscFirstSysDepend()
|*
|*    Beschreibung
|*
*************************************************************************/
RscFirstSysDepend::RscFirstSysDepend( Atom nId, sal_uInt32 nTypeId,
                                        RscTop * pSuper )
            : RscSysDepend( nId, nTypeId, pSuper )
{}

/*************************************************************************
|*
|*    RscFirstSysDepend::WriteRc()
|*
|*    Beschreibung
|*
*************************************************************************/
ERRTYPE RscFirstSysDepend::WriteRc( const RSCINST & rInst, RscWriteRc & rMem,
                            RscTypCont * pTC, sal_uInt32 nDeep, BOOL bExtra )
{
    ERRTYPE aError = RscClass::WriteRc( rInst, rMem, pTC, nDeep, bExtra );
    aError = WriteSysDependRc( rInst, rMem, pTC, nDeep, bExtra, TRUE );
    return aError;
}

/*************************************************************************
|*
|*    RscTupel::RscTupel()
|*
|*    Beschreibung
|*
*************************************************************************/
RscTupel::RscTupel( Atom nId, sal_uInt32 nTypeId, RscTop * pSuper )
    : RscClass( nId, nTypeId, pSuper )
{}

/*************************************************************************
|*
|*    RscTupel::GetTupelVar()
|*
|*    Beschreibung
|*
*************************************************************************/
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

/*************************************************************************
|*
|*    RscTupel::WriteSrc()
|*
|*    Beschreibung
|*
*************************************************************************/
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
                aTmpI = GetInstData( rInst.pData, i, TRUE );

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
