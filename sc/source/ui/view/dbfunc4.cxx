/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dbfunc4.cxx,v $
 * $Revision: 1.11 $
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
#include "precompiled_sc.hxx"



//------------------------------------------------------------------

// INCLUDE ---------------------------------------------------------------

#include <svx/svditer.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdpage.hxx>

#include "dbfunc.hxx"
#include "drwlayer.hxx"
#include "document.hxx"

// -----------------------------------------------------------------------

#ifdef _MSC_VER
#pragma optimize ( "", off )
#endif

using namespace com::sun::star;

//==================================================================

// static
USHORT ScDBFunc::DoUpdateCharts( const ScAddress& rPos, ScDocument* pDoc, BOOL bAllCharts )
{
    ScDrawLayer* pModel = pDoc->GetDrawLayer();
    if (!pModel)
        return 0;

    USHORT nFound = 0;

    USHORT nPageCount = pModel->GetPageCount();
    for (USHORT nPageNo=0; nPageNo<nPageCount; nPageNo++)
    {
        SdrPage* pPage = pModel->GetPage(nPageNo);
        DBG_ASSERT(pPage,"Page ?");

        SdrObjListIter aIter( *pPage, IM_DEEPNOGROUPS );
        SdrObject* pObject = aIter.Next();
        while (pObject)
        {
            if ( pObject->GetObjIdentifier() == OBJ_OLE2 && pDoc->IsChart( pObject ) )
            {
                String aName = ((SdrOle2Obj*)pObject)->GetPersistName();
                BOOL bHit = TRUE;
                if ( !bAllCharts )
                {
                    ScRangeList aRanges;
                    BOOL bColHeaders = FALSE;
                    BOOL bRowHeaders = FALSE;
                    pDoc->GetOldChartParameters( aName, aRanges, bColHeaders, bRowHeaders );
                    bHit = aRanges.In( rPos );
                }
                if ( bHit )
                {
                    pDoc->UpdateChart( aName );
                    ++nFound;
                }
            }
            pObject = aIter.Next();
        }
    }
    return nFound;
}






