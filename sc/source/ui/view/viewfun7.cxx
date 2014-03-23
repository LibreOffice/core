/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"
#include <com/sun/star/embed/NoVisualAreaSizeException.hpp>



// INCLUDE ---------------------------------------------------------------

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
#include <svx/svdlegacy.hxx>

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

extern basegfx::B2DPoint aDragStartDiff;

// STATIC DATA -----------------------------------------------------------

sal_Bool bPasteIsMove = sal_False;

using namespace com::sun::star;

//==================================================================

void lcl_AdjustInsertPos( ScViewData* pData, basegfx::B2DPoint& rPos, basegfx::B2DVector& rSize )
{
//  SdrPage* pPage = pData->GetDocument()->GetDrawLayer()->GetPage( pData->GetTabNo() );
    SdrPage* pPage = pData->GetScDrawView()->getSdrModelFromSdrView().GetPage( static_cast< sal_uInt32 >(pData->GetTabNo()) );
    DBG_ASSERT(pPage,"pPage ???");
    const basegfx::B2DVector aPgScale(absolute(pPage->GetPageScale()));
    basegfx::B2DVector aDelta(aPgScale - rPos - rSize);

    if(aDelta.getX() < 0.0)
    {
        rPos.setX(rPos.getX() + aDelta.getX() + 80.0);
    }

    if(aDelta.getY() < 0.0)
    {
        rPos.setY(rPos.getY() + aDelta.getY() + 200.0);
    }

    rPos += rSize * 0.5;
}

void ScViewFunc::PasteDraw( const basegfx::B2DPoint& rLogicPos, SdrModel* pModel,
        sal_Bool bGroup, sal_Bool bSameDocClipboard )
{
    MakeDrawLayer();
    basegfx::B2DPoint aPos( rLogicPos );

    //  #64184# MapMode am Outliner-RefDevice muss stimmen (wie in FuText::MakeOutliner)
    //! mit FuText::MakeOutliner zusammenfassen?
    MapMode aOldMapMode;
    OutputDevice* pRef = GetViewData()->GetDocument()->GetDrawLayer()->GetReferenceDevice();
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
            if (aPos.getX() > 0.0) aPos.setX(0.0);
        }
        else
        {
            if (aPos.getX() < 0.0) aPos.setX(0.0);
        }

        if (aPos.getY() < 0.0) aPos.setY(0.0);
    }

    ScDrawView* pScDrawView = GetScDrawView();
    if (bGroup)
        pScDrawView->BegUndo( ScGlobal::GetRscString( STR_UNDO_PASTE ) );

    sal_Bool bSameDoc = ( pDragEditView && &pDragEditView->getSdrModelFromSdrView() == &pScDrawView->getSdrModelFromSdrView() );
    if (bSameDoc)
    {
            // lokal kopieren - incl. Charts

        const basegfx::B2DPoint aSourceStart(pDragEditView->getMarkedObjectSnapRange().getMinimum());
        const basegfx::B2DVector aDiff(aPos - aSourceStart);

            // innerhalb einer Page verschieben?

        if ( bPasteIsMove
            && pScDrawView->GetSdrPageView()
            && pDragEditView->GetSdrPageView()
            && &pScDrawView->GetSdrPageView()->getSdrPageFromSdrPageView() == &pDragEditView->GetSdrPageView()->getSdrPageFromSdrPageView() )
        {
            if ( !aDiff.equalZero() )
                pDragEditView->MoveMarkedObj(aDiff, false);
        }
        else
        {
            const SCTAB nTab = GetViewData()->GetTabNo();
            SdrPage* pDestPage = pDragEditView->getSdrModelFromSdrView().GetPage( static_cast<sal_uInt16>(nTab) );
            DBG_ASSERT(pDestPage,"nanu, Page?");

            ::std::vector< ::rtl::OUString > aExcludedChartNames;
            if ( pDestPage )
            {
                ScChartHelper::GetChartNames( aExcludedChartNames, pDestPage );
            }

            const SdrObjectVector aSelection(pDragEditView->getSelectedSdrObjectVectorFromSdrMarkView());

            for (sal_uInt32 nm(0); nm < aSelection.size(); nm++)
            {
                SdrObject* pNeuObj = aSelection[nm]->CloneSdrObject(&pDestPage->getSdrModelFromSdrPage());

                if (pNeuObj)
                {
                    //  #68787# copy graphics within the same model - always needs new name
                    if ( dynamic_cast< SdrGrafObj* >(pNeuObj) && !bPasteIsMove )
                    {
                        ScDrawLayer& rScDrawLayer = dynamic_cast< ScDrawLayer& >(pDragEditView->getSdrModelFromSdrView());
                        pNeuObj->SetName(rScDrawLayer.GetNewGraphicName());
                    }

                    if ( !aDiff.equalZero())
                        sdr::legacy::transformSdrObject(*pNeuObj, basegfx::tools::createTranslateB2DHomMatrix(aDiff));
                    pDestPage->InsertObjectToSdrObjList(*pNeuObj);
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
        bPasteIsMove = sal_False;       // kein internes Verschieben passiert

        OSL_ENSURE(pModel, "ScViewFunc::PasteDraw without SdrModel (!)");
        SdrView aView(*pModel);      // #i71529# never create a base class of SdrView directly!
        aView.ShowSdrPage(*aView.getSdrModelFromSdrView().GetPage(0));
        aView.MarkAllObj();
        basegfx::B2DVector aScale(aView.getMarkedObjectSnapRange().getRange());
        lcl_AdjustInsertPos( GetViewData(), aPos, aScale );

        //  #41333# Markierung nicht aendern, wenn Ole-Objekt aktiv
        //  (bei Drop aus Ole-Objekt wuerde sonst mitten im ExecuteDrag deaktiviert!)

        sal_uLong nOptions = 0;
        SfxInPlaceClient* pClient = GetViewData()->GetViewShell()->GetIPClient();
        if ( pClient && pClient->IsObjectInPlaceActive() )
            nOptions |= SDRINSERT_DONTMARK;

        ::std::vector< ::rtl::OUString > aExcludedChartNames;
        const SCTAB nTab = GetViewData()->GetTabNo();
        SdrPage* pPage = pScDrawView->getSdrModelFromSdrView().GetPage(static_cast<sal_uInt16>(nTab));
        DBG_ASSERT( pPage, "Page?" );
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
            GetViewData()->GetDocument()->SetPastingDrawFromOtherDoc( sal_False );

        // #68991# Paste puts all objects on the active (front) layer
        // controls must be on SC_LAYER_CONTROLS
        if (pPage)
        {
            SdrObjListIter aIter( *pPage, IM_DEEPNOGROUPS );
            SdrObject* pObject = aIter.Next();
            while (pObject)
            {
                if ( dynamic_cast< SdrUnoObj* >(pObject) && pObject->GetLayer() != SC_LAYER_CONTROLS )
                {
                    pObject->SetLayer(SC_LAYER_CONTROLS);
                }

                pObject = aIter.Next();
            }
        }

        // #75299# all graphics objects must have names
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
}

sal_Bool ScViewFunc::PasteObject( const basegfx::B2DPoint& rPos, const uno::Reference < embed::XEmbeddedObject >& xObj,
    const basegfx::B2DVector* pDescSize, const Graphic* pReplGraph, const ::rtl::OUString& aMediaType, sal_Int64 nAspect )
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

        basegfx::B2DVector aScale(1.0, 1.0);

        if ( nAspect == embed::Aspects::MSOLE_ICON )
        {
            MapMode aMapMode( MAP_100TH_MM );
            const Size aOldSize(aObjRef.GetSize(&aMapMode));

            aScale = basegfx::B2DVector(aOldSize.Width(), aOldSize.Height());
        }
        else
        {
            // working with visual area can switch object to running state
            MapUnit aMapObj = VCLUnoHelper::UnoEmbed2VCLMapUnit( xObj->getMapUnit( nAspect ) );
            MapUnit aMap100 = MAP_100TH_MM;

            if(pDescSize && !pDescSize->equalZero())
            {
                // use size from object descriptor if given
                aScale = *pDescSize * OutputDevice::GetFactorLogicToLogic(aMap100, aMapObj);

                awt::Size aSz;

                aSz.Width = basegfx::fround(aScale.getX());
                aSz.Height = basegfx::fround(aScale.getY());

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

            aScale = basegfx::B2DVector( aSz.Width, aSz.Height );
            aScale *= OutputDevice::GetFactorLogicToLogic(aMapObj, aMap100);    // fuer SdrOle2Obj

            if(basegfx::fTools::equalZero(aScale.getY()) || basegfx::fTools::equalZero(aScale.getX()))
            {
                DBG_ERROR("SvObjectDescriptor::GetSize == 0");
                aScale.setX(5000.0);
                aScale.setY(5000.0);
                aScale *= OutputDevice::GetFactorLogicToLogic(aMap100, aMapObj);
                aSz.Width = basegfx::fround(aScale.getX());
                aSz.Height = basegfx::fround(aScale.getY());
                xObj->setVisualAreaSize( nAspect, aSz );
            }
        }

        // don't call AdjustInsertPos
        basegfx::B2DPoint aInsPos(rPos);

        if ( GetViewData()->GetDocument()->IsNegativePage( GetViewData()->GetTabNo() ) )
        {
            aInsPos.setX(aInsPos.getX() - aScale.getX());
        }

        ScDrawView* pDrView = GetScDrawView();
        SdrOle2Obj* pSdrObj = new SdrOle2Obj(
            pDrView->getSdrModelFromSdrView(),
            aObjRef,
            aName,
            basegfx::tools::createScaleTranslateB2DHomMatrix(
                aScale,
                aInsPos));

        pDrView->InsertObjectSafe( *pSdrObj );
        GetViewData()->GetViewShell()->SetDrawShell( sal_True );
        return sal_True;
    }
    else
        return sal_False;
}

sal_Bool ScViewFunc::PasteBitmapEx( const basegfx::B2DPoint& rPos, const BitmapEx& rBmpEx )
{
    String aEmpty;
    Graphic aGraphic(rBmpEx);
    return PasteGraphic( rPos, aGraphic, aEmpty, aEmpty );
}

sal_Bool ScViewFunc::PasteMetaFile( const basegfx::B2DPoint& rPos, const GDIMetaFile& rMtf )
{
    String aEmpty;
    Graphic aGraphic(rMtf);
    return PasteGraphic( rPos, aGraphic, aEmpty, aEmpty );
}

sal_Bool ScViewFunc::PasteGraphic( const basegfx::B2DPoint& rPos, const Graphic& rGraphic,
                                const String& rFile, const String& rFilter )
{
    MakeDrawLayer();
    ScDrawView* pScDrawView = GetScDrawView();

    // #123922# check if the drop was over an existing object; if yes, evtl. replace
    // the graphic for a SdrGraphObj (including link state updates) or adapt the fill
    // style for other objects
    if(pScDrawView)
    {
        SdrObject* pPickObj = 0;
        pScDrawView->PickObj(rPos, pScDrawView->getHitTolLog(), pPickObj);

        if(pPickObj)
        {
            const String aBeginUndo(ScGlobal::GetRscString(STR_UNDO_DRAGDROP));
            SdrObject* pResult = pScDrawView->ApplyGraphicToObject(
                *pPickObj,
                rGraphic,
                aBeginUndo,
                rFile,
                rFilter);

            if(pResult)
            {
                // we are done; mark the modified/new object
                pScDrawView->MarkObj(*pResult);
                return sal_True;
            }
        }
    }

    basegfx::B2DPoint aPos( rPos );
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

    const basegfx::B2DHomMatrix aTransSourceMap2DestMap(pWin->GetTransformLogicToLogic(aSourceMap, aDestMap));
    const basegfx::B2DVector aScale(aTransSourceMap2DestMap * basegfx::B2DVector(rGraphic.GetPrefSize().Width(), rGraphic.GetPrefSize().Height()));

//  lcl_AdjustInsertPos( GetViewData(), aPos, aSize );
    if ( GetViewData()->GetDocument()->IsNegativePage( GetViewData()->GetTabNo() ) )
    {
        aPos.setX(aPos.getX() - aScale.getX());
    }

    GetViewData()->GetViewShell()->SetDrawShell( sal_True );

    SdrGrafObj* pGrafObj = new SdrGrafObj(
        pScDrawView->getSdrModelFromSdrView(),
        rGraphic,
        basegfx::tools::createScaleTranslateB2DHomMatrix(
            aScale,
            aPos));

    // #118522# calling SetGraphicLink here doesn't work

    //  #49961# Pfad wird nicht mehr als Name der Grafik gesetzt

    ScDrawLayer& rLayer = dynamic_cast< ScDrawLayer& >(pScDrawView->getSdrModelFromSdrView());
    String aName = rLayer.GetNewGraphicName();                  // "Grafik x"
    pGrafObj->SetName(aName);

    // nicht markieren wenn Ole
    pScDrawView->InsertObjectSafe(*pGrafObj);

    // #118522# SetGraphicLink has to be used after inserting the object,
    // otherwise an empty graphic is swapped in and the contact stuff crashes.
    // See #i37444#.
    if (rFile.Len())
        pGrafObj->SetGraphicLink( rFile, rFilter );

    return sal_True;
}

// eof
