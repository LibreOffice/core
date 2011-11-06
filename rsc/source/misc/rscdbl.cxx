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

#include <stdio.h>
#include <rscdb.hxx>
#include <rscall.h>
#include <rschash.hxx>
#include <rsctree.hxx>
#include <rsctop.hxx>
#include "rsclst.hxx"

/*************************************************************************
|*
|*    RscTypCont::FillNameIdList()
|*
|*    Beschreibung
|*    Ersterstellung    MM 07.05.91
|*    Letzte Aenderung  MM 30.05.91
|*
*************************************************************************/
REResourceList * InsertList( Atom nClassName, const RscId& rId,
                             REResourceList * pList ){
    REResourceList  *   pSubList;
    const char *                pStrClass;
    ByteString          aStrClass;

    pStrClass = pHS->getString( nClassName ).getStr();
    if( pStrClass )
        aStrClass = pStrClass;
    else
        aStrClass = ByteString::CreateFromInt32( (long)nClassName );

    pSubList = new REResourceList( pList, aStrClass, rId );

    pList->Insert( pSubList, 0xFFFFFFFF );
    return( pSubList );
}

void FillSubList( RSCINST & rInst, REResourceList * pList )
{
    sal_uInt32      nCount, i;
    SUBINFO_STRUCT  aInfo;
    REResourceList* pSubList;
    RSCINST         aTmpI;

    nCount = rInst.pClass->GetCount( rInst );
    for( i = 0; i < nCount; i++ ){
        aInfo = rInst.pClass->GetInfoEle( rInst, i );
        aTmpI = rInst.pClass->GetPosEle( rInst, i );
        pSubList = InsertList( aInfo.pClass->GetId(),
                               aInfo.aId, pList );
        FillSubList( aTmpI, pSubList );
    };
}

void FillListObj( ObjNode * pObjNode, RscTop * pRscTop,
                  REResourceList * pList, sal_uLong lFileKey )
{
    if( pObjNode ){
        if( pObjNode->GetFileKey() == lFileKey ){
            RSCINST         aTmpI;
            REResourceList* pSubList;

            FillListObj( (ObjNode*)pObjNode->Left(), pRscTop,
                         pList, lFileKey );

            pSubList = InsertList( pRscTop->GetId(),
                                   pObjNode->GetRscId(), pList );

            aTmpI.pClass = pRscTop;
            aTmpI.pData = pObjNode->GetRscObj();
            FillSubList( aTmpI, pSubList );

            FillListObj( (ObjNode*)pObjNode->Right(), pRscTop,
                         pList, lFileKey );
        }
    };
}

void FillList( RscTop * pRscTop, REResourceList * pList, sal_uLong lFileKey ){
    if( pRscTop ){
        FillList( (RscTop*)pRscTop->Left(), pList, lFileKey );

        FillListObj( pRscTop->GetObjNode(), pRscTop, pList, lFileKey );

        FillList( (RscTop*)pRscTop->Right(), pList, lFileKey );
    };
}

void RscTypCont::FillNameIdList( REResourceList * pList, sal_uLong lFileKey ){
    FillList( pRoot, pList, lFileKey );
}
