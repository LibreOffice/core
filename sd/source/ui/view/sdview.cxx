/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sdview.cxx,v $
 *
 *  $Revision: 1.62 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 11:59:12 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#ifndef _COM_SUN_STAR_EMBED_NOVISUALAREASIZEEXCEPTION_HPP_
#include <com/sun/star/embed/NoVisualAreaSizeException.hpp>
#endif
#ifndef _COM_SUN_STAR_LINGUISTIC2_XSPELLCHECKER1_HPP_
#include <com/sun/star/linguistic2/XSpellChecker1.hpp>
#endif

#include "View.hxx"

#ifndef _UNO_LINGU_HXX
#include <svx/unolingu.hxx>
#endif

#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifndef _E3D_OBJ3D_HXX
#include <svx/obj3d.hxx>
#endif
#ifndef _SVX_FMVIEW_HXX
#include <svx/fmview.hxx>
#endif
#ifndef _OUTLINER_HXX //autogen
#include <svx/outliner.hxx>
#endif
#ifndef _SVX_SVXIDS_HRC
#include <svx/svxids.hrc>
#endif
#ifndef _SVDOGRAF_HXX
#include <svx/svdograf.hxx>
#endif
#ifndef _SVDOOLE2_HXX
#include <svx/svdoole2.hxx>
#endif
#ifndef _SVDUNDO_HXX
#include <svx/svdundo.hxx>
#endif
#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif
#ifndef _SVDPAGV_HXX //autogen
#include <svx/svdpagv.hxx>
#endif
#ifndef _SFXDOCFILE_HXX //autogen
#include <sfx2/docfile.hxx>
#endif
#ifndef _SVDOUTL_HXX //autogen
#include <svx/svdoutl.hxx>
#endif
#ifndef _SDR_CONTACT_DISPLAYINFO_HXX
#include <svx/sdr/contact/displayinfo.hxx>
#endif

#include <svx/svdetc.hxx>
#include <svx/editstat.hxx>

#include <svx/dialogs.hrc>
#include <sfx2/viewfrm.hxx>
#include <svx/xoutx.hxx>
#include <svx/svdopage.hxx>
#include <toolkit/helper/vclunohelper.hxx>

#ifndef _SVX_XLNDSIT_HXX
#include <svx/xlndsit.hxx>
#endif
#ifndef _SVX_XLINEIT0_HXX
#include <svx/xlineit0.hxx>
#endif
#ifndef _SVX_XLNCLIT_HXX
#include <svx/xlnclit.hxx>
#endif

#ifndef _SV_VIRDEV_HXX
#include <vcl/virdev.hxx>
#endif

#include "app.hrc"
#include "strings.hrc"
#ifndef SD_WINDOW_HXX
#include "Window.hxx"
#endif
#ifndef SD_CLIENT_HXX
#include "Client.hxx"
#endif
#include "drawdoc.hxx"
#include "DrawDocShell.hxx"
#include "app.hxx"
#include "sdpage.hxx"
#include "glob.hrc"
#include "sdresid.hxx"
#ifndef SD_DRAW_VIEW_SHELL_HXX
#include "DrawViewShell.hxx"
#endif
#ifndef SD_FU_TEXT_HXX
#include "futext.hxx"
#endif
#ifndef SD_FU_INSERT_FILE_HXX
#include "fuinsfil.hxx"
#endif
#ifndef _SD_SLIDESHOW_HXX
#include "slideshow.hxx"
#endif
#include "stlpool.hxx"
#ifndef SD_FRAME_VIEW_HXX
#include "FrameView.hxx"
#endif
#include "ViewClipboard.hxx"
#include "undo/undomanager.hxx"

#ifndef _SDR_CONTACT_VIEWOBJECTCONTACT_HXX
#include <svx/sdr/contact/viewobjectcontact.hxx>
#endif

#ifndef _SDR_CONTACT_VIEWCONTACT_HXX
#include <svx/sdr/contact/viewcontact.hxx>
#endif

#ifndef _SDR_CONTACT_DISPLAYINFO_HXX
#include <svx/sdr/contact/displayinfo.hxx>
#endif
#ifndef SD_TOOLS_EVENT_MULTIPLEXER_HXX
#include "EventMultiplexer.hxx"
#endif
#ifndef SD_VIEW_SHELL_BASE_HXX
#include "ViewShellBase.hxx"
#endif
#ifndef SD_VIEW_SHELL_MANAGER_HXX
#include "ViewShellManager.hxx"
#endif

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
    mbIsDropAllowed(TRUE),
    maSmartTags(*this),
    mpClipboard (new ViewClipboard (*this))
{
    // #i73602# Use default from the configuration
    SetBufferedOverlayAllowed(getOptionsDrawinglayer().IsOverlayBuffer_DrawImpress());

    // #i74769#, #i75172# Use default from the configuration
    SetBufferedOutputAllowed(getOptionsDrawinglayer().IsPaintBuffer_DrawImpress());

    EnableExtendedKeyInputDispatcher(FALSE);
    EnableExtendedMouseEventDispatcher(FALSE);
    EnableExtendedCommandEventDispatcher(FALSE);

    SetUseIncompatiblePathCreateInterface(FALSE);
    SetMarkHdlWhenTextEdit(TRUE);
    EnableTextEditOnObjectsWithoutTextIfTextTool(TRUE);

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
    UpdateSelectionClipboard( TRUE );

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
    virtual void PaintObject(::sdr::contact::ViewObjectContact& rOriginal, ::sdr::contact::DisplayInfo& rDisplayInfo);
};

ViewRedirector::ViewRedirector()
{
}

ViewRedirector::~ViewRedirector()
{
}

// all default implementations just call the same methods at the original. To do something
// different, overload the method and at least do what the method does.
void ViewRedirector::PaintObject(::sdr::contact::ViewObjectContact& rOriginal, ::sdr::contact::DisplayInfo& rDisplayInfo)
{
    bool bDoPaintObject = true;

    SdrObject* pObject = rOriginal.GetViewContact().TryToGetSdrObject();

    if(pObject && pObject->GetPage() )
    {
        OutputDevice* pOutDev = rDisplayInfo.GetOutputDevice();

        bDoPaintObject = pObject->GetPage()->checkVisibility( rOriginal, rDisplayInfo, true );

        if(!bDoPaintObject && !(( pObject->GetObjInventor() == SdrInventor ) && ( pObject->GetObjIdentifier() == OBJ_PAGE )) )
            return;

        bool bPaintOutline = false;
        PresObjKind eKind = PRESOBJ_NONE;
        const bool bMasterObj(rDisplayInfo.GetMasterPagePainting());
        const bool bIsPrinting(rDisplayInfo.OutputToPrinter());

        // check if we need to draw a placeholder border
        if(rDisplayInfo.GetProcessedPage() == rDisplayInfo.GetPageView()->GetPage())
        {
            // we never draw for objects inside SdrPageObj
            if( pObject->IsEmptyPresObj() && pObject->ISA(SdrTextObj) )
            {
                if( !bMasterObj || !pObject->IsNotVisibleAsMaster() )
                {
                    SdPage* pPage = (SdPage*)pObject->GetPage();
                    eKind = pPage ? pPage->GetPresObjKind(pObject) : PRESOBJ_NONE;

                    if( eKind != PRESOBJ_BACKGROUND )
                        bPaintOutline = true;
                }
            }
            else if( ( pObject->GetObjInventor() == SdrInventor ) && ( pObject->GetObjIdentifier() == OBJ_TEXT ) )
            {
                SdPage* pPage = (SdPage*)pObject->GetPage();
                if( pPage )
                {
                    eKind = pPage->GetPresObjKind(pObject);

                    if((eKind == PRESOBJ_FOOTER) || (eKind == PRESOBJ_HEADER) || (eKind == PRESOBJ_DATETIME) || (eKind == PRESOBJ_SLIDENUMBER) )
                    {
                        if( !bMasterObj )
                        {
                            // only draw a boundary for header&footer objects on the masterpage itself
                            bPaintOutline = true;
                        }
                    }
                }
            }
            else if( ( pObject->GetObjInventor() == SdrInventor ) && ( pObject->GetObjIdentifier() == OBJ_PAGE ) )
            {
                if( !bIsPrinting )
                    bPaintOutline = true;
            }

            if( bPaintOutline && !bIsPrinting )
            {
//              SdrTextObj* pTextObj = (SdrTextObj*)pObject;

                // leere Praesentationsobjekte bekommen einen grauen Rahmen
                svtools::ColorConfig aColorConfig;
                svtools::ColorConfigValue aColor( aColorConfig.GetColorValue( svtools::OBJECTBOUNDARIES ) );

                if( aColor.bIsVisible )
                {
                    pOutDev->Push();
                    pOutDev->SetFillColor();
                    pOutDev->SetLineColor( aColor.nColor );

                    {
                        XDash aDash( XDASH_RECT, 1, 80, 1, 80, 80);
                        SfxItemSet aSet( pObject->GetModel()->GetItemPool() );
                        String aEmpty;
                        aSet.Put( XLineDashItem( aEmpty, aDash ) );
                        aSet.Put( XLineStyleItem( XLINE_DASH ) );
                        aSet.Put( XLineColorItem(aEmpty,Color(aColor.nColor)) );
                        aSet.Put( XFillStyleItem( XFILL_NONE ) );
                        rDisplayInfo.GetExtendedOutputDevice()->SetLineAttr(aSet);
                        rDisplayInfo.GetExtendedOutputDevice()->SetFillAttr(aSet);
                    }
                    pOutDev->Pop();

                    Rectangle aRect;
                    GeoStat aGeo;

                    SdrTextObj* pTextObj = dynamic_cast< SdrTextObj* >( pObject );
                    if( pTextObj )
                    {
                        aGeo = pTextObj->GetGeoStat();
                        aRect = pTextObj->GetGeoRect();
                    }
                    else
                    {
                        aRect = pObject->GetCurrentBoundRect();
                    }

                    if( aGeo.nDrehWink!=0 || aGeo.nShearWink!=0 )
                    {
                        Polygon aPoly(aRect);
                        if(aGeo.nShearWink!=0)
                            ShearPoly(aPoly,aRect.TopLeft(),aGeo.nTan);

                        if(aGeo.nDrehWink!=0)
                            RotatePoly(aPoly,aRect.TopLeft(),aGeo.nSin,aGeo.nCos);

                        rDisplayInfo.GetExtendedOutputDevice()->DrawPolyLine(aPoly);
                    }
                    else
                    {
                        rDisplayInfo.GetExtendedOutputDevice()->DrawRect(aRect);

                        // now paint the placeholder description, but only on the masterpage
                        if( !bMasterObj && (pObject->GetPage()->IsMasterPage()) )
                        {
                            String aOut;
                            switch( eKind )
                            {
                                case PRESOBJ_TITLE:
                                {
                                    static String aTitleAreaStr( SdResId( STR_PLACEHOLDER_DESCRIPTION_TITLE ) );
                                    if( ((SdPage*)pObject->GetPage())->GetPageKind() == PK_STANDARD)
                                        aOut = aTitleAreaStr;
                                    break;
                                }
                                case PRESOBJ_OUTLINE:
                                {
                                    static String aOutlineAreaStr( SdResId( STR_PLACEHOLDER_DESCRIPTION_OUTLINE ) );
                                    aOut = aOutlineAreaStr;
                                    break;
                                }
                                case PRESOBJ_FOOTER:
                                {
                                    static String aFooterAreaStr( SdResId( STR_PLACEHOLDER_DESCRIPTION_FOOTER ) );
                                    aOut = aFooterAreaStr;
                                    break;
                                }
                                case PRESOBJ_HEADER:
                                {
                                    static String aHeaderAreaStr( SdResId( STR_PLACEHOLDER_DESCRIPTION_HEADER ) );
                                    aOut = aHeaderAreaStr;
                                    break;
                                }
                                case PRESOBJ_DATETIME:
                                {
                                    static String aDateTimeStr( SdResId( STR_PLACEHOLDER_DESCRIPTION_DATETIME ) );
                                    aOut = aDateTimeStr;
                                    break;
                                }
                                case PRESOBJ_NOTES:
                                {
                                    static String aDateTimeStr( SdResId( STR_PLACEHOLDER_DESCRIPTION_NOTES ) );
                                    aOut = aDateTimeStr;
                                    break;
                                }
                                case PRESOBJ_SLIDENUMBER:
                                {
                                    static String aNumberAreaStr( SdResId( STR_PLACEHOLDER_DESCRIPTION_NUMBER ) );
                                    static String aSlideAreaStr( SdResId( STR_PLACEHOLDER_DESCRIPTION_SLIDE ) );

                                    if( ((SdPage*)pObject->GetPage())->GetPageKind() == PK_STANDARD)
                                        aOut = aSlideAreaStr;
                                    else
                                        aOut = aNumberAreaStr;
                                    break;
                                }
                                default:
                                    break;
                            }

                            if( aOut.Len() )
                            {
                                SdrTextVertAdjust eTVA = (SdrTextVertAdjust)((const SdrTextVertAdjustItem&)pObject->GetMergedItem(SDRATTR_TEXT_VERTADJUST)).GetValue();

                                pOutDev->Push();

                                Font aFont;
                                aFont.SetHeight( 500 );
                                aFont.SetAlign( (eTVA == SDRTEXTVERTADJUST_BOTTOM) ? ALIGN_TOP : ALIGN_BOTTOM );
                                pOutDev->SetFont( aFont );
                                pOutDev->SetTextColor( Color(aColor.nColor) );
                                pOutDev->SetBackground();

                                Point aPos;
                                if(eTVA == SDRTEXTVERTADJUST_BOTTOM)
                                    aPos = aRect.TopRight();
                                else
                                    aPos = aRect.BottomRight();

                                aPos.X() -= pOutDev->GetTextWidth( aOut );

                                pOutDev->DrawText( aPos, aOut );
                                pOutDev->Pop();
                            }
                        }
                    }
                }
            }
        }
    }

    // draw object in any case
    if( bDoPaintObject )
        rOriginal.PaintObject(rDisplayInfo);
}

/*************************************************************************
|*
|* Paint-Methode: das Ereignis wird an die View weitergeleitet
|*
\************************************************************************/

void View::CompleteRedraw(OutputDevice* pOutDev, const Region& rReg, USHORT nPaintMode, ::sdr::contact::ViewObjectContactRedirector* pRedirector /*=0L*/)
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
        FmFormView::CompleteRedraw(pOutDev, rReg, nPaintMode, pRedirector ? pRedirector : &aViewRedirector);
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

BOOL View::SetAttributes(const SfxItemSet& rSet, BOOL bReplaceAll)
{
    BOOL bOk = FmFormView::SetAttributes(rSet, bReplaceAll);
    return (bOk);
}


/*************************************************************************
|*
|* Attribute holen
|*
\************************************************************************/

BOOL View::GetAttributes( SfxItemSet& rTargetSet, BOOL bOnlyHardAttr ) const
{
    return( FmFormView::GetAttributes( rTargetSet, bOnlyHardAttr ) );
}


/*************************************************************************
|*
|* Ist ein Praesentationsobjekt selektiert?
|*
\************************************************************************/

BOOL View::IsPresObjSelected(BOOL bOnPage, BOOL bOnMasterPage, BOOL bCheckPresObjListOnly, BOOL bCheckLayoutOnly) const
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

    BOOL bSelected = FALSE;
    BOOL bMasterPage = FALSE;
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

            if (bMasterPage && bOnMasterPage || !bMasterPage && bOnPage)
            {
                if ( pPage && pPage->IsPresObj(pObj) )
                {
                    if( bCheckLayoutOnly )
                    {
                        PresObjKind eKind = pPage->GetPresObjKind(pObj);

                        if((eKind != PRESOBJ_FOOTER) && (eKind != PRESOBJ_HEADER) && (eKind != PRESOBJ_DATETIME) && (eKind != PRESOBJ_SLIDENUMBER) )
                            bSelected = TRUE;
                    }
                    else
                    {
                        bSelected = TRUE;
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
        pOLV->SelectRange( 0, (USHORT) pOutliner->GetParagraphCount() );
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

BOOL View::SetStyleSheet(SfxStyleSheet* pStyleSheet, BOOL bDontRemoveHardAttr)
{
    // weiter an SdrView
    return FmFormView::SetStyleSheet(pStyleSheet, bDontRemoveHardAttr);
}


/*************************************************************************
|*
|* Texteingabe beginnen
|*
\************************************************************************/

static void SetSpellOptions( SdDrawDocument* pDoc, ULONG& rCntrl )
{
    BOOL bHideSpell = pDoc->GetHideSpell();
    BOOL bOnlineSpell = pDoc->GetOnlineSpell();

    if( bHideSpell )
        rCntrl |= EE_CNTRL_NOREDLINES;
    else
        rCntrl &= ~EE_CNTRL_NOREDLINES;

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
    GetViewShell()->GetViewShellBase().GetEventMultiplexer().MultiplexEvent( sd::tools::EventMultiplexerEvent::EID_BEGIN_TEXT_EDIT, (void*)pObj );

    if( pOutl==NULL && pObj )
        pOutl = SdrMakeOutliner( OUTLINERMODE_TEXTOBJECT, pObj->GetModel() );

    // make draw&impress specific initialisations
    if( pOutl )
    {
        pOutl->SetMinDepth(0);
        pOutl->SetStyleSheetPool((SfxStyleSheetPool*) mpDoc->GetStyleSheetPool());
        pOutl->SetCalcFieldValueHdl(LINK(SD_MOD(), SdModule, CalcFieldValueHdl));
        ULONG nCntrl = pOutl->GetControlWord();
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

        // in einem Gliederungstext darf nicht auf die 0-te
        // Ebene ausgerueckt werden
        if (pObj->GetObjInventor() == SdrInventor && pObj->GetObjIdentifier() == OBJ_OUTLINETEXT)
            pOutl->SetMinDepth(1);
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
                aBackground = ImpGetTextEditBackgroundColor();
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
SdrEndTextEditKind View::SdrEndTextEdit(BOOL bDontDeleteReally )
{
    SdrObjectWeakRef xObj( GetTextEditObject() );

    BOOL bDefaultTextRestored = RestoreDefaultText( dynamic_cast< SdrTextObj* >( GetTextEditObject() ) );

    SdrEndTextEditKind eKind = FmFormView::SdrEndTextEdit(bDontDeleteReally);

    if( bDefaultTextRestored )
    {
        if( xObj.is() && !xObj->IsEmptyPresObj() )
        {
            xObj->SetEmptyPresObj( TRUE );
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
            pObj->SetEmptyPresObj( FALSE );
    }

    GetViewShell()->GetViewShellBase().GetEventMultiplexer().MultiplexEvent( sd::tools::EventMultiplexerEvent::EID_END_TEXT_EDIT, (void*)xObj.get() );

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
    ULONG           nCount = GetMarkedObjectCount();
    BOOL            bOK = FALSE;

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
                        bOK = TRUE;
                    }
                    else
                    {
                        MapUnit aUnit = VCLUnoHelper::UnoEmbed2VCLMapUnit( xObj->getMapUnit( nAspect ) );
                        try
                        {
                            awt::Size aSz = xObj->getVisualAreaSize( nAspect );
                            aOleSize = OutputDevice::LogicToLogic( Size( aSz.Width, aSz.Height ), aUnit, MAP_100TH_MM );
                            bOK = TRUE;
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

                bOK = TRUE;
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

/** create a virtual device and paints the slide contents into it.
    The caller must delete the returned device */
VirtualDevice* View::CreatePageVDev(USHORT nSdPage, PageKind ePageKind, ULONG nWidthPixel)
{
    ViewShell*  pViewShell = mpDocSh->GetViewShell();
    OutputDevice* pRefDevice = 0;
    if( pViewShell )
        pRefDevice = pViewShell->GetActiveWindow();

    if( !pRefDevice )
        pRefDevice = Application::GetDefaultDevice();

    DBG_ASSERT( pRefDevice, "sd::View::CreatePageVDev(), I need a reference device to work properly!" );

    VirtualDevice* pVDev;
    if( pRefDevice )
        pVDev = new VirtualDevice( *pRefDevice );
    else
        pVDev = new VirtualDevice();

    MapMode         aMM( MAP_100TH_MM );

    SdPage* pPage = mpDoc->GetSdPage(nSdPage, ePageKind);
    DBG_ASSERT(pPage, "sd::View::CreatePageVDev(), slide not found!");

    if( pPage )
    {
        Size aPageSize(pPage->GetSize());
        aPageSize.Width()  -= pPage->GetLftBorder();
        aPageSize.Width()  -= pPage->GetRgtBorder();
        aPageSize.Height() -= pPage->GetUppBorder();
        aPageSize.Height() -= pPage->GetLwrBorder();

        // use scaling?
        if( nWidthPixel )
        {
            const Fraction aFrac( (long) nWidthPixel, pVDev->LogicToPixel( aPageSize, aMM ).Width() );

            aMM.SetScaleX( aFrac );
            aMM.SetScaleY( aFrac );
        }

        pVDev->SetMapMode( aMM );
        if( pVDev->SetOutputSize(aPageSize) )
        {
            std::auto_ptr< SdrView > pView( new SdrView(mpDoc, pVDev) );
            pView->SetPageVisible( FALSE );
            pView->SetBordVisible( FALSE );
            pView->SetGridVisible( FALSE );
            pView->SetHlplVisible( FALSE );
            pView->SetGlueVisible( FALSE );
            pView->ShowSdrPage(pPage); // WAITING FOR SJ , Point(-pPage->GetLftBorder(), -pPage->GetUppBorder()));
            SdrPageView* pPageView  = pView->GetSdrPageView();
            if( pViewShell )
            {
                FrameView* pFrameView   = pViewShell->GetFrameView();
                if( pFrameView )
                {
                    pPageView->SetVisibleLayers( pFrameView->GetVisibleLayers() );
                    pPageView->SetLockedLayers( pFrameView->GetLockedLayers() );
                    pPageView->SetPrintableLayers( pFrameView->GetPrintableLayers() );
                }
            }

            // SJ: i40609, the vdev mapmode seems to be dangled after CompleteRedraw,
            // so we are pushing here, because the mapmode is used afterwards
            pVDev->Push();

            Point aPoint( 0, 0 );
            Region aRegion (Rectangle( aPoint, aPageSize ) );
            pView->CompleteRedraw(pVDev, aRegion);
            pVDev->Pop();
        }
        else
        {
            DBG_ERROR("sd::View::CreatePageVDev(), virt. device creation failed!");
        }
    }
    return pVDev;
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

BOOL View::IsMorphingAllowed() const
{
    const SdrMarkList&  rMarkList = GetMarkedObjectList();
    BOOL                bRet = FALSE;

    if ( rMarkList.GetMarkCount() == 2 )
    {
        const SdrObject*    pObj1 = rMarkList.GetMark( 0 )->GetMarkedSdrObj();
        const SdrObject*    pObj2 = rMarkList.GetMark( 1 )->GetMarkedSdrObj();
        const UINT16        nKind1 = pObj1->GetObjIdentifier();
        const UINT16        nKind2 = pObj2->GetObjIdentifier();

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
                bRet = TRUE;
        }
    }

    return bRet;
}

/*************************************************************************
|*
|*
|*
\************************************************************************/

BOOL View::IsVectorizeAllowed() const
{
    const SdrMarkList&  rMarkList = GetMarkedObjectList();
    BOOL                bRet = FALSE;

    if( rMarkList.GetMarkCount() == 1 )
    {
        const SdrObject* pObj = rMarkList.GetMark( 0 )->GetMarkedSdrObj();

        if( pObj->ISA( SdrGrafObj ) && ( (SdrGrafObj*) pObj )->GetGraphicType() == GRAPHIC_BITMAP )
            bRet = TRUE;
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

            USHORT nOutputSlot, nPreviewSlot;

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
    sd::UndoManager* pUndoManager = mpDoc ? mpDoc->GetUndoManager() : 0;
    return pUndoManager && pUndoManager->isInListAction();
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

BOOL View::HasMarkablePoints() const
{
    if( maSmartTags.HasMarkablePoints() )
        return true;
    else
        return FmFormView::HasMarkablePoints();
}

ULONG View::GetMarkablePointCount() const
{
    ULONG nCount = FmFormView::GetMarkablePointCount();
    nCount += maSmartTags.GetMarkablePointCount();
    return nCount;
}

BOOL View::HasMarkedPoints() const
{
    if( maSmartTags.HasMarkedPoints() )
        return true;
    else
        return FmFormView::HasMarkedPoints();
}

ULONG View::GetMarkedPointCount() const
{
    ULONG nCount = FmFormView::GetMarkedPointCount();
    nCount += maSmartTags.GetMarkedPointCount();
    return nCount;
}

BOOL View::IsPointMarkable(const SdrHdl& rHdl) const
{
    if( maSmartTags.IsPointMarkable( rHdl ) )
        return true;
    else
        return FmFormView::IsPointMarkable( rHdl );
}

BOOL View::MarkPoint(SdrHdl& rHdl, BOOL bUnmark )
{
    if( maSmartTags.MarkPoint( rHdl, bUnmark ) )
        return true;
    else
        return FmFormView::MarkPoint( rHdl, bUnmark );
}

BOOL View::MarkPoints(const Rectangle* pRect, BOOL bUnmark)
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

} // end of namespace sd
