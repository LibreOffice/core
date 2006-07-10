/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dbfunc4.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-10 14:09:56 $
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

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

//------------------------------------------------------------------

// INCLUDE ---------------------------------------------------------------

#include <svx/svditer.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdpage.hxx>
#include <sch/schdll.hxx>
#include <sch/memchrt.hxx>

#include "dbfunc.hxx"
#include "chartarr.hxx"
#include "drwlayer.hxx"
#include "document.hxx"

// -----------------------------------------------------------------------

#ifdef WNT
#pragma optimize ( "", off )
#endif

using namespace com::sun::star;

//==================================================================

// static
USHORT ScDBFunc::DoUpdateCharts( const ScAddress& rPos, ScDocument* pDoc,
                        Window* pActiveWin, BOOL bAllCharts )
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
            if ( pObject->GetObjIdentifier() == OBJ_OLE2 )
            {
                uno::Reference < embed::XEmbeddedObject > xObj = ((SdrOle2Obj*)pObject)->GetObjRef();
                if (xObj.is())
                {
                    const SchMemChart* pChartData = SchDLL::GetChartData(xObj);
                    if ( pChartData )
                    {
                        ScChartArray aArray( pDoc, *pChartData );
                        if (aArray.IsValid())
                        {
                            if ( bAllCharts || aArray.IsAtCursor(rPos) )
                            {
                                SchMemChart* pMemChart = aArray.CreateMemChart();
                                ScChartArray::CopySettings( *pMemChart, *pChartData );

                                SchDLL::Update( xObj, pMemChart, pActiveWin );
                                delete pMemChart;
                                ++nFound;
                                ((SdrOle2Obj*)pObject)->GetNewReplacement();

                                // redraw only
                                pObject->ActionChanged();
                                // pObject->SendRepaintBroadcast();
                            }
                        }
                    }
                }
            }
            pObject = aIter.Next();
        }
    }
    return nFound;
}






