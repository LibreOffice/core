/*************************************************************************
 *
 *  $RCSfile: textsh1.cxx,v $
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
#ifndef _CMDID_H
#include <cmdid.h>
#endif
#ifndef _HELPID_H
#include <helpid.h>
#endif

#ifndef _SFXSTRITEM_HXX
#include <svtools/stritem.hxx>
#endif
#ifndef _SVX_HTMLMODE_HXX //autogen
#include <svx/htmlmode.hxx>
#endif
#ifndef _SFX_WHITER_HXX //autogen
#include <svtools/whiter.hxx>
#endif
#ifndef _SFX_BINDINGS_HXX //autogen
#include <sfx2/bindings.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SFX_OBJITEM_HXX //autogen
#include <sfx2/objitem.hxx>
#endif
#ifndef _OFF_APP_HXX //autogen
#include <offmgr/app.hxx>
#endif
#ifndef _OFAACCFG_HXX //autogen
#include <offmgr/ofaaccfg.hxx>
#endif
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _SFXMACITEM_HXX
#include <svtools/macitem.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX //autogen
#include <svx/lrspitem.hxx>
#endif
#ifndef _SVX_ULSPITEM_HXX //autogen
#include <svx/ulspitem.hxx>
#endif
#ifndef _SVX_COLRITEM_HXX //autogen
#include <svx/colritem.hxx>
#endif
#ifndef _SVX_TSTPITEM_HXX //autogen
#include <svx/tstpitem.hxx>
#endif
#ifndef _SVX_BRSHITEM_HXX //autogen
#include <svx/brshitem.hxx>
#endif
#ifndef _MySVXACORR_HXX //autogen
#include <svx/svxacorr.hxx>
#endif


#ifndef _FMTINFMT_HXX //autogen
#include <fmtinfmt.hxx>
#endif
#ifndef _SWWAIT_HXX
#include <swwait.hxx>
#endif
#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _WVIEW_HXX
#include <wview.hxx>
#endif
#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif
#ifndef _VIEWOPT_HXX
#include <viewopt.hxx>
#endif
#ifndef _UITOOL_HXX
#include <uitool.hxx>
#endif
#ifndef _UIPARAM_HXX
#include <uiparam.hxx>
#endif
#ifndef _SWEVENT_HXX
#include <swevent.hxx>
#endif
#ifndef _TEXTSH_HXX
#include <textsh.hxx>
#endif
#ifndef _BOOKMARK_HXX
#include <bookmark.hxx>
#endif
#ifndef _BOOKMRK_HXX
#include <bookmrk.hxx>
#endif
#ifndef _BREAK_HXX
#include <break.hxx>
#endif
#ifndef _DATAEX_HXX
#include <dataex.hxx>
#endif
#ifndef _DOCSTAT_HXX
#include <docstat.hxx>
#endif
#ifndef _INSFNOTE_HXX
#include <insfnote.hxx>
#endif
#ifndef _OUTLINE_HXX
#include <outline.hxx>
#endif
#ifndef _SRTDLG_HXX
#include <srtdlg.hxx>
#endif
#ifndef _TABLEMGR_HXX
#include <tablemgr.hxx>
#endif
#ifndef _SWUNDO_HXX
#include <swundo.hxx>       // fuer Undo-IDs
#endif
#ifndef _AUTOFDLG_HXX
#include <autofdlg.hxx>
#endif
#ifndef _REFFLD_HXX
#include <reffld.hxx>
#endif
#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
#ifndef _MDIEXP_HXX
#include <mdiexp.hxx>
#endif
#ifndef _INPUTWIN_HXX
#include <inputwin.hxx>
#endif
#ifndef _PARDLG_HXX
#include <pardlg.hxx>
#endif
#ifndef _CHRDLG_HXX
#include <chrdlg.hxx>
#endif
#ifndef _DOCFNOTE_HXX
#include <docfnote.hxx>
#endif
#ifndef _FRMATR_HXX
#include <frmatr.hxx>
#endif
#ifndef _FMTCOL_HXX
#include <fmtcol.hxx>
#endif
#ifndef _CELLATR_HXX
#include <cellatr.hxx>
#endif
#ifndef _EDTWIN_HXX
#include <edtwin.hxx>
#endif
#ifndef _REDLNDLG_HXX
#include <redlndlg.hxx>
#endif
#ifndef _REDLENUM_HXX
#include <redlenum.hxx>
#endif

#ifndef _GLOBALS_HRC
#include <globals.hrc>
#endif
#ifndef _SHELLS_HRC
#include <shells.hrc>
#endif
#ifndef _APP_HRC
#include <app.hrc>
#endif
#ifndef _WEB_HRC
#include <web.hrc>
#endif

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

short lcl_AskRedlineMode(Window *pWin)
{
    MessBox aQBox( pWin, 0,
                    String( SW_RES( STR_REDLINE_TITLE ) ),
                    String( SW_RES( STR_REDLINE_MSG ) ) );
    aQBox.SetImage( QueryBox::GetStandardImage() );
    USHORT nBtnFlags = BUTTONDIALOG_DEFBUTTON |
                        BUTTONDIALOG_OKBUTTON |
                        BUTTONDIALOG_FOCUSBUTTON;

    aQBox.AddButton(String(SW_RES(STR_REDLINE_ACCEPT_ALL)), RET_OK, nBtnFlags);
    aQBox.GetPushButton( RET_OK )->SetHelpId(HID_AUTOFORMAT_ACCEPT);
    aQBox.AddButton(String(SW_RES(STR_REDLINE_REJECT_ALL)), RET_CANCEL, BUTTONDIALOG_CANCELBUTTON);
    aQBox.GetPushButton( RET_CANCEL )->SetHelpId(HID_AUTOFORMAT_REJECT  );
    aQBox.AddButton(String(SW_RES(STR_REDLINE_EDIT)), 2, 0);
    aQBox.GetPushButton( 2 )->SetHelpId(HID_AUTOFORMAT_EDIT_CHG);
    aQBox.SetButtonHelpText( RET_OK, aEmptyStr );

    return aQBox.Execute();
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

void SwTextShell::Execute(SfxRequest &rReq)
{
    const SfxItemSet *pArgs = rReq.GetArgs();
    SwWrtShell& rWrtSh = GetShell();
    const SfxPoolItem* pItem = 0;
    USHORT nSlot = rReq.GetSlot();
    if(pArgs)
        pArgs->GetItemState(GetPool().GetWhich(nSlot), FALSE, &pItem);
    switch( nSlot )
    {
        case FN_INSERT_SYMBOL:
            InsertSymbol(aEmptyStr,aEmptyStr);
        break;
        case FN_INSERT_FOOTNOTE:
        case FN_INSERT_ENDNOTE:
            rWrtSh.InsertFootnote(aEmptyStr, nSlot == FN_INSERT_ENDNOTE);
        break;
        case FN_INSERT_FOOTNOTE_DLG:
        {
            SwInsFootNoteDlg *pDlg = new SwInsFootNoteDlg(
                                GetView().GetWindow(), rWrtSh, FALSE);
            pDlg->SetHelpId(nSlot);
            pDlg->Execute();
            delete pDlg;
        }
        break;
        case FN_FORMAT_FOOTNOTE_DLG:
        {
            SwFootNoteOptionDlg *pDlg = new SwFootNoteOptionDlg(GetView().GetWindow(), rWrtSh);
            pDlg->Execute();
            delete pDlg;
            break;
        }
        case SID_INSERT_GRAPHIC:
            if (!pItem)
            {
                rReq.SetReturnValue(SfxBoolItem(nSlot, InsertGraphicDlg()));
            }
            else
            {
                String rName = aEmptyStr;
                BOOL bLink = FALSE;

                rName = ((const SfxStringItem *)pItem)->GetValue();

                String rFilter = aEmptyStr;
                if ( SFX_ITEM_SET ==
                        pArgs->GetItemState(FN_PARAM_FILTER, TRUE, &pItem) )
                    rFilter = ((const SfxStringItem *)pItem)->GetValue();

                if ( SFX_ITEM_SET ==
                        pArgs->GetItemState(FN_PARAM_1, TRUE, &pItem) )
                    bLink = ((const SfxBoolItem *)pItem)->GetValue();

                if ( !rName.Len() )
                    rReq.SetReturnValue(SfxBoolItem(nSlot, InsertGraphicDlg()));
                else
                    rReq.SetReturnValue(SfxBoolItem(nSlot, InsertGraphic( rName, rFilter, bLink )));
            }
            break;
        case SID_INSERTDOC:
            if (!pItem)
                rReq.SetReturnValue(SfxBoolItem(nSlot, GetView().InsertDoc(nSlot, aEmptyStr, aEmptyStr) != -1));
            else
            {
                String aFile    = aEmptyStr;
                String aFilter  = aEmptyStr;
                aFile = ((const SfxStringItem *)pItem)->GetValue();
                if(SFX_ITEM_SET == pArgs->GetItemState(FN_PARAM_1, TRUE, &pItem ))
                    aFilter = ((const SfxStringItem *)pItem)->GetValue();

                rReq.SetReturnValue(SfxBoolItem(nSlot, GetView().InsertDoc( nSlot, aFile, aFilter ) != -1));
            }
            break;
        case FN_FORMAT_RESET:
            rWrtSh.ResetAttr();
            break;
        case FN_INSERT_BREAK_DLG:
        {
            SwBreakDlg *pDlg = new SwBreakDlg(GetView().GetWindow(), rWrtSh);
            pDlg->Execute();
            delete pDlg;
        }
            break;
        case FN_INSERT_BOOKMARK:
        {
            SwInsertBookmarkDlg *pDlg = new SwInsertBookmarkDlg(
                                    GetView().GetWindow(), rWrtSh );
            pDlg->Execute();
            delete pDlg;
        }
        break;
        case FN_AUTOFORMAT_REDLINE_APPLY:
        {
            SvxSwAutoFmtFlags aFlags(OFF_APP()->GetAutoCorrect()->GetSwFlags());
            // das muss fuer die Nachbearbeitung immer FALSE sein
            aFlags.bAFmtByInput = FALSE;
            aFlags.bWithRedlining = TRUE;
            rWrtSh.AutoFormat( &aFlags );
            aFlags.bWithRedlining = FALSE;

            SfxViewFrame* pVFrame = GetView().GetViewFrame();
            if (pVFrame->HasChildWindow(FN_REDLINE_ACCEPT))
                pVFrame->ToggleChildWindow(FN_REDLINE_ACCEPT);

            SwModalRedlineAcceptDlg aDlg(&GetView().GetEditWin());

            switch (lcl_AskRedlineMode(&GetView().GetEditWin()))
            {
                case RET_OK:
                    aDlg.AcceptAll(TRUE);
                    break;

                case RET_CANCEL:
                    aDlg.AcceptAll(FALSE);
                    break;

                case 2:
                    aDlg.Execute();
                    break;
            }
        }
        break;

        case FN_AUTOFORMAT_APPLY:
        {
            SvxSwAutoFmtFlags aFlags(OFF_APP()->GetAutoCorrect()->GetSwFlags());
            // das muss fuer die Nachbearbeitung immer FALSE sein
            aFlags.bAFmtByInput = FALSE;
            rWrtSh.AutoFormat( &aFlags );
        }
        break;
        case FN_AUTOFORMAT_AUTO:
        {
            OfaAutoCorrCfg* pACfg = OFF_APP()->GetAutoCorrConfig();
            BOOL bSet = pItem ? ((const SfxBoolItem*)pItem)->GetValue()
                              : !pACfg->IsAutoFmtByInput();
            if( bSet != pACfg->IsAutoFmtByInput() )
                pACfg->SetAutoFmtByInput( bSet );
        }
        break;
        case FN_AUTO_CORRECT:
        {
            // erstmal auf Blank defaulten
            sal_Unicode cChar = ' ';
            rWrtSh.AutoCorrect( *OFF_APP()->GetAutoCorrect(), cChar );
        }
        break;

        case FN_SORTING_DLG:
        {
            SwSortDlg *pDlg = new SwSortDlg(GetView().GetWindow(), rWrtSh );
            pDlg->Execute();
            delete pDlg;
        }
        break;
        case FN_NUMBERING_OUTLINE_DLG:
        {
            SfxItemSet aTmp(GetPool(), FN_PARAM_1, FN_PARAM_1);
            SwOutlineTabDialog* pDlg = new SwOutlineTabDialog(GetView().GetWindow(), &aTmp, rWrtSh);
            pDlg->Execute();
            delete pDlg;
        }
            break;
        case FN_CALCULATE:
        {
            SwDataExchangeRef aRef( new SwDataExchange( rWrtSh ) );
            aRef->CalculateAndCopy();
        }
            break;
        case FN_GOTO_REFERENCE:
        {
            SwField *pFld = rWrtSh.GetCurFld();
            if(pFld && pFld->GetTypeId() == TYP_GETREFFLD)
            {
                rWrtSh.StartAllAction();
                rWrtSh.SwCrsrShell::GotoRefMark( ((SwGetRefField*)pFld)->GetSetRefName(),
                                    ((SwGetRefField*)pFld)->GetSubType(),
                                    ((SwGetRefField*)pFld)->GetSeqNo() );
                rWrtSh.EndAllAction();
            }
        }
            break;
        case FN_EDIT_FORMULA:
        {
            rWrtSh.EndAllTblBoxEdit();
            SfxViewFrame* pVFrame = GetView().GetViewFrame();
            const USHORT nId = SwInputChild::GetChildWindowId();
            pVFrame->ToggleChildWindow( nId );
            if( !pVFrame->HasChildWindow( nId ) )
                pVFrame->GetBindings().InvalidateAll( TRUE );
        }

        break;
        case FN_TABLE_UNSET_READ_ONLY:
        {
            rWrtSh.UnProtectTbls();
        }
        break;
        case FN_EDIT_HYPERLINK:
            GetView().GetViewFrame()->ToggleChildWindow(SID_HYPERLINK_DIALOG);
        break;
        case FN_INSERT_HYPERLINK:
        case SID_CHAR_DLG:
        {
            SwView* pView = GetActiveView();
            if(pView)
            {
                FieldUnit eMetric = ::GetDfltMetric(0 != PTR_CAST(SwWebView, pView));
                SW_MOD()->PutItem(SfxUInt16Item(SID_ATTR_METRIC, eMetric));
                SfxItemSet aCoreSet( GetPool(),
                                 RES_CHRATR_BEGIN,      RES_CHRATR_END-1,
                                 RES_TXTATR_INETFMT,    RES_TXTATR_INETFMT,
                                 RES_BACKGROUND,        RES_BACKGROUND,
                                 FN_PARAM_SELECTION,    FN_PARAM_SELECTION,
                                 SID_HTML_MODE,         SID_HTML_MODE,
                                 0 );
                rWrtSh.GetAttr( aCoreSet );
                BOOL bSel = rWrtSh.HasSelection();
                BOOL bSelectionPutted = FALSE;
                if(bSel || rWrtSh.IsInWord())
                {
                    if(!bSel)
                    {
                        rWrtSh.StartAction();
                        rWrtSh.Push();
                        if(!rWrtSh.SelectTxtAttr( RES_TXTATR_INETFMT ))
                            rWrtSh.SelWrd();
                    }
                    aCoreSet.Put(SfxStringItem(FN_PARAM_SELECTION, rWrtSh.GetSelTxt()));
                    bSelectionPutted = TRUE;
                    if(!bSel)
                    {
                        rWrtSh.Pop(FALSE);
                        rWrtSh.EndAction();
                    }
                }

                // Das CHRATR_BACKGROUND-Attribut wird fuer den Dialog in
                // ein RES_BACKGROUND verwandelt und wieder zurueck ...
                const SfxPoolItem *pTmpBrush;
                if( SFX_ITEM_SET == aCoreSet.GetItemState( RES_CHRATR_BACKGROUND,
                    TRUE, &pTmpBrush ) )
                {
                    SvxBrushItem aTmpBrush( *((SvxBrushItem*)pTmpBrush) );
                    aTmpBrush.SetWhich( RES_BACKGROUND );
                    aCoreSet.Put( aTmpBrush );
                }

                aCoreSet.Put(SfxUInt16Item(SID_HTML_MODE,
                                ::GetHtmlMode(GetView().GetDocShell())));
                SwCharDlg* pDlg = new SwCharDlg(GetView().GetWindow(), GetView(), aCoreSet);
                if(FN_INSERT_HYPERLINK == nSlot)
                    pDlg->SetCurPageId(TP_CHAR_URL);
                if (RET_OK == pDlg->Execute())
                {
                    SfxItemSet aTmpSet( *pDlg->GetOutputItemSet() );
                    if( SFX_ITEM_SET == aTmpSet.GetItemState( RES_BACKGROUND,
                        FALSE, &pTmpBrush ) )
                    {
                        SvxBrushItem aTmpBrush( *((SvxBrushItem*)pTmpBrush) );
                        aTmpBrush.SetWhich( RES_CHRATR_BACKGROUND );
                        aTmpSet.Put( aTmpBrush );
                    }
                    aTmpSet.ClearItem( RES_BACKGROUND );

                    const SfxPoolItem* pItem;
                    BOOL bInsert = FALSE;

                    // aus ungeklaerter Ursache ist das alte Item wieder im Set
                    if(!bSelectionPutted && SFX_ITEM_SET == aTmpSet.GetItemState(FN_PARAM_SELECTION, FALSE, &pItem))
                    {
                        String sInsert = ((const SfxStringItem*)pItem)->GetValue();
                        bInsert = sInsert.Len() != 0;
                        if(bInsert)
                        {
                            rWrtSh.StartAction();
                            rWrtSh.Insert( sInsert );
                            rWrtSh.SetMark();
                            rWrtSh.ExtendSelection(FALSE, sInsert.Len());
                        }
                    }
                    SwTxtFmtColl* pColl = rWrtSh.GetCurTxtFmtColl();
                    if(bSel &&
                            rWrtSh.IsSelFullPara() &&
                                pColl && pColl->IsAutoUpdateFmt())
                    {
                        rWrtSh.AutoUpdatePara(pColl, aTmpSet);
                    }
                    else
                        rWrtSh.SetAttr( aTmpSet );
                    if(bInsert)
                    {
                        rWrtSh.SwapPam();
                        rWrtSh.ClearMark();
                        rWrtSh.DontExpandFmt();
                        rWrtSh.EndAction();
                    }
                }
                delete pDlg;
            }
        }
        break;
        case SID_PARA_DLG:
        {
        //Damit aus dem Basic keine Dialoge fuer Hintergrund-Views aufgerufen werden:
            SwView* pView = GetActiveView();
            if(pView)
            {
                FieldUnit eMetric = ::GetDfltMetric(0 != PTR_CAST(SwWebView, pView));
                SW_MOD()->PutItem(SfxUInt16Item(SID_ATTR_METRIC, eMetric));
                SfxItemSet aCoreSet( GetPool(),
                                RES_PARATR_BEGIN,           RES_PARATR_END - 1,
                                RES_FRMATR_BEGIN,           RES_FRMATR_END - 1,
                                SID_ATTR_TABSTOP_POS,       SID_ATTR_TABSTOP_POS,
                                SID_ATTR_TABSTOP_DEFAULTS,  SID_ATTR_TABSTOP_DEFAULTS,
                                SID_ATTR_TABSTOP_OFFSET,    SID_ATTR_TABSTOP_OFFSET,
                                SID_ATTR_BORDER_INNER,      SID_ATTR_BORDER_INNER,
                                SID_ATTR_PARA_MODEL,        SID_ATTR_PARA_KEEP,
                                SID_ATTR_PARA_PAGENUM,      SID_ATTR_PARA_PAGENUM,
                                SID_HTML_MODE,              SID_HTML_MODE,
                                FN_PARAM_1,                 FN_PARAM_1,
                                FN_NUMBER_NEWSTART,         FN_NUMBER_NEWSTART_AT,
                                0);
                rWrtSh.GetAttr( aCoreSet );
                aCoreSet.Put(SfxUInt16Item(SID_HTML_MODE,
                                ::GetHtmlMode(GetView().GetDocShell())));

                // Tabulatoren, DefaultTabs ins ItemSet Stecken
                const SvxTabStopItem& rDefTabs = (const SvxTabStopItem&)
                                GetPool().GetDefaultItem(RES_PARATR_TABSTOP);

                USHORT nDefDist = ::GetTabDist( rDefTabs );
                SfxUInt16Item aDefDistItem( SID_ATTR_TABSTOP_DEFAULTS, nDefDist );
                aCoreSet.Put( aDefDistItem );

                // Aktueller Tab
                SfxUInt16Item aTabPos( SID_ATTR_TABSTOP_POS, 0 );
                aCoreSet.Put( aTabPos );

                // linker Rand als Offset
                const long nOff = ((SvxLRSpaceItem&)aCoreSet.Get( RES_LR_SPACE )).
                                                                    GetTxtLeft();
                SfxInt32Item aOff( SID_ATTR_TABSTOP_OFFSET, nOff );
                aCoreSet.Put( aOff );


                // BoxInfo setzen
                ::PrepareBoxInfo( aCoreSet, rWrtSh );

                //aktuelles Seitenformat
                ::SwToSfxPageDescAttr( aCoreSet );

                UINT16 nDefPage = 0;
                if( pItem )
                    nDefPage = ((SfxUInt16Item *)pItem)->GetValue();

                // Numerierungseigenschaften
                if(rWrtSh.GetCurNumRule())
                {
                    aCoreSet.Put(SfxBoolItem(FN_NUMBER_NEWSTART,
                                        USHRT_MAX != rWrtSh.IsNodeNumStart()));
                    aCoreSet.Put(SfxUInt16Item(FN_NUMBER_NEWSTART_AT,
                                                    rWrtSh.IsNodeNumStart()));
                }
                SwParaDlg *pDlg = new SwParaDlg( GetView().GetWindow(),
                                                 GetView(), aCoreSet, DLG_STD,
                                                 NULL, FALSE, nDefPage );

                if ( pDlg->Execute() == RET_OK )
                {
                    // Defaults evtl umsetzen
                    SfxItemSet* pSet = (SfxItemSet*)pDlg->GetOutputItemSet();
                    USHORT nNewDist;
                    if( SFX_ITEM_SET == pSet->GetItemState( SID_ATTR_TABSTOP_DEFAULTS,
                        FALSE, &pItem ) &&
                        nDefDist != (nNewDist = ((SfxUInt16Item*)pItem)->GetValue()) )
                    {
                        SvxTabStopItem aDefTabs( 0, 0 );
                        MakeDefTabs( nNewDist, aDefTabs );
                        rWrtSh.SetDefault( aDefTabs );
                        pSet->ClearItem( SID_ATTR_TABSTOP_DEFAULTS );
                    }

                    ::SfxToSwPageDescAttr( rWrtSh, *pSet );

                    if( pSet->Count() )
                    {
                        rWrtSh.StartAction();
                        rWrtSh.StartUndo( UNDO_START );
                        if ( SFX_ITEM_SET == pSet->GetItemState(FN_PARAM_1,FALSE,&pItem) )
                        {
                            if ( ((SfxStringItem*)pItem)->GetValue().Len() )
                                rWrtSh.ReplaceDropTxt(((SfxStringItem*)pItem)->GetValue());
                        }
                        rWrtSh.SetAttr( *pSet );
                        rWrtSh.EndUndo( UNDO_END );
                        rWrtSh.EndAction();
                        SwTxtFmtColl* pColl = rWrtSh.GetCurTxtFmtColl();
                        if(pColl && pColl->IsAutoUpdateFmt())
                        {
                            rWrtSh.AutoUpdatePara(pColl, *pSet);
                        }
                    }
                    if(SFX_ITEM_SET == pSet->GetItemState(FN_NUMBER_NEWSTART))
                    {
                        BOOL bStart = ((SfxBoolItem&)pSet->Get(FN_NUMBER_NEWSTART_AT)).GetValue();
                        // das zweite Item muss immer drin sein!
                        USHORT nNumStart = ((SfxUInt16Item&)pSet->Get(FN_NUMBER_NEWSTART_AT)).GetValue();
                        if(!bStart)
                            nNumStart = USHRT_MAX;
                        rWrtSh.SetNodeNumStart(nNumStart);
                    }
                }
                delete pDlg;
            }
        }
        break;
        case FN_SELECT_PARA:
        {
            if(!rWrtSh.IsSttOfPara())
                rWrtSh.SttPara(FALSE);
            else
                rWrtSh.EnterStdMode();
            rWrtSh.EndPara(TRUE);
        }
        break;

        case SID_DEC_INDENT:
        case SID_INC_INDENT:
            rWrtSh.MoveLeftMargin( SID_INC_INDENT == nSlot,
                                    rReq.GetModifier() != KEY_MOD1 );
            break;
        case FN_DEC_INDENT_OFFSET:
        case FN_INC_INDENT_OFFSET:
            rWrtSh.MoveLeftMargin( FN_INC_INDENT_OFFSET == nSlot,
                                    rReq.GetModifier() == KEY_MOD1 );
            break;

        case SID_ATTR_CHAR_COLOR2:
        {
            if(pItem)
            {
                Color aSet = ((const SvxColorItem*)pItem)->GetValue();
                SwView& rView = GetView();
                SwEditWin& rEditWin = rView.GetEditWin();
                rEditWin.SetTextColor(aSet);
                SwApplyTemplate* pApply = rEditWin.GetApplyTemplate();
                SvxColorItem aItem(aSet, RES_CHRATR_COLOR);

                // besteht eine Selektion, wird sie gleich gefaerbt
                if(!pApply && rWrtSh.HasSelection())
                {
                    SvxColorItem aItem(aSet, RES_CHRATR_COLOR);
                    rWrtSh.SetAttr(aItem);
                }
                else if(!pApply || pApply->nColor != SID_ATTR_CHAR_COLOR_EXT)
                {
                    rView.GetViewFrame()->GetDispatcher()->Execute(SID_ATTR_CHAR_COLOR_EXT);
                }

/*              OS 22.02.97 18:40 Das alte Verhalten ist unerwuenscht
                SwEditWin& rEdtWin = GetView().GetEditWin();

                SwApplyTemplate* pApply = rEdtWin.GetApplyTemplate();
                SvxColorItem aItem(aSet, RES_CHRATR_COLOR);

                if(!pApply || pApply->nColor != SID_ATTR_CHAR_COLOR_EXT)
                {
                    GetShell().SetAttr(aItem);
                }
*/
            }
        }
        break;
        case SID_ATTR_CHAR_COLOR_BACKGROUND:
        {
            SwView& rView = GetView();
            SwEditWin& rEdtWin = rView.GetEditWin();
            SwApplyTemplate* pApply = rEdtWin.GetApplyTemplate();
            rEdtWin.SetTextBackColorTransparent(0 == pItem);
            Color aSet;
            if(pItem)
            {
                aSet = ((const SvxColorItem*)pItem)->GetValue();
                rEdtWin.SetTextBackColor(aSet);
            }
            if(!pApply && (rWrtSh.HasSelection() || rReq.IsAPI()))
            {
                SvxBrushItem aBrushItem(RES_CHRATR_BACKGROUND);
                if(pItem)
                    aBrushItem.SetColor(aSet);
                else
                    aBrushItem.SetColor(Color(COL_TRANSPARENT));
                rWrtSh.SetAttr( aBrushItem );
            }
            else if(!pApply || pApply->nColor != SID_ATTR_CHAR_COLOR_BACKGROUND_EXT)
            {
                rView.GetViewFrame()->GetDispatcher()->Execute(SID_ATTR_CHAR_COLOR_BACKGROUND_EXT);
            }
/*          OS 22.02.97 18:40 Das alte Verhalten ist unerwuenscht
            if(!pApply || pApply->nColor != SID_ATTR_CHAR_COLOR_BACKGROUND_EXT)
            {
                Brush aBrush(pItem ? BRUSH_SOLID : BRUSH_NULL);
                if(pItem)
                    aBrush.SetColor( aSet );
                GetShell().SetAttr( SvxBrushItem(aBrush, RES_CHRATR_BACKGROUND) );
            }
*/
        }
        break;
        case SID_ATTR_CHAR_COLOR_BACKGROUND_EXT:
        case SID_ATTR_CHAR_COLOR_EXT:
        {
            SwEditWin& rEdtWin = GetView().GetEditWin();
            SwApplyTemplate* pApply = rEdtWin.GetApplyTemplate();
            SwApplyTemplate aTempl;
            BOOL bSelection = rWrtSh.HasSelection();
            if(bSelection)
            {

                if(nSlot == SID_ATTR_CHAR_COLOR_BACKGROUND_EXT)
                {
                    rWrtSh.SetAttr( SvxBrushItem(
                        rEdtWin.GetTextBackColor(), RES_CHRATR_BACKGROUND) );
                }
                else
                    rWrtSh.SetAttr( SvxColorItem( rEdtWin.GetTextColor(),
                                                                RES_CHRATR_COLOR) );
            }
            else
            {
                if(!pApply || pApply->nColor != nSlot)
                    aTempl.nColor = nSlot;
                rEdtWin.SetApplyTemplate(aTempl);
            }
        }
        break;

        case FN_NUM_BULLET_MOVEDOWN:
            if (!rWrtSh.IsAddMode())
                rWrtSh.MoveParagraph(1);
            break;

        case FN_NUM_BULLET_MOVEUP:
            if (!rWrtSh.IsAddMode())
                rWrtSh.MoveParagraph(-1);
            break;

        case SID_HYPERLINK_DIALOG:
        {
            SfxRequest aReq(SID_HYPERLINK_DIALOG, SFX_CALLMODE_SLOT, SFX_APP()->GetPool());
            GetView().GetViewFrame()->ExecuteSlot( aReq);
        }
        break;

    default:
        ASSERT(!this, falscher Dispatcher);
        return;
    }
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/


void SwTextShell::GetState( SfxItemSet &rSet )
{
    SwWrtShell &rSh = GetShell();
    SfxWhichIter aIter( rSet );
    USHORT nWhich = aIter.FirstWhich();
    while ( nWhich )
    {
        switch ( nWhich )
        {
        case FN_NUM_NUMBERING_ON:
        case FN_NUM_BULLET_ON:
            if ( rSh.IsTableMode() )
                rSet.DisableItem( nWhich );
            break;

        case FN_NUMBER_BULLETS:
            if ( rSh.IsTableMode() )
                rSet.DisableItem(nWhich);
            break;

        case FN_EDIT_FORMULA:
        case FN_INSERT_SYMBOL:
            {
                const int nType = rSh.GetSelectionType();
                if (!(nType & SwWrtShell::SEL_TXT) &&
                    !(nType & SwWrtShell::SEL_TBL) &&
                    !(nType & SwWrtShell::SEL_NUM))
                    rSet.DisableItem(nWhich);
            }
            break;

        case FN_INSERT_ENDNOTE:
        case FN_INSERT_FOOTNOTE:
        case FN_INSERT_FOOTNOTE_DLG:
            {
                const USHORT nNoType = FRMTYPE_FLY_ANY | FRMTYPE_HEADER |
                                        FRMTYPE_FOOTER  | FRMTYPE_FOOTNOTE;
                if ( (rSh.GetFrmType(0,TRUE) & nNoType) )
                    rSet.DisableItem(nWhich);
            }
            break;
        case FN_INSERT_TABLE:
            if ( rSh.GetTableFmt() ||
                    (rSh.GetFrmType(0,TRUE) & FRMTYPE_FOOTNOTE) )
                rSet.DisableItem( nWhich );
            break;

        case FN_CALCULATE:
            if ( !rSh.IsSelection() )
                rSet.DisableItem(nWhich);
            break;
        case FN_GOTO_REFERENCE:
            {
                SwField *pFld = rSh.GetCurFld();
                if ( !pFld || (pFld &&  pFld->GetTypeId() != TYP_GETREFFLD) )
                    rSet.DisableItem(nWhich);
            }
            break;
        case FN_AUTOFORMAT_AUTO:
            {
                rSet.Put( SfxBoolItem( nWhich, OFF_APP()->
                                GetAutoCorrConfig()->IsAutoFmtByInput() ));
            }
            break;
        case FN_GLOSSARY_DLG:
            {
                rSet.Put(SfxBoolItem(nWhich), TRUE);
            }
            break;

        case SID_DEC_INDENT:
        case SID_INC_INDENT:
            {
                USHORT nHtmlMode = ::GetHtmlMode(GetView().GetDocShell());
                nHtmlMode &= HTMLMODE_ON|HTMLMODE_SOME_STYLES;
                if( (nHtmlMode == HTMLMODE_ON) || !rSh.IsMoveLeftMargin(
                                        SID_INC_INDENT == nWhich, TRUE ))
                    rSet.DisableItem( nWhich );
            }
            break;

        case FN_DEC_INDENT_OFFSET:
        case FN_INC_INDENT_OFFSET:
            {
                USHORT nHtmlMode = ::GetHtmlMode(GetView().GetDocShell());
                nHtmlMode &= HTMLMODE_ON|HTMLMODE_SOME_STYLES;
                if( (nHtmlMode == HTMLMODE_ON) ||
                    !rSh.IsMoveLeftMargin( FN_INC_INDENT_OFFSET == nWhich,
                                            FALSE ))
                    rSet.DisableItem( nWhich );
            }
            break;

        case SID_ATTR_CHAR_COLOR2:
            {
                rSet.Put(SvxColorItem(GetView().GetEditWin().GetTextColor(), SID_ATTR_CHAR_COLOR2));
            }
            break;
        case SID_ATTR_CHAR_COLOR_BACKGROUND:
            {
                if(GetView().GetEditWin().IsTextBackColorTransparent())
                    rSet.Put(SvxColorItem(Color(COL_LIGHTGRAY), SID_ATTR_CHAR_COLOR_BACKGROUND));
                else
                    rSet.Put(SvxColorItem(GetView().GetEditWin().GetTextBackColor(), SID_ATTR_CHAR_COLOR_BACKGROUND));
            }
            break;
        case SID_ATTR_CHAR_COLOR_BACKGROUND_EXT:
        case SID_ATTR_CHAR_COLOR_EXT:
            {
                SwEditWin& rEdtWin = GetView().GetEditWin();
                SwApplyTemplate* pApply = rEdtWin.GetApplyTemplate();
                rSet.Put(SfxBoolItem(nWhich, pApply && pApply->nColor == nWhich));
            }
            break;
        case FN_INSERT_BOOKMARK:
            if( rSh.IsTableMode() )
                rSet.DisableItem( nWhich );
            break;

        case FN_INSERT_PAGEHEADER:
        case FN_INSERT_PAGEFOOTER:
            {
#ifndef CHECK_MENU
                rSet.Put( SfxObjectItem( nWhich, GetView().GetDocShell() ));
#else
                // Seitenvorlagen besorgen
                BOOL bFound = FALSE;
                USHORT n, nCnt = rSh.GetPageDescCnt();
                for( n = 0; n < nCnt; ++n )
                {
                    const SwPageDesc& rDesc = rSh.GetPageDesc( n );
                    if( FN_INSERT_PAGEHEADER == nWhich
                        ? !rDesc.GetMaster().GetHeader().IsActive()
                        : !rDesc.GetMaster().GetFooter().IsActive() )
                    {
                        bFound = TRUE;
                        break;
                    }
                }

                if( bFound )
                    rSet.Put( SfxObjectItem( nWhich, GetView().GetDocShell() ));
                else
                    rSet.DisableItem( nWhich );
#endif
            }
            break;
            case FN_SORTING_DLG:
                if(!rSh.HasSelection())
                    rSet.DisableItem( nWhich );
            break;
            case SID_HYPERLINK_DIALOG:
                if(!GetView().GetViewFrame()->HasChildWindow(SID_HYPERLINK_DIALOG)  && rSh.HasReadonlySel())
                    rSet.DisableItem(SID_HYPERLINK_DIALOG);
            break;
            case FN_EDIT_HYPERLINK:
            {
                SfxItemSet aSet(GetPool(),
                                RES_TXTATR_INETFMT,
                                RES_TXTATR_INETFMT);
                rSh.GetAttr(aSet);
                if(SFX_ITEM_SET > aSet.GetItemState( RES_TXTATR_INETFMT, TRUE ) || rSh.HasReadonlySel())
                {
                    rSet.DisableItem(FN_EDIT_HYPERLINK);
                }
            }
            break;
        }
        nWhich = aIter.NextWhich();
    }
}


/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.1.1.1  2000/09/18 17:14:47  hr
    initial import

    Revision 1.171  2000/09/18 16:06:06  willem.vandorp
    OpenOffice header added.

    Revision 1.170  2000/09/12 14:11:19  os
    SfxApplication::ChildWindowExecute removed

    Revision 1.169  2000/09/11 06:52:45  os
    Get/Set/Has/ToggleChildWindow SfxApplication -> SfxViewFrame

    Revision 1.168  2000/09/08 08:12:52  os
    Change: Set/Toggle/Has/Knows/Show/GetChildWindow

    Revision 1.167  2000/09/07 15:59:30  os
    change: SFX_DISPATCHER/SFX_BINDINGS removed

    Revision 1.166  2000/08/31 11:32:07  jp
    add missing include

    Revision 1.165  2000/05/26 07:21:33  os
    old SW Basic API Slots removed

    Revision 1.164  2000/05/10 11:53:02  os
    Basic API removed

    Revision 1.163  2000/04/18 14:58:24  os
    UNICODE

    Revision 1.162  2000/04/04 15:11:48  os
    #74685# hyperlink button only disabled in readonly sections

    Revision 1.161  2000/03/03 15:17:03  os
    StarView remainders removed

    Revision 1.160  2000/02/11 14:57:58  hr
    #70473# changes for unicode ( patched by automated patchtool )

    Revision 1.159  2000/01/12 16:43:04  os
    #71278# edit hyperlink: use the new dialog

    Revision 1.158  2000/01/03 08:38:28  os
    #71278# edit hyperlink slot

    Revision 1.157  1999/07/08 13:58:46  MA
    Use internal object to toggle wait cursor


      Rev 1.156   08 Jul 1999 15:58:46   MA
   Use internal object to toggle wait cursor

      Rev 1.155   15 Apr 1999 14:02:44   OS
   #64801# keine Endnoten in Rahmen

      Rev 1.154   23 Mar 1999 18:28:12   JP
   Bug #63449#: vorm Formeledit alle Zellveraenderungen pruefen

      Rev 1.153   18 Mar 1999 14:41:18   OS
   #61169# #61489# Masseinheiten fuer Text u. HTML am Module setzen, nicht an der App

      Rev 1.152   05 Feb 1999 17:05:48   OS
   #61547# InsertEndnoteDirect

      Rev 1.151   15 Jan 1999 15:45:30   JP
   Bug #60203#: MoveLeftMargin - optional um Betrag verschieben

      Rev 1.150   13 Jan 1999 14:54:02   TJ
   include

      Rev 1.149   13 Jan 1999 08:14:18   OS
   #60380# HelpIds fuer AutoFormat - QueryBox

      Rev 1.148   27 Nov 1998 14:55:38   AMA
   Fix #59951#59825#: Unterscheiden zwischen Rahmen-,Seiten- und Bereichsspalten

      Rev 1.147   06 Nov 1998 16:34:50   OS
   #58450# Fussnoten auch im HTML

      Rev 1.146   21 Oct 1998 10:56:54   OM
   #57586# Redlining nach Autoformat

      Rev 1.145   20 Oct 1998 17:52:46   OM
   #57586# Redlining nach Autoformat

      Rev 1.144   20 Oct 1998 11:39:34   OM
   #57586# Redlining nach Autoformat

      Rev 1.143   30 Sep 1998 14:05:26   OS
   #52654# C40_Insert

      Rev 1.142   30 Sep 1998 10:28:56   OS
   #52654# Sortable und SortDescriptor eingebaut, auf- und absteigend fuer jeden Schluessel

      Rev 1.141   08 Sep 1998 17:03:02   OS
   #56134# Metric fuer Text und HTML getrennt

      Rev 1.140   09 Jul 1998 14:04:12   OS
   Kapitelnumerierung jetzt TabDialog

      Rev 1.139   26 May 1998 15:34:00   JP
   neu: Kopf-/Fusszeilen per Menu ein-/ausschalten

      Rev 1.138   13 May 1998 14:58:10   OM
   Autokorrektur/Autoformat umgestaltet und zusammengefasst

      Rev 1.137   08 May 1998 17:02:48   OM
   AutoFormat-Optionen an Ofa und Svx verschoben

      Rev 1.136   07 May 1998 12:58:40   OM
   AutoFormat-Optionen Umstellung

      Rev 1.135   12 Mar 1998 13:08:00   OS
   Numerierungsbutton nicht mehr gecheckt und nicht disabled

      Rev 1.134   04 Mar 1998 14:16:26   OM
   Dokument Merge/Compare

      Rev 1.133   05 Feb 1998 14:46:26   OS
   Numerierungsstart nur bei gesetzer Numerierung putten

      Rev 1.132   02 Feb 1998 09:12:52   OS
   neue Num-Tabpage

      Rev 1.131   12 Dec 1997 13:57:36   OS
   neuer Bookmark-Typ

      Rev 1.130   03 Dec 1997 17:02:34   OS
   Numerierungsumbau

------------------------------------------------------------------------*/


