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
#include <ctype.h>

#include <rscmgr.hxx>
#include <rscdb.hxx>

RscMgr::RscMgr( Atom nId, sal_uInt32 nTypeId, RscTop * pSuperCl )
            : RscClass( nId, nTypeId, pSuperCl )
{
}

sal_uInt32 RscMgr::Size()
{
    return RscClass::Size() + ALIGNED_SIZE( sizeof( RscMgrInst ) );
}

RSCINST RscMgr::Create( RSCINST * pInst, const RSCINST & rDflt, sal_Bool bOwnClass ){
    RSCINST aInst;
    RscMgrInst * pClassData;

    if( !pInst ){
        aInst.pClass = this;
        aInst.pData = (CLASS_DATA) rtl_allocateMemory( Size() );
    }
    else
        aInst = *pInst;
    if( !bOwnClass && rDflt.IsInst() )
        bOwnClass = rDflt.pClass->InHierarchy( this );

    RscClass::Create( &aInst, rDflt, bOwnClass );

    pClassData = (RscMgrInst *)(aInst.pData + RscClass::Size() );
    pClassData->Create();

    if( bOwnClass ){
        RscMgrInst * pDfltData = (RscMgrInst *)(rDflt.pData + RscClass::Size());
        *pClassData = *pDfltData;
    };

    return( aInst );
}

void RscMgr::Destroy( const RSCINST & rInst ){
    RscMgrInst * pClassData;

    RscClass::Destroy( rInst );

    pClassData = (RscMgrInst *)(rInst.pData + RscClass::Size());
    pClassData->Destroy();
}

void RscMgr::SetToDefault( const RSCINST & rInst )
{
    RscMgrInst * pClassData;

    pClassData = (RscMgrInst *)(rInst.pData + RscClass::Size());
    pClassData->bDflt = sal_True;

    RscClass::SetToDefault( rInst );
}

sal_Bool RscMgr::IsDefault( const RSCINST & rInst ){
    RscMgrInst * pClassData;

    pClassData = (RscMgrInst *)(rInst.pData + RscClass::Size());
    if( !pClassData->bDflt )
        return( sal_False );

    return( RscClass::IsDefault( rInst ) );
}

sal_Bool RscMgr::IsValueDefault( const RSCINST & rInst, CLASS_DATA pDef ){
    if( !RscClass::IsValueDefault( rInst, pDef ) )
        return sal_False;

    if( pDef ){
        RscMgrInst * pClassData = (RscMgrInst *)(rInst.pData + RscClass::Size());
        RscMgrInst * pDfltData  = (RscMgrInst *)(pDef + RscClass::Size());

        if( !pClassData->aRefId.IsId() && !pDfltData->aRefId.IsId() ){
            return sal_True;
        }
    }

    return sal_False;
}


void RscMgr::WriteSrcHeader( const RSCINST & rInst, FILE * fOutput,
                             RscTypCont * pTC, sal_uInt32 nTab,
                             const RscId & rId, const char * pVarName )
{
    RscMgrInst * pClassData;
    sal_uInt32       i;

    pClassData = (RscMgrInst *)(rInst.pData + RscClass::Size());

    fprintf( fOutput, "%s %s",
             pHS->getString( rInst.pClass->GetId() ).getStr(),
             (rId.GetName()).getStr() );
    if( pClassData->aRefId.IsId() )
        fprintf( fOutput, ",%s", pClassData->aRefId.GetName().getStr() );
    else
    {
        fprintf( fOutput, "\n" );
        for( i = 0; i < nTab; i++ )
            fputc( '\t', fOutput );
        fprintf( fOutput, "{\n" );

        rInst.pClass->WriteSrc( rInst, fOutput, pTC, nTab +1, pVarName );

        RscClass::WriteSrc( rInst, fOutput, pTC, nTab +1, pVarName);

        for( i = 0; i < nTab; i++ )
            fputc( '\t', fOutput );
        fprintf( fOutput, "}" );
    }
}

void RscMgr::WriteSrc( const RSCINST &, FILE *, RscTypCont *, sal_uInt32,
                        const char * )
{
}

ERRTYPE RscMgr::WriteRcHeader( const RSCINST & rInst, RscWriteRc & rMem,
                               RscTypCont * pTC, const RscId &rId,
                               sal_uInt32 nDeep, sal_Bool bExtra )
{
    RscMgrInst *    pClassData;
    ERRTYPE         aError;
    ObjNode *       pObjNode = NULL;

    pClassData = (RscMgrInst *)(rInst.pData + RscClass::Size());

    if( pClassData->aRefId.IsId() )
    {
        //Erhoehen und abfragen um Endlosrekusion zu vermeiden
        nDeep++;
        if( nDeep > nRefDeep )
            aError = ERR_REFTODEEP;
        else
            pObjNode = rInst.pClass->GetRefClass()->
                                        GetObjNode( pClassData->aRefId );
        if( !pObjNode && pTC )
        {
            OStringBuffer aMsg(pHS->getString(rInst.pClass->GetId()));
            aMsg.append(' ').append(pClassData->aRefId.GetName());
            aError = WRN_MGR_REFNOTFOUND;
            pTC->pEH->Error(aError, rInst.pClass, rId, aMsg.getStr());
        }
    }

    if( aError.IsOk() )
    {
        if( pObjNode )
        {
            RSCINST     aRefI;
            RscTop *    pTmpRefClass = rInst.pClass->GetRefClass();

            aRefI = RSCINST( rInst.pClass, pObjNode->GetRscObj() );
            if( pTmpRefClass == rInst.pClass )
            {
                aError = aRefI.pClass->WriteRcHeader( aRefI, rMem, pTC,
                                                       rId, nDeep, bExtra );
            }
            else
            {
                RSCINST aRefInst = rInst.pClass->Create( NULL, aRefI );
                aError = aRefI.pClass->WriteRcHeader( aRefInst, rMem, pTC,
                                                       rId, nDeep, bExtra );
                pTmpRefClass->Destroy( aRefInst );
            }
        }
        else
        {
            sal_uInt32          nOldSize;
            sal_uInt32          nLocalSize;

            nOldSize = rMem.IncSize( 16 /*sizeof( RSHEADER_TYPE )*/ );

            aError = rInst.pClass->WriteRc( rInst, rMem, pTC, nDeep, bExtra );
            if( aError.IsOk() )
                aError = WriteInstRc( rInst, rMem, pTC, nDeep, bExtra );
            nLocalSize = rMem.Size();

            if( aError.IsOk() )
            {
                // RscClass wird uebersprungen
                aError = RscTop::WriteRc( rInst, rMem, pTC, nDeep, bExtra );
            };

            /*
            // Definition der Struktur, aus denen die Resource aufgebaut ist
            struct RSHEADER_TYPE{
                RESOURCE_TYPE   nRT;        // Resource Typ
                sal_uInt32          nRT;        // Resource Typ
                sal_uInt32          nGlobOff;   // Globaler Offset
                sal_uInt32          nLocalOff;  // Lokaler Offset
            };
            */
            sal_uInt32 nID = rId;
            rMem.PutAt( nOldSize, nID );
            rMem.PutAt( nOldSize +4, (sal_uInt32)rInst.pClass->GetTypId() );
            rMem.PutAt( nOldSize +8, (sal_uInt32)(rMem.Size() - nOldSize) );
            rMem.PutAt( nOldSize +12, (sal_uInt32)(nLocalSize - nOldSize) );
        };
    };

    return( aError );
}

ERRTYPE RscMgr::WriteRc( const RSCINST &, RscWriteRc &,
                         RscTypCont *, sal_uInt32, sal_Bool )

{
    return( ERR_OK );
}

sal_Bool RscMgr::IsConsistent( const RSCINST & rInst )
{
    sal_Bool    bRet;
    RscMgrInst * pClassData;

    bRet = RscClass::IsConsistent( rInst );

    pClassData = (RscMgrInst *)(rInst.pData + RscClass::Size());
    if( pClassData->aRefId.IsId() &&
      ((pClassData->aRefId.GetNumber() < 1)
        || (pClassData->aRefId.GetNumber() > 0x7FFF)
        || IsToDeep( rInst ).IsError()) )
    {
        bRet = sal_False;
    }

    return( bRet );
}

ERRTYPE RscMgr::GetRef( const RSCINST & rInst, RscId * pRscId ){
    RscMgrInst * pClassData;

    pClassData = (RscMgrInst *)(rInst.pData + RscClass::Size());
    *pRscId = pClassData->aRefId;
    return ERR_OK;
}

ERRTYPE RscMgr::IsToDeep( const RSCINST & rInst, sal_uInt32 nDeep )
{
    RscMgrInst *    pClassData;
    RscId           aOldId, aId;
    ERRTYPE         aError;
    RSCINST         aTmpI = rInst;
    ObjNode *       pObjNode;

    pClassData = (RscMgrInst *)(rInst.pData + RscClass::Size());

    while( aTmpI.IsInst() && (nDeep < nRefDeep) && aError.IsOk() )
    {
        // Referenz holen
        aTmpI.pClass->GetRef( aTmpI, &aId );
        // Referenziertes Objekt holen
        pObjNode = aTmpI.pClass->GetObjNode( aId );
        // Referenzierte Objekt gefunden ?
        if( pObjNode )
        {
            aTmpI.pData = pObjNode->GetRscObj();
            nDeep++;
        }
        else //aTmpI.IsInst() wird sal_False, Schleife beenden
            aTmpI.pData = NULL;
    }

    if( nDeep >= nRefDeep )
    {
        pClassData->aRefId = aOldId;
        aError             = ERR_REFTODEEP;
    }

    return( aError );
}

ERRTYPE RscMgr::SetRef( const RSCINST & rInst, const RscId & rRefId )
{
    RscMgrInst *    pClassData;
    RscId           aOldId, aId;
    ERRTYPE         aError;

    if( rRefId.IsId() &&
      ((rRefId.GetNumber() < 1) || (rRefId.GetNumber() > 0x7FFF)) )
    {
        aError = ERR_IDRANGE;
    }
    else
    {
        pClassData = (RscMgrInst *)(rInst.pData + RscClass::Size());
        aOldId = pClassData->aRefId;// Alten Wert merken
        pClassData->aRefId = rRefId;// vorher eintragen,
                                    // sonst Fehler bei rekursion


        aError = IsToDeep( rInst );
        if( aError.IsOk() )
            pClassData->bDflt  = sal_False;
        else
            pClassData->aRefId = aOldId;
    }

    return( aError );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
