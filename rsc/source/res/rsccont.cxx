/*************************************************************************
 *
 *  $RCSfile: rsccont.cxx,v $
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

    $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/rsc/source/res/rsccont.cxx,v 1.1.1.1 2000-09-18 16:42:56 hr Exp $

    Source Code Control System - Update

    $Log: not supported by cvs2svn $
    Revision 1.20  2000/09/17 12:51:11  willem.vandorp
    OpenOffice header added.

    Revision 1.19  2000/07/26 17:13:22  willem.vandorp
    Headers/footers replaced

    Revision 1.18  2000/07/12 11:38:19  th
    Unicode

    Revision 1.17  1999/11/24 17:54:47  th
    remove dependencies to vcl

    Revision 1.16  1999/09/08 09:24:59  mm
    BigEndian/LittleEndian komplett durchgezogen

    Revision 1.15  1998/02/19 17:50:42  HJS
    includes

**************************************************************************/
/****************** I N C L U D E S **************************************/

// C and C++ Includes.
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Programmabh„ngige Includes.
#ifndef _RSCCONT_HXX
#include <rsccont.hxx>
#endif

#include <tools/rcid.h>

/****************** C O D E **********************************************/
/****************** E N T R Y S T R U C T ********************************/
/*************************************************************************
|*
|*    ENTRYSTRUCT::Destroy()
|*
|*    Beschreibung
|*    Ersterstellung    MM 06.08.91
|*    Letzte Aenderung  MM 06.08.91
|*
*************************************************************************/
void ENTRY_STRUCT::Destroy()
{
    aName.Destroy();
    if( aInst.IsInst() ){
        aInst.pClass->Destroy( aInst );
        RscMem::Free( aInst.pData );
    };
}

/****************** R s c B a s e C o n t ********************************/
/*************************************************************************
|*
|*    RscBaseCont::RscBaseCont()
|*
|*    Beschreibung
|*    Ersterstellung    MM 25.05.91
|*    Letzte Aenderung  MM 25.05.91
|*
*************************************************************************/
RscBaseCont::RscBaseCont( HASHID nId, USHORT nTypeId, RscTop * pSuper,
                          BOOL bNoIdent )
    : RscTop( nId, nTypeId, pSuper )
{
    pTypeClass = NULL;
    pTypeClass1 = NULL;
    bNoId = bNoIdent;
    nOffInstData = RscTop::Size();
    nSize = nOffInstData + ALIGNED_SIZE( sizeof( RscBaseContInst ) );
}

/*************************************************************************
|*
|*    RscBaseCont::~RscBaseCont()
|*
|*    Beschreibung
|*    Ersterstellung    MM 25.05.91
|*    Letzte Aenderung  MM 25.05.91
|*
*************************************************************************/
RscBaseCont::~RscBaseCont(){
}

/*************************************************************************
|*
|*    RscBaseCont::GetClassType()
|*
|*    Beschreibung
|*    Ersterstellung    MM 25.05.91
|*    Letzte Aenderung  MM 25.05.91
|*
*************************************************************************/
RSCCLASS_TYPE RscBaseCont::GetClassType() const
{
    return RSCCLASS_COMPLEX;
}

/*************************************************************************
|*
|*    DestroyElements()
|*
|*    Beschreibung
|*    Ersterstellung    MM 26.04.91
|*    Letzte Aenderung  MM 26.04.91
|*
*************************************************************************/
void RscBaseCont::DestroyElements( RscBaseContInst * pClassData ){
    USHORT  i;

    if( pClassData->nEntries ){
        for( i = 0; i < pClassData->nEntries; i++ ){
            pClassData->pEntries[ i ].Destroy();
        };
        RscMem::Free( pClassData->pEntries );
        pClassData->pEntries = NULL;
        pClassData->nEntries = 0;
    };
}

/*************************************************************************
|*
|*    RscBaseCont::Create()
|*
|*    Beschreibung
|*    Ersterstellung    MM 26.04.91
|*    Letzte Aenderung  MM 26.04.91
|*
*************************************************************************/
RSCINST RscBaseCont::Create( RSCINST * pInst, const RSCINST & rDflt,
                             BOOL bOwnClass )
{
    USHORT  i;
    RSCINST aInst;
    RscBaseContInst * pClassData;

    if( !pInst ){
        aInst.pClass = this;
        aInst.pData = (CLASS_DATA) RscMem::Malloc( Size() );
    }
    else
        aInst = *pInst;
    if( !bOwnClass && rDflt.IsInst() )
        bOwnClass = rDflt.pClass->InHierarchy( this );

    RscTop::Create( &aInst, rDflt, bOwnClass );

    pClassData = (RscBaseContInst *)(aInst.pData + nOffInstData);
    pClassData->nEntries = 0;
    pClassData->pEntries = NULL;
    pClassData->bDflt = TRUE;

    if( bOwnClass ){
        RscBaseContInst *   pDfltClassData;
        RSCINST         aDfltI;

        pDfltClassData = (RscBaseContInst *)(rDflt.pData + nOffInstData);

        if( 0 != pDfltClassData->nEntries ){
            *pClassData = *pDfltClassData;
            pClassData->pEntries =
                (ENTRY_STRUCT *)RscMem::Malloc( sizeof( ENTRY_STRUCT )
                                                * pClassData->nEntries );
            for( i = 0; i < pClassData->nEntries; i++ ){
                pClassData->pEntries[ i ].Create();
                pClassData->pEntries[ i ].aName =
                                    pDfltClassData->pEntries[ i ].aName;
                aDfltI = pDfltClassData->pEntries[ i ].aInst;
                pClassData->pEntries[ i ].aInst =
                                    aDfltI.pClass->Create( NULL, aDfltI );
            };
        };
    }

    return( aInst );
}

/*************************************************************************
|*
|*    RscBaseCont::Destroy()
|*
|*    Beschreibung
|*    Ersterstellung    MM 25.05.91
|*    Letzte Aenderung  MM 25.05.91
|*
*************************************************************************/
void RscBaseCont::Destroy( const RSCINST & rInst ){
    RscBaseContInst * pClassData;

    RscTop::Destroy( rInst);

    pClassData = (RscBaseContInst *)(rInst.pData + nOffInstData);
    DestroyElements( pClassData );
}

/*************************************************************************
|*
|*    RscBaseCont::SearchEle()
|*
|*    Beschreibung
|*    Ersterstellung    MM 17.05.91
|*    Letzte Aenderung  MM 17.05.91
|*
*************************************************************************/
RSCINST RscBaseCont::SearchElePos( const RSCINST & rInst, const RscId & rEleName,
                                   RscTop * pClass, USHORT nPos )
{
    USHORT  i;
    RscBaseContInst * pClassData;

    pClassData = (RscBaseContInst *)(rInst.pData + nOffInstData);
    if( !pClass )
        pClass = pTypeClass;

    if( rEleName.IsId() ){
        for( i = nPos; i < pClassData->nEntries; i++ ){
            if( pClassData->pEntries[ i ].aName == rEleName
              && pClassData->pEntries[ i ].aInst.pClass == pClass )
            {
                return( pClassData->pEntries[ i ].aInst );
            };
        };
    };
    return RSCINST();
}

/*************************************************************************
|*
|*    RscBaseCont::SearchEle()
|*
|*    Beschreibung
|*    Ersterstellung    MM 17.05.91
|*    Letzte Aenderung  MM 17.05.91
|*
*************************************************************************/
RSCINST RscBaseCont::SearchEle( const RSCINST & rInst, const RscId & rEleName,
                                RscTop * pClass )
{
    return SearchElePos( rInst, rEleName, pClass, 0 );
}

/*************************************************************************
|*
|*    RscBaseCont::GetElement()
|*
|*    Beschreibung
|*    Ersterstellung    MM 26.04.91
|*    Letzte Aenderung  MM 26.04.91
|*
*************************************************************************/
ERRTYPE RscBaseCont::GetElement( const RSCINST & rInst, const RscId & rEleName,
                                 RscTop * pCreateClass,
                                 const RSCINST & rCreateInst, RSCINST * pGetInst )
{
    RscBaseContInst * pClassData;
    RSCINST           aTmpI;
    ERRTYPE           aError;

    if( !bNoId && !rEleName.IsId() )
        aError = WRN_CONT_NOID;
    pClassData = (RscBaseContInst *)(rInst.pData + nOffInstData);

    if( pCreateClass ){
        if( !pCreateClass->InHierarchy( pTypeClass ) ){
            if( pTypeClass1 ){
                if( !pCreateClass->InHierarchy( pTypeClass1 ) ){
                    // Falscher Typ
                    return ERR_CONT_INVALIDTYPE;
                }
            }
            else{
                // Falscher Typ
                return ERR_CONT_INVALIDTYPE;
            };
        };
    }
    else
        pCreateClass = pTypeClass;

    pClassData->bDflt = FALSE;
    if( !bNoId )
        aTmpI = SearchEle( rInst, rEleName, pCreateClass );
    // Eintrag gefunden
    if( aTmpI.IsInst() ){
        aError = WRN_CONT_DOUBLEID;
        if( rCreateInst.IsInst() ){
            aTmpI.pClass->Destroy( aTmpI );
            aTmpI.pClass->Create( &aTmpI, rCreateInst );
        };
    }
    else {
        if( pClassData->pEntries ){
            pClassData->pEntries =
                (ENTRY_STRUCT *)RscMem::Realloc( pClassData->pEntries,
                         sizeof( ENTRY_STRUCT ) * (pClassData->nEntries +1) );
        }
        else {
            pClassData->pEntries =
                (ENTRY_STRUCT *)RscMem::Malloc( sizeof( ENTRY_STRUCT )
                                                * (pClassData->nEntries +1) );
        };

        pClassData->pEntries[ pClassData->nEntries ].Create();
        pClassData->pEntries[ pClassData->nEntries ].aName = rEleName;

        if( rCreateInst.IsInst() ){
            // Instanz mit CreateInst-Daten initialisieren
            pClassData->pEntries[ pClassData->nEntries ].aInst =
                        pCreateClass->Create( NULL, rCreateInst );
        }
        else {
            pClassData->pEntries[ pClassData->nEntries ].aInst =
                        pCreateClass->Create( NULL, RSCINST() );
        };

        pClassData->nEntries++;
        aTmpI = pClassData->pEntries[ pClassData->nEntries -1 ].aInst;
    }

    *pGetInst = aTmpI;
    return aError;
}

/*************************************************************************
|*
|*    RscBaseCont::GetCount()
|*
|*    Beschreibung
|*    Ersterstellung    MM 08.05.91
|*    Letzte Aenderung  MM 08.05.91
|*
*************************************************************************/
USHORT RscBaseCont::GetCount( const RSCINST & rInst ){
    RscBaseContInst * pClassData;

    pClassData = (RscBaseContInst *)(rInst.pData + nOffInstData);
    return( pClassData->nEntries );
}

/*************************************************************************
|*
|*    RscBaseCont::GetPosEle()
|*
|*    Beschreibung
|*    Ersterstellung    MM 08.05.91
|*    Letzte Aenderung  MM 08.05.91
|*
*************************************************************************/
RSCINST RscBaseCont::GetPosEle( const RSCINST & rInst, USHORT nPos ){
    RscBaseContInst * pClassData;

    pClassData = (RscBaseContInst *)(rInst.pData + nOffInstData);

    if( nPos < pClassData->nEntries )
        return( pClassData->pEntries[ nPos ].aInst );
    return RSCINST();
}

/*************************************************************************
|*
|*    RscBaseCont::MovePosEle()
|*
|*    Beschreibung
|*    Ersterstellung    MM 23.10.91
|*    Letzte Aenderung  MM 23.10.91
|*
*************************************************************************/
ERRTYPE RscBaseCont::MovePosEle( const RSCINST & rInst, USHORT nDestPos,
                                 USHORT nSourcePos )
{
    ERRTYPE aError;
    RscBaseContInst * pClassData;

    pClassData = (RscBaseContInst *)(rInst.pData + nOffInstData);

    if( (nDestPos < pClassData->nEntries) && (nSourcePos < pClassData->nEntries) ){
        ENTRY_STRUCT aEntry;
        short        nInc = 1;
        USHORT       i;

        // Quelle Merken
        aEntry = pClassData->pEntries[ nSourcePos ];
        // Richtung der for-Schleife bestimmen
        if( nDestPos < nSourcePos )
            nInc = -1;

        for( i = nSourcePos; i != nDestPos; i += nInc )
            pClassData->pEntries[ i ] = pClassData->pEntries[ i + nInc ];

        // Zuweisung Quelle auf Ziel
        pClassData->pEntries[ nDestPos ] = aEntry;
    }
    else
        aError = ERR_RSCCONT;
    return aError;
}

/*************************************************************************
|*
|*    RscBaseCont::SetPosRscId()
|*
|*    Beschreibung
|*    Ersterstellung    MM 30.09.91
|*    Letzte Aenderung  MM 30.09.91
|*
*************************************************************************/
ERRTYPE RscBaseCont::SetPosRscId( const RSCINST & rInst, USHORT nPos,
                                  const RscId & rId ){
    RscBaseContInst * pClassData;
    RSCINST           aTmpI;
    ERRTYPE           aError;

    pClassData = (RscBaseContInst *)(rInst.pData + nOffInstData);

    if( nPos < pClassData->nEntries ){
        if( ! (rId == pClassData->pEntries[ nPos ].aName) )
            aTmpI = SearchEle( rInst, rId,
                               pClassData->pEntries[ nPos ].aInst.pClass );
        if( !aTmpI.IsInst() )
            pClassData->pEntries[ nPos ].aName = rId;
        else
            aError = ERR_RSCCONT;
    }
    else
        aError = ERR_RSCCONT;
    return( aError );
}

/*************************************************************************
|*
|*    RscBaseCont::GetInfoEle()
|*
|*    Beschreibung
|*    Ersterstellung    MM 08.05.91
|*    Letzte Aenderung  MM 08.05.91
|*
*************************************************************************/
SUBINFO_STRUCT RscBaseCont::GetInfoEle( const RSCINST & rInst, USHORT nPos ){
    RscBaseContInst * pClassData;
    SUBINFO_STRUCT  aInfo;

    pClassData = (RscBaseContInst *)(rInst.pData + nOffInstData);

    if( nPos < pClassData->nEntries ){
        aInfo.aId    = pClassData->pEntries[ nPos ].aName;
        aInfo.nPos   = nPos;
        aInfo.pClass = pClassData->pEntries[ nPos ].aInst.pClass;
    }
    return( aInfo );
}

/*************************************************************************
|*
|*    RscBaseCont::SetString()
|*
|*    Beschreibung
|*    Ersterstellung    MM 29.04.91
|*    Letzte Aenderung  MM 29.04.91
|*
*************************************************************************/
ERRTYPE RscBaseCont::SetString( const RSCINST & rInst, char * pStr ){
    RscBaseContInst * pClassData;
    RSCINST aTmpI;
    ERRTYPE aError;
    char    *pTmpStr;

    pClassData = (RscBaseContInst *)(rInst.pData + nOffInstData);

//?B 040991
//?W 040991 sonst Endlosrekursion moeglich
if( RSC_NOTYPE == pTypeClass->GetTypId() ){
    aError = GetElement( rInst, RscId(), pTypeClass, RSCINST(), &aTmpI );
    aError = aTmpI.pClass->GetString( aTmpI, &pTmpStr );
    if( aError.IsOk() )
        aError = aTmpI.pClass->SetString( aTmpI, pStr );
    else {
        aError.Clear();
        DeletePos( rInst, pClassData->nEntries -1 );
        aError = GetElement( rInst, RscId(), pTypeClass1, RSCINST(), &aTmpI );
        aError = aTmpI.pClass->GetString( aTmpI, &pTmpStr );
        if( aError.IsOk() )
            aError = aTmpI.pClass->SetString( aTmpI, pStr );
    }
    if( aError.IsError() )
        DeletePos( rInst, pClassData->nEntries -1 );
}
else
    aError = ERR_UNKNOWN_METHOD;

    return( aError );
}

/*************************************************************************
|*
|*    RscBaseCont::SetNumber()
|*
|*    Beschreibung
|*    Ersterstellung    MM 12.06.91
|*    Letzte Aenderung  MM 12.06.91
|*
*************************************************************************/
ERRTYPE RscBaseCont::SetNumber( const RSCINST & rInst, long lValue ){
    RscBaseContInst * pClassData;
    RSCINST aTmpI;
    ERRTYPE aError;
    long    lNumber;

    pClassData = (RscBaseContInst *)(rInst.pData + nOffInstData);

//?B 040991
//?W 040991 sonst Endlosrekursion moeglich
if( RSC_NOTYPE == pTypeClass->GetTypId() ){
    aError = GetElement( rInst, RscId(), pTypeClass, RSCINST(), &aTmpI );
    aError = aTmpI.pClass->GetNumber( aTmpI, &lNumber );
    if( aError.IsOk() )
        aError = aTmpI.pClass->SetNumber( aTmpI, lValue );
    else {
        aError.Clear();
        DeletePos( rInst, pClassData->nEntries -1 );
        aError = GetElement( rInst, RscId(), pTypeClass1, RSCINST(), &aTmpI );
        aError = aTmpI.pClass->GetNumber( aTmpI, &lNumber );
        if( aError.IsOk() )
            aError = aTmpI.pClass->SetNumber( aTmpI, lValue );
    }
    if( aError.IsError() )
        DeletePos( rInst, pClassData->nEntries -1 );
}
else
    aError = ERR_UNKNOWN_METHOD;

    return( aError );
}

//==================================================================
ERRTYPE RscBaseCont::SetBool
(
    const RSCINST & rInst,
    BOOL bValue
)
{
    RscBaseContInst * pClassData;
    RSCINST aTmpI;
    ERRTYPE aError;
    BOOL    bBool;

    pClassData = (RscBaseContInst *)(rInst.pData + nOffInstData);

//?B 040991
//?W 040991 sonst Endlosrekursion moeglich
if( RSC_NOTYPE == pTypeClass->GetTypId() )
{
    aError = GetElement( rInst, RscId(), pTypeClass, RSCINST(), &aTmpI );
    aError = aTmpI.pClass->GetBool( aTmpI, &bBool );
    if( aError.IsOk() )
        aError = aTmpI.pClass->SetBool( aTmpI, bValue );
    else {
        aError.Clear();
        DeletePos( rInst, pClassData->nEntries -1 );
        aError = GetElement( rInst, RscId(), pTypeClass1, RSCINST(), &aTmpI );
        aError = aTmpI.pClass->GetBool( aTmpI, &bBool );
        if( aError.IsOk() )
            aError = aTmpI.pClass->SetBool( aTmpI, bValue );
    }
    if( aError.IsError() )
        DeletePos( rInst, pClassData->nEntries -1 );
}
else
    aError = ERR_UNKNOWN_METHOD;

    return( aError );
}

//==================================================================
ERRTYPE RscBaseCont::SetConst
(
    const RSCINST & rInst,
    HASHID nValueId,
    long lValue
)
{
    RscBaseContInst * pClassData;
    RSCINST aTmpI;
    ERRTYPE aError;
    HASHID  nConst;

    pClassData = (RscBaseContInst *)(rInst.pData + nOffInstData);

//?B 040991
//?W 040991 sonst Endlosrekursion moeglich
if( RSC_NOTYPE == pTypeClass->GetTypId() ){
    aError = GetElement( rInst, RscId(), pTypeClass, RSCINST(), &aTmpI );
    aError = aTmpI.pClass->GetConst( aTmpI, &nConst );
    if( aError.IsOk() )
        aError = aTmpI.pClass->SetConst( aTmpI, nValueId, lValue );
    else {
        aError.Clear();
        DeletePos( rInst, pClassData->nEntries -1 );
        aError = GetElement( rInst, RscId(), pTypeClass1, RSCINST(), &aTmpI );
        aError = aTmpI.pClass->GetConst( aTmpI, &nConst );
        if( aError.IsOk() )
            aError = aTmpI.pClass->SetConst( aTmpI, nValueId, lValue );
    }
    if( aError.IsError() )
        DeletePos( rInst, pClassData->nEntries -1 );
}
else
    aError = ERR_UNKNOWN_METHOD;

    return( aError );
}

/*************************************************************************
|*
|*    RscBaseCont::SetRef()
|*
|*    Beschreibung
|*    Ersterstellung    MM 02.12.91
|*    Letzte Aenderung  MM 02.12.91
|*
*************************************************************************/
ERRTYPE RscBaseCont::SetRef( const RSCINST & rInst, const RscId & rRefId ){
    RscBaseContInst * pClassData;
    RSCINST aTmpI;
    ERRTYPE aError;
    RscId   aId;

    pClassData = (RscBaseContInst *)(rInst.pData + nOffInstData);

//?B 040991
//?W 040991 sonst Endlosrekursion moeglich
if( RSC_NOTYPE == pTypeClass->GetTypId() ){
    aError = GetElement( rInst, RscId(), pTypeClass, RSCINST(), &aTmpI );
    aError = aTmpI.pClass->GetRef( aTmpI, &aId );
    if( aError.IsOk() )
        aError = aTmpI.pClass->SetRef( aTmpI, rRefId );
    else {
        aError.Clear();
        DeletePos( rInst, pClassData->nEntries -1 );
        aError = GetElement( rInst, RscId(), pTypeClass1, RSCINST(), &aTmpI );
        aError = aTmpI.pClass->GetRef( aTmpI, &aId );
        if( aError.IsOk() )
            aError = aTmpI.pClass->SetNumber( aTmpI, rRefId );
    }
    if( aError.IsError() )
        DeletePos( rInst, pClassData->nEntries -1 );
}
else
    aError = ERR_UNKNOWN_METHOD;

    return( aError );
}

/*************************************************************************
|*
|*    RscBaseCont::IsConsistent()
|*
|*    Beschreibung
|*    Ersterstellung    MM 23.09.91
|*    Letzte Aenderung  MM 23.09.91
|*
*************************************************************************/
BOOL RscBaseCont::IsConsistent( const RSCINST & rInst, RscInconsList * pList ){
    USHORT  i;
    RscBaseContInst * pClassData;
    BOOL    bRet;

    bRet = RscTop::IsConsistent( rInst, pList );

    pClassData = (RscBaseContInst *)(rInst.pData + nOffInstData);

    // auf doppelten Id Testen und Reihenfolge beibehalten
    // Komplexitaet = n^2 / 2
    for( i = 0; i < pClassData->nEntries; i++ ){
        if( !bNoId ){
            if( (long)pClassData->pEntries[ i ].aName > 0x7FFF
              || (long)pClassData->pEntries[ i ].aName < 1 ){
                bRet = FALSE;
                if( pList )
                    pList->Insert( new RscInconsistent(
                        pClassData->pEntries[ i ].aName,
                        pClassData->pEntries[ i ].aName ) );
            }
            else if( SearchElePos( rInst, pClassData->pEntries[ i ].aName,
                pClassData->pEntries[ i ].aInst.pClass, i +1 ).IsInst() )
            {
                 bRet = FALSE;
                 if( pList )
                     pList->Insert( new RscInconsistent(
                         pClassData->pEntries[ i    ].aName,
                         pClassData->pEntries[ i +1 ].aName ) );
             };
        }
        if( ! pClassData->pEntries[ i ].aInst.pClass->
              IsConsistent( pClassData->pEntries[ i ].aInst, pList ) )
            bRet = FALSE;
    };

    return( bRet );
}

/*************************************************************************
|*
|*    RscBaseCont::SetToDefault()
|*
|*    Beschreibung
|*    Ersterstellung    MM 25.04.91
|*    Letzte Aenderung  MM 25.04.91
|*
*************************************************************************/
void RscBaseCont::SetToDefault( const RSCINST & rInst )
{
    USHORT  i;
    RscBaseContInst * pClassData;

    pClassData = (RscBaseContInst *)(rInst.pData + nOffInstData);

    for( i = 0; i < pClassData->nEntries; i++ )
    {
        pClassData->pEntries[ i ].aInst.pClass->
                  SetToDefault( pClassData->pEntries[ i ].aInst );
    };

    RscTop::SetToDefault( rInst );
}

/*************************************************************************
|*
|*    RscBaseCont::IsDefault()
|*
|*    Beschreibung
|*    Ersterstellung    MM 25.04.91
|*    Letzte Aenderung  MM 25.04.91
|*
*************************************************************************/
BOOL RscBaseCont::IsDefault( const RSCINST & rInst ){
    USHORT  i;
    RscBaseContInst * pClassData;

    pClassData = (RscBaseContInst *)(rInst.pData + nOffInstData);

    if( !pClassData->bDflt )
        return FALSE;

    for( i = 0; i < pClassData->nEntries; i++ ){
        if( ! pClassData->pEntries[ i ].aInst.pClass->
                  IsDefault( pClassData->pEntries[ i ].aInst ) )
        {
            return( FALSE );
        };
    };

    return( RscTop::IsDefault( rInst ) );
}

/*************************************************************************
|*
|*    RscBaseCont::IsValueDefault()
|*
|*    Beschreibung
|*    Ersterstellung    MM 25.04.91
|*    Letzte Aenderung  MM 15.01.92
|*
*************************************************************************/
BOOL RscBaseCont::IsValueDefault( const RSCINST & rInst, CLASS_DATA pDef )
{
    RscBaseContInst * pClassData;

    if( !RscTop::IsValueDefault( rInst, pDef ) )
        return FALSE;

    pClassData = (RscBaseContInst *)(rInst.pData + nOffInstData);

    if( pClassData->nEntries )
        return FALSE;
    else
        return TRUE;
}

/*************************************************************************
|*
|*    RscBaseCont::Delete()
|*
|*    Beschreibung
|*    Ersterstellung    MM 12.06.91
|*    Letzte Aenderung  MM 12.06.91
|*
*************************************************************************/
void RscBaseCont::Delete( const RSCINST & rInst, RscTop * pClass,
                          const RscId & rId )
{
    USHORT  i;
    RscBaseContInst * pClassData;

    pClassData = (RscBaseContInst *)(rInst.pData + nOffInstData);
    if( !pClass )
        pClass = pTypeClass;

    for( i = 0; i < pClassData->nEntries; i++ ){
        if( pClassData->pEntries[ i ].aName == rId )
            if( pClassData->pEntries[ i ].aInst.pClass == pClass || !pClass ){
                DeletePos( rInst, i );
                return;
            };
    };

}

/*************************************************************************
|*
|*    RscBaseCont::DeletePos()
|*
|*    Beschreibung
|*    Ersterstellung    MM 12.06.91
|*    Letzte Aenderung  MM 12.06.91
|*
*************************************************************************/
void RscBaseCont::DeletePos( const RSCINST & rInst, USHORT nPos ){
    RscBaseContInst *   pClassData;
    USHORT              i;

    pClassData = (RscBaseContInst *)(rInst.pData + nOffInstData);

    if( nPos < pClassData->nEntries ){
        if( 1 == pClassData->nEntries )
            DestroyElements( pClassData );
        else{
            pClassData->pEntries[ nPos ].Destroy();
            pClassData->nEntries--;
            for( i = nPos; i < pClassData->nEntries; i++ )
                pClassData->pEntries[ i ] = pClassData->pEntries[ i + 1 ];
        };
    };
}

/*************************************************************************
|*
|*    RscBaseCont::ContWriteSrc()
|*
|*    Beschreibung
|*    Ersterstellung    MM 26.04.91
|*    Letzte Aenderung  MM 26.04.91
|*
*************************************************************************/
void RscBaseCont::ContWriteSrc( const RSCINST & rInst, FILE * fOutput,
                                RscTypCont * pTC, USHORT nTab,
                                const char * pVarName )
{
    USHORT  i, t;
    RscBaseContInst * pClassData;

    pClassData = (RscBaseContInst *)(rInst.pData + nOffInstData);

    for( i = 0; i < pClassData->nEntries; i++ )
    {
        for( t = 0; t < nTab; t++ )
            fputc( '\t', fOutput );
        pClassData->pEntries[ i ].aInst.pClass->
                  WriteSrcHeader( pClassData->pEntries[ i ].aInst,
                                  fOutput, pTC, nTab,
                                  pClassData->pEntries[ i ].aName, pVarName );
        fprintf( fOutput, ";\n" );
    };
}

/*************************************************************************
|*
|*    RscBaseCont::ContWriteRc()
|*
|*    Beschreibung
|*    Ersterstellung    MM 26.04.91
|*    Letzte Aenderung  MM 26.04.91
|*
*************************************************************************/
ERRTYPE RscBaseCont::ContWriteRc( const RSCINST & rInst, RscWriteRc & rMem,
                              RscTypCont * pTC, USHORT nDeep, BOOL bExtra )
{
    USHORT        i;
    RscBaseContInst * pClassData;
    ERRTYPE       aError;

    if( bExtra || bNoId )
    { // Nur Subresourcen schreiben, wenn bExtra == TRUE
        pClassData = (RscBaseContInst *)(rInst.pData + nOffInstData);

        for( i = 0; i < pClassData->nEntries && aError.IsOk(); i++ ){
            aError = pClassData->pEntries[ i ].aInst.pClass->
                         WriteRcHeader( pClassData->pEntries[ i ].aInst,
                                        rMem, pTC,
                                        pClassData->pEntries[ i ].aName,
                                        nDeep, bExtra );
        }
    }

    return( aError );
}

//==================================================================
void RscBaseCont::ContWriteRcAccess
(
    FILE * fOutput,
    RscTypCont * pTC,
    const char * pName,
    BOOL bWriteSize
)
{
    if( (bNoId || bWriteSize) && !pTypeClass1 )
    {
        fprintf( fOutput, "\t\tUSHORT nItems = *(USHORT *)(pResData+nOffset) );\n" );
        fprintf( fOutput, "\t\tnOffset += sizeof( USHORT );\n" );

        fprintf( fOutput, "\t\t// Items hinzufuegen\n" );
        fprintf( fOutput, "\t\tfor( USHORT i = 0; i < nItems; i++ )\n" );
        fprintf( fOutput, "\t\t{\n" );
        pTypeClass->WriteRcAccess( fOutput, pTC, "Item" );
        fprintf( fOutput, "\t\t}\n" );
    }
    else
    {
        fprintf( fOutput, "\t\tSet%s( %s ", pName, pHS->Get( GetId() ) );
        fprintf( fOutput, "%s ", aCallPar2.GetBuffer() );
        fprintf( fOutput, "ResId( (RSHEADER_TYPE*)(pResData+nOffset) ) ) );\n" );
        fprintf( fOutput, "\t\tnOffset += GetObjSizeRes( (RSHEADER_TYPE*)(pResData+nOffset) );\n" );
    }
}


/*************************************************************************
|*
|*    RscBaseCont::WriteSrc()
|*
|*    Beschreibung
|*    Ersterstellung    MM 26.04.91
|*    Letzte Aenderung  MM 26.04.91
|*
*************************************************************************/
void RscBaseCont::WriteSrc( const RSCINST & rInst, FILE * fOutput,
                            RscTypCont * pTC, USHORT nTab,
                            const char * pVarName )
{
    RscTop::WriteSrc( rInst, fOutput, pTC, nTab, pVarName );
    ContWriteSrc( rInst, fOutput, pTC, nTab, pVarName );
}

/*************************************************************************
|*
|*    RscBaseCont::WriteRc()
|*
|*    Beschreibung
|*    Ersterstellung    MM 26.04.91
|*    Letzte Aenderung  MM 26.04.91
|*
*************************************************************************/
ERRTYPE RscBaseCont::WriteRc( const RSCINST & rInst, RscWriteRc & rMem,
                              RscTypCont * pTC, USHORT nDeep, BOOL bExtra )
{
    ERRTYPE       aError;

    aError = RscTop::WriteRc( rInst, rMem, pTC, nDeep, bExtra );
    if( aError.IsOk() )
        aError = ContWriteRc( rInst, rMem, pTC, nDeep, bExtra );

    return( aError );
}

/*************************************************************************
|*
|*    RscBaseCont::WriteHxx()
|*
|*    Beschreibung
|*    Ersterstellung    MM 26.04.91
|*    Letzte Aenderung  MM 26.04.91
|*
*************************************************************************/
ERRTYPE RscBaseCont::WriteHxx( const RSCINST & rInst, FILE * fOutput,
                               RscTypCont * pTC, const RscId & rRscId )
{
    USHORT        i;
    RscBaseContInst * pClassData;
    ERRTYPE       aError;

    aError = RscTop::WriteHxx( rInst, fOutput, pTC, rRscId );

    pClassData = (RscBaseContInst *)(rInst.pData + nOffInstData);

    for( i = 0; i < pClassData->nEntries && aError.IsOk(); i++ )
    {
        if( pClassData->pEntries[ i ].aName.IsId()
          && pClassData->pEntries[ i ].aInst.pClass->IsCodeWriteable() )
        {
            aError = pClassData->pEntries[ i ].aInst.pClass->
                         WriteHxx( pClassData->pEntries[ i ].aInst,
                                   fOutput, pTC,
                                   pClassData->pEntries[ i ].aName );
        }
    };

    return( aError );
}

/*************************************************************************
|*
|*    RscBaseCont::WriteCxx()
|*
|*    Beschreibung
|*    Ersterstellung    MM 26.04.91
|*    Letzte Aenderung  MM 26.04.91
|*
*************************************************************************/
ERRTYPE RscBaseCont::WriteCxx( const RSCINST & rInst, FILE * fOutput,
                               RscTypCont * pTC, const RscId & rRscId )
{
    USHORT        i;
    RscBaseContInst * pClassData;
    ERRTYPE       aError;

    aError = RscTop::WriteCxx( rInst, fOutput, pTC, rRscId );

    pClassData = (RscBaseContInst *)(rInst.pData + nOffInstData);

    for( i = 0; i < pClassData->nEntries && aError.IsOk(); i++ )
    {
        if( pClassData->pEntries[ i ].aName.IsId()
          && pClassData->pEntries[ i ].aInst.pClass->IsCodeWriteable() )
        {
            aError = pClassData->pEntries[ i ].aInst.pClass->
                         WriteCxx( pClassData->pEntries[ i ].aInst,
                                   fOutput, pTC,
                                   pClassData->pEntries[ i ].aName );
        }
    };

    return( aError );
}

/*************** R s c C o n t W r i t e S r c ***************************/
/*************************************************************************
|*
|*    RscContWriteSrc::RscContWriteSrc()
|*
|*    Beschreibung
|*    Ersterstellung    MM 08.05.91
|*    Letzte Aenderung  MM 08.05.91
|*
*************************************************************************/
RscContWriteSrc::RscContWriteSrc( HASHID nId, USHORT nTypeId,
                                 RscTop * pSuper, BOOL bNoIdent )
                  : RscBaseCont( nId, nTypeId, pSuper, bNoIdent )
{}

/*************************************************************************
|*
|*    RscContWriteSrc::WriteSrc()
|*
|*    Beschreibung
|*    Ersterstellung    MM 08.05.91
|*    Letzte Aenderung  MM 08.05.91
|*
*************************************************************************/
void RscContWriteSrc::WriteSrc( const RSCINST & rInst, FILE * fOutput,
                                RscTypCont * pTC, USHORT nTab,
                                const char * pVarName )
{
    USHORT  i;

    RscTop::WriteSrc( rInst, fOutput, pTC, nTab, pVarName );

    fprintf( fOutput, "\n" );
    for( i = 0; i < nTab; i++ )
        fputc( '\t', fOutput );
    fprintf( fOutput, "{\n" );

    ContWriteSrc( rInst, fOutput, pTC, nTab +1, pVarName );

    for( i = 0; i < nTab; i++ )
        fputc( '\t', fOutput );
    fprintf( fOutput, "}" );
}

/*************** R s c C o n t *******************************************/
/*************************************************************************
|*
|*    RscCont::RscCont()
|*
|*    Beschreibung
|*    Ersterstellung    MM 08.05.91
|*    Letzte Aenderung  MM 08.05.91
|*
*************************************************************************/
RscCont::RscCont( HASHID nId, USHORT nTypeId, RscTop * pSuper, BOOL bNoIdent )
              : RscContWriteSrc( nId, nTypeId, pSuper, bNoIdent )
{}

/*************************************************************************
|*
|*    RscCont::WriteRc()
|*
|*    Beschreibung
|*    Ersterstellung    MM 08.05.91
|*    Letzte Aenderung  MM 08.05.91
|*
*************************************************************************/
ERRTYPE RscCont::WriteRc( const RSCINST & rInst, RscWriteRc & rMem,
                                 RscTypCont * pTC, USHORT nDeep, BOOL bExtra )
{
    RscBaseContInst * pClassData;
    ERRTYPE aError;

    aError = RscTop::WriteRc( rInst, rMem, pTC, nDeep, bExtra );

    pClassData = (RscBaseContInst *)(rInst.pData + nOffInstData);

    rMem.Put( pClassData->nEntries );

    if( aError.IsOk() )
        aError = ContWriteRc( rInst, rMem, pTC, nDeep, bExtra );

    return aError;
}

//==================================================================
void RscCont::WriteRcAccess
(
    FILE * fOutput,
    RscTypCont * pTC,
    const char * pName
)
{
    ContWriteRcAccess( fOutput, pTC, pName, TRUE );
}

/*************************************************************************
|*
|*    RscContExtraData::RscContExtraData()
|*
|*    Beschreibung
|*    Ersterstellung    MM 08.05.91
|*    Letzte Aenderung  MM 08.05.91
|*
*************************************************************************/
RscContExtraData::RscContExtraData( HASHID nId, USHORT nTypeId,
                             RscTop * pSuper, BOOL bNoIdent )
                     : RscContWriteSrc( nId, nTypeId, pSuper, bNoIdent )
{}

/*************************************************************************
|*
|*    RscContExtraData::WriteRc()
|*
|*    Beschreibung
|*    Ersterstellung    MM 08.05.91
|*    Letzte Aenderung  MM 08.05.91
|*
*************************************************************************/
ERRTYPE RscContExtraData::WriteRc( const RSCINST & rInst, RscWriteRc & rMem,
                             RscTypCont * pTC, USHORT nDeep, BOOL bExtra )
{
    ERRTYPE aError;

    if( bExtra )
        aError = RscContWriteSrc::WriteRc( rInst, rMem, pTC, nDeep, bExtra );
    else
        aError = RscTop::WriteRc( rInst, rMem, pTC, nDeep, bExtra );

    return aError;
}
