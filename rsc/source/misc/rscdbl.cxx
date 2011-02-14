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
