/*************************************************************************
 *
 *  $RCSfile: drawvie4.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: rt $ $Date: 2003-09-19 08:25:57 $
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

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

// INCLUDE ---------------------------------------------------------------

#include <sch/memchrt.hxx>
#include <sch/schdll.hxx>
#include <svx/svditer.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdogrp.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdundo.hxx>
#include <sfx2/docfile.hxx>
#include <tools/urlobj.hxx>

#include "drawview.hxx"
#include "global.hxx"
#include "drwlayer.hxx"
#include "viewdata.hxx"
#include "document.hxx"
#include "docsh.hxx"
#include "drwtrans.hxx"
#include "transobj.hxx"     // SetDrawClipDoc
#include "drawutil.hxx"
#include "scmod.hxx"
#include "globstr.hrc"
#include "chartarr.hxx"

using namespace com::sun::star;

// STATIC DATA -----------------------------------------------------------

Point aDragStartDiff;

// -----------------------------------------------------------------------

//! welche Funktionen aus drawview/drawvie4 muessen wirklich ohne Optimierung sein?

#ifdef WNT
#pragma optimize ( "", off )
#endif

// -----------------------------------------------------------------------

void lcl_CheckOle( const SdrMarkList& rMarkList, BOOL& rAnyOle, BOOL& rOneOle )
{
    rAnyOle = rOneOle = FALSE;
    ULONG nCount = rMarkList.GetMarkCount();
    for (ULONG i=0; i<nCount; i++)
    {
        SdrMark* pMark = rMarkList.GetMark(i);
        SdrObject* pObj = pMark->GetObj();
        UINT16 nSdrObjKind = pObj->GetObjIdentifier();
        if (nSdrObjKind == OBJ_OLE2)
        {
            rAnyOle = TRUE;
            rOneOle = (nCount == 1);
            break;
        }
        else if ( pObj->ISA(SdrObjGroup) )
        {
            SdrObjListIter aIter( *pObj, IM_DEEPNOGROUPS );
            SdrObject* pSubObj = aIter.Next();
            while (pSubObj)
            {
                if ( pSubObj->GetObjIdentifier() == OBJ_OLE2 )
                {
                    rAnyOle = TRUE;
                    // rOneOle remains FALSE - a group isn't treated like a single OLE object
                    return;
                }
                pSubObj = aIter.Next();
            }
        }
    }
}

#if 0
void lcl_RefreshChartData( SdrModel* pModel, ScDocument* pSourceDoc )
{
    USHORT nPages = pModel->GetPageCount();
    for (USHORT nTab=0; nTab<nPages; nTab++)
    {
        SdrPage* pPage = pModel->GetPage(nTab);
        SdrObjListIter aIter( *pPage, IM_DEEPNOGROUPS );
        SdrObject* pObject = aIter.Next();
        while (pObject)
        {
            if ( pObject->GetObjIdentifier() == OBJ_OLE2 )
            {
                SvInPlaceObjectRef aIPObj = ((SdrOle2Obj*)pObject)->GetObjRef();
                if ( aIPObj.Is() && SotExchange::IsChart( aIPObj->GetStorage()->GetClassName() ) )
                {
                    SchMemChart* pOldData = SchDLL::GetChartData(aIPObj);
                    if ( pOldData )
                    {
                        //  create data from source document
                        ScChartArray aArray( pSourceDoc, *pOldData );
                        if ( aArray.IsValid() )
                        {
                            SchMemChart* pNewData = aArray.CreateMemChart();
                            SchDLL::Update( aIPObj, pNewData );
                            delete pNewData;
                        }
                    }
                }
            }
            pObject = aIter.Next();
        }
    }
}
#endif


BOOL ScDrawView::BeginDrag( Window* pWindow, const Point& rStartPos )
{
    BOOL bReturn = FALSE;

    if ( HasMarkedObj() )
    {
        ScModule* pScMod = SC_MOD();

        BrkAction();

        Rectangle aMarkedRect = GetAllMarkedRect();
        Region aRegion( aMarkedRect );

        aDragStartDiff = rStartPos - aMarkedRect.TopLeft();

        BOOL bAnyOle, bOneOle;
        const SdrMarkList& rMarkList = GetMarkList();
        lcl_CheckOle( rMarkList, bAnyOle, bOneOle );

        ScDocShellRef aDragShellRef;
        if (bAnyOle)
        {
            aDragShellRef = new ScDocShell;     // DocShell needs a Ref immediately
            aDragShellRef->DoInitNew(NULL);
        }
        ScDrawLayer::SetGlobalDrawPersist(aDragShellRef);
        SdrModel* pModel = GetAllMarkedModel();
        ScDrawLayer::SetGlobalDrawPersist(NULL);

        //  Charts now always copy their data in addition to the source reference, so
        //  there's no need to call SchDLL::Update for the charts in the clipboard doc.
        //  Update with the data (including NumberFormatter) from the live document would
        //  also store the NumberFormatter in the clipboard chart (#88749#)
        // lcl_RefreshChartData( pModel, pViewData->GetDocument() );

        ScDocShell* pDocSh = pViewData->GetDocShell();

        TransferableObjectDescriptor aObjDesc;
        pDocSh->FillTransferableObjectDescriptor( aObjDesc );
        aObjDesc.maDisplayName = pDocSh->GetMedium()->GetURLObject().GetURLNoPass();
        // maSize is set in ScDrawTransferObj ctor

        ScDrawTransferObj* pTransferObj = new ScDrawTransferObj( pModel, pDocSh, aObjDesc );
        uno::Reference<datatransfer::XTransferable> xTransferable( pTransferObj );

        SvEmbeddedObjectRef aPersistRef( aDragShellRef );
        pTransferObj->SetDrawPersist( aPersistRef );    // keep persist for ole objects alive
        pTransferObj->SetDragSource( this );            // copies selection

        SC_MOD()->SetDragObject( NULL, pTransferObj );      // for internal D&D
        pTransferObj->StartDrag( pWindow, DND_ACTION_COPYMOVE | DND_ACTION_LINK );
    }

    return bReturn;
}

void ScDrawView::DoCopy()
{
    BOOL bAnyOle, bOneOle;
    const SdrMarkList& rMarkList = GetMarkList();
    lcl_CheckOle( rMarkList, bAnyOle, bOneOle );

    // update ScGlobal::pDrawClipDocShellRef
    ScDrawLayer::SetGlobalDrawPersist( ScTransferObj::SetDrawClipDoc( bAnyOle ) );
    SdrModel* pModel = GetAllMarkedModel();
    ScDrawLayer::SetGlobalDrawPersist(NULL);

    //  Charts now always copy their data in addition to the source reference, so
    //  there's no need to call SchDLL::Update for the charts in the clipboard doc.
    //  Update with the data (including NumberFormatter) from the live document would
    //  also store the NumberFormatter in the clipboard chart (#88749#)
    // lcl_RefreshChartData( pModel, pViewData->GetDocument() );

    ScDocShell* pDocSh = pViewData->GetDocShell();

    TransferableObjectDescriptor aObjDesc;
    pDocSh->FillTransferableObjectDescriptor( aObjDesc );
    aObjDesc.maDisplayName = pDocSh->GetMedium()->GetURLObject().GetURLNoPass();
    // maSize is set in ScDrawTransferObj ctor

    ScDrawTransferObj* pTransferObj = new ScDrawTransferObj( pModel, pDocSh, aObjDesc );
    uno::Reference<datatransfer::XTransferable> xTransferable( pTransferObj );

    if ( ScGlobal::pDrawClipDocShellRef )
    {
        SvEmbeddedObjectRef aPersistRef( *ScGlobal::pDrawClipDocShellRef );
        pTransferObj->SetDrawPersist( aPersistRef );    // keep persist for ole objects alive
    }

    pTransferObj->CopyToClipboard( pViewData->GetActiveWin() );     // system clipboard
    SC_MOD()->SetClipObject( NULL, pTransferObj );                  // internal clipboard
}

//  Korrektur fuer 100% berechnen, unabhaengig von momentanen Einstellungen

void ScDrawView::CalcNormScale( Fraction& rFractX, Fraction& rFractY ) const
{
    Point aLogic = pDev->LogicToPixel( Point(1000,1000), MAP_TWIP );
    double nPPTX = ScGlobal::nScreenPPTX;
    double nPPTY = ScGlobal::nScreenPPTY;

    if (pViewData)
        nPPTX /= pViewData->GetDocShell()->GetOutputFactor();

    USHORT nEndCol = 0;
    USHORT nEndRow = 0;
    pDoc->GetTableArea( nTab, nEndCol, nEndRow );
    if (nEndCol<20)
        nEndCol = 20;
    if (nEndRow<20)
        nEndRow = 20;

    Fraction aZoom(1,1);
    ScDrawUtil::CalcScale( pDoc, nTab, 0,0, nEndCol,nEndRow, pDev, aZoom,aZoom,
                            nPPTX, nPPTY, rFractX,rFractY );
}

void ScDrawView::SetMarkedOriginalSize()
{
    SdrUndoGroup* pUndoGroup = new SdrUndoGroup(*GetModel());

    const SdrMarkList& rMarkList = GetMarkList();
    long nDone = 0;
    ULONG nCount = rMarkList.GetMarkCount();
    for (ULONG i=0; i<nCount; i++)
    {
        SdrObject* pObj = rMarkList.GetMark(i)->GetObj();
        USHORT nIdent = pObj->GetObjIdentifier();
        BOOL bDo = FALSE;
        Size aOriginalSize;
        if (nIdent == OBJ_OLE2)
        {
            SvInPlaceObjectRef xIPObj = ((SdrOle2Obj*)pObj)->GetObjRef();
            if (xIPObj.Is())
            {
                aOriginalSize = OutputDevice::LogicToLogic(
                                        xIPObj->GetVisArea().GetSize(),
                                        xIPObj->GetMapUnit(), MAP_100TH_MM );
                bDo = TRUE;
            }
        }
        else if (nIdent == OBJ_GRAF)
        {
            const Graphic& rGraphic = ((SdrGrafObj*)pObj)->GetGraphic();

            MapMode aSourceMap = rGraphic.GetPrefMapMode();
            MapMode aDestMap( MAP_100TH_MM );
            if (aSourceMap.GetMapUnit() == MAP_PIXEL)
            {
                //  Pixel-Korrektur beruecksichtigen, damit Bitmap auf dem Bildschirm stimmt

                Fraction aScaleX, aScaleY;
                CalcNormScale( aScaleX, aScaleY );
                aDestMap.SetScaleX(aScaleX);
                aDestMap.SetScaleY(aScaleY);
            }
            if (pViewData)
            {
                Window* pActWin = pViewData->GetActiveWin();
                if (pActWin)
                {
                    aOriginalSize = pActWin->LogicToLogic(
                                    rGraphic.GetPrefSize(), &aSourceMap, &aDestMap );
                    bDo = TRUE;
                }
            }
        }

        if ( bDo )
        {
            Rectangle aDrawRect = pObj->GetLogicRect();

            pUndoGroup->AddAction( new SdrUndoGeoObj( *pObj ) );
            pObj->Resize( aDrawRect.TopLeft(), Fraction( aOriginalSize.Width(), aDrawRect.GetWidth() ),
                                                 Fraction( aOriginalSize.Height(), aDrawRect.GetHeight() ) );
            ++nDone;
        }
    }

    if (nDone)
    {
        pUndoGroup->SetComment(ScGlobal::GetRscString( STR_UNDO_ORIGINALSIZE ));
        pViewData->GetDocShell()->GetUndoManager()->AddUndoAction(pUndoGroup);
    }
    else
        delete pUndoGroup;
}


#ifdef WNT
#pragma optimize ( "", on )
#endif




