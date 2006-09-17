/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: rscdbl.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 15:57:57 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_rsc.hxx"

#include <stdio.h>

#ifndef _RSCDB_HXX
#include <rscdb.hxx>
#endif
#ifndef _RSCALL_H
#include <rscall.h>
#endif
#ifndef _RSCHASH_HXX
#include <rschash.hxx>
#endif
#ifndef _RSCTREE_HXX
#include <rsctree.hxx>
#endif
#ifndef _RSCTOP_HXX
#include <rsctop.hxx>
#endif
#ifndef _RSCLST_HXX
#include "rsclst.hxx"
#endif

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
                  REResourceList * pList, ULONG lFileKey )
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

void FillList( RscTop * pRscTop, REResourceList * pList, ULONG lFileKey ){
    if( pRscTop ){
        FillList( (RscTop*)pRscTop->Left(), pList, lFileKey );

        FillListObj( pRscTop->GetObjNode(), pRscTop, pList, lFileKey );

        FillList( (RscTop*)pRscTop->Right(), pList, lFileKey );
    };
}

void RscTypCont::FillNameIdList( REResourceList * pList, ULONG lFileKey ){
    FillList( pRoot, pList, lFileKey );
}
