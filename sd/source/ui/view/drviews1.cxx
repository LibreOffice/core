/*************************************************************************
 *
 *  $RCSfile: drviews1.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:48:43 $
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
#ifndef _SFX_SAVEOPT_HXX //autogen
#include <sfx2/saveopt.hxx>
#endif
#ifndef _SV_SCRBAR_HXX //autogen
#include <vcl/scrbar.hxx>
#endif
#ifndef _SVDOGRAF_HXX //autogen
#include <svx/svdograf.hxx>
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
#ifndef _SFX_HELP_HXX //autogen
#include <sfx2/sfxhelp.hxx>
#endif
#ifndef _SFX_HELP_HXX //autogen
#include <sfx2/sfxhelp.hxx>
#endif
#ifndef _OUTLINER_HXX //autogen
#include <svx/outliner.hxx>
#endif
#ifndef _OFF_APP_HXX //autogen
#include <offmgr/app.hxx>
#endif

#pragma hdrstop

#include "misc.hxx"

#ifdef STARIMAGE_AVAILABLE
#ifndef _SIMDLL_HXX
#include <sim2/simdll.hxx>
#endif
#endif

// #define ITEMID_SIZE             0

#include "glob.hrc"
#include "app.hrc"
#include "res_bmp.hrc"
#include "strings.hrc"
#include "helpids.h"

#include "app.hxx"
#include "fupoor.hxx"
#include "sdresid.hxx"
#include "fusel.hxx"
#include "sdpage.hxx"
#include "frmview.hxx"
#include "stlpool.hxx"
#include "sdwindow.hxx"
#include "drawview.hxx"
#include "drawdoc.hxx"
#include "docshell.hxx"
#include "drviewsh.hxx"
#include "sdruler.hxx"
#include "sdclient.hxx"
#include "preview.hxx"
#include "prevchld.hxx"
#include "fuslshow.hxx"
#include "optsitem.hxx"
#include "fusearch.hxx"
#include "fuspell.hxx"
#include "sdoutl.hxx"
#include "animobjs.hxx"

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

/*************************************************************************
|*
|* Activate(), beim ersten Aufruf wird eine Shell fuer den Object Bar erzeugt
|*
\************************************************************************/

void __EXPORT SdDrawViewShell::Activate(BOOL bIsMDIActivate)
{
    SdViewShell::Activate(bIsMDIActivate);
    if (nCurrentObjectBar == 0)    // nur wenn noch kein Object Bar existent
    {
        nCurrentObjectBar = RID_DRAW_OBJ_TOOLBOX;

        RemoveSubShell();
        AddSubShell( *(SfxShell*) aShellTable.Get( RID_FORMLAYER_TOOLBOX ) );

        AddSubShell( *(SfxShell*) aShellTable.Get( nCurrentObjectBar ) );
    }

    if (bIsMDIActivate)
    {
        BOOL bPreview = FALSE;

        if (eEditMode == EM_PAGE)
            bPreview = pFrameView->IsShowPreviewInPageMode();
        else
            bPreview = pFrameView->IsShowPreviewInMasterPageMode();

        SfxBoolItem aItem(SID_PREVIEW_WIN, bPreview);
        SFX_DISPATCHER().Execute(SID_PREVIEW_WIN, SFX_CALLMODE_ASYNCHRON |
                                 SFX_CALLMODE_RECORD, &aItem, 0L);
    }
}

/*************************************************************************
|*
|* Deactivate()
|*
\************************************************************************/

void __EXPORT SdDrawViewShell::Deactivate(BOOL bIsMDIActivate)
{
    SdViewShell::Deactivate(bIsMDIActivate);
}

/*************************************************************************
|*
|* Wird gerufen, wenn sich der Selektionszustand der View aendert
|*
\************************************************************************/

void SdDrawViewShell::SelectionHasChanged()
{
    // Um die Performance zu steigern wird jetzt die komplette
    // Shell invalidiert statt alle Slots einzeln
    Invalidate();

    // Damit das Effekte-Window auch einen aktuellen Status bekommt
    //SFX_BINDINGS().Invalidate( SID_EFFECT_STATE, TRUE, FALSE );
    UpdateEffectWindow();

    //Update3DWindow(); // 3D-Controller
    SfxBoolItem aItem( SID_3D_STATE, TRUE );
    SFX_DISPATCHER().Execute( SID_3D_STATE, SFX_CALLMODE_ASYNCHRON |
                                SFX_CALLMODE_RECORD, &aItem, 0L );

    SdrOle2Obj* pOleObj = NULL;
    SdrGrafObj* pGrafObj = NULL;

    if ( pDrView->HasMarkedObj() )
    {
        const SdrMarkList& rMarkList = pDrView->GetMarkList();

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

    SdClient* pIPClient = (SdClient*) GetIPClient();

    if ( pIPClient && pIPClient->IsInPlaceActive() )
    {
        /**********************************************************************
        * Ggf. OLE-Objekt beruecksichtigen und deaktivieren
        **********************************************************************/
        if (!pOleObj)
        {
            pIPClient->GetProtocol().Reset2Open();

            SFX_APP()->SetViewFrame( GetViewFrame() );
            SetVerbs(0);
            pDrView->ShowMarkHdl(NULL);

            aTabControl.Enable();
            aLayerTab.Enable();
            aDrawBtn.Enable();
            aNotesBtn.Enable();
            aHandoutBtn.Enable();
            aOutlineBtn.Enable();
            aSlideBtn.Enable();
            aPresentationBtn.Enable();
        }
        else
        {
            SvInPlaceObjectRef aIPObj = pOleObj->GetObjRef();

            if ( aIPObj.Is() )
            {
                SetVerbs( &aIPObj->GetVerbList() );
            }
            else
            {
                SetVerbs(NULL);
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
            SetVerbs( &aVerbList );
        }
        else
#endif
        if ( pOleObj )
        {
            SvInPlaceObjectRef aIPObj = pOleObj->GetObjRef();

            if ( aIPObj.Is() )
            {
                SetVerbs( &aIPObj->GetVerbList() );
            }
            else
            {
                SetVerbs(NULL);
            }
        }
        else
        {
            SetVerbs(NULL);
        }
    }

    if( pFuActual )
    {
        pFuActual->SelectionHasChanged();
    }
    else
    {
        USHORT nObjBarId;

        if (pView->GetContext() == SDRCONTEXT_POINTEDIT)
            nObjBarId = RID_BEZIER_TOOLBOX;
        else if (pView->GetContext() == SDRCONTEXT_GRAPHIC)
            nObjBarId = RID_DRAW_GRAF_TOOLBOX;
        else
            nObjBarId = RID_DRAW_OBJ_TOOLBOX;

        SwitchObjectBar(nObjBarId);
    }

    if (GetObjectBar() == RID_BEZIER_TOOLBOX)
    {
        SfxShell* pShell = (SfxShell*) aShellTable.Get(RID_BEZIER_TOOLBOX);
        pShell->Invalidate();
    }
    else if (GetObjectBar() == RID_DRAW_GRAF_TOOLBOX)
    {
        SfxShell* pShell = (SfxShell*) aShellTable.Get(RID_DRAW_GRAF_TOOLBOX);
        pShell->Invalidate();
    }

    if( SFX_APP()->GetHelpPI() )
        SetHelpIdBySelection();
}


/*************************************************************************
|*
|* Zoomfaktor setzen
|*
\************************************************************************/

void SdDrawViewShell::SetZoom( long nZoom )
{
    SdViewShell::SetZoom( nZoom );
    SFX_BINDINGS().Invalidate( SID_ATTR_ZOOM );
}

/*************************************************************************
|*
|* Zoomrechteck fuer aktives Fenster einstellen
|*
\************************************************************************/

void SdDrawViewShell::SetZoomRect( const Rectangle& rZoomRect )
{
    SdViewShell::SetZoomRect( rZoomRect );
    SFX_BINDINGS().Invalidate( SID_ATTR_ZOOM );
}

/*************************************************************************
|*
|* Modus-Umschaltung (Draw, Slide, Outline)
|*
\************************************************************************/

IMPL_LINK( SdDrawViewShell, TabModeBtnHdl, Button *, pButton )
{
    if ( !((ImageButton*) pButton)->IsChecked() ||
         pButton == &aLayerBtn)
    {
        USHORT nSlotId;

        if ( pButton == &aPageBtn )
            nSlotId = SID_PAGEMODE;
        else if ( pButton == &aMasterPageBtn )
            nSlotId = SID_MASTERPAGE;
        else if ( pButton == &aLayerBtn )
            nSlotId = SID_LAYERMODE;

        GetViewFrame()->GetDispatcher()->Execute(nSlotId,
                    SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD);
    }

    pWindow->GrabFocus();         // SdWindow soll den Focus erhalten

    return 0;
}

/*************************************************************************
|*
|* QueryDrop-Event
|*
\************************************************************************/

BOOL SdDrawViewShell::QueryDrop(DropEvent& rEvt, SdWindow* pWin,
                                USHORT nPage, USHORT nLayer)
{
    if (nPage != SDRPAGE_NOTFOUND)
    {
        SdPage* pPage = pDoc->GetSdPage(nPage, ePageKind);
        nPage = pPage->GetPageNum();
    }

    if (nLayer != SDRLAYER_NOTFOUND)
    {
    }

    BOOL bReturn = pDrView->QueryDrop(rEvt, pWin, nPage, nLayer);

    return (bReturn);
}

/*************************************************************************
|*
|* Drop-Event
|*
\************************************************************************/

BOOL SdDrawViewShell::Drop(const DropEvent& rEvt, SdWindow* pWin,
                           USHORT nPage, USHORT nLayer)
{
    if (nPage != SDRPAGE_NOTFOUND)
    {
        SdPage* pPage = pDoc->GetSdPage(nPage, ePageKind);
        nPage = pPage->GetPageNum();
    }

    if (nLayer != SDRLAYER_NOTFOUND)
    {
    }

    BOOL bReturn = pDrView->Drop(rEvt, pWin, nPage, nLayer);

    return (bReturn);
}

/*************************************************************************
|*
|* PrepareClose, ggfs. Texteingabe beenden, damit andere Viewshells ein
|* aktualisiertes Textobjekt vorfinden
|*
\************************************************************************/

USHORT SdDrawViewShell::PrepareClose( BOOL bUI, BOOL bForBrowsing )
{
    if ( SdViewShell::PrepareClose(bUI, bForBrowsing) != TRUE )
        return FALSE;

    SfxChildWindow* pPreviewChild = GetViewFrame()->GetChildWindow( SdPreviewChildWindow::GetChildWindowId() );
    BOOL            bRet = TRUE;

    if( pFuSlideShow )
    {
        pFuSlideShow->Terminate();
        bRet = FALSE;
    }

    if( pPreviewChild && pPreviewChild->GetWindow() )
    {
        SdPreviewWin*   pPreviewWin = (SdPreviewWin*) pPreviewChild->GetWindow();
        FuSlideShow*    pShow = pPreviewWin ? pPreviewWin->GetSlideShow() : NULL;

        if( pPreviewWin->GetDoc() == pDoc && pShow && pShow->IsInputLocked() )
        {
            pShow->Terminate();
            bRet = FALSE;
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
        aCloseTimer.SetTimeoutHdl( LINK( this, SdDrawViewShell, CloseHdl ) );
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

void SdDrawViewShell::ChangeEditMode(EditMode eEMode, BOOL bLMode)
{
    if (eEditMode != eEMode || bLayerMode != bLMode)
    {
        if ( pDrView->IsTextEdit() )
        {
            pDrView->EndTextEdit();
        }

        aLayerTab.EndEditMode();
        aTabControl.EndEditMode();

        if (ePageKind == PK_HANDOUT)
        {
            // Bei Handzetteln nur MasterPage zulassen
            eEMode = EM_MASTERPAGE;
        }

        eEditMode = eEMode;
        bLayerMode = bLMode;

        if (eEditMode == EM_PAGE)
        {
            /******************************************************************
            * PAGEMODE
            ******************************************************************/
            aPageBtn.Check(TRUE);
            aMasterPageBtn.Check(FALSE);

            aTabControl.Clear();

            SdPage* pPage;
            USHORT nActualPageNum = 0;
            String aPageName;
            USHORT nPageCnt = pDoc->GetSdPageCount(ePageKind);

            for (USHORT i = 0; i < nPageCnt; i++)
            {
                pPage = pDoc->GetSdPage(i, ePageKind);
                aPageName = pPage->GetName();
                aTabControl.InsertPage(i + 1, aPageName);

                if ( pPage->IsSelected() && nActualPageNum == 0 )
                {
                    nActualPageNum = i;
                }
            }

            aTabControl.SetCurPageId(nActualPageNum + 1);

            SwitchPage(nActualPageNum);

            SfxBoolItem aItem(SID_PREVIEW_WIN, pFrameView->IsShowPreviewInPageMode());
            SFX_DISPATCHER().Execute(SID_PREVIEW_WIN, SFX_CALLMODE_ASYNCHRON |
                                     SFX_CALLMODE_RECORD, &aItem, 0L);
        }
        else
        {
            /******************************************************************
            * MASTERPAGE
            ******************************************************************/
            GetViewFrame()->SetChildWindow(SdAnimationChildWindow::GetChildWindowId(), FALSE );

            aPageBtn.Check(FALSE);
            aMasterPageBtn.Check(TRUE);

            if (!pActualPage)
            {
                // Sofern es keine pActualPage gibt, wird die erste genommen
                pActualPage = pDoc->GetSdPage(0, ePageKind);
            }

            SdPage* pPreviewPage = pActualPage;

            aTabControl.Clear();
            USHORT nActualMasterPageNum = 0;
            USHORT nMasterPageCnt = pDoc->GetMasterSdPageCount(ePageKind);

            for (USHORT i = 0; i < nMasterPageCnt; i++)
            {
                SdPage* pMaster = pDoc->GetMasterSdPage(i, ePageKind);
                String aLayoutName(pMaster->GetLayoutName());
                aLayoutName.Erase(aLayoutName.SearchAscii(SD_LT_SEPARATOR));

                aTabControl.InsertPage(i + 1, aLayoutName);

                if (pActualPage->GetMasterPage(0) == pMaster)
                {
                    nActualMasterPageNum = i;
                }
            }

            aTabControl.SetCurPageId(nActualMasterPageNum + 1);
            SwitchPage(nActualMasterPageNum);

            SfxBoolItem aItem(SID_PREVIEW_WIN, pFrameView->IsShowPreviewInMasterPageMode());
            SFX_DISPATCHER().Execute(SID_PREVIEW_WIN, SFX_CALLMODE_ASYNCHRON |
                                     SFX_CALLMODE_RECORD, &aItem, 0L);
        }

        if (bLayerMode)
        {
            /******************************************************************
            * LAYER ein
            ******************************************************************/
            aTabControl.Hide();
            aLayerTab.Show();
            aLayerBtn.Check(TRUE);
        }
        else
        {
            /******************************************************************
            * LAYER aus
            ******************************************************************/
            aTabControl.Show();
            aLayerTab.Hide();
            aLayerBtn.Check(FALSE);
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

/*************************************************************************
|*
|* Groesse des TabControls und der ModeButtons zurueckgeben
|*
\************************************************************************/

long SdDrawViewShell::GetHCtrlWidth()
{
    return ( aTabControl.GetSizePixel().Width() +
             aPageBtn.GetSizePixel().Width() * 3 );
}


/*************************************************************************
|*
|* Horizontales Lineal erzeugen
|*
\************************************************************************/

SvxRuler* SdDrawViewShell::CreateHRuler(SdWindow* pWin, BOOL bIsFirst)
{
    SdRuler* pRuler;
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

    pRuler = new SdRuler(*this, &GetViewFrame()->GetWindow(), pWin, nFlags,
                          GetViewFrame()->GetBindings(), aWBits);
    pRuler->SetSourceUnit(pWin->GetMapMode().GetMapUnit());

    // Metric ...
    UINT16 nMetric = pDoc->GetUIUnit();
    if( nMetric == 0xffff )
        nMetric = SFX_APP()->GetOptions().GetMetric(); // Metric der Applikation
    pRuler->SetUnit( FieldUnit( nMetric ) );

    // ... und auch DefTab am Lineal einstellen
    pRuler->SetDefTabDist( pDoc->GetDefaultTabulator() ); // Neu

    Fraction aUIScale(pWin->GetMapMode().GetScaleX());
    aUIScale *= pDoc->GetUIScale();
    pRuler->SetZoom(aUIScale);

    return pRuler;
}

/*************************************************************************
|*
|* Vertikales Lineal erzeugen
|*
\************************************************************************/

SvxRuler* SdDrawViewShell::CreateVRuler(SdWindow* pWin)
{
    SdRuler* pRuler;
    WinBits  aWBits = WB_VSCROLL | WB_3DLOOK | WB_BORDER;
    USHORT   nFlags = SVXRULER_SUPPORT_OBJECT;

    pRuler = new SdRuler(*this, &GetViewFrame()->GetWindow(), pWin, nFlags,
                          GetViewFrame()->GetBindings(), aWBits);
    pRuler->SetSourceUnit(pWin->GetMapMode().GetMapUnit());

    // Metric am Lineal einstellen
    SdOptions* pOptions = SD_MOD()->GetSdOptions( pDoc->GetDocumentType() );
    UINT16 nMetric = pOptions->GetMetric();
    if( nMetric == 0xffff )
        nMetric = SFX_APP()->GetOptions().GetMetric(); // Metric der Applikation
    pRuler->SetUnit( FieldUnit( nMetric ) );

    Fraction aUIScale(pWin->GetMapMode().GetScaleY());
    aUIScale *= pDoc->GetUIScale();
    pRuler->SetZoom(aUIScale);

    return pRuler;
}

/*************************************************************************
|*
|* Horizontales Lineal aktualisieren
|*
\************************************************************************/

void SdDrawViewShell::UpdateHRuler()
{
    Invalidate( SID_ATTR_LONG_LRSPACE );
    Invalidate( SID_RULER_PAGE_POS );
//  Invalidate( SID_RULER_OBJECT );

    for (USHORT nIndex=0; nIndex<MAX_HSPLIT_CNT; nIndex++)
    {
        if (pHRulerArray[nIndex])
        {
            pHRulerArray[nIndex]->ForceUpdate();
        }
    }
}

/*************************************************************************
|*
|* Vertikales Lineal aktualisieren
|*
\************************************************************************/

void SdDrawViewShell::UpdateVRuler()
{
    Invalidate( SID_ATTR_LONG_LRSPACE );
    Invalidate( SID_RULER_PAGE_POS );
//  Invalidate( SID_RULER_OBJECT );

    for (USHORT nIndex=0; nIndex<MAX_VSPLIT_CNT; nIndex++)
    {
        if (pVRulerArray[nIndex])
        {
            pVRulerArray[nIndex]->ForceUpdate();
        }
    }
}

/*************************************************************************
|*
|* Metrik setzen
|*
\************************************************************************/

void SdDrawViewShell::SetUIUnit(FieldUnit eUnit)
{
    SdViewShell::SetUIUnit(eUnit);
}

/*************************************************************************
|*
|* TabControl nach Splitteraenderung aktualisieren
|*
\************************************************************************/

IMPL_LINK( SdDrawViewShell, TabSplitHdl, TabBar *, pTab )
{
    long nMax = aHSplit.GetPosPixel().X() - aTabControl.GetPosPixel().X();

    Size aTabSize = aTabControl.GetSizePixel();
    aTabSize.Width() = Min(pTab->GetSplitSize(), (long)(nMax-1));

    aTabControl.SetSizePixel(aTabSize);
    aLayerTab.SetSizePixel(aTabSize);

    Point aPos = aTabControl.GetPosPixel();
    aPos.X() += aTabSize.Width();

    Size aScrSize(nMax - aTabSize.Width(), aScrBarWH.Height());
    pHScrlArray[0]->SetPosSizePixel(aPos, aScrSize);

    return 0;
}

/*************************************************************************
|*
|* neue aktuelle Seite auswaehlen, falls sich die Seitenfolge geaendert
|* hat (z. B. durch Undo)
|*
\************************************************************************/

void SdDrawViewShell::ResetActualPage()
{
    USHORT nCurrentPage = aTabControl.GetCurPageId() - 1;
    USHORT nPageCount   = (eEditMode == EM_PAGE)?pDoc->GetSdPageCount(ePageKind):pDoc->GetMasterSdPageCount(ePageKind);
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
            pPage = pDoc->GetSdPage(i, ePageKind);
            aPageName = pPage->GetName();
            aTabControl.InsertPage(i + 1, aPageName);

            // Selektionskennungen der Seiten korrigieren
            pDoc->SetSelected(pPage, i == nCurrentPage);
        }

        aTabControl.SetCurPageId(nCurrentPage + 1);
    }
    else // EM_MASTERPAGE
    {
        SdPage* pActualPage = pDoc->GetMasterSdPage(nCurrentPage, ePageKind);
        aTabControl.Clear();
        USHORT nActualMasterPageNum = 0;

        USHORT nMasterPageCnt = pDoc->GetMasterSdPageCount(ePageKind);
        for (USHORT i = 0; i < nMasterPageCnt; i++)
        {
            SdPage* pMaster = pDoc->GetMasterSdPage(i, ePageKind);
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


ErrCode __EXPORT SdDrawViewShell::DoVerb(long nVerb)
{
    if ( pDrView->HasMarkedObj() )
    {
        const SdrMarkList& rMarkList = pDrView->GetMarkList();

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
#ifndef SO3
                    SvInPlaceObjectRef aNewIPObj = &SvInPlaceObject::ClassFactory()
                    ->CreateAndInit(SimModuleDummy::GetID(SOFFICE_FILEFORMAT_50), aStor);
#else
                    SvInPlaceObjectRef aNewIPObj = &((SvFactory*)SvInPlaceObject::ClassFactory())
                    ->CreateAndInit(SimModuleDummy::GetID(SOFFICE_FILEFORMAT_50), aStor);
#endif
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

                        SdClient* pClient = (SdClient*) GetIPClient();

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

BOOL SdDrawViewShell::ActivateObject(SdrOle2Obj* pObj, long nVerb)
{
    BOOL bActivated = FALSE;

    if ( !pDocSh->IsUIActive() )
    {
        bActivated = SdViewShell::ActivateObject(pObj, nVerb);

        SdClient* pClient = (SdClient*) GetIPClient();

        if (pClient)
        {
            pClient->SetSdrGrafObj(NULL);
        }

        if (bActivated)
        {
            aTabControl.Disable();
            aLayerTab.Disable();
            aDrawBtn.Disable();
            aNotesBtn.Disable();
            aHandoutBtn.Disable();
            aOutlineBtn.Disable();
            aSlideBtn.Disable();
            aPresentationBtn.Disable();
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

BOOL SdDrawViewShell::SwitchPage(USHORT nSelectedPage)
{
    BOOL bOK = FALSE;

    if (IsSwitchPageAllowed())
    {
        bOK = TRUE;

        BOOL bIsChanged = pDoc->IsChanged();

        if (pActualPage)
        {
            SdPage* pNewPage = pDoc->GetSdPage(nSelectedPage, ePageKind);

            if (pActualPage == pNewPage)
            {
                if (eEditMode == EM_MASTERPAGE)
                {
                    pNewPage = (SdPage*) pNewPage->GetMasterPageNum(0);
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
            pActualPage = pDoc->GetSdPage(nSelectedPage, ePageKind);
        }
        else
        {
            SdPage* pMaster = pDoc->GetMasterSdPage(nSelectedPage, ePageKind);

            // Passt die selektierte Seite zur MasterPage?
            USHORT nPageCount = pDoc->GetSdPageCount(ePageKind);
            for (USHORT i = 0; i < nPageCount; i++)
            {
                SdPage* pPage = pDoc->GetSdPage(i, ePageKind);
                if(pPage && pPage->IsSelected() && pPage->GetMasterPage(0) == pMaster)
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
                    SdPage* pPage = pDoc->GetSdPage(i, ePageKind);
                    if(pPage && pPage->GetMasterPage(0) == pMaster)
                    {
                        pActualPage = pPage;
                        break;
                    }
                }
            }
        }

        for (USHORT i = 0; i < pDoc->GetSdPageCount(ePageKind); i++)
        {
            // Alle Seiten deselektieren
            pDoc->SetSelected( pDoc->GetSdPage(i, ePageKind), FALSE);
        }

        if (!pActualPage)
        {
            // Sofern es keine pActualPage gibt, wird die erste genommen
            pActualPage = pDoc->GetSdPage(0, ePageKind);
        }

        // diese Seite auch selektieren (pActualPage zeigt immer auf Zeichenseite,
        // nie auf eine Masterpage)
        pDoc->SetSelected(pActualPage, TRUE);

        // VisArea zuziehen, um ggf. Objekte zu deaktivieren
        DisconnectAllClients();
        VisAreaChanged(Rectangle(Point(), Size(1, 1)));

        if (eEditMode == EM_PAGE)
        {
            /**********************************************************************
            * PAGEMODE
            **********************************************************************/
            pActualPage->SetAutoLayout( pActualPage->GetAutoLayout() );
            pDoc->SetSelected(pActualPage, TRUE);

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

            SdPage* pMaster = pDoc->GetMasterSdPage(nSelectedPage, ePageKind);

            if( !pMaster )              // Falls es diese Page nicht geben sollte
                pMaster = pDoc->GetMasterSdPage(0, ePageKind);

            USHORT nNum = pMaster->GetPageNum();
            pDrView->ShowMasterPagePgNum(nNum, Point(0, 0));

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
        }

        Size aVisSizePixel = pWindow->GetOutputSizePixel();
        Rectangle aVisAreaWin = pWindow->PixelToLogic( Rectangle( Point(0,0), aVisSizePixel) );
        VisAreaChanged(aVisAreaWin);
        pDrView->VisAreaChanged(pWindow);

        // Damit der Navigator (und das Effekte-Window) das mitbekommt (/-men)
        SFX_BINDINGS().Invalidate(SID_NAVIGATOR_PAGENAME, TRUE, FALSE);
        SFX_BINDINGS().Invalidate(SID_EFFECT_STATE, TRUE, FALSE);
        UpdateSlideChangeWindow();

        // ggfs. Preview den neuen Kontext mitteilen
        SfxChildWindow* pPreviewChildWindow =
            GetViewFrame()->GetChildWindow(SdPreviewChildWindow::GetChildWindowId());
        if (pPreviewChildWindow)
        {
            SdPreviewWin* pPreviewWin =
                (SdPreviewWin*)pPreviewChildWindow->GetWindow();
            if (pPreviewWin && pPreviewWin->GetDoc() == pDoc)
            {
                USHORT nSdPageNo = (pActualPage->GetPageNum() - 1) / 2;
                pPreviewWin->SetContext(pDoc, nSdPageNo, pFrameView);
            }
        }

        if (pDrView->GetSlideShow())
        {
            pDrView->SetAnimationMode(TRUE);
        }

        pDoc->SetChanged(bIsChanged);
    }

    return (bOK);
}


/*************************************************************************
|*
|* Pruefen, ob ein Seitenwechsel erlaubt ist
|*
\************************************************************************/

BOOL SdDrawViewShell::IsSwitchPageAllowed() const
{
    BOOL bOK = TRUE;

    FmFormShell* pShell = (FmFormShell*) aShellTable.Get(RID_FORMLAYER_TOOLBOX);

    if (pShell && !pShell->PrepareClose(FALSE))
    {
        bOK = FALSE;
    }

    return( bOK && !bInEffectAssignment );
}

/*************************************************************************
|*
|* neue aktuelle Seite auswaehlen, falls sich die Seitenfolge geaendert
|* hat (z. B. durch Undo)
|*
\************************************************************************/

void SdDrawViewShell::ResetActualLayer()
{
    /*************************************************************
    * Update fuer LayerTab
    *************************************************************/
    aLayerTab.Clear();

    String aName;
    String aActiveLayer = pDrView->GetActiveLayer();
    String aBackgroundLayer( SdResId(STR_LAYER_BCKGRND) );
    String aBackgroundObjLayer( SdResId(STR_LAYER_BCKGRNDOBJ) );
    String aLayoutLayer( SdResId(STR_LAYER_LAYOUT) );
    String aControlsLayer( SdResId(STR_LAYER_CONTROLS) );
    String aMeasureLinesLayer( SdResId(STR_LAYER_MEASURELINES) );
    USHORT nNewLayer = 0;
    USHORT nActiveLayer = SDRLAYER_NOTFOUND;
    SdrLayerAdmin& rLayerAdmin = pDoc->GetLayerAdmin();
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
                    aLayerTab.InsertPage(nLayer+1, aName);

                    TabBarPageBits nBits = 0;
                    SdrPageView* pPV = pDrView->GetPageViewPvNum(0);

                    if (pPV && !pPV->IsLayerVisible(aName))
                    {
                        // Unsichtbare Layer werden anders dargestellt
                        nBits = TPB_SPECIAL;
                    }

                    aLayerTab.SetPageBits(nLayer+1, nBits);
                }
            }
            else
            {
                // Layer der MasterPage nicht auf Page anzeigen
                if ( aName != aBackgroundObjLayer )
                {
                    aLayerTab.InsertPage(nLayer+1, aName);

                    TabBarPageBits nBits = 0;

                    if (!pDrView->GetPageViewPvNum(0)->IsLayerVisible(aName))
                    {
                        // Unsichtbare Layer werden anders dargestellt
                        nBits = TPB_SPECIAL;
                    }

                    aLayerTab.SetPageBits(nLayer+1, nBits);
                }
            }
        }
    }

    if ( nActiveLayer == SDRLAYER_NOTFOUND )
    {
        nActiveLayer = ( eEditMode == EM_MASTERPAGE ) ? 2 : 0;
        pDrView->SetActiveLayer( aLayerTab.GetPageText(nActiveLayer + 1) );
    }

    aLayerTab.SetCurPageId(nActiveLayer + 1);
    SFX_BINDINGS().Invalidate( SID_MODIFYLAYER );
}

/*************************************************************************
|*
|* Verzoegertes Close ausfuehren
|*
\************************************************************************/

IMPL_LINK( SdDrawViewShell, CloseHdl, Timer*, pTimer )
{
    pTimer->Stop();
    GetViewFrame()->GetDispatcher()->Execute( SID_CLOSEWIN, SFX_CALLMODE_ASYNCHRON );
    return 0L;
}

/*************************************************************************
|*
|* Setzt die HelpId in Abhaengigkeit von der Selektion
|*
\************************************************************************/

void SdDrawViewShell::SetHelpIdBySelection()
{
    UINT32 nHelpId = 0;
    const SdrMarkList& rMarkList = pDrView->GetMarkList();

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

    if( nHelpId != 0 && SFX_APP()->GetHelpPI() )
        SfxHelp::ShowHelp( nHelpId, TRUE, 0, TRUE );
}


#ifdef WNT
#pragma optimize ( "", on )
#endif


