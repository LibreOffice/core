/*************************************************************************
 *
 *  $RCSfile: tabvwsh2.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: nn $ $Date: 2000-09-22 18:31:22 $
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

#define _SVDRAG_HXX
#define _SVINCVW_HXX
#define _SV_MULTISEL_HXX
#define _SVRTV_HXX
#define _SVTABBX_HXX

//#define _BASE_DLGS_HXX
#define _BIGINT_HXX
#define _CACHESTR_HXX
#define _CONFIG_HXX
#define _CURSOR_HXX
#define _CTRLTOOL_HXX
#define _DLGCFG_HXX
#define _DYNARR_HXX
#define _EXTATTR_HXX
#define _FILDLG_HXX
#define _FONTDLG_HXX
#define _FRM3D_HXX
#define _INTRO_HXX
#define _ISETBWR_HXX
#define _NO_SVRTF_PARSER_HXX
#define _MACRODLG_HXX
#define _MODALDLG_HXX
#define _MOREBUTTON_HXX
#define _OUTLINER_HXX
#define _PASSWD_HXX
#define _SOUND_HXX

#if defined  WIN
#define _MENUBTN_HXX
#endif

//svtools
#define _SCRWIN_HXX
#define _RULER_HXX
#define _TABBAR_HXX
#define _VALUESET_HXX
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

// SFX
//#define _BASEDLGS_HXX
#define _DLGCFG_HXX
#define _SFXBASIC_HXX
#define _SFXFILEDLG_HXX
#define _SFXMNUMGR_HXX
#define _SFXMNUITEM_HXX
#define _SFXMSG_HXX
#define _SFXMSGDESCR_HXX
#define _SFXMSGPOOL_HXX
#define _SFXOBJFACE_HXX
//#define _SFXREQUEST_HXX
#define _SFXSTBMGR_HXX
#define _SFX_TEMPLDLG_HXX
#define _SFXSTBITEM_HXX
//#define _SFXTBXCTRL_HXX
#define _PASSWD_HXX
#define _SFXFILEDLG_HXX
//#define _SFXREQUEST_HXX
#define _SFXOBJFACE_HXX
#define _SFXMSGPOOL_HXX
#define _SFXMSGDESCR_HXX
#define _SFXMSG_HXX
#define _INTRO_HXX
#define _SFX_PRNMON_HXX
//#define _SFXDISPATCH_HXX
//#define _SFXCTRLITEM_HXX
#define _SFX_MINFITEM_HXX
#define _SFX_MACRO_HXX
#define _SFX_CHOR
#define _SFXEVENT_HXX
#define _SFX_MINFITEM_HXX

//sfxdoc.hxx
//#define _SFX_OBJSH_HXX
//#define _SFX_CLIENTSH_HXX
//#define _SFXDOCINF_HXX
//#define _SFX_OBJFAC_HXX
#define _SFX_DOCFILT_HXX
#define _SFXDOCFILE_HXX
//#define _VIEWFAC_HXX
//#define _SFXVIEWFRM_HXX
//#define _SFXVIEWSH_HXX
#define _MDIFRM_HXX
//#define _SFX_IPFRM_HXX //*
//#define _SFX_INTERNO_HXX

//#define SI_NOITEMS
//#define SI_NODRW
#define _SI_NOSBXCONTROLS
//#define _VCATTR_HXX
#define _VCONT_HXX
//#define _VCSBX_HXX
#define _SI_NOOTHERFORMS
//#define _VCTRLS_HXX
//#define _VCDRWOBJ_HXX
#define _SI_NOCONTROL
#define _SETBRW_HXX
#define _VCBRW_HXX
#define _SI_NOSBXCONTROLS
//#define _VCSBX_HXX
#define _SIDLL_HXX

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

// soui.hxx
#define _IPWIN_HXX
#define _IPMENU_HXX
#define _PASTEDLG_HXX

// basicsh.hxx
#define _BASIDESH_HXX
#define _TBXCTL_HXX

//xout.hxx
//#define _XENUM_HXX
//#define _XPOLY_HXX
//#define _XATTR_HXX ***
#define _XOUTX_HXX //*
//#define _XPOOL_HXX ***
#define _XTABLE_HXX //*


#define _SDR_NOVIEWMARKER
#define _SDR_NODRAGMETHODS
#define _SDR_NOXOUTDEV
//#define _SDR_NOOBJECTS
//#define _SDR_NOVIEWS


#define _SDR_NOITEMS
#define _SVDXOUT_HXX
#define _SDR_NOTRANSFORM        // Transformationen, selten verwendet
#define _SDR_NOTOUCH            // Hit-Tests, selten verwendet
#define _SDR_NOEXTDEV           // ExtOutputDevice
//#define   _SDR_NOUNDO             // Undo-Objekte
#define _SDR_NOSURROGATEOBJ     // SdrObjSurrogate
#define _SDR_NOPAGEOBJ          // SdrPageObj
#define _SDR_NOVIRTOBJ          // SdrVirtObj
#define _SDR_NOGROUPOBJ         // SdrGroupObj
//#define _SDR_NOTEXTOBJ          // SdrTextObj
#define _SDR_NOPATHOBJ          // SdrPathObj
#define _SDR_NOEDGEOBJ          // SdrEdgeObj
#define _SDR_NORECTOBJ          // SdrRectObj
#define _SDR_NOCAPTIONOBJ       // SdrCaptionObj
#define _SDR_NOCIRCLEOBJ        // SdrCircleObj
#define _SDR_NOGRAFOBJ          // SdrGrafObj
#define _SDR_NOOLE2OBJ          // SdrOle2Obj

// INCLUDE ---------------------------------------------------------------

#ifdef WNT
#pragma optimize ("", off)
#endif

#include <basctl/idetemp.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/ipfrm.hxx>
#include <svtools/aeitem.hxx>
#include <svtools/whiter.hxx>
#include <sfx2/dispatch.hxx>

#include "tabvwsh.hxx"
#include "drawattr.hxx"
#include "drawsh.hxx"
#include "drawview.hxx"
#include "fupoor.hxx"
#include "fuconrec.hxx"
#include "fuconpol.hxx"
#include "fuconarc.hxx"
#include "fuconctl.hxx"
#include "fuconuno.hxx"
#include "fusel.hxx"
#include "futext.hxx"
#include "fumark.hxx"
#include "global.hxx"
#include "sc.hrc"
#include "scmod.hxx"


// -----------------------------------------------------------------------

SdrView* __EXPORT ScTabViewShell::GetDrawView() const
{
    return ((ScTabViewShell*)this)->GetScDrawView();    // GetScDrawView ist nicht-const
}

void ScTabViewShell::WindowChanged()
{
    Window* pWin = GetActiveWin();

    ScDrawView* pDrView = GetScDrawView();
    if (pDrView)
        pDrView->SetActualWin(pWin);

    FuPoor* pFunc = GetDrawFuncPtr();
    if (pFunc)
        pFunc->SetWindow(pWin);
}

void ScTabViewShell::ExecDraw(SfxRequest& rReq)
{
    SC_MOD()->InputEnterHandler();
    UpdateInputHandler();

    MakeDrawLayer();

    ScTabView* pTabView = GetViewData()->GetView();
    USHORT nTab = GetViewData()->GetTabNo();
    SfxBindings& rBindings = GetViewFrame()->GetBindings();

    Window*     pWin    = pTabView->GetActiveWin();
    SdrView*    pView   = pTabView->GetSdrView();
    SdrModel*   pDoc    = pView->GetModel();

    const SfxItemSet *pArgs = rReq.GetArgs();
    USHORT nNewId = rReq.GetSlot();

    //
    //  Pseudo-Slots von Draw-Toolbox auswerten
    //! wird das ueberhaupt noch gebraucht ?????
    //

    if (nNewId == SID_INSERT_DRAW && pArgs)
    {
        const SfxPoolItem* pItem;
        if ( pArgs->GetItemState( SID_INSERT_DRAW, TRUE, &pItem ) == SFX_ITEM_SET &&
             pItem->ISA( SvxDrawToolItem ) )
        {
            SvxDrawToolEnum eSel = (SvxDrawToolEnum)((const SvxDrawToolItem*)pItem)->GetValue();
            switch (eSel)
            {
                case SVX_SNAP_DRAW_SELECT:          nNewId = SID_OBJECT_SELECT;         break;
                case SVX_SNAP_DRAW_LINE:            nNewId = SID_DRAW_LINE;             break;
                case SVX_SNAP_DRAW_RECT:            nNewId = SID_DRAW_RECT;             break;
                case SVX_SNAP_DRAW_ELLIPSE:         nNewId = SID_DRAW_ELLIPSE;          break;
                case SVX_SNAP_DRAW_POLYGON_NOFILL:  nNewId = SID_DRAW_POLYGON_NOFILL;   break;
                case SVX_SNAP_DRAW_BEZIER_NOFILL:   nNewId = SID_DRAW_BEZIER_NOFILL;    break;
                case SVX_SNAP_DRAW_FREELINE_NOFILL: nNewId = SID_DRAW_FREELINE_NOFILL;  break;
                case SVX_SNAP_DRAW_ARC:             nNewId = SID_DRAW_ARC;              break;
                case SVX_SNAP_DRAW_PIE:             nNewId = SID_DRAW_PIE;              break;
                case SVX_SNAP_DRAW_CIRCLECUT:       nNewId = SID_DRAW_CIRCLECUT;        break;
                case SVX_SNAP_DRAW_TEXT:            nNewId = SID_DRAW_TEXT;             break;
                case SVX_SNAP_DRAW_TEXT_MARQUEE:    nNewId = SID_DRAW_TEXT_MARQUEE;     break;
                case SVX_SNAP_DRAW_CAPTION:         nNewId = SID_DRAW_CAPTION;          break;
            }
        }
        else                    // USHORT-Item vom Controller
        {
            rReq.Done();
            return;
        }
    }

    //
    //  Pseudo-Slots von Control-Toolbox auswerten
    //

    if (nNewId == SID_CHOOSE_CONTROLS && pArgs)
    {
        const SvxChooseControlItem* pItem = (const SvxChooseControlItem*)&pArgs->Get( SID_CHOOSE_CONTROLS );
        SvxChooseControlEnum eSel = (SvxChooseControlEnum) pItem->GetValue();

        nCtrlSfxId = ((USHORT)eSel==nCtrlSfxId) ? USHRT_MAX : (USHORT) eSel;

        if( (eSel == SVX_SNAP_PREVIEW)) // || (eSel==SVX_SNAP_URLBUTTON))
        {
            rReq.Done();
            return;
        }
        switch (eSel)
        {
            case SVX_SNAP_PUSHBUTTON:       nNewId = SID_INSERT_PUSHBUTTON; break;
            case SVX_SNAP_CHECKBOX:         nNewId = SID_INSERT_CHECKBOX;   break;
            case SVX_SNAP_RADIOBUTTON:      nNewId = SID_INSERT_RADIOBUTTON;break;
            case SVX_SNAP_SPINBUTTON:       nNewId = SID_INSERT_SPINBUTTON; break;
            case SVX_SNAP_FIXEDTEXT:        nNewId = SID_INSERT_FIXEDTEXT;  break;
            case SVX_SNAP_GROUPBOX:         nNewId = SID_INSERT_GROUPBOX;   break;
            case SVX_SNAP_LISTBOX:          nNewId = SID_INSERT_LISTBOX;    break;
            case SVX_SNAP_COMBOBOX:         nNewId = SID_INSERT_COMBOBOX;   break;
            case SVX_SNAP_EDIT:             nNewId = SID_INSERT_EDIT;       break;
            case SVX_SNAP_HSCROLLBAR:       nNewId = SID_INSERT_HSCROLLBAR; break;
            case SVX_SNAP_VSCROLLBAR:       nNewId = SID_INSERT_VSCROLLBAR; break;
            case SVX_SNAP_URLBUTTON:        nNewId = SID_INSERT_URLBUTTON;  break;
            case SVX_SNAP_SELECT:           nNewId = SID_OBJECT_SELECT;     break;
        }
    }

    if ( nNewId == SID_DRAW_SELECT )
        nNewId = SID_OBJECT_SELECT;

    USHORT nNewFormId = 0;
    if ( nNewId == SID_FM_CREATE_CONTROL && pArgs )
    {
        const SfxPoolItem* pItem;
        if ( pArgs->GetItemState( SID_FM_CONTROL_IDENTIFIER, TRUE, &pItem ) == SFX_ITEM_SET &&
             pItem->ISA( SfxUInt16Item ) )
            nNewFormId = ((const SfxUInt16Item*)pItem)->GetValue();
    }

    if (nNewId == SID_INSERT_FRAME)                     // vom Tbx-Button
        nNewId = SID_DRAW_TEXT;

    BOOL bEx = IsDrawSelMode();
    if ( nNewId == nDrawSfxId && ( nNewId != SID_FM_CREATE_CONTROL ||
                                    nNewFormId == nFormSfxId || nNewFormId == 0 ) )
    {
        //  SID_FM_CREATE_CONTROL mit nNewFormId==0 (ohne Parameter) kommt beim Deaktivieren
        //  aus FuConstruct::SimpleMouseButtonUp
        //  #59280# Execute fuer die Form-Shell, um im Controller zu deselektieren
        if ( nNewId == SID_FM_CREATE_CONTROL )
        {
            GetViewData()->GetDispatcher().Execute(SID_FM_LEAVE_CREATE);
            GetViewFrame()->GetBindings().InvalidateAll(FALSE);
            //! was fuer einen Slot braucht der komische Controller wirklich, um das anzuzeigen????
        }

        bEx = !bEx;
        nNewId = SID_OBJECT_SELECT;
    }
    else
        bEx = TRUE;

    if ( nDrawSfxId == SID_FM_CREATE_CONTROL && nNewId != nDrawSfxId )
    {
        //  Wechsel von Control- zu Zeichenfunktion -> im Control-Controller deselektieren
        GetViewData()->GetDispatcher().Execute(SID_FM_LEAVE_CREATE);
        GetViewFrame()->GetBindings().InvalidateAll(FALSE);
        //! was fuer einen Slot braucht der komische Controller wirklich, um das anzuzeigen????
    }

    SetDrawSelMode(bEx);

    SdrLayer* pLayer = pView->GetModel()->GetLayerAdmin().GetLayerPerID(SC_LAYER_BACK);
    if (pLayer)
        pView->SetLayerLocked( pLayer->GetName(), !bEx );

    nDrawSfxId = nNewId;

    if ( nNewId != SID_DRAW_CHART )             // Chart nicht mit DrawShell
    {
        if ( nNewId == SID_DRAW_TEXT || nNewId == SID_DRAW_TEXT_MARQUEE || nNewId == SID_DRAW_NOTEEDIT )
            SetDrawTextShell( TRUE );
        else
        {
            if (bEx)
            {
                SetDrawShell( TRUE );
            }
            else
            {
                SetDrawShell( pView->GetMarkList().GetMarkCount() != 0 );
                nCtrlSfxId = USHRT_MAX;// hier pfuschte JN
            }
        }
    }

    if (pTabView->GetDrawFuncPtr())
    {
        if (pTabView->GetDrawFuncOldPtr() != pTabView->GetDrawFuncPtr())
            delete pTabView->GetDrawFuncOldPtr();

        pTabView->GetDrawFuncPtr()->Deactivate();
        pTabView->SetDrawFuncOldPtr(pTabView->GetDrawFuncPtr());
        pTabView->SetDrawFuncPtr(NULL);
    }

    SfxRequest aNewReq(rReq);
    aNewReq.SetSlot(nDrawSfxId);

    switch (nNewId)
    {
        case SID_OBJECT_SELECT:
            //@#70206# Nicht immer zurueckschalten
            if(pView->GetMarkList().GetMarkCount() == 0) SetDrawShell(bEx);
            pTabView->SetDrawFuncPtr(new FuSelection(this, pWin, pView, pDoc, aNewReq));
            break;

        case SID_DRAW_LINE:
        case SID_DRAW_RECT:
        case SID_DRAW_ELLIPSE:
            pTabView->SetDrawFuncPtr(new FuConstRectangle(this, pWin, pView, pDoc, aNewReq));
            break;

        case SID_DRAW_CAPTION:
            pTabView->SetDrawFuncPtr(new FuConstRectangle(this, pWin, pView, pDoc, aNewReq));
            pView->SetFrameDragSingles( FALSE );
            rBindings.Invalidate( SID_BEZIER_EDIT );
            break;

        case SID_DRAW_POLYGON:
        case SID_DRAW_POLYGON_NOFILL:
        case SID_DRAW_BEZIER_NOFILL:
        case SID_DRAW_FREELINE_NOFILL:
            pTabView->SetDrawFuncPtr(new FuConstPolygon(this, pWin, pView, pDoc, aNewReq));
            break;

        case SID_DRAW_ARC:
        case SID_DRAW_PIE:
        case SID_DRAW_CIRCLECUT:
            pTabView->SetDrawFuncPtr(new FuConstArc(this, pWin, pView, pDoc, aNewReq));
            break;

        case SID_DRAW_TEXT:
        case SID_DRAW_TEXT_MARQUEE:
        case SID_DRAW_NOTEEDIT:
            pTabView->SetDrawFuncPtr(new FuText(this, pWin, pView, pDoc, aNewReq));
            break;

        case SID_INSERT_PUSHBUTTON:
        case SID_INSERT_CHECKBOX:
        case SID_INSERT_RADIOBUTTON:
        case SID_INSERT_SPINBUTTON:
        case SID_INSERT_FIXEDTEXT:
        case SID_INSERT_GROUPBOX:
        case SID_INSERT_LISTBOX:
        case SID_INSERT_COMBOBOX:
        case SID_INSERT_EDIT:
        case SID_INSERT_HSCROLLBAR:
        case SID_INSERT_VSCROLLBAR:
        case SID_INSERT_URLBUTTON:
            pTabView->SetDrawFuncPtr(new FuConstControl(this, pWin, pView, pDoc, aNewReq));
            break;

        case SID_FM_CREATE_CONTROL:
            SetDrawFormShell(TRUE);
            pTabView->SetDrawFuncPtr(new FuConstUnoControl(this, pWin, pView, pDoc, aNewReq));
            nFormSfxId = nNewFormId;
            break;

        case SID_DRAW_CHART:
            bChartDlgIsEdit = FALSE;
            pTabView->SetDrawFuncPtr(new FuMarkRect(this, pWin, pView, pDoc, aNewReq));
            break;

        default:
            break;
    }

    if (pTabView->GetDrawFuncPtr())
        pTabView->GetDrawFuncPtr()->Activate();

    rReq.Done();

    rBindings.Invalidate( SID_INSERT_DRAW );
    rBindings.Invalidate( SID_CHOOSE_CONTROLS );

    rBindings.Update( SID_INSERT_DRAW );
    rBindings.Update( SID_CHOOSE_CONTROLS );
}

void ScTabViewShell::GetDrawState(SfxItemSet &rSet)
{
    SfxWhichIter    aIter(rSet);
    USHORT          nWhich = aIter.FirstWhich();

    while ( nWhich )
    {
        switch ( nWhich )
        {
            case SID_INSERT_DRAW:
                {
                    //  SID_OBJECT_SELECT nur, wenn "harter" Selektionsmodus
                    USHORT nPutId = nDrawSfxId;
                    if ( nPutId == SID_OBJECT_SELECT && !IsDrawSelMode() )
                        nPutId = USHRT_MAX;
                    // nur die Images, die auch auf dem Controller liegen
                    if ( nPutId != SID_OBJECT_SELECT &&
                         nPutId != SID_DRAW_LINE &&
                         nPutId != SID_DRAW_RECT &&
                         nPutId != SID_DRAW_ELLIPSE &&
                         nPutId != SID_DRAW_POLYGON_NOFILL &&
                         nPutId != SID_DRAW_BEZIER_NOFILL &&
                         nPutId != SID_DRAW_FREELINE_NOFILL &&
                         nPutId != SID_DRAW_ARC &&
                         nPutId != SID_DRAW_PIE &&
                         nPutId != SID_DRAW_CIRCLECUT &&
                         nPutId != SID_DRAW_TEXT &&
                         nPutId != SID_DRAW_TEXT_MARQUEE &&
                         nPutId != SID_DRAW_CAPTION )
                        nPutId = USHRT_MAX;
                    SfxAllEnumItem aItem( nWhich, nPutId );
                    rSet.Put( aItem );
                }
                break;
            case SID_CHOOSE_CONTROLS:
                if (nCtrlSfxId != USHRT_MAX)
                {
                    SfxAllEnumItem aItem( nWhich, nCtrlSfxId );
                    rSet.Put( aItem );
                }
                break;

            case SID_DRAW_CHART:
                {
                    BOOL bOle = GetViewFrame()->ISA(SfxInPlaceFrame);
                    if ( bOle || !SFX_APP()->HasFeature(SFX_FEATURE_SCHART) )
                        rSet.DisableItem( nWhich );
                }
                break;

            case SID_OBJECT_SELECT:     // wichtig fuer den ollen Control-Controller
                rSet.Put( SfxBoolItem( nWhich, nDrawSfxId == SID_OBJECT_SELECT && IsDrawSelMode() ) );
                break;
        }
        nWhich = aIter.NextWhich();
    }
}

BOOL ScTabViewShell::SelectObject( const String& rName )
{
    ScDrawView* pView = GetViewData()->GetScDrawView();
    if (!pView)
        return FALSE;

    BOOL bFound = pView->SelectObject( rName );
    // DrawShell etc. is handled in MarkListHasChanged

    return bFound;
}



