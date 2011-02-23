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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"
#include <com/sun/star/embed/NoVisualAreaSizeException.hpp>
#include <com/sun/star/linguistic2/XSpellChecker1.hpp>

#include "View.hxx"
#include <editeng/unolingu.hxx>
#include <sfx2/request.hxx>
#include <svx/obj3d.hxx>
#include <svx/fmview.hxx>
#include <editeng/outliner.hxx>
#ifndef _SVX_SVXIDS_HRC
#include <svx/svxids.hrc>
#endif
#include <svx/svdograf.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdundo.hxx>
#include <vcl/msgbox.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/app.hxx>
#include <svx/svdpagv.hxx>
#include <sfx2/docfile.hxx>
#include <svx/svdoutl.hxx>
#include <svx/sdr/contact/displayinfo.hxx>

#include <svx/svdetc.hxx>
#include <editeng/editstat.hxx>

#include <svx/dialogs.hrc>
#include <sfx2/viewfrm.hxx>
#include <svx/svdopage.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <svx/xlndsit.hxx>
#include <svx/xlineit0.hxx>
#include <svx/xlnclit.hxx>
#include <vcl/virdev.hxx>

#include "app.hrc"
#include "strings.hrc"
#include "Window.hxx"
#include "Client.hxx"
#include "drawdoc.hxx"
#include "DrawDocShell.hxx"
#include "app.hxx"
#include "sdpage.hxx"
#include "glob.hrc"
#include "sdresid.hxx"
#include "DrawViewShell.hxx"
#include "futext.hxx"
#include "fuinsfil.hxx"
#include "slideshow.hxx"
#include "stlpool.hxx"
#include "FrameView.hxx"
#include "ViewClipboard.hxx"
#include "undo/undomanager.hxx"
#include <svx/sdr/contact/viewobjectcontact.hxx>
#include <svx/sdr/contact/viewcontact.hxx>
#include <svx/sdr/contact/displayinfo.hxx>
#include "EventMultiplexer.hxx"
#include "ViewShellBase.hxx"
#include "ViewShellManager.hxx"

#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/color/bcolor.hxx>
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/textlayoutdevice.hxx>
#include <drawinglayer/primitive2d/groupprimitive2d.hxx>
#include <svx/sdr/contact/objectcontact.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <drawinglayer/primitive2d/textprimitive2d.hxx>
#include <svx/unoapi.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>

#include <numeric>

using namespace com::sun::star;
using namespace com::sun::star::uno;
namespace sd {

#ifndef SO2_DECL_SVINPLACEOBJECT_DEFINED
#define SO2_DECL_SVINPLACEOBJECT_DEFINED
SO2_DECL_REF(SvInPlaceObject)
#endif

TYPEINIT1(View, FmFormView);

/*************************************************************************
|*
|* Ctor
|*
\************************************************************************/

View::View(SdDrawDocument* pDrawDoc, OutputDevice* pOutDev,
               ViewShell* pViewShell)
  : FmFormView(pDrawDoc, pOutDev),
    mpDoc(pDrawDoc),
    mpDocSh( pDrawDoc->GetDocSh() ),
    mpViewSh(pViewShell),
    mpDragSrcMarkList(NULL),
    mpDropMarkerObj(NULL),
    mpDropMarker(NULL),
    mnDragSrcPgNum(SDRPAGE_NOTFOUND),
    mnAction(DND_ACTION_NONE),
    mnLockRedrawSmph(0),
    mpLockedRedraws(NULL),
    mbIsDropAllowed(sal_True),
    maSmartTags(*this),
    mpClipboard (new ViewClipboard (*this))
{
    // #i73602# Use default from the configuration
    SetBufferedOverlayAllowed(getOptionsDrawinglayer().IsOverlayBuffer_DrawImpress());

    // #i74769#, #i75172# Use default from the configuration
    SetBufferedOutputAllowed(getOptionsDrawinglayer().IsPaintBuffer_DrawImpress());

    EnableExtendedKeyInputDispatcher(sal_False);
    EnableExtendedMouseEventDispatcher(sal_False);
    EnableExtendedCommandEventDispatcher(sal_False);

    SetUseIncompatiblePathCreateInterface(sal_False);
    SetMarkHdlWhenTextEdit(sal_True);
    EnableTextEditOnObjectsWithoutTextIfTextTool(sal_True);

    SetMinMoveDistancePixel(2);
    SetHitTolerancePixel(2);
    SetMeasureLayer(String(SdResId(STR_LAYER_MEASURELINES)));

    // Timer fuer verzoegertes Drop (muss fuer MAC sein)
    maDropErrorTimer.SetTimeoutHdl( LINK(this, View, DropErrorHdl) );
    maDropErrorTimer.SetTimeout(50);
    maDropInsertFileTimer.SetTimeoutHdl( LINK(this, View, DropInsertFileHdl) );
    maDropInsertFileTimer.SetTimeout(50);
}

void View::ImplClearDrawDropMarker()
{
    if(mpDropMarker)
    {
        delete mpDropMarker;
        mpDropMarker = 0L;
    }
}

/*************************************************************************
|*
|* Dtor
|*
\************************************************************************/

View::~View()
{
    maSmartTags.Dispose();

    // release content of selection clipboard, if we own the content
    UpdateSelectionClipboard( sal_True );

    maDropErrorTimer.Stop();
    maDropInsertFileTimer.Stop();

    ImplClearDrawDropMarker();

    while(PaintWindowCount())
    {
        // Alle angemeldeten OutDevs entfernen
        DeleteWindowFromPaintView(GetFirstOutputDevice() /*GetWin(0)*/);
    }

    // gespeicherte Redraws loeschen
    if (mpLockedRedraws)
    {
        SdViewRedrawRec* pRec = (SdViewRedrawRec*)mpLockedRedraws->First();
        while (pRec)
        {
            delete pRec;
            pRec = (SdViewRedrawRec*)mpLockedRedraws->Next();
        }
        delete mpLockedRedraws;
    }
}


class ViewRedirector : public ::sdr::contact::ViewObjectContactRedirector
{
public:
    ViewRedirector();
    virtual ~ViewRedirector();

    // all default implementations just call the same methods at the original. To do something
    // different, overload the method and at least do what the method does.
    virtual drawinglayer::primitive2d::Primitive2DSequence createRedirectedPrimitive2DSequence(
        const sdr::contact::ViewObjectContact& rOriginal,
        const sdr::contact::DisplayInfo& rDisplayInfo);
};

ViewRedirector::ViewRedirector()
{
}

ViewRedirector::~ViewRedirector()
{
}

drawinglayer::primitive2d::Primitive2DSequence ViewRedirector::createRedirectedPrimitive2DSequence(
    const sdr::contact::ViewObjectContact& rOriginal,
    const sdr::contact::DisplayInfo& rDisplayInfo)
{
    SdrObject* pObject = rOriginal.GetViewContact().TryToGetSdrObject();
    drawinglayer::primitive2d::Primitive2DSequence xRetval;

    if(pObject && pObject->GetPage())
    {
        const bool bDoCreateGeometry(pObject->GetPage()->checkVisibility( rOriginal, rDisplayInfo, true ));

        if(!bDoCreateGeometry && !(( pObject->GetObjInventor() == SdrInventor ) && ( pObject->GetObjIdentifier() == OBJ_PAGE )) )
            return xRetval;

        PresObjKind eKind(PRESOBJ_NONE);
        const bool bSubContentProcessing(rDisplayInfo.GetSubContentActive());
        const bool bIsMasterPageObject(pObject->GetPage()->IsMasterPage());
        const bool bIsPrinting(rOriginal.GetObjectContact().isOutputToPrinter());
        const SdrPageView* pPageView = rOriginal.GetObjectContact().TryToGetSdrPageView();
        const SdrPage* pVisualizedPage = GetSdrPageFromXDrawPage(rOriginal.GetObjectContact().getViewInformation2D().getVisualizedPage());
        const SdPage* pObjectsSdPage = dynamic_cast< SdPage* >(pObject->GetPage());
        const bool bIsInsidePageObj(pPageView && pPageView->GetPage() != pVisualizedPage);

        // check if we need to draw a placeholder border. Never do it for
        // objects inside a SdrPageObj and never when printing
        if(!bIsInsidePageObj && !bIsPrinting)
        {
            bool bCreateOutline(false);

            if( pObject->IsEmptyPresObj() && pObject->ISA(SdrTextObj) )
            {
                if( !bSubContentProcessing || !pObject->IsNotVisibleAsMaster() )
                {
                    eKind = pObjectsSdPage ? pObjectsSdPage->GetPresObjKind(pObject) : PRESOBJ_NONE;
                    bCreateOutline = true;
                }
            }
            else if( ( pObject->GetObjInventor() == SdrInventor ) && ( pObject->GetObjIdentifier() == OBJ_TEXT ) )
            {
                if( pObjectsSdPage )
                {
                    eKind = pObjectsSdPage->GetPresObjKind(pObject);

                    if((eKind == PRESOBJ_FOOTER) || (eKind == PRESOBJ_HEADER) || (eKind == PRESOBJ_DATETIME) || (eKind == PRESOBJ_SLIDENUMBER) )
                    {
                        if( !bSubContentProcessing )
                        {
                            // only draw a boundary for header&footer objects on the masterpage itself
                            bCreateOutline = true;
                        }
                    }
                }
            }
            else if( ( pObject->GetObjInventor() == SdrInventor ) && ( pObject->GetObjIdentifier() == OBJ_PAGE ) )
            {
                // only for handout page, else this frame will be created for each
                // page preview object in SlideSorter and PagePane
                if(pObjectsSdPage && PK_HANDOUT == pObjectsSdPage->GetPageKind())
                {
                    bCreateOutline = true;
                }
            }

            if(bCreateOutline)
            {
                // empty presentation objects get a gray frame
                const svtools::ColorConfig aColorConfig;
                const svtools::ColorConfigValue aColor( aColorConfig.GetColorValue( svtools::OBJECTBOUNDARIES ) );

                if( aColor.bIsVisible )
                {
                    // get basic object transformation
                    const basegfx::BColor aRGBColor(Color(aColor.nColor).getBColor());
                    basegfx::B2DHomMatrix aObjectMatrix;
                    basegfx::B2DPolyPolygon aObjectPolyPolygon;
                    pObject->TRGetBaseGeometry(aObjectMatrix, aObjectPolyPolygon);

                    // create dashed border
                    {
                        // create object polygon
                        basegfx::B2DPolygon aPolygon(basegfx::tools::createUnitPolygon());
                        aPolygon.transform(aObjectMatrix);

                        // create line and stroke attribute
                        ::std::vector< double > aDotDashArray;

                        aDotDashArray.push_back(160.0);
                        aDotDashArray.push_back(80.0);

                        const double fFullDotDashLen(::std::accumulate(aDotDashArray.begin(), aDotDashArray.end(), 0.0));
                        const drawinglayer::attribute::LineAttribute aLine(aRGBColor);
                        const drawinglayer::attribute::StrokeAttribute aStroke(aDotDashArray, fFullDotDashLen);

                        // create primitive and add
                        const drawinglayer::primitive2d::Primitive2DReference xRef(new drawinglayer::primitive2d::PolygonStrokePrimitive2D(
                            aPolygon,
                            aLine,
                            aStroke));
                        drawinglayer::primitive2d::appendPrimitive2DReferenceToPrimitive2DSequence(xRetval, xRef);
                    }

                    // now paint the placeholder description, but only when masterpage
                    // is displayed as page directly (MasterPage view)
                    if(!bSubContentProcessing && bIsMasterPageObject)
                    {
                        String aObjectString;

                        switch( eKind )
                        {
                            case PRESOBJ_TITLE:
                            {
                                if(pObjectsSdPage && pObjectsSdPage->GetPageKind() == PK_STANDARD)
                                {
                                    static String aTitleAreaStr( SdResId( STR_PLACEHOLDER_DESCRIPTION_TITLE ) );
                                    aObjectString = aTitleAreaStr;
                                }

                                break;
                            }
                            case PRESOBJ_OUTLINE:
                            {
                                static String aOutlineAreaStr( SdResId( STR_PLACEHOLDER_DESCRIPTION_OUTLINE ) );
                                aObjectString = aOutlineAreaStr;
                                break;
                            }
                            case PRESOBJ_FOOTER:
                            {
                                static String aFooterAreaStr( SdResId( STR_PLACEHOLDER_DESCRIPTION_FOOTER ) );
                                aObjectString = aFooterAreaStr;
                                break;
                            }
                            case PRESOBJ_HEADER:
                            {
                                static String aHeaderAreaStr( SdResId( STR_PLACEHOLDER_DESCRIPTION_HEADER ) );
                                aObjectString = aHeaderAreaStr;
                                break;
                            }
                            case PRESOBJ_DATETIME:
                            {
                                static String aDateTimeStr( SdResId( STR_PLACEHOLDER_DESCRIPTION_DATETIME ) );
                                aObjectString = aDateTimeStr;
                                break;
                            }
                            case PRESOBJ_NOTES:
                            {
                                static String aDateTimeStr( SdResId( STR_PLACEHOLDER_DESCRIPTION_NOTES ) );
                                aObjectString = aDateTimeStr;
                                break;
                            }
                            case PRESOBJ_SLIDENUMBER:
                            {
                                if(pObjectsSdPage && pObjectsSdPage->GetPageKind() == PK_STANDARD)
                                {
                                    static String aSlideAreaStr( SdResId( STR_PLACEHOLDER_DESCRIPTION_SLIDE ) );
                                    aObjectString = aSlideAreaStr;
                                }
                                else
                                {
                                    static String aNumberAreaStr( SdResId( STR_PLACEHOLDER_DESCRIPTION_NUMBER ) );
                                    aObjectString = aNumberAreaStr;
                                }
                                break;
                            }
                            default:
                            {
                                break;
                            }
                        }

                        if( aObjectString.Len() )
                        {
                            // decompose object matrix to be able to place text correctly
                            basegfx::B2DTuple aScale;
                            basegfx::B2DTuple aTranslate;
                            double fRotate, fShearX;
                            aObjectMatrix.decompose(aScale, aTranslate, fRotate, fShearX);

                            // create font
                            SdrTextObj* pTextObj = dynamic_cast< SdrTextObj* >( pObject );
                            const SdrTextVertAdjust eTVA(pTextObj ? pTextObj->GetTextVerticalAdjust() : SDRTEXTVERTADJUST_CENTER);
                            Font aScaledVclFont;

                            // use a text size factor to get more reliable text sizes from the text layouter
                            // (and from vcl), tipp from HDU
                            static sal_uInt32 nTextSizeFactor(100);

                            // use a factor to get more linear text size calculations
                            aScaledVclFont.SetHeight( 500 * nTextSizeFactor );

                            // get basic geometry and get text size
                            drawinglayer::primitive2d::TextLayouterDevice aTextLayouter;
                            aTextLayouter.setFont(aScaledVclFont);
                            const xub_StrLen nTextLength(aObjectString.Len());

                            // do not forget to use the factor again to get the width for the 500
                            const double fTextWidth(aTextLayouter.getTextWidth(aObjectString, 0, nTextLength) * (1.0 / nTextSizeFactor));
                            const double fTextHeight(aTextLayouter.getTextHeight() * (1.0 / nTextSizeFactor));

                            // calculate text primitive position. If text is at bottom, use top for
                            // the extra text and vice versa
                            const double fHorDist(125);
                            const double fVerDist(125);
                            const double fPosX((aTranslate.getX() + aScale.getX()) - fTextWidth - fHorDist);
                            const double fPosY((SDRTEXTVERTADJUST_BOTTOM == eTVA)
                                ? aTranslate.getY() - fVerDist + fTextHeight
                                : (aTranslate.getY() + aScale.getY()) - fVerDist);

                            // get font attributes; use normally scaled font
                            const basegfx::BColor aFontColor(aRGBColor);
                            Font aVclFont;
                            basegfx::B2DVector aTextSizeAttribute;

                            aVclFont.SetHeight( 500 );

                            const drawinglayer::attribute::FontAttribute aFontAttribute(
                                drawinglayer::primitive2d::getFontAttributeFromVclFont(
                                    aTextSizeAttribute,
                                    aVclFont,
                                    false,
                                    false));

                            // fill text matrix
                            const basegfx::B2DHomMatrix aTextMatrix(basegfx::tools::createScaleShearXRotateTranslateB2DHomMatrix(
                                aTextSizeAttribute.getX(), aTextSizeAttribute.getY(),
                                fShearX,
                                fRotate,
                                fPosX, fPosY));

                            // create DXTextArray (can be empty one)
                            const ::std::vector< double > aDXArray;

                            // create locale; this may need some more information in the future
                            const ::com::sun::star::lang::Locale aLocale;

                            // create primitive and add
                            const drawinglayer::primitive2d::Primitive2DReference xRef(
                                new drawinglayer::primitive2d::TextSimplePortionPrimitive2D(
                                    aTextMatrix,
                                    aObjectString,
                                    0,
                                    nTextLength,
                                    aDXArray,
                                    aFontAttribute,
                                    aLocale,
                                    aFontColor));
                            drawinglayer::primitive2d::appendPrimitive2DReferenceToPrimitive2DSequence(xRetval, xRef);
                        }
                    }
                }
            }
        }

        if(bDoCreateGeometry)
        {
            drawinglayer::primitive2d::appendPrimitive2DSequenceToPrimitive2DSequence(
                xRetval,
                sdr::contact::ViewObjectContactRedirector::createRedirectedPrimitive2DSequence(
                    rOriginal,
                    rDisplayInfo));
        }
    }
    else
    {
        // not a SdrObject visualisation (maybe e.g. page) or no page
        xRetval = sdr::contact::ViewObjectContactRedirector::createRedirectedPrimitive2DSequence(rOriginal, rDisplayInfo);
    }

    return xRetval;
}

/*************************************************************************
|*
|* Paint-Methode: das Ereignis wird an die View weitergeleitet
|*
\************************************************************************/

void View::CompleteRedraw(OutputDevice* pOutDev, const Region& rReg, sdr::contact::ViewObjectContactRedirector* pRedirector /*=0L*/)
{
    // ausfuehren ??
    if (mnLockRedrawSmph == 0)
    {
        SdrPageView* pPgView = GetSdrPageView();

        if (pPgView)
        {
            SdPage* pPage = (SdPage*) pPgView->GetPage();
            if( pPage )
            {
                SdrOutliner& rOutl=mpDoc->GetDrawOutliner(NULL);
                bool bScreenDisplay(true);

                if(bScreenDisplay && pOutDev && OUTDEV_PRINTER == pOutDev->GetOutDevType())
                {
                    // #i75566# printing; suppress AutoColor BackgroundColor generation
                    // for visibility reasons by giving GetPageBackgroundColor()
                    // the needed hint
                    bScreenDisplay = false;
                }

                if(bScreenDisplay && pOutDev && pOutDev->GetPDFWriter())
                {
                    // #i75566# PDF export; suppress AutoColor BackgroundColor generation (see above)
                    bScreenDisplay = false;
                }

                // #i75566# Name change GetBackgroundColor -> GetPageBackgroundColor and
                // hint value if screen display. Only then the AutoColor mechanisms shall be applied
                rOutl.SetBackgroundColor( pPage->GetPageBackgroundColor(pPgView, bScreenDisplay) );
            }
        }

        ViewRedirector aViewRedirector;
        FmFormView::CompleteRedraw(pOutDev, rReg, pRedirector ? pRedirector : &aViewRedirector);
    }
    // oder speichern?
    else
    {
        if (!mpLockedRedraws)
            mpLockedRedraws = new List;

        SdViewRedrawRec* pRec = new SdViewRedrawRec;
        pRec->mpOut = pOutDev;
        pRec->aRect = rReg.GetBoundRect();
        mpLockedRedraws->Insert(pRec, LIST_APPEND);
    }
}


/*************************************************************************
|*
|* Selektion hat sich geaendert
|*
\************************************************************************/

void View::MarkListHasChanged()
{
    FmFormView::MarkListHasChanged();

    if( GetMarkedObjectCount() > 0 )
        maSmartTags.deselect();
}


/*************************************************************************
|*
|* Attribute setzen
|*
\************************************************************************/

sal_Bool View::SetAttributes(const SfxItemSet& rSet, sal_Bool bReplaceAll)
{
    sal_Bool bOk = FmFormView::SetAttributes(rSet, bReplaceAll);
    return (bOk);
}


/*************************************************************************
|*
|* Attribute holen
|*
\************************************************************************/

sal_Bool View::GetAttributes( SfxItemSet& rTargetSet, sal_Bool bOnlyHardAttr ) const
{
    return( FmFormView::GetAttributes( rTargetSet, bOnlyHardAttr ) );
}


/*************************************************************************
|*
|* Ist ein Praesentationsobjekt selektiert?
|*
\************************************************************************/

sal_Bool View::IsPresObjSelected(sal_Bool bOnPage, sal_Bool bOnMasterPage, sal_Bool bCheckPresObjListOnly, sal_Bool bCheckLayoutOnly) const
{
    /**************************************************************************
    * Ist ein Presentationsobjekt selektiert?
    **************************************************************************/
    SdrMarkList* pMarkList;

    if (mnDragSrcPgNum != SDRPAGE_NOTFOUND &&
        mnDragSrcPgNum != GetSdrPageView()->GetPage()->GetPageNum())
    {
        // Es laeuft gerade Drag&Drop
        // Source- und Destination-Page unterschiedlich:
        // es wird die gemerkte MarkList verwendet
        pMarkList = mpDragSrcMarkList;
    }
    else
    {
        // Es wird die aktuelle MarkList verwendet
        pMarkList = new SdrMarkList(GetMarkedObjectList());
    }

    SdrMark* pMark;
    SdPage* pPage;
    SdrObject* pObj;

    sal_Bool bSelected = sal_False;
    sal_Bool bMasterPage = sal_False;
    long nMark;
    long nMarkMax = long(pMarkList->GetMarkCount()) - 1;

    for (nMark = nMarkMax; (nMark >= 0) && !bSelected; nMark--)
    {
        // Rueckwaerts durch die Marklist
        pMark = pMarkList->GetMark(nMark);
        pObj = pMark->GetMarkedSdrObj();

        if ( pObj && ( bCheckPresObjListOnly || pObj->IsEmptyPresObj() || pObj->GetUserCall() ) )
        {
            pPage = (SdPage*) pObj->GetPage();
            bMasterPage = pPage->IsMasterPage();

            if ( (bMasterPage && bOnMasterPage) || (!bMasterPage && bOnPage) )
            {
                if ( pPage && pPage->IsPresObj(pObj) )
                {
                    if( bCheckLayoutOnly )
                    {
                        PresObjKind eKind = pPage->GetPresObjKind(pObj);

                        if((eKind != PRESOBJ_FOOTER) && (eKind != PRESOBJ_HEADER) && (eKind != PRESOBJ_DATETIME) && (eKind != PRESOBJ_SLIDENUMBER) )
                            bSelected = sal_True;
                    }
                    else
                    {
                        bSelected = sal_True;
                    }
                }
            }
        }
    }

    if (pMarkList != mpDragSrcMarkList)
    {
       delete pMarkList;
    }

    return (bSelected);
}

/*************************************************************************
|*
|* Alles selektieren
|*
\************************************************************************/

void View::SelectAll()
{
    if ( IsTextEdit() )
    {
        OutlinerView* pOLV = GetTextEditOutlinerView();
        const ::Outliner* pOutliner = GetTextEditOutliner();
        pOLV->SelectRange( 0, (sal_uInt16) pOutliner->GetParagraphCount() );
    }
    else
    {
        MarkAll();
    }
}


/*************************************************************************
|*
|* Dokument hat sich geaendert
|*
\************************************************************************/

void View::ModelHasChanged()
{
    // Erst SdrView benachrichtigen
    FmFormView::ModelHasChanged();
}

/*************************************************************************
|*
|* StyleSheet setzen
|*
\************************************************************************/

sal_Bool View::SetStyleSheet(SfxStyleSheet* pStyleSheet, sal_Bool bDontRemoveHardAttr)
{
    // weiter an SdrView
    return FmFormView::SetStyleSheet(pStyleSheet, bDontRemoveHardAttr);
}


/*************************************************************************
|*
|* Texteingabe beginnen
|*
\************************************************************************/

static void SetSpellOptions( SdDrawDocument* pDoc, sal_uLong& rCntrl )
{
    sal_Bool bOnlineSpell = pDoc->GetOnlineSpell();

    if( bOnlineSpell )
        rCntrl |= EE_CNTRL_ONLINESPELLING;
    else
        rCntrl &= ~EE_CNTRL_ONLINESPELLING;
}

sal_Bool View::SdrBeginTextEdit(
    SdrObject* pObj, SdrPageView* pPV, ::Window* pWin,
    sal_Bool bIsNewObj,
    SdrOutliner* pOutl, OutlinerView* pGivenOutlinerView,
    sal_Bool bDontDeleteOutliner, sal_Bool bOnlyOneView, sal_Bool bGrabFocus )
{
    GetViewShell()->GetViewShellBase().GetEventMultiplexer()->MultiplexEvent(
        sd::tools::EventMultiplexerEvent::EID_BEGIN_TEXT_EDIT, (void*)pObj );

    if( pOutl==NULL && pObj )
        pOutl = SdrMakeOutliner( OUTLINERMODE_TEXTOBJECT, pObj->GetModel() );

    // make draw&impress specific initialisations
    if( pOutl )
    {
        pOutl->SetStyleSheetPool((SfxStyleSheetPool*) mpDoc->GetStyleSheetPool());
        pOutl->SetCalcFieldValueHdl(LINK(SD_MOD(), SdModule, CalcFieldValueHdl));
        sal_uLong nCntrl = pOutl->GetControlWord();
        nCntrl |= EE_CNTRL_ALLOWBIGOBJS;
        nCntrl |= EE_CNTRL_URLSFXEXECUTE;
        nCntrl |= EE_CNTRL_MARKFIELDS;
        nCntrl |= EE_CNTRL_AUTOCORRECT;

        nCntrl &= ~EE_CNTRL_ULSPACESUMMATION;
        if ( mpDoc->IsSummationOfParagraphs() )
            nCntrl |= EE_CNTRL_ULSPACESUMMATION;

        SetSpellOptions( mpDoc, nCntrl );

        pOutl->SetControlWord(nCntrl);

        Reference< linguistic2::XSpellChecker1 > xSpellChecker( LinguMgr::GetSpellChecker() );
        if ( xSpellChecker.is() )
            pOutl->SetSpeller( xSpellChecker );

        Reference< linguistic2::XHyphenator > xHyphenator( LinguMgr::GetHyphenator() );
        if( xHyphenator.is() )
            pOutl->SetHyphenator( xHyphenator );

        pOutl->SetDefaultLanguage( Application::GetSettings().GetLanguage() );
    }

    sal_Bool bReturn = FmFormView::SdrBeginTextEdit(
        pObj, pPV, pWin, bIsNewObj, pOutl,
        pGivenOutlinerView, bDontDeleteOutliner,
        bOnlyOneView, bGrabFocus);

    if (bReturn)
    {
        ::Outliner* pOL = GetTextEditOutliner();

        if( pObj && pObj->GetPage() )
        {
            Color aBackground;
            if( pObj->GetObjInventor() == SdrInventor && pObj->GetObjIdentifier() == OBJ_TABLE )
            {
                aBackground = GetTextEditBackgroundColor(*this);
            }
            else
            {
                aBackground = pObj->GetPage()->GetPageBackgroundColor(pPV);
            }
            pOL->SetBackgroundColor( aBackground  );
        }

        pOL->SetParaInsertedHdl(LINK(this, View, OnParagraphInsertedHdl));
        pOL->SetParaRemovingHdl(LINK(this, View, OnParagraphRemovingHdl));
    }

    return(bReturn);
}

/** ends current text editing */
SdrEndTextEditKind View::SdrEndTextEdit(sal_Bool bDontDeleteReally )
{
    SdrObjectWeakRef xObj( GetTextEditObject() );

    sal_Bool bDefaultTextRestored = RestoreDefaultText( dynamic_cast< SdrTextObj* >( GetTextEditObject() ) );

    SdrEndTextEditKind eKind = FmFormView::SdrEndTextEdit(bDontDeleteReally);

    if( bDefaultTextRestored )
    {
        if( xObj.is() && !xObj->IsEmptyPresObj() )
        {
            xObj->SetEmptyPresObj( sal_True );
        }
        else
        {
            eKind = SDRENDTEXTEDIT_UNCHANGED;
        }
    }
    else if( xObj.is() && xObj->IsEmptyPresObj() )
    {
        SdrTextObj* pObj = dynamic_cast< SdrTextObj* >( xObj.get() );
        if( pObj && pObj->HasText() )
        {
            SdrPage* pPage = pObj->GetPage();
            if( !pPage || !pPage->IsMasterPage() )
                pObj->SetEmptyPresObj( sal_False );
        }
    }

    GetViewShell()->GetViewShellBase().GetEventMultiplexer()->MultiplexEvent(sd::tools::EventMultiplexerEvent::EID_END_TEXT_EDIT, (void*)xObj.get() );

    if( xObj.is() )
    {
        SdPage* pPage = dynamic_cast< SdPage* >( xObj->GetPage() );
        if( pPage )
            pPage->onEndTextEdit( xObj.get() );
    }

    return(eKind);
}

// --------------------------------------------------------------------

/** restores the default text if the given text object is currently in edit mode and
    no text has been entered already. Is only usefull just before text edit ends. */
bool View::RestoreDefaultText( SdrTextObj* pTextObj )
{
    bool bRestored = false;

    if( pTextObj && (pTextObj == GetTextEditObject()) )
    {
        if( !pTextObj->HasText() )
        {
            SdPage* pPage = dynamic_cast< SdPage* >( pTextObj->GetPage() );

            if(pPage)
            {
                bRestored = pPage->RestoreDefaultText( pTextObj );
                if( bRestored )
                {
                    SdrOutliner* pOutliner = GetTextEditOutliner();
                    pTextObj->SetTextEditOutliner( pOutliner );
                    OutlinerParaObject* pParaObj = pTextObj->GetOutlinerParaObject();
                    if (pOutliner)
                        pOutliner->SetText(*pParaObj);
                }
            }
        }
    }

    return bRestored;
}

/*************************************************************************
|*
|* Originalgroesse der markierten Objekte setzen
|*
\************************************************************************/

void View::SetMarkedOriginalSize()
{
    SdrUndoGroup*   pUndoGroup = new SdrUndoGroup(*mpDoc);
    sal_uLong           nCount = GetMarkedObjectCount();
    sal_Bool            bOK = sal_False;

    for( sal_uInt32 i = 0; i < nCount; i++ )
    {
        SdrObject* pObj = GetMarkedObjectByIndex(i);

        if( pObj->GetObjInventor() == SdrInventor )
        {
            if( pObj->GetObjIdentifier() == OBJ_OLE2 )
            {
                uno::Reference < embed::XEmbeddedObject > xObj = ((SdrOle2Obj*)pObj)->GetObjRef();
                if( xObj.is() )
                {
                    // TODO/LEAN: working with VisualArea can switch object to running state

                       sal_Int64 nAspect = ((SdrOle2Obj*)pObj)->GetAspect();
                    Size aOleSize;

                    if ( nAspect == embed::Aspects::MSOLE_ICON )
                    {
                        MapMode aMap100( MAP_100TH_MM );
                        aOleSize = ((SdrOle2Obj*)pObj)->GetOrigObjSize( &aMap100 );
                        bOK = sal_True;
                    }
                    else
                    {
                        MapUnit aUnit = VCLUnoHelper::UnoEmbed2VCLMapUnit( xObj->getMapUnit( nAspect ) );
                        try
                        {
                            awt::Size aSz = xObj->getVisualAreaSize( nAspect );
                            aOleSize = OutputDevice::LogicToLogic( Size( aSz.Width, aSz.Height ), aUnit, MAP_100TH_MM );
                            bOK = sal_True;
                        }
                        catch( embed::NoVisualAreaSizeException& )
                        {}
                    }

                    if ( bOK )
                    {
                        Rectangle   aDrawRect( pObj->GetLogicRect() );

                        pUndoGroup->AddAction( mpDoc->GetSdrUndoFactory().CreateUndoGeoObject( *pObj ) );
                        pObj->Resize( aDrawRect.TopLeft(), Fraction( aOleSize.Width(), aDrawRect.GetWidth() ),
                                                           Fraction( aOleSize.Height(), aDrawRect.GetHeight() ) );
                    }
                }
            }
            else if( pObj->GetObjIdentifier() == OBJ_GRAF )
            {
                const MapMode   aMap100( MAP_100TH_MM );
                Size            aSize;

                if ( static_cast< SdrGrafObj* >( pObj )->GetGrafPrefMapMode().GetMapUnit() == MAP_PIXEL )
                    aSize = Application::GetDefaultDevice()->PixelToLogic( static_cast< SdrGrafObj* >( pObj )->GetGrafPrefSize(), aMap100 );
                else
                {
                    aSize = OutputDevice::LogicToLogic( static_cast< SdrGrafObj* >( pObj )->GetGrafPrefSize(),
                                                        static_cast< SdrGrafObj* >( pObj )->GetGrafPrefMapMode(),
                                                        aMap100 );
                }

                pUndoGroup->AddAction( GetModel()->GetSdrUndoFactory().CreateUndoGeoObject(*pObj ) );
                Rectangle aRect( pObj->GetLogicRect() );
                aRect.SetSize( aSize );
                pObj->SetLogicRect( aRect );

                bOK = sal_True;
            }
        }
    }

    if( bOK )
    {
        pUndoGroup->SetComment( String(SdResId(STR_UNDO_ORIGINALSIZE)) );
        mpDocSh->GetUndoManager()->AddUndoAction(pUndoGroup);
    }
    else
        delete pUndoGroup;
}

/*************************************************************************
|*
|* OLE-Obj am Client connecten
|*
\************************************************************************/

void View::DoConnect(SdrOle2Obj* pObj)
{
    if (mpViewSh)
    {
        uno::Reference < embed::XEmbeddedObject > xObj( pObj->GetObjRef() );
        if( xObj.is() )
        {
            ::sd::Window* pWindow = mpViewSh->GetActiveWindow();
            SfxInPlaceClient* pSdClient = mpViewSh-> GetViewShellBase().FindIPClient( xObj, pWindow );
            if ( !pSdClient )
            {
                pSdClient = new Client(pObj, mpViewSh, pWindow);
                Rectangle aRect = pObj->GetLogicRect();
                {
                    // TODO/LEAN: working with visual area can switch object to running state
                    Size aDrawSize = aRect.GetSize();
                    awt::Size aSz;

                    MapMode aMapMode( mpDoc->GetScaleUnit() );
                    Size aObjAreaSize = pObj->GetOrigObjSize( &aMapMode );

                    Fraction aScaleWidth (aDrawSize.Width(),  aObjAreaSize.Width() );
                    Fraction aScaleHeight(aDrawSize.Height(), aObjAreaSize.Height() );
                    aScaleWidth.ReduceInaccurate(10);       // kompatibel zum SdrOle2Obj
                    aScaleHeight.ReduceInaccurate(10);
                    pSdClient->SetSizeScale(aScaleWidth, aScaleHeight);

                    // sichtbarer Ausschnitt wird nur inplace veraendert!
                    // the object area must be set after the scaling, since it triggers resize
                    aRect.SetSize(aObjAreaSize);
                    pSdClient->SetObjArea(aRect);
                }
            }
        }
    }
}

/*************************************************************************
|*
|*
|*
\************************************************************************/

sal_Bool View::IsMorphingAllowed() const
{
    const SdrMarkList&  rMarkList = GetMarkedObjectList();
    sal_Bool                bRet = sal_False;

    if ( rMarkList.GetMarkCount() == 2 )
    {
        const SdrObject*    pObj1 = rMarkList.GetMark( 0 )->GetMarkedSdrObj();
        const SdrObject*    pObj2 = rMarkList.GetMark( 1 )->GetMarkedSdrObj();
        const sal_uInt16        nKind1 = pObj1->GetObjIdentifier();
        const sal_uInt16        nKind2 = pObj2->GetObjIdentifier();

        if ( ( nKind1 != OBJ_TEXT && nKind2 != OBJ_TEXT ) &&
             ( nKind1 != OBJ_TITLETEXT && nKind2 != OBJ_TITLETEXT ) &&
             ( nKind1 != OBJ_OUTLINETEXT && nKind2 != OBJ_OUTLINETEXT ) &&
             ( nKind1 != OBJ_GRUP && nKind2 != OBJ_GRUP ) &&
             ( nKind1 != OBJ_LINE && nKind2 != OBJ_LINE ) &&
             ( nKind1 != OBJ_PLIN && nKind2 != OBJ_PLIN ) &&
             ( nKind1 != OBJ_PATHLINE && nKind2 != OBJ_PATHLINE ) &&
             ( nKind1 != OBJ_FREELINE && nKind2 != OBJ_FREELINE ) &&
             ( nKind1 != OBJ_PATHPLIN && nKind2 != OBJ_PATHPLIN ) &&
             ( nKind1 != OBJ_MEASURE && nKind2 != OBJ_MEASURE ) &&
             ( nKind1 != OBJ_EDGE && nKind2 != OBJ_EDGE ) &&
             ( nKind1 != OBJ_GRAF && nKind2 != OBJ_GRAF ) &&
             ( nKind1 != OBJ_OLE2 && nKind2 != OBJ_OLE2 ) &&
             ( nKind1 != OBJ_CAPTION && nKind2 !=  OBJ_CAPTION ) &&
             !pObj1->ISA( E3dObject) && !pObj2->ISA( E3dObject) )
        {
            SfxItemSet      aSet1( mpDoc->GetPool(), XATTR_FILLSTYLE, XATTR_FILLSTYLE );
            SfxItemSet      aSet2( mpDoc->GetPool(), XATTR_FILLSTYLE, XATTR_FILLSTYLE );

            aSet1.Put(pObj1->GetMergedItemSet());
            aSet2.Put(pObj2->GetMergedItemSet());

            const XFillStyle    eFillStyle1 = ( (const XFillStyleItem&) aSet1.Get( XATTR_FILLSTYLE ) ).GetValue();
            const XFillStyle    eFillStyle2 = ( (const XFillStyleItem&) aSet2.Get( XATTR_FILLSTYLE ) ).GetValue();

            if( ( eFillStyle1 == XFILL_NONE || eFillStyle1 == XFILL_SOLID ) &&
                ( eFillStyle2 == XFILL_NONE || eFillStyle2 == XFILL_SOLID ) )
                bRet = sal_True;
        }
    }

    return bRet;
}

/*************************************************************************
|*
|*
|*
\************************************************************************/

sal_Bool View::IsVectorizeAllowed() const
{
    const SdrMarkList&  rMarkList = GetMarkedObjectList();
    sal_Bool                bRet = sal_False;

    if( rMarkList.GetMarkCount() == 1 )
    {
        const SdrObject* pObj = rMarkList.GetMark( 0 )->GetMarkedSdrObj();

        if( pObj->ISA( SdrGrafObj ) && ( (SdrGrafObj*) pObj )->GetGraphicType() == GRAPHIC_BITMAP )
            bRet = sal_True;
    }

    return bRet;
}

void View::onAccessibilityOptionsChanged()
{
    if( mpViewSh )
    {
        ::sd::Window* pWindow = mpViewSh->GetActiveWindow();
        if( pWindow )
        {
            const StyleSettings& rStyleSettings = pWindow->GetSettings().GetStyleSettings();

            sal_uInt16 nOutputSlot, nPreviewSlot;

            SvtAccessibilityOptions& aAccOptions = getAccessibilityOptions();

            if( mpViewSh->GetViewFrame() && mpViewSh->GetViewFrame()->GetDispatcher() )
            {
                if( rStyleSettings.GetHighContrastMode() )
                {
                    nOutputSlot = SID_OUTPUT_QUALITY_CONTRAST;
                }
                else
                {
                    nOutputSlot = SID_OUTPUT_QUALITY_COLOR;
                }

                if( rStyleSettings.GetHighContrastMode() && aAccOptions.GetIsForPagePreviews() )
                {
                    nPreviewSlot = SID_PREVIEW_QUALITY_CONTRAST;
                }
                else
                {
                    nPreviewSlot = SID_PREVIEW_QUALITY_COLOR;
                }

                mpViewSh->GetViewFrame()->GetDispatcher()->Execute( nOutputSlot, SFX_CALLMODE_ASYNCHRON );
                mpViewSh->GetViewFrame()->GetDispatcher()->Execute( nPreviewSlot, SFX_CALLMODE_ASYNCHRON );
            }

            mpViewSh->Invalidate();
        }
    }
}

IMPL_LINK( View, OnParagraphInsertedHdl, ::Outliner *, pOutliner )
{
    Paragraph* pPara = pOutliner->GetHdlParagraph();
    SdrObject* pObj = GetTextEditObject();

    if( pPara && pObj )
    {
        SdPage* pPage = dynamic_cast< SdPage* >( pObj->GetPage() );
        if( pPage )
            pPage->onParagraphInserted( pOutliner, pPara, pObj );
    }
    return 0;
}

/*************************************************************************
|*
|* Handler fuer das Loeschen von Seiten (Absaetzen)
|*
\************************************************************************/

IMPL_LINK( View, OnParagraphRemovingHdl, ::Outliner *, pOutliner )
{
    Paragraph* pPara = pOutliner->GetHdlParagraph();
    SdrObject* pObj = GetTextEditObject();

    if( pPara && pObj )
    {
        SdPage* pPage = dynamic_cast< SdPage* >( pObj->GetPage() );
        if( pPage )
            pPage->onParagraphRemoving( pOutliner, pPara, pObj );
    }
    return 0;
}

bool View::isRecordingUndo() const
{
    if( mpDoc && mpDoc->IsUndoEnabled() )
    {
        sd::UndoManager* pUndoManager = mpDoc ? mpDoc->GetUndoManager() : 0;
        return pUndoManager && pUndoManager->IsInListAction();
    }
    else
    {
        return false;
    }
}

void View::AddCustomHdl()
{
    maSmartTags.addCustomHandles( aHdl );
}

void View::updateHandles()
{
    AdjustMarkHdl();
}

SdrViewContext View::GetContext() const
{
    SdrViewContext eContext = SDRCONTEXT_STANDARD;
    if( maSmartTags.getContext( eContext ) )
        return eContext;
    else
        return FmFormView::GetContext();
}

sal_Bool View::HasMarkablePoints() const
{
    if( maSmartTags.HasMarkablePoints() )
        return true;
    else
        return FmFormView::HasMarkablePoints();
}

sal_uLong View::GetMarkablePointCount() const
{
    sal_uLong nCount = FmFormView::GetMarkablePointCount();
    nCount += maSmartTags.GetMarkablePointCount();
    return nCount;
}

sal_Bool View::HasMarkedPoints() const
{
    if( maSmartTags.HasMarkedPoints() )
        return true;
    else
        return FmFormView::HasMarkedPoints();
}

sal_uLong View::GetMarkedPointCount() const
{
    sal_uLong nCount = FmFormView::GetMarkedPointCount();
    nCount += maSmartTags.GetMarkedPointCount();
    return nCount;
}

sal_Bool View::IsPointMarkable(const SdrHdl& rHdl) const
{
    if( maSmartTags.IsPointMarkable( rHdl ) )
        return true;
    else
        return FmFormView::IsPointMarkable( rHdl );
}

sal_Bool View::MarkPoint(SdrHdl& rHdl, sal_Bool bUnmark )
{
    if( maSmartTags.MarkPoint( rHdl, bUnmark ) )
        return true;
    else
        return FmFormView::MarkPoint( rHdl, bUnmark );
}

sal_Bool View::MarkPoints(const Rectangle* pRect, sal_Bool bUnmark)
{
    if( maSmartTags.MarkPoints( pRect, bUnmark ) )
        return true;
    else
        return FmFormView::MarkPoints( pRect, bUnmark );
}

void View::CheckPossibilities()
{
    FmFormView::CheckPossibilities();
    maSmartTags.CheckPossibilities();
}

void View::OnBeginPasteOrDrop( PasteOrDropInfos* /*pInfos*/ )
{
}

/** this is called after a paste or drop operation, make sure that the newly inserted paragraphs
    get the correct style sheet. */
void View::OnEndPasteOrDrop( PasteOrDropInfos* pInfos )
{
    SdrTextObj* pTextObj = dynamic_cast< SdrTextObj* >( GetTextEditObject() );
    SdrOutliner* pOutliner = GetTextEditOutliner();
    if( pOutliner && pTextObj && pTextObj->GetPage() )
    {
        SdPage* pPage = static_cast< SdPage* >( pTextObj->GetPage() );

        SfxStyleSheet* pStyleSheet = 0;

        const PresObjKind eKind = pPage->GetPresObjKind(pTextObj);
        if( eKind != PRESOBJ_NONE )
            pStyleSheet = pPage->GetStyleSheetForPresObj(eKind);
        else
            pStyleSheet = pTextObj->GetStyleSheet();

        if( eKind == PRESOBJ_OUTLINE )
        {
            // for outline shapes, set the correct outline style sheet for each
            // new paragraph, depending on the paragraph depth
            SfxStyleSheetBasePool* pStylePool = GetDoc()->GetStyleSheetPool();

            for ( sal_uInt16 nPara = pInfos->nStartPara; nPara <= pInfos->nEndPara; nPara++ )
            {
                sal_Int16 nDepth = pOutliner->GetDepth( nPara );

                SfxStyleSheet* pStyle = 0;
                if( nDepth > 0 )
                {
                    String aStyleSheetName( pStyleSheet->GetName() );
                    aStyleSheetName.Erase( aStyleSheetName.Len() - 1, 1 );
                    aStyleSheetName += String::CreateFromInt32( nDepth );
                    pStyle = static_cast<SfxStyleSheet*>( pStylePool->Find( aStyleSheetName, pStyleSheet->GetFamily() ) );
                    DBG_ASSERT( pStyle, "sd::View::OnEndPasteOrDrop(), Style not found!" );
                }

                if( !pStyle )
                    pStyle = pStyleSheet;

                pOutliner->SetStyleSheet( nPara, pStyle );
            }
        }
        else
        {
            // just put the object style on each new paragraph
            for ( sal_uInt16 nPara = pInfos->nStartPara; nPara <= pInfos->nEndPara; nPara++ )
            {
                pOutliner->SetStyleSheet( nPara, pStyleSheet );
            }
        }
    }
}

} // end of namespace sd
