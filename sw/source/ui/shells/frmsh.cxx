/*************************************************************************
 *
 *  $RCSfile: frmsh.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jp $ $Date: 2000-10-06 13:36:37 $
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

#ifdef PRECOMPILED
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif
#ifndef _UIPARAM_HXX
#include <uiparam.hxx>
#endif

#ifndef _SFX_WHITER_HXX //autogen
#include <svtools/whiter.hxx>
#endif
#ifndef _OFF_APP_HXX //autogen
#include <offmgr/app.hxx>
#endif
#ifndef _GOODIES_IMAPOBJ_HXX
#include <svtools/imapobj.hxx>
#endif
#ifndef _SVX_SRCHITEM_HXX
#include <svx/srchitem.hxx>
#endif
#ifndef _IMAP_HXX //autogen
#include <svtools/imap.hxx>
#endif
#ifndef _SFXVIEWFRM_HXX //autogen
#include <sfx2/viewfrm.hxx>
#endif
#ifndef _SB_SBSTAR_HXX //autogen
#include <basic/sbstar.hxx>
#endif
#ifndef _SFXRECTITEM_HXX //autogen
#include <svtools/rectitem.hxx>
#endif
#ifndef _SFXPTITEM_HXX //autogen
#include <svtools/ptitem.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _SVX_COLRITEM_HXX //autogen
#include <svx/colritem.hxx>
#endif
#ifndef _SVX_BOLNITEM_HXX //autogen
#include <svx/bolnitem.hxx>
#endif
#ifndef _SVX_BOXITEM_HXX //autogen
#include <svx/boxitem.hxx>
#endif
#ifndef _SVX_PROTITEM_HXX //autogen
#include <svx/protitem.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifndef _SFXOBJFACE_HXX //autogen
#include <sfx2/objface.hxx>
#endif
#ifndef _SVX_HLNKITEM_HXX //autogen
#include <svx/hlnkitem.hxx>
#endif


#ifndef _FMTURL_HXX //autogen
#include <fmturl.hxx>
#endif
#ifndef _FMTCLDS_HXX //autogen
#include <fmtclds.hxx>
#endif
#ifndef _FMTCNCT_HXX //autogen
#include <fmtcnct.hxx>
#endif
#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif
#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#ifndef _FRMATR_HXX
#include <frmatr.hxx>
#endif
#ifndef _UITOOL_HXX
#include <uitool.hxx>
#endif
#ifndef _FRMFMT_HXX
#include <frmfmt.hxx>
#endif
#ifndef _FRMSH_HXX
#include <frmsh.hxx>
#endif
#ifndef _FRMMGR_HXX
#include <frmmgr.hxx>
#endif
#ifndef _FRMDLG_HXX
#include <frmdlg.hxx>
#endif
#ifndef _SWEVENT_HXX
#include <swevent.hxx>
#endif
#ifndef _USRPREF_HXX
#include <usrpref.hxx>
#endif
#ifndef _EDTWIN_HXX
#include <edtwin.hxx>
#endif

#ifndef _HELPID_H
#include <helpid.h>
#endif
#ifndef _CMDID_H
#include <cmdid.h>
#endif
#ifndef _GLOBALS_HRC
#include <globals.hrc>
#endif
#ifndef _POPUP_HRC
#include <popup.hrc>
#endif
#ifndef _SHELLS_HRC
#include <shells.hrc>
#endif

// Prototypen ------------------------------------------------------------

void lcl_FrmGetMaxLineWidth(const SvxBorderLine* pBorderLine, SvxBorderLine& rBorderLine);

#define SwFrameShell
#include "itemdef.hxx"
#include "swslots.hxx"

SFX_IMPL_INTERFACE(SwFrameShell, SwBaseShell, SW_RES(STR_SHELLNAME_FRAME))
{
    SFX_POPUPMENU_REGISTRATION(SW_RES(MN_FRM_POPUPMENU));
    SFX_OBJECTBAR_REGISTRATION(SFX_OBJECTBAR_OBJECT, SW_RES(RID_FRAME_TOOLBOX));
    SFX_OBJECTMENU_REGISTRATION(SID_OBJECTMENU0, SW_RES(MN_OBJECTMENU_FRAME));
}



void SwFrameShell::Execute(SfxRequest &rReq)
{
    //Erstmal die, die keinen FrmMgr benoetigen.
    SwWrtShell &rSh = GetShell();
    BOOL bMore = FALSE;
    const SfxItemSet* pArgs = rReq.GetArgs();
    const SfxPoolItem* pItem;
    USHORT nSlot = rReq.GetSlot();

    switch ( nSlot )
    {
        case FN_FRAME_TO_ANCHOR:
            if ( rSh.IsFrmSelected() )
            {
                rSh.GotoFlyAnchor();
                rSh.EnterStdMode();
                rSh.CallChgLnk();
            }
            break;
        case SID_FRAME_TO_TOP:
            rSh.SelectionToTop();
            break;

        case SID_FRAME_TO_BOTTOM:
            rSh.SelectionToBottom();
            break;

        case FN_FRAME_UP:
            rSh.SelectionToTop( FALSE );
            break;

        case FN_FRAME_DOWN:
            rSh.SelectionToBottom( FALSE );
            break;
        case FN_INSERT_FRAME:
            if (!pArgs)
            {
                // Rahmen existiert bereits, Rahmendialog zur Bearbeitung oeffnen
                SfxUInt16Item aDefPage(FN_FORMAT_FRAME_DLG, TP_COLUMN);
                rSh.GetView().GetViewFrame()->GetDispatcher()->Execute( FN_FORMAT_FRAME_DLG,
                                SFX_CALLMODE_SYNCHRON|SFX_CALLMODE_RECORD,
                                &aDefPage, 0L );

            }
            else
            {
                // Rahmen existiert bereits, nur Spaltenanzahl wird geaendert
                USHORT nCols = 1;
                if(pArgs->GetItemState(SID_ATTR_COLUMNS, FALSE, &pItem) == SFX_ITEM_SET)
                    nCols = ((SfxUInt16Item *)pItem)->GetValue();

                SfxItemSet aSet(GetPool(),RES_COL,RES_COL);
                rSh.GetFlyFrmAttr( aSet );
                SwFmtCol aCol((const SwFmtCol&)aSet.Get(RES_COL));
                // GutterWidth wird nicht immer uebergeben, daher erst besorgen (siehe view2: Execute auf diesen Slot)
                USHORT nGutterWidth = aCol.GetGutterWidth();
                if(!nCols )
                    nCols++;
                aCol.Init(nCols, nGutterWidth, aCol.GetWishWidth());
                aSet.Put(aCol);
                // Vorlagen-AutoUpdate
                SwFrmFmt* pFmt = rSh.GetCurFrmFmt();
                if(pFmt && pFmt->IsAutoUpdateFmt())
                {
                    rSh.AutoUpdateFrame(pFmt, aSet);
                }
                else
                {
                    rSh.StartAllAction();
                    rSh.SetFlyFrmAttr( aSet );
                    rSh.SetModified();
                    rSh.EndAllAction();
                }

            }
            return;

        case SID_HYPERLINK_SETLINK:
        {
            if(pArgs && SFX_ITEM_SET == pArgs->GetItemState(SID_HYPERLINK_SETLINK, FALSE, &pItem))
            {
                const SvxHyperlinkItem& rHLinkItem = *(const SvxHyperlinkItem *)pItem;
                const String& rURL = rHLinkItem.GetURL();
                const String& rTarget = rHLinkItem.GetTargetFrame();

                SfxItemSet aSet( rSh.GetAttrPool(), RES_URL, RES_URL );
                rSh.GetFlyFrmAttr( aSet );
                SwFmtURL aURL( (SwFmtURL&)aSet.Get( RES_URL ) );

                String sOldName(rHLinkItem.GetName());
                String sFlyName(rSh.GetFlyName());
                if (sOldName.ToUpperAscii() != sFlyName.ToUpperAscii())
                {
                    String sName(sOldName);
                    USHORT i = 1;
                    while (rSh.FindFlyByName(sName))
                    {
                        sName = sOldName;
                        sName += '_';
                        sName += String::CreateFromInt32(i++);
                    }
                    rSh.SetFlyName(sName);
                }
                aURL.SetURL( rURL, FALSE );
                aURL.SetTargetFrameName(rTarget);

                aSet.Put( aURL );
                rSh.SetFlyFrmAttr( aSet );
            }
        }
        break;

        case FN_FRAME_CHAIN:
            rSh.GetView().GetEditWin().SetChainMode( !rSh.GetView().GetEditWin().IsChainMode() );
            break;

        case FN_FRAME_UNCHAIN:
            rSh.Unchain( (SwFrmFmt&)*rSh.GetFlyFrmFmt() );
            GetView().GetViewFrame()->GetBindings().Invalidate(FN_FRAME_CHAIN);
            break;

        default: bMore = TRUE;
    }

    if ( !bMore )
    {
        return;
    }

    SwFlyFrmAttrMgr aMgr( FALSE, &rSh, FRMMGR_TYPE_NONE );
    BOOL bUpdateMgr = TRUE;
    BOOL bCopyToFmt = FALSE;
    switch ( nSlot )
    {
        case FN_FRAME_ALIGN_VERT_CENTER:
            aMgr.SetVertOrientation( SVX_VERT_CENTER );
            break;

        case FN_FRAME_ALIGN_VERT_BOTTOM:
            aMgr.SetVertOrientation( SVX_VERT_BOTTOM );
            break;

        case FN_FRAME_ALIGN_VERT_TOP:
            aMgr.SetVertOrientation( SVX_VERT_TOP );
            break;

        case FN_FRAME_ALIGN_VERT_CHAR_CENTER:
            aMgr.SetVertOrientation( SVX_VERT_CHAR_CENTER );
            break;

        case FN_FRAME_ALIGN_VERT_CHAR_BOTTOM:
            aMgr.SetVertOrientation( SVX_VERT_CHAR_BOTTOM );
            break;

        case FN_FRAME_ALIGN_VERT_CHAR_TOP:
            aMgr.SetVertOrientation( SVX_VERT_CHAR_TOP );
            break;

        case FN_FRAME_ALIGN_VERT_ROW_CENTER:
            aMgr.SetVertOrientation( SVX_VERT_LINE_CENTER );
            break;

        case FN_FRAME_ALIGN_VERT_ROW_BOTTOM:
            aMgr.SetVertOrientation( SVX_VERT_LINE_BOTTOM );
            break;

        case FN_FRAME_ALIGN_VERT_ROW_TOP:
            aMgr.SetVertOrientation( SVX_VERT_LINE_TOP );
            break;

        case FN_FRAME_ALIGN_HORZ_CENTER:
            aMgr.SetHorzOrientation( HORI_CENTER );
            break;

        case FN_FRAME_ALIGN_HORZ_RIGHT:
            aMgr.SetHorzOrientation( HORI_RIGHT );
            break;

        case FN_FRAME_ALIGN_HORZ_LEFT:
            aMgr.SetHorzOrientation( HORI_LEFT );
            break;

        case FN_SET_FRM_POSITION:
        {
            aMgr.SetAbsPos(((SfxPointItem &)pArgs->Get
                                (FN_SET_FRM_POSITION)).GetValue());
        }
        break;
        case SID_ATTR_BRUSH:
        {
            if(pArgs)
            {
                aMgr.SetAttrSet( *pArgs );
                bCopyToFmt = TRUE;
            }
        }
        break;
        case SID_ATTR_ULSPACE:
        case SID_ATTR_LRSPACE:
        {
            if(pArgs && SFX_ITEM_SET == pArgs->GetItemState(GetPool().GetWhich(nSlot), FALSE, &pItem))
            {
                aMgr.SetAttrSet( *pArgs );
                if(SID_ATTR_ULSPACE == nSlot && SID_ATTR_ULSPACE == nSlot)
                    bCopyToFmt = TRUE;
            }
        }
        break;
        case FN_FORMAT_FRAME_DLG:
        {
            const int nSel = rSh.GetSelectionType();
            if (nSel & SwWrtShell::SEL_GRF)
            {
                rSh.GetView().GetViewFrame()->GetDispatcher()->Execute(FN_FORMAT_GRAFIC_DLG);
                bUpdateMgr = FALSE;
            }
            else
            {
                SfxItemSet aSet(GetPool(),  RES_FRMATR_BEGIN,       RES_FRMATR_END-1,
                                            SID_ATTR_BORDER_INNER,  SID_ATTR_BORDER_INNER,
                                            FN_GET_PRINT_AREA,      FN_GET_PRINT_AREA,
                                            SID_ATTR_PAGE_SIZE,     SID_ATTR_PAGE_SIZE,
                                            SID_ATTR_BRUSH,         SID_ATTR_BRUSH,
                                            SID_ATTR_LRSPACE,       SID_ATTR_ULSPACE,
                                            FN_SURROUND,            FN_HORI_ORIENT,
                                            FN_SET_FRM_NAME,        FN_SET_FRM_NAME,
                                            FN_KEEP_ASPECT_RATIO,   FN_KEEP_ASPECT_RATIO,
                                            SID_DOCFRAME,           SID_DOCFRAME,
                                            SID_HTML_MODE,          SID_HTML_MODE,
                                            FN_SET_FRM_ALT_NAME,    FN_SET_FRM_ALT_NAME,
                                            0);

                const SwViewOption* pVOpt = rSh.GetViewOptions();
                if(nSel & SwWrtShell::SEL_OLE)
                {
                    aSet.Put(SfxBoolItem(FN_KEEP_ASPECT_RATIO,
                        pVOpt->IsKeepRatio()));
                }
                aSet.Put(SfxUInt16Item(SID_HTML_MODE, ::GetHtmlMode(GetView().GetDocShell())));
                aSet.Put(SfxStringItem(FN_SET_FRM_NAME, rSh.GetFlyName()));
                if(nSel & SwWrtShell::SEL_OLE)
                    aSet.Put(SfxStringItem(FN_SET_FRM_ALT_NAME, rSh.GetAlternateText()));

                const SwRect &rPg = rSh.GetAnyCurRect(RECT_PAGE);
                SwFmtFrmSize aFrmSize(ATT_VAR_SIZE, rPg.Width(), rPg.Height());
                aFrmSize.SetWhich(GetPool().GetWhich(SID_ATTR_PAGE_SIZE));
                aSet.Put(aFrmSize);

                const SwRect &rPr = rSh.GetAnyCurRect(RECT_PAGE_PRT);
                SwFmtFrmSize aPrtSize(ATT_VAR_SIZE, rPr.Width(), rPr.Height());
                aPrtSize.SetWhich(GetPool().GetWhich(FN_GET_PRINT_AREA));
                aSet.Put(aPrtSize);

                aSet.Put(aMgr.GetAttrSet());
                aSet.SetParent( aMgr.GetAttrSet().GetParent() );

                // Bei %-Werten Groesse initialisieren
                SwFmtFrmSize& rSize = (SwFmtFrmSize&)aSet.Get(RES_FRM_SIZE);
                if (rSize.GetWidthPercent() && rSize.GetWidthPercent() != 0xff)
                    rSize.SetWidth(rSh.GetAnyCurRect(RECT_FLY_EMBEDDED).Width());
                if (rSize.GetHeightPercent() && rSize.GetHeightPercent() != 0xff)
                    rSize.SetHeight(rSh.GetAnyCurRect(RECT_FLY_EMBEDDED).Height());

                UINT16 nDefPage = 0;
                if(pArgs && pArgs->GetItemState(FN_FORMAT_FRAME_DLG, FALSE, &pItem) == SFX_ITEM_SET)
                    nDefPage = ((SfxUInt16Item *)pItem)->GetValue();

                aSet.Put(SfxFrameItem( SID_DOCFRAME, GetView().GetViewFrame()->GetTopFrame()));


                SwFrmDlg *pDlg = new SwFrmDlg( GetView().GetViewFrame(),
                                               GetView().GetWindow(),
                                   aSet, FALSE,
                                   nSel & SwWrtShell::SEL_GRF ? DLG_FRM_GRF :
                                   nSel & SwWrtShell::SEL_OLE ? DLG_FRM_OLE :
                                                                DLG_FRM_STD,
                                   FALSE,
                                   nDefPage);

                if ( pDlg->Execute() )
                {
                    const SfxItemSet* pOutSet = pDlg->GetOutputItemSet();
                    if(pOutSet)
                    {
                        if(nSel & SwWrtShell::SEL_OLE &&
                        SFX_ITEM_SET == pOutSet->GetItemState(FN_KEEP_ASPECT_RATIO, TRUE, &pItem))
                        {
                            SwViewOption aUsrPref( *pVOpt );
                            aUsrPref.SetKeepRatio(((const SfxBoolItem*)pItem)->GetValue());
                            SW_MOD()->ApplyUsrPref(aUsrPref, &GetView());
                        }
                        if (SFX_ITEM_SET == pOutSet->GetItemState(FN_SET_FRM_ALT_NAME, TRUE, &pItem))
                        {
                            rSh.SetAlternateText(((const SfxStringItem*)pItem)->GetValue());
                        }
                        // Vorlagen-AutoUpdate
                        SwFrmFmt* pFmt = rSh.GetCurFrmFmt();
                        if(pFmt && pFmt->IsAutoUpdateFmt())
                        {
                            rSh.AutoUpdateFrame(pFmt, *pOutSet);
                            const SfxPoolItem* pItem;
                            // alles, dass das Format nicht kann, muss hart
                            // gesetzt werden
                            if(SFX_ITEM_SET == pOutSet->GetItemState(FN_SET_FRM_NAME, FALSE, &pItem))
                                rSh.SetFlyName(((SfxStringItem*)pItem)->GetValue());
                            SfxItemSet aShellSet(GetPool(), RES_FRM_SIZE,   RES_FRM_SIZE,
                                                            RES_SURROUND,   RES_SURROUND,
                                                            RES_ANCHOR,     RES_ANCHOR,
                                                            RES_VERT_ORIENT,RES_HORI_ORIENT,
                                                            0);
                            aShellSet.Put(*pOutSet);
                            aMgr.SetAttrSet(aShellSet);
                            if(SFX_ITEM_SET == pOutSet->GetItemState(FN_SET_FRM_NAME, FALSE, &pItem))
                                rSh.SetFlyName(((SfxStringItem*)pItem)->GetValue());
                        }
                        else
                            aMgr.SetAttrSet( *pOutSet );

                    }
                }
                else
                    bUpdateMgr = FALSE;
                delete pDlg;
            }
        }
        break;
        case FN_FRAME_MIRROR_ON_EVEN_PAGES:
        {
            SwFmtHoriOrient aHori(aMgr.GetHoriOrient());
            BOOL bMirror = !aHori.IsPosToggle();
            aHori.SetPosToggle(bMirror);
            SfxItemSet aSet(GetPool(), RES_HORI_ORIENT, RES_HORI_ORIENT);
            aSet.Put(aHori);
            aMgr.SetAttrSet(aSet);
            bCopyToFmt = TRUE;
            rReq.SetReturnValue(SfxBoolItem(nSlot, bMirror));
        }
        break;
        default:
            ASSERT( !this, "falscher Dispatcher" );
            return;
    }
    // Vorlagen-AutoUpdate
    SwFrmFmt* pFmt = rSh.GetCurFrmFmt();
    if ( bUpdateMgr )
    {
        if(bCopyToFmt && pFmt && pFmt->IsAutoUpdateFmt())
        {
            rSh.AutoUpdateFrame(pFmt, aMgr.GetAttrSet());
        }
        else
            aMgr.UpdateFlyFrm();
    }

}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/


void SwFrameShell::GetState(SfxItemSet& rSet)
{
    SwWrtShell &rSh = GetShell();
    BOOL bHtmlMode = 0 != ::GetHtmlMode(rSh.GetView().GetDocShell());
    if (rSh.IsFrmSelected())
    {
        SfxItemSet aSet( rSh.GetAttrPool(),
                            RES_LR_SPACE, RES_UL_SPACE,
                            RES_PROTECT, RES_HORI_ORIENT,
                            RES_OPAQUE, RES_OPAQUE,
                            RES_PRINT, RES_OPAQUE,
                            0 );
        rSh.GetFlyFrmAttr( aSet );

        BOOL bProtect = rSh.IsSelObjProtected(FLYPROTECT_POS);
        BOOL bParentCntProt = rSh.IsSelObjProtected( (FlyProtectType)(FLYPROTECT_CONTENT|FLYPROTECT_PARENT) ) != 0;

        bProtect |= bParentCntProt;

        const USHORT eFrmType = rSh.GetFrmType(0,TRUE);
        SwFlyFrmAttrMgr aMgr( FALSE, &rSh, FRMMGR_TYPE_NONE );

        SfxWhichIter aIter( rSet );
        USHORT nWhich = aIter.FirstWhich();
        while ( nWhich )
        {
            switch ( nWhich )
            {
                case RES_FRM_SIZE:
                {
                    SwFmtFrmSize aSz(aMgr.GetFrmSize());
                    rSet.Put(aSz);
                }
                break;
                case RES_VERT_ORIENT:
                case RES_HORI_ORIENT:
                case SID_ATTR_ULSPACE:
                case SID_ATTR_LRSPACE:
                case RES_LR_SPACE:
                case RES_UL_SPACE:
                case RES_PROTECT:
                case RES_OPAQUE:
                case RES_PRINT:
                case RES_SURROUND:
                {
                    rSet.Put(aSet.Get(GetPool().GetWhich(nWhich), TRUE ));
                }
                break;
                case FN_FRAME_ALIGN_HORZ_CENTER:
                case FN_FRAME_ALIGN_HORZ_RIGHT:
                case FN_FRAME_ALIGN_HORZ_LEFT:
                    if ( (eFrmType & FRMTYPE_FLY_INCNT) ||
                            bProtect ||
                            nWhich == FN_FRAME_ALIGN_HORZ_CENTER && bHtmlMode )
                        rSet.DisableItem( nWhich );
                break;
                case FN_FRAME_ALIGN_VERT_ROW_TOP:
                case FN_FRAME_ALIGN_VERT_ROW_CENTER:
                case FN_FRAME_ALIGN_VERT_ROW_BOTTOM:
                case FN_FRAME_ALIGN_VERT_CHAR_TOP:
                case FN_FRAME_ALIGN_VERT_CHAR_CENTER:
                case FN_FRAME_ALIGN_VERT_CHAR_BOTTOM:
                    if ( !(eFrmType & FRMTYPE_FLY_INCNT) || bProtect
                            || bHtmlMode && FN_FRAME_ALIGN_VERT_CHAR_BOTTOM == nWhich )
                        rSet.DisableItem( nWhich );
                break;
                case FN_FRAME_ALIGN_VERT_TOP:
                case FN_FRAME_ALIGN_VERT_CENTER:
                case FN_FRAME_ALIGN_VERT_BOTTOM:
                    if ( bProtect || bHtmlMode && eFrmType & FRMTYPE_FLY_ATCNT)
                        rSet.DisableItem( nWhich );
                    else
                    {
                        USHORT nId = 0;
                        if (eFrmType & FRMTYPE_FLY_INCNT)
                        {
                            switch (nWhich)
                            {
                                case FN_FRAME_ALIGN_VERT_TOP:
                                    nId = STR_TOP_BASE; break;
                                case FN_FRAME_ALIGN_VERT_CENTER:
                                    nId = STR_CENTER_BASE;  break;
                                case FN_FRAME_ALIGN_VERT_BOTTOM:
                                    if(!bHtmlMode)
                                        nId = STR_BOTTOM_BASE;
                                    else
                                        rSet.DisableItem( nWhich );
                                break;
                            }
                        }
                        else
                        {
                            if (nWhich == FN_FRAME_ALIGN_VERT_CENTER ||
                                nWhich == FN_FRAME_ALIGN_VERT_BOTTOM)
                            {
                                if (aMgr.GetAnchor() == FLY_AT_FLY)
                                {
                                    const SwFrmFmt* pFmt = rSh.IsFlyInFly();
                                    if (pFmt)
                                    {
                                        const SwFmtFrmSize& rFrmSz = pFmt->GetFrmSize();
                                        if (rFrmSz.GetSizeType() != ATT_FIX_SIZE)
                                        {
                                            rSet.DisableItem( nWhich );
                                            break;
                                        }
                                    }
                                }
                            }
                            switch (nWhich)
                            {
                                case FN_FRAME_ALIGN_VERT_TOP:
                                    nId = STR_TOP; break;
                                case FN_FRAME_ALIGN_VERT_CENTER:
                                    nId = STR_CENTER_VERT; break;
                                case FN_FRAME_ALIGN_VERT_BOTTOM:
                                    nId = STR_BOTTOM; break;
                            }
                        }
                        if ( nId )
                            rSet.Put( SfxStringItem( nWhich, SW_RES(nId) ));
                    }
                break;
                case SID_HYPERLINK_GETLINK:
                {
                    String sURL;
                    SvxHyperlinkItem aHLinkItem;
                    const SfxPoolItem* pItem;

                    SfxItemSet aSet(GetPool(), RES_URL, RES_URL);
                    rSh.GetFlyFrmAttr( aSet );

                    if(SFX_ITEM_SET == aSet.GetItemState(RES_URL, TRUE, &pItem))
                    {
                        const SwFmtURL* pFmtURL = (const SwFmtURL*)pItem;
                        aHLinkItem.SetURL(pFmtURL->GetURL());
                        aHLinkItem.SetTargetFrame(pFmtURL->GetTargetFrameName());
                        aHLinkItem.SetName(rSh.GetFlyName());
                    }

                    aHLinkItem.SetInsertMode((SvxLinkInsertMode)(aHLinkItem.GetInsertMode() |
                        (bHtmlMode ? HLINK_HTMLMODE : 0)));

                    rSet.Put(aHLinkItem);
                }
                break;

                case FN_FRAME_CHAIN:
                {
                    const SwFrmFmt *pFmt = rSh.GetFlyFrmFmt();
                    if ( bParentCntProt || rSh.GetView().GetEditWin().GetApplyTemplate() ||
                         !pFmt || pFmt->GetChain().GetNext() )
                    {
                        rSet.DisableItem( FN_FRAME_CHAIN );
                    }
                    else
                    {
                        BOOL bChainMode = rSh.GetView().GetEditWin().IsChainMode();
                        rSet.Put( SfxBoolItem( FN_FRAME_CHAIN, bChainMode ) );
                    }
                    break;
                }

                case FN_FRAME_UNCHAIN:
                {
                    const SwFrmFmt *pFmt = rSh.GetFlyFrmFmt();
                    if ( bParentCntProt || rSh.GetView().GetEditWin().GetApplyTemplate() ||
                         !pFmt || !pFmt->GetChain().GetNext() )
                    {
                        rSet.DisableItem( FN_FRAME_UNCHAIN );
                    }
                    break;
                }

                case SID_FRAME_TO_TOP:
                case SID_FRAME_TO_BOTTOM:
                case FN_FRAME_UP:
                case FN_FRAME_DOWN:
                case FN_FORMAT_FRAME_DLG:
                    if ( bParentCntProt )
                        rSet.DisableItem( nWhich );
                    break;

                default:
                    /* do nothing */;
                    break;
            }
            nWhich = aIter.NextWhich();
        }
    }
}

/*--------------------------------------------------------------------
    Beschreibung:   Ctor fuer FrameShell
 --------------------------------------------------------------------*/


SwFrameShell::SwFrameShell(SwView &rView) :
    SwBaseShell( rView )
{
    SetName(String::CreateFromAscii("Frame"));
    SetHelpId(SW_FRAMESHELL);
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/



void SwFrameShell::ExecFrameStyle(SfxRequest& rReq)
{
    SwWrtShell &rSh = GetShell();
    BOOL bDefault = FALSE;
    if (!rSh.IsFrmSelected())
        return;

    // Erst Default-BoxItem aus Pool holen. Wenn ungleich normalem Boxitem,
    // dann ist es bereits geaendert worden (neues ist kein Default).
    const SvxBoxItem* pPoolBoxItem = (const SvxBoxItem*)::GetDfltAttr(RES_BOX);

    const SfxItemSet *pArgs = rReq.GetArgs();
    SfxItemSet aFrameSet(rSh.GetAttrPool(), RES_BOX, RES_BOX);

    rSh.GetFlyFrmAttr( aFrameSet );
    const SvxBoxItem& rBoxItem = (const SvxBoxItem&)aFrameSet.Get(RES_BOX);

    if (pPoolBoxItem == &rBoxItem)
        bDefault = TRUE;

    SvxBoxItem aBoxItem(rBoxItem);

    SvxBorderLine aBorderLine;
    const SfxPoolItem *pItem = 0;

    if(pArgs)    //irgendein Controller kann auch mal nichts liefern #48169#
    {
        switch (rReq.GetSlot())
        {
            case SID_ATTR_BORDER:
            {
                if (pArgs->GetItemState(RES_BOX, TRUE, &pItem) == SFX_ITEM_SET)
                {
                    SvxBoxItem aNewBox(*((SvxBoxItem *)pItem));
                    const SvxBorderLine* pBorderLine;

                    if ((pBorderLine = aBoxItem.GetTop()) != NULL)
                        lcl_FrmGetMaxLineWidth(pBorderLine, aBorderLine);
                    if ((pBorderLine = aBoxItem.GetBottom()) != NULL)
                        lcl_FrmGetMaxLineWidth(pBorderLine, aBorderLine);
                    if ((pBorderLine = aBoxItem.GetLeft()) != NULL)
                        lcl_FrmGetMaxLineWidth(pBorderLine, aBorderLine);
                    if ((pBorderLine = aBoxItem.GetRight()) != NULL)
                        lcl_FrmGetMaxLineWidth(pBorderLine, aBorderLine);

                    if(aBorderLine.GetOutWidth() == 0)
                    {
                        aBorderLine.SetInWidth(0);
                        aBorderLine.SetOutWidth(DEF_LINE_WIDTH_0);
                        aBorderLine.SetDistance(0);
                    }
                    //Distance nur setzen, wenn der Request vom Controller kommt

                    if(!StarBASIC::IsRunning())
                    {
                        aNewBox.SetDistance( rBoxItem.GetDistance() );
                    }

                    aBoxItem = aNewBox;
                    SvxBorderLine aDestBorderLine;

                    if ((pBorderLine = aBoxItem.GetTop()) != NULL)
                        aBoxItem.SetLine(&aBorderLine, BOX_LINE_TOP);
                    if ((pBorderLine = aBoxItem.GetBottom()) != NULL)
                        aBoxItem.SetLine(&aBorderLine, BOX_LINE_BOTTOM);
                    if ((pBorderLine = aBoxItem.GetLeft()) != NULL)
                        aBoxItem.SetLine(&aBorderLine, BOX_LINE_LEFT);
                    if ((pBorderLine = aBoxItem.GetRight()) != NULL)
                        aBoxItem.SetLine(&aBorderLine, BOX_LINE_RIGHT);
                }
            }
            break;

            case SID_FRAME_LINESTYLE:
            {
                if (pArgs->GetItemState(SID_FRAME_LINESTYLE, FALSE, &pItem) == SFX_ITEM_SET)
                {
                    const SvxLineItem* pLineItem =
                            (const SvxLineItem*)pItem;

                    if ( pLineItem->GetLine() )
                    {
                        aBorderLine = *(pLineItem->GetLine());

                        if (!aBoxItem.GetTop() && !aBoxItem.GetBottom() &&
                            !aBoxItem.GetLeft() && !aBoxItem.GetRight())
                        {
                            aBoxItem.SetLine(&aBorderLine, BOX_LINE_TOP);
                            aBoxItem.SetLine(&aBorderLine, BOX_LINE_BOTTOM);
                            aBoxItem.SetLine(&aBorderLine, BOX_LINE_LEFT);
                            aBoxItem.SetLine(&aBorderLine, BOX_LINE_RIGHT);
                        }
                        else
                        {
                            if( aBoxItem.GetTop() )
                            {
                                aBorderLine.SetColor( aBoxItem.GetTop()->GetColor() );
                                aBoxItem.SetLine(&aBorderLine, BOX_LINE_TOP);
                            }
                            if( aBoxItem.GetBottom() )
                            {
                                aBorderLine.SetColor( aBoxItem.GetBottom()->GetColor());
                                aBoxItem.SetLine(&aBorderLine, BOX_LINE_BOTTOM);
                            }
                            if( aBoxItem.GetLeft() )
                            {
                                aBorderLine.SetColor( aBoxItem.GetLeft()->GetColor());
                                aBoxItem.SetLine(&aBorderLine, BOX_LINE_LEFT);
                            }
                            if( aBoxItem.GetRight() )
                            {
                                aBorderLine.SetColor(aBoxItem.GetRight()->GetColor());
                                aBoxItem.SetLine(&aBorderLine, BOX_LINE_RIGHT);
                            }
                        }
                    }
                    else
                    {
                        aBoxItem.SetLine(0, BOX_LINE_TOP);
                        aBoxItem.SetLine(0, BOX_LINE_BOTTOM);
                        aBoxItem.SetLine(0, BOX_LINE_LEFT);
                        aBoxItem.SetLine(0, BOX_LINE_RIGHT);
                    }
                }
            }
            break;

            case SID_FRAME_LINECOLOR:
            {
                if (pArgs->GetItemState(SID_FRAME_LINECOLOR, FALSE, &pItem) == SFX_ITEM_SET)
                {
                    const Color& rNewColor = ((const SvxColorItem*)pItem)->GetValue();

                    if (!aBoxItem.GetTop() && !aBoxItem.GetBottom() &&
                        !aBoxItem.GetLeft() && !aBoxItem.GetRight())
                    {
                        aBorderLine.SetColor( rNewColor );
                        aBoxItem.SetLine(&aBorderLine, BOX_LINE_TOP);
                        aBoxItem.SetLine(&aBorderLine, BOX_LINE_BOTTOM);
                        aBoxItem.SetLine(&aBorderLine, BOX_LINE_LEFT);
                        aBoxItem.SetLine(&aBorderLine, BOX_LINE_RIGHT);
                    }
                    else
                    {
                        if ( aBoxItem.GetTop() )
                            ((SvxBorderLine*)aBoxItem.GetTop())->SetColor( rNewColor );
                        if ( aBoxItem.GetBottom() )
                            ((SvxBorderLine*)aBoxItem.GetBottom())->SetColor( rNewColor );
                        if ( aBoxItem.GetLeft() )
                            ((SvxBorderLine*)aBoxItem.GetLeft())->SetColor( rNewColor );
                        if ( aBoxItem.GetRight() )
                            ((SvxBorderLine*)aBoxItem.GetRight())->SetColor( rNewColor );
                    }
                }
            }
            break;
        }
    }
    if (bDefault && (aBoxItem.GetTop() || aBoxItem.GetBottom() ||
        aBoxItem.GetLeft() || aBoxItem.GetRight()))
    {
        aBoxItem.SetDistance(MIN_BORDER_DIST);
    }
    aFrameSet.Put( aBoxItem );
    // Vorlagen-AutoUpdate
    SwFrmFmt* pFmt = rSh.GetCurFrmFmt();
    if(pFmt && pFmt->IsAutoUpdateFmt())
    {
        rSh.AutoUpdateFrame(pFmt, aFrameSet);
    }
    else
        rSh.SetFlyFrmAttr( aFrameSet );

}



void lcl_FrmGetMaxLineWidth(const SvxBorderLine* pBorderLine, SvxBorderLine& rBorderLine)
{
    if(pBorderLine->GetInWidth() > rBorderLine.GetInWidth())
        rBorderLine.SetInWidth(pBorderLine->GetInWidth());

    if(pBorderLine->GetOutWidth() > rBorderLine.GetOutWidth())
        rBorderLine.SetOutWidth(pBorderLine->GetOutWidth());

    if(pBorderLine->GetDistance() > rBorderLine.GetDistance())
        rBorderLine.SetDistance(pBorderLine->GetDistance());

    rBorderLine.SetColor(pBorderLine->GetColor());
}



void SwFrameShell::GetLineStyleState(SfxItemSet &rSet)
{
    SwWrtShell &rSh = GetShell();
    BOOL bParentCntProt = rSh.IsSelObjProtected( (FlyProtectType)(FLYPROTECT_CONTENT|FLYPROTECT_PARENT) ) != 0;

    if (bParentCntProt)
    {
        if (rSh.IsFrmSelected())
            rSet.DisableItem( SID_FRAME_LINECOLOR );

        rSet.DisableItem( SID_ATTR_BORDER );
        rSet.DisableItem( SID_FRAME_LINESTYLE );
    }
    else
    {
        if (rSh.IsFrmSelected())
        {
            SfxItemSet aFrameSet( rSh.GetAttrPool(), RES_BOX, RES_BOX );

            rSh.GetFlyFrmAttr(aFrameSet);

            const SvxBorderLine* pLine = ((const SvxBoxItem&)aFrameSet.Get(RES_BOX)).GetTop();
            rSet.Put(SvxColorItem(pLine ? pLine->GetColor() : Color(), SID_FRAME_LINECOLOR));
        }
        rSet.Put(SfxUInt16Item(SID_ATTR_BORDER, TRUE)); // Rahmencontroller: letzte Zeile einklappen
    }
}



void  SwFrameShell::GetBckColState(SfxItemSet &rSet)
{
    SwBaseShell::GetBckColState( rSet );
}



void  SwFrameShell::ExecBckCol(SfxRequest& rReq)
{
    SwBaseShell::ExecBckCol( rReq );
}



void  SwFrameShell::StateInsert(SfxItemSet &rSet)
{
    const int nSel = GetShell().GetSelectionType();

    if ((nSel & SwWrtShell::SEL_GRF) || (nSel & SwWrtShell::SEL_OLE))
        rSet.DisableItem(FN_INSERT_FRAME);
}

/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.1.1.1  2000/09/18 17:14:46  hr
    initial import

    Revision 1.199  2000/09/18 16:06:04  willem.vandorp
    OpenOffice header added.

    Revision 1.198  2000/09/07 15:59:29  os
    change: SFX_DISPATCHER/SFX_BINDINGS removed

    Revision 1.197  2000/08/17 11:42:59  jp
    remove the SW graphicmanager

    Revision 1.196  2000/05/26 07:21:32  os
    old SW Basic API Slots removed

    Revision 1.195  2000/05/10 11:53:01  os
    Basic API removed

    Revision 1.194  2000/04/20 12:51:52  os
    GetName() returns String&

    Revision 1.193  2000/04/18 14:58:23  os
    UNICODE

    Revision 1.192  2000/03/03 15:17:03  os
    StarView remainders removed

    Revision 1.191  1999/09/24 14:38:32  os
    hlnkitem.hxx now in SVX

    Revision 1.190  1999/07/30 15:39:08  HR
    #65293#: conflict with <goodies/grfmgr.hxx>


      Rev 1.189   30 Jul 1999 17:39:08   HR
   #65293#: conflict with <goodies/grfmgr.hxx>

      Rev 1.188   02 Feb 1999 13:35:28   JP
   Task #58423#: InsertDBCol - optional kann eine Rahmenvorlage mitgegeben werden

      Rev 1.187   29 Jan 1999 17:19:38   JP
   Task #58423#: Reports mit Bildern aus der Datenbank

      Rev 1.186   27 Nov 1998 15:50:18   AMA
   Fix #59951#59825#: Unterscheiden zwischen Rahmen-,Seiten- und Bereichsspalten

      Rev 1.185   08 Sep 1998 17:03:06   OS
   #56134# Metric fuer Text und HTML getrennt

------------------------------------------------------------------------*/



