/*************************************************************************
 *
 *  $RCSfile: rscdbl.cxx,v $
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
REResourceList * InsertList( HASHID nClassName, const RscId& rId,
                             REResourceList * pList ){
    REResourceList  *   pSubList;
    char *              pStrClass;
    ByteString          aStrClass;

    pStrClass = pHS->Get( nClassName );
    if( pStrClass )
        aStrClass = pStrClass;
    else
        aStrClass = ByteString::CreateFromInt32( (long)nClassName );

    pSubList = new REResourceList( pList, aStrClass, rId );

    pList->Insert( pSubList, 0xFFFF );
    return( pSubList );
}

void FillSubList( RSCINST & rInst, REResourceList * pList ){
    USHORT          nCount, i;
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
