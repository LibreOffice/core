/*************************************************************************
 *
 *  $RCSfile: drviews1.cxx,v $
 *
 *  $Revision: 1.40 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 14:54:26 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#include <so3/iface.hxx>

#ifndef _SVXIDS_HRC
#include <svx/svxids.hrc>
#endif
#ifndef _SVDPAGV_HXX //autogen
#include <svx/svdpagv.hxx>
#endif
#ifndef _SFXVIEWFRM_HXX //autogen
#include <sfx2/viewfrm.hxx>
#endif
#ifndef _SFX_BINDINGS_HXX //autogen
#include <sfx2/bindings.hxx>
#endif
#ifndef _SVDOOLE2_HXX //autogen
#include <svx/svdoole2.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SV_SCRBAR_HXX //autogen
#include <vcl/scrbar.hxx>
#endif
#ifndef _SVDOGRAF_HXX //autogen
#include <svx/svdograf.hxx>
#endif
#ifndef _SVDOPAGE_HXX
#include <svx/svdopage.hxx>
#endif
#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SVSTOR_HXX //autogen
#include <so3/svstor.hxx>
#endif
#ifndef _SVX_FMSHELL_HXX //autogen
#include <svx/fmshell.hxx>
#endif
#ifndef _E3D_GLOBL3D_HXX
#include <svx/globl3d.hxx>
#endif
#ifndef _SVX_FMGLOB_HXX
#include <svx/fmglob.hxx>
#endif
#ifndef _OUTLINER_HXX //autogen
#include <svx/outliner.hxx>
#endif

#pragma hdrstop

#include "misc.hxx"

#ifdef STARIMAGE_AVAILABLE
#ifndef _SIMDLL_HXX
#include <sim2/simdll.hxx>
#endif
#endif

#include "glob.hrc"
#include "app.hrc"
#include "res_bmp.hrc"
#include "strings.hrc"
#include "helpids.h"

#include "app.hxx"
#ifndef SD_FU_POOR_HXX
#include "fupoor.hxx"
#endif
#include "sdresid.hxx"
#ifndef SD_FU_SELECTION_HXX
#include "fusel.hxx"
#endif
#include "sdpage.hxx"
#ifndef SD_FRAME_VIEW_HXX
#include "FrameView.hxx"
#endif
#include "stlpool.hxx"
#ifndef SD_WINDOW_HXX
#include "Window.hxx"
#endif
#ifndef SD_DRAWVIEW_HXX
#include "drawview.hxx"
#endif
#include "drawdoc.hxx"
#include "DrawDocShell.hxx"
#ifndef SD_DRAW_VIEW_SHELL_HXX
#include "DrawViewShell.hxx"
#endif
#include "Ruler.hxx"
#ifndef SD_CLIENT_HXX
#include "Client.hxx"
#endif
#ifndef SD_PREVIEW_WINDOW_HXX
#include "PreviewWindow.hxx"
#endif
#ifndef SD_PREVIEW_CHILD_WINDOW_HXX
#include "PreviewChildWindow.hxx"
#endif
#ifndef SD_FU_SLIDE_SHOW_HXX
#include "fuslshow.hxx"
#endif
#include "optsitem.hxx"
#ifndef SD_FU_SEARCH_HXX
#include "fusearch.hxx"
#endif
#ifndef SD_FU_SPELL_HXX
#include "fuspell.hxx"
#endif
#ifndef SD_OUTLINER_HXX
#include "Outliner.hxx"
#endif
#ifndef SD_ANIMATION_CHILD_WINDOW_HXX
#include "AnimationChildWindow.hxx"
#endif
#include "SdUnoDrawView.hxx"
#ifndef SD_OBJECT_BAR_MANAGER_HXX
#include "ObjectBarManager.hxx"
#endif
#ifndef SD_VIEW_SHELL_BASE_HXX
#include "ViewShellBase.hxx"
#endif
#ifndef SD_LAYER_DIALOG_CHILD_WINDOW_HXX
#include "LayerDialogChildWindow.hxx"
#endif
#include "LayerTabBar.hxx"

#ifndef SO2_DECL_SVINPLACEOBJECT_DEFINED
#define SO2_DECL_SVINPLACEOBJECT_DEFINED
SO2_DECL_REF(SvInPlaceObject)
#endif
#ifndef SO2_DECL_SVSTORAGE_DEFINED
#define SO2_DECL_SVSTORAGE_DEFINED
SO2_DECL_REF(SvStorage)
#endif

#ifdef WNT
#pragma optimize ( "", off )
#endif

namespace sd {

/*************************************************************************
|*
|* Activate(), beim ersten Aufruf wird eine Shell fuer den Object Bar erzeugt
|*
\************************************************************************/

void DrawViewShell::Activate(BOOL bIsMDIActivate)
{
    ViewShell::Activate(bIsMDIActivate);

    // When no object bars are active then activate some.
    if (GetObjectBarManager().GetTopObjectBarId() == snInvalidShellId)
        GetObjectBarManager().SwitchObjectBar (RID_DRAW_OBJ_TOOLBOX);

    if (bIsMDIActivate)
    {
        BOOL bPreview = FALSE;

        if (eEditMode == EM_PAGE)
            bPreview = pFrameView->IsShowPreviewInPageMode() != 0;
        else
            bPreview = pFrameView->IsShowPreviewInMasterPageMode() != 0;

        SfxBoolItem aItem(SID_PREVIEW_WIN, bPreview);
        GetViewFrame()->GetDispatcher()->Execute(
            SID_PREVIEW_WIN, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD, &aItem, 0L);
    }
}

void DrawViewShell::UIActivate( SvInPlaceObject *pIPObj )
{
    ViewShell::UIActivate(pIPObj);

    // #94252# Disable own controls
    aTabControl.Disable();
    if (GetLayerTabControl() != NULL)
        GetLayerTabControl()->Disable();
}

void DrawViewShell::UIDeactivate( SvInPlaceObject *pIPObj )
{
    // #94252# Enable own controls
    aTabControl.Enable();
    if (GetLayerTabControl() != NULL)
        GetLayerTabControl()->Enable();

    ViewShell::UIDeactivate(pIPObj);
}

/*************************************************************************
|*
|* Deactivate()
|*
\************************************************************************/

void DrawViewShell::Deactivate(BOOL bIsMDIActivate)
{
    ViewShell::Deactivate(bIsMDIActivate);
}

/*************************************************************************
|*
|* Wird gerufen, wenn sich der Selektionszustand der View aendert
|*
\************************************************************************/

void DrawViewShell::SelectionHasChanged (void)
{
    // Um die Performance zu steigern wird jetzt die komplette
    // Shell invalidiert statt alle Slots einzeln
    Invalidate();

    // Damit das Effekte-Window auch einen aktuellen Status bekommt
    UpdateEffectWindow();

    //Update3DWindow(); // 3D-Controller
    SfxBoolItem aItem( SID_3D_STATE, TRUE );
    GetViewFrame()->GetDispatcher()->Execute(
        SID_3D_STATE, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD, &aItem, 0L );

    SdrOle2Obj* pOleObj = NULL;
    SdrGrafObj* pGrafObj = NULL;

    if ( pDrView->AreObjectsMarked() )
    {
        const SdrMarkList& rMarkList = pDrView->GetMarkedObjectList();

        if (rMarkList.GetMarkCount() == 1)
        {
            SdrMark* pMark = rMarkList.GetMark(0);
            SdrObject* pObj = pMark->GetObj();

            UINT32 nInv = pObj->GetObjInventor();
            UINT16 nSdrObjKind = pObj->GetObjIdentifier();

            if (nInv = SdrInventor && nSdrObjKind == OBJ_OLE2)
            {
                pOleObj = (SdrOle2Obj*) pObj;
                UpdateIMapDlg( pObj );
            }
            else if (nSdrObjKind == OBJ_GRAF)
            {
                pGrafObj = (SdrGrafObj*) pObj;
                UpdateIMapDlg( pObj );
            }
        }
    }

    ViewShellBase& rBase = GetViewShellBase();
    Client* pIPClient = static_cast<Client*>(rBase.GetIPClient());

    if ( pIPClient && pIPClient->IsInPlaceActive() )
    {
        /**********************************************************************
        * Ggf. OLE-Objekt beruecksichtigen und deaktivieren
        **********************************************************************/

        // this means we recently deselected an inplace active ole object so
        // we need to deselect it now
        if (!pOleObj)
        {
            pIPClient->GetProtocol().Reset2Open();

            SFX_APP()->SetViewFrame( GetViewFrame() );
            rBase.SetVerbs(0);
            pDrView->ShowMarkHdl(NULL);
        }
        else
        {
            SvInPlaceObjectRef aIPObj = pOleObj->GetObjRef();

            if ( aIPObj.Is() )
            {
                rBase.SetVerbs( &aIPObj->GetVerbList() );
            }
            else
            {
                rBase.SetVerbs(NULL);
            }
        }
    }
    else
    {
#ifdef STARIMAGE_AVAILABLE
        if ( pGrafObj && pGrafObj->GetGraphicType() == GRAPHIC_BITMAP && SFX_APP()->HasFeature(SFX_FEATURE_SIMAGE))
        {
            SvVerb aVerb( 0, String( SdResId(STR_EDIT_OBJ) ) );
            SvVerbList aVerbList;
            aVerbList.Append( aVerb );
            rBase.SetVerbs( &aVerbList );
        }
        else
#endif
        if ( pOleObj )
        {
            SvInPlaceObjectRef aIPObj = pOleObj->GetObjRef();

            if ( aIPObj.Is() )
            {
                rBase.SetVerbs( &aIPObj->GetVerbList() );
            }
            else
            {
                rBase.SetVerbs(NULL);
            }
        }
        else
        {
            rBase.SetVerbs(NULL);
        }
    }

    if( pFuActual )
    {
        pFuActual->SelectionHasChanged();
    }
    else
    {
        USHORT nObjBarId;

        if (GetView()->GetContext() == SDRCONTEXT_POINTEDIT)
            nObjBarId = RID_BEZIER_TOOLBOX;
        else if (GetView()->GetContext() == SDRCONTEXT_GRAPHIC)
            nObjBarId = RID_DRAW_GRAF_TOOLBOX;
        else if (GetView()->GetContext() == SDRCONTEXT_TEXTEDIT)
            nObjBarId = RID_DRAW_TEXT_TOOLBOX;  // #96124# Keep text bar when in textedit
        else
            nObjBarId = RID_DRAW_OBJ_TOOLBOX;

        GetObjectBarManager().SwitchObjectBar (nObjBarId);
    }

    // #96124# Invalidate for every subshell
    GetObjectBarManager().InvalidateAllObjectBars();

    if( SFX_APP()->GetHelpPI() )
        SetHelpIdBySelection();

    pDrView->UpdateSelectionClipboard( FALSE );

    if (GetController() != NULL)
        GetController()->FireSelectionChangeListener();
}


/*************************************************************************
|*
|* Zoomfaktor setzen
|*
\************************************************************************/

void DrawViewShell::SetZoom( long nZoom )
{
    // Make sure that the zoom factor will not be recalculated on
    // following window resizings.
    bZoomOnPage = FALSE;
    ViewShell::SetZoom( nZoom );
    GetViewFrame()->GetBindings().Invalidate( SID_ATTR_ZOOM );
}

/*************************************************************************
|*
|* Zoomrechteck fuer aktives Fenster einstellen
|*
\************************************************************************/

void DrawViewShell::SetZoomRect( const Rectangle& rZoomRect )
{
    ViewShell::SetZoomRect( rZoomRect );
    GetViewFrame()->GetBindings().Invalidate( SID_ATTR_ZOOM );
}


/*************************************************************************
|*
|* PrepareClose, ggfs. Texteingabe beenden, damit andere Viewshells ein
|* aktualisiertes Textobjekt vorfinden
|*
\************************************************************************/

USHORT DrawViewShell::PrepareClose( BOOL bUI, BOOL bForBrowsing )
{
    if ( ViewShell::PrepareClose(bUI, bForBrowsing) != TRUE )
        return FALSE;

    SfxChildWindow* pPreviewChild = GetViewFrame()->GetChildWindow(
        PreviewChildWindow::GetChildWindowId() );
    BOOL            bRet = TRUE;

    if( pFuSlideShow )
    {
        pFuSlideShow->Terminate();
        bRet = FALSE;
    }

    if( pPreviewChild && pPreviewChild->GetWindow() )
    {
        PreviewWindow* pPreviewWin = static_cast<PreviewWindow*>(
            pPreviewChild->GetWindow());
        FuSlideShow* pShow = pPreviewWin ? pPreviewWin->GetSlideShow() : NULL;

        if( pPreviewWin->GetDoc() == GetDoc() && pShow && pShow->IsInputLocked() )
        {
            //          pShow->Terminate();
            //          bRet = FALSE;
            pShow->Destroy ();
        }
    }

    if( bRet && pFuActual )
    {
        USHORT nID = pFuActual->GetSlotID();
        if (nID == SID_TEXTEDIT || nID == SID_ATTR_CHAR)
        {
            pDrView->EndTextEdit();
        }
    }
    else if( !bRet )
    {
        aCloseTimer.SetTimeoutHdl( LINK( this, DrawViewShell, CloseHdl ) );
        aCloseTimer.SetTimeout( 20 );
        aCloseTimer.Start();
    }

    return bRet;
}

/*************************************************************************
|*
|* Status (Enabled/Disabled) von Menue-SfxSlots setzen
|*
\************************************************************************/

void DrawViewShell::ChangeEditMode(EditMode eEMode, bool bIsLayerModeActive)
{
    if (eEditMode != eEMode
        || mbIsLayerModeActive != bIsLayerModeActive)
    {
        USHORT nActualPageNum = 0;

        SdUnoDrawView* pController =
            static_cast<SdUnoDrawView*>(GetController());
        if (pController != NULL)
        {
            pController->FireChangeEditMode (eEMode == EM_MASTERPAGE);
            pController->FireChangeLayerMode (bIsLayerModeActive);
        }

        if ( pDrView->IsTextEdit() )
        {
            pDrView->EndTextEdit();
        }

        LayerTabBar* pLayerBar = GetLayerTabControl();
        if (pLayerBar != NULL)
            pLayerBar->EndEditMode();
        aTabControl.EndEditMode();

        if (ePageKind == PK_HANDOUT)
        {
            // Bei Handzetteln nur MasterPage zulassen
            eEMode = EM_MASTERPAGE;
        }

        eEditMode = eEMode;
        mbIsLayerModeActive = bIsLayerModeActive;

        if (eEditMode == EM_PAGE)
        {
            /******************************************************************
            * PAGEMODE
            ******************************************************************/

            aTabControl.Clear();

            SdPage* pPage;
            String aPageName;
            USHORT nPageCnt = GetDoc()->GetSdPageCount(ePageKind);

            for (USHORT i = 0; i < nPageCnt; i++)
            {
                pPage = GetDoc()->GetSdPage(i, ePageKind);
                aPageName = pPage->GetName();
                aTabControl.InsertPage(i + 1, aPageName);

                if ( pPage->IsSelected() && nActualPageNum == 0 )
                {
                    nActualPageNum = i;
                }
            }

            aTabControl.SetCurPageId(nActualPageNum + 1);

            SwitchPage(nActualPageNum);

            SfxBoolItem aItem(SID_PREVIEW_WIN, pFrameView->IsShowPreviewInPageMode() != 0 );
            GetViewFrame()->GetDispatcher()->Execute(
                SID_PREVIEW_WIN, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD, &aItem, 0L );
        }
        else
        {
            /******************************************************************
            * MASTERPAGE
            ******************************************************************/
            GetViewFrame()->SetChildWindow(
                AnimationChildWindow::GetChildWindowId(), FALSE );

            if (!pActualPage)
            {
                // Sofern es keine pActualPage gibt, wird die erste genommen
                pActualPage = GetDoc()->GetSdPage(0, ePageKind);
            }

            SdPage* pPreviewPage = pActualPage;

            aTabControl.Clear();
            USHORT nActualMasterPageNum = 0;
            USHORT nMasterPageCnt = GetDoc()->GetMasterSdPageCount(ePageKind);

            for (USHORT i = 0; i < nMasterPageCnt; i++)
            {
                SdPage* pMaster = GetDoc()->GetMasterSdPage(i, ePageKind);
                String aLayoutName(pMaster->GetLayoutName());
                aLayoutName.Erase(aLayoutName.SearchAscii(SD_LT_SEPARATOR));

                aTabControl.InsertPage(i + 1, aLayoutName);

                if (&(pActualPage->TRG_GetMasterPage()) == pMaster)
                {
                    nActualMasterPageNum = i;
                }
            }

            aTabControl.SetCurPageId(nActualMasterPageNum + 1);
            SwitchPage(nActualMasterPageNum);

            SfxBoolItem aItem(SID_PREVIEW_WIN, pFrameView->IsShowPreviewInMasterPageMode() != 0 );
            GetViewFrame()->GetDispatcher()->Execute(
                SID_PREVIEW_WIN, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD, &aItem, 0L );
        }

        if ( ! mbIsLayerModeActive)
        {
            aTabControl.Show();
            // Set the tab control only for draw pages.  For master page
            // this has been done already above.
            if (eEditMode == EM_PAGE)
                aTabControl.SetCurPageId (nActualPageNum + 1);
        }
        if (GetViewFrame()->KnowsChildWindow(
            LayerDialogChildWindow::GetChildWindowId()))
        {
            GetViewFrame()->SetChildWindow(
                LayerDialogChildWindow::GetChildWindowId(),
                IsLayerModeActive());
        }

        ResetActualLayer();

        Invalidate( SID_PAGEMODE );
        Invalidate( SID_LAYERMODE );
        Invalidate( SID_MASTERPAGE );
        Invalidate( SID_SLIDE_MASTERPAGE );
        Invalidate( SID_TITLE_MASTERPAGE );
        Invalidate( SID_NOTES_MASTERPAGE );
        Invalidate( SID_HANDOUT_MASTERPAGE );
        Invalidate(SID_PREVIEW_WIN);
    }
}




bool DrawViewShell::IsLayerModeActive (void) const
{
    return mbIsLayerModeActive;
}




/*************************************************************************
|*
|* Groesse des TabControls und der ModeButtons zurueckgeben
|*
\************************************************************************/

long DrawViewShell::GetHCtrlWidth()
{
    //  return aTabControl.GetSizePixel().Width();
    return 0;
}


/*************************************************************************
|*
|* Horizontales Lineal erzeugen
|*
\************************************************************************/

SvxRuler* DrawViewShell::CreateHRuler (::sd::Window* pWin, BOOL bIsFirst)
{
    Ruler* pRuler;
    WinBits  aWBits;
    USHORT   nFlags = SVXRULER_SUPPORT_OBJECT;

    if ( bIsFirst )
    {
        aWBits  = WB_HSCROLL | WB_3DLOOK | WB_BORDER | WB_EXTRAFIELD;
        nFlags |= ( SVXRULER_SUPPORT_SET_NULLOFFSET |
                    SVXRULER_SUPPORT_TABS |
                    SVXRULER_SUPPORT_PARAGRAPH_MARGINS ); // Neu
    }
    else
        aWBits = WB_HSCROLL | WB_3DLOOK | WB_BORDER;

    pRuler = new Ruler (*this, GetParentWindow(), pWin, nFlags,
        GetViewFrame()->GetBindings(), aWBits);
    pRuler->SetSourceUnit(pWin->GetMapMode().GetMapUnit());

    // Metric ...
    UINT16 nMetric = GetDoc()->GetUIUnit();

    if( nMetric == 0xffff )
        nMetric = GetModuleFieldUnit();

    pRuler->SetUnit( FieldUnit( nMetric ) );

    // ... und auch DefTab am Lineal einstellen
    pRuler->SetDefTabDist( GetDoc()->GetDefaultTabulator() ); // Neu

    Fraction aUIScale(pWin->GetMapMode().GetScaleX());
    aUIScale *= GetDoc()->GetUIScale();
    pRuler->SetZoom(aUIScale);

    return pRuler;
}

/*************************************************************************
|*
|* Vertikales Lineal erzeugen
|*
\************************************************************************/

SvxRuler* DrawViewShell::CreateVRuler(::sd::Window* pWin)
{
    Ruler* pRuler;
    WinBits  aWBits = WB_VSCROLL | WB_3DLOOK | WB_BORDER;
    USHORT   nFlags = SVXRULER_SUPPORT_OBJECT;

    pRuler = new Ruler(*this, GetParentWindow(), pWin, nFlags,
        GetViewFrame()->GetBindings(), aWBits);
    pRuler->SetSourceUnit(pWin->GetMapMode().GetMapUnit());

    // #96629# Metric same as HRuler, use document setting
    UINT16 nMetric = GetDoc()->GetUIUnit();

    if( nMetric == 0xffff )
        nMetric = GetModuleFieldUnit();

    pRuler->SetUnit( FieldUnit( nMetric ) );

    Fraction aUIScale(pWin->GetMapMode().GetScaleY());
    aUIScale *= GetDoc()->GetUIScale();
    pRuler->SetZoom(aUIScale);

    return pRuler;
}

/*************************************************************************
|*
|* Horizontales Lineal aktualisieren
|*
\************************************************************************/

void DrawViewShell::UpdateHRuler()
{
    Invalidate( SID_ATTR_LONG_LRSPACE );
    Invalidate( SID_RULER_PAGE_POS );
    Invalidate( SID_RULER_OBJECT );
    Invalidate( SID_RULER_TEXT_RIGHT_TO_LEFT );

    if (mpHorizontalRuler.get() != NULL)
        mpHorizontalRuler->ForceUpdate();
}

/*************************************************************************
|*
|* Vertikales Lineal aktualisieren
|*
\************************************************************************/

void DrawViewShell::UpdateVRuler()
{
    Invalidate( SID_ATTR_LONG_LRSPACE );
    Invalidate( SID_RULER_PAGE_POS );
    Invalidate( SID_RULER_OBJECT );

    if (mpVerticalRuler.get() != NULL)
        mpVerticalRuler->ForceUpdate();
}

/*************************************************************************
|*
|* Metrik setzen
|*
\************************************************************************/

void DrawViewShell::SetUIUnit(FieldUnit eUnit)
{
    ViewShell::SetUIUnit(eUnit);
}

/*************************************************************************
|*
|* TabControl nach Splitteraenderung aktualisieren
|*
\************************************************************************/

IMPL_LINK( DrawViewShell, TabSplitHdl, TabBar *, pTab )
{
    const long int nMax = aViewSize.Width() - aScrBarWH.Width()
        - aTabControl.GetPosPixel().X() ;

    Size aTabSize = aTabControl.GetSizePixel();
    aTabSize.Width() = Min(pTab->GetSplitSize(), (long)(nMax-1));

    aTabControl.SetSizePixel(aTabSize);
    GetLayerTabControl()->SetSizePixel(aTabSize);

    Point aPos = aTabControl.GetPosPixel();
    aPos.X() += aTabSize.Width();

    Size aScrSize(nMax - aTabSize.Width(), aScrBarWH.Height());
    mpHorizontalScrollBar->SetPosSizePixel(aPos, aScrSize);

    return 0;
}

/*************************************************************************
|*
|* neue aktuelle Seite auswaehlen, falls sich die Seitenfolge geaendert
|* hat (z. B. durch Undo)
|*
\************************************************************************/

void DrawViewShell::ResetActualPage()
{
    USHORT nCurrentPage = aTabControl.GetCurPageId() - 1;
    USHORT nPageCount   = (eEditMode == EM_PAGE)?GetDoc()->GetSdPageCount(ePageKind):GetDoc()->GetMasterSdPageCount(ePageKind);
    if (nPageCount > 0)
        nCurrentPage = Min((USHORT)(nPageCount - 1), nCurrentPage);
    else
        nCurrentPage = 0;

    if (eEditMode == EM_PAGE)
    {

        // Update fuer TabControl
        aTabControl.Clear();

        SdPage* pPage = NULL;
        String aPageName;

        for (USHORT i = 0; i < nPageCount; i++)
        {
            pPage = GetDoc()->GetSdPage(i, ePageKind);
            aPageName = pPage->GetName();
            aTabControl.InsertPage(i + 1, aPageName);

            // Selektionskennungen der Seiten korrigieren
            GetDoc()->SetSelected(pPage, i == nCurrentPage);
        }

        aTabControl.SetCurPageId(nCurrentPage + 1);
    }
    else // EM_MASTERPAGE
    {
        SdPage* pActualPage = GetDoc()->GetMasterSdPage(nCurrentPage, ePageKind);
        aTabControl.Clear();
        USHORT nActualMasterPageNum = 0;

        USHORT nMasterPageCnt = GetDoc()->GetMasterSdPageCount(ePageKind);
        for (USHORT i = 0; i < nMasterPageCnt; i++)
        {
            SdPage* pMaster = GetDoc()->GetMasterSdPage(i, ePageKind);
            String aLayoutName(pMaster->GetLayoutName());
            aLayoutName.Erase(aLayoutName.SearchAscii(SD_LT_SEPARATOR));
            aTabControl.InsertPage(i + 1, aLayoutName);

            if (pActualPage == pMaster)
                nActualMasterPageNum = i;
        }

        aTabControl.SetCurPageId(nActualMasterPageNum + 1);
        SwitchPage(nActualMasterPageNum);
    }

    GetViewFrame()->GetDispatcher()->Execute(SID_SWITCHPAGE,
                SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD);
}

/*************************************************************************
|*
|* Verb auf OLE-Objekt anwenden
|*
\************************************************************************/


ErrCode DrawViewShell::DoVerb(long nVerb)
{
    if ( pDrView->AreObjectsMarked() )
    {
        const SdrMarkList& rMarkList = pDrView->GetMarkedObjectList();

        if (rMarkList.GetMarkCount() == 1)
        {
            SdrMark* pMark = rMarkList.GetMark(0);
            SdrObject* pObj = pMark->GetObj();

            UINT32 nInv = pObj->GetObjInventor();
            UINT16 nSdrObjKind = pObj->GetObjIdentifier();

            if (nInv = SdrInventor && nSdrObjKind == OBJ_OLE2)
            {
                ActivateObject( (SdrOle2Obj*) pObj, nVerb);
            }
#ifdef STARIMAGE_AVAILABLE
            else if (nInv = SdrInventor && nSdrObjKind == OBJ_GRAF &&
                     ((SdrGrafObj*) pObj)->GetGraphicType() == GRAPHIC_BITMAP &&
                     SFX_APP()->HasFeature(SFX_FEATURE_SIMAGE))
            {
                SdrGrafObj* pSdrGrafObj = (SdrGrafObj*) pObj;
                short nOK = RET_YES;

                if ( pSdrGrafObj->GetFileName().Len() )
                {
                    // Graphik ist gelinkt, soll der Link aufgehoben werden?
                    QueryBox aBox(pWindow, WB_YES_NO | WB_DEF_YES,
                                  String( SdResId(STR_REMOVE_LINK) ) );
                    nOK = aBox.Execute();

                    if (nOK == RET_YES)
                    {
                        // Link aufheben (File- und Filtername zuruecksetzen)
                        pSdrGrafObj->SetGraphicLink(String(), String());
                    }
                }

                if (nOK == RET_YES)
                {
                    /**************************************************************
                    * OLE-Objekt erzeugen, StarImage starten
                    * Grafik-Objekt loeschen (durch OLE-Objekt ersetzt)
                    **************************************************************/
                    pDrView->HideMarkHdl(NULL);

                    SvStorageRef aStor = new SvStorage(String());
                    SvInPlaceObjectRef aNewIPObj = &((SvFactory*)SvInPlaceObject::ClassFactory())
                    ->CreateAndInit(SimModuleDummy::GetID(SOFFICE_FILEFORMAT_CURRENT), aStor);
                    if ( aNewIPObj.Is() )
                    {
                        SdrGrafObj* pTempSdrGrafObj = (SdrGrafObj*) pSdrGrafObj->Clone ();

                        SvEmbeddedInfoObject * pInfo;
                        pInfo = GetViewFrame()->GetObjectShell()->
                                       InsertObject( aNewIPObj, String() );

                        String aName;
                        if (pInfo)
                        {
                            aName = pInfo->GetObjName();
                        }

                        Rectangle aRect = pObj->GetLogicRect();
                        SdrOle2Obj* pSdrOle2Obj = new SdrOle2Obj( aNewIPObj,
                                                                  aName, aRect );

                        SdrPageView* pPV = pDrView->GetPageViewPvNum(0);

                        pPV->GetObjList()->InsertObject( pSdrOle2Obj );
                        pDrView->ReplaceObject( pObj, *pPV, pTempSdrGrafObj );

                        pSdrOle2Obj->SetLogicRect(aRect);
                        aNewIPObj->SetVisAreaSize(aRect.GetSize());

                        SimDLL::Update(aNewIPObj, pTempSdrGrafObj->GetGraphic(), pWindow);
                        ActivateObject(pSdrOle2Obj, SVVERB_SHOW);

                        Client* pClient = (Client*) GetIPClient();

                        if (pClient)
                            pClient->SetSdrGrafObj( pTempSdrGrafObj );
                    }
                }
            }
#endif
        }
    }

    return 0;
}


/*************************************************************************
|*
|* OLE-Object aktivieren
|*
\************************************************************************/

BOOL DrawViewShell::ActivateObject(SdrOle2Obj* pObj, long nVerb)
{
    BOOL bActivated = FALSE;

    if ( !GetDocSh()->IsUIActive() )
    {
        bActivated = ViewShell::ActivateObject(pObj, nVerb);

        OSL_ASSERT(GetViewShell()!=NULL);
        Client* pClient = static_cast<Client*>(GetViewShell()->GetIPClient());

        if (pClient)
        {
            pClient->SetSdrGrafObj(NULL);
        }
    }

    return(bActivated);
}

/*************************************************************************
|*
|* Auf gewuenschte Seite schalten
|* Der Parameter nSelectedPage bezieht sich auf den aktuellen EditMode
|*
\************************************************************************/

BOOL DrawViewShell::SwitchPage(USHORT nSelectedPage)
{
    BOOL bOK = FALSE;

    if (IsSwitchPageAllowed())
    {
        bOK = TRUE;

        BOOL bIsChanged = GetDoc()->IsChanged();

        if (pActualPage)
        {
            SdPage* pNewPage = GetDoc()->GetSdPage(nSelectedPage, ePageKind);

            if (pActualPage == pNewPage)
            {
                if (eEditMode == EM_MASTERPAGE)
                {
                    pNewPage = (SdPage*)(&(pNewPage->TRG_GetMasterPage()));
                }

                SdrPageView* pPV = pDrView->GetPageViewPvNum(0);

                if (pPV && pNewPage == (SdPage*) pPV->GetPage() &&
                    pNewPage->GetName() == aTabControl.GetPageText(nSelectedPage+1))
                {
                    // Die Seite wird schon angezeigt
                    return(TRUE);
                }
            }
        }

        pActualPage = NULL;

        if (eEditMode == EM_PAGE)
        {
            pActualPage = GetDoc()->GetSdPage(nSelectedPage, ePageKind);
        }
        else
        {
            SdPage* pMaster = GetDoc()->GetMasterSdPage(nSelectedPage, ePageKind);

            // Passt die selektierte Seite zur MasterPage?
            USHORT nPageCount = GetDoc()->GetSdPageCount(ePageKind);
            for (USHORT i = 0; i < nPageCount; i++)
            {
                SdPage* pPage = GetDoc()->GetSdPage(i, ePageKind);
                if(pPage && pPage->IsSelected() && pMaster == &(pPage->TRG_GetMasterPage()))
                {
                    pActualPage = pPage;
                    break;
                }
            }

            if (!pActualPage)
            {
                // Die erste Seite nehmen, welche zur MasterPage passt
                for (USHORT i = 0; i < nPageCount; i++)
                {
                    SdPage* pPage = GetDoc()->GetSdPage(i, ePageKind);
                    if(pPage && pMaster == &(pPage->TRG_GetMasterPage()))
                    {
                        pActualPage = pPage;
                        break;
                    }
                }
            }
        }

        for (USHORT i = 0; i < GetDoc()->GetSdPageCount(ePageKind); i++)
        {
            // Alle Seiten deselektieren
            GetDoc()->SetSelected( GetDoc()->GetSdPage(i, ePageKind), FALSE);
        }

        if (!pActualPage)
        {
            // Sofern es keine pActualPage gibt, wird die erste genommen
            pActualPage = GetDoc()->GetSdPage(0, ePageKind);
        }

        // diese Seite auch selektieren (pActualPage zeigt immer auf Zeichenseite,
        // nie auf eine Masterpage)
        GetDoc()->SetSelected(pActualPage, TRUE);

        if( !pFuSlideShow || ( pFuSlideShow->GetAnimationMode() != ANIMATIONMODE_SHOW ) )
        {
            // VisArea zuziehen, um ggf. Objekte zu deaktivieren
            // !!! only if we are not in presentation mode (#96279) !!!
            OSL_ASSERT (GetViewShell()!=NULL);
            GetViewShell()->DisconnectAllClients();
            VisAreaChanged(Rectangle(Point(), Size(1, 1)));
        }

        if( pActualPage )
            pActualPage->SetAutoLayout( pActualPage->GetAutoLayout() );

        if (eEditMode == EM_PAGE)
        {
            /**********************************************************************
            * PAGEMODE
            **********************************************************************/
            GetDoc()->SetSelected(pActualPage, TRUE);

            SdrPageView* pPageView = pDrView->GetPageViewPvNum(0);

            if (pPageView)
            {
                pFrameView->SetVisibleLayers( pPageView->GetVisibleLayers() );
                pFrameView->SetPrintableLayers( pPageView->GetPrintableLayers() );
                pFrameView->SetLockedLayers( pPageView->GetLockedLayers() );

                if (ePageKind == PK_NOTES)
                {
                    pFrameView->SetNotesHelpLines( pPageView->GetHelpLines() );
                }
                else if (ePageKind == PK_HANDOUT)
                {
                    pFrameView->SetHandoutHelpLines( pPageView->GetHelpLines() );
                }
                else
                {
                    pFrameView->SetStandardHelpLines( pPageView->GetHelpLines() );
                }
            }

            pDrView->HideAllPages();
            pDrView->ShowPage(pActualPage, Point(0, 0));
            SdUnoDrawView* pController =
                static_cast<SdUnoDrawView*>(GetController());
            if (pController != NULL)
                pController->FireSwitchCurrentPage (pActualPage);

            SdrPageView* pNewPageView = pDrView->GetPageViewPvNum(0);

            if (pNewPageView)
            {
                pNewPageView->SetVisibleLayers( pFrameView->GetVisibleLayers() );
                pNewPageView->SetPrintableLayers( pFrameView->GetPrintableLayers() );
                pNewPageView->SetLockedLayers( pFrameView->GetLockedLayers() );

                if (ePageKind == PK_NOTES)
                {
                    pNewPageView->SetHelpLines( pFrameView->GetNotesHelpLines() );
                }
                else if (ePageKind == PK_HANDOUT)
                {
                    pNewPageView->SetHelpLines( pFrameView->GetHandoutHelpLines() );
                }
                else
                {
                    pNewPageView->SetHelpLines( pFrameView->GetStandardHelpLines() );
                }
            }

            aTabControl.SetCurPageId(nSelectedPage+1);
            String aPageName = pActualPage->GetName();

            if (aTabControl.GetPageText(nSelectedPage+1) != aPageName)
            {
                aTabControl.SetPageText(nSelectedPage+1, aPageName);
            }
        }
        else
        {
            /**********************************************************************
            * MASTERPAGE
            **********************************************************************/
            SdrPageView* pPageView = pDrView->GetPageViewPvNum(0);

            if (pPageView)
            {
                pFrameView->SetVisibleLayers( pPageView->GetVisibleLayers() );
                pFrameView->SetPrintableLayers( pPageView->GetPrintableLayers() );
                pFrameView->SetLockedLayers( pPageView->GetLockedLayers() );

                if (ePageKind == PK_NOTES)
                {
                    pFrameView->SetNotesHelpLines( pPageView->GetHelpLines() );
                }
                else if (ePageKind == PK_HANDOUT)
                {
                    pFrameView->SetHandoutHelpLines( pPageView->GetHelpLines() );
                }
                else
                {
                    pFrameView->SetStandardHelpLines( pPageView->GetHelpLines() );
                }
            }

            pDrView->HideAllPages();

            SdPage* pMaster = GetDoc()->GetMasterSdPage(nSelectedPage, ePageKind);

            if( !pMaster )              // Falls es diese Page nicht geben sollte
                pMaster = GetDoc()->GetMasterSdPage(0, ePageKind);

            USHORT nNum = pMaster->GetPageNum();
            pDrView->ShowMasterPagePgNum(nNum, Point(0, 0));

            SdUnoDrawView* pController =
                static_cast<SdUnoDrawView*>(GetController());
            if (pController != NULL)
                pController->FireSwitchCurrentPage (pMaster);

            SdrPageView* pNewPageView = pDrView->GetPageViewPvNum(0);

            if (pNewPageView)
            {
                pNewPageView->SetVisibleLayers( pFrameView->GetVisibleLayers() );
                pNewPageView->SetPrintableLayers( pFrameView->GetPrintableLayers() );
                pNewPageView->SetLockedLayers( pFrameView->GetLockedLayers() );

                if (ePageKind == PK_NOTES)
                {
                    pNewPageView->SetHelpLines( pFrameView->GetNotesHelpLines() );
                }
                else if (ePageKind == PK_HANDOUT)
                {
                    pNewPageView->SetHelpLines( pFrameView->GetHandoutHelpLines() );
                }
                else
                {
                    pNewPageView->SetHelpLines( pFrameView->GetStandardHelpLines() );
                }
            }

            String aLayoutName(pMaster->GetLayoutName());
            aLayoutName.Erase(aLayoutName.SearchAscii(SD_LT_SEPARATOR));

            aTabControl.SetCurPageId(nSelectedPage+1);

            if (aTabControl.GetPageText(nSelectedPage+1) != aLayoutName)
            {
                aTabControl.SetPageText(nSelectedPage+1, aLayoutName);
            }

            if( ePageKind == PK_HANDOUT )
            {
                // set pages for all available handout presentation objects
                sd::PresentationObjectList::iterator aIter( pMaster->GetPresObjList().begin() );
                const sd::PresentationObjectList::iterator aEnd( pMaster->GetPresObjList().end() );

                sal_uInt16 nPgNum = 0;
                while( aIter != aEnd )
                {
                    if( (*aIter).meKind == PRESOBJ_HANDOUT )
                    {
                        const sal_uInt16 nDestinationPageNum(2 * nPgNum + 1);

                        if(nDestinationPageNum < GetDoc()->GetPageCount())
                        {
                            static_cast<SdrPageObj*>((*aIter).mpObject)->SetReferencedPage(GetDoc()->GetPage(nDestinationPageNum));
                        }
                        else
                        {
                            static_cast<SdrPageObj*>((*aIter).mpObject)->SetReferencedPage(0L);
                        }

                        nPgNum++;
                    }

                    aIter++;
                }
            }
        }

        Size aVisSizePixel = GetActiveWindow()->GetOutputSizePixel();
        Rectangle aVisAreaWin = GetActiveWindow()->PixelToLogic( Rectangle( Point(0,0), aVisSizePixel) );
        VisAreaChanged(aVisAreaWin);
        pDrView->VisAreaChanged(GetActiveWindow());

        // Damit der Navigator (und das Effekte-Window) das mitbekommt (/-men)
        SfxBindings& rBindings = GetViewFrame()->GetBindings();
        rBindings.Invalidate(SID_NAVIGATOR_PAGENAME, TRUE, FALSE);
        rBindings.Invalidate(SID_EFFECT_STATE, TRUE, FALSE);
        rBindings.Invalidate(SID_STATUS_PAGE, TRUE, FALSE);
        UpdateSlideChangeWindow();
        UpdatePreview( pActualPage );

        if (pDrView->GetSlideShow())
        {
            pDrView->SetAnimationMode(TRUE);
        }

        GetDoc()->SetChanged(bIsChanged);
    }

    return (bOK);
}


/*************************************************************************
|*
|* Pruefen, ob ein Seitenwechsel erlaubt ist
|*
\************************************************************************/

BOOL DrawViewShell::IsSwitchPageAllowed() const
{
    bool bOK = true;

    FmFormShell* pFormShell = static_cast<FmFormShell*>(
        GetObjectBarManager().GetObjectBar(RID_FORMLAYER_TOOLBOX));
    if (pFormShell!=NULL && !pFormShell->PrepareClose (FALSE))
        bOK = false;

    return bOK && !bInEffectAssignment;
}

/*************************************************************************
|*
|* neue aktuelle Seite auswaehlen, falls sich die Seitenfolge geaendert
|* hat (z. B. durch Undo)
|*
\************************************************************************/

void DrawViewShell::ResetActualLayer()
{
    LayerTabBar* pLayerBar = GetLayerTabControl();
    if (pLayerBar != NULL)
    {
        // remember old layer cound and current layer id
        // this is needed when one layer is renamed to
        // restore current layer
        USHORT nOldLayerCnt = pLayerBar->GetPageCount();
        USHORT nOldLayerId = pLayerBar->GetCurPageId();

        /*************************************************************
            * Update fuer LayerTab
            *************************************************************/
        pLayerBar->Clear();

        String aName;
        String aActiveLayer = pDrView->GetActiveLayer();
        String aBackgroundLayer( SdResId(STR_LAYER_BCKGRND) );
        String aBackgroundObjLayer( SdResId(STR_LAYER_BCKGRNDOBJ) );
        String aLayoutLayer( SdResId(STR_LAYER_LAYOUT) );
        String aControlsLayer( SdResId(STR_LAYER_CONTROLS) );
        String aMeasureLinesLayer( SdResId(STR_LAYER_MEASURELINES) );
        USHORT nNewLayer = 0;
        USHORT nActiveLayer = SDRLAYER_NOTFOUND;
        SdrLayerAdmin& rLayerAdmin = GetDoc()->GetLayerAdmin();
        USHORT nLayerCnt = rLayerAdmin.GetLayerCount();

        for ( USHORT nLayer = 0; nLayer < nLayerCnt; nLayer++ )
        {
            aName = rLayerAdmin.GetLayer(nLayer)->GetName();

            if ( aName == aActiveLayer )
            {
                nActiveLayer = nLayer;
            }

            if ( aName != aBackgroundLayer )
            {
                if (eEditMode == EM_MASTERPAGE)
                {
                    // Layer der Page nicht auf MasterPage anzeigen
                    if (aName != aLayoutLayer   &&
                        aName != aControlsLayer &&
                        aName != aMeasureLinesLayer)
                    {
                        pLayerBar->InsertPage(nLayer+1, aName);

                        TabBarPageBits nBits = 0;
                        SdrPageView* pPV = pDrView->GetPageViewPvNum(0);

                        if (pPV && !pPV->IsLayerVisible(aName))
                        {
                            // Unsichtbare Layer werden anders dargestellt
                            nBits = TPB_SPECIAL;
                        }

                        pLayerBar->SetPageBits(nLayer+1, nBits);
                    }
                }
                else
                {
                    // Layer der MasterPage nicht auf Page anzeigen
                    if ( aName != aBackgroundObjLayer )
                    {
                        pLayerBar->InsertPage(nLayer+1, aName);

                        TabBarPageBits nBits = 0;

                        if (!pDrView->GetPageViewPvNum(0)->IsLayerVisible(aName))
                        {
                            // Unsichtbare Layer werden anders dargestellt
                            nBits = TPB_SPECIAL;
                        }

                        pLayerBar->SetPageBits(nLayer+1, nBits);
                    }
                }
            }
        }

        if ( nActiveLayer == SDRLAYER_NOTFOUND )
        {
            if( nOldLayerCnt == pLayerBar->GetPageCount() )
            {
                nActiveLayer = nOldLayerId - 1;
            }
            else
            {
                nActiveLayer = ( eEditMode == EM_MASTERPAGE ) ? 2 : 0;
            }

            pDrView->SetActiveLayer( pLayerBar->GetPageText(nActiveLayer + 1) );
        }

        pLayerBar->SetCurPageId(nActiveLayer + 1);
        GetViewFrame()->GetBindings().Invalidate( SID_MODIFYLAYER );
    }
}

/*************************************************************************
|*
|* Verzoegertes Close ausfuehren
|*
\************************************************************************/

IMPL_LINK( DrawViewShell, CloseHdl, Timer*, pTimer )
{
    pTimer->Stop();
    GetViewFrame()->GetBindings().Execute( SID_CLOSEWIN );
    return 0L;
}

/*************************************************************************
|*
|* Setzt die HelpId in Abhaengigkeit von der Selektion
|*
\************************************************************************/

void DrawViewShell::SetHelpIdBySelection()
{
    UINT32 nHelpId = 0;
    const SdrMarkList& rMarkList = pDrView->GetMarkedObjectList();

    if( rMarkList.GetMarkCount() > 0 )
    {
        SdrObject* pObj = rMarkList.GetMark(0)->GetObj();
        UINT32 nInventor = pObj->GetObjInventor();
        UINT16 nObjId = pObj->GetObjIdentifier();

        if( nInventor == SdrInventor)
        {
            switch ( nObjId )
            {
                case OBJ_LINE:
                    // Alle Linien
                    nHelpId = SID_DRAWTBX_LINES;
                break;

                case OBJ_RECT:
                    // Gefuellt:   Rechteck, Quadrat, Rechteck abgerundet, Quadrat abgerundet
                    // Ungefuellt: Rechteck, Quadrat, Rechteck abgerundet, Quadrat abgerundet
                    nHelpId = SID_DRAWTBX_RECTANGLES;
                break;

                case OBJ_CIRC:
                    // Gefuellt:   Ellipse, Kreis
                    // Ungefuellt: Ellipse, Kreis
                    nHelpId = SID_DRAWTBX_ELLIPSES;
                break;

                case OBJ_SECT:
                    // Gefuellt:   Ellipsensektor, Kreissektor
                    // Ungefuellt: Ellipsensektor, Kreissektor
                    nHelpId = SID_DRAWTBX_ELLIPSES;
                break;

                case OBJ_CARC:
                    // Ellipsenbogen, Kreisbogen
                    nHelpId = SID_DRAWTBX_ELLIPSES;
                break;

                case OBJ_CCUT:
                    // Gefuellt:   Ellipsensegment, Kreissegment
                    // Ungefuellt: Ellipsensegment, Kreissegment
                    nHelpId = SID_DRAWTBX_ELLIPSES;
                break;

                case OBJ_POLY:
                    // Gefuellt: Polygon, Polygon 45
                    nHelpId = SID_DRAWTBX_LINES;
                break;

                case OBJ_PLIN:
                    // Ungefuellt: Polygon, Polygon 45
                    nHelpId = SID_DRAWTBX_LINES;
                break;

                case OBJ_PATHLINE:
                    // Ungefuellt: Kurve, Freihandlinie
                    nHelpId = SID_DRAWTBX_LINES;
                break;

                case OBJ_PATHFILL:
                    // Gefuellt: Kurve, Freihandlinie
                    nHelpId = SID_DRAWTBX_LINES;
                break;

                case OBJ_TEXT:
                    // Text, Text an Rahmen
                    nHelpId = SID_DRAWTBX_TEXT;
                break;

                case OBJ_TITLETEXT:
                    nHelpId = SID_DRAWTBX_TEXT;
                break;

                case OBJ_OUTLINETEXT:
                    nHelpId = SID_DRAWTBX_TEXT;
                break;

                case OBJ_GRAF:
                    // Graphik
                    nHelpId = 0;
                break;

                case OBJ_OLE2:
                    // OLE
                    nHelpId = 0;
                break;

                case OBJ_EDGE:
                    // Alle Verbinder
                    nHelpId = SID_DRAWTBX_CONNECTORS;
                break;

                case OBJ_CAPTION:
                    // Legende
                    nHelpId = SID_DRAWTBX_TEXT;
                break;

                case OBJ_PAGE:
                    // Seitendarstellungsobjekt
                    nHelpId = 0;
                break;

                case OBJ_MEASURE:
                    // Masslinie
                    nHelpId = SID_DRAWTBX_LINES;
                break;

                case OBJ_UNO:
                    // UNO
                    nHelpId = 0;
                break;
            }
        }
        else if( nInventor == E3dInventor)
        {
            // Alle 3D-Objekte
            nHelpId = SID_DRAWTBX_3D_OBJECTS;
        }
        else if( nInventor == FmFormInventor)
        {
            // Alle FormControls
            nHelpId = HID_FM_CTL_SELECTION;
        }
    }
}

/*************************************************************************
|*
|* AcceptDrop
|*
\************************************************************************/

sal_Int8 DrawViewShell::AcceptDrop (
    const AcceptDropEvent& rEvt,
    DropTargetHelper& rTargetHelper,
    ::sd::Window* pTargetWindow,
    USHORT nPage,
    USHORT nLayer )
{
    if( nPage != SDRPAGE_NOTFOUND )
        nPage = GetDoc()->GetSdPage( nPage, ePageKind )->GetPageNum();

    if( pFuSlideShow )
    {
        if( !pFuSlideShow->IsLivePresentation() )
        {
            return DND_ACTION_NONE;
        }
    }

    return pDrView->AcceptDrop( rEvt, rTargetHelper, pTargetWindow, nPage, nLayer );
}

/*************************************************************************
|*
|* ExecuteDrop
|*
\************************************************************************/

sal_Int8 DrawViewShell::ExecuteDrop (
    const ExecuteDropEvent& rEvt,
    DropTargetHelper& rTargetHelper,
    ::sd::Window* pTargetWindow,
    USHORT nPage,
    USHORT nLayer)
{
    if( nPage != SDRPAGE_NOTFOUND )
        nPage = GetDoc()->GetSdPage( nPage, ePageKind )->GetPageNum();

    if( pFuSlideShow )
    {
        if( !pFuSlideShow->IsLivePresentation() )
        {
            return DND_ACTION_NONE;
        }
    }

    return pDrView->ExecuteDrop( rEvt, rTargetHelper, pTargetWindow, nPage, nLayer );
}

} // end of namespace sd

#ifdef WNT
#pragma optimize ( "", on )
#endif

