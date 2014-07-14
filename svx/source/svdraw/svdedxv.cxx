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
#include "precompiled_svx.hxx"

#include <com/sun/star/i18n/WordType.hpp>
#include <svtools/accessibilityoptions.hxx>
#include <svx/svdedxv.hxx>
#include <svl/solar.hrc>
#include <svl/itemiter.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/hatch.hxx>
#include <svl/whiter.hxx>
#include <svl/style.hxx>
#include <editeng/editstat.hxx>
#include <tools/config.hxx>
#include <vcl/cursor.hxx>
#include <editeng/unotext.hxx>
#include <editeng/editeng.hxx>
#include <editeng/editobj.hxx>
#include <editeng/outlobj.hxx>
#include <editeng/scripttypeitem.hxx>
#include <editeng/editdata.hxx>
#include <svx/svditext.hxx>
#include <svx/svdoutl.hxx>
#include <svx/sdtfchim.hxx>
#include <svx/svdotext.hxx>
#include <svx/svdundo.hxx>
#include <svx/svditer.hxx>
#include "svx/svdpagv.hxx"
#include "svx/svdpage.hxx"
#include "svx/svdetc.hxx"
#include "svx/svdotable.hxx"
#include <svx/selectioncontroller.hxx>

#ifdef DBG_UTIL
#include <svdibrow.hxx>
#endif
#include <svx/svdoutl.hxx>
#include <svx/svddrgv.hxx>
#include "svx/svdstr.hrc"
#include "svx/svdglob.hxx"
#include "svx/globl3d.hxx"
#include <editeng/outliner.hxx>
#include <editeng/adjitem.hxx>
#include <svtools/colorcfg.hxx>
#include <vcl/svapp.hxx>
#include <svx/svdlegacy.hxx>
#include <svx/sdrpaintwindow.hxx>
#include <svx/sdrtexthelpers.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <svx/sdrundomanager.hxx>
#include <svx/sdr/overlay/overlaytools.hxx>
#include <drawinglayer/processor2d/processor2dtools.hxx>

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrObjEditView::SdrObjEditView(SdrModel& rModel1, OutputDevice* pOut)
:   SdrGlueEditView(rModel1, pOut),
    mxTextEditObj(),
    mpTextEditOutliner(0),
    mpTextEditOutlinerView(0),
    mpTextEditWin(0),
    mpTextEditCursorMerker(0),
    mpMacroObj(0),
    mpMacroWin(0),
    maTextEditArea(),
    maMinTextEditArea(),
    maOldCalcFieldValueLink(),
    maMacroDownPos(0.0, 0.0),
    mnMacroTol(0),
    mbTextEditDontDelete(false),
    mbTextEditOnlyOneView(false),
    mbTextEditNewObj(false),
    mbQuickTextEditMode(true),
    mbMacroDown(false),
    mxSelectionController(),
    mxLastSelectionController(),
    mpOldTextEditUndoManager(0)
{
}

SdrObjEditView::~SdrObjEditView()
{
    mpTextEditWin = 0; // Damit es in SdrEndTextEdit kein ShowCursor gibt

    if(IsTextEdit())
    {
        SdrEndTextEdit();
    }

    if(GetTextEditOutliner())
    {
        delete mpTextEditOutliner;
    }

    if(mpOldTextEditUndoManager)
    {
        delete mpOldTextEditUndoManager;
    }
}

bool SdrObjEditView::IsAction() const
{
    return IsMacroObj() || SdrGlueEditView::IsAction();
}

void SdrObjEditView::MovAction(const basegfx::B2DPoint& rPnt)
{
    if(IsMacroObj())
    {
        MovMacroObj(rPnt);
    }

    SdrGlueEditView::MovAction(rPnt);
}

void SdrObjEditView::EndAction()
{
    if(IsMacroObj())
    {
        EndMacroObj();
    }

    SdrGlueEditView::EndAction();
}

void SdrObjEditView::BckAction()
{
    BrkMacroObj();
    SdrGlueEditView::BckAction();
}

void SdrObjEditView::BrkAction()
{
    BrkMacroObj();
    SdrGlueEditView::BrkAction();
}

basegfx::B2DRange SdrObjEditView::TakeActionRange() const
{
    if(IsMacroObj())
    {
        return mpMacroObj->getObjectRange(getAsSdrView());
    }
    else
    {
        return SdrGlueEditView::TakeActionRange();
    }
}

void __EXPORT SdrObjEditView::Notify(SfxBroadcaster& rBC, const SfxHint& rHint)
{
    SdrGlueEditView::Notify(rBC,rHint);

    // Printerwechsel waerend des Editierens
    const SdrBaseHint* pSdrHint = dynamic_cast< const SdrBaseHint* >(&rHint);

    if(pSdrHint && GetTextEditOutliner())
    {
        const SdrHintKind eKind(pSdrHint->GetSdrHintKind());

        if(HINT_REFDEVICECHG == eKind)
        {
            GetTextEditOutliner()->SetRefDevice(getSdrModelFromSdrView().GetReferenceDevice());
        }

        if(HINT_DEFAULTTABCHG == eKind)
        {
            GetTextEditOutliner()->SetDefTab(getSdrModelFromSdrView().GetDefaultTabulator());
        }

//      if(eKind==HINT_DEFFONTHGTCHG)
//      {
//            // ...
//      }
    }
}

void SdrObjEditView::LazyReactOnObjectChanges()
{
    // call parent
    SdrGlueEditView::LazyReactOnObjectChanges();

    if(mxTextEditObj.is() && !mxTextEditObj->IsObjectInserted())
    {
        SdrEndTextEdit(); // Objekt geloescht
    }

    // TextEditObj geaendert?
    if(IsTextEdit())
    {
        SdrTextObj* pTextObj=dynamic_cast<SdrTextObj*>( mxTextEditObj.get() );

        if(pTextObj)
        {
            SdrOutliner* pTextEditOutliner = GetTextEditOutliner();
            OSL_ENSURE(pTextEditOutliner, "TextEdit, but no TextEditOutliner (!)");

            const sal_uInt32 nOutlViewAnz(pTextEditOutliner->GetViewCount());
            bool bAreaChg(false);
            bool bAnchorChg(false);
            bool bColorChg(false);
            bool bContourFrame(pTextObj->IsContourTextFrame());
            EVAnchorMode eNewAnchor(ANCHOR_VCENTER_HCENTER);
            Color aNewColor;

            basegfx::B2DRange aOldArea(maMinTextEditArea);

            aOldArea.expand(maTextEditArea);

            { // Area Checken
                basegfx::B2DVector aPaperMinVector1;
                basegfx::B2DVector aPaperMaxVector1;
                basegfx::B2DRange aEditArea1;
                basegfx::B2DRange aMinArea1;

                pTextObj->TakeTextEditArea(&aPaperMinVector1, &aPaperMaxVector1, &aEditArea1, &aMinArea1);

                const Size aPaperMin1(basegfx::fround(aPaperMinVector1.getX()), basegfx::fround(aPaperMinVector1.getY()));
                const Size aPaperMax1(basegfx::fround(aPaperMaxVector1.getX()), basegfx::fround(aPaperMaxVector1.getY()));

                // #108784#
                const basegfx::B2DHomMatrix aTextEditOffsetTransform(
                    basegfx::tools::createTranslateB2DHomMatrix(
                        pTextObj->GetTextEditOffset()));

                aEditArea1.transform(aTextEditOffsetTransform);
                aMinArea1.transform(aTextEditOffsetTransform);

                basegfx::B2DRange aNewArea(aMinArea1);

                aNewArea.expand(aEditArea1);

                if(!aNewArea.equal(aOldArea)
                    || !aEditArea1.equal(maTextEditArea)
                    || !aMinArea1.equal(maMinTextEditArea)
                    || pTextEditOutliner->GetMinAutoPaperSize() != aPaperMin1
                    || pTextEditOutliner->GetMaxAutoPaperSize() != aPaperMax1)
                {
                    maTextEditArea = aEditArea1;
                    maMinTextEditArea = aMinArea1;
                    pTextEditOutliner->SetUpdateMode(false);
                    pTextEditOutliner->SetMinAutoPaperSize(aPaperMin1);
                    pTextEditOutliner->SetMaxAutoPaperSize(aPaperMax1);
                    pTextEditOutliner->SetPaperSize(Size(0,0)); // Damit der Outliner neu formatiert

                    if(!bContourFrame)
                    {
                        pTextEditOutliner->ClearPolygon();
                        pTextEditOutliner->SetControlWord(pTextEditOutliner->GetControlWord() | EE_CNTRL_AUTOPAGESIZE);
                    }
                    else
                    {
                        pTextEditOutliner->SetControlWord(pTextEditOutliner->GetControlWord() &~EE_CNTRL_AUTOPAGESIZE);
                        basegfx::B2DPolyPolygon aContourOutline(getAlignedTextContourPolyPolygon(*pTextObj));
                        aContourOutline.transform(basegfx::tools::createTranslateB2DHomMatrix(-aContourOutline.getB2DRange().getMinimum()));
                        pTextEditOutliner->SetPolygon(aContourOutline);
                    }

                    for(sal_uInt32 nOV(0); nOV < nOutlViewAnz; nOV++)
                    {
                        OutlinerView* pOLV=pTextEditOutliner->GetView(nOV);
                        sal_uInt32 nStat0 = pOLV->GetControlWord();
                        sal_uInt32 nStat = nStat0;

                        // AutoViewSize nur wenn nicht KontourFrame.
                        if(!bContourFrame)
                        {
                            nStat |= EV_CNTRL_AUTOSIZE;
                        }
                        else
                        {
                            nStat &= ~EV_CNTRL_AUTOSIZE;
                        }

                        if(nStat != nStat0)
                        {
                            pOLV->SetControlWord(nStat);
                        }
                    }

                    pTextEditOutliner->SetUpdateMode(true);
                    bAreaChg = true;
                }
            }

            if(GetTextEditOutlinerView())
            {
                // Fuellfarbe und Anker checken
                EVAnchorMode eOldAnchor = GetTextEditOutlinerView()->GetAnchorMode();
                eNewAnchor=(EVAnchorMode)pTextObj->GetOutlinerViewAnchorMode();
                bAnchorChg=eOldAnchor!=eNewAnchor;
                Color aOldColor(GetTextEditOutlinerView()->GetBackgroundColor());
                aNewColor = GetTextEditBackgroundColor(*this);
                bColorChg=aOldColor!=aNewColor;
            }

            // #104082# refresh always when it's a contour frame. That
            // refresh is necessary since it triggers the repaint
            // which makes the Handles visible. Changes at TakeTextRange()
            // seem to have resulted in a case where no refresh is executed.
            // Before that, a refresh must have been always executed
            // (else this error would have happened earlier), thus i
            // even think here a refresh should be done always.
            // Since follow-up problems cannot even be guessed I only
            // add this one more case to the if below.
            // BTW: It's VERY bad style that here, inside LazyReactOnObjectChanges()
            // the outliner is again massively changed for the text object
            // in text edit mode. Normally, all necessary data should be
            // set at SdrBeginTextEdit(). Some changes and value assigns in
            // SdrBeginTextEdit() are completely useless since they are set here
            // again on LazyReactOnObjectChanges().
            if (bContourFrame || bAreaChg || bAnchorChg || bColorChg)
            {
                for(sal_uInt32 nOV(0); nOV < nOutlViewAnz; nOV++)
                {
                    OutlinerView* pOLV=pTextEditOutliner->GetView(nOV);

                    {
                        // Alten OutlinerView-Bereich invalidieren
                        Window* pWin=pOLV->GetWindow();
                        basegfx::B2DRange aRange(aOldArea);
                        const basegfx::B2DVector aLogicPix(pWin->GetInverseViewTransformation() * basegfx::B2DVector(1.0, 1.0));

                        aRange.grow(((aLogicPix.getX() + aLogicPix.getY()) * 0.5) * (pOLV->GetInvalidateMore() + 1.0));

                        InvalidateOneWin(*pWin, aRange);
                    }

                    if (bAnchorChg)
                    {
                        pOLV->SetAnchorMode(eNewAnchor);
                    }

                    if (bColorChg)
                    {
                        pOLV->SetBackgroundColor( aNewColor );
                    }

                    pOLV->SetOutputArea(maTextEditArea); // weil sonst scheinbar nicht richtig umgeankert wird
                    ImpInvalidateOutlinerView(*pOLV);
                }

                GetTextEditOutlinerView()->ShowCursor();
            }
        }

        ImpMakeTextCursorAreaVisible();
    }
}

void SdrObjEditView::TextEditDrawing(SdrPaintWindow& rPaintWindow) const
{
    // draw old text edit stuff
    if(IsTextEdit())
    {
        const SdrOutliner* pActiveOutliner = GetTextEditOutliner();

        if(pActiveOutliner)
        {
            const sal_uInt32 nViewAnz(pActiveOutliner->GetViewCount());

            if(nViewAnz)
            {
                const Region& rRedrawRegion = rPaintWindow.GetRedrawRegion();
                const Rectangle aCheckRect(rRedrawRegion.GetBoundRect());
                const basegfx::B2DRange aCheckRange(aCheckRect.Left(), aCheckRect.Top(), aCheckRect.Right(), aCheckRect.Bottom());

                for(sal_uInt32 i(0); i < nViewAnz; i++)
                {
                    OutlinerView* pOLV = pActiveOutliner->GetView(i);

                    if(pOLV->GetWindow() == &rPaintWindow.GetOutputDevice())
                    {
                        ImpPaintOutlinerView(*pOLV, aCheckRange, rPaintWindow.GetTargetOutputDevice());
                        return;
                    }
                }
            }
        }
    }
}

void SdrObjEditView::ImpPaintOutlinerView(OutlinerView& rOutlView, const basegfx::B2DRange& rRange, OutputDevice& rTargetDevice) const
{
    const SdrTextObj* pText = dynamic_cast< const SdrTextObj* >(GetTextEditObject());
    bool bTextFrame(pText && pText->IsTextFrame());
    bool bFitToSize(0 != (mpTextEditOutliner->GetControlWord() & EE_CNTRL_STRETCHING));
    bool bModifyMerk(mpTextEditOutliner->IsModified()); // #43095#
    basegfx::B2DRange aBlankRange(rOutlView.GetOutputAreaRange());

    aBlankRange.expand(maMinTextEditArea);

    const basegfx::B2DRange aPixRange(rTargetDevice.GetViewTransformation() * aBlankRange);

    aBlankRange.intersect(rRange);

    rOutlView.GetOutliner()->SetUpdateMode(true); // Bugfix #22596#
    rOutlView.Paint(aBlankRange, &rTargetDevice);

    if(!bModifyMerk)
    {
        // #43095#
        mpTextEditOutliner->ClearModifyFlag();
    }

    if(bTextFrame && !bFitToSize)
    {
        // completely reworked to use primitives; this ensures same look and functionality
        const drawinglayer::geometry::ViewInformation2D aViewInformation2D;
        drawinglayer::processor2d::BaseProcessor2D* pProcessor = drawinglayer::processor2d::createProcessor2DFromOutputDevice(
            rTargetDevice,
            aViewInformation2D);

        if(pProcessor)
        {
            const bool bMerk(rTargetDevice.IsMapModeEnabled());
            const SvtOptionsDrawinglayer aSvtOptionsDrawinglayer;
            const Color aHilightColor(aSvtOptionsDrawinglayer.getHilightColor());
            const double fTransparence(aSvtOptionsDrawinglayer.GetTransparentSelectionPercent() * 0.01);
            const sal_uInt16 nPixSiz(rOutlView.GetInvalidateMore() - 1);
            const basegfx::B2DHomMatrix aTransformation(
                basegfx::tools::createScaleTranslateB2DHomMatrix(
                    aPixRange.getWidth(),
                    aPixRange.getHeight(),
                    aPixRange.getMinX(),
                    aPixRange.getMinY()));
            const drawinglayer::primitive2d::Primitive2DReference xReference(
                new drawinglayer::primitive2d::OverlayRectanglePrimitive(
                    aTransformation,
                    aHilightColor.getBColor(),
                    fTransparence,
                    std::max(6, nPixSiz - 2), // grow
                    0.0)); // shrink
            const drawinglayer::primitive2d::Primitive2DSequence aSequence(&xReference, 1);

            rTargetDevice.EnableMapMode(false);
            pProcessor->process(aSequence);
            rTargetDevice.EnableMapMode(bMerk);
            delete pProcessor;
        }
    }

    rOutlView.ShowCursor();
}

void SdrObjEditView::ImpInvalidateOutlinerView(OutlinerView& rOutlView) const
{
    Window* pWin = rOutlView.GetWindow();

    if(pWin)
    {
        const SdrTextObj* pText = dynamic_cast< SdrTextObj* >(GetTextEditObject());
        const bool bTextFrame(pText && pText->IsTextFrame());
        const bool bFitToSize(0 != (GetTextEditOutliner()->GetControlWord() & EE_CNTRL_STRETCHING));

        if(bTextFrame && !bFitToSize)
        {
            basegfx::B2DRange aBlankRange(rOutlView.GetOutputAreaRange());

            aBlankRange.expand(maMinTextEditArea);

            basegfx::B2DRange aPixRange(pWin->GetViewTransformation() * aBlankRange);
            aPixRange.grow(1.0);

            const sal_uInt16 nPixSiz(rOutlView.GetInvalidateMore() - 1);

            {
                // xPixRect Begrenzen, wegen Treiberproblem bei zu weit hinausragenden Pixelkoordinaten
                const basegfx::B2DVector aDiscreteScale(pWin->GetDiscreteRange().getRange());
                const double a(2.0 * nPixSiz);
                const double fMaxX(aDiscreteScale.getX() + a);
                const double fMaxY(aDiscreteScale.getY() + a);

                aPixRange = basegfx::B2DRange(
                    basegfx::fTools::less(aPixRange.getMinX(), -a) ? aPixRange.getMinX() - a : aPixRange.getMinX(),
                    basegfx::fTools::less(aPixRange.getMinY(), -a) ? aPixRange.getMinY() - a : aPixRange.getMinY(),
                    basegfx::fTools::more(aPixRange.getMaxX(), fMaxX) ? fMaxX : aPixRange.getMaxX(),
                    basegfx::fTools::more(aPixRange.getMaxY(), fMaxY) ? fMaxY : aPixRange.getMaxY());
            }

            basegfx::B2DRange aOuterPix(aPixRange);
            aOuterPix.grow(nPixSiz);

            const bool bMerk(pWin->IsMapModeEnabled());

            pWin->EnableMapMode(false);
            InvalidateOneWin(*pWin, aOuterPix);
            pWin->EnableMapMode(bMerk);
        }
    }
}

OutlinerView* SdrObjEditView::ImpMakeOutlinerView(Window* pWin, bool /*bNoPaint*/, OutlinerView* pGivenView) const
{
    // Hintergrund
    Color aBackground(GetTextEditBackgroundColor(*this));
    SdrTextObj* pText = dynamic_cast< SdrTextObj * >( mxTextEditObj.get() );
    const bool bTextFrame(pText && pText->IsTextFrame());
    const bool bContourFrame(pText && pText->IsContourTextFrame());

    // OutlinerView erzeugen
    OutlinerView* pOutlView=pGivenView;
    SdrOutliner* pTextEditOutliner = const_cast< SdrOutliner* >(GetTextEditOutliner());
    pTextEditOutliner->SetUpdateMode(false);

    if(!pOutlView)
    {
        pOutlView = new OutlinerView(pTextEditOutliner, pWin);
    }
    else
    {
        pOutlView->SetWindow(pWin);
    }

    // Scrollen verbieten
    sal_uInt32 nStat = pOutlView->GetControlWord();
    nStat&=~EV_CNTRL_AUTOSCROLL;

    // AutoViewSize nur wenn nicht KontourFrame.
    if(!bContourFrame)
    {
        nStat |= EV_CNTRL_AUTOSIZE;
    }

    if(bTextFrame)
    {
        sal_uInt16 nPixSiz = GetHdlList().GetHdlSize() * 2 + 1;
        nStat|=EV_CNTRL_INVONEMORE;
        pOutlView->SetInvalidateMore(nPixSiz);
    }

    pOutlView->SetControlWord(nStat);
    pOutlView->SetBackgroundColor( aBackground );

    if(pText)
    {
        pOutlView->SetAnchorMode((EVAnchorMode)(pText->GetOutlinerViewAnchorMode()));
        pTextEditOutliner->SetFixedCellHeight(((const SdrTextFixedCellHeightItem&)pText->GetMergedItem(SDRATTR_TEXT_USEFIXEDCELLHEIGHT)).GetValue());
    }

    pOutlView->SetOutputArea(maTextEditArea);
    pTextEditOutliner->SetUpdateMode(true);
    ImpInvalidateOutlinerView(*pOutlView);

    return pOutlView;
}

bool SdrObjEditView::IsTextEditFrame() const
{
    SdrTextObj* pText = dynamic_cast< SdrTextObj* >( mxTextEditObj.get() );

    return (pText && pText->IsTextFrame());
}

IMPL_LINK(SdrObjEditView,ImpOutlinerStatusEventHdl,EditStatus*,pEditStat)
{
    if(GetTextEditOutliner() )
    {
        SdrTextObj* pTextObj = dynamic_cast< SdrTextObj * >( mxTextEditObj.get() );

        if( pTextObj )
        {
            pTextObj->onEditOutlinerStatusEvent( pEditStat );
        }
    }

    return 0;
}

IMPL_LINK(SdrObjEditView,ImpOutlinerCalcFieldValueHdl,EditFieldInfo*,pFI)
{
    bool bOk(false);
    String& rStr=pFI->GetRepresentation();
    rStr.Erase();
    SdrTextObj* pTextObj = dynamic_cast< SdrTextObj* >( mxTextEditObj.get() );

    if(pTextObj)
    {
        Color* pTxtCol = 0;
        Color* pFldCol = 0;

        bOk = pTextObj->CalcFieldValue(pFI->GetField(), pFI->GetPara(), pFI->GetPos(), true, pTxtCol, pFldCol, rStr);

        if(bOk)
        {
            if(pTxtCol)
            {
                pFI->SetTxtColor(*pTxtCol);
                delete pTxtCol;
            }

            if(pFldCol)
            {
                pFI->SetFldColor(*pFldCol);
                delete pFldCol;
            }
            else
            {
                pFI->SetFldColor(Color(COL_LIGHTGRAY)); // kann spaeter (357) raus
            }
        }
    }

    Outliner& rDrawOutl = getSdrModelFromSdrView().GetDrawOutliner(pTextObj);
    Link aDrawOutlLink=rDrawOutl.GetCalcFieldValueHdl();

    if(!bOk && aDrawOutlLink.IsSet())
    {
        aDrawOutlLink.Call(pFI);
        bOk = (bool)rStr.Len();
    }

    if(!bOk && maOldCalcFieldValueLink.IsSet())
    {
        return maOldCalcFieldValueLink.Call(pFI);
    }

    return 0;
}

IMPL_LINK(SdrObjEditView, EndTextEditHdl, SdrUndoManager*, /*pUndoManager*/)
{
    SdrEndTextEdit();
    return 0;
}

SdrUndoManager* SdrObjEditView::getSdrUndoManagerForEnhancedTextEdit() const
{
    // default returns registered UndoManager
    return dynamic_cast< SdrUndoManager* >(getSdrModelFromSdrView().GetSdrUndoManager());
}

bool SdrObjEditView::SdrBeginTextEdit(
    SdrObject* pObj, Window* pWin,
    bool bIsNewObj, SdrOutliner* pGivenOutliner,
    OutlinerView* pGivenOutlinerView,
    bool bDontDeleteOutliner, bool bOnlyOneView,
    bool bGrabFocus)
{
    SdrEndTextEdit();

    if( dynamic_cast< SdrTextObj* >( pObj ) == 0 )
        return false; // currently only possible with text objects

    if(bGrabFocus && pWin)
    {
        // attetion, this call may cause an EndTextEdit() call to this view
        pWin->GrabFocus(); // to force the cursor into the edit view
    }

    mbTextEditDontDelete = bDontDeleteOutliner && pGivenOutliner;
    mbTextEditOnlyOneView = bOnlyOneView;
    mbTextEditNewObj = bIsNewObj;
    const sal_uInt32 nWinAnz(PaintWindowCount());
    sal_uInt32 i;
    bool bBrk(false);

    // Abbruch, wenn kein Objekt angegeben.
    if(!pObj)
    {
        bBrk = true;
    }

    if(!bBrk && !pWin)
    {
        for(i = 0; i < nWinAnz && !pWin; i++)
        {
            SdrPaintWindow* pPaintWindow = GetPaintWindow(i);

            if(OUTDEV_WINDOW == pPaintWindow->GetOutputDevice().GetOutDevType())
            {
                pWin = (Window*)(&pPaintWindow->GetOutputDevice());
            }
        }

        // Abbruch, wenn kein Window da.
        if(!pWin)
        {
            bBrk = true;
        }
    }

    if(!bBrk && !GetSdrPageView())
    {
        // Abbruch, wenn keine PageView zu dem Objekt vorhanden.
        bBrk = true;
    }

    if(pObj && GetSdrPageView())
    {
        // Kein TextEdit an Objekten im gesperrten Layer
        if(GetSdrPageView()->GetLockedLayers().IsSet(pObj->GetLayer()))
        {
            bBrk = true;
        }
    }

    if(GetTextEditOutliner())
    {
        DBG_ERROR("SdrObjEditView::SdrBeginTextEdit() da stand noch ein alter Outliner rum");
        delete mpTextEditOutliner;
        mpTextEditOutliner = 0;
    }

    if(!bBrk)
    {
        mpTextEditWin = pWin;
        mxTextEditObj.reset( pObj );
        mpTextEditOutliner = pGivenOutliner;

        if(!GetTextEditOutliner())
        {
            mpTextEditOutliner = SdrMakeOutliner(OUTLINERMODE_TEXTOBJECT, &mxTextEditObj->getSdrModelFromSdrObject());
        }

        {
            SvtAccessibilityOptions aOptions;
            GetTextEditOutliner()->ForceAutoColor(aOptions.GetIsAutomaticFontColor());
        }

        bool bEmpty(!mxTextEditObj->GetOutlinerParaObject());

        maOldCalcFieldValueLink = GetTextEditOutliner()->GetCalcFieldValueHdl();
        // Der FieldHdl muss von SdrBeginTextEdit gesetzt sein, da dor ein UpdateFields gerufen wird.
        GetTextEditOutliner()->SetCalcFieldValueHdl(LINK(this,SdrObjEditView,ImpOutlinerCalcFieldValueHdl));
        GetTextEditOutliner()->SetBeginPasteOrDropHdl(LINK(this,SdrObjEditView,BeginPasteOrDropHdl));
        GetTextEditOutliner()->SetEndPasteOrDropHdl(LINK(this,SdrObjEditView, EndPasteOrDropHdl));

        // It is just necessary to make the visualized page known. Set it.
        GetTextEditOutliner()->setVisualizedPage(GetSdrPageView() ? &GetSdrPageView()->getSdrPageFromSdrPageView() : 0);
        SdrTextObj* pTextObj = dynamic_cast< SdrTextObj* >( mxTextEditObj.get() );
        GetTextEditOutliner()->SetTextObjNoInit(pTextObj);

        if(mxTextEditObj->BegTextEdit(*GetTextEditOutliner()))
        {
            DBG_ASSERT( pTextObj, "svx::SdrObjEditView::BegTextEdit(), no text object?" );
            if( !pTextObj )
            {
                return false;
            }

            // #111096# Switch off evtl. running TextAnimation
            pTextObj->SetTextAnimationAllowed(false);

            // alten Cursor merken
            if(GetTextEditOutliner()->GetViewCount())
            {
                OutlinerView* pTmpOLV = GetTextEditOutliner()->RemoveView(sal_uLong(0));

                if(pTmpOLV && pTmpOLV != pGivenOutlinerView)
                {
                    delete pTmpOLV;
                }
            }

            // EditArea ueberTakeTextEditArea bestimmen
            // Das koennte eigentlich entfallen, da TakeTextRange() die Berechnung der maTextEditArea vornimmt
            // Die maMinTextEditArea muss jedoch wohl auch erfolgen (darum bleibt es voerst drinnen)
            pTextObj->TakeTextEditArea(0, 0, &maTextEditArea, &maMinTextEditArea);

            basegfx::B2DRange aTextRange;
            basegfx::B2DRange aAnchorRange;

            pTextObj->TakeTextRange(*GetTextEditOutliner(), aTextRange, aAnchorRange);

            if ( !pTextObj->IsContourTextFrame() )
            {
                // FitToSize erstmal nicht mit ContourFrame
                SdrFitToSizeType eFit(pTextObj->GetFitToSize());

                if(SDRTEXTFIT_PROPORTIONAL == eFit || SDRTEXTFIT_ALLLINES == eFit)
                {
                    aTextRange = aAnchorRange;
                }
            }

            maTextEditArea = aTextRange;

            // #108784#
            const basegfx::B2DHomMatrix aTextEditOffsetTransform(
                basegfx::tools::createTranslateB2DHomMatrix(
                    pTextObj->GetTextEditOffset()));

            maTextEditArea.transform(aTextEditOffsetTransform);
            maMinTextEditArea.transform(aTextEditOffsetTransform);

            mpTextEditCursorMerker = pWin->GetCursor();
            SetMoveOutside(true);
            // RecreateAllMarkHandles(); TTTT: Should be triggered by SetMoveOutside directly

            mpTextEditOutlinerView = ImpMakeOutlinerView(pWin, !bEmpty, pGivenOutlinerView);

            // check if this view is already inserted
            sal_uInt32 i2, nCount = GetTextEditOutliner()->GetViewCount();

            for( i2 = 0; i2 < nCount; i2++ )
            {
                if(GetTextEditOutliner()->GetView(i2) == GetTextEditOutlinerView())
                {
                    break;
                }
            }

            if( i2 == nCount )
            {
                GetTextEditOutliner()->InsertView(GetTextEditOutlinerView(), 0);
            }

            SetMoveOutside(false);
            SetMoveOutside(true);

            // alle Wins als OutlinerView beim Outliner anmelden
            if(!bOnlyOneView)
            {
                for(i = 0; i < nWinAnz; i++)
                {
                    SdrPaintWindow* pPaintWindow = GetPaintWindow(i);
                    OutputDevice& rOutDev = pPaintWindow->GetOutputDevice();

                    if(&rOutDev != pWin && OUTDEV_WINDOW == rOutDev.GetOutDevType())
                    {
                        OutlinerView* pOutlView = ImpMakeOutlinerView((Window*)(&rOutDev), !bEmpty, 0);
                        GetTextEditOutliner()->InsertView(pOutlView, (sal_uInt16)i);
                    }
                }
            }

            GetTextEditOutlinerView()->ShowCursor();
            GetTextEditOutliner()->SetStatusEventHdl(LINK(this, SdrObjEditView, ImpOutlinerStatusEventHdl));
#ifdef DBG_UTIL
            if(GetItemBrowser())
            {
                mpItemBrowser->SetDirty();
            }
#endif
            GetTextEditOutliner()->ClearModifyFlag();

            // #71519#, #91453#
            if(pWin)
            {
                bool bExtraInvalidate(false);

                // #71519#
                if(!bExtraInvalidate)
                {
                    SdrFitToSizeType eFit(pTextObj->GetFitToSize());

                    if(SDRTEXTFIT_PROPORTIONAL == eFit || SDRTEXTFIT_ALLLINES == eFit)
                    {
                        bExtraInvalidate = true;
                    }
                }

                if(bExtraInvalidate)
                {
                    InvalidateOneWin(*pWin, maTextEditArea);
                }
            }

            // send HINT_BEGEDIT #99840#
            const SdrObjectChangeBroadcaster aSdrObjectChangeBroadcaster(*pTextObj, HINT_BEGEDIT);
            GetTextEditOutliner()->setVisualizedPage(0);

            if( mxSelectionController.is() )
            {
                mxSelectionController->onSelectionHasChanged();
            }

            if(IsUndoEnabled() && !getSdrModelFromSdrView().GetDisableTextEditUsesCommonUndoManager())
            {
                SdrUndoManager* pSdrUndoManager = getSdrUndoManagerForEnhancedTextEdit();

                if(pSdrUndoManager)
                {
                    // we have an outliner, undo manager and it's an EditUndoManager, exchange
                    // the document undo manager and the default one from the outliner and tell
                    // it that text edit starts by setting a callback if it needs to end text edit mode.
                    if(mpOldTextEditUndoManager)
                    {
                        // should not happen, delete it since it was probably forgotten somewhere
                        OSL_ENSURE(false, "Deleting forgotten old TextEditUndoManager, should be checked (!)");
                        delete mpOldTextEditUndoManager;
                        mpOldTextEditUndoManager = 0;
                    }

                    mpOldTextEditUndoManager = mpTextEditOutliner->SetUndoManager(pSdrUndoManager);
                    pSdrUndoManager->SetEndTextEditHdl(LINK(this, SdrObjEditView, EndTextEditHdl));
                }
                else
                {
                    OSL_ENSURE(false, "The document undo manager is not derived from SdrUndoManager (!)");
                }
            }

            return sal_True; // Gut gelaufen, TextEdit laeuft nun
        }
        else
        {
            bBrk = true;

            GetTextEditOutliner()->SetCalcFieldValueHdl(maOldCalcFieldValueLink);
            GetTextEditOutliner()->SetBeginPasteOrDropHdl(Link());
            GetTextEditOutliner()->SetEndPasteOrDropHdl(Link());

        }
    }

    if(GetTextEditOutliner())
    {
        GetTextEditOutliner()->setVisualizedPage(0);
    }

    // wenn hier angekommen, dann ist irgendwas schief gelaufen
    if(!bDontDeleteOutliner)
    {
        if(pGivenOutliner)
        {
            delete pGivenOutliner;
            mpTextEditOutliner = 0;
        }

        if(pGivenOutlinerView)
        {
            delete pGivenOutlinerView;

            pGivenOutlinerView = 0;
        }
    }

    if(GetTextEditOutliner())
    {
        delete mpTextEditOutliner;
    }

    mpTextEditOutliner = 0;
    mpTextEditOutlinerView = 0;
    mxTextEditObj.reset(0);
    mpTextEditWin = 0;
    SetMoveOutside(false);

    return false;
}

SdrEndTextEditKind SdrObjEditView::SdrEndTextEdit(bool bDontDeleteReally)
{
    SdrEndTextEditKind eRet(SDRENDTEXTEDIT_UNCHANGED);
    SdrTextObj* pTEObj = dynamic_cast< SdrTextObj* >( mxTextEditObj.get() );
    Window* pTEWin = GetTextEditWin();
    SdrOutliner* pTEOutliner = GetTextEditOutliner();
    OutlinerView* pTEOutlinerView = GetTextEditOutlinerView();
    Cursor* pTECursorMerker = mpTextEditCursorMerker;
    SdrUndoManager* pUndoEditUndoManager = 0;
    bool bNeedToUndoSavedRedoTextEdit(false);

    if(IsUndoEnabled() && pTEObj && pTEOutliner && !getSdrModelFromSdrView().GetDisableTextEditUsesCommonUndoManager())
    {
        // change back the UndoManager to the remembered original one
        ::svl::IUndoManager* pOriginal = pTEOutliner->SetUndoManager(mpOldTextEditUndoManager);
        mpOldTextEditUndoManager = 0;

        if(pOriginal)
        {
            // check if we got back our document undo manager
            SdrUndoManager* pSdrUndoManager = getSdrUndoManagerForEnhancedTextEdit();

            if(pSdrUndoManager && dynamic_cast< SdrUndoManager* >(pOriginal) == pSdrUndoManager)
            {
                if(pSdrUndoManager->isEndTextEditTriggeredFromUndo())
                {
                    // remember the UndoManager where missing Undos have to be triggered after end
                    // text edit. When the undo had triggered the end text edit, the original action
                    // which had to be undone originally is not yet undone.
                    pUndoEditUndoManager = pSdrUndoManager;

                    // We are ending text edit; if text edit was triggered from undo, execute all redos
                    // to create a complete text change undo action for the redo buffer. Also mark this
                    // state when at least one redo was executed; the created extra TextChange needs to
                    // be undone in addition to the first real undo outside the text edit changes
                    while(pSdrUndoManager->GetRedoActionCount())
                    {
                        bNeedToUndoSavedRedoTextEdit = true;
                        pSdrUndoManager->Redo();
                    }
                }

                // reset the callback link and let the undo manager cleanup all text edit
                // undo actions to get the stack back to the form before the text edit
                pSdrUndoManager->SetEndTextEditHdl(Link());
            }
            else
            {
                OSL_ENSURE(false, "�Got UndoManager back in SdrEndTextEdit which is NOT the expected document UndoManager (!)");
                delete pOriginal;
            }
        }
    }

    // send HINT_ENDEDIT #99840#
    if(mxTextEditObj.is())
    {
        const SdrObjectChangeBroadcaster aSdrObjectChangeBroadcaster(*mxTextEditObj.get(), HINT_ENDEDIT);
    }

    mxTextEditObj.reset(0);
    mpTextEditWin = 0;
    mpTextEditOutliner = 0;
    mpTextEditOutlinerView = 0;
    mpTextEditCursorMerker = 0;
    maTextEditArea.reset();

    if(pTEOutliner)
    {
        bool bModified(pTEOutliner->IsModified());

        if(pTEOutlinerView)
        {
            pTEOutlinerView->HideCursor();
        }

        if(pTEObj)
        {
            pTEOutliner->CompleteOnlineSpelling();
            SdrUndoObjSetText* pTxtUndo = 0;

            if( bModified )
            {
                sal_Int32 nText;

                for( nText = 0; nText < pTEObj->getTextCount(); ++nText )
                {
                    if( pTEObj->getText( nText ) == pTEObj->getActiveText() )
                    {
                        break;
                    }
                }

                pTxtUndo = dynamic_cast< SdrUndoObjSetText* >(getSdrModelFromSdrView().GetSdrUndoFactory().CreateUndoObjectSetText(*pTEObj, nText));
            }

            DBG_ASSERT( !bModified || pTxtUndo, "svx::SdrObjEditView::EndTextEdit(), could not create undo action!" );

            // Den alten CalcFieldValue-Handler wieder setzen
            // Muss vor Obj::EndTextEdit() geschehen, da dort ein UpdateFields() gemacht wird.
            pTEOutliner->SetCalcFieldValueHdl(maOldCalcFieldValueLink);
            pTEOutliner->SetBeginPasteOrDropHdl(Link());
            pTEOutliner->SetEndPasteOrDropHdl(Link());

            const bool bUndo(IsUndoEnabled());

            if( bUndo )
            {
                XubString aObjName;
                pTEObj->TakeObjNameSingul(aObjName);
                BegUndo(ImpGetResStr(STR_UndoObjSetText),aObjName);
            }

            pTEObj->EndTextEdit(*pTEOutliner);

            // TTTT should not be needed
            // if(pTEObj && pTEObj->IsFontwork())
            // {
            //  pTEObj->ActionChanged();
            // }
            // else if(!basegfx::fTools::equalZero(pTEObj->getSdrObjectRotate()))
            // {
            //  pTEObj->ActionChanged();
            // }

            if(pTxtUndo)
            {
                pTxtUndo->AfterSetText();

                if (!pTxtUndo->IsDifferent())
                {
                    delete pTxtUndo;
                    pTxtUndo = 0;
                }
            }

            // Loeschung des gesamten TextObj checken
            SdrUndoAction* pDelUndo = 0;
            bool bDelObj(false);

            if(pTEObj && mbTextEditNewObj)
            {
                bDelObj = pTEObj->IsTextFrame() &&
                        !pTEObj->HasText() &&
                        !pTEObj->IsEmptyPresObj() &&
                        !pTEObj->HasFill() &&
                        !pTEObj->HasLine();

                if(pTEObj->IsObjectInserted() && bDelObj && SdrInventor == pTEObj->GetObjInventor() && !bDontDeleteReally)
                {
                    const SdrObjKind eIdent((SdrObjKind)pTEObj->GetObjIdentifier());

                    if(OBJ_TEXT == eIdent)
                    {
                        pDelUndo = getSdrModelFromSdrView().GetSdrUndoFactory().CreateUndoDeleteObject(*pTEObj);
                    }
                }
            }

            if(pTxtUndo)
            {
                if( bUndo )
                {
                    AddUndo(pTxtUndo);
                }

                eRet=SDRENDTEXTEDIT_CHANGED;
            }

            if(pDelUndo)
            {
                if( bUndo )
                {
                    AddUndo(pDelUndo);
                }
                else
                {
                    delete pDelUndo;
                }

                eRet=SDRENDTEXTEDIT_DELETED;
                DBG_ASSERT(pTEObj->getParentOfSdrObject(), "SdrObjEditView::SdrEndTextEdit(): Fatal: Edited Object is not inserted (!)");

                if(pTEObj->getParentOfSdrObject())
                {
                    pTEObj->getParentOfSdrObject()->RemoveObjectFromSdrObjList(pTEObj->GetNavigationPosition());
                }
            }
            else if (bDelObj)
            {
                // Fuer den Writer: Loeschen muss die App nachholen.
                eRet=SDRENDTEXTEDIT_SHOULDBEDELETED;
            }

            if( bUndo )
            {
                EndUndo(); // EndUndo hinter Remove, falls der UndoStack gleich weggehaun' wird
            }

            // #111096#
            // Switch on evtl. TextAnimation again after TextEdit
            pTEObj->SetTextAnimationAllowed(true);
            RecreateAllMarkHandles();
        }

        // alle OutlinerViews loeschen
        for(sal_uInt32 i(pTEOutliner->GetViewCount()); i > 0;)
        {
            i--;
            OutlinerView* pOLV=pTEOutliner->GetView(i);
            Window* pWin=pOLV->GetWindow();
            basegfx::B2DRange aRange(pOLV->GetOutputAreaRange());
            pTEOutliner->RemoveView(i);

            if(!mbTextEditDontDelete || i)
            {
                // die nullte gehoert mir u.U. nicht.
                delete pOLV;
            }

            aRange.expand(maTextEditArea);
            aRange.expand(maMinTextEditArea);

            const basegfx::B2DVector aLogicPixel(pWin->GetInverseViewTransformation() * basegfx::B2DVector(1.0, 1.0));
            aRange.grow((aLogicPixel.getX() + aLogicPixel.getY()) * 0.5);

            InvalidateOneWin(*pWin, aRange);
        }

        // und auch den Outliner selbst
        if(!mbTextEditDontDelete)
        {
            delete pTEOutliner;
        }
        else
        {
            pTEOutliner->Clear();
        }

        if(pTEWin)
        {
            pTEWin->SetCursor(pTECursorMerker);
        }

        SetMoveOutside(false);

#ifdef DBG_UTIL
        if(GetItemBrowser())
        {
            mpItemBrowser->SetDirty();
        }
#endif
    }

    // #108784#
    if(pTEObj)
    {
        const SdrObjectChangeBroadcaster aSdrObjectChangeBroadcaster(*pTEObj, HINT_ENDEDIT);
    }

    if(pUndoEditUndoManager)
    {
        if(bNeedToUndoSavedRedoTextEdit)
        {
            // undo the text edit action since it was created as part of an EndTextEdit
            // callback from undo itself. This needs to be done after the call to
            // FmFormView::SdrEndTextEdit since it gets created there
            pUndoEditUndoManager->Undo();
        }

        // trigger the Undo which was not executed, but lead to this
        // end text edit
        pUndoEditUndoManager->Undo();
    }

    return eRet;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// info about TextEdit. Default is false.
bool SdrObjEditView::IsTextEdit() const
{
    return mxTextEditObj.is();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

OutlinerView* SdrObjEditView::ImpFindOutlinerView(Window* pWin) const
{
    if(!pWin)
    {
        return 0;
    }

    if(!GetTextEditOutliner())
    {
        return 0;
    }

    OutlinerView* pNewView = 0;
    const sal_uInt32 nWinAnz(GetTextEditOutliner()->GetViewCount());

    for(sal_uInt32 i(0); i < nWinAnz && !pNewView; i++)
    {
        OutlinerView* pView = GetTextEditOutliner()->GetView(i);

        if(pView->GetWindow() == pWin)
        {
            pNewView = pView;
        }
    }

    return pNewView;
}

void SdrObjEditView::SetTextEditWin(Window* pWin)
{
    if(mxTextEditObj.is() && pWin && pWin != GetTextEditWin())
    {
        OutlinerView* pNewView=ImpFindOutlinerView(pWin);

        if(pNewView && pNewView != GetTextEditOutlinerView())
        {
            if(GetTextEditOutlinerView())
            {
                GetTextEditOutlinerView()->HideCursor();
            }

            mpTextEditOutlinerView = pNewView;
            mpTextEditWin = pWin;
            pWin->GrabFocus(); // Damit der Cursor hier auch blinkt
            pNewView->ShowCursor();
            ImpMakeTextCursorAreaVisible();
        }
    }
}

bool SdrObjEditView::IsTextEditHit(const basegfx::B2DPoint& rHit, double fTol) const
{
    bool bOk(false);

    if(mxTextEditObj.is())
    {
        fTol = 0.0; // Joe am 6.3.1997: Keine Hittoleranz mehr hier

        if (!bOk)
        {
            basegfx::B2DRange aEditRange;
            OutlinerView* pOLV = GetTextEditOutliner()->GetView(0);

            if(pOLV)
            {
                aEditRange.expand(pOLV->GetOutputAreaRange());
            }

            if(!basegfx::fTools::equalZero(fTol))
            {
                aEditRange.grow(fabs(fTol));
            }

            bOk = aEditRange.isInside(rHit);

            if (bOk)
            {
                // Nun noch checken, ob auch wirklich Buchstaben getroffen wurden
                const basegfx::B2DPoint aTextPos(rHit - aEditRange.getMinimum());
                long nHitTol(2000);
                OutputDevice* pRef = GetTextEditOutliner()->GetRefDevice();

                if( pRef )
                {
                    nHitTol = pRef->LogicToLogic( nHitTol, MAP_100TH_MM, pRef->GetMapMode().GetMapUnit() );
                }

                bOk = const_cast< SdrOutliner* >(GetTextEditOutliner())->IsTextPos(
                    Point(basegfx::fround(aTextPos.getX()), basegfx::fround(aTextPos.getY())),
                    (sal_uInt16)nHitTol);
            }
        }
    }

    return bOk;
}

bool SdrObjEditView::IsTextEditFrameHit(const basegfx::B2DPoint& rHit) const
{
    bool bOk(false);

    if(mxTextEditObj.is())
    {
        SdrTextObj* pText = dynamic_cast<SdrTextObj*>(mxTextEditObj.get());
        OutlinerView* pOLV = GetTextEditOutliner()->GetView(0);

        if(pOLV)
        {
            Window* pWin = pOLV->GetWindow();

            if(pText && pText->IsTextFrame() && pOLV && pWin)
            {
                const sal_uInt16 nPixSiz(pOLV->GetInvalidateMore());
                basegfx::B2DRange aEditArea(maMinTextEditArea);

                aEditArea.expand(pOLV->GetOutputAreaRange());

                if(!aEditArea.isInside(rHit))
                {
                    const basegfx::B2DVector aLogic(pWin->GetInverseViewTransformation() * basegfx::B2DVector(1.0, 1.0));
                    const double fMetric(aLogic.getLength());

                    aEditArea.grow(fMetric);

                    bOk = aEditArea.isInside(rHit);
                }
            }
        }
    }

    return bOk;
}

void SdrObjEditView::AddTextEditOfs(MouseEvent& rMEvt) const
{
    if(mxTextEditObj.is())
    {
        Point aPvOfs;
        SdrTextObj* pTextObj = dynamic_cast< SdrTextObj* >(mxTextEditObj.get());

        if(pTextObj)
        {
            // #108784#
            const basegfx::B2DPoint aTextEdOff(pTextObj->GetTextEditOffset());
            aPvOfs = Point(basegfx::fround(aTextEdOff.getX()), basegfx::fround(aTextEdOff.getY()));
        }

        Point aObjOfs(sdr::legacy::GetLogicRect(*mxTextEditObj.get()).TopLeft());
        (Point&)(rMEvt.GetPosPixel()) += aPvOfs + aObjOfs;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool SdrObjEditView::KeyInput(const KeyEvent& rKEvt, Window* pWin)
{
    if(GetTextEditOutlinerView())
    {
#ifdef DBG_UTIL
        if(KEY_RETURN == rKEvt.GetKeyCode().GetCode() && 1 == GetTextEditOutliner()->GetParagraphCount())
        {
            ByteString aLine(
                GetTextEditOutliner()->GetText(GetTextEditOutliner()->GetParagraph( 0 ), 1),
                gsl_getSystemTextEncoding());
            aLine = aLine.ToUpperAscii();

            if(aLine == "HELLO JOE, PLEASE SHOW THE ITEMBROWSER")
                ShowItemBrowser();
        }
#endif
        if(GetTextEditOutlinerView()->PostKeyEvent(rKEvt))
        {
            if(GetTextEditOutliner() && GetTextEditOutliner()->IsModified())
            {
                getSdrModelFromSdrView().SetChanged(true);
            }

            if(pWin && pWin != GetTextEditWin())
            {
                SetTextEditWin(pWin);
            }

#ifdef DBG_UTIL
            if(GetItemBrowser())
            {
                mpItemBrowser->SetDirty();
            }
#endif
            ImpMakeTextCursorAreaVisible();

            return true;
        }
    }

    return SdrGlueEditView::KeyInput(rKEvt, pWin);
}

bool SdrObjEditView::MouseButtonDown(const MouseEvent& rMEvt, Window* pWin)
{
    if(GetTextEditOutlinerView())
    {
        bool bPostIt(GetTextEditOutliner()->IsInSelectionMode());

        if(!bPostIt)
        {
            basegfx::B2DPoint aPt(rMEvt.GetPosPixel().X(), rMEvt.GetPosPixel().Y());

            if(pWin)
            {
                aPt = pWin->GetInverseViewTransformation() * aPt;
            }
            else if(GetTextEditWin())
            {
                aPt = GetTextEditWin()->GetInverseViewTransformation() * aPt;
            }

            bPostIt = IsTextEditHit(aPt, getHitTolLog());
        }

        if(bPostIt)
        {
            Point aPixPos(rMEvt.GetPosPixel());
            Rectangle aR(pWin->LogicToPixel(GetTextEditOutlinerView()->GetOutputArea()));

            if(aPixPos.X() < aR.Left())
            {
                aPixPos.X() = aR.Left();
            }

            if(aPixPos.X() > aR.Right())
            {
                aPixPos.X() = aR.Right();
            }

            if(aPixPos.Y() < aR.Top())
            {
                aPixPos.Y() = aR.Top();
            }

            if(aPixPos.Y() > aR.Bottom())
            {
                aPixPos.Y() = aR.Bottom();
            }

            MouseEvent aMEvt(aPixPos, rMEvt.GetClicks(), rMEvt.GetMode(), rMEvt.GetButtons(), rMEvt.GetModifier());

            if(GetTextEditOutlinerView()->MouseButtonDown(aMEvt))
            {
                if(pWin && pWin != GetTextEditWin())
                {
                    SetTextEditWin(pWin);
                }
#ifdef DBG_UTIL
                if(GetItemBrowser())
                {
                    mpItemBrowser->SetDirty();
                }
#endif
                ImpMakeTextCursorAreaVisible();

                return true;
            }
        }
    }

    return SdrGlueEditView::MouseButtonDown(rMEvt, pWin);
}

bool SdrObjEditView::MouseButtonUp(const MouseEvent& rMEvt, Window* pWin)
{
    if(GetTextEditOutlinerView())
    {
        bool bPostIt(GetTextEditOutliner()->IsInSelectionMode());

        if(!bPostIt)
        {
            basegfx::B2DPoint aPt(rMEvt.GetPosPixel().X(), rMEvt.GetPosPixel().Y());

            if(pWin)
            {
                aPt = pWin->GetInverseViewTransformation() * aPt;
            }
            else if(GetTextEditWin())
            {
                aPt = GetTextEditWin()->GetInverseViewTransformation() * aPt;
            }

            bPostIt = IsTextEditHit(aPt, getHitTolLog());
        }

        if(bPostIt)
        {
            Point aPixPos(rMEvt.GetPosPixel());
            Rectangle aR(pWin->LogicToPixel(GetTextEditOutlinerView()->GetOutputArea()));

            if(aPixPos.X() < aR.Left())
            {
                aPixPos.X() = aR.Left();
            }

            if(aPixPos.X() > aR.Right())
            {
                aPixPos.X() = aR.Right();
            }

            if(aPixPos.Y() < aR.Top())
            {
                aPixPos.Y() = aR.Top();
            }

            if(aPixPos.Y() > aR.Bottom())
            {
                aPixPos.Y() = aR.Bottom();
            }

            MouseEvent aMEvt(aPixPos, rMEvt.GetClicks(), rMEvt.GetMode(), rMEvt.GetButtons(), rMEvt.GetModifier());

            if(GetTextEditOutlinerView()->MouseButtonUp(aMEvt))
            {
#ifdef DBG_UTIL
                if(GetItemBrowser())
                {
                    mpItemBrowser->SetDirty();
                }
#endif
                ImpMakeTextCursorAreaVisible();
                return true;
            }
        }
    }

    return SdrGlueEditView::MouseButtonUp(rMEvt, pWin);
}

bool SdrObjEditView::MouseMove(const MouseEvent& rMEvt, Window* pWin)
{
    if(GetTextEditOutlinerView())
    {
        bool bSelMode(GetTextEditOutliner()->IsInSelectionMode());
        bool bPostIt(bSelMode);

        if(!bPostIt)
        {
            basegfx::B2DPoint aPt(rMEvt.GetPosPixel().X(), rMEvt.GetPosPixel().Y());

            if(pWin)
            {
                aPt = pWin->GetInverseViewTransformation() * aPt;
            }
            else if(GetTextEditWin())
            {
                aPt = GetTextEditWin()->GetInverseViewTransformation() * aPt;
            }

            bPostIt = IsTextEditHit(aPt, getHitTolLog());
        }

        if(bPostIt)
        {
            Point aPixPos(rMEvt.GetPosPixel());
            Rectangle aR(pWin->LogicToPixel(GetTextEditOutlinerView()->GetOutputArea()));

            if(aPixPos.X() < aR.Left())
            {
                aPixPos.X() = aR.Left();
            }

            if(aPixPos.X() > aR.Right())
            {
                aPixPos.X() = aR.Right();
            }

            if(aPixPos.Y() < aR.Top())
            {
                aPixPos.Y() = aR.Top();
            }

            if(aPixPos.Y() > aR.Bottom())
            {
                aPixPos.Y() = aR.Bottom();
            }

            MouseEvent aMEvt(aPixPos, rMEvt.GetClicks(), rMEvt.GetMode(), rMEvt.GetButtons(), rMEvt.GetModifier());

            if(GetTextEditOutlinerView()->MouseMove(aMEvt) && bSelMode)
            {
#ifdef DBG_UTIL
                if(GetItemBrowser())
                {
                    mpItemBrowser->SetDirty();
                }
#endif
                ImpMakeTextCursorAreaVisible();

                return true;
            }
        }
    }

    return SdrGlueEditView::MouseMove(rMEvt, pWin);
}

bool SdrObjEditView::Command(const CommandEvent& rCEvt, Window* pWin)
{
    // solange bis die OutlinerView einen bool zurueckliefert
    // bekommt sie nur COMMAND_STARTDRAG
    if(GetTextEditOutlinerView())
    {
        if(COMMAND_STARTDRAG == rCEvt.GetCommand())
        {
            bool bPostIt(GetTextEditOutliner()->IsInSelectionMode() || !rCEvt.IsMouseEvent());

            if(!bPostIt && rCEvt.IsMouseEvent())
            {
                basegfx::B2DPoint aPt(rCEvt.GetMousePosPixel().X(), rCEvt.GetMousePosPixel().Y());

                if(pWin)
                {
                    aPt = pWin->GetInverseViewTransformation() * aPt;
                }
                else if(GetTextEditWin())
                {
                    aPt = GetTextEditWin()->GetInverseViewTransformation() * aPt;
                }

                bPostIt = IsTextEditHit(aPt, getHitTolLog());
            }

            if(bPostIt)
            {
                Point aPixPos(rCEvt.GetMousePosPixel());

                if(rCEvt.IsMouseEvent())
                {
                    Rectangle aR(pWin->LogicToPixel(GetTextEditOutlinerView()->GetOutputArea()));

                    if(aPixPos.X() < aR.Left())
                    {
                        aPixPos.X() = aR.Left();
                    }

                    if(aPixPos.X() > aR.Right())
                    {
                        aPixPos.X() = aR.Right();
                    }

                    if(aPixPos.Y() < aR.Top())
                    {
                        aPixPos.Y() = aR.Top();
                    }

                    if(aPixPos.Y() > aR.Bottom())
                    {
                        aPixPos.Y() = aR.Bottom();
                    }
                }

                CommandEvent aCEvt(aPixPos,rCEvt.GetCommand(),rCEvt.IsMouseEvent());

                // Command ist an der OutlinerView leider void
                GetTextEditOutlinerView()->Command(aCEvt);

                if(pWin && pWin != GetTextEditWin())
                {
                    SetTextEditWin(pWin);
                }
#ifdef DBG_UTIL
                if(GetItemBrowser())
                {
                    mpItemBrowser->SetDirty();
                }
#endif
                ImpMakeTextCursorAreaVisible();
                return true;
            }
        }
        else // if (rCEvt.GetCommand() == COMMAND_VOICE )
        {
            GetTextEditOutlinerView()->Command(rCEvt);

            return true;
        }
    }

    return SdrGlueEditView::Command(rCEvt,pWin);
}

bool SdrObjEditView::Cut(sal_uInt32 nFormat)
{
    if(GetTextEditOutliner())
    {
        GetTextEditOutlinerView()->Cut();
#ifdef DBG_UTIL
        if(GetItemBrowser())
        {
            mpItemBrowser->SetDirty();
        }
#endif
        ImpMakeTextCursorAreaVisible();

        return true;
    }
    else
    {
        return SdrGlueEditView::Cut(nFormat);
    }
}

bool SdrObjEditView::Yank(sal_uInt32 nFormat)
{
    if(GetTextEditOutliner())
    {
        GetTextEditOutlinerView()->Copy();
        return true;
    }
    else
    {
        return SdrGlueEditView::Yank(nFormat);
    }
}

bool SdrObjEditView::Paste(Window* pWin, sal_uInt32 nFormat)
{
    if(GetTextEditOutliner())
    {
        if(pWin)
        {
            OutlinerView* pNewView=ImpFindOutlinerView(pWin);

            if(pNewView)
            {
                pNewView->Paste();
            }
        }
        else
        {
            GetTextEditOutlinerView()->Paste();
        }
#ifdef DBG_UTIL
        if(GetItemBrowser())
        {
            mpItemBrowser->SetDirty();
        }
#endif
        ImpMakeTextCursorAreaVisible();

        return true;
    }
    else
    {
        return SdrGlueEditView::Paste(pWin,nFormat);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool SdrObjEditView::ImpIsTextEditAllSelected() const
{
    bool bRet(false);

    if(GetTextEditOutliner() && GetTextEditOutlinerView())
    {
        if(SdrTextObj::HasTextImpl(GetTextEditOutliner()))
        {
            const sal_uInt32 nParaAnz(GetTextEditOutliner()->GetParagraphCount());
            Paragraph* pLastPara = GetTextEditOutliner()->GetParagraph(nParaAnz > 1 ? nParaAnz - 1 : 0);
            ESelection aESel(const_cast< OutlinerView* >(GetTextEditOutlinerView())->GetSelection());

            if(0 == aESel.nStartPara && 0 == aESel.nStartPos && aESel.nEndPara == sal_uInt16(nParaAnz - 1))
            {
                XubString aStr(GetTextEditOutliner()->GetText(pLastPara));

                if(aStr.Len() == aESel.nEndPos)
                {
                    bRet = true;
                }
            }

            // und nun auch noch fuer den Fall, das rueckwaerts selektiert wurde
            if(!bRet && 0 == aESel.nEndPara && 0 == aESel.nEndPos && aESel.nStartPara == sal_uInt16(nParaAnz - 1))
            {
                XubString aStr(GetTextEditOutliner()->GetText(pLastPara));

                if(aStr.Len() == aESel.nStartPos)
                {
                    bRet = true;
                }
            }
        }
        else
        {
            bRet = true;
        }
    }

    return bRet;
}

void SdrObjEditView::ImpMakeTextCursorAreaVisible()
{
    if(GetTextEditOutlinerView() && GetTextEditWin())
    {
        const Cursor* pCsr = GetTextEditWin()->GetCursor();

        if(pCsr)
        {
            const Size aSiz(pCsr->GetSize());

            if(aSiz.Width() && aSiz.Height())
            {
                // #38450#
                basegfx::B2DRange aRange(basegfx::B2DTuple(pCsr->GetPos().X(), pCsr->GetPos().Y()));

                aRange.expand(aRange.getMinimum() + basegfx::B2DPoint(aSiz.getWidth(), aSiz.getHeight()));
                MakeVisibleAtView(aRange, *GetTextEditWin());
            }
        }
    }
}

sal_uInt16 SdrObjEditView::GetScriptType() const
{
    sal_uInt16 nScriptType(0);

    if( IsTextEdit() )
    {
        if( mxTextEditObj->GetOutlinerParaObject() )
        {
            nScriptType = mxTextEditObj->GetOutlinerParaObject()->GetTextObject().GetScriptType();
        }

        if(GetTextEditOutlinerView())
        {
            nScriptType = GetTextEditOutlinerView()->GetSelectedScriptType();
        }
    }
    else if(areSdrObjectsSelected())
    {
        const SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());

        for(sal_uInt32 i(0); i < aSelection.size(); i++)
        {
            OutlinerParaObject* pParaObj = aSelection[i]->GetOutlinerParaObject();

            if( pParaObj )
            {
                nScriptType |= pParaObj->GetTextObject().GetScriptType();
            }
        }
    }

    if(!nScriptType)
    {
        nScriptType = SCRIPTTYPE_LATIN;
    }

    return nScriptType;
}

bool SdrObjEditView::GetAttributes(SfxItemSet& rTargetSet, bool bOnlyHardAttr) const
{
    if( mxSelectionController.is() )
    {
        if( mxSelectionController->GetAttributes( rTargetSet, bOnlyHardAttr ) )
        {
            return true;
        }
    }

    if(IsTextEdit())
    {
        DBG_ASSERT(GetTextEditOutlinerView()!=NULL,"SdrObjEditView::GetAttributes(): GetTextEditOutlinerView()=NULL");
        DBG_ASSERT(GetTextEditOutliner()!=NULL,"SdrObjEditView::GetAttributes(): GetTextEditOutliner()=NULL");

        // #92389# take care of bOnlyHardAttr(!)
        if(!bOnlyHardAttr && mxTextEditObj->GetStyleSheet())
        {
            rTargetSet.Put(mxTextEditObj->GetStyleSheet()->GetItemSet());
        }

        // add object attributes
        rTargetSet.Put( mxTextEditObj->GetMergedItemSet() );

        if( mxTextEditObj->GetOutlinerParaObject() )
        {
            rTargetSet.Put( SvxScriptTypeItem( mxTextEditObj->GetOutlinerParaObject()->GetTextObject().GetScriptType() ) );
        }

        if(GetTextEditOutlinerView())
        {
            // false= InvalidItems nicht al Default, sondern als "Loecher" betrachten
            rTargetSet.Put(const_cast< OutlinerView* >(GetTextEditOutlinerView())->GetAttribs(), false);
            rTargetSet.Put( SvxScriptTypeItem( GetTextEditOutlinerView()->GetSelectedScriptType() ), false );
        }

        const SdrObject* pSingleSelected = getSelectedIfSingle();

        if(pSingleSelected && pSingleSelected == mxTextEditObj.get())
        {
            MergeNotPersistAttrFromMarked(rTargetSet);
        }

        return true;
    }
    else
    {
        return SdrGlueEditView::GetAttributes(rTargetSet, bOnlyHardAttr);
    }
}

bool SdrObjEditView::SetAttributes(const SfxItemSet& rSet, bool bReplaceAll)
{
    bool bRet(false);
    const bool bTextEdit(GetTextEditOutlinerView() && mxTextEditObj.is());
    bool bAllTextSelected(ImpIsTextEditAllSelected());
    SfxItemSet* pModifiedSet = 0;
    const SfxItemSet* pSet=&rSet;

    if (!bTextEdit)
    {
        // Kein TextEdit aktiv -> alle Items ans Zeichenobjekt
        if( mxSelectionController.is() )
        {
            bRet=mxSelectionController->SetAttributes(*pSet,bReplaceAll );
        }

        if( !bRet )
        {
            bRet=SdrGlueEditView::SetAttributes(*pSet,bReplaceAll);
        }
    }
    else
    {
#ifdef DBG_UTIL
        {
            bool bHasEEFeatureItems(false);
            SfxItemIter aIter(rSet);
            const SfxPoolItem* pItem=aIter.FirstItem();

            while(!bHasEEFeatureItems && pItem)
            {
                if (!IsInvalidItem(pItem))
                {
                    sal_uInt16 nW(pItem->Which());

                    if (nW>=EE_FEATURE_START && nW<=EE_FEATURE_END)
                    {
                        bHasEEFeatureItems = true;
                    }
                }

                pItem=aIter.NextItem();
            }

            if(bHasEEFeatureItems)
            {
                String aMessage;
                aMessage.AppendAscii("SdrObjEditView::SetAttributes(): Das setzen von EE_FEATURE-Items an der SdrView macht keinen Sinn! Es fuehrt nur zu Overhead und nicht mehr lesbaren Dokumenten.");
                InfoBox(NULL, aMessage).Execute();
            }
        }
#endif

        bool bOnlyEEItems;
        bool bNoEEItems(!SearchOutlinerItems(*pSet, bReplaceAll, &bOnlyEEItems));

        // alles selektiert? -> Attrs auch an den Rahmen
        // und falls keine EEItems, dann Attrs nur an den Rahmen
        if (bAllTextSelected || bNoEEItems)
        {
            if( mxSelectionController.is() )
            {
                bRet=mxSelectionController->SetAttributes(*pSet,bReplaceAll );
            }

            if( !bRet )
            {
                const bool bUndo(IsUndoEnabled());

                if( bUndo )
                {
                    String aStr;
                    TakeMarkedDescriptionString(STR_EditSetAttributes,aStr);
                    BegUndo(aStr);
                    AddUndo(getSdrModelFromSdrView().GetSdrUndoFactory().CreateUndoGeoObject(*mxTextEditObj.get()));

                    // #i43537#
                    // If this is a text object also rescue the OutlinerParaObject since
                    // applying attributes to the object may change text layout when
                    // multiple portions exist with multiple formats. If a OutlinerParaObject
                    // really exists and needs to be rescued is evaluated in the undo
                    // implementation itself.
                    bool bRescueText = dynamic_cast< SdrTextObj* >(mxTextEditObj.get());

                    AddUndo(getSdrModelFromSdrView().GetSdrUndoFactory().CreateUndoAttrObject(*mxTextEditObj.get(), false, !bNoEEItems || bRescueText));
                    EndUndo();
                }

                mxTextEditObj->SetMergedItemSetAndBroadcast(*pSet, bReplaceAll);

                ForceLazyReactOnObjectChanges(); // Damit ModeHasChanged sofort kommt
                bRet = true;
            }
        }
        else if (!bOnlyEEItems)
        {
            // sonst Set ggf. splitten
            // Es wird nun ein ItemSet aSet gemacht, in den die EE_Items von
            // *pSet nicht enhalten ist (ansonsten ist es eine Kopie).
            sal_uInt16* pNewWhichTable=RemoveWhichRange(pSet->GetRanges(),EE_ITEMS_START,EE_ITEMS_END);
            SfxItemSet aSet(getSdrModelFromSdrView().GetItemPool(), pNewWhichTable);
            delete[] pNewWhichTable;
            SfxWhichIter aIter(aSet);
            sal_uInt16 nWhich=aIter.FirstWhich();

            while(nWhich)
            {
                const SfxPoolItem* pItem;
                SfxItemState eState = pSet->GetItemState(nWhich, false, &pItem);

                if(SFX_ITEM_SET == eState)
                {
                    aSet.Put(*pItem);
                }

                nWhich = aIter.NextWhich();
            }

            if( mxSelectionController.is() )
            {
                bRet=mxSelectionController->SetAttributes(aSet,bReplaceAll );
            }

            if( !bRet )
            {
                if( IsUndoEnabled() )
                {
                    String aStr;
                    TakeMarkedDescriptionString(STR_EditSetAttributes,aStr);
                    BegUndo(aStr);
                    AddUndo(getSdrModelFromSdrView().GetSdrUndoFactory().CreateUndoGeoObject(*mxTextEditObj.get()));
                    AddUndo(getSdrModelFromSdrView().GetSdrUndoFactory().CreateUndoAttrObject(*mxTextEditObj.get(), false, false));
                    EndUndo();
                }

                mxTextEditObj->SetMergedItemSetAndBroadcast(aSet, bReplaceAll);
                const SdrObject* pSingleSelected = getSelectedIfSingle();

                if(pSingleSelected && pSingleSelected == mxTextEditObj.get())
                {
                    SetNotPersistAttrToMarked(aSet,bReplaceAll);
                }
            }

            ForceLazyReactOnObjectChanges();
            bRet = true;
        }

        if(!bNoEEItems)
        {
            // und nun die Attribute auch noch an die EditEngine
            if(bReplaceAll)
            {
                // Am Outliner kann man leider nur alle Attribute platthauen
                GetTextEditOutlinerView()->RemoveAttribs(true);
            }

            GetTextEditOutlinerView()->SetAttribs(rSet);

#ifdef DBG_UTIL
            if(GetItemBrowser())
            {
                mpItemBrowser->SetDirty();
            }
#endif

            ImpMakeTextCursorAreaVisible();
        }

        bRet = true;
    }

    if(pModifiedSet)
    {
        delete pModifiedSet;
    }

    return bRet;
}

SfxStyleSheet* SdrObjEditView::GetStyleSheet() const
{
    SfxStyleSheet* pSheet = 0;

    if( mxSelectionController.is() )
    {
        if( mxSelectionController->GetStyleSheet( pSheet ) )
        {
            return pSheet;
        }
    }

    if(GetTextEditOutlinerView())
    {
        pSheet = GetTextEditOutlinerView()->GetStyleSheet();
    }
    else
    {
        pSheet = SdrGlueEditView::GetStyleSheet();
    }

    return pSheet;
}

bool SdrObjEditView::SetStyleSheet(SfxStyleSheet* pStyleSheet, bool bDontRemoveHardAttr)
{
    if( mxSelectionController.is() )
    {
        if( mxSelectionController->SetStyleSheet( pStyleSheet, bDontRemoveHardAttr ) )
        {
            return true;
        }
    }

    // if we are currently in edit mode we must also set the stylesheet
    // on all paragraphs in the Outliner for the edit view
    // #92191#
    if(GetTextEditOutlinerView())
    {
        Outliner* pOutliner = GetTextEditOutlinerView()->GetOutliner();
        const sal_uInt32 nParaCount(pOutliner->GetParagraphCount());
        sal_uInt32 nPara;

        for( nPara = 0; nPara < nParaCount; nPara++ )
        {
            pOutliner->SetStyleSheet( nPara, pStyleSheet );
        }
    }

    return SdrGlueEditView::SetStyleSheet(pStyleSheet,bDontRemoveHardAttr);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrObjEditView::AddWindowToPaintView(OutputDevice* pNewWin)
{
    SdrGlueEditView::AddWindowToPaintView(pNewWin);

    if(mxTextEditObj.is() && !mbTextEditOnlyOneView && OUTDEV_WINDOW == pNewWin->GetOutDevType())
    {
        OutlinerView* pOutlView = ImpMakeOutlinerView((Window*)pNewWin, false, 0);
        GetTextEditOutliner()->InsertView(pOutlView);
    }
}

void SdrObjEditView::DeleteWindowFromPaintView(OutputDevice* pOldWin)
{
    SdrGlueEditView::DeleteWindowFromPaintView(pOldWin);

    if(mxTextEditObj.is() && !mbTextEditOnlyOneView && OUTDEV_WINDOW == pOldWin->GetOutDevType())
    {
        for(sal_uInt32 i(GetTextEditOutliner()->GetViewCount()); i > 0;)
        {
            i--;
            OutlinerView* pOLV = GetTextEditOutliner()->GetView(i);

            if(pOLV && pOLV->GetWindow() == (Window*)pOldWin)
            {
                delete GetTextEditOutliner()->RemoveView(i);
            }
        }
    }
}

bool SdrObjEditView::IsTextEditInSelectionMode() const
{
    return GetTextEditOutliner() && GetTextEditOutliner()->IsInSelectionMode();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool SdrObjEditView::BegMacroObj(const basegfx::B2DPoint& rPnt, double fTol, SdrObject* pObj, Window* pWin)
{
    bool bRet(false);
    BrkMacroObj();

    if(pObj && pWin && pObj->HasMacro())
    {
        mpMacroObj = pObj;
        mpMacroWin = pWin;
        mbMacroDown = false;
        mnMacroTol = sal_uInt16(fTol);
        maMacroDownPos = rPnt;

        MovMacroObj(rPnt);
    }

    return bRet;
}

void SdrObjEditView::ImpMacroUp(const basegfx::B2DPoint& rUpPos)
{
    if(mpMacroObj && mbMacroDown)
    {
        SdrObjMacroHitRec aHitRec;

        aHitRec.maPos = rUpPos;
        aHitRec.maDownPos = maMacroDownPos;
        aHitRec.mfTol = mnMacroTol;
        aHitRec.mpSdrView = getAsSdrView();
        aHitRec.mpOut = mpMacroWin;

        mpMacroObj->PaintMacro(*mpMacroWin, aHitRec);
        mbMacroDown = false;
    }
}

void SdrObjEditView::ImpMacroDown(const basegfx::B2DPoint& rDownPos)
{
    if(mpMacroObj && !mbMacroDown)
    {
        SdrObjMacroHitRec aHitRec;

        aHitRec.maPos = rDownPos;
        aHitRec.maDownPos = maMacroDownPos;
        aHitRec.mfTol = mnMacroTol;
        aHitRec.mpSdrView = getAsSdrView();
        aHitRec.mbDown = true;
        aHitRec.mpOut = mpMacroWin;

        mpMacroObj->PaintMacro(*mpMacroWin, aHitRec);
        mbMacroDown = true;
    }
}

void SdrObjEditView::MovMacroObj(const basegfx::B2DPoint& rPnt)
{
    if(mpMacroObj)
{
        SdrObjMacroHitRec aHitRec;

        aHitRec.maPos = rPnt;
        aHitRec.maDownPos = maMacroDownPos;
        aHitRec.mfTol = mnMacroTol;
        aHitRec.mpSdrView = getAsSdrView();
        aHitRec.mbDown = mbMacroDown;
        aHitRec.mpOut = mpMacroWin;

        bool bDown(mpMacroObj->IsMacroHit(aHitRec));

        if(bDown)
        {
            ImpMacroDown(rPnt);
        }
        else
        {
            ImpMacroUp(rPnt);
        }
    }
}

void SdrObjEditView::BrkMacroObj()
{
    if(mpMacroObj)
    {
        ImpMacroUp(maMacroDownPos);

        mpMacroObj = 0;
        mpMacroWin = 0;
    }
}

bool SdrObjEditView::EndMacroObj()
{
    if(mpMacroObj && mbMacroDown)
    {
        ImpMacroUp(maMacroDownPos);
        SdrObjMacroHitRec aHitRec;

        aHitRec.maPos = maMacroDownPos;
        aHitRec.maDownPos = maMacroDownPos;
        aHitRec.mfTol = mnMacroTol;
        aHitRec.mpSdrView = getAsSdrView();
        aHitRec.mbDown = true;
        aHitRec.mpOut = mpMacroWin;

        bool bRet(mpMacroObj->DoMacro(aHitRec));

        mpMacroObj = 0;
        mpMacroWin = 0;

        return bRet;
    }
    else
    {
        BrkMacroObj();

        return false;
    }
}

/** fills the given any with a XTextCursor for the current text selection.
    Leaves the any untouched if there currently is no text selected */
void SdrObjEditView::getTextSelection( ::com::sun::star::uno::Any& rSelection )
{
    if( IsTextEdit() )
    {
        OutlinerView* pOutlinerView = GetTextEditOutlinerView();

        if( pOutlinerView && pOutlinerView->HasSelection() )
        {
            SdrObject* pObj = GetTextEditObject();

            if( pObj )
            {
                ::com::sun::star::uno::Reference< ::com::sun::star::text::XText > xText( pObj->getUnoShape(), ::com::sun::star::uno::UNO_QUERY );

                if( xText.is() )
                {
                    SvxUnoTextBase* pRange = SvxUnoTextBase::getImplementation( xText );

                    if( pRange )
                    {
                        rSelection <<= pRange->createTextCursorBySelection( pOutlinerView->GetSelection() );
                    }
                }
            }
        }
    }
}

namespace sdr { namespace table {
extern rtl::Reference< sdr::SelectionController > CreateTableController( SdrObjEditView* pView, const SdrObject* pObj, const rtl::Reference< sdr::SelectionController >& xRefController );
} }

/* check if we have a single selection and that single object likes
    to handle the mouse and keyboard events itself

    @todo: the selection controller should be queried from the
    object specific view contact. Currently this method only
    works for tables.
*/
void SdrObjEditView::handleSelectionChange()
{
    // call parent
    SdrGlueEditView::handleSelectionChange();

    // local reactions
    if( mxSelectionController.is() )
    {
        mxLastSelectionController = mxSelectionController;
        mxSelectionController->onSelectionHasChanged();
    }

    mxSelectionController.clear();

    const sdr::table::SdrTableObj* pSdrTableObj = dynamic_cast< const sdr::table::SdrTableObj* >(getSelectedIfSingle());

    if(pSdrTableObj)
    {
        mxSelectionController = sdr::table::CreateTableController(this, pSdrTableObj, mxLastSelectionController);

        if( mxSelectionController.is() )
        {
            mxLastSelectionController.clear();
            mxSelectionController->onSelectionHasChanged();
        }
    }
}

IMPL_LINK( SdrObjEditView, EndPasteOrDropHdl, PasteOrDropInfos*, pInfos )
{
    OnEndPasteOrDrop( pInfos );
    return 0;
}

IMPL_LINK( SdrObjEditView, BeginPasteOrDropHdl, PasteOrDropInfos*, pInfos )
{
    OnBeginPasteOrDrop( pInfos );
    return 0;
}

void SdrObjEditView::OnBeginPasteOrDrop( PasteOrDropInfos* )
{
    // applications can derive from these virtual methods to do something before a drop or paste operation
}

void SdrObjEditView::OnEndPasteOrDrop( PasteOrDropInfos* )
{
    // applications can derive from these virtual methods to do something before a drop or paste operation
}

sal_uInt16 SdrObjEditView::GetSelectionLevel() const
{
    sal_uInt16 nLevel = 0xFFFF;
    if( IsTextEdit() )
    {
        OSL_ENSURE(GetTextEditOutlinerView()," SdrObjEditView::GetSelectionLevel(): no TextEditOutlinerView (!)");
        OSL_ENSURE(GetTextEditOutliner(), "SdrObjEditView::GetAttributes(): no TextEditOutliner (!)");

        if( GetTextEditOutlinerView() )
        {
            //start and end position
            const ESelection aSelect(GetTextEditOutlinerView()->GetSelection());
            sal_uInt16 nStartPara = ::std::min( aSelect.nStartPara, aSelect.nEndPara );
            sal_uInt16 nEndPara = ::std::max( aSelect.nStartPara, aSelect.nEndPara );

            //get level from each paragraph
            nLevel = 0;

            for( sal_uInt16 nPara = nStartPara; nPara <= nEndPara; nPara++ )
            {
                sal_uInt16 nParaDepth = 1 << GetTextEditOutliner()->GetDepth( nPara );
                if( !(nLevel & nParaDepth) )
                    nLevel += nParaDepth;
            }

            //reduce one level for Outliner Object
            //if( nLevel > 0 && GetTextEditObject()->GetObjIdentifier() == OBJ_OUTLINETEXT )
            //  nLevel = nLevel >> 1;

            //no bullet paragraph selected
            if( nLevel == 0)
                nLevel = 0xFFFF;
        }
    }

    return nLevel;
}

bool SdrObjEditView::SupportsFormatPaintbrush(const SdrObject& rSdrObject) const
{
    if(SdrInventor == rSdrObject.GetObjInventor())
    {
        switch(rSdrObject.GetObjIdentifier())
        {
            case OBJ_RECT:
            case OBJ_CIRC:
            case OBJ_POLY:
            case OBJ_TEXT:
            case OBJ_TITLETEXT:
            case OBJ_OUTLINETEXT:
            case OBJ_GRAF:
            case OBJ_OLE2:
            case OBJ_TABLE:
            case OBJ_CUSTOMSHAPE:
                return true;
                break;
            default:
                break;
        }
    }

    return false;
}

static const sal_uInt16* GetFormatRangeImpl( bool bTextOnly )
{
    static const sal_uInt16 gRanges[] = {
        SDRATTR_SHADOW_FIRST, SDRATTR_SHADOW_LAST,
        SDRATTR_GRAF_FIRST, SDRATTR_GRAF_LAST,
        SDRATTR_TABLE_FIRST, SDRATTR_TABLE_LAST,
        XATTR_LINE_FIRST, XATTR_LINE_LAST,
        XATTR_FILL_FIRST, XATTRSET_FILL,
        EE_PARA_START, EE_PARA_END,
        EE_CHAR_START, EE_CHAR_END,
        0,0
    };
    return &gRanges[ bTextOnly ? 10 : 0];
}

bool SdrObjEditView::TakeFormatPaintBrush( boost::shared_ptr< SfxItemSet >& rFormatSet  )
{
    if( mxSelectionController.is() && mxSelectionController->TakeFormatPaintBrush(rFormatSet) )
        return true;

    if( getSelectedIfSingle() )
    {
        OutlinerView* pOLV = GetTextEditOutlinerView();

        rFormatSet.reset( new SfxItemSet( getSdrModelFromSdrView().GetItemPool(), GetFormatRangeImpl( pOLV != NULL ) ) );
        if( pOLV )
        {
            rFormatSet->Put( pOLV->GetAttribs() );
        }
        else
        {
            const bool bOnlyHardAttr = false;
            rFormatSet->Put( GetAttrFromMarked(bOnlyHardAttr) );
        }
        return true;
    }

    return false;
}

static SfxItemSet CreatePaintSet( const sal_uInt16 *pRanges, SfxItemPool& rPool, const SfxItemSet& rSourceSet, const SfxItemSet& rTargetSet, bool bNoCharacterFormats, bool bNoParagraphFormats )
{
    SfxItemSet aPaintSet( rPool, pRanges );

    while( *pRanges )
    {
        sal_uInt16 nWhich = *pRanges++;
        const sal_uInt16 nLastWhich = *pRanges++;

        if( bNoCharacterFormats && (nWhich == EE_CHAR_START) )
            continue;

        if( bNoParagraphFormats && (nWhich == EE_PARA_START ) )
            continue;

        for( ; nWhich < nLastWhich; nWhich++ )
        {
            const SfxPoolItem* pSourceItem = rSourceSet.GetItem( nWhich );
            const SfxPoolItem* pTargetItem = rTargetSet.GetItem( nWhich );

            if( (pSourceItem && !pTargetItem) || (pSourceItem && pTargetItem && !((*pSourceItem) == (*pTargetItem)) ) )
            {
                aPaintSet.Put( *pSourceItem );
            }
        }
    }
    return aPaintSet;
}

void SdrObjEditView::ApplyFormatPaintBrushToText( SfxItemSet& rFormatSet, SdrTextObj& rTextObj, SdrText* pText, bool bNoCharacterFormats, bool bNoParagraphFormats )
{
    OutlinerParaObject* pParaObj = pText ? pText->GetOutlinerParaObject() : 0;
    if(pParaObj)
    {
        SdrOutliner& rOutliner = rTextObj.ImpGetDrawOutliner();
        rOutliner.SetText(*pParaObj);

        sal_uInt32 nParaCount(rOutliner.GetParagraphCount());

        if(nParaCount)
        {
            for(sal_uInt16 nPara = 0; nPara < nParaCount; nPara++)
            {
                if( !bNoCharacterFormats )
                    rOutliner.QuickRemoveCharAttribs( nPara, /* remove all */0 );

                SfxItemSet aSet(rOutliner.GetParaAttribs(nPara));
                aSet.Put(CreatePaintSet( GetFormatRangeImpl(true), *aSet.GetPool(), rFormatSet, aSet, bNoCharacterFormats, bNoParagraphFormats ) );
                rOutliner.SetParaAttribs(nPara, aSet);
            }

            OutlinerParaObject* pTemp = rOutliner.CreateParaObject(0, (sal_uInt16)nParaCount);
            rOutliner.Clear();

            rTextObj.SetOutlinerParaObjectForText(pTemp,pText);
        }
    }
}

void SdrObjEditView::ApplyFormatPaintBrush( SfxItemSet& rFormatSet, bool bNoCharacterFormats, bool bNoParagraphFormats )
{
    SdrObject* pObj = getSelectedIfSingle();

    if( pObj && (!mxSelectionController.is() || !mxSelectionController->ApplyFormatPaintBrush( rFormatSet, bNoCharacterFormats, bNoParagraphFormats )))
    {
        OutlinerView* pOLV = GetTextEditOutlinerView();
        const SfxItemSet& rShapeSet = pObj->GetMergedItemSet();

        if( !pOLV )
        {
            // if not in text edit mode (aka the user selected text or clicked on a word)
            // apply formating attributes to selected shape
            // All formating items (see ranges above) that are unequal in selected shape and
            // the format paintbrush are hard set on the selected shape.

            const sal_uInt16* pRanges = rFormatSet.GetRanges();
            bool bTextOnly = true;

            while( *pRanges )
            {
                if( (*pRanges != EE_PARA_START) && (*pRanges != EE_CHAR_START) )
                {
                    bTextOnly = false;
                    break;
                }
                pRanges += 2;
            }

            if( !bTextOnly )
            {
                SfxItemSet aPaintSet( CreatePaintSet( GetFormatRangeImpl(false), *rShapeSet.GetPool(), rFormatSet, rShapeSet, bNoCharacterFormats, bNoParagraphFormats ) );
                const bool bReplaceAll = false;
                SetAttrToMarked(aPaintSet, bReplaceAll);
            }

            // now apply character and paragraph formating to text, if the shape has any
            SdrTextObj* pTextObj = dynamic_cast<SdrTextObj*>(pObj);
            if( pTextObj )
            {
                sal_Int32 nText = pTextObj->getTextCount();

                while( --nText >= 0 )
                {
                    SdrText* pText = pTextObj->getText( nText );
                    ApplyFormatPaintBrushToText( rFormatSet, *pTextObj, pText, bNoCharacterFormats, bNoParagraphFormats );
                }
            }
        }
        else
        {
            ::Outliner* pOutliner = pOLV->GetOutliner();
            if( pOutliner )
            {
                const EditEngine& rEditEngine = pOutliner->GetEditEngine();

                ESelection aSel( pOLV->GetSelection() );
                if( !aSel.HasRange() )
                    pOLV->SetSelection( rEditEngine.GetWord( aSel, com::sun::star::i18n::WordType::DICTIONARY_WORD ) );

                const bool bRemoveParaAttribs = !bNoParagraphFormats;
                pOLV->RemoveAttribsKeepLanguages( bRemoveParaAttribs );
                SfxItemSet aSet( pOLV->GetAttribs() );
                SfxItemSet aPaintSet( CreatePaintSet(GetFormatRangeImpl(true), *aSet.GetPool(), rFormatSet, aSet, bNoCharacterFormats, bNoParagraphFormats ) );
                pOLV->SetAttribs( aPaintSet );
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// eof
