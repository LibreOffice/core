/*************************************************************************
 *
 *  $RCSfile: rscclass.cxx,v $
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

    $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/rsc/source/res/rscclass.cxx,v 1.1.1.1 2000-09-18 16:42:55 hr Exp $

    Source Code Control System - Update

    $Log: not supported by cvs2svn $
    Revision 1.39  2000/09/17 12:51:11  willem.vandorp
    OpenOffice header added.

    Revision 1.38  2000/07/26 17:13:22  willem.vandorp
    Headers/footers replaced

    Revision 1.37  2000/07/12 11:38:05  th
    Unicode

    Revision 1.36  1999/11/24 17:54:35  th
    remove dependencies to vcl

    Revision 1.35  1999/09/08 09:24:59  mm
    BigEndian/LittleEndian komplett durchgezogen

    Revision 1.34  1999/09/07 13:30:05  mm
    UniCode

**************************************************************************/
/****************** I N C L U D E S **************************************/

// C and C++ Includes.
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Programmabhaengige Includes.
#ifndef _RSCDB_HXX
#include <rscdb.hxx>
#endif
#ifndef _RSCCLASS_HXX
#include <rscclass.hxx>
#endif

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
RscClass::RscClass( HASHID nId, USHORT nTypeId, RscTop * pSuperCl )
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
    USHORT  i;

    RscTop::Pre_dtor();

    for( i = 0; i < nEntries; i++ )
    {
        if( pVarTypeList[ i ].pDefault )
        {
            pVarTypeList[ i ].pClass->Destroy(
                         RSCINST( pVarTypeList[ i ].pClass,
                                  pVarTypeList[ i ].pDefault ) );
            RscMem::Free( pVarTypeList[ i ].pDefault );
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
        RscMem::Free( (void *)pVarTypeList );
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
    USHORT nEle,
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
CLASS_DATA RscClass::GetDfltData( USHORT nEle )
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
void RscClass::SetVarDflt( CLASS_DATA pData, USHORT nEle, BOOL bSet )
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
BOOL RscClass::IsDflt( CLASS_DATA pData, USHORT nEle )
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
    USHORT  i;
    CLASS_DATA  * ppData;
    RSCINST aInst;
    RSCINST aMemInst, aDfltI;

    if( !pInst )
    {
        aInst.pClass = this;
        aInst.pData = (CLASS_DATA) RscMem::Malloc( Size() );
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
        ((RscClassInst *)(aInst.pData + nSuperSize))->nVarDflt = 0xFFFFFFFF;

    for( i = 0; i < nEntries; i++ )
    {
        aDfltI = GetInstData( bOwnClass ? rDflt.pData : NULL, i, TRUE );

        if( (VAR_POINTER & pVarTypeList[ i ].nVarType)
          && !(VAR_NODATAINST & pVarTypeList[ i ].nVarType) )
        {
            if( VAR_EXTENDABLE & pVarTypeList[ i ].nVarType )
            {
                RSCINST * pInst = (RSCINST *)
                        (aInst.pData + pVarTypeList[ i ].nOffset );
                pInst->pClass = pVarTypeList[ i ].pClass;
                ppData = &pInst->pData;
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
    USHORT  i;

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
                    RscMem::Free( aTmpI.pData );
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
    HASHID nVarName,
    RscTop * pClass,
    RSCINST * pDflt,
    RSCVAR nVarType,
    USHORT nMask,
    HASHID nDataBaseName
)
{
    if( pVarTypeList )
        pVarTypeList = (VARTYPE_STRUCT *)
                 RscMem::Realloc( (void *)pVarTypeList,
                 (USHORT)((nEntries +1) * sizeof( VARTYPE_STRUCT )) );
    else
        pVarTypeList = (VARTYPE_STRUCT *)
            RscMem::Malloc( (USHORT)((nEntries +1)
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
    USHORT i;

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
    HASHID nVarName,
    const RSCINST & rInitInst,
    BOOL bInitDflt,
    RscTop * pCreateClass
)
{
    USHORT  i;
    RSCINST aTmpI;

    i = 0;
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
            RSCINST aDfltInst = rInitInst;
            if( !aDfltInst.IsInst() && bInitDflt )
            {
                // mit dem Variablen-Default besetzen
                aDfltInst.pData  = pVarTypeList[ i ].pDefault;
                aDfltInst.pClass = pVarTypeList[ i ].pClass;
            }

            aTmpI = GetInstData( rInst.pData, i );
            if( aTmpI.IsInst() )
            {
                if( aDfltInst.IsInst() )
                {
                    aTmpI.pClass->Destroy( aTmpI );
                    aTmpI.pClass->Create( &aTmpI, aDfltInst );
                }
            }
            else
            { // Wird ueber Zeiger angegeben
                if( VAR_EXTENDABLE & pVarTypeList[ i ].nVarType )
                {
                    RSCINST * pInst = (RSCINST *)
                            (rInst.pData + pVarTypeList[ i ].nOffset );
                    if( pCreateClass && pCreateClass->InHierarchy( aTmpI.pClass ) )
                        *pInst = pCreateClass->Create( NULL, aDfltInst );
                    else
                        *pInst = aTmpI.pClass->Create( NULL, aDfltInst );
                    aTmpI = *pInst;
                }
                else
                {
                    CLASS_DATA  * ppData
                        = (CLASS_DATA *)(rInst.pData + pVarTypeList[ i ].nOffset);
                    aTmpI = aTmpI.pClass->Create( NULL, aDfltInst );
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
    HASHID nVarName
)
{
    USHORT  i;
    RSCINST aVarI;

    i = 0;
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
    USHORT  i;
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
    USHORT  i;
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
    pClass->nVarDflt = 0xFFFFFFFF; // alles auf Default

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
    USHORT  i;
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
RSCINST RscClass::GetDefault( HASHID nVarId )
{
    USHORT  i;

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
BOOL RscClass::IsValueDflt( CLASS_DATA pData, USHORT nEle )
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
    USHORT  i;
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
void RscClass::SetDefault( const RSCINST & rInst, HASHID nVarName )
{
    USHORT  i;
    RSCINST aTmpI;

    i = 0;
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
    USHORT nTab,
    const char * pVarName
)
{
    USHORT  i, n;
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
                    char * pName = pHS->Get( pVarTypeList[ i ].nVarName );

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
long RscClass::GetCorrectValues
(
    const RSCINST & rInst,
    USHORT nVarPos,
    USHORT nTupelIdx,
    RscTypCont * pTC
)
{
    long nLang = 0;
    long nBaseValue;

    // Basiswert holen
    RSCINST aTmpI = GetInstData( rInst.pData, nVarPos, TRUE );
    aTmpI.pClass->GetNumber( aTmpI, &nBaseValue );

    // Sprach Delta holen
    aTmpI = rInst.pClass->GetVariable( rInst, nRsc_DELTALANG, RSCINST() );
    if( aTmpI.IsInst() )
    {
        RscWriteRc aMem;
        aTmpI.pClass->WriteRc( aTmpI, aMem, pTC, 0, FALSE );
        nLang = (short)aMem.GetShort( nTupelIdx * sizeof( short ) );
    }

    return nLang + nBaseValue;
}

ERRTYPE RscClass::WriteInstRc
(
    const RSCINST & rInst,
    RscWriteRc & rMem,
    RscTypCont * pTC,
    USHORT nDeep,
    BOOL bExtra
)
{
    USHORT i;
    ERRTYPE aError;
    RSCINST aTmpI;
    USHORT  nMaskOff;// Offset um Maskenfeld zu addressieren

    // Wenn eine Variable Maskierung hat, dann Maskenfeld
    for( i = 0; i < nEntries; i++ )
    {
        if( pVarTypeList[ i ].nMask )
        {
            nMaskOff = rMem.Size();
            rMem.Put( (USHORT)0 );
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
                    USHORT nMask = rMem.GetShort( nMaskOff );
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
    USHORT nDeep,
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

    USHORT i;
    // Wenn eine Variable Maskierung hat, dann Maskenfeld
    fprintf( fOutput, "\t//%s\n", pHS->Get( GetId() ) );
    for( i = 0; i < nEntries; i++ )
    {
        fprintf( fOutput, "\t%s", pHS->Get( pVarTypeList[ i ].nVarName ) );
        USHORT n = strlen( pHS->Get( pVarTypeList[ i ].nVarName ) );
        while( n < 20 )
        {
            putc( ' ', fOutput );
            n++;
        }
        fprintf( fOutput, " = %s;\n",
                pHS->Get( pVarTypeList[ i ].pClass->GetId() ) );
    };
}

//==================================================================
void RscClass::WriteRcAccess
(
    FILE * fOutput,
    RscTypCont * pTC,
    const char * pName
)
{
    fprintf( fOutput, "\t\tSet%s( %s ", pName, pHS->Get( GetId() ) );
    fprintf( fOutput, "%s ", aCallPar2.GetBuffer() );
    fprintf( fOutput, "ResId( (RSHEADER_TYPE*)(pResData+nOffset) ) ) );\n" );
    fprintf( fOutput, "\t\tnOffset += GetObjSizeRes( (RSHEADER_TYPE*)(pResData+nOffset) );\n" );
}

//==================================================================
void RscClass::WriteRcCtor( FILE * fOutput, RscTypCont * pTC )
{
    if( GetId() != HASH_NONAME )
    {
        // Konstruktor
        fprintf( fOutput, "%s::%s%s bFreeResource )",
                pHS->Get( GetId() ), pHS->Get( GetId() ),
                aCallParType.GetBuffer() );
        if( GetSuperClass() )
        {
            // Superaufruf
            fprintf( fOutput, "\n\t: %s", pHS->Get( GetSuperClass()->GetId() ) );
            fprintf( fOutput, "%s", GetSuperClass()->aCallPar1.GetBuffer() );
            fprintf( fOutput, " rResId.SetRT2( 0x%x ) )", GetTypId() );
        }
        fprintf( fOutput, "\n{\n" );
        fprintf( fOutput, "\tUSHORT\tnObjMask;\n" );
        fprintf( fOutput, "\tUSHORT\tnOffset = 0;\n" );
        fprintf( fOutput, "\tBYTE *\tpResData;\n\n" );
        fprintf( fOutput, "\tpResData = (BYTE *)GetClassRes();\n\n" );
        fprintf( fOutput, "\tnObjMask = *(USHORT*)pResData;\n" );
        fprintf( fOutput, "\tnOffset += 2;\n\n" );

        for( USHORT i = 0; i < nEntries; i++ )
        {
            if( !((VAR_NODATAINST | VAR_NORC) & pVarTypeList[ i ].nVarType ))
            {
                fprintf( fOutput, "\tif( nObjMask & 0x%x )\n\t{\n",
                        pVarTypeList[ i ].nMask );

                pVarTypeList[ i ].pClass->WriteRcAccess( fOutput, pTC,
                                    pHS->Get( pVarTypeList[ i ].nVarName ) );

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
RscSysDepend::RscSysDepend( HASHID nId, USHORT nTypeId, RscTop * pSuper )
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
                RscTypCont * pTC, USHORT nDeep, BOOL bExtra, BOOL bFirst )
{
    USHORT      nId = 0xFFFF;
    ERRTYPE     aError;
    RSCINST     aFileName;

    //Instanz mit dem Dateinamen "FILENAME" holen
    aFileName = RscClass::GetCopyVar( rInst, pHS->Test( "FILE" ) );
    if( aFileName.IsInst() )
    {
        RscWriteRc aTmpMem;
        aError = aFileName.pClass->WriteRcHeader( aFileName, aTmpMem, pTC,
                                                  RscId(), nDeep, bExtra );
        // Obsolete - need changes in VCL
        rMem.Put( (USHORT)0 );

        // Identifier schreiben
        if( aTmpMem.Size() && pTC && (*aTmpMem.GetUTF8( 0 ) != '\0') )
        {
            nId = pTC->PutSysName( rInst.pClass->GetTypId(),
                                   aTmpMem.GetUTF8( 0 ),
                                   0, (USHORT)0, bFirst );
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
                            RscTypCont * pTC, USHORT nDeep, BOOL bExtra )
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
RscFirstSysDepend::RscFirstSysDepend( HASHID nId, USHORT nTypeId,
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
                            RscTypCont * pTC, USHORT nDeep, BOOL bExtra )
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
RscTupel::RscTupel( HASHID nId, USHORT nTypeId, RscTop * pSuper )
    : RscClass( nId, nTypeId, pSuper )
{}

/*************************************************************************
|*
|*    RscTupel::GetTupelVar()
|*
|*    Beschreibung
|*
*************************************************************************/
RSCINST RscTupel::GetTupelVar( const RSCINST & rInst, USHORT nPos,
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
                         RscTypCont * pTC, USHORT nTab,
                         const char * pVarName )
{
    USHORT  i;
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
