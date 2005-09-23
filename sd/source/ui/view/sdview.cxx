/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sdview.cxx,v $
 *
 *  $Revision: 1.43 $
 *
 *  last change: $Author: hr $ $Date: 2005-09-23 11:34:38 $
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

#ifndef _COM_SUN_STAR_EMBED_NOVISUALAREASIZEEXCEPTION_HPP_
#include <com/sun/star/embed/NoVisualAreaSizeException.hpp>
#endif

#include "View.hxx"

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

#pragma hdrstop

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
#include "graphpro.hxx"
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

#include "ViewShellBase.hxx"

using namespace com::sun::star;

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
    pDoc(pDrawDoc),
    pDocSh( pDrawDoc->GetDocSh() ),
    pViewSh(pViewShell),
    pDragSrcMarkList(NULL),
    nDragSrcPgNum(SDRPAGE_NOTFOUND),
    pDropMarkerObj(NULL),
    pDropMarker(NULL),
    pLockedRedraws(NULL),
    nLockRedrawSmph(0),
    nAction(DND_ACTION_NONE),
    bIsDropAllowed(TRUE),
    mpClipboard (new ViewClipboard (*this))
{
    // #114898#
    SetBufferedOutputAllowed(sal_True);

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
    aDropErrorTimer.SetTimeoutHdl( LINK(this, View, DropErrorHdl) );
    aDropErrorTimer.SetTimeout(50);
    aDropInsertFileTimer.SetTimeoutHdl( LINK(this, View, DropInsertFileHdl) );
    aDropInsertFileTimer.SetTimeout(50);
}

/*************************************************************************
|*
|* Dtor
|*
\************************************************************************/

View::~View()
{
    // release content of selection clipboard, if we own the content
    UpdateSelectionClipboard( TRUE );

    aDropErrorTimer.Stop();
    aDropInsertFileTimer.Stop();

    delete pDropMarker;

    while (GetWin(0))
    {
        // Alle angemeldeten OutDevs entfernen
        DelWin(GetWin(0));
    }

    // gespeicherte Redraws loeschen
    if (pLockedRedraws)
    {
        SdViewRedrawRec* pRec = (SdViewRedrawRec*)pLockedRedraws->First();
        while (pRec)
        {
            delete pRec;
            pRec = (SdViewRedrawRec*)pLockedRedraws->Next();
        }
        delete pLockedRedraws;
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
    SdrObject* pObject = rOriginal.GetViewContact().TryToGetSdrObject();

    if(pObject)
    {
        OutputDevice* pOutDev = rDisplayInfo.GetOutputDevice();
        UINT16 nID = pObject->GetObjIdentifier();

        if( (pObject->GetPage() == NULL) || !pObject->GetPage()->checkVisibility( rOriginal, rDisplayInfo, true ) )
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

            if( bPaintOutline && !bIsPrinting )
            {
                SdrTextObj* pTextObj = (SdrTextObj*)pObject;

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

                    const GeoStat& aGeo = pTextObj->GetGeoStat();
                    const Rectangle &aRect = pTextObj->GetGeoRect();

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
                        if( !bMasterObj && (pTextObj->GetPage()->IsMasterPage()) )
                        {
                            String aOut;
                            switch( eKind )
                            {
                                case PRESOBJ_TITLE:
                                {
                                    static String aTitleAreaStr( SdResId( STR_PLACEHOLDER_DESCRIPTION_TITLE ) );
                                    if( ((SdPage*)pTextObj->GetPage())->GetPageKind() == PK_STANDARD)
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

                                    if( ((SdPage*)pTextObj->GetPage())->GetPageKind() == PK_STANDARD)
                                        aOut = aSlideAreaStr;
                                    else
                                        aOut = aNumberAreaStr;
                                    break;
                                }
                            }

                            if( aOut.Len() )
                            {
                                SdrTextVertAdjust eTVA = (SdrTextVertAdjust)((const SdrTextVertAdjustItem&)pTextObj->GetMergedItem(SDRATTR_TEXT_VERTADJUST)).GetValue();

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
    rOriginal.PaintObject(rDisplayInfo);
}

/*************************************************************************
|*
|* Paint-Methode: das Ereignis wird an die View weitergeleitet
|*
\************************************************************************/

void View::CompleteRedraw(OutputDevice* pOutDev, const Region& rReg, ::sdr::contact::ViewObjectContactRedirector* pRedirector /*=0L*/)
{
    // ausfuehren ??
    if (nLockRedrawSmph == 0)
    {
        SdrPageView* pPgView = GetPageViewPvNum(0);

        if (pPgView)
        {
            SdPage* pPage = (SdPage*) pPgView->GetPage();
            if( pPage )
            {
                SdrOutliner& rOutl=pDoc->GetDrawOutliner(NULL);
                rOutl.SetBackgroundColor( pPage->GetBackgroundColor(pPgView) );
            }
        }

        ViewRedirector aViewRedirector;
        FmFormView::CompleteRedraw(pOutDev, rReg, 0, pRedirector ? pRedirector : &aViewRedirector);

        USHORT nDemoKind =  SFX_APP()->GetDemoKind();

        if ( (nDemoKind == SFX_DEMOKIND_DEMO || nDemoKind == SFX_DEMOKIND_INVALID)
             && pOutDev->GetOutDevType() == OUTDEV_PRINTER )
        {
            SdrPageView* pPgView = GetPageViewPvNum(0);

            if (pPgView)
            {
                SdPage* pPage = (SdPage*) pPgView->GetPage();

                if (pPage)
                {
                    Rectangle aPageRect ( Point(0, 0), pPage->GetSize() );
                    SFX_APP()->SpoilDemoOutput(*pOutDev, aPageRect);
                }
            }
        }
    }
    // oder speichern?
    else
    {
        if (!pLockedRedraws)
            pLockedRedraws = new List;

        SdViewRedrawRec* pRec = new SdViewRedrawRec;
        pRec->pOut  = pOutDev;
        pRec->aRect = rReg.GetBoundRect();
        pLockedRedraws->Insert(pRec, LIST_APPEND);
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

    if (nDragSrcPgNum != SDRPAGE_NOTFOUND &&
        nDragSrcPgNum != GetPageViewPvNum(0)->GetPage()->GetPageNum())
    {
        // Es laeuft gerade Drag&Drop
        // Source- und Destination-Page unterschiedlich:
        // es wird die gemerkte MarkList verwendet
        pMarkList = pDragSrcMarkList;
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
    long nMarkMax = pMarkList->GetMarkCount() - 1;

    for (nMark = nMarkMax; (nMark >= 0) && !bSelected; nMark--)
    {
        // Rueckwaerts durch die Marklist
        pMark = pMarkList->GetMark(nMark);
        pObj = pMark->GetObj();

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

    if (pMarkList != pDragSrcMarkList)
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

BOOL View::BegTextEdit(SdrObject* pObj, SdrPageView* pPV, Window* pWin,
                         BOOL bIsNewObj, SdrOutliner* pGivenOutliner,
                         OutlinerView* pGivenOutlinerView, BOOL bDontDeleteOutliner,
                         BOOL bOnlyOneView, BOOL bGrabFocus)
{
    GetViewShell()->GetViewShellBase().GetEventMultiplexer().MultiplexEvent( sd::tools::EventMultiplexerEvent::EID_BEGIN_TEXT_EDIT, (void*)pObj );

    BOOL bReturn = FmFormView::BegTextEdit(pObj, pPV, pWin, bIsNewObj, pGivenOutliner,
                                        pGivenOutlinerView, bDontDeleteOutliner,
                                        bOnlyOneView, bGrabFocus);

    if (bReturn)
    {
        // UndoManager an der obersten Shell (SdDrawTextObjectBar) setzen
        ::Outliner* pOL = GetTextEditOutliner();

        if( pObj && pObj->GetPage() )
            pOL->SetBackgroundColor( pObj->GetPage()->GetBackgroundColor(pPV) );

        SfxUndoManager& rUndoMgr = pOL->GetUndoManager();
        rUndoMgr.Clear();
        ViewShell* pViewShell = pDocSh->GetViewShell();
        DBG_ASSERT(pViewShell, "ViewShell nicht gefunden");
        if (pViewShell)
        {
            pViewShell->GetViewFrame()->GetDispatcher()->
                GetShell(0)->SetUndoManager(&rUndoMgr);
        }

        pOL->SetParaInsertedHdl(LINK(this, View, ParagraphInsertedHdl));
        pOL->SetParaRemovingHdl(LINK(this, View, ParagraphRemovingHdl));
    }

    return(bReturn);
}

/*************************************************************************
|*
|* Texteingabe beenden
|*
\************************************************************************/

SdrEndTextEditKind View::EndTextEdit(BOOL bDontDeleteReally)
{
    return EndTextEdit(bDontDeleteReally, NULL );
}

SdrEndTextEditKind View::EndTextEdit(BOOL bDontDeleteReally, FuPoor* pFunc)
{
    SdrObject* pObj = GetTextEditObject();

    BOOL bIsTextEdit = IsTextEdit();

    SdrEndTextEditKind eKind;

    ViewShell* pViewShell= pDocSh->GetViewShell();

    if( pFunc == 0 )
    {
        if( pViewShell && pViewShell->ISA(DrawViewShell) )
        {
            pFunc = ( (DrawViewShell*) pViewShell)->GetActualFunction();

            if ( !pFunc || !pFunc->ISA(FuText) )
                pFunc = ( (DrawViewShell*) pViewShell)->GetOldFunction();
        }
    }

    // Replace the currently used undo manager by the one of the doc shell.
    // This is necessary because, in case of an active text editing, the
    // current undo manager belongs to the edit engine and will be destroyed
    // together with it when the text editing is stopped.
    if (bIsTextEdit)
    {
        // Get the undo manager from the doc shell and set it at the top
        // most shell on the shell stack (this makes it the active undo
        // manager.)
        SfxUndoManager* pUndoMgr = pDocSh->GetUndoManager();
        ViewShell* pViewShell = pDocSh->GetViewShell();
        DBG_ASSERT(pViewShell, "ViewShell not found");
        if (pViewShell != NULL)
        {
            pViewShell->GetViewFrame()->GetDispatcher()->GetShell(0)->SetUndoManager(pUndoMgr);
        }
    }

    if ( pFunc && pFunc->ISA(FuText) )
    {
        SdrTextObj* pTextObj = ( (FuText*) pFunc)->GetTextObj();
        BOOL bDefaultTextRestored = ( (FuText*) pFunc)->RestoreDefaultText();
        eKind = FmFormView::EndTextEdit(bDontDeleteReally);

        pTextObj = ( (FuText*) pFunc)->GetTextObj();

        if( pTextObj )
        {
            if( bDefaultTextRestored )
            {
                pTextObj->SetEmptyPresObj( TRUE );
            }
            else if( pTextObj->IsEmptyPresObj() && (pTextObj->GetEditOutlinerParaObject() == 0) )
            {
                pTextObj->SetEmptyPresObj( FALSE );
            }
        }

        if (eKind == SDRENDTEXTEDIT_CHANGED && !bDefaultTextRestored)
            ( (FuText*) pFunc)->ObjectChanged();

        // Tell the text function that the text object is not
        // edited anymore and must not be accessed.
        static_cast<FuText*>(pFunc)->TextEditingHasEnded(pTextObj);
    }
    else
        eKind = FmFormView::EndTextEdit(bDontDeleteReally);

    if( eKind != SDRENDTEXTEDIT_CHANGED )
        pObj = 0;

    GetViewShell()->GetViewShellBase().GetEventMultiplexer().MultiplexEvent( sd::tools::EventMultiplexerEvent::EID_END_TEXT_EDIT, (void*)pObj );

    if( pObj )
    {
        SdPage* pPage = dynamic_cast< SdPage* >( pObj->GetPage() );
        if( pPage )
            pPage->onEndTextEdit( pObj );
    }

    return(eKind);
}


/*************************************************************************
|*
|* Originalgroesse der markierten Objekte setzen
|*
\************************************************************************/

void View::SetMarkedOriginalSize()
{
    SdrUndoGroup*   pUndoGroup = new SdrUndoGroup(*pDoc);
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
                    MapUnit aUnit = VCLUnoHelper::UnoEmbed2VCLMapUnit( xObj->getMapUnit( ((SdrOle2Obj*)pObj)->GetAspect() ) );
                    try
                    {
                        awt::Size aSz = xObj->getVisualAreaSize( ((SdrOle2Obj*)pObj)->GetAspect() );
                        Size        aOleSize( OutputDevice::LogicToLogic( Size( aSz.Width, aSz.Height ), aUnit, MAP_100TH_MM) );
                        Rectangle   aDrawRect( pObj->GetLogicRect() );

                        pUndoGroup->AddAction( new SdrUndoGeoObj( *pObj ) );
                        pObj->Resize( aDrawRect.TopLeft(), Fraction( aOleSize.Width(), aDrawRect.GetWidth() ),
                                                           Fraction( aOleSize.Height(), aDrawRect.GetHeight() ) );
                        bOK = TRUE;
                    }
                    catch( embed::NoVisualAreaSizeException& )
                    {}
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

                pUndoGroup->AddAction( new SdrUndoGeoObj( *pObj ) );
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
        pDocSh->GetUndoManager()->AddUndoAction(pUndoGroup);
    }
    else
        delete pUndoGroup;
}

/*************************************************************************
|*
|* virt. Device mit Seiteninhalt erzeugen, Device gehoert dem Caller
|*
\************************************************************************/

VirtualDevice* View::CreatePageVDev(USHORT nSdPage, PageKind ePageKind,
                                      ULONG nWidthPixel)
{
    ViewShell*  pViewShell = pDocSh->GetViewShell();
    ::sd::Window* pWin = pViewShell->GetActiveWindow();
    VirtualDevice*  pVDev = new VirtualDevice( *pWin );
    MapMode         aMM( MAP_100TH_MM );

    SdPage* pPage = pDoc->GetSdPage(nSdPage, ePageKind);
    DBG_ASSERT(pPage, "Seite nicht gefunden");

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
    BOOL bAbort = !pVDev->SetOutputSize(aPageSize);
    DBG_ASSERT(!bAbort, "virt. Device nicht korrekt erzeugt");

    SdrView* pView = new SdrView(pDoc, pVDev);
    pView->SetPageVisible( FALSE );
    pView->SetBordVisible( FALSE );
    pView->SetGridVisible( FALSE );
    pView->SetHlplVisible( FALSE );
    pView->SetGlueVisible( FALSE );
    pView->ShowPage(pPage, Point(-pPage->GetLftBorder(), -pPage->GetUppBorder()));
    SdrPageView* pPageView  = pView->GetPageView(pPage);
    DBG_ASSERT(pViewShell, "ViewShell nicht gefunden");
    FrameView* pFrameView   = pViewShell->GetFrameView();
    pPageView->SetVisibleLayers( pFrameView->GetVisibleLayers() );
    pPageView->SetLockedLayers( pFrameView->GetLockedLayers() );
    pPageView->SetPrintableLayers( pFrameView->GetPrintableLayers() );

    /* #103186# following change undone:

      revision 1.168 (old cvs)
      date: 1996/02/27 17:57:10;  author: SB;  state: Exp;  lines: +14 -5
      CreatePageVDev(): Einschraenkung auf Seitengrenzen

      reason:

      > SdrPageView:
      > // rReg bezieht sich auf's OutDev, nicht auf die Page
      > void CompleteRedraw( ... );
    */

    pVDev->Push();  //SJ: i40609, the vdev mapmode seems to be dangled after CompleteRedraw,
                    //so we are pushing here, because the mapmode is used afterwards
    // temporary for gcc
    Point aPoint( 0, 0 );
    Region aRegion (Rectangle( aPoint, aPageSize ) );
    pView->CompleteRedraw(pVDev, aRegion);
    pVDev->Pop();
    delete pView;
    return pVDev;
}



/*************************************************************************
|*
|* OLE-Obj am Client connecten
|*
\************************************************************************/

void View::DoConnect(SdrOle2Obj* pObj)
{
    if (pViewSh)
    {
        uno::Reference < embed::XEmbeddedObject > xObj( pObj->GetObjRef() );
        if( xObj.is() )
        {
            ::sd::Window* pWindow = pViewSh->GetActiveWindow();
            SfxInPlaceClient* pSdClient = pViewSh-> GetViewShellBase().FindIPClient( xObj, pWindow );
            if ( !pSdClient )
            {
                pSdClient = new Client(pObj, pViewSh, pWindow);
                Rectangle aRect = pObj->GetLogicRect();
                {
                    // TODO/LEAN: working with visual area can switch object to running state
                    Size aDrawSize = aRect.GetSize();
                    awt::Size aSz;
                    try
                    {
                        aSz = xObj->getVisualAreaSize( pSdClient->GetAspect() );
                    }
                    catch( embed::NoVisualAreaSizeException& )
                    {
                        DBG_ASSERT( sal_False, "Can not get visual area size!\n");
                        aSz.Width = 5000;
                        aSz.Height = 5000;
                    }

                    Size aObjAreaSize( aSz.Width, aSz.Height );

                    MapUnit aMapUnit = VCLUnoHelper::UnoEmbed2VCLMapUnit( xObj->getMapUnit( pSdClient->GetAspect() ) );
                    aObjAreaSize = OutputDevice::LogicToLogic( aObjAreaSize,
                                                           aMapUnit,
                                                           pDoc->GetScaleUnit() );

                    // sichtbarer Ausschnitt wird nur inplace veraendert!
                    aRect.SetSize(aObjAreaSize);
                    pSdClient->SetObjArea(aRect);

                    Fraction aScaleWidth (aDrawSize.Width(),  aObjAreaSize.Width() );
                    Fraction aScaleHeight(aDrawSize.Height(), aObjAreaSize.Height() );
                    aScaleWidth.ReduceInaccurate(10);       // kompatibel zum SdrOle2Obj
                    aScaleHeight.ReduceInaccurate(10);
                    pSdClient->SetSizeScale(aScaleWidth, aScaleHeight);
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
        const SdrObject*    pObj1 = rMarkList.GetMark( 0 )->GetObj();
        const SdrObject*    pObj2 = rMarkList.GetMark( 1 )->GetObj();
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
            SfxItemSet      aSet1( pDoc->GetPool(), XATTR_FILLSTYLE, XATTR_FILLSTYLE );
            SfxItemSet      aSet2( pDoc->GetPool(), XATTR_FILLSTYLE, XATTR_FILLSTYLE );

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
        const SdrObject* pObj = rMarkList.GetMark( 0 )->GetObj();

        if( pObj->ISA( SdrGrafObj ) && ( (SdrGrafObj*) pObj )->GetGraphicType() == GRAPHIC_BITMAP )
            bRet = TRUE;
    }

    return bRet;
}

void View::onAccessibilityOptionsChanged()
{
    if( pViewSh )
    {
        ::sd::Window* pWindow = pViewSh->GetActiveWindow();
        if( pWindow )
        {
            const StyleSettings& rStyleSettings = pWindow->GetSettings().GetStyleSettings();

            USHORT nOutputSlot, nPreviewSlot;

            SvtAccessibilityOptions& aAccOptions = getAccessibilityOptions();

            if( pViewSh->GetViewFrame() && pViewSh->GetViewFrame()->GetDispatcher() )
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

                pViewSh->GetViewFrame()->GetDispatcher()->Execute( nOutputSlot, SFX_CALLMODE_ASYNCHRON );
                pViewSh->GetViewFrame()->GetDispatcher()->Execute( nPreviewSlot, SFX_CALLMODE_ASYNCHRON );
            }

            pViewSh->Invalidate();
        }
    }
}

IMPL_LINK( View, ParagraphInsertedHdl, ::Outliner *, pOutliner )
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

IMPL_LINK( View, ParagraphRemovingHdl, ::Outliner *, pOutliner )
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


} // end of namespace sd
