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
#include <sfx2/sidebar/EnumContext.hxx>
#include <svx/svdopage.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <svx/xlndsit.hxx>
#include <svx/xlineit0.hxx>
#include <svx/xlnclit.hxx>
#include <svx/sidebar/ContextChangeEventMultiplexer.hxx>
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
#include <svx/svdotable.hxx>
#include "EventMultiplexer.hxx"
#include "ViewShellBase.hxx"
#include "ViewShellManager.hxx"

#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/color/bcolor.hxx>
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/textlayoutdevice.hxx>
#include <drawinglayer/primitive2d/groupprimitive2d.hxx>
#include <svx/sdr/contact/objectcontact.hxx>
#include <svx/sdr/table/tablecontroller.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <drawinglayer/primitive2d/textprimitive2d.hxx>
#include <svx/unoapi.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <svx/svdlegacy.hxx>
#include "DrawController.hxx"
#include <numeric>

using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace sdr::table;
namespace sd {

#ifndef SO2_DECL_SVINPLACEOBJECT_DEFINED
#define SO2_DECL_SVINPLACEOBJECT_DEFINED
SO2_DECL_REF(SvInPlaceObject)
#endif

/*************************************************************************
|*
|* Ctor
|*
\************************************************************************/

View::View(SdDrawDocument& rDrawDoc, OutputDevice* pOutDev,
               ViewShell* pViewShell)
  : FmFormView(rDrawDoc, pOutDev),
    mpDoc(&rDrawDoc),
    mpDocSh( rDrawDoc.GetDocSh() ),
    mpViewSh(pViewShell),
    maDragSrcMarkList(),
    mpDropMarkerObj(NULL),
    mpDropMarker(NULL),
    mnDragSrcPgNum(SDRPAGE_NOTFOUND),
    mnAction(DND_ACTION_NONE),
    mnLockRedrawSmph(0),
    mpLockedRedraws(NULL),
    mbIsDropAllowed(true),
    maSmartTags(*this),
    mpClipboard (new ViewClipboard (*this))
{
    // #i73602# Use default from the configuration
    SetBufferedOverlayAllowed(getOptionsDrawinglayer().IsOverlayBuffer_DrawImpress());

    // #i74769#, #i75172# Use default from the configuration
    SetBufferedOutputAllowed(getOptionsDrawinglayer().IsPaintBuffer_DrawImpress());

    EnableExtendedKeyInputDispatcher(false);
    EnableExtendedMouseEventDispatcher(false);
    EnableExtendedCommandEventDispatcher(false);

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
    UpdateSelectionClipboard( true );

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

    if(pObject && pObject->getSdrPageFromSdrObject())
    {
        const bool bDoCreateGeometry(pObject->getSdrPageFromSdrObject()->checkVisibility( rOriginal, rDisplayInfo, true ));

        if(!bDoCreateGeometry && !(( pObject->GetObjInventor() == SdrInventor ) && ( pObject->GetObjIdentifier() == OBJ_PAGE )) )
            return xRetval;

        PresObjKind eKind(PRESOBJ_NONE);
        const bool bSubContentProcessing(rDisplayInfo.GetSubContentActive());
        const bool bIsMasterPageObject(pObject->getSdrPageFromSdrObject()->IsMasterPage());
        const bool bIsPrinting(rOriginal.GetObjectContact().isOutputToPrinter());
        const SdrPage* pVisualizedPage = GetSdrPageFromXDrawPage(rOriginal.GetObjectContact().getViewInformation2D().getVisualizedPage());
        const SdPage* pObjectsSdPage = dynamic_cast< SdPage* >(pObject->getSdrPageFromSdrObject());
        const SdrView* pSdrView = rOriginal.GetObjectContact().TryToGetSdrView();
        const bool bIsInsidePageObj(pSdrView && pSdrView->GetSdrPageView() && &pSdrView->GetSdrPageView()->getSdrPageFromSdrPageView() != pVisualizedPage);

        // check if we need to draw a placeholder border. Never do it for
        // objects inside a SdrPageObj and never when printing
        if(!bIsInsidePageObj && !bIsPrinting)
        {
            bool bCreateOutline(false);

            if( pObject->IsEmptyPresObj() && dynamic_cast< SdrTextObj* >(pObject) )
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
                    const basegfx::B2DHomMatrix aObjectMatrix(pObject->getSdrObjectTransformation());

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
                            basegfx::B2DVector aScale;
                            basegfx::B2DPoint aTranslate;
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

void View::CompleteRedraw(OutputDevice* pOutDev, const Region& rReg, sdr::contact::ViewObjectContactRedirector* pRedirector /*=0L*/) const
{
    // ausfuehren ??
    if (mnLockRedrawSmph == 0)
    {
        SdrPageView* pPgView = GetSdrPageView();

        if (pPgView)
        {
            SdPage& rPage = (SdPage&) pPgView->getSdrPageFromSdrPageView();
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
            rOutl.SetBackgroundColor( rPage.GetPageBackgroundColor(pPgView, bScreenDisplay) );
        }

        ViewRedirector aViewRedirector;
        FmFormView::CompleteRedraw(pOutDev, rReg, pRedirector ? pRedirector : &aViewRedirector);
    }
    // oder speichern?
    else
    {
        if (!mpLockedRedraws)
        {
            const_cast< View* >(this)->mpLockedRedraws = new List;
        }

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

void View::handleSelectionChange()
{
    // call parent
    FmFormView::handleSelectionChange();

    // local reactions
    if( areSdrObjectsSelected() )
        maSmartTags.deselect();
}


/*************************************************************************
|*
|* Attribute setzen
|*
\************************************************************************/

bool View::SetAttributes(const SfxItemSet& rSet, bool bReplaceAll)
{
    bool bOk = FmFormView::SetAttributes(rSet, bReplaceAll);
    return (bOk);
}


/*************************************************************************
|*
|* Attribute holen
|*
\************************************************************************/

bool View::GetAttributes( SfxItemSet& rTargetSet, bool bOnlyHardAttr ) const
{
    return( FmFormView::GetAttributes( rTargetSet, bOnlyHardAttr ) );
}


/*************************************************************************
|*
|* Ist ein Praesentationsobjekt selektiert?
|*
\************************************************************************/

bool View::IsPresObjSelected(bool bOnPage, bool bOnMasterPage, bool bCheckPresObjListOnly, bool bCheckLayoutOnly) const
{
    /**************************************************************************
    * Ist ein Presentationsobjekt selektiert?
    **************************************************************************/
    SdrObjectVector aSelection;

    if (mnDragSrcPgNum != SDRPAGE_NOTFOUND && GetSdrPageView() && mnDragSrcPgNum != GetSdrPageView()->getSdrPageFromSdrPageView().GetPageNumber())
    {
        // Es laeuft gerade Drag&Drop
        // Source- und Destination-Page unterschiedlich:
        // es wird die gemerkte MarkList verwendet
        aSelection = maDragSrcMarkList;
    }
    else
    {
        // Es wird die aktuelle MarkList verwendet
        aSelection = getSelectedSdrObjectVectorFromSdrMarkView();
    }

    SdPage* pPage;
    SdrObject* pObj;

    bool bSelected = false;
    bool bMasterPage = false;

    if(aSelection.size())
    {
        for(sal_uInt32 nMark(aSelection.size()); nMark && !bSelected;)
        {
            // Rueckwaerts durch die Marklist
            pObj = aSelection[--nMark];

            if ( pObj && ( bCheckPresObjListOnly || pObj->IsEmptyPresObj() || findConnectionToSdrObject(pObj) ) )
            {
                pPage = (SdPage*) pObj->getSdrPageFromSdrObject();
                bMasterPage = pPage->IsMasterPage();

                if ( (bMasterPage && bOnMasterPage) || (!bMasterPage && bOnPage) )
                {
                    if ( pPage && pPage->IsPresObj(pObj) )
                    {
                        if( bCheckLayoutOnly )
                        {
                            PresObjKind eKind = pPage->GetPresObjKind(pObj);

                            if((eKind != PRESOBJ_FOOTER) && (eKind != PRESOBJ_HEADER) && (eKind != PRESOBJ_DATETIME) && (eKind != PRESOBJ_SLIDENUMBER) )
                                    bSelected = true;
                        }
                        else
                        {
                                bSelected = true;
                        }
                    }
                }
            }
        }
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

void View::LazyReactOnObjectChanges()
{
    // call parent
    FmFormView::LazyReactOnObjectChanges();
}

/*************************************************************************
|*
|* StyleSheet setzen
|*
\************************************************************************/

bool View::SetStyleSheet(SfxStyleSheet* pStyleSheet, bool bDontRemoveHardAttr)
{
    // weiter an SdrView
    return FmFormView::SetStyleSheet(pStyleSheet, bDontRemoveHardAttr);
}


/*************************************************************************
|*
|* Texteingabe beginnen
|*
\************************************************************************/

static void SetSpellOptions( SdDrawDocument* pDoc, sal_uInt32& rCntrl )
{
    bool bOnlineSpell = pDoc->GetOnlineSpell();

    if( bOnlineSpell )
        rCntrl |= EE_CNTRL_ONLINESPELLING;
    else
        rCntrl &= ~EE_CNTRL_ONLINESPELLING;
}

bool View::SdrBeginTextEdit(
    SdrObject* pObj, ::Window* pWin,
    bool bIsNewObj, SdrOutliner* pOutl, OutlinerView* pGivenOutlinerView,
    bool bDontDeleteOutliner, bool bOnlyOneView, bool bGrabFocus )
{
    GetViewShell()->GetViewShellBase().GetEventMultiplexer()->MultiplexEvent(
        sd::tools::EventMultiplexerEvent::EID_BEGIN_TEXT_EDIT, (void*)pObj );

    if( pOutl==NULL && pObj )
        pOutl = SdrMakeOutliner( OUTLINERMODE_TEXTOBJECT, &pObj->getSdrModelFromSdrObject() );

    // make draw&impress specific initialisations
    if( pOutl )
    {
        pOutl->SetStyleSheetPool((SfxStyleSheetPool*) mpDoc->GetStyleSheetPool());
        pOutl->SetCalcFieldValueHdl(LINK(SD_MOD(), SdModule, CalcFieldValueHdl));
        sal_uInt32 nCntrl = pOutl->GetControlWord();
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
        pObj, pWin, bIsNewObj, pOutl,
        pGivenOutlinerView, bDontDeleteOutliner,
        bOnlyOneView, bGrabFocus);

    if ( mpViewSh )
    {
        mpViewSh->GetViewShellBase().GetDrawController().FireSelectionChangeListener();
    }

    if (bReturn)
    {
        ::Outliner* pOL = GetTextEditOutliner();

        if( pObj && pObj->getSdrPageFromSdrObject() )
        {
            Color aBackground;
            if( pObj->GetObjInventor() == SdrInventor && pObj->GetObjIdentifier() == OBJ_TABLE )
            {
                aBackground = GetTextEditBackgroundColor(*this);
            }
            else
            {
                if(GetSdrPageView())
                {
                    aBackground = pObj->getSdrPageFromSdrObject()->GetPageBackgroundColor(GetSdrPageView());
                }
            }

            if(pOL)
            {
                pOL->SetBackgroundColor( aBackground  );
            }
        }

        if (pOL)
        {
            pOL->SetParaInsertedHdl(LINK(this, View, OnParagraphInsertedHdl));
            pOL->SetParaRemovingHdl(LINK(this, View, OnParagraphRemovingHdl));
        }
    }

    return(bReturn);
}

/** ends current text editing */
SdrEndTextEditKind View::SdrEndTextEdit(bool bDontDeleteReally )
{
    SdrObjectWeakRef xObj( GetTextEditObject() );

    bool bDefaultTextRestored = RestoreDefaultText( dynamic_cast< SdrTextObj* >( GetTextEditObject() ) );

    SdrEndTextEditKind eKind = FmFormView::SdrEndTextEdit(bDontDeleteReally);

    if( bDefaultTextRestored )
    {
        if( xObj.is() && !xObj->IsEmptyPresObj() )
        {
            xObj->SetEmptyPresObj( true );
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
            SdrPage* pPage = pObj->getSdrPageFromSdrObject();
            if( !pPage || !pPage->IsMasterPage() )
                pObj->SetEmptyPresObj( false );
        }
    }

    GetViewShell()->GetViewShellBase().GetEventMultiplexer()->MultiplexEvent(
        sd::tools::EventMultiplexerEvent::EID_END_TEXT_EDIT,
        (void*)xObj.get() );

    if( xObj.is() )
    {
        if ( mpViewSh )
        {
            mpViewSh->GetViewShellBase().GetDrawController().FireSelectionChangeListener();
        }

        SdPage* pPage = dynamic_cast< SdPage* >( xObj->getSdrPageFromSdrObject() );
        if( pPage )
            pPage->onEndTextEdit( xObj.get() );
    }

    return(eKind);
}

// --------------------------------------------------------------------

/** restores the default text if the given text object is currently in edit mode and
    no text has been entered already. Is only useful just before text edit ends. */
bool View::RestoreDefaultText( SdrTextObj* pTextObj )
{
    bool bRestored = false;

    if( pTextObj && (pTextObj == GetTextEditObject()) )
    {
        if( !pTextObj->HasText() )
        {
            SdPage* pPage = dynamic_cast< SdPage* >( pTextObj->getSdrPageFromSdrObject() );

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
    const SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView() );
    bool bOK(false);

    for(sal_uInt32 i(0); i < aSelection.size(); i++)
    {
        SdrOle2Obj* pSdrOle2Obj = dynamic_cast< SdrOle2Obj* >(aSelection[i]);
        SdrGrafObj* pSdrGrafObj = dynamic_cast< SdrGrafObj* >(aSelection[i]);

        if(pSdrOle2Obj)
        {
            uno::Reference < embed::XEmbeddedObject > xObj = pSdrOle2Obj->GetObjRef();

            if( xObj.is() )
            {
                // TODO/LEAN: working with VisualArea can switch object to running state
                sal_Int64 nAspect = pSdrOle2Obj->GetAspect();
                basegfx::B2DVector aOleSize(0.0, 0.0);

                if ( nAspect == embed::Aspects::MSOLE_ICON )
                {
                    MapMode aMap100( MAP_100TH_MM );
                    const Size aSize(pSdrOle2Obj->GetOrigObjSize( &aMap100 ));

                    aOleSize.setX(aSize.Width());
                    aOleSize.setY(aSize.Height());
                    bOK = true;
                }
                else
                {
                    MapUnit aUnit = VCLUnoHelper::UnoEmbed2VCLMapUnit( xObj->getMapUnit( nAspect ) );
                    try
                    {
                        const awt::Size aSz(xObj->getVisualAreaSize( nAspect ));
                        const double fFactor(OutputDevice::GetFactorLogicToLogic(aUnit, MAP_100TH_MM));
                        aOleSize = basegfx::B2DVector(aSz.Width, aSz.Height) * fFactor;
                        bOK = true;
                    }
                    catch( embed::NoVisualAreaSizeException& )
                    {}
                }

                if ( bOK )
                {
                    const basegfx::B2DVector& rScale = pSdrOle2Obj->getSdrObjectScale();
                    const basegfx::B2DPoint& rTopLeft = pSdrOle2Obj->getSdrObjectTranslate();
                    basegfx::B2DHomMatrix aTransform;

                    pUndoGroup->AddAction( mpDoc->GetSdrUndoFactory().CreateUndoGeoObject( *pSdrOle2Obj ) );

                    aTransform.translate(-rTopLeft);
                    aTransform.scale(
                        aOleSize.getX() / (basegfx::fTools::equalZero(rScale.getX()) ? 1.0 : rScale.getX()),
                        aOleSize.getY() / (basegfx::fTools::equalZero(rScale.getY()) ? 1.0 : rScale.getY()));
                    aTransform.translate(rTopLeft);

                    sdr::legacy::transformSdrObject(*pSdrOle2Obj, aTransform);
                }
            }
        }
        else if( pSdrGrafObj )
        {
            double fFactor(1.0);

            if ( MAP_PIXEL == pSdrGrafObj->GetGrafPrefMapMode().GetMapUnit() )
            {
                fFactor = OutputDevice::GetFactorLogicToLogic(MAP_PIXEL, MAP_100TH_MM);
            }
            else
            {
                fFactor = OutputDevice::GetFactorLogicToLogic(pSdrGrafObj->GetGrafPrefMapMode().GetMapUnit(), MAP_100TH_MM);
            }

            const basegfx::B2DVector aSize(
                pSdrGrafObj->GetGrafPrefSize().Width() * fFactor,
                pSdrGrafObj->GetGrafPrefSize().Height() * fFactor);

            pUndoGroup->AddAction( getSdrModelFromSdrView().GetSdrUndoFactory().CreateUndoGeoObject(*pSdrGrafObj));

            const basegfx::B2DVector& rScale = pSdrGrafObj->getSdrObjectScale();
            const basegfx::B2DPoint& rTopLeft = pSdrGrafObj->getSdrObjectTranslate();
            basegfx::B2DHomMatrix aTransform;

            aTransform.translate(-rTopLeft);
            aTransform.scale(
                aSize.getX() / (basegfx::fTools::equalZero(rScale.getX()) ? 1.0 : rScale.getX()),
                aSize.getY() / (basegfx::fTools::equalZero(rScale.getY()) ? 1.0 : rScale.getY()));
            aTransform.translate(rTopLeft);
            sdr::legacy::transformSdrObject(*pSdrGrafObj, aTransform);

            bOK = true;
        }
    }

    if( bOK )
    {
        pUndoGroup->SetComment( String(SdResId(STR_UNDO_ORIGINALSIZE)) );
        mpDocSh->GetUndoManager()->AddUndoAction(pUndoGroup);
    }
    else
    {
        delete pUndoGroup;
    }
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
                Rectangle aRect(sdr::legacy::GetLogicRect(*pObj));
                {
                    // TODO/LEAN: working with visual area can switch object to running state
                    Size aDrawSize = aRect.GetSize();
                    awt::Size aSz;

                    MapMode aMapMode( mpDoc->GetExchangeObjectUnit() );
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

bool View::IsMorphingAllowed() const
{
    const SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());
    bool bRet(false);

    if(2 == aSelection.size())
    {
        const SdrObject* pObj1 = aSelection[0];
        const SdrObject* pObj2 = aSelection[1];
        const SdrPathObj* pSdrPathObj = dynamic_cast< const SdrPathObj* >(pObj1);

        if(!pSdrPathObj || pSdrPathObj->isClosed()) // not a path or closed (not OBJ_LINE, OBJ_PLIN, OBJ_PATHLINE, OBJ_FREELINE or OBJ_PATHPLIN)
        {
            pSdrPathObj = dynamic_cast< const SdrPathObj* >(pObj2);

            if(!pSdrPathObj || pSdrPathObj->isClosed()) // not a path or closed (not OBJ_LINE, OBJ_PLIN, OBJ_PATHLINE, OBJ_FREELINE or OBJ_PATHPLIN)
            {
                const sal_uInt16 nKind1 = pObj1->GetObjIdentifier();
                const sal_uInt16 nKind2 = pObj2->GetObjIdentifier();

                if ( ( nKind1 != OBJ_TEXT && nKind2 != OBJ_TEXT ) &&
                     ( nKind1 != OBJ_TITLETEXT && nKind2 != OBJ_TITLETEXT ) &&
                     ( nKind1 != OBJ_OUTLINETEXT && nKind2 != OBJ_OUTLINETEXT ) &&
                     ( nKind1 != OBJ_GRUP && nKind2 != OBJ_GRUP ) &&
                     ( nKind1 != OBJ_MEASURE && nKind2 != OBJ_MEASURE ) &&
                     ( nKind1 != OBJ_EDGE && nKind2 != OBJ_EDGE ) &&
                     ( nKind1 != OBJ_GRAF && nKind2 != OBJ_GRAF ) &&
                     ( nKind1 != OBJ_OLE2 && nKind2 != OBJ_OLE2 ) &&
                     ( nKind1 != OBJ_CAPTION && nKind2 !=  OBJ_CAPTION ) &&
                     !dynamic_cast< const E3dObject* >(pObj1) && !dynamic_cast< const E3dObject* >(pObj2) )
                {
                    const XFillStyle eFillStyle1(((const XFillStyleItem&)pObj1->GetMergedItem(XATTR_FILLSTYLE)).GetValue());
                    const XFillStyle eFillStyle2(((const XFillStyleItem&)pObj2->GetMergedItem(XATTR_FILLSTYLE)).GetValue());

                    if( ( eFillStyle1 == XFILL_NONE || eFillStyle1 == XFILL_SOLID ) &&
                        ( eFillStyle2 == XFILL_NONE || eFillStyle2 == XFILL_SOLID ) )
                    {
                        bRet = true;
                    }
                }
            }
        }
    }

    return bRet;
}

/*************************************************************************
|*
|*
|*
\************************************************************************/

bool View::IsVectorizeAllowed() const
{
    const SdrGrafObj* pObj = dynamic_cast< const SdrGrafObj* >(getSelectedIfSingle());

    return (pObj && GRAPHIC_BITMAP == pObj->GetGraphicType() && !pObj->isEmbeddedSvg());
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
        SdPage* pPage = dynamic_cast< SdPage* >( pObj->getSdrPageFromSdrObject() );
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
        SdPage* pPage = dynamic_cast< SdPage* >( pObj->getSdrPageFromSdrObject() );
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

void View::AddCustomHdl(SdrHdlList& rTarget)
{
    maSmartTags.addCustomHandles(rTarget);
}

SdrViewContext View::GetContext() const
{
    SdrViewContext eContext = SDRCONTEXT_STANDARD;
    if( maSmartTags.getContext( eContext ) )
        return eContext;
    else
        return FmFormView::GetContext();
}

bool View::HasMarkablePoints() const
{
    if( maSmartTags.HasMarkablePoints() )
    {
        return true;
    }
    else
    {
        return FmFormView::HasMarkablePoints();
    }
}

sal_uInt32 View::GetMarkablePointCount() const
{
    sal_uInt32 nCount = FmFormView::GetMarkablePointCount();
    nCount += maSmartTags.GetMarkablePointCount();
    return nCount;
}

bool View::HasMarkedPoints() const
{
    if( maSmartTags.HasMarkedPoints() )
        return true;
    else
        return FmFormView::HasMarkedPoints();
}

sal_uInt32 View::GetMarkedPointCount() const
{
    sal_uInt32 nCount = FmFormView::GetMarkedPointCount();
    nCount += maSmartTags.GetMarkedPointCount();
    return nCount;
}

bool View::IsPointMarkable(const SdrHdl& rHdl) const
{
    if( maSmartTags.IsPointMarkable( rHdl ) )
        return true;
    else
        return FmFormView::IsPointMarkable( rHdl );
}

bool View::MarkPoint(SdrHdl& rHdl, bool bUnmark )
{
    if( maSmartTags.MarkPoint( rHdl, bUnmark ) )
        return true;
    else
        return FmFormView::MarkPoint( rHdl, bUnmark );
}

void View::MarkPoints(const basegfx::B2DRange* pRange, bool bUnmark)
{
    if( !maSmartTags.MarkPoints( pRange, bUnmark ) )
    {
        FmFormView::MarkPoints( pRange, bUnmark );
    }
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
    if( pOutliner && pTextObj && pTextObj->getSdrPageFromSdrObject() )
    {
        SdPage* pPage = static_cast< SdPage* >( pTextObj->getSdrPageFromSdrObject() );

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

bool View::ShouldToggleOn(
    const bool bBulletOnOffMode,
    const bool bNormalBullet)
{
    // If setting bullets/numbering by the dialog, always should toggle on.
    if (!bBulletOnOffMode)
    {
        return true;
    }

    if(!areSdrObjectsSelected())
    {
        return false;
    }

    bool bToggleOn(false);
    const SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());
    SdrOutliner* pOutliner = SdrMakeOutliner(OUTLINERMODE_TEXTOBJECT, &getSdrModelFromSdrView());

    for(sal_uInt32 nIndex(0); nIndex < aSelection.size() && !bToggleOn; nIndex++)
    {
        SdrTextObj* pTextObj = dynamic_cast< SdrTextObj* >(aSelection[nIndex]);

        if (!pTextObj || pTextObj->IsTextEditActive())
        {
            continue;
        }

        SdrTableObj* pTableObj = dynamic_cast< SdrTableObj* >(pTextObj);

        if(pTableObj)
        {
            CellPos aStart, aEnd;
            SvxTableController* pTableController = dynamic_cast< SvxTableController* >(getSelectionController().get());

            if (pTableController)
            {
                pTableController->getSelectedCells(aStart, aEnd);
            }
            else
            {
                aStart = pTableObj->getFirstCell();
                aEnd = pTableObj->getLastCell();
            }

            const sal_Int32 nColCount(pTableObj->getColumnCount());

            for (sal_Int32 nRow = aStart.mnRow; nRow <= aEnd.mnRow && !bToggleOn; nRow++)
            {
                for (sal_Int32 nCol = aStart.mnCol; nCol <= aEnd.mnCol && !bToggleOn; nCol++)
                {
                    sal_Int32 nIndex = nRow * nColCount + nCol;
                    SdrText* pText = pTableObj->getText(nIndex);
                    if (!pText || !pText->GetOutlinerParaObject())
                        continue;

                    pOutliner->SetText(*(pText->GetOutlinerParaObject()));
                    sal_Int16 nStatus = pOutliner->GetBulletsNumberingStatus();
                    bToggleOn = ((bNormalBullet && nStatus != 0) || (!bNormalBullet && nStatus != 1)) ? sal_True : bToggleOn;
                    pOutliner->Clear();
                }
            }
        }
        else
        {
            OutlinerParaObject* pParaObj = pTextObj->GetOutlinerParaObject();
            if (!pParaObj)
                continue;

            pOutliner->SetText(*pParaObj);
            sal_Int16 nStatus = pOutliner->GetBulletsNumberingStatus();
            bToggleOn = ((bNormalBullet && nStatus != 0) || (!bNormalBullet && nStatus != 1)) ? sal_True : bToggleOn;
            pOutliner->Clear();
        }
    }

    delete pOutliner;

    return bToggleOn;
}

void View::ChangeMarkedObjectsBulletsNumbering(
    const bool bToggle,
    const bool bHandleBullets,
    const SvxNumRule* pNumRule,
    const bool bSwitchOff )
{
    if(!areSdrObjectsSelected())
    {
        return;
    }

    Window* pWindow = dynamic_cast< Window* >(GetFirstOutputDevice());
    const bool bUndoEnabled = getSdrModelFromSdrView().IsUndoEnabled();
    SdrUndoGroup* pUndoGroup = bUndoEnabled ? new SdrUndoGroup(getSdrModelFromSdrView()) : 0;

    const bool bToggleOn =
        bSwitchOff
        ? false
        : ShouldToggleOn( bToggle, bHandleBullets );

    SdrOutliner* pOutliner = SdrMakeOutliner(OUTLINERMODE_TEXTOBJECT, &getSdrModelFromSdrView());
    OutlinerView* pOutlinerView = new OutlinerView(pOutliner, pWindow);
    const SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());

    for (sal_uInt32 nIndex = 0; nIndex < aSelection.size(); nIndex++)
    {
        SdrTextObj* pTextObj = dynamic_cast< SdrTextObj* >(aSelection[nIndex]);
        if (!pTextObj || pTextObj->IsTextEditActive())
            continue;
        SdrTableObj* pTableObj = dynamic_cast< SdrTableObj* >(pTextObj);
        if (pTableObj)
        {
            CellPos aStart, aEnd;
            SvxTableController* pTableController = dynamic_cast< SvxTableController* >(getSelectionController().get());
            if (pTableController)
            {
                pTableController->getSelectedCells(aStart, aEnd);
            }
            else
            {
                aStart = pTableObj->getFirstCell();
                aEnd = pTableObj->getLastCell();
            }
            sal_Int32 nColCount = pTableObj->getColumnCount();
            for (sal_Int32 nRow = aStart.mnRow; nRow <= aEnd.mnRow; nRow++)
            {
                for (sal_Int32 nCol = aStart.mnCol; nCol <= aEnd.mnCol; nCol++)
                {
                    sal_Int32 nIndex = nRow * nColCount + nCol;
                    SdrText* pText = pTableObj->getText(nIndex);
                    if (!pText || !pText->GetOutlinerParaObject())
                        continue;

                    pOutliner->SetText(*(pText->GetOutlinerParaObject()));
                    if (bUndoEnabled)
                    {
                        SdrUndoObjSetText* pTxtUndo = dynamic_cast< SdrUndoObjSetText* >(
                            getSdrModelFromSdrView().GetSdrUndoFactory().CreateUndoObjectSetText(*pTextObj, nIndex));
                        pUndoGroup->AddAction(pTxtUndo);
                    }
                    if ( !bToggleOn )
                    {
                        pOutlinerView->SwitchOffBulletsNumbering();
                    }
                    else
                    {
                        pOutlinerView->ApplyBulletsNumbering( bHandleBullets, pNumRule, bToggle );
                    }
                    sal_uInt32 nParaCount = pOutliner->GetParagraphCount();
                    pText->SetOutlinerParaObject(pOutliner->CreateParaObject(0, (sal_uInt16)nParaCount));
                    pOutliner->Clear();
                }
            }

            // broadcast if no change from AdjustTextFrameWidthAndHeight
            const basegfx::B2DHomMatrix aOld(pTextObj->getSdrObjectTransformation());

            pTextObj->AdjustTextFrameWidthAndHeight();

            if(aOld == pTextObj->getSdrObjectTransformation())
            {
                pTextObj->SetChanged();
                const SdrObjectChangeBroadcaster aSdrObjectChangeBroadcaster(*pTextObj);
            }
        }
        else
        {
            OutlinerParaObject* pParaObj = pTextObj->GetOutlinerParaObject();
            if (!pParaObj)
                continue;
            pOutliner->SetText(*pParaObj);
            if (bUndoEnabled)
            {
                SdrUndoObjSetText* pTxtUndo = dynamic_cast< SdrUndoObjSetText* >(
                    getSdrModelFromSdrView().GetSdrUndoFactory().CreateUndoObjectSetText(*pTextObj, 0));
                pUndoGroup->AddAction(pTxtUndo);
            }
            if ( !bToggleOn )
            {
                pOutlinerView->SwitchOffBulletsNumbering();
            }
            else
            {
                pOutlinerView->ApplyBulletsNumbering( bHandleBullets, pNumRule, bToggle );
            }
            sal_uInt32 nParaCount = pOutliner->GetParagraphCount();
            pTextObj->SetOutlinerParaObject(pOutliner->CreateParaObject(0, (sal_uInt16)nParaCount));
            pOutliner->Clear();
        }
    }

    if ( bUndoEnabled && pUndoGroup->GetActionCount() > 0 )
    {
        getSdrModelFromSdrView().BegUndo();
        getSdrModelFromSdrView().AddUndo(pUndoGroup);
        getSdrModelFromSdrView().EndUndo();
    }

    delete pOutliner;
    delete pOutlinerView;
}

} // end of namespace sd
