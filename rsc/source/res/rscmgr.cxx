/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#include <ctype.h>

// Programmabhaengige Includes.
#include <rscmgr.hxx>
#include <rscdb.hxx>

/****************** C O D E **********************************************/
/****************** R s c M g r ******************************************/
/*************************************************************************
|*
|*    RscMgr::RscMgr()
|*
*************************************************************************/
RscMgr::RscMgr( Atom nId, sal_uInt32 nTypeId, RscTop * pSuperCl )
            : RscClass( nId, nTypeId, pSuperCl )
{
}

/*************************************************************************
|*
|*    RscMgr::Size()
|*
*************************************************************************/
sal_uInt32 RscMgr::Size()
{
    return RscClass::Size() + ALIGNED_SIZE( sizeof( RscMgrInst ) );
}

/*************************************************************************
|*
|*    RscMgr::Create()
|*
*************************************************************************/
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

/*************************************************************************
|*
|*    RscMgr::Destroy()
|*
*************************************************************************/
void RscMgr::Destroy( const RSCINST & rInst ){
    RscMgrInst * pClassData;

    RscClass::Destroy( rInst );

    pClassData = (RscMgrInst *)(rInst.pData + RscClass::Size());
    pClassData->Destroy();
}

/*************************************************************************
|*
|*    RscMgr::SetToDefault()
|*
*************************************************************************/
void RscMgr::SetToDefault( const RSCINST & rInst )
{
    RscMgrInst * pClassData;

    pClassData = (RscMgrInst *)(rInst.pData + RscClass::Size());
    pClassData->bDflt = sal_True;

    RscClass::SetToDefault( rInst );
}

/*************************************************************************
|*
|*    RscMgr::IsDefault()
|*
*************************************************************************/
sal_Bool RscMgr::IsDefault( const RSCINST & rInst ){
    RscMgrInst * pClassData;

    pClassData = (RscMgrInst *)(rInst.pData + RscClass::Size());
    if( !pClassData->bDflt )
        return( sal_False );

    return( RscClass::IsDefault( rInst ) );
}

/*************************************************************************
|*
|*    RscMgr::IsValueDefault()
|*
*************************************************************************/
sal_Bool RscMgr::IsValueDefault( const RSCINST & rInst, CLASS_DATA pDef ){
    RscMgrInst * pClassData;
    RscMgrInst * pDfltData;

    if( !RscClass::IsValueDefault( rInst, pDef ) )
        return sal_False;

    if( pDef ){
        pClassData = (RscMgrInst *)(rInst.pData + RscClass::Size());
        pDfltData  = (RscMgrInst *)(pDef + RscClass::Size());

        if( !pClassData->aRefId.IsId() && !pDfltData->aRefId.IsId() ){
            return sal_True;
        }
    }

    return sal_False;
}


/*************************************************************************
|*
|*    RscMgr::WriteSrcHeader()
|*
*************************************************************************/
void RscMgr::WriteSrcHeader( const RSCINST & rInst, FILE * fOutput,
                             RscTypCont * pTC, sal_uInt32 nTab,
                             const RscId & rId, const char * pVarName )
{
    RscMgrInst * pClassData;
    sal_uInt32       i;

    pClassData = (RscMgrInst *)(rInst.pData + RscClass::Size());

    fprintf( fOutput, "%s %s",
             pHS->getString( rInst.pClass->GetId() ).getStr(),
             (rId.GetName()).GetBuffer() );
    if( pClassData->aRefId.IsId() )
        fprintf( fOutput, ",%s", pClassData->aRefId.GetName().GetBuffer() );
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

/*************************************************************************
|*
|*    RscMgr::WriteSrc()
|*
*************************************************************************/
void RscMgr::WriteSrc( const RSCINST &, FILE *, RscTypCont *, sal_uInt32,
                        const char * )
{
}

/*************************************************************************
|*
|*    RscMgr::WriteRcHeader()
|*
*************************************************************************/
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
            ByteString aMsg( pHS->getString( rInst.pClass->GetId() ).getStr() );
            aMsg += ' ';
            aMsg += pClassData->aRefId.GetName();
            aError = WRN_MGR_REFNOTFOUND;
            pTC->pEH->Error( aError, rInst.pClass, rId, aMsg.GetBuffer() );
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

/*************************************************************************
|*
|*    RscMgr::WriteRc()
|*
*************************************************************************/
ERRTYPE RscMgr::WriteRc( const RSCINST &, RscWriteRc &,
                         RscTypCont *, sal_uInt32, sal_Bool )

{
    return( ERR_OK );
}


static ByteString MakeSmartName( const ByteString & rDefName )
{
    ByteString aSmartName;
    if( rDefName.Len() )
    {
        char * pStr = (char *)rDefName.GetBuffer();
        aSmartName = (char)toupper( *pStr );
        while( *++pStr )
        {
            if( '_' == *pStr )
            {
                if( *++pStr )
                    aSmartName += (char)toupper( *pStr );
                else
                    break;
            }
            else
                aSmartName += (char)tolower( *pStr );
        }
    }
    return aSmartName;
}

static ByteString MakeName( RscTypCont * pTypCon, RscTop * pClass,
                            const ByteString & rName )
{
    ByteString aRet;
    if( !pTypCon->IsSmart() || isdigit( rName.GetChar(0) ) )
    {
        aRet += pHS->getString( pClass->GetId() ).getStr();
        aRet += rName;
    }
    else
        aRet += MakeSmartName( rName );
    return aRet;
}

/*************************************************************************
|*
|*    RscMgr::WriteHxxHeader()
|*
*************************************************************************/
ERRTYPE RscMgr::WriteHxxHeader( const RSCINST & rInst, FILE * fOutput,
                                RscTypCont * pTC, const RscId &rId )
{
    RscMgrInst *    pClassData;
    ERRTYPE         aError;
    ObjNode *       pObjNode = NULL;

    pClassData = (RscMgrInst *)(rInst.pData + RscClass::Size());

    if( pClassData->aRefId.IsId() )
    {
        pObjNode = rInst.pClass->GetObjNode( pClassData->aRefId );
        if( !pObjNode && pTC )
        {
            ByteString  aMsg( pHS->getString( rInst.pClass->GetId() ).getStr() );
            aMsg += ' ';
            aMsg += pClassData->aRefId.GetName();
            aError = WRN_MGR_REFNOTFOUND;
            pTC->pEH->Error( aError, rInst.pClass, rId, aMsg.GetBuffer() );
        }
    }

    if( pObjNode )
    {
        RSCINST     aRefI;

        aRefI = RSCINST( rInst.pClass, pObjNode->GetRscObj() );
        aError = aRefI.pClass->WriteHxxHeader( aRefI, fOutput, pTC,
                                               rId );
    }
    else if (pTC)
    {
        fprintf( fOutput, "class %s",
                          MakeName( pTC, rInst.pClass,
                                    rId.GetName() ).GetBuffer() );
        fprintf( fOutput, " : public %s",
                 pHS->getString( rInst.pClass->GetId() ).getStr() );
        fprintf( fOutput, "\n{\nprotected:\n" );

        aError = RscClass::WriteHxx( rInst, fOutput, pTC, rId );

        RSCINST aExtraInst = rInst.pClass->GetCopyVar( rInst, nRsc_EXTRADATA );
        if( aExtraInst.IsInst() )
        {
            if( aExtraInst.pClass->GetCount( aExtraInst ) )
                fprintf( fOutput, "    char * pExtraData;\n" );
        }
        if( aError.IsOk() )
        {
            fprintf( fOutput, "public:\n    " );
            fprintf( fOutput, "%s%s bFreeRes = TRUE )",
                     MakeName( pTC, rInst.pClass,
                               rId.GetName() ).GetBuffer(),
                     (rInst.pClass->aCallParType).GetBuffer() );
            fprintf( fOutput, ";\n};\n\n" );
        }
    };
    return aError;
}

/*************************************************************************
|*
|*    RscMgr::WriteHxx()
|*
*************************************************************************/
ERRTYPE RscMgr::WriteHxx( const RSCINST & rInst, FILE * fOutput,
                          RscTypCont * pTC, const RscId & rId )
{
    fprintf( fOutput, "    %s", pHS->getString( rInst.pClass->GetId() ).getStr() );
    fprintf( fOutput, " a%s;\n",
             MakeName( pTC, rInst.pClass, rId.GetName() ).GetBuffer() );

    return ERR_OK;
}

/*************************************************************************
|*
|*    RscClass::WriteCxxHeader()
|*
*************************************************************************/
ERRTYPE RscMgr::WriteCxxHeader( const RSCINST & rInst, FILE * fOutput,
                                RscTypCont * pTC, const RscId & rId )
{
    RscMgrInst *    pClassData;
    ERRTYPE         aError;
    ObjNode *       pObjNode = NULL;

    pClassData = (RscMgrInst *)(rInst.pData + RscClass::Size());

    if( pClassData->aRefId.IsId() )
    {
        pObjNode = rInst.pClass->GetObjNode( pClassData->aRefId );
        if( !pObjNode && pTC )
        {
            ByteString  aMsg( pHS->getString( rInst.pClass->GetId() ).getStr() );
            aMsg += ' ';
            aMsg += pClassData->aRefId.GetName();
            aError = WRN_MGR_REFNOTFOUND;
            pTC->pEH->Error( aError, rInst.pClass, rId, aMsg.GetBuffer() );
        }
    }

    if( pObjNode )
    {
        RSCINST     aRefI;

        aRefI = RSCINST( rInst.pClass, pObjNode->GetRscObj() );
        aError = aRefI.pClass->WriteCxxHeader( aRefI, fOutput, pTC,
                                               rId );
    }
    else if (pTC)
    {
        fprintf( fOutput, "%s::%s",
                 MakeName( pTC, rInst.pClass, rId.GetName() ).GetBuffer(),
                 MakeName( pTC, rInst.pClass, rId.GetName() ).GetBuffer() );
        fprintf( fOutput, "%s", (rInst.pClass->aCallParType).GetBuffer() );
        if( GetCount( rInst ) )
            fprintf( fOutput, " bFreeRes" );
        fprintf( fOutput, " )\n    : %s", pHS->getString( rInst.pClass->GetId() ).getStr() );
        fprintf( fOutput, "%s", (rInst.pClass->aCallPar1).GetBuffer() );
        fprintf( fOutput, " rResId )" );

        aError = RscClass::WriteCxx( rInst, fOutput, pTC, rId );

        fprintf( fOutput, "\n{\n" );
        RSCINST aExtraInst = rInst.pClass->GetCopyVar( rInst, nRsc_EXTRADATA );
        if( aExtraInst.IsInst() )
        {
            if( aExtraInst.pClass->GetCount( aExtraInst ) )
            {
                fprintf( fOutput, "    //read extra data\n" );
                fprintf( fOutput, "    pExtraData = new char "
                                "[ GetRemainSizeRes() ];\n" );
                fprintf( fOutput, "    memcpy( pExtraData, "
                                "GetClassRes(), GetRemainSizeRes() );\n" );
                fprintf( fOutput, "    IncrementRes( GetRemainSizeRes() );\n" );
            }
        }

        if( GetCount( rInst ) )
        { // Es gibt UnterResourcen
            fprintf( fOutput, "    if( bFreeRes ) FreeResource();\n" );
        }
        else
        {
            fprintf( fOutput,
                    "    // No subresources, automatic free resource\n" );
        }
        fprintf( fOutput, "}\n\n" );
    }
    return aError;
}

/*************************************************************************
|*
|*    RscClass::WriteCxx()
|*
*************************************************************************/
ERRTYPE RscMgr::WriteCxx( const RSCINST & rInst, FILE * fOutput,
                          RscTypCont * pTC, const RscId & rId )
{
    fprintf( fOutput, ",\n    a%s",
             MakeName( pTC, rInst.pClass, rId.GetName() ).GetBuffer() );
    fprintf( fOutput, "%s", (rInst.pClass->aCallPar2).GetBuffer() );
    fprintf( fOutput, " ResId( %s ) )", (rId.GetName()).GetBuffer() );

    return ERR_OK;
}

/*************************************************************************
|*
|*    RscArray::IsConsistent()
|*
*************************************************************************/
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

/*************************************************************************
|*
|*    RscMgr::GetRef()
|*
*************************************************************************/
ERRTYPE RscMgr::GetRef( const RSCINST & rInst, RscId * pRscId ){
    RscMgrInst * pClassData;

    pClassData = (RscMgrInst *)(rInst.pData + RscClass::Size());
    *pRscId = pClassData->aRefId;
    return ERR_OK;
}

/*************************************************************************
|*
|*    RscMgr::IsToDeep()
|*
*************************************************************************/
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

/*************************************************************************
|*
|*    RscMgr::SetRef()
|*
*************************************************************************/
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
