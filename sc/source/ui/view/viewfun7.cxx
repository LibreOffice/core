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
#include <svx/svdoole2.hxx>
#include <svx/svdouno.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdundo.hxx>
#include <svx/xbitmap.hxx>
#include <svx/xbtmpit.hxx>
#include <svx/xoutbmp.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/viewfrm.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <com/sun/star/embed/Aspects.hpp>

#include "document.hxx"     // fuer MapMode Initialisierung in PasteDraw
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

// STATIC DATA -----------------------------------------------------------

sal_Bool bPasteIsMove = false;

using namespace com::sun::star;

//==================================================================

static void lcl_AdjustInsertPos( ScViewData* pData, Point& rPos, Size& rSize )
{
    SdrPage* pPage = pData->GetScDrawView()->GetModel()->GetPage( static_cast<sal_uInt16>(pData->GetTabNo()) );
    OSL_ENSURE(pPage,"pPage ???");
    Size aPgSize( pPage->GetSize() );
    if (aPgSize.Width() < 0)
        aPgSize.Width() = -aPgSize.Width();
    long x = aPgSize.Width() - rPos.X() - rSize.Width();
    long y = aPgSize.Height() - rPos.Y() - rSize.Height();
    // ggf. Ajustments (80/200) fuer Pixel-Rundungsfehler
    if( x < 0 )
        rPos.X() += x + 80;
    if( y < 0 )
        rPos.Y() += y + 200;
    rPos.X() += rSize.Width() / 2;          // Position bei Paste gibt Mittelpunkt an
    rPos.Y() += rSize.Height() / 2;
}

void ScViewFunc::PasteDraw( const Point& rLogicPos, SdrModel* pModel,
        sal_Bool bGroup, sal_Bool bSameDocClipboard )
{
    MakeDrawLayer();
    Point aPos( rLogicPos );

    //  MapMode am Outliner-RefDevice muss stimmen (wie in FuText::MakeOutliner)
    //! mit FuText::MakeOutliner zusammenfassen?
    MapMode aOldMapMode;
    OutputDevice* pRef = GetViewData()->GetDocument()->GetDrawLayer()->GetRefDevice();
    if (pRef)
    {
        aOldMapMode = pRef->GetMapMode();
        pRef->SetMapMode( MapMode(MAP_100TH_MM) );
    }

    sal_Bool bNegativePage = GetViewData()->GetDocument()->IsNegativePage( GetViewData()->GetTabNo() );

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

    sal_Bool bSameDoc = ( pDragEditView && pDragEditView->GetModel() == pScDrawView->GetModel() );
    if (bSameDoc)
    {
            // lokal kopieren - incl. Charts

        Point aSourceStart = pDragEditView->GetAllMarkedRect().TopLeft();
        long nDiffX = aPos.X() - aSourceStart.X();
        long nDiffY = aPos.Y() - aSourceStart.Y();

            // innerhalb einer Page verschieben?

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
            OSL_ENSURE(pDestPage,"nanu, Page?");

            ::std::vector< ::rtl::OUString > aExcludedChartNames;
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

                    //  copy graphics within the same model - always needs new name
                    if ( pNeuObj->ISA(SdrGrafObj) && !bPasteIsMove )
                        pNeuObj->SetName(((ScDrawLayer*)pDrawModel)->GetNewGraphicName());

                    if (nDiffX!=0 || nDiffY!=0)
                        pNeuObj->NbcMove(Size(nDiffX,nDiffY));
                    pDestPage->InsertObject( pNeuObj );
                    pScDrawView->AddUndo(new SdrUndoInsertObj( *pNeuObj ));

                    //  Chart braucht nicht mehr getrennt behandelt zu werden,
                    //  weil es seine Daten jetzt selber hat
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
        bPasteIsMove = false;       // kein internes Verschieben passiert

        SdrView aView(pModel);      // #i71529# never create a base class of SdrView directly!
        SdrPageView* pPv = aView.ShowSdrPage(aView.GetModel()->GetPage(0));
        aView.MarkAllObj(pPv);
        Size aSize = aView.GetAllMarkedRect().GetSize();
        lcl_AdjustInsertPos( GetViewData(), aPos, aSize );

        //  Markierung nicht aendern, wenn Ole-Objekt aktiv
        //  (bei Drop aus Ole-Objekt wuerde sonst mitten im ExecuteDrag deaktiviert!)

        sal_uLong nOptions = 0;
        SfxInPlaceClient* pClient = GetViewData()->GetViewShell()->GetIPClient();
        if ( pClient && pClient->IsObjectInPlaceActive() )
            nOptions |= SDRINSERT_DONTMARK;

        ::std::vector< ::rtl::OUString > aExcludedChartNames;
        SCTAB nTab = GetViewData()->GetTabNo();
        SdrPage* pPage = pScDrawView->GetModel()->GetPage( static_cast< sal_uInt16 >( nTab ) );
        OSL_ENSURE( pPage, "Page?" );
        if ( pPage )
        {
            ScChartHelper::GetChartNames( aExcludedChartNames, pPage );
        }

        // #89247# Set flag for ScDocument::UpdateChartListeners() which is
        // called during paste.
        if ( !bSameDocClipboard )
            GetViewData()->GetDocument()->SetPastingDrawFromOtherDoc( sal_True );

        pScDrawView->Paste( *pModel, aPos, NULL, nOptions );

        if ( !bSameDocClipboard )
            GetViewData()->GetDocument()->SetPastingDrawFromOtherDoc( false );

        // Paste puts all objects on the active (front) layer
        // controls must be on SC_LAYER_CONTROLS
        if (pPage)
        {
            SdrObjListIter aIter( *pPage, IM_DEEPNOGROUPS );
            SdrObject* pObject = aIter.Next();
            while (pObject)
            {
                if ( pObject->ISA(SdrUnoObj) && pObject->GetLayer() != SC_LAYER_CONTROLS )
                    pObject->NbcSetLayer(SC_LAYER_CONTROLS);
                pObject = aIter.Next();
            }
        }

        // all graphics objects must have names
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

    // GetViewData()->GetViewShell()->SetDrawShell( TRUE );
    // It is not sufficient to just set the DrawShell if we pasted, for
    // example, a chart.  SetDrawShellOrSub() would only work for D&D in the
    // same document but not if inserting from the clipboard, therefore
    // MarkListHasChanged() is what we need.
    pScDrawView->MarkListHasChanged();

}

sal_Bool ScViewFunc::PasteObject( const Point& rPos, const uno::Reference < embed::XEmbeddedObject >& xObj,
                                const Size* pDescSize, const Graphic* pReplGraph, const ::rtl::OUString& aMediaType, sal_Int64 nAspect )
{
    MakeDrawLayer();
    if ( xObj.is() )
    {
        ::rtl::OUString aName;
        //TODO/MBA: is that OK?
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
            // working with visual area can switch object to running state
            MapUnit aMapObj = VCLUnoHelper::UnoEmbed2VCLMapUnit( xObj->getMapUnit( nAspect ) );
            MapUnit aMap100 = MAP_100TH_MM;

            if ( pDescSize && pDescSize->Width() && pDescSize->Height() )
            {
                // use size from object descriptor if given
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
                // the default size will be set later
            }

            aSize = Size( aSz.Width, aSz.Height );
            aSize = OutputDevice::LogicToLogic( aSize, aMapObj, aMap100 );  // fuer SdrOle2Obj

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

        // don't call AdjustInsertPos
        Point aInsPos = rPos;
        if ( GetViewData()->GetDocument()->IsNegativePage( GetViewData()->GetTabNo() ) )
            aInsPos.X() -= aSize.Width();
        Rectangle aRect( aInsPos, aSize );

        ScDrawView* pDrView = GetScDrawView();
        SdrOle2Obj* pSdrObj = new SdrOle2Obj( aObjRef, aName, aRect );

        SdrPageView* pPV = pDrView->GetSdrPageView();
        pDrView->InsertObjectSafe( pSdrObj, *pPV );             // nicht markieren wenn Ole
        GetViewData()->GetViewShell()->SetDrawShell( sal_True );
        return sal_True;
    }
    else
        return false;
}

sal_Bool ScViewFunc::PasteBitmap( const Point& rPos, const Bitmap& rBmp )
{
    String aEmpty;
    Graphic aGraphic(rBmp);
    return PasteGraphic( rPos, aGraphic, aEmpty, aEmpty );
}

sal_Bool ScViewFunc::PasteMetaFile( const Point& rPos, const GDIMetaFile& rMtf )
{
    String aEmpty;
    Graphic aGraphic(rMtf);
    return PasteGraphic( rPos, aGraphic, aEmpty, aEmpty );
}

sal_Bool ScViewFunc::PasteGraphic( const Point& rPos, const Graphic& rGraphic,
                                const String& rFile, const String& rFilter )
{
    MakeDrawLayer();
    ScDrawView* pScDrawView = GetScDrawView();

    Point aPos( rPos );
    Window* pWin = GetActiveWin();
    MapMode aSourceMap = rGraphic.GetPrefMapMode();
    MapMode aDestMap( MAP_100TH_MM );

    if (aSourceMap.GetMapUnit() == MAP_PIXEL)
    {
            //  Pixel-Korrektur beruecksichtigen, damit Bitmap auf dem Bildschirm stimmt

        Fraction aScaleX, aScaleY;
        pScDrawView->CalcNormScale( aScaleX, aScaleY );
        aDestMap.SetScaleX(aScaleX);
        aDestMap.SetScaleY(aScaleY);
    }

    Size aSize = pWin->LogicToLogic( rGraphic.GetPrefSize(), &aSourceMap, &aDestMap );

    if ( GetViewData()->GetDocument()->IsNegativePage( GetViewData()->GetTabNo() ) )
        aPos.X() -= aSize.Width();

    GetViewData()->GetViewShell()->SetDrawShell( sal_True );

    Rectangle aRect(aPos, aSize);
    SdrGrafObj* pGrafObj = new SdrGrafObj(rGraphic, aRect);

    // calling SetGraphicLink here doesn't work

    //  Pfad wird nicht mehr als Name der Grafik gesetzt

    ScDrawLayer* pLayer = (ScDrawLayer*) pScDrawView->GetModel();
    String aName = pLayer->GetNewGraphicName();                 // "Grafik x"
    pGrafObj->SetName(aName);

    // nicht markieren wenn Ole
    pScDrawView->InsertObjectSafe(pGrafObj, *pScDrawView->GetSdrPageView());

    // SetGraphicLink has to be used after inserting the object,
    // otherwise an empty graphic is swapped in and the contact stuff crashes.
    // See #i37444#.
    if (rFile.Len())
        pGrafObj->SetGraphicLink( rFile, rFilter );

    return sal_True;
}

sal_Bool ScViewFunc::ApplyGraphicToObject( SdrObject* pPickObj, const Graphic& rGraphic )
{
    sal_Bool bRet = false;
    SdrGrafObj* pNewGrafObj = NULL;

    ScDrawView* pScDrawView = GetScDrawView();
    if ( pScDrawView && pPickObj )
    {
        /**********************************************************************
        * Objekt neu attributieren
        **********************************************************************/
        SdrPageView* pPV = pScDrawView->GetSdrPageView();
        if (pPickObj->ISA(SdrGrafObj))
        {
            /******************************************************************
            * Das Graphik-Objekt bekommt eine neue Graphik
            ******************************************************************/
            pNewGrafObj = (SdrGrafObj*) pPickObj->Clone();
            pNewGrafObj->SetGraphic(rGraphic);

            pScDrawView->BegUndo(ScGlobal::GetRscString(STR_UNDO_DRAGDROP));
            pScDrawView->ReplaceObjectAtView(pPickObj, *pPV, pNewGrafObj);
            pScDrawView->EndUndo();

            bRet = sal_True;
        }
        else if (pPickObj->IsClosedObj() && !pPickObj->ISA(SdrOle2Obj))
        {
            /******************************************************************
            * Das Objekt wird mit der Graphik gefuellt
            ******************************************************************/
            //pScDrawView->BegUndo(ScGlobal::GetRscString(STR_UNDO_DRAGDROP));
            pScDrawView->AddUndo(new SdrUndoAttrObj(*pPickObj));
            //pScDrawView->EndUndo();

            XOBitmap aXOBitmap( rGraphic.GetBitmap() );
            SfxItemSet aSet( pScDrawView->GetModel()->GetItemPool(),
                                XATTR_FILLSTYLE, XATTR_FILLBITMAP );
            aSet.Put(XFillStyleItem(XFILL_BITMAP));
            aSet.Put(XFillBitmapItem(String(), aXOBitmap));

            pPickObj->SetMergedItemSetAndBroadcast(aSet);

            bRet = sal_True;
        }
    }
    return bRet;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
