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

#include <com/sun/star/embed/NoVisualAreaSizeException.hpp>

#include <svx/svditer.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdouno.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdundo.hxx>
#include <svx/xbtmpit.hxx>
#include <svx/xoutbmp.hxx>
#include <svtools/embedhlp.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/ipclient.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <com/sun/star/embed/Aspects.hpp>

#include <document.hxx>
#include <viewfunc.hxx>
#include <tabvwsh.hxx>
#include <drawview.hxx>
#include <scmod.hxx>
#include <drwlayer.hxx>
#include <drwtrans.hxx>
#include <globstr.hrc>
#include <scresid.hxx>
#include <chartlis.hxx>
#include <docuno.hxx>
#include <docsh.hxx>
#include <convuno.hxx>
#include <dragdata.hxx>
#include <gridwin.hxx>

bool bPasteIsMove = false;

using namespace com::sun::star;

static void lcl_AdjustInsertPos( ScViewData* pData, Point& rPos, const Size& rSize )
{
    SdrPage* pPage = pData->GetScDrawView()->GetModel()->GetPage( static_cast<sal_uInt16>(pData->GetTabNo()) );
    OSL_ENSURE(pPage,"pPage ???");
    Size aPgSize( pPage->GetSize() );
    if (aPgSize.Width() < 0)
        aPgSize.setWidth( -aPgSize.Width() );
    long x = aPgSize.Width() - rPos.X() - rSize.Width();
    long y = aPgSize.Height() - rPos.Y() - rSize.Height();
    // if necessary: adjustments (80/200) for pixel approx. errors
    if( x < 0 )
        rPos.AdjustX(x + 80 );
    if( y < 0 )
        rPos.AdjustY(y + 200 );
    rPos.AdjustX(rSize.Width() / 2 );          // position at paste is center
    rPos.AdjustY(rSize.Height() / 2 );
}

void ScViewFunc::PasteDraw( const Point& rLogicPos, SdrModel* pModel,
        bool bGroup, const OUString& rSrcShellID, const OUString& rDestShellID )
{
    bool bSameDocClipboard = rSrcShellID == rDestShellID;

    MakeDrawLayer();
    Point aPos( rLogicPos );

    // MapMode at Outliner-RefDevice has to be right (as in FuText::MakeOutliner)
    //! merge with FuText::MakeOutliner?
    MapMode aOldMapMode;
    OutputDevice* pRef = GetViewData().GetDocument()->GetDrawLayer()->GetRefDevice();
    if (pRef)
    {
        aOldMapMode = pRef->GetMapMode();
        pRef->SetMapMode( MapMode(MapUnit::Map100thMM) );
    }

    bool bNegativePage = GetViewData().GetDocument()->IsNegativePage( GetViewData().GetTabNo() );

    SdrView* pDragEditView = nullptr;
    ScModule* pScMod = SC_MOD();
    const ScDragData& rData = pScMod->GetDragData();
    ScDrawTransferObj* pDrawTrans = rData.pDrawTransfer;
    if (pDrawTrans)
    {
        pDragEditView = pDrawTrans->GetDragSourceView();

        aPos -= aDragStartDiff;
        if ( bNegativePage )
        {
            if (aPos.X() > 0) aPos.setX( 0 );
        }
        else
        {
            if (aPos.X() < 0) aPos.setX( 0 );
        }
        if (aPos.Y() < 0) aPos.setY( 0 );
    }

    ScDrawView* pScDrawView = GetScDrawView();
    if (bGroup)
        pScDrawView->BegUndo( ScResId( STR_UNDO_PASTE ) );

    bool bSameDoc = ( pDragEditView && pDragEditView->GetModel() == pScDrawView->GetModel() );
    if (bSameDoc)
    {
            // copy locally - incl. charts

        Point aSourceStart = pDragEditView->GetAllMarkedRect().TopLeft();
        long nDiffX = aPos.X() - aSourceStart.X();
        long nDiffY = aPos.Y() - aSourceStart.Y();

            // move within a page?

        if ( bPasteIsMove &&
                pScDrawView->GetSdrPageView()->GetPage() ==
                pDragEditView->GetSdrPageView()->GetPage() )
        {
            if ( nDiffX != 0 || nDiffY != 0 )
                pDragEditView->MoveAllMarked(Size(nDiffX,nDiffY));
        }
        else
        {
            SdrModel* pDrawModel = pDragEditView->GetModel();
            SCTAB nTab = GetViewData().GetTabNo();
            SdrPage* pDestPage = pDrawModel->GetPage( static_cast< sal_uInt16 >( nTab ) );
            OSL_ENSURE(pDestPage,"who is this, Page?");

            ::std::vector< OUString > aExcludedChartNames;
            if ( pDestPage )
            {
                ScChartHelper::GetChartNames( aExcludedChartNames, pDestPage );
            }

            SdrMarkList aMark = pDragEditView->GetMarkedObjectList();
            aMark.ForceSort();
            const size_t nMarkCnt=aMark.GetMarkCount();
            for (size_t nm=0; nm<nMarkCnt; ++nm) {
                const SdrMark* pM=aMark.GetMark(nm);
                const SdrObject* pObj=pM->GetMarkedSdrObj();

                // Directly Clone to target SdrModel
                SdrObject* pNewObj(pObj->CloneSdrObject(*pDrawModel));

                if (pNewObj!=nullptr)
                {
                    //  copy graphics within the same model - always needs new name
                    if ( dynamic_cast<const SdrGrafObj*>( pNewObj) !=  nullptr && !bPasteIsMove )
                        pNewObj->SetName(static_cast<ScDrawLayer*>(pDrawModel)->GetNewGraphicName());

                    if (nDiffX!=0 || nDiffY!=0)
                        pNewObj->NbcMove(Size(nDiffX,nDiffY));
                    if (pDestPage)
                        pDestPage->InsertObject( pNewObj );
                    pScDrawView->AddUndo(std::make_unique<SdrUndoInsertObj>( *pNewObj ));

                    if (ScDrawLayer::IsCellAnchored(*pNewObj))
                        ScDrawLayer::SetCellAnchoredFromPosition(*pNewObj, *GetViewData().GetDocument(), nTab,
                                                                 ScDrawLayer::IsResizeWithCell(*pNewObj));
                }
            }

            if (bPasteIsMove)
                pDragEditView->DeleteMarked();

            ScDocument* pDocument = GetViewData().GetDocument();
            ScDocShell* pDocShell = GetViewData().GetDocShell();
            ScModelObj* pModelObj = ( pDocShell ? ScModelObj::getImplementation( pDocShell->GetModel() ) : nullptr );
            if ( pDocument && pDestPage && pModelObj && pDrawTrans )
            {
                const ScRangeListVector& rProtectedChartRangesVector( pDrawTrans->GetProtectedChartRangesVector() );
                ScChartHelper::CreateProtectedChartListenersAndNotify( pDocument, pDestPage, pModelObj, nTab,
                    rProtectedChartRangesVector, aExcludedChartNames, bSameDoc );
            }
        }
    }
    else
    {
        bPasteIsMove = false;       // no internal move happened
        SdrView aView(*pModel);     // #i71529# never create a base class of SdrView directly!
        SdrPageView* pPv = aView.ShowSdrPage(aView.GetModel()->GetPage(0));
        aView.MarkAllObj(pPv);
        Size aSize = aView.GetAllMarkedRect().GetSize();
        lcl_AdjustInsertPos( &GetViewData(), aPos, aSize );

        // don't change marking if OLE object is active
        // (at Drop from OLE object it would be deactivated in the middle of ExecuteDrag!)

        SdrInsertFlags nOptions = SdrInsertFlags::NONE;
        SfxInPlaceClient* pClient = GetViewData().GetViewShell()->GetIPClient();
        if ( pClient && pClient->IsObjectInPlaceActive() )
            nOptions |= SdrInsertFlags::DONTMARK;

        ::std::vector< OUString > aExcludedChartNames;
        SCTAB nTab = GetViewData().GetTabNo();
        SdrPage* pPage = pScDrawView->GetModel()->GetPage( static_cast< sal_uInt16 >( nTab ) );
        OSL_ENSURE( pPage, "Page?" );
        if ( pPage )
        {
            ScChartHelper::GetChartNames( aExcludedChartNames, pPage );
        }

        // #89247# Set flag for ScDocument::UpdateChartListeners() which is
        // called during paste.
        if ( !bSameDocClipboard )
            GetViewData().GetDocument()->SetPastingDrawFromOtherDoc( true );

        pScDrawView->Paste(*pModel, aPos, nullptr, nOptions);

        if ( !bSameDocClipboard )
            GetViewData().GetDocument()->SetPastingDrawFromOtherDoc( false );

        // Paste puts all objects on the active (front) layer
        // controls must be on SC_LAYER_CONTROLS
        if (pPage)
        {
            SdrObjListIter aIter( pPage, SdrIterMode::DeepNoGroups );
            SdrObject* pObject = aIter.Next();
            while (pObject)
            {
                if ( dynamic_cast<const SdrUnoObj*>( pObject) !=  nullptr && pObject->GetLayer() != SC_LAYER_CONTROLS )
                    pObject->NbcSetLayer(SC_LAYER_CONTROLS);

                if (ScDrawLayer::IsCellAnchored(*pObject))
                    ScDrawLayer::SetCellAnchoredFromPosition(*pObject, *GetViewData().GetDocument(), nTab,
                                                             ScDrawLayer::IsResizeWithCell(*pObject));

                pObject = aIter.Next();
            }
        }

        // all graphics objects must have names
        GetViewData().GetDocument()->EnsureGraphicNames();

        ScDocument* pDocument = GetViewData().GetDocument();
        ScDocShell* pDocShell = GetViewData().GetDocShell();
        ScModelObj* pModelObj = ( pDocShell ? ScModelObj::getImplementation( pDocShell->GetModel() ) : nullptr );
        const ScDrawTransferObj* pTransferObj = ScDrawTransferObj::GetOwnClipboard(ScTabViewShell::GetClipData(GetViewData().GetActiveWin()));
        if ( pDocument && pPage && pModelObj && ( pTransferObj || pDrawTrans ) )
        {
            const ScRangeListVector& rProtectedChartRangesVector(
                pTransferObj ? pTransferObj->GetProtectedChartRangesVector() : pDrawTrans->GetProtectedChartRangesVector() );
            ScChartHelper::CreateProtectedChartListenersAndNotify( pDocument, pPage, pModelObj, nTab,
                rProtectedChartRangesVector, aExcludedChartNames, bSameDocClipboard );
        }
    }

    if (bGroup)
    {
        pScDrawView->GroupMarked();
        pScDrawView->EndUndo();
    }

    if (pRef)
        pRef->SetMapMode( aOldMapMode );

    // GetViewData().GetViewShell()->SetDrawShell( true );
    // It is not sufficient to just set the DrawShell if we pasted, for
    // example, a chart.  SetDrawShellOrSub() would only work for D&D in the
    // same document but not if inserting from the clipboard, therefore
    // MarkListHasChanged() is what we need.
    pScDrawView->MarkListHasChanged();

}

bool ScViewFunc::PasteObject( const Point& rPos, const uno::Reference < embed::XEmbeddedObject >& xObj,
                                const Size* pDescSize, const Graphic* pReplGraph, const OUString& aMediaType, sal_Int64 nAspect )
{
    MakeDrawLayer();
    if ( xObj.is() )
    {
        OUString aName;
        //TODO/MBA: is that OK?
        comphelper::EmbeddedObjectContainer& aCnt = GetViewData().GetViewShell()->GetObjectShell()->GetEmbeddedObjectContainer();
        if ( !aCnt.HasEmbeddedObject( xObj ) )
            aCnt.InsertEmbeddedObject( xObj, aName );
        else
            aName = aCnt.GetEmbeddedObjectName( xObj );

        svt::EmbeddedObjectRef aObjRef( xObj, nAspect );
        if ( pReplGraph )
            aObjRef.SetGraphic( *pReplGraph, aMediaType );

        Size aSize;
        if ( nAspect == embed::Aspects::MSOLE_ICON )
        {
            MapMode aMapMode( MapUnit::Map100thMM );
            aSize = aObjRef.GetSize( &aMapMode );
        }
        else
        {
            // working with visual area can switch object to running state
            MapUnit aMapObj = VCLUnoHelper::UnoEmbed2VCLMapUnit( xObj->getMapUnit( nAspect ) );
            MapUnit aMap100 = MapUnit::Map100thMM;

            if ( pDescSize && pDescSize->Width() && pDescSize->Height() )
            {
                // use size from object descriptor if given
                aSize = OutputDevice::LogicToLogic(*pDescSize, MapMode(aMap100), MapMode(aMapObj));
                awt::Size aSz;
                aSz.Width = aSize.Width();
                aSz.Height = aSize.Height();
                xObj->setVisualAreaSize( nAspect, aSz );
            }

            awt::Size aSz;
            try
            {
                aSz = xObj->getVisualAreaSize( nAspect );
            }
            catch ( embed::NoVisualAreaSizeException& )
            {
                // the default size will be set later
            }

            aSize = Size( aSz.Width, aSz.Height );
            aSize = OutputDevice::LogicToLogic(aSize, MapMode(aMapObj), MapMode(aMap100)); // for SdrOle2Obj

            if( aSize.Height() == 0 || aSize.Width() == 0 )
            {
                OSL_FAIL("SvObjectDescriptor::GetSize == 0");
                aSize.setWidth( 5000 );
                aSize.setHeight( 5000 );
                aSize = OutputDevice::LogicToLogic(aSize, MapMode(aMap100), MapMode(aMapObj));
                aSz.Width = aSize.Width();
                aSz.Height = aSize.Height();
                xObj->setVisualAreaSize( nAspect, aSz );
            }
        }

        // don't call AdjustInsertPos
        Point aInsPos = rPos;
        if ( GetViewData().GetDocument()->IsNegativePage( GetViewData().GetTabNo() ) )
            aInsPos.AdjustX( -(aSize.Width()) );
        tools::Rectangle aRect( aInsPos, aSize );

        ScDrawView* pDrView = GetScDrawView();
        SdrOle2Obj* pSdrObj = new SdrOle2Obj(
            pDrView->getSdrModelFromSdrView(),
            aObjRef,
            aName,
            aRect);

        SdrPageView* pPV = pDrView->GetSdrPageView();
        pDrView->InsertObjectSafe( pSdrObj, *pPV );             // don't mark if OLE
        GetViewData().GetViewShell()->SetDrawShell( true );
        return true;
    }
    else
        return false;
}

bool ScViewFunc::PasteBitmapEx( const Point& rPos, const BitmapEx& rBmpEx )
{
    Graphic aGraphic(rBmpEx);
    return PasteGraphic( rPos, aGraphic, "", "" );
}

bool ScViewFunc::PasteMetaFile( const Point& rPos, const GDIMetaFile& rMtf )
{
    Graphic aGraphic(rMtf);
    return PasteGraphic( rPos, aGraphic, "", "" );
}

bool ScViewFunc::PasteGraphic( const Point& rPos, const Graphic& rGraphic,
                                const OUString& rFile, const OUString& rFilter )
{
    MakeDrawLayer();
    ScDrawView* pScDrawView = GetScDrawView();

    if (!pScDrawView)
        return false;

    // #i123922# check if the drop was over an existing object; if yes, evtl. replace
    // the graphic for a SdrGraphObj (including link state updates) or adapt the fill
    // style for other objects
    SdrPageView* pPageView = pScDrawView->GetSdrPageView();
    if (pPageView)
    {
        SdrObject* pPickObj = pScDrawView->PickObj(rPos, pScDrawView->getHitTolLog(), pPageView);
        if (pPickObj)
        {
            const OUString aBeginUndo(ScResId(STR_UNDO_DRAGDROP));
            SdrObject* pResult = pScDrawView->ApplyGraphicToObject(
                *pPickObj,
                rGraphic,
                aBeginUndo,
                rFile,
                rFilter);

            if (pResult)
            {
                // we are done; mark the modified/new object
                pScDrawView->MarkObj(pResult, pScDrawView->GetSdrPageView());
                return true;
            }
        }
    }

    Point aPos( rPos );
    vcl::Window* pWin = GetActiveWin();
    MapMode aSourceMap = rGraphic.GetPrefMapMode();
    MapMode aDestMap( MapUnit::Map100thMM );

    if (aSourceMap.GetMapUnit() == MapUnit::MapPixel)
    {
        // consider pixel correction, so bitmap fits to screen
        Fraction aScaleX, aScaleY;
        pScDrawView->CalcNormScale( aScaleX, aScaleY );
        aDestMap.SetScaleX(aScaleX);
        aDestMap.SetScaleY(aScaleY);
    }

    Size aSize = pWin->LogicToLogic( rGraphic.GetPrefSize(), &aSourceMap, &aDestMap );

    if ( GetViewData().GetDocument()->IsNegativePage( GetViewData().GetTabNo() ) )
        aPos.AdjustX( -(aSize.Width()) );

    GetViewData().GetViewShell()->SetDrawShell( true );
    tools::Rectangle aRect(aPos, aSize);
    SdrGrafObj* pGrafObj = new SdrGrafObj(
        pScDrawView->getSdrModelFromSdrView(),
        rGraphic,
        aRect);

    // path was the name of the graphic in history

    ScDrawLayer* pLayer = static_cast<ScDrawLayer*>( pScDrawView->GetModel() );
    OUString aName = pLayer->GetNewGraphicName();                 // "Graphics"
    pGrafObj->SetName(aName);

    // don't mark if OLE
    pScDrawView->InsertObjectSafe(pGrafObj, *pScDrawView->GetSdrPageView());

    // SetGraphicLink has to be used after inserting the object,
    // otherwise an empty graphic is swapped in and the contact stuff crashes.
    // See #i37444#.
    if (!rFile.isEmpty())
        pGrafObj->SetGraphicLink( rFile, ""/*TODO?*/, rFilter );

    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
