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
#include <rscstr.hxx>

#include <rtl/textcvt.h>
#include <rtl/textenc.h>

RscString::RscString( Atom nId, sal_uInt32 nTypeId )
                : RscTop( nId, nTypeId )
{
    nSize = ALIGNED_SIZE( sizeof( RscStringInst ) );
    pRefClass = NULL;
}

RSCCLASS_TYPE RscString::GetClassType() const
{
    return RSCCLASS_STRING;
}

ERRTYPE RscString::SetString( const RSCINST & rInst, const char * pStr ){
    char    * pTmp;
    ERRTYPE aError;

    if( aError.IsOk() ){
        ((RscStringInst *)rInst.pData)->bDflt = sal_False;

        pTmp = ((RscStringInst *)rInst.pData)->pStr;
        if( pTmp ){
            rtl_freeMemory( pTmp );
            pTmp = NULL;
        }

        if( pStr ){
            sal_uInt32  nLen = strlen( pStr ) +1;
            pTmp = (char *)rtl_allocateMemory( nLen );
            memcpy( pTmp, pStr, nLen );
        };

        ((RscStringInst *)rInst.pData)->pStr = pTmp;
    }

    return( aError );
}

ERRTYPE RscString::GetString( const RSCINST & rInst, char ** ppStr ){
    *ppStr = ((RscStringInst *)rInst.pData)->pStr;
    return( ERR_OK );
}

ERRTYPE RscString::GetRef( const RSCINST & rInst, RscId * pRscId ){
    *pRscId = ((RscStringInst *)rInst.pData)->aRefId;
    return( ERR_OK );
}

ERRTYPE RscString::SetRef( const RSCINST & rInst, const RscId & rRefId ){
    if( pRefClass ){
        ((RscStringInst *)rInst.pData)->aRefId = rRefId;
        ((RscStringInst *)rInst.pData)->bDflt  = sal_False;
    }
    else
        return( ERR_REFNOTALLOWED );

    return ERR_OK;
}

RSCINST RscString::Create( RSCINST * pInst, const RSCINST & rDflt,
                           sal_Bool bOwnClass )
{
    RSCINST aInst;

    if( !pInst ){
        aInst.pClass = this;
        aInst.pData = (CLASS_DATA)
                      rtl_allocateMemory( sizeof( RscStringInst ) );
    }
    else
        aInst = *pInst;
    if( !bOwnClass && rDflt.IsInst() )
        bOwnClass = rDflt.pClass->InHierarchy( this );

    ((RscStringInst *)aInst.pData)->aRefId.Create();
    ((RscStringInst *)aInst.pData)->pStr = NULL;
    ((RscStringInst *)aInst.pData)->bDflt = sal_True;

    if( bOwnClass ){
        ((RscStringInst *)aInst.pData)->aRefId =
                          ((RscStringInst *)rDflt.pData)->aRefId;
        SetString( aInst, ((RscStringInst *)rDflt.pData)->pStr );
        ((RscStringInst *)aInst.pData)->bDflt =
                            ((RscStringInst *)rDflt.pData)->bDflt ;
    }

    return( aInst );
}

void RscString::Destroy( const RSCINST & rInst ){
    if( ((RscStringInst *)rInst.pData)->pStr )
        rtl_freeMemory( ((RscStringInst *)rInst.pData)->pStr );
    ((RscStringInst *)rInst.pData)->aRefId.Destroy();
}

sal_Bool RscString::IsValueDefault( const RSCINST & rInst, CLASS_DATA pDef ){
    RscStringInst * pData    = (RscStringInst*)rInst.pData;
    RscStringInst * pDefData = (RscStringInst*)pDef;

    if( pDef ){
        if( pData->aRefId.IsId() || pDefData->aRefId.IsId() )
        {
            if( pData->aRefId.aExp.IsNumber()
              && pDefData->aRefId.aExp.IsNumber() )
            {
                // Sind die Referenzidentifier gleich
                if( pData->aRefId.GetNumber() == pDefData->aRefId.GetNumber() )
                {
                    return sal_True;
                }
            }
        }
        else {
            sal_Bool bStrEmpty = sal_False;
            sal_Bool bDefStrEmpty = sal_False;

            if( pData->pStr ){
                bStrEmpty = ('\0' == *pData->pStr);
            }
            else
                bStrEmpty = sal_True;

            if( pDefData->pStr ){
                bDefStrEmpty = ('\0' == *pDefData->pStr);
            }
            else
                bDefStrEmpty = sal_True;

            if( !bStrEmpty || !bDefStrEmpty ){
                return sal_False;
            }
            return sal_True;
        }
    }

    return sal_False;
}

void RscString::WriteSrc( const RSCINST & rInst, FILE * fOutput,
                          RscTypCont *, sal_uInt32, const char * )
{
    if ( ((RscStringInst *)rInst.pData)->aRefId.IsId() )
    {
        fprintf( fOutput, "%s",
                 ((RscStringInst *)rInst.pData)->aRefId.GetName().getStr() );
    }
    else
    {
        RscStringInst * pStrI = ((RscStringInst *)rInst.pData);
        if(  pStrI->pStr ){
            //char *  pChangeTab = RscChar::GetChangeTab();
            sal_uInt32  n = 0;
            sal_uInt32  nPos, nSlashPos;

            do {
                fputc( '\"', fOutput );
                nSlashPos = nPos = 0;
                while( pStrI->pStr[ n ]
                  && (nPos < 72 || nPos - nSlashPos <= 3) )
                { // nach \ mindesten 3 Zeichen wegeb \xa7
                    fputc( pStrI->pStr[ n ], fOutput );
                    if( pStrI->pStr[ n ] == '\\' )
                        nSlashPos = nPos;
                    n++;
                    nPos++;
                }

                fputc( '\"', fOutput );
                if( pStrI->pStr[ n ] ) //nocht nicht zu ende
                    fputc( '\n', fOutput );
            } while( pStrI->pStr[ n ] );
        }
        else
            fprintf( fOutput, "\"\"" );
    }
}

ERRTYPE RscString::WriteRc( const RSCINST & rInst, RscWriteRc & rMem,
                            RscTypCont * pTC, sal_uInt32 nDeep, sal_Bool bExtra )
{
    ERRTYPE aError;
    ObjNode *       pObjNode = NULL;


    if( ((RscStringInst *)rInst.pData)->aRefId.IsId() ){
        RscId   aId( ((RscStringInst *)rInst.pData)->aRefId );
        RSCINST aTmpI;

        aTmpI.pClass = pRefClass;

        while( aError.IsOk() && aId.IsId() ){
            //Erhoehen und abfragen um Endlosrekusion zu vermeiden
            nDeep++;
            if( nDeep > nRefDeep )
                aError = ERR_REFTODEEP;
            else
            {
                pObjNode = pRefClass->GetObjNode( aId );
                if( pObjNode )
                {
                    aTmpI.pData = pObjNode->GetRscObj();
                    aError = pRefClass->GetRef( aTmpI, &aId );
                }
                else
                {
                    if( pTC )
                    {
                        rtl::OStringBuffer aMsg(pHS->getString(
                            pRefClass->GetId()));
                        aMsg.append(' ').append(aId.GetName());
                        aError = WRN_STR_REFNOTFOUND;
                        pTC->pEH->Error( aError, rInst.pClass,
                                         RscId(), aMsg.getStr() );
                    }
                    break;
                }
            }
        }
    }

    if( aError.IsOk() )
    {
        if( pObjNode )
        {
            RSCINST     aRefI;

            aRefI = RSCINST( pRefClass, pObjNode->GetRscObj() );
            aError = aRefI.pClass->WriteRc( aRefI, rMem, pTC, nDeep, bExtra );
        }
        else
        {
            if( ((RscStringInst *)rInst.pData)->pStr && pTC )
            {
                char * pStr = RscChar::MakeUTF8( ((RscStringInst *)rInst.pData)->pStr,
                                                pTC->GetSourceCharSet() );
                rMem.PutUTF8( pStr );
                rtl_freeMemory( pStr );
            }
            else
                rMem.PutUTF8( ((RscStringInst *)rInst.pData)->pStr );
        };
    };
    return( aError );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
