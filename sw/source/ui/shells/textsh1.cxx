/*************************************************************************
 *
 *  $RCSfile: textsh1.cxx,v $
 *
 *  $Revision: 1.32 $
 *
 *  last change: $Author: vg $ $Date: 2003-07-21 11:23:04 $
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

#ifndef _COM_SUN_STAR_UI_DIALOGS_XEXECUTABLEDIALOG_HPP_
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
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
#ifndef _SVTOOLS_CJKOPTIONS_HXX
#include <svtools/cjkoptions.hxx>
#endif

#ifndef _CHARFMT_HXX
#include <charfmt.hxx>
#endif
#ifndef _SVX_FONTITEM_HXX
#include <svx/fontitem.hxx>
#endif
#ifndef _SVX_DIALMGR_HXX
#include <svx/dialmgr.hxx>
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
#ifndef _FMTHDFT_HXX //autogen
#include <fmthdft.hxx>
#endif
#ifndef _PAGEDESC_HXX
#include <pagedesc.hxx>
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
#ifndef _SWDTFLVR_HXX
#include <swdtflvr.hxx>
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
#include "fldmgr.hxx"

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
#ifndef _PARATR_HXX
#include "paratr.hxx"
#endif
#ifndef _CRSSKIP_HXX
#include <crsskip.hxx>
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
    BOOL bUseDialog = TRUE;
    const SfxItemSet *pArgs = rReq.GetArgs();
    SwWrtShell& rWrtSh = GetShell();
    const SfxPoolItem* pItem = 0;
    USHORT nSlot = rReq.GetSlot();
    if(pArgs)
        pArgs->GetItemState(GetPool().GetWhich(nSlot), FALSE, &pItem);
    switch( nSlot )
    {
        case FN_INSERT_SYMBOL:
        {
            InsertSymbol( rReq );
        }
        break;
        case FN_INSERT_FOOTNOTE:
        case FN_INSERT_ENDNOTE:
        {
            String aStr;
            SFX_REQUEST_ARG( rReq, pFont, SfxStringItem, FN_PARAM_1 , sal_False );
            SFX_REQUEST_ARG( rReq, pCharset, SfxInt16Item, FN_PARAM_2 , sal_False );
            SFX_REQUEST_ARG( rReq, pItem, SfxStringItem, nSlot , sal_False );
            if ( pItem )
                aStr = pItem->GetValue();
            BOOL bFont = pFont && pFont->GetValue().Len();
            rWrtSh.StartUndo( UIUNDO_INSERT_FOOTNOTE );
            rWrtSh.InsertFootnote( aStr, nSlot == FN_INSERT_ENDNOTE, !bFont );
            if ( bFont )
            {
                rWrtSh.Left( CRSR_SKIP_CHARS, TRUE, 1, FALSE );
                SfxItemSet aSet( rWrtSh.GetAttrPool(), RES_CHRATR_FONT, RES_CHRATR_FONT );
                rWrtSh.GetAttr( aSet );
                SvxFontItem &rFont = (SvxFontItem &) aSet.Get( RES_CHRATR_FONT );
                SvxFontItem aFont( rFont.GetFamily(), pFont->GetValue(),
                                    rFont.GetStyleName(), rFont.GetPitch() );
                                    //pCharset ? (CharSet) pCharset->GetValue() : RTL_TEXTENCODING_DONTKNOW );
                rWrtSh.SetAttr( aSet, SETATTR_DONTEXPAND );
                rWrtSh.ResetSelect(0, FALSE);
                rWrtSh.EndSelect();
                rWrtSh.GotoFtnTxt();
            }
            rWrtSh.EndUndo( UIUNDO_INSERT_FOOTNOTE );
            rReq.Done();
        }
        break;
        case FN_INSERT_FOOTNOTE_DLG:
        {
            SwInsFootNoteDlg *pDlg = new SwInsFootNoteDlg( GetView().GetWindow(), rWrtSh, FALSE );
            pDlg->SetHelpId(nSlot);
            if ( pDlg->Execute() == RET_OK )
            {
                USHORT nId = pDlg->IsEndNote() ? FN_INSERT_ENDNOTE : FN_INSERT_FOOTNOTE;
                SfxRequest aReq( GetView().GetViewFrame(), nId );
                if ( pDlg->GetStr().Len() )
                    aReq.AppendItem( SfxStringItem( nId, pDlg->GetStr() ) );
                if ( pDlg->GetFontName().Len() )
                    aReq.AppendItem( SfxStringItem( FN_PARAM_1, pDlg->GetFontName() ) );
                //aReq.AppendItem( SfxStringItem( FN_PARAM_2, pDlg->GetCharSet() ) );
                ExecuteSlot( aReq );
            }

            rReq.Ignore();
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
            //if (!pItem)
            {
                rReq.SetReturnValue(SfxBoolItem(nSlot, InsertGraphicDlg( rReq )));
            }
/*
            else
            {
                String rName = aEmptyStr;
                BOOL bLink = FALSE;
                rName = ((const SfxStringItem *)pItem)->GetValue();
                String rFilter = aEmptyStr;

                if ( SFX_ITEM_SET == pArgs->GetItemState(FN_PARAM_FILTER, TRUE, &pItem) )
                    rFilter = ((const SfxStringItem *)pItem)->GetValue();
                if ( SFX_ITEM_SET == pArgs->GetItemState(FN_PARAM_1, TRUE, &pItem) )
                    bLink = ((const SfxBoolItem *)pItem)->GetValue();

                if ( !rName.Len() )
                    rReq.SetReturnValue(SfxBoolItem(nSlot, InsertGraphicDlg()));
                else
                    rReq.SetReturnValue(SfxBoolItem(nSlot, InsertGraphic( rName, rFilter, bLink )));
            }
*/
            break;
        case SID_INSERTDOC:
            if (!pItem)
            {
                rReq.SetReturnValue(SfxBoolItem(nSlot, GetView().InsertDoc(nSlot, aEmptyStr, aEmptyStr) != -1));
                rReq.Ignore();
            }
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
            USHORT nKind=0, nPageNumber=0;
            String aTemplateName;
            if ( pItem )
            {
                nKind = ((SfxInt16Item*)pItem)->GetValue();
                SFX_REQUEST_ARG( rReq, pTemplate, SfxStringItem, FN_PARAM_1 , sal_False );
                SFX_REQUEST_ARG( rReq, pNumber, SfxUInt16Item, FN_PARAM_2 , sal_False );
                if ( pTemplate )
                    aTemplateName = pTemplate->GetValue();
                if ( pNumber )
                    nPageNumber = pNumber->GetValue();
            }
            else
            {
                SwBreakDlg *pDlg = new SwBreakDlg(GetView().GetWindow(), rWrtSh);
                if ( pDlg->Execute() == RET_OK )
                {
                    nKind = pDlg->GetKind();
                    aTemplateName = pDlg->GetTemplateName();
                    nPageNumber = pDlg->GetPageNumber();
                    rReq.AppendItem( SfxInt16Item( FN_INSERT_BREAK_DLG, nKind ) );
                    rReq.AppendItem( SfxUInt16Item( FN_PARAM_2, nPageNumber ) );
                    rReq.AppendItem( SfxStringItem( FN_PARAM_1, aTemplateName ) );
                    rReq.Done();
                }
                else
                    rReq.Ignore();
                delete pDlg;
            }

            switch ( nKind )
            {
                case 1 :
                    rWrtSh.InsertLineBreak(); break;
                case 2 :
                    rWrtSh.InsertColumnBreak(); break;
                case 3 :
                {
                    rWrtSh.StartAllAction();
                    if( aTemplateName.Len() )
                        rWrtSh.InsertPageBreak( &aTemplateName, nPageNumber );
                    else
                        rWrtSh.InsertPageBreak();
                    rWrtSh.EndAllAction();
                }
            }

            break;
        }
        case FN_INSERT_BOOKMARK:
        {
            if ( pItem )
            {
                String sName = ((SfxStringItem*)pItem)->GetValue();
                rWrtSh.MakeUniqueBookmarkName(sName);
                rWrtSh.SetBookmark( KeyCode(), sName, aEmptyStr );
            }
            else
            {
                SwInsertBookmarkDlg *pDlg = new SwInsertBookmarkDlg( GetView().GetWindow(), rWrtSh, rReq );
                pDlg->Execute();
                delete pDlg;
            }

            break;
        }
        case FN_DELETE_BOOKMARK:
        {
            if ( pItem )
                rWrtSh.DelBookmark( ((SfxStringItem*)pItem)->GetValue() );
            break;
        }
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
                {
                    aDlg.AcceptAll(TRUE);
                    SfxRequest aReq( pVFrame, FN_AUTOFORMAT_APPLY );
                    aReq.Done();
                    rReq.Ignore();
                    break;
                }

                case RET_CANCEL:
                    aDlg.AcceptAll(FALSE);
                    rReq.Ignore();
                    break;

                case 2:
                    aDlg.Execute();
                    rReq.Done();
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
            rReq.Done();
        }
        break;
        case FN_AUTOFORMAT_AUTO:
        {
            OfaAutoCorrCfg* pACfg = OFF_APP()->GetAutoCorrConfig();
            BOOL bSet = pItem ? ((const SfxBoolItem*)pItem)->GetValue() : !pACfg->IsAutoFmtByInput();
            if( bSet != pACfg->IsAutoFmtByInput() )
            {
                pACfg->SetAutoFmtByInput( bSet );
                GetView().GetViewFrame()->GetBindings().Invalidate( nSlot );
                if ( !pItem )
                    rReq.AppendItem( SfxBoolItem( GetPool().GetWhich(nSlot), bSet ) );
                rReq.Done();
            }
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
                SwTransferable* pTransfer = new SwTransferable( rWrtSh );
/*??*/          ::com::sun::star::uno::Reference<
                    ::com::sun::star::datatransfer::XTransferable > xRef(
                                                    pTransfer );
                pTransfer->CalculateAndCopy();
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
            const USHORT nId = SwInputChild::GetChildWindowId();
            SfxViewFrame* pVFrame = GetView().GetViewFrame();
            if(pItem)
            {
                //if the ChildWindow is active it has to be removed
                if( pVFrame->HasChildWindow( nId ) )
                {
                    pVFrame->ToggleChildWindow( nId );
                    pVFrame->GetBindings().InvalidateAll( TRUE );
                }

                String sFormula(((const SfxStringItem*)pItem)->GetValue());
                SwFldMgr aFldMgr;
                rWrtSh.StartAllAction();
                BOOL bDelSel;
                if( 0 != (bDelSel = rWrtSh.HasSelection()) )
                {
                    rWrtSh.StartUndo( UNDO_START );
                    rWrtSh.DelRight();
                }
                else
                {
                    rWrtSh.EnterStdMode();
                }

                if( !bDelSel && aFldMgr.GetCurFld() && TYP_FORMELFLD == aFldMgr.GetCurTypeId() )
                    aFldMgr.UpdateCurFld( aFldMgr.GetCurFld()->GetFormat(), aEmptyStr, sFormula );
                else if( sFormula.Len() )
                {
                    if( rWrtSh.IsCrsrInTbl() )
                    {
                        SfxItemSet aSet( rWrtSh.GetAttrPool(), RES_BOXATR_FORMULA, RES_BOXATR_FORMULA );
                        aSet.Put( SwTblBoxFormula( sFormula ));
                        rWrtSh.SetTblBoxFormulaAttrs( aSet );
                        rWrtSh.UpdateTable();
                    }
                    else
                    {
                        SvNumberFormatter* pFormatter = rWrtSh.GetNumberFormatter();
                        ULONG nSysNumFmt = pFormatter->GetFormatIndex( NF_NUMBER_STANDARD, LANGUAGE_SYSTEM);
                        SwInsertFld_Data aData(TYP_FORMELFLD, GSE_FORMULA, aEmptyStr, sFormula, nSysNumFmt);
                        aFldMgr.InsertFld(aData);
                    }
                }

                if( bDelSel )
                    rWrtSh.EndUndo( UNDO_END );
                rWrtSh.EndAllAction();
                rReq.Done();
            }
            else
            {
                rWrtSh.EndAllTblBoxEdit();
                pVFrame->ToggleChildWindow( nId );
                if( !pVFrame->HasChildWindow( nId ) )
                    pVFrame->GetBindings().InvalidateAll( TRUE );
                rReq.Ignore();
            }
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
        case SID_ATTR_BRUSH_CHAR :
        case SID_ATTR_CHAR_SCALEWIDTH :
        case SID_ATTR_CHAR_ROTATED :
        case FN_TXTATR_INET :
        case FN_INSERT_HYPERLINK:
        {
            USHORT nWhich = GetPool().GetWhich( nSlot );
            if ( pArgs && pArgs->GetItemState( nWhich ) == SFX_ITEM_SET )
                bUseDialog = FALSE;
            // intentionally no break
        }
        case SID_CHAR_DLG:
        {
            FieldUnit eMetric = ::GetDfltMetric(0 != PTR_CAST(SwWebView, &GetView()));
            SW_MOD()->PutItem(SfxUInt16Item(SID_ATTR_METRIC, eMetric));
            SfxItemSet aCoreSet( GetPool(),
                                RES_CHRATR_BEGIN,      RES_CHRATR_END-1,
                                RES_TXTATR_INETFMT,    RES_TXTATR_INETFMT,
                                RES_BACKGROUND,        RES_BACKGROUND,
                                FN_PARAM_SELECTION,    FN_PARAM_SELECTION,
                                SID_HTML_MODE,         SID_HTML_MODE,
                                SID_ATTR_CHAR_WIDTH_FIT_TO_LINE,   SID_ATTR_CHAR_WIDTH_FIT_TO_LINE,
                                0 );
            rWrtSh.GetAttr( aCoreSet );
            BOOL bSel = rWrtSh.HasSelection();
            BOOL bSelectionPut = FALSE;
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
                bSelectionPut = TRUE;
                if(!bSel)
                {
                    rWrtSh.Pop(FALSE);
                    rWrtSh.EndAction();
                }
            }

            aCoreSet.Put( SfxUInt16Item( SID_ATTR_CHAR_WIDTH_FIT_TO_LINE,
                            rWrtSh.GetScalingOfSelectedText() ) );

            // Das CHRATR_BACKGROUND-Attribut wird fuer den Dialog in
            // ein RES_BACKGROUND verwandelt und wieder zurueck ...
            const SfxPoolItem *pTmpBrush;
            if( SFX_ITEM_SET == aCoreSet.GetItemState( RES_CHRATR_BACKGROUND, TRUE, &pTmpBrush ) )
            {
                SvxBrushItem aTmpBrush( *((SvxBrushItem*)pTmpBrush) );
                aTmpBrush.SetWhich( RES_BACKGROUND );
                aCoreSet.Put( aTmpBrush );
            }

            aCoreSet.Put(SfxUInt16Item(SID_HTML_MODE, ::GetHtmlMode(GetView().GetDocShell())));
            SwCharDlg* pDlg = NULL;
            if ( bUseDialog && GetActiveView() )
            {
                pDlg = new SwCharDlg(GetView().GetWindow(), GetView(), aCoreSet);
                if( FN_INSERT_HYPERLINK == nSlot )
                    pDlg->SetCurPageId(TP_CHAR_URL);
            }

            const SfxItemSet* pSet = NULL;
            if ( !bUseDialog )
                pSet = pArgs;
            else if ( NULL != pDlg && pDlg->Execute() == RET_OK ) /* #110771# pDlg can be NULL */
            {
                pSet = pDlg->GetOutputItemSet();
            }

            if ( pSet)
            {
                SfxItemSet aTmpSet( *pSet );
                if( SFX_ITEM_SET == aTmpSet.GetItemState( RES_BACKGROUND, FALSE, &pTmpBrush ) )
                {
                    SvxBrushItem aTmpBrush( *((SvxBrushItem*)pTmpBrush) );
                    aTmpBrush.SetWhich( RES_CHRATR_BACKGROUND );
                    aTmpSet.Put( aTmpBrush );
                }

                aTmpSet.ClearItem( RES_BACKGROUND );

                const SfxPoolItem* pItem;
                BOOL bInsert = FALSE;
                xub_StrLen nInsert = 0;

                // aus ungeklaerter Ursache ist das alte Item wieder im Set
                if( !bSelectionPut && SFX_ITEM_SET == aTmpSet.GetItemState(FN_PARAM_SELECTION, FALSE, &pItem) )
                {
                    String sInsert = ((const SfxStringItem*)pItem)->GetValue();
                    bInsert = sInsert.Len() != 0;
                    if(bInsert)
                    {
                        nInsert = sInsert.Len();
                        rWrtSh.StartAction();
                        rWrtSh.Insert( sInsert );
                        rWrtSh.SetMark();
                        rWrtSh.ExtendSelection(FALSE, sInsert.Len());
                        SfxRequest aReq( GetView().GetViewFrame(), FN_INSERT_STRING );
                        aReq.AppendItem( SfxStringItem( FN_INSERT_STRING, sInsert ) );
                        aReq.Done();
                        SfxRequest aReq1( GetView().GetViewFrame(), FN_CHAR_LEFT );
                        aReq1.AppendItem( SfxInt16Item(FN_PARAM_MOVE_COUNT, nInsert) );
                        aReq1.AppendItem( SfxBoolItem(FN_PARAM_MOVE_SELECTION, TRUE) );
                        aReq1.Done();
                    }
                }
                aTmpSet.ClearItem(FN_PARAM_SELECTION);

                SwTxtFmtColl* pColl = rWrtSh.GetCurTxtFmtColl();
                if(bSel && rWrtSh.IsSelFullPara() && pColl && pColl->IsAutoUpdateFmt())
                {
                    rWrtSh.AutoUpdatePara(pColl, aTmpSet);
                }
                else
                    rWrtSh.SetAttr( aTmpSet );
                rReq.Done(aTmpSet);
                if(bInsert)
                {
                    SfxRequest aReq1( GetView().GetViewFrame(), FN_CHAR_RIGHT );
                    aReq1.AppendItem( SfxInt16Item(FN_PARAM_MOVE_COUNT, nInsert) );
                    aReq1.AppendItem( SfxBoolItem(FN_PARAM_MOVE_SELECTION, FALSE) );
                    aReq1.Done();
                    rWrtSh.SwapPam();
                    rWrtSh.ClearMark();
                    rWrtSh.DontExpandFmt();
                    rWrtSh.EndAction();
                }
            }

            delete pDlg;
        }
        break;
        case SID_ATTR_LRSPACE :
        case SID_ATTR_ULSPACE :
        case SID_ATTR_BRUSH :
        case SID_PARA_VERTALIGN :
        case SID_ATTR_PARA_NUMRULE :
        case SID_ATTR_PARA_REGISTER :
        case SID_ATTR_PARA_PAGENUM :
        case FN_FORMAT_LINENUMBER :
        case FN_NUMBER_NEWSTART :
        case FN_NUMBER_NEWSTART_AT :
        case FN_FORMAT_DROPCAPS :
        case FN_DROP_TEXT:
        {
            USHORT nWhich = GetPool().GetWhich( nSlot );
            if ( pArgs && pArgs->GetItemState( nWhich ) == SFX_ITEM_SET )
                bUseDialog = FALSE;
            // intentionally no break

        }
        case SID_PARA_DLG:
        {
            FieldUnit eMetric = ::GetDfltMetric(0 != PTR_CAST(SwWebView, &GetView()));
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
                            FN_DROP_TEXT,               FN_DROP_CHAR_STYLE_NAME,
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
                SfxBoolItem aStart(FN_NUMBER_NEWSTART,
                                    rWrtSh.IsNumRuleStart());
                aCoreSet.Put(aStart);
                SfxUInt16Item aStartAt(FN_NUMBER_NEWSTART_AT,
                                                rWrtSh.IsNodeNumStart());
                aCoreSet.Put(aStartAt);
            }
            SwParaDlg *pDlg = NULL;
            if ( bUseDialog && GetActiveView() )
                pDlg = new SwParaDlg( GetView().GetWindow(),
                                                GetView(), aCoreSet, DLG_STD,
                                                NULL, FALSE, nDefPage );

            SfxItemSet* pSet = NULL;
            if ( !bUseDialog )
            {
                pSet = (SfxItemSet*) pArgs;

            }
            else if ( pDlg->Execute() == RET_OK )
            {
                // Defaults evtl umsetzen
                pSet = (SfxItemSet*)pDlg->GetOutputItemSet();
                USHORT nNewDist;
                if( SFX_ITEM_SET == pSet->GetItemState( SID_ATTR_TABSTOP_DEFAULTS, FALSE, &pItem ) &&
                    nDefDist != (nNewDist = ((SfxUInt16Item*)pItem)->GetValue()) )
                {
                    SvxTabStopItem aDefTabs( 0, 0 );
                    MakeDefTabs( nNewDist, aDefTabs );
                    rWrtSh.SetDefault( aDefTabs );
                    pSet->ClearItem( SID_ATTR_TABSTOP_DEFAULTS );
                }

                if ( SFX_ITEM_SET == pSet->GetItemState(FN_PARAM_1,FALSE,&pItem) )
                {
                    pSet->Put(SfxStringItem(FN_DROP_TEXT, ((const SfxStringItem*)pItem)->GetValue()));
                    pSet->ClearItem(FN_PARAM_1);
                }

                if( SFX_ITEM_SET == pSet->GetItemState( RES_PARATR_DROP, FALSE, &pItem ))
                {
                    String sCharStyleName;
                    if(((const SwFmtDrop*)pItem)->GetCharFmt())
                        sCharStyleName = ((const SwFmtDrop*)pItem)->GetCharFmt()->GetName();
                    pSet->Put(SfxStringItem(FN_DROP_CHAR_STYLE_NAME, sCharStyleName));
                }
            }

            if ( pSet )
            {
                rReq.Done( *pSet );
                ::SfxToSwPageDescAttr( rWrtSh, *pSet );
                if( pSet->Count() )
                {
                    rWrtSh.StartAction();
                    rWrtSh.StartUndo( UNDO_START );
                    if ( SFX_ITEM_SET == pSet->GetItemState(FN_DROP_TEXT, FALSE, &pItem) )
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

                if( SFX_ITEM_SET == pSet->GetItemState(FN_NUMBER_NEWSTART) )
                {
                    //SetNumRuleStart(TRUE) restarts the numbering at the value
                    //that is defined at the starting point of the numbering level
                    //otherwise the SetNodeNumStart() value determines the start
                    //if it's set to something different than USHRT_MAX

                    BOOL bStart = ((SfxBoolItem&)pSet->Get(FN_NUMBER_NEWSTART)).GetValue();
                    USHORT nNumStart = USHRT_MAX;
                    if( SFX_ITEM_SET == pSet->GetItemState(FN_NUMBER_NEWSTART_AT) )
                    {
                        nNumStart = ((SfxUInt16Item&)pSet->Get(FN_NUMBER_NEWSTART_AT)).GetValue();
                        if(USHRT_MAX != nNumStart)
                            bStart = FALSE;
                    }
                    rWrtSh.SetNumRuleStart(bStart);
                    rWrtSh.SetNodeNumStart(nNumStart);
                }
                else if( SFX_ITEM_SET == pSet->GetItemState(FN_NUMBER_NEWSTART_AT) )
                {
                    USHORT nNumStart = ((SfxUInt16Item&)pSet->Get(FN_NUMBER_NEWSTART_AT)).GetValue();
                    rWrtSh.SetNodeNumStart(nNumStart);
                    rWrtSh.SetNumRuleStart(FALSE);
                }
            }

            delete pDlg;
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

                rReq.Done();
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

            rReq.Done();

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
        case SID_RUBY_DIALOG:
        case SID_HYPERLINK_DIALOG:
        {
            SfxRequest aReq(nSlot, SFX_CALLMODE_SLOT, SFX_APP()->GetPool());
            GetView().GetViewFrame()->ExecuteSlot( aReq);
            rReq.Ignore();
        }
        break;
    case FN_INSERT_PAGEHEADER:
    case FN_INSERT_PAGEFOOTER:
    if(pItem)
    {
        String sStyleName = ((const SfxStringItem*)pItem)->GetValue();
        BOOL bOn = TRUE;
        if( SFX_ITEM_SET == pArgs->GetItemState(FN_PARAM_1, FALSE, &pItem))
            bOn = ((const SfxBoolItem*)pItem)->GetValue();
        ChangeHeaderOrFooter(sStyleName, FN_INSERT_PAGEHEADER == nSlot, bOn, !rReq.IsAPI());
        rReq.Done();
    }
    break;
    case FN_READONLY_SELECTION_MODE :
        if(GetView().GetDocShell()->IsReadOnly())
        {
            rWrtSh.SetReadonlySelectionOption(
                !rWrtSh.GetViewOptions()->IsSelectionInReadonly());
            rWrtSh.ShowCrsr();
        }
    break;
    case SID_OPEN_HYPERLINK:
    {
        SfxItemSet aSet(GetPool(),
                        RES_TXTATR_INETFMT,
                        RES_TXTATR_INETFMT);
        rWrtSh.GetAttr(aSet);
        if(SFX_ITEM_SET <= aSet.GetItemState( RES_TXTATR_INETFMT, TRUE ))
        {
            const SfxPoolItem& rItem = aSet.Get(RES_TXTATR_INETFMT, TRUE);
            rWrtSh.ClickToINetAttr((const SwFmtINetFmt&)rItem, URLLOAD_NOFILTER);
        }
    }

    break;

    case SID_OPEN_XML_FILTERSETTINGS:
    {
        try
        {
            com::sun::star::uno::Reference < ::com::sun::star::ui::dialogs::XExecutableDialog > xDialog(::comphelper::getProcessServiceFactory()->createInstance(rtl::OUString::createFromAscii("com.sun.star.comp.ui.XSLTFilterDialog")), com::sun::star::uno::UNO_QUERY);
            if( xDialog.is() )
            {
                xDialog->execute();
            }
        }
        catch( ::com::sun::star::uno::Exception& )
        {
        }
        rReq.Ignore ();
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

        case FN_NUMBER_NEWSTART :
            rSet.Put(SfxBoolItem(FN_NUMBER_NEWSTART,
                rSh.IsNumRuleStart()||USHRT_MAX != rSh.IsNodeNumStart()));
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
                  rSet.Put(SvxColorItem(Color(COL_TRANSPARENT), SID_ATTR_CHAR_COLOR_BACKGROUND));
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
            case SID_RUBY_DIALOG:
            {
                SvtCJKOptions aCJKOptions;
                if(!aCJKOptions.IsRubyEnabled())
                {
                    rSet.DisableItem(nWhich);
                    break;
                }
            }
            //no break!
            case SID_HYPERLINK_DIALOG:
                if( GetView().GetDocShell()->IsReadOnly() ||
                    (!GetView().GetViewFrame()->HasChildWindow(nWhich) &&
                     rSh.HasReadonlySel()) )
                    rSet.DisableItem(nWhich);
                else
                    rSet.Put(SfxBoolItem( nWhich, 0 != GetView().
                                GetViewFrame()->GetChildWindow( nWhich ) ));
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
            case SID_TRANSLITERATE_HALFWIDTH:
            case SID_TRANSLITERATE_FULLWIDTH:
            case SID_TRANSLITERATE_HIRAGANA:
            case SID_TRANSLITERATE_KATAGANA:
            {
                SvtCJKOptions aCJKOptions;
                if(!aCJKOptions.IsChangeCaseMapEnabled())
                    rSet.DisableItem(nWhich);
            }
            break;
            case FN_READONLY_SELECTION_MODE :
                if(!GetView().GetDocShell()->IsReadOnly())
                    rSet.DisableItem( nWhich );
                else
                {
                    rSet.Put(SfxBoolItem(nWhich, rSh.GetViewOptions()->IsSelectionInReadonly()));
                }
            break;
            case  SID_OPEN_HYPERLINK:
            {
                SfxItemSet aSet(GetPool(),
                                RES_TXTATR_INETFMT,
                                RES_TXTATR_INETFMT);
                rSh.GetAttr(aSet);
                if(SFX_ITEM_SET > aSet.GetItemState( RES_TXTATR_INETFMT, FALSE ))
                    rSet.DisableItem(nWhich);
            }
            break;
        }
        nWhich = aIter.NextWhich();
    }
}
/* -----------------------------2002/07/05 10:31------------------------------
    Switch on/off header of footer of a page style - if an empty name is
    given all styles are changed
 ---------------------------------------------------------------------------*/
void SwTextShell::ChangeHeaderOrFooter(
    const String& rStyleName, BOOL bHeader, BOOL bOn, BOOL bShowWarning)
{
    SwWrtShell& rSh = GetShell();
    rSh.StartAllAction();
    rSh.StartUndo( 0 );
    BOOL bExecute = TRUE;
    BOOL bCrsrSet = FALSE;
    for( USHORT nFrom = 0, nTo = rSh.GetPageDescCnt();
            nFrom < nTo; ++nFrom )
    {
        int bChgd = FALSE;
        SwPageDesc aDesc( rSh.GetPageDesc( nFrom ));
        String sTmp(aDesc.GetName());
        if( !rStyleName.Len() || rStyleName == sTmp )
        {
            if( bShowWarning && !bOn && GetActiveView() && GetActiveView() == &GetView() &&
                    (bHeader && aDesc.GetMaster().GetHeader().IsActive() ||
                        !bHeader && aDesc.GetMaster().GetFooter().IsActive()))
            {
                bShowWarning = FALSE;
                //Actions have to be closed while the dialog is showing
                rSh.EndAllAction();

                Window* pParent = &GetView().GetViewFrame()->GetWindow();
                BOOL bRet = RET_YES == QueryBox( pParent, ResId( RID_SVXQBX_DELETE_HEADFOOT,
                                        DIALOG_MGR() ) ).Execute();
                bExecute = bRet;
                rSh.StartAllAction();
            }
            if( bExecute )
            {
                bChgd = TRUE;
                SwFrmFmt &rMaster = aDesc.GetMaster();
                if(bHeader)
                    rMaster.SetAttr( SwFmtHeader( bOn ));
                else
                    rMaster.SetAttr( SwFmtFooter( bOn ));
                if( bOn )
                {
                    SvxULSpaceItem aUL(bHeader ? 0 : MM50, bHeader ? MM50 : 0 );
                    SwFrmFmt* pFmt = bHeader ?
                        (SwFrmFmt*)rMaster.GetHeader().GetHeaderFmt() :
                        (SwFrmFmt*)rMaster.GetFooter().GetFooterFmt();
                    pFmt->SetAttr( aUL );
                }
            }
            if( bChgd )
            {
                rSh.ChgPageDesc( nFrom, aDesc );

                if( !bCrsrSet && bOn )
                    bCrsrSet = rSh.SetCrsrInHdFt(
                            !rStyleName.Len() ? USHRT_MAX : nFrom,
                            bHeader );
            }
        }
    }
    rSh.EndUndo( 0 );
    rSh.EndAllAction();
}

