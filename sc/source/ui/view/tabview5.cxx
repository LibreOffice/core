/*************************************************************************
 *
 *  $RCSfile: tabview5.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:45:10 $
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

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

//------------------------------------------------------------------

#ifndef OS2
#define _MACRODLG_HXX
#define _BIGINT_HXX
#define _SVCONTNR_HXX
#define BASIC_NODIALOGS
#define _SFXMNUITEM_HXX
#define _SFXMNUITEM_HXX
#define _DLGCFG_HXX
#define _SFXMNUMGR_HXX
#define _SFXBASIC_HXX
#define _MODALDLG_HXX
#define _SFX_TEMPLDLG_HXX
#define _SFXSTBMGR_HXX
#define _SFXTBXMGR_HXX
#define _BASE_DLGS_HXX
#define _SFXIMGMGR_HXX
#define _SFXMNUMGR_HXX
#define _SFXSTBITEM_HXX
#define _SFXTBXCTRL_HXX
#define _PASSWD_HXX
//#define _SFXFILEDLG_HXX
//#define _SFXREQUEST_HXX
#define _SFXOBJFACE_HXX

// TOOLS
#define _BIGINT_HXX
#define _SFXMULTISEL_HXX
#define _STACK_HXX
#define _QUEUE_HXX
#define _DYNARR_HXX
#define _TREELIST_HXX
#define _CACHESTR_HXX

//SV
//#define _CLIP_HXX
#define _CONFIG_HXX
#define _CURSOR_HXX
#define _FONTDLG_HXX
#define _PRVWIN_HXX
//#define _COLOR_HXX
//#define _PAL_HXX
//#define _BITMAP_HXX
//#define _GDIOBJ_HXX
//#define _POINTR_HXX
//#define _ICON_HXX
//#define _IMAGE_HXX
//#define _KEYCOD_HXX
//#define _EVENT_HXX
#define _HELP_HXX
//#define _APP_HXX
//#define _MDIAPP_HXX
//#define _TIMER_HXX
//#define _METRIC_HXX
//#define _REGION_HXX
//#define _OUTDEV_HXX
//#define _SYSTEM_HXX
//#define _VIRDEV_HXX
//#define _JOBSET_HXX
//#define _PRINT_HXX
//#define _WINDOW_HXX
//#define _SYSWIN_HXX
//#define _WRKWIN_HXX
#define _MDIWIN_HXX
//#define _FLOATWIN_HXX
//#define _DOCKWIN_HXX
//#define _CTRL_HXX
//#define _SCRBAR_HXX
//#define _BUTTON_HXX
//#define _IMAGEBTN_HXX
//#define _FIXED_HXX
//#define _GROUP_HXX
//#define _EDIT_HXX
//#define _COMBOBOX_HXX
#define _LSTBOX_HXX //???
//#define _SELENG_HXX ***
//#define _SPLIT_HXX ***
#define _SPIN_HXX
//#define _FIELD_HXX ***
//#define _MOREBTN_HXX ***
#define _TOOLBOX_HXX //???
//#define _STATUS_HXX ***
//#define _DIALOG_HXX ***
#define _MSGBOX_HXX //???
#define _SYSDLG_HXX //???
#define _FILDLG_HXX
#define _PRNDLG_HXX //???
#define _COLDLG_HXX
#define _TABDLG_HXX //???
#ifdef WIN
#define _MENU_HXX
#endif
//#define _GDIMTF_HXX ***
//#define _POLY_HXX ***
#define _ACCEL_HXX //???
#define _GRAPH_HXX //???
#define _SOUND_HXX



#ifdef WIN
#define _MENUBTN_HXX
#endif

//svtools
#define _SCRWIN_HXX
#define _RULER_HXX
//#define _TABBAR_HXX  ***
#define _VALUESET_HXX //???
#define _STDMENU_HXX
#define _STDCTRL_HXX
#define _CTRLBOX_HXX
#define _CTRLTOOL_HXX
#define _EXTATTR_HXX
#define _FRM3D_HXX

//SVTOOLS
#define _SVTREELIST_HXX
#define _FILTER_HXX
#define _SVLBOXITM_HXX
#define _SVTREEBOX_HXX
#define _SVICNVW_HXX
#define _SVTABBX_HXX

#define _SFX_DOCFILT_HXX //???
#define _SFX_PRNMON_HXX
#define _SFX_RESMGR_HXX
#define _SFX_TEMPLDLG_HXX
#define _SFXAPPWIN_HXX
#define _SFXBASIC_HXX
#define _SFXCTRLITEM
#define _SFXDLGCFG_HXX
//#define _SFXDISPATCH_HXX ***
#define _SFXDOCFILE_HXX
#define _SFXDOCMAN_HXX
#define _SFXDOCMGR_HXX
#define _SFXDOCTDLG_HXX
#define _SFXFILEDLG_HXX
#define _SFXIMGMGR_HXX
#define _SFXIPFRM_HXX
#define _SFX_MACRO_HXX
#define _SFXMNUITEM_HXX
#define _SFXMNUMGR_HXX
#define _SFXMULTISEL_HXX
//#define _SFXMSG_HXX ***
#define _SFXMSGDESCR_HXX
#define _SFXMSGPOOL_HXX
#define _SFX_MINFITEM_HXX
#define _SFXOBJFACE_HXX
//#define _SFXOBJFAC_HXX ???
#define _SFX_SAVEOPT_HXX
#define _SFXSTBITEM_HXX
#define _SFXSTBMGR_HXX
#define _SFXTBXCTRL_HXX
#define _SFXTBXMGR_HXX

//sfxdoc.hxx
//#define _SFX_OBJSH_HXX ***
//#define _SFX_CLIENTSH_HXX ***
#define _SFXDOCINF_HXX //???
#define _SFX_DOCFILT_HXX
//#define _SFXDOCFILE_HXX ***
//#define _VIEWFAC_HXX ???
//#define _SFXVIEWFRM_HXX  ***
//#define _SFXVIEWSH_HXX ***
//#define _MDIFRM_HXX ***
#define _SFX_IPFRM_HXX
#define _SFX_INTERNO_HXX

//sfxdlg.hxx
//#define _SFXTABDLG_HXX ***
//#define _BASEDLGS_HXX ***
#define _SFX_DINFDLG_HXX
#define _SFXDINFEDT_HXX
#define _SFX_MGETEMPL_HXX
#define _SFX_TPLPITEM_HXX
#define _SFX_STYLEDLG_HXX
#define _NEWSTYLE_HXX
//#define _SFXDOCTEMPL_HXX ***
//#define _SFXDOCTDLG_HXX ***
//#define _SFX_TEMPLDLG_HXX ***
//#define _SFXNEW_HXX ***
#define _SFXDOCMAN_HXX
//#define _SFXDOCKWIN_HXX ***

//sfxitems.hxx
#define _SFX_WHMAP_HXX
#define _ARGS_HXX
//#define _SFXPOOLITEM_HXX ***
//#define _SFXINTITEM_HXX ***
//#define _SFXENUMITEM_HXX  ***
#define _SFXFLAGITEM_HXX
//#define _SFXSTRITEM_HXX ***
#define _SFXPTITEM_HXX
#define _SFXRECTITEM_HXX
//#define _SFXITEMPOOL_HXX  ***
//#define _SFXITEMSET_HXX  ***
#define _SFXITEMITER_HXX
#define _SFX_WHITER_HXX
#define _SFXPOOLCACH_HXX
#define _AEITEM_HXX
#define _SFXRNGITEM_HXX
#define _SFXSLSTITM_HXX
//#define _SFXSTYLE_HXX ***

//xout.hxx
//#define _XENUM_HXX
//#define _XPOLY_HXX
//#define _XATTR_HXX
#define _XOUTX_HXX
//#define _XPOOL_HXX ***
#define _XTABLE_HXX

//svdraw.hxx
#define _SDR_NOITEMS
#define _SDR_NOTOUCH
#define _SDR_NOTRANSFORM
#define _SDR_NOOBJECTS
//#define _SDR_NOVIEWS ***

//#define _VCATTR_HXX
#define _VCONT_HXX
#define _SI_NOOTHERFORMS
#define _VCTRLS_HXX
#define _SI_NOCONTROL
#define _SETBRW_HXX
#define _VCBRW_HXX
#define _SI_NOSBXCONTROLS
#define _SVBOXITM_HXX

#define _SVDRAG_HXX
#define _SVINCVW_HXX
#define _SV_MULTISEL_HXX
#define _SVRTV_HXX
#define _SVTABBX_HXX

#define _SVX_DAILDLL_HXX
#define _SVX_HYPHEN_HXX
#define _SVX_IMPGRF_HXX
#define _SVX_OPTITEMS_HXX
#define _SVX_OPTGERL_HXX
#define _SVX_OPTSAVE_HXX
#define _SVX_OPTSPELL_HXX
#define _SVX_OPTPATH_HXX
#define _SVX_OPTLINGU_HXX
#define _SVX_RULER_HXX
#define _SVX_RULRITEM_HXX
#define _SVX_SPLWRAP_HXX
#define _SVX_SPLDLG_HXX
#define _SVX_THESDLG_HXX

//#define _SI_HXX
#define _SI_DLL_HXX
#define _SIDLL_HXX
#define _SI_NOITEMS
#define _SI_NOOTHERFORMS
#define _SI_NOSBXCONTROLS
#define _SINOSBXCONTROLS
#define _SI_NOCONTROL
#define _VCBRW_HXX
#define _VCTRLS_HXX
#define _VCONT_HXX
#define _VDRWOBJ_HXX
#define _VCATTR_HXX
#define _VCONT_HXX

#define _SDR_NOTRANSFORM
#define _SDR_NOITEMS
#define _SDR_NOOBJECTS
#define _SVDXOUT_HXX
#endif

// INCLUDE ---------------------------------------------------------------

#include "scitems.hxx"
#include <svx/fmshell.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdoutl.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/objsh.hxx>
#include <tools/ref.hxx>

#include "tabview.hxx"
#include "tabvwsh.hxx"
#include "document.hxx"
#include "gridwin.hxx"
#include "olinewin.hxx"
#include "tabsplit.hxx"
#include "colrowba.hxx"
#include "tabcont.hxx"
#include "hintwin.hxx"
#include "sc.hrc"
#include "pagedata.hxx"
#include "hiranges.hxx"
#include "drawview.hxx"
#include "drwlayer.hxx"
#include "fusel.hxx"                // Start-Function


// STATIC DATA -----------------------------------------------------------


void __EXPORT ScTabView::Init()
{
    USHORT i;

    aScrollTimer.SetTimeout(10);
    aScrollTimer.SetTimeoutHdl( LINK( this, ScTabView, TimerHdl ) );

    for (i=0; i<4; i++)
        pGridWin[i] = NULL;
    pGridWin[SC_SPLIT_BOTTOMLEFT] = new ScGridWindow( pFrameWin, &aViewData, SC_SPLIT_BOTTOMLEFT );

    pSelEngine = new ScViewSelectionEngine( pGridWin[SC_SPLIT_BOTTOMLEFT], this,
                                                SC_SPLIT_BOTTOMLEFT );
    aFunctionSet.SetSelectionEngine( pSelEngine );

    pHdrSelEng = new ScHeaderSelectionEngine( pFrameWin, &aHdrFunc );

    pColBar[SC_SPLIT_LEFT] = new ScColBar( pFrameWin, &aViewData, SC_SPLIT_LEFT,
                                                &aHdrFunc, pHdrSelEng );
    pColBar[SC_SPLIT_RIGHT] = NULL;
    pRowBar[SC_SPLIT_BOTTOM] = new ScRowBar( pFrameWin, &aViewData, SC_SPLIT_BOTTOM,
                                                &aHdrFunc, pHdrSelEng );
    pRowBar[SC_SPLIT_TOP] = NULL;
    for (i=0; i<2; i++)
        pColOutline[i] = pRowOutline[i] = NULL;

    pHSplitter = new ScTabSplitter( pFrameWin, WinBits( WB_HSCROLL ), &aViewData );
    pVSplitter = new ScTabSplitter( pFrameWin, WinBits( WB_VSCROLL ), &aViewData );

    pTabControl = new ScTabControl( pFrameWin, &aViewData );
    //MI: nie! das war mal eine MUSS-Aenderung von MBA
    //if (!aViewData.IsBasicView())
    //  pTabControl->ToTop();       // ueber dem Splitter

    InitScrollBar( aHScrollLeft,    MAXCOL+1 );
    InitScrollBar( aHScrollRight,   MAXCOL+1 );
    InitScrollBar( aVScrollTop,     MAXROW+1 );
    InitScrollBar( aVScrollBottom,  MAXROW+1 );

    //  Hier noch nichts anzeigen (Show), weil noch falsch angeordnet ist
    //  Show kommt dann aus UpdateShow beim ersten Resize
    //      pTabControl, pGridWin, aHScrollLeft, aVScrollBottom,
    //      aCornerButton, aScrollBarBox, pHSplitter, pVSplitter

    //      Splitter

    pHSplitter->SetSplitHdl( LINK( this, ScTabView, SplitHdl ) );
    pVSplitter->SetSplitHdl( LINK( this, ScTabView, SplitHdl ) );

    //  UpdateShow kommt beim Resize, oder bei Kopie einer bestehenden View aus dem ctor

    pDrawActual = NULL;
    pDrawOld    = NULL;

            //  DrawView darf nicht im TabView - ctor angelegt werden,
            //  wenn die ViewShell noch nicht konstruiert ist...
            //  Das gilt auch fuer ViewOptionsHasChanged()

    TestHintWindow();
}

__EXPORT ScTabView::~ScTabView()
{
    USHORT i;

    DELETEZ(pPageBreakData);
    DELETEZ(pHighlightRanges);

    DELETEZ(pDrawOld);
    DELETEZ(pDrawActual);

    aViewData.KillEditView();           // solange GridWin's noch existieren

    DELETEZ(pInputHintWindow);

    if (pDrawView)
    {
        for (i=0; i<4; i++)
            if (pGridWin[i])
            {
                pDrawView->VCRemoveWin(pGridWin[i]);
                pDrawView->DelWin(pGridWin[i]);
            }

        pDrawView->HidePagePvNum(0);
        delete pDrawView;
    }

    delete pSelEngine;

    for (i=0; i<4; i++)
        delete pGridWin[i];

    delete pHdrSelEng;

    for (i=0; i<2; i++)
    {
        delete pColBar[i];
        delete pRowBar[i];
        delete pColOutline[i];
        delete pRowOutline[i];
    }

    delete pHSplitter;
    delete pVSplitter;

    delete pTabControl;
}

void ScTabView::MakeDrawView()
{
    if (!pDrawView)
    {
        ScDrawLayer* pLayer = aViewData.GetDocument()->GetDrawLayer();
        DBG_ASSERT(pLayer, "wo ist der DrawLayer ??");

        USHORT i;
        pDrawView = new ScDrawView( pGridWin[SC_SPLIT_BOTTOMLEFT], &aViewData );
        for (i=0; i<4; i++)
            if (pGridWin[i])
            {
                if ( SC_SPLIT_BOTTOMLEFT != (ScSplitPos)i )
                    pDrawView->AddWin(pGridWin[i]);
                pDrawView->VCAddWin(pGridWin[i]);
            }
        pDrawView->RecalcScale();
        for (i=0; i<4; i++)
            if (pGridWin[i])
            {
                pGridWin[i]->SetMapMode(pGridWin[i]->GetDrawMapMode());

                pGridWin[i]->Update();      // wegen Invalidate im DrawView ctor (ShowPage),
                                            // damit gleich gezeichnet werden kann
            }
        SfxRequest aSfxRequest(SID_OBJECT_SELECT, 0,aViewData.GetViewShell()->GetPool());
        SetDrawFuncPtr(new FuSelection( aViewData.GetViewShell(), GetActiveWin(), pDrawView,
                                        pLayer,aSfxRequest));

        //  an der FormShell anmelden
        FmFormShell* pFormSh = aViewData.GetViewShell()->GetFormShell();
        if (pFormSh)
            pFormSh->SetView(pDrawView);
    }
}

void ScTabView::DoAddWin( ScGridWindow* pWin )
{
    if (pDrawView)
    {
        pDrawView->AddWin(pWin);
        pDrawView->VCAddWin(pWin);
    }
}

//==================================================================

void ScTabView::TabChanged()
{
    if (pDrawView)
    {
        DrawDeselectAll();      // beendet auch Text-Edit-Modus

        USHORT i;
        for (i=0; i<4; i++)
            if (pGridWin[i])
                pDrawView->VCRemoveWin(pGridWin[i]);    // fuer alte Page

        USHORT nTab = aViewData.GetTabNo();
        pDrawView->HideAllPages();
        pDrawView->ShowPagePgNum( nTab, Point() );

        UpdateLayerLocks();

        pDrawView->RecalcScale();
        pDrawView->UpdateWorkArea();    // #54782# PageSize ist pro Page unterschiedlich

        for (i=0; i<4; i++)
            if (pGridWin[i])
                pDrawView->VCAddWin(pGridWin[i]);       // fuer neue Page
    }

    SfxBindings& rBindings = SFX_BINDINGS();

    //  Es gibt keine einfache Moeglichkeit, alle Slots der FormShell zu invalidieren
    //  (fuer disablete Slots auf geschuetzten Tabellen), darum hier einfach alles...
    rBindings.InvalidateAll(FALSE);

#if 0
    rBindings.Invalidate( SID_SELECT_SCENARIO );
    rBindings.Invalidate( FID_PROTECT_TABLE );
    rBindings.Invalidate( FID_DELETE_TABLE );
    rBindings.Invalidate( FID_TABLE_SHOW );
    rBindings.Invalidate( FID_TABLE_HIDE );

                // Auswirkungen von geschuetzten Tabellen.
    rBindings.Invalidate( FID_TAB_RENAME );
    rBindings.Invalidate( FID_TAB_MOVE );
    rBindings.Invalidate( SID_DEL_ROWS );
    rBindings.Invalidate( SID_DEL_COLS );
    rBindings.Invalidate( FID_INS_ROW );
    rBindings.Invalidate( FID_INS_COLUMN );
    rBindings.Invalidate( FID_INS_CELL );
    rBindings.Invalidate( FID_INS_CELLSDOWN );
    rBindings.Invalidate( FID_INS_CELLSRIGHT );
    rBindings.Invalidate( FID_DELETE_CELL );

    rBindings.Invalidate( SID_OPENDLG_CHART );
    rBindings.Invalidate( SID_INSERT_OBJECT );
    rBindings.Invalidate( SID_INSERT_DIAGRAM );
    rBindings.Invalidate( SID_INSERT_SIMAGE );
    rBindings.Invalidate( SID_INSERT_SMATH );
    rBindings.Invalidate( SID_INSERT_GRAPHIC );
#endif
}

void ScTabView::UpdateLayerLocks()
{
    if (pDrawView)
    {
        USHORT nTab = aViewData.GetTabNo();
        BOOL bEx = aViewData.GetViewShell()->IsDrawSelMode();
        BOOL bProt = aViewData.GetDocument()->IsTabProtected( nTab ) ||
                     aViewData.GetSfxDocShell()->IsReadOnly();

        SdrLayer* pLayer;
        SdrLayerAdmin& rAdmin = pDrawView->GetModel()->GetLayerAdmin();
        pLayer = rAdmin.GetLayerPerID(SC_LAYER_BACK);
        if (pLayer)
            pDrawView->SetLayerLocked( pLayer->GetName(), bProt || !bEx );
        pLayer = rAdmin.GetLayerPerID(SC_LAYER_INTERN);
        if (pLayer)
            pDrawView->SetLayerLocked( pLayer->GetName(), TRUE );
        pLayer = rAdmin.GetLayerPerID(SC_LAYER_FRONT);
        if (pLayer)
            pDrawView->SetLayerLocked( pLayer->GetName(), bProt );
        pLayer = rAdmin.GetLayerPerID(SC_LAYER_CONTROLS);
        if (pLayer)
            pDrawView->SetLayerLocked( pLayer->GetName(), bProt );
    }
}

void ScTabView::DrawDeselectAll()
{
    if (pDrawView)
    {
        ScTabViewShell* pViewSh = aViewData.GetViewShell();
        if ( pDrawActual &&
            ( pViewSh->IsDrawTextShell() || pDrawActual->GetSlotID() == SID_DRAW_NOTEEDIT ) )
        {
            // end text edit (as if escape pressed, in FuDraw)
            SFX_DISPATCHER().Execute( pDrawActual->GetSlotID(),
                                        SFX_CALLMODE_SLOT | SFX_CALLMODE_RECORD );
        }

        pDrawView->ScEndTextEdit();
        pDrawView->UnmarkAll();

        if (!pViewSh->IsDrawSelMode())
            pViewSh->SetDrawShell( FALSE );
    }
}

BOOL ScTabView::IsDrawTextEdit() const
{
    if (pDrawView)
        return pDrawView->IsTextEdit();
    else
        return FALSE;
}

String ScTabView::GetSelectedChartName() const
{
    if (pDrawView)
        return pDrawView->GetSelectedChartName();
    else
        return EMPTY_STRING;
}

void ScTabView::SetZoom( const Fraction& rNewX, const Fraction& rNewY )
{
    aViewData.SetZoom(rNewX,rNewY);
    if (pDrawView)
        pDrawView->RecalcScale();
    ZoomChanged();              // einzeln wegen CLOOKs
}

void ScTabView::SetPagebreakMode( BOOL bSet )
{
    aViewData.SetPagebreakMode(bSet);
    if (pDrawView)
        pDrawView->RecalcScale();
    ZoomChanged();              // einzeln wegen CLOOKs
}

void ScTabView::ResetDrawDragMode()
{
    if (pDrawView)
        pDrawView->SetDragMode( SDRDRAG_MOVE );
}

void ScTabView::ViewOptionsHasChanged( BOOL bHScrollChanged, BOOL bGraphicsChanged )
{
    //  DrawView erzeugen, wenn Gitter angezeigt werden soll
    if ( !pDrawView && aViewData.GetOptions().GetGridOptions().GetGridVisible() )
        MakeDrawLayer();

    if (pDrawView)
        pDrawView->UpdateUserViewOptions();

    if (bGraphicsChanged)
        DrawEnableAnim(TRUE);   // DrawEnableAnim checks the options state

    // if TabBar is set to visible, make sure its size is not 0
    BOOL bGrow = ( aViewData.IsTabMode() && pTabControl->GetSizePixel().Width() <= 0 );

    // if ScrollBar is set to visible, TabBar must make room
    BOOL bShrink = ( bHScrollChanged && aViewData.IsTabMode() && aViewData.IsHScrollMode() &&
                        pTabControl->GetSizePixel().Width() > SC_TABBAR_DEFWIDTH );

    if ( bGrow || bShrink )
    {
        Size aSize = pTabControl->GetSizePixel();
        aSize.Width() = SC_TABBAR_DEFWIDTH;             // initial size
        pTabControl->SetSizePixel(aSize);               // DoResize is called later...
    }
}

// Helper-Funktion gegen das Include des Drawing Layers

SdrView* ScTabView::GetSdrView()
{
    return pDrawView;
}

void ScTabView::DrawMarkListHasChanged()
{
    if ( pDrawView )
        pDrawView->MarkListHasChanged();
}

void ScTabView::UpdateAnchorHandles()
{
    if ( pDrawView )
        pDrawView->AdjustMarkHdl();
}

void ScTabView::UpdateIMap( SdrObject* pObj )
{
    if ( pDrawView )
        pDrawView->UpdateIMap( pObj );
}

void ScTabView::DrawMarkRect( const Rectangle& rRect )
{
    //! store rectangle for repaint during drag

    for (USHORT i=0; i<4; i++)
    {
        if ( pGridWin[i] && pGridWin[i]->IsVisible() )
        {
            RasterOp aROp = pGridWin[i]->GetRasterOp();
            BOOL bHasLine = pGridWin[i]->IsLineColor();
            Color aLine   = pGridWin[i]->GetLineColor();
            BOOL bHasFill = pGridWin[i]->IsFillColor();
            Color aFill   = pGridWin[i]->GetFillColor();

            pGridWin[i]->SetRasterOp( ROP_INVERT );
            pGridWin[i]->SetLineColor( COL_BLACK );
            pGridWin[i]->SetFillColor();

            pGridWin[i]->DrawRect(rRect);

            pGridWin[i]->SetRasterOp(aROp);
            if (bHasLine)
                pGridWin[i]->SetLineColor(aLine);
            else
                pGridWin[i]->SetLineColor();
            if (bHasFill)
                pGridWin[i]->SetFillColor(aFill);
            else
                pGridWin[i]->SetFillColor();
        }
    }
}

void ScTabView::DrawEnableAnim(BOOL bSet)
{
    USHORT i;
    if ( pDrawView )
    {
        //  #71040# dont start animations if display of graphics is disabled
        //  graphics are controlled by VOBJ_TYPE_OLE
        if ( bSet && aViewData.GetOptions().GetObjMode(VOBJ_TYPE_OLE) == VOBJ_MODE_SHOW )
        {
            if ( !pDrawView->IsAnimationEnabled() )
            {
                pDrawView->SetAnimationEnabled(TRUE);

                //  Animierte GIFs muessen wieder gestartet werden:
                ScDocument* pDoc = aViewData.GetDocument();
                for (i=0; i<4; i++)
                    if ( pGridWin[i] && pGridWin[i]->IsVisible() )
                        pDoc->StartAnimations( aViewData.GetTabNo(), pGridWin[i] );
            }
        }
        else
        {
            pDrawView->SetAnimationEnabled(FALSE);
            //  StopAnimations am Dokument ist nicht mehr noetig
        }
    }
}

void ScTabView::DrawShowMarkHdl(BOOL bShow)
{
    if (!pDrawView)
        return;

    if (bShow)
    {
        if (!pDrawView->IsDisableHdl())
            pDrawView->ShowMarkHdl(NULL);
    }
    else
        pDrawView->HideMarkHdl(NULL);
}

void ScTabView::UpdateDrawTextOutliner()
{
    if ( pDrawView )
    {
        Outliner* pOL = pDrawView->GetTextEditOutliner();
        if (pOL)
            aViewData.UpdateOutlinerFlags( *pOL );
    }
}

//---------------------------------------------------------------

void ScTabView::ScrollToObject( SdrObject* pDrawObj )
{
    if ( pDrawObj )
        MakeVisible( pDrawObj->GetLogicRect() );
}

void ScTabView::MakeVisible( const Rectangle& rHMMRect )
{
    Window* pWin = GetActiveWin();
    Size aWinSize = pWin->GetOutputSizePixel();
    USHORT nTab = aViewData.GetTabNo();

    Rectangle aRect = pWin->LogicToPixel( rHMMRect );

    long nScrollX=0, nScrollY=0;        // Pixel

    if ( aRect.Right() >= aWinSize.Width() )                // rechts raus
    {
        nScrollX = aRect.Right() - aWinSize.Width() + 1;    // rechter Rand sichtbar
        if ( aRect.Left() < nScrollX )
            nScrollX = aRect.Left();                        // links sichtbar (falls zu gross)
    }
    if ( aRect.Bottom() >= aWinSize.Height() )              // unten raus
    {
        nScrollY = aRect.Bottom() - aWinSize.Height() + 1;  // unterer Rand sichtbar
        if ( aRect.Top() < nScrollY )
            nScrollY = aRect.Top();                         // oben sichtbar (falls zu gross)
    }

    if ( aRect.Left() < 0 )             // links raus
        nScrollX = aRect.Left();        // linker Rand sichtbar
    if ( aRect.Top() < 0 )              // oben raus
        nScrollY = aRect.Top();         // oberer Rand sichtbar

    if (nScrollX || nScrollY)
    {
        ScDocument* pDoc = aViewData.GetDocument();
        double nPPTX = aViewData.GetPPTX();
        double nPPTY = aViewData.GetPPTY();
        ScSplitPos eWhich = aViewData.GetActivePart();
        USHORT nPosX = aViewData.GetPosX(WhichH(eWhich));
        USHORT nPosY = aViewData.GetPosY(WhichV(eWhich));

        long nLinesX=0, nLinesY=0;      // Spalten/Zeilen - um mindestens nScrollX/Y scrollen

        if (nScrollX > 0)
            while (nScrollX > 0 && nPosX < MAXCOL)
            {
                nScrollX -= (long) ( pDoc->GetColWidth(nPosX, nTab) * nPPTX );
                ++nPosX;
                ++nLinesX;
            }
        else if (nScrollX < 0)
            while (nScrollX < 0 && nPosX > 0)
            {
                --nPosX;
                nScrollX += (long) ( pDoc->GetColWidth(nPosX, nTab) * nPPTX );
                --nLinesX;
            }

        if (nScrollY > 0)
            while (nScrollY > 0 && nPosY < MAXROW)
            {
                nScrollY -= (long) ( pDoc->FastGetRowHeight(nPosY, nTab) * nPPTY );
                ++nPosY;
                ++nLinesY;
            }
        else if (nScrollY < 0)
            while (nScrollY < 0 && nPosY > 0)
            {
                --nPosY;
                nScrollY += (long) ( pDoc->FastGetRowHeight(nPosY, nTab) * nPPTY );
                --nLinesY;
            }

        ScrollLines( nLinesX, nLinesY );                    // ausfuehren
    }
}



