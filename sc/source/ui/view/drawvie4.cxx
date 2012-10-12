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

#include <com/sun/star/embed/NoVisualAreaSizeException.hpp>

#include <svx/svditer.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdogrp.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdundo.hxx>
#include <sfx2/docfile.hxx>
#include <tools/urlobj.hxx>
#include <toolkit/helper/vclunohelper.hxx>

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

#ifdef _MSC_VER
#pragma optimize ( "", off )
#endif

// -----------------------------------------------------------------------

void ScDrawView::CheckOle( const SdrMarkList& rMarkList, sal_Bool& rAnyOle, sal_Bool& rOneOle )
{
    rAnyOle = rOneOle = false;
    sal_uLong nCount = rMarkList.GetMarkCount();
    for (sal_uLong i=0; i<nCount; i++)
    {
        SdrMark* pMark = rMarkList.GetMark(i);
        SdrObject* pObj = pMark->GetMarkedSdrObj();
        sal_uInt16 nSdrObjKind = pObj->GetObjIdentifier();
        if (nSdrObjKind == OBJ_OLE2)
        {
            rAnyOle = sal_True;
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
                    rAnyOle = sal_True;
                    // rOneOle remains sal_False - a group isn't treated like a single OLE object
                    return;
                }
                pSubObj = aIter.Next();
            }
        }
    }
}

sal_Bool ScDrawView::BeginDrag( Window* pWindow, const Point& rStartPos )
{
    sal_Bool bReturn = false;

    if ( AreObjectsMarked() )
    {
        BrkAction();

        Rectangle aMarkedRect = GetAllMarkedRect();
        Region aRegion( aMarkedRect );

        aDragStartDiff = rStartPos - aMarkedRect.TopLeft();

        sal_Bool bAnyOle, bOneOle;
        const SdrMarkList& rMarkList = GetMarkedObjectList();
        CheckOle( rMarkList, bAnyOle, bOneOle );

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

        pTransferObj->SetDrawPersist( &aDragShellRef );    // keep persist for ole objects alive
        pTransferObj->SetDragSource( this );            // copies selection

        SC_MOD()->SetDragObject( NULL, pTransferObj );      // for internal D&D
        pTransferObj->StartDrag( pWindow, DND_ACTION_COPYMOVE | DND_ACTION_LINK );
    }

    return bReturn;
}

void ScDrawView::DoCopy()
{
    sal_Bool bAnyOle, bOneOle;
    const SdrMarkList& rMarkList = GetMarkedObjectList();
    CheckOle( rMarkList, bAnyOle, bOneOle );

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
        pTransferObj->SetDrawPersist( &(*ScGlobal::pDrawClipDocShellRef) );    // keep persist for ole objects alive
    }

    pTransferObj->CopyToClipboard( pViewData->GetActiveWin() );     // system clipboard
    SC_MOD()->SetClipObject( NULL, pTransferObj );                  // internal clipboard
}

uno::Reference<datatransfer::XTransferable> ScDrawView::CopyToTransferable()
{
    sal_Bool bAnyOle, bOneOle;
    const SdrMarkList& rMarkList = GetMarkedObjectList();
    CheckOle( rMarkList, bAnyOle, bOneOle );

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
        pTransferObj->SetDrawPersist( &(*ScGlobal::pDrawClipDocShellRef) );    // keep persist for ole objects alive
    }

    return xTransferable;
}

//  Korrektur fuer 100% berechnen, unabhaengig von momentanen Einstellungen

void ScDrawView::CalcNormScale( Fraction& rFractX, Fraction& rFractY ) const
{
    double nPPTX = ScGlobal::nScreenPPTX;
    double nPPTY = ScGlobal::nScreenPPTY;

    if (pViewData)
        nPPTX /= pViewData->GetDocShell()->GetOutputFactor();

    SCCOL nEndCol = 0;
    SCROW nEndRow = 0;
    pDoc->GetTableArea( nTab, nEndCol, nEndRow );
    if (nEndCol<20)
        nEndCol = 20;
    if (nEndRow<20)
        nEndRow = 1000;

    Fraction aZoom(1,1);
    ScDrawUtil::CalcScale( pDoc, nTab, 0,0, nEndCol,nEndRow, pDev, aZoom,aZoom,
                            nPPTX, nPPTY, rFractX,rFractY );
}

void ScDrawView::SetMarkedOriginalSize()
{
    SdrUndoGroup* pUndoGroup = new SdrUndoGroup(*GetModel());

    const SdrMarkList& rMarkList = GetMarkedObjectList();
    long nDone = 0;
    sal_uLong nCount = rMarkList.GetMarkCount();
    for (sal_uLong i=0; i<nCount; i++)
    {
        SdrObject* pObj = rMarkList.GetMark(i)->GetMarkedSdrObj();
        sal_uInt16 nIdent = pObj->GetObjIdentifier();
        sal_Bool bDo = false;
        Size aOriginalSize;
        if (nIdent == OBJ_OLE2)
        {
            // TODO/LEAN: working with visual area can switch object to running state
            uno::Reference < embed::XEmbeddedObject > xObj( ((SdrOle2Obj*)pObj)->GetObjRef(), uno::UNO_QUERY );
            if ( xObj.is() )    // NULL for an invalid object that couldn't be loaded
            {
                sal_Int64 nAspect = ((SdrOle2Obj*)pObj)->GetAspect();

                if ( nAspect == embed::Aspects::MSOLE_ICON )
                {
                    MapMode aMapMode( MAP_100TH_MM );
                    aOriginalSize = ((SdrOle2Obj*)pObj)->GetOrigObjSize( &aMapMode );
                    bDo = sal_True;
                }
                else
                {
                    MapUnit aUnit = VCLUnoHelper::UnoEmbed2VCLMapUnit( xObj->getMapUnit( ((SdrOle2Obj*)pObj)->GetAspect() ) );
                    awt::Size aSz;
                    try
                    {
                        aSz = xObj->getVisualAreaSize( ((SdrOle2Obj*)pObj)->GetAspect() );
                        aOriginalSize = OutputDevice::LogicToLogic(
                                            Size( aSz.Width, aSz.Height ),
                                            aUnit, MAP_100TH_MM );
                        bDo = sal_True;
                    } catch( embed::NoVisualAreaSizeException& )
                    {
                        OSL_ENSURE( false, "Can't get the original size of the object!" );
                    }
                }
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

                Fraction aNormScaleX, aNormScaleY;
                CalcNormScale( aNormScaleX, aNormScaleY );
                aDestMap.SetScaleX(aNormScaleX);
                aDestMap.SetScaleY(aNormScaleY);
            }
            if (pViewData)
            {
                Window* pActWin = pViewData->GetActiveWin();
                if (pActWin)
                {
                    aOriginalSize = pActWin->LogicToLogic(
                                    rGraphic.GetPrefSize(), &aSourceMap, &aDestMap );
                    bDo = sal_True;
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
        ScDocShell* pDocSh = pViewData->GetDocShell();
        pDocSh->GetUndoManager()->AddUndoAction(pUndoGroup);
        pDocSh->SetDrawModified();
    }
    else
        delete pUndoGroup;
}


#ifdef _MSC_VER
#pragma optimize ( "", on )
#endif




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
