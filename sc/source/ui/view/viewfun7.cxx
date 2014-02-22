/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
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
#include <sfx2/objsh.hxx>
#include <sfx2/viewfrm.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <com/sun/star/embed/Aspects.hpp>

#include "document.hxx"
#include "viewfunc.hxx"
#include "tabvwsh.hxx"
#include "drawview.hxx"
#include "scmod.hxx"
#include "drwlayer.hxx"
#include "drwtrans.hxx"
#include "globstr.hrc"
#include "chartlis.hxx"
#include "docuno.hxx"
#include "docsh.hxx"
#include "convuno.hxx"
#include "dragdata.hxx"

extern Point aDragStartDiff;

bool bPasteIsMove = false;

using namespace com::sun::star;


static void lcl_AdjustInsertPos( ScViewData* pData, Point& rPos, Size& rSize )
{
    SdrPage* pPage = pData->GetScDrawView()->GetModel()->GetPage( static_cast<sal_uInt16>(pData->GetTabNo()) );
    OSL_ENSURE(pPage,"pPage ???");
    Size aPgSize( pPage->GetSize() );
    if (aPgSize.Width() < 0)
        aPgSize.Width() = -aPgSize.Width();
    long x = aPgSize.Width() - rPos.X() - rSize.Width();
    long y = aPgSize.Height() - rPos.Y() - rSize.Height();
    
    if( x < 0 )
        rPos.X() += x + 80;
    if( y < 0 )
        rPos.Y() += y + 200;
    rPos.X() += rSize.Width() / 2;          
    rPos.Y() += rSize.Height() / 2;
}

void ScViewFunc::PasteDraw( const Point& rLogicPos, SdrModel* pModel,
        bool bGroup, bool bSameDocClipboard )
{
    MakeDrawLayer();
    Point aPos( rLogicPos );

    
    
    MapMode aOldMapMode;
    OutputDevice* pRef = GetViewData()->GetDocument()->GetDrawLayer()->GetRefDevice();
    if (pRef)
    {
        aOldMapMode = pRef->GetMapMode();
        pRef->SetMapMode( MapMode(MAP_100TH_MM) );
    }

    bool bNegativePage = GetViewData()->GetDocument()->IsNegativePage( GetViewData()->GetTabNo() );

    SdrView* pDragEditView = NULL;
    ScModule* pScMod = SC_MOD();
    const ScDragData& rData = pScMod->GetDragData();
    ScDrawTransferObj* pDrawTrans = rData.pDrawTransfer;
    if (pDrawTrans)
    {
        pDragEditView = pDrawTrans->GetDragSourceView();

        aPos -= aDragStartDiff;
        if ( bNegativePage )
        {
            if (aPos.X() > 0) aPos.X() = 0;
        }
        else
        {
            if (aPos.X() < 0) aPos.X() = 0;
        }
        if (aPos.Y() < 0) aPos.Y() = 0;
    }

    ScDrawView* pScDrawView = GetScDrawView();
    if (bGroup)
        pScDrawView->BegUndo( ScGlobal::GetRscString( STR_UNDO_PASTE ) );

    bool bSameDoc = ( pDragEditView && pDragEditView->GetModel() == pScDrawView->GetModel() );
    if (bSameDoc)
    {
            

        Point aSourceStart = pDragEditView->GetAllMarkedRect().TopLeft();
        long nDiffX = aPos.X() - aSourceStart.X();
        long nDiffY = aPos.Y() - aSourceStart.Y();

            

        if ( bPasteIsMove &&
                pScDrawView->GetSdrPageView()->GetPage() ==
                pDragEditView->GetSdrPageView()->GetPage() )
        {
            if ( nDiffX != 0 || nDiffY != 0 )
                pDragEditView->MoveAllMarked(Size(nDiffX,nDiffY), false);
        }
        else
        {
            SdrModel* pDrawModel = pDragEditView->GetModel();
            SCTAB nTab = GetViewData()->GetTabNo();
            SdrPage* pDestPage = pDrawModel->GetPage( static_cast< sal_uInt16 >( nTab ) );
            OSL_ENSURE(pDestPage,"who is this, Page?");

            ::std::vector< OUString > aExcludedChartNames;
            if ( pDestPage )
            {
                ScChartHelper::GetChartNames( aExcludedChartNames, pDestPage );
            }

            SdrMarkList aMark = pDragEditView->GetMarkedObjectList();
            aMark.ForceSort();
            sal_uLong nMarkAnz=aMark.GetMarkCount();
            for (sal_uLong nm=0; nm<nMarkAnz; nm++) {
                const SdrMark* pM=aMark.GetMark(nm);
                const SdrObject* pObj=pM->GetMarkedSdrObj();

                SdrObject* pNeuObj=pObj->Clone();

                if (pNeuObj!=NULL)
                {
                    pNeuObj->SetModel(pDrawModel);
                    pNeuObj->SetPage(pDestPage);

                    
                    if ( pNeuObj->ISA(SdrGrafObj) && !bPasteIsMove )
                        pNeuObj->SetName(((ScDrawLayer*)pDrawModel)->GetNewGraphicName());

                    if (nDiffX!=0 || nDiffY!=0)
                        pNeuObj->NbcMove(Size(nDiffX,nDiffY));
                    pDestPage->InsertObject( pNeuObj );
                    pScDrawView->AddUndo(new SdrUndoInsertObj( *pNeuObj ));

                    if (ScDrawLayer::IsCellAnchored(*pNeuObj))
                        ScDrawLayer::SetCellAnchoredFromPosition(*pNeuObj, *GetViewData()->GetDocument(), nTab);
                }
            }

            if (bPasteIsMove)
                pDragEditView->DeleteMarked();

            ScDocument* pDocument = GetViewData()->GetDocument();
            ScDocShell* pDocShell = GetViewData()->GetDocShell();
            ScModelObj* pModelObj = ( pDocShell ? ScModelObj::getImplementation( pDocShell->GetModel() ) : NULL );
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
        bPasteIsMove = false;       

        SdrView aView(pModel);      
        SdrPageView* pPv = aView.ShowSdrPage(aView.GetModel()->GetPage(0));
        aView.MarkAllObj(pPv);
        Size aSize = aView.GetAllMarkedRect().GetSize();
        lcl_AdjustInsertPos( GetViewData(), aPos, aSize );

        
        

        sal_uLong nOptions = 0;
        SfxInPlaceClient* pClient = GetViewData()->GetViewShell()->GetIPClient();
        if ( pClient && pClient->IsObjectInPlaceActive() )
            nOptions |= SDRINSERT_DONTMARK;

        ::std::vector< OUString > aExcludedChartNames;
        SCTAB nTab = GetViewData()->GetTabNo();
        SdrPage* pPage = pScDrawView->GetModel()->GetPage( static_cast< sal_uInt16 >( nTab ) );
        OSL_ENSURE( pPage, "Page?" );
        if ( pPage )
        {
            ScChartHelper::GetChartNames( aExcludedChartNames, pPage );
        }

        
        
        if ( !bSameDocClipboard )
            GetViewData()->GetDocument()->SetPastingDrawFromOtherDoc( true );

        pScDrawView->Paste( *pModel, aPos, NULL, nOptions );

        if ( !bSameDocClipboard )
            GetViewData()->GetDocument()->SetPastingDrawFromOtherDoc( false );

        
        
        if (pPage)
        {
            SdrObjListIter aIter( *pPage, IM_DEEPNOGROUPS );
            SdrObject* pObject = aIter.Next();
            while (pObject)
            {
                if ( pObject->ISA(SdrUnoObj) && pObject->GetLayer() != SC_LAYER_CONTROLS )
                    pObject->NbcSetLayer(SC_LAYER_CONTROLS);

                if (ScDrawLayer::IsCellAnchored(*pObject))
                    ScDrawLayer::SetCellAnchoredFromPosition(*pObject, *GetViewData()->GetDocument(), nTab);

                pObject = aIter.Next();
            }
        }

        
        GetViewData()->GetDocument()->EnsureGraphicNames();

        ScDocument* pDocument = GetViewData()->GetDocument();
        ScDocShell* pDocShell = GetViewData()->GetDocShell();
        ScModelObj* pModelObj = ( pDocShell ? ScModelObj::getImplementation( pDocShell->GetModel() ) : NULL );
        ScDrawTransferObj* pTransferObj = ScDrawTransferObj::GetOwnClipboard( NULL );
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

    
    
    
    
    
    pScDrawView->MarkListHasChanged();

}

bool ScViewFunc::PasteObject( const Point& rPos, const uno::Reference < embed::XEmbeddedObject >& xObj,
                                const Size* pDescSize, const Graphic* pReplGraph, const OUString& aMediaType, sal_Int64 nAspect )
{
    MakeDrawLayer();
    if ( xObj.is() )
    {
        OUString aName;
        
        comphelper::EmbeddedObjectContainer& aCnt = GetViewData()->GetViewShell()->GetObjectShell()->GetEmbeddedObjectContainer();
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
            MapMode aMapMode( MAP_100TH_MM );
            aSize = aObjRef.GetSize( &aMapMode );
        }
        else
        {
            
            MapUnit aMapObj = VCLUnoHelper::UnoEmbed2VCLMapUnit( xObj->getMapUnit( nAspect ) );
            MapUnit aMap100 = MAP_100TH_MM;

            if ( pDescSize && pDescSize->Width() && pDescSize->Height() )
            {
                
                aSize = OutputDevice::LogicToLogic( *pDescSize, aMap100, aMapObj );
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
                
            }

            aSize = Size( aSz.Width, aSz.Height );
            aSize = OutputDevice::LogicToLogic( aSize, aMapObj, aMap100 );  

            if( aSize.Height() == 0 || aSize.Width() == 0 )
            {
                OSL_FAIL("SvObjectDescriptor::GetSize == 0");
                aSize.Width() = 5000;
                aSize.Height() = 5000;
                aSize = OutputDevice::LogicToLogic( aSize, aMap100, aMapObj );
                aSz.Width = aSize.Width();
                aSz.Height = aSize.Height();
                xObj->setVisualAreaSize( nAspect, aSz );
            }
        }

        
        Point aInsPos = rPos;
        if ( GetViewData()->GetDocument()->IsNegativePage( GetViewData()->GetTabNo() ) )
            aInsPos.X() -= aSize.Width();
        Rectangle aRect( aInsPos, aSize );

        ScDrawView* pDrView = GetScDrawView();
        SdrOle2Obj* pSdrObj = new SdrOle2Obj( aObjRef, aName, aRect );

        SdrPageView* pPV = pDrView->GetSdrPageView();
        pDrView->InsertObjectSafe( pSdrObj, *pPV );             
        GetViewData()->GetViewShell()->SetDrawShell( true );
        return true;
    }
    else
        return false;
}

bool ScViewFunc::PasteBitmapEx( const Point& rPos, const BitmapEx& rBmpEx )
{
    OUString aEmpty;
    Graphic aGraphic(rBmpEx);
    return PasteGraphic( rPos, aGraphic, aEmpty, aEmpty );
}

bool ScViewFunc::PasteMetaFile( const Point& rPos, const GDIMetaFile& rMtf )
{
    OUString aEmpty;
    Graphic aGraphic(rMtf);
    return PasteGraphic( rPos, aGraphic, aEmpty, aEmpty );
}

bool ScViewFunc::PasteGraphic( const Point& rPos, const Graphic& rGraphic,
                                const OUString& rFile, const OUString& rFilter )
{
    MakeDrawLayer();
    ScDrawView* pScDrawView = GetScDrawView();

    
    
    
    if(pScDrawView)
    {
        SdrObject* pPickObj = 0;
        SdrPageView* pPageView = pScDrawView->GetSdrPageView();

        if(pPageView)
        {
            pScDrawView->PickObj(rPos, pScDrawView->getHitTolLog(), pPickObj, pPageView);
        }

        if(pPickObj)
        {
            const OUString aBeginUndo(ScGlobal::GetRscString(STR_UNDO_DRAGDROP));
            SdrObject* pResult = pScDrawView->ApplyGraphicToObject(
                *pPickObj,
                rGraphic,
                aBeginUndo,
                rFile,
                rFilter);

            if(pResult)
            {
                
                pScDrawView->MarkObj(pResult, pScDrawView->GetSdrPageView());
                return true;
            }
        }
    }

    Point aPos( rPos );
    Window* pWin = GetActiveWin();
    MapMode aSourceMap = rGraphic.GetPrefMapMode();
    MapMode aDestMap( MAP_100TH_MM );

    if (aSourceMap.GetMapUnit() == MAP_PIXEL)
    {
        
        Fraction aScaleX, aScaleY;
        pScDrawView->CalcNormScale( aScaleX, aScaleY );
        aDestMap.SetScaleX(aScaleX);
        aDestMap.SetScaleY(aScaleY);
    }

    Size aSize = pWin->LogicToLogic( rGraphic.GetPrefSize(), &aSourceMap, &aDestMap );

    if ( GetViewData()->GetDocument()->IsNegativePage( GetViewData()->GetTabNo() ) )
        aPos.X() -= aSize.Width();

    GetViewData()->GetViewShell()->SetDrawShell( true );
    Rectangle aRect(aPos, aSize);
    SdrGrafObj* pGrafObj = new SdrGrafObj(rGraphic, aRect);

    

    ScDrawLayer* pLayer = (ScDrawLayer*) pScDrawView->GetModel();
    OUString aName = pLayer->GetNewGraphicName();                 
    pGrafObj->SetName(aName);

    
    pScDrawView->InsertObjectSafe(pGrafObj, *pScDrawView->GetSdrPageView());

    
    
    
    if (!rFile.isEmpty())
        pGrafObj->SetGraphicLink( rFile, ""/*TODO?*/, rFilter );

    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
