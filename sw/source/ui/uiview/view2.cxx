/*************************************************************************
 *
 *  $RCSfile: view2.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: os $ $Date: 2000-10-17 15:17:16 $
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

#ifndef _SVX_LANGITEM_HXX //autogen
#include <svx/langitem.hxx>
#endif
#ifndef _APP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _WRKWIN_HXX //autogen
#include <vcl/wrkwin.hxx>
#endif
#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifndef _SFX_BINDINGS_HXX //autogen
#include <sfx2/bindings.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX //autogen
#include <svx/lrspitem.hxx>
#endif
#ifndef _LINKDLG_HXX //autogen
#include <so3/linkdlg.hxx>
#endif
#ifndef _TXTCMP_HXX //autogen
#include <svtools/txtcmp.hxx>
#endif
#ifndef _SVX_ZOOM_HXX //autogen
#include <svx/zoom.hxx>
#endif
#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SVX_TSPTITEM_HXX //autogen
#include <svx/tstpitem.hxx>
#endif
#ifndef _SFXEVENT_HXX //autogen
#include <sfx2/event.hxx>
#endif
#ifndef _SFXDOCFILE_HXX //autogen
#include <sfx2/docfile.hxx>
#endif
#ifndef _SFX_DOCFILT_HACK_HXX //autogen
#include <sfx2/docfilt.hxx>
#endif
#ifndef _SFX_FCONTNR_HXX //autogen
#include <sfx2/fcontnr.hxx>
#endif
#ifndef _SVX_SIZEITEM_HXX //autogen
#include <svx/sizeitem.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SFX_TOPFRM_HXX
#include <sfx2/topfrm.hxx>
#endif
#ifndef _SFX_WHITER_HXX //autogen
#include <svtools/whiter.hxx>
#endif
#ifndef _SFXPTITEM_HXX //autogen
#include <svtools/ptitem.hxx>
#endif
#ifndef _SVXLINKMGR_HXX
#include <svx/linkmgr.hxx>
#endif
#ifndef __RSC //autogen
#include <tools/errinf.hxx>
#endif
#ifndef _URLOBJ_HXX //autogen
#include <tools/urlobj.hxx>
#endif
#ifndef _SVDVIEW_HXX //autogen
#include <svx/svdview.hxx>
#endif

#ifndef _SWWAIT_HXX
#include <swwait.hxx>
#endif
#ifndef _REDLNDLG_HXX
#include <redlndlg.hxx>
#endif
#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#ifndef _UIVWIMP_HXX
#include <uivwimp.hxx>
#endif
#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _VIEWOPT_HXX
#include <viewopt.hxx>
#endif
#ifndef _BASESH_HXX
#include <basesh.hxx>
#endif
#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif
#ifndef _UITOOL_HXX
#include <uitool.hxx>
#endif
#ifndef _SHELLIO_HXX
#include <shellio.hxx>
#endif
#ifndef _FMTINFMT_HXX
#include <fmtinfmt.hxx>
#endif
#ifndef _REDLENUM_HXX
#include <redlenum.hxx>
#endif
#ifndef _MDIEXP_HXX
#include <mdiexp.hxx>
#endif
#ifndef _DRAWBASE_HXX
#include <drawbase.hxx>
#endif
#ifndef _FRMMGR_HXX
#include <frmmgr.hxx>
#endif
#ifndef _PAGEDESC_HXX
#include <pagedesc.hxx>
#endif
#ifndef _LINENUM_HXX
#include <linenum.hxx>
#endif
#ifndef _SECTION_HXX
#include <section.hxx>
#endif
#ifndef _USRPREF_HXX
#include <usrpref.hxx>
#endif
#ifndef _BOOKMRK_HXX
#include <bookmrk.hxx>
#endif
#ifndef _NAVIPI_HXX
#include <navipi.hxx>
#endif
#ifndef _TOX_HXX
#include <tox.hxx>
#endif
#ifndef _WORKCTRL_HXX
#include <workctrl.hxx>
#endif
#ifndef _SCROLL_HXX
#include <scroll.hxx>
#endif
#ifndef _EDTWIN_HXX
#include <edtwin.hxx>
#endif
#ifndef _WVIEW_HXX
#include <wview.hxx>
#endif
#ifndef _TEXTSH_HXX
#include <textsh.hxx>
#endif
#ifndef _TABSH_HXX
#include <tabsh.hxx>
#endif
#ifndef _LISTSH_HXX
#include <listsh.hxx>
#endif

#ifndef _CMDID_H
#include <cmdid.h>
#endif
#ifndef _SWSWERROR_H
#include <swerror.h>
#endif
#ifndef _GLOBALS_HRC
#include <globals.hrc>
#endif
#ifndef _SHELLS_HRC
#include <shells.hrc>
#endif
#ifndef _WEB_HRC
#include <web.hrc>
#endif
#ifndef _VIEW_HRC
#include <view.hrc>
#endif
#ifndef _FMTCLDS_HXX //autogen
#include <fmtclds.hxx>
#endif

//Damit die Seitenanzeige in der Statusleiste nicht unnoetig erfolgt.
static String sLstPg;
static USHORT nPageCnt = 0;
const char __FAR_DATA sStatusDelim[] = " : ";

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::scanner;

/*---------------------------------------------------------------------------
    Beschreibung:   String fuer die Seitenanzeige in der Statusbar basteln.
 ----------------------------------------------------------------------------*/

String SwView::GetPageStr( USHORT nPg, USHORT nLogPg,
                            const String& rDisplay )
{
    String aStr( aPageStr );
    if( rDisplay.Len() )
        aStr += rDisplay;
    else
        aStr += String::CreateFromInt32(nLogPg);

    if( nLogPg && nLogPg != nPg )
    {
        aStr.AppendAscii( RTL_CONSTASCII_STRINGPARAM("   "));
        aStr += String::CreateFromInt32(nPg);
    }
    aStr.AppendAscii( RTL_CONSTASCII_STRINGPARAM(" / "));
    aStr += String::CreateFromInt32( GetWrtShell().GetPageCnt() );

    return aStr;
}

void __EXPORT SwView::Execute(SfxRequest &rReq)
{
    USHORT nSlot = rReq.GetSlot();
    const SfxItemSet* pArgs = rReq.GetArgs();
    const SfxPoolItem* pItem;
    switch( nSlot )
    {
        case SID_CREATE_SW_DRAWVIEW:
            pWrtShell->GetDoc()->MakeDrawModel();
            break;

        case FN_LINE_NUMBERING_DLG:
        {
            SwLineNumberingDlg *pDlg = new SwLineNumberingDlg(this);
            pDlg->Execute();
            delete pDlg;
            break;
        }
        case FN_EDIT_LINK_DLG:
            EditLinkDlg();
            break;
        case FN_REFRESH_VIEW:
            GetEditWin().Invalidate();
            break;
        case FN_PAGEUP:
        case FN_PAGEUP_SEL:
        {
            Rectangle aVis( GetVisArea() );
            PageUpCrsr(FN_PAGEUP_SEL == nSlot);
            rReq.SetReturnValue(SfxBoolItem(nSlot,
                                                aVis != GetVisArea()));
        }
        break;
        case FN_PAGEDOWN_SEL:
        case FN_PAGEDOWN:
        {
            Rectangle aVis( GetVisArea() );
            PageDownCrsr(FN_PAGEDOWN_SEL == nSlot);
            rReq.SetReturnValue(SfxBoolItem(nSlot,
                                                aVis != GetVisArea()));
        }
        break;
        case FN_REDLINE_ON:
        {
            if (SFX_ITEM_SET == pArgs->GetItemState(nSlot, FALSE, &pItem))
            {
                USHORT nOn = ((const SfxBoolItem*)pItem)->GetValue() ? REDLINE_ON : 0;
                USHORT nMode = pWrtShell->GetRedlineMode();
                pWrtShell->SetRedlineMode( (nMode & ~REDLINE_ON) | nOn);
            }
        }
        break;
        case FN_REDLINE_SHOW:

            if (SFX_ITEM_SET == pArgs->GetItemState(nSlot, FALSE, &pItem))
            {
                USHORT nMode = ( ~(REDLINE_SHOW_INSERT | REDLINE_SHOW_DELETE)
                        & pWrtShell->GetRedlineMode() ) | REDLINE_SHOW_INSERT;
                if( ((const SfxBoolItem*)pItem)->GetValue() )
                    nMode |= REDLINE_SHOW_DELETE;

                pWrtShell->SetRedlineMode( nMode );
            }
            break;
        case FN_REDLINE_ACCEPT:
            GetViewFrame()->ToggleChildWindow(FN_REDLINE_ACCEPT);
            break;

        case SID_DOCUMENT_COMPARE:
        case SID_DOCUMENT_MERGE:
            {
                String sFileName, sFilterName;
                INT16 nVersion = 0;

                if( pArgs )
                {
                    if( SFX_ITEM_SET == pArgs->GetItemState( SID_FILE_NAME, FALSE, &pItem ))
                        sFileName = ((const SfxStringItem*)pItem)->GetValue();

                    if( SFX_ITEM_SET == pArgs->GetItemState( SID_FILTER_NAME, FALSE, &pItem ))
                        sFilterName = ((const SfxStringItem*)pItem)->GetValue();

                    if( SFX_ITEM_SET == pArgs->GetItemState( SID_VERSION, FALSE, &pItem ))
                        nVersion = ((const SfxInt16Item *)pItem)->GetValue();
                }

                long nFound = InsertDoc( nSlot, sFileName, sFilterName, nVersion );
                rReq.SetReturnValue( SfxInt32Item( nSlot, nFound ));

                if (nFound > 0) // Redline-Browser anzeigen
                {
                    SfxViewFrame* pVFrame = GetViewFrame();
                    pVFrame->ShowChildWindow(FN_REDLINE_ACCEPT);

                    // RedlineDlg neu initialisieren
                    USHORT nId = SwRedlineAcceptChild::GetChildWindowId();
                    SwRedlineAcceptChild *pRed = (SwRedlineAcceptChild*)
                                            pVFrame->GetChildWindow(nId);
                    if (pRed)
                        pRed->ReInitDlg(GetDocShell());
                }
        }
        break;
        case FN_SYNC_LABELS:
            GetViewFrame()->ShowChildWindow(FN_SYNC_LABELS, TRUE);
            break;
        case FN_ESCAPE:
            if ( pWrtShell->HasDrawView() && pWrtShell->GetDrawView()->IsDragObj() )
            {
                pWrtShell->BreakDrag();
                pWrtShell->EnterSelFrmMode();
            }
            else if ( pWrtShell->IsDrawCreate() )
            {
                GetDrawFuncPtr()->BreakCreate();
                AttrChangedNotify(pWrtShell); // ggf Shellwechsel...
            }
            else if ( pWrtShell->HasSelection() || IsDrawMode() )
            {
                LeaveDrawCreate();
                pWrtShell->EnterStdMode();
                AttrChangedNotify(pWrtShell); // ggf Shellwechsel...
            }
            else if ( GetEditWin().GetApplyTemplate() )
                GetEditWin().SetApplyTemplate(SwApplyTemplate());
            else if( ((SfxObjectShell*)GetDocShell())->GetInPlaceObject() &&
                        ((SfxObjectShell*)GetDocShell())->GetInPlaceObject()->GetIPClient() )
            {
                ErrCode nErr = GetDocShell()->DoInPlaceActivate( FALSE );
                if ( nErr )
                    ErrorHandler::HandleError( nErr );
            }
            else if ( GetEditWin().IsChainMode() )
                GetEditWin().SetChainMode( FALSE );
//JP 10.06.99: warten auf SLOT von MBA
//          else if(Application::GetAppWindow()->IsFullScreenView())
//              GetViewFrame()->GetDispatcher()->Execute(SID_WIN_FULLSCREEN);
            break;
        case SID_ATTR_BORDER_INNER:
        case SID_ATTR_BORDER_OUTER:
        case SID_ATTR_BORDER_SHADOW:
            if(pArgs)
                pWrtShell->SetAttr(*pArgs);
            break;

        case SID_ATTR_PAGE:
        case SID_ATTR_PAGE_SIZE:
        case SID_ATTR_PAGE_MAXSIZE:
        case SID_ATTR_PAGE_PAPERBIN:
        case SID_ATTR_PAGE_EXT1:
        case FN_PARAM_FTN_INFO:
        {
            if(pArgs)
            {
                const USHORT nCurIdx = pWrtShell->GetCurPageDesc();
                SwPageDesc aPageDesc( pWrtShell->GetPageDesc( nCurIdx ) );
                ::ItemSetToPageDesc( *pArgs, aPageDesc,
                                    pWrtShell );
                // Den Descriptor der Core veraendern.
                pWrtShell->ChgPageDesc( nCurIdx, aPageDesc );
            }
        }
        break;
        case FN_NAVIGATION_PI_GOTO_PAGE:
        {
            SfxViewFrame* pVFrame = GetViewFrame();
            SfxChildWindow* pCh = pVFrame->GetChildWindow( SID_NAVIGATOR );
            if(!pCh)
            {
                pVFrame->ToggleChildWindow( SID_NAVIGATOR );
                pCh = pVFrame->GetChildWindow( SID_NAVIGATOR );

            }
                ((SwNavigationPI*) pCh->GetContextWindow(SW_MOD()))->GotoPage();
        }
        break;
        case  FN_EDIT_CURRENT_TOX:
        {
            GetViewFrame()->GetDispatcher()->Execute(
                                FN_INSERT_MULTI_TOX, SFX_CALLMODE_ASYNCHRON);
        }
        break;
        case FN_UPDATE_CUR_TOX:
        {
            const SwTOXBase* pBase = pWrtShell->GetCurTOX();
            if(pBase)
            {
                pWrtShell->StartAction();
                if(TOX_INDEX == pBase->GetType())
                    pWrtShell->ApplyAutoMark();
                pWrtShell->UpdateTableOf( *pBase );
                pWrtShell->EndAction();
            }
        }
        break;
        case FN_UPDATE_TOX:
        {
            pWrtShell->StartAction();
            pWrtShell->EnterStdMode();
            FASTBOOL bOldCrsrInReadOnly = pWrtShell->IsReadOnlyAvailable();
            pWrtShell->SetReadOnlyAvailable( TRUE );

            for( USHORT i = 0; i < 2; ++i )
            {
                USHORT nCount = pWrtShell->GetTOXCount();
                if( 1 == nCount )
                    ++i;

                while( pWrtShell->GotoPrevTOXBase() )
                    ;   // aufs erste Verzeichnis springen

                // falls wir nicht mehr in einem stehen, dann zum naechsten
                // springen.
                const SwTOXBase* pBase = pWrtShell->GetCurTOX();
                if( !pBase )
                {
                    pWrtShell->GotoNextTOXBase();
                    pBase = pWrtShell->GetCurTOX();
                }

                BOOL bAutoMarkApplied = FALSE;
                while( pBase )
                {
                    if(TOX_INDEX == pBase->GetType() && !bAutoMarkApplied)
                    {
                        pWrtShell->ApplyAutoMark();
                        bAutoMarkApplied = TRUE;
                    }
                    // JP 15.07.96: das pBase wird nur fuer die Schnittstelle
                    //              benoetigt. Muss mal umgetstellt werden!!!
                    pWrtShell->UpdateTableOf( *pBase );

                    if( pWrtShell->GotoNextTOXBase() )
                        pBase = pWrtShell->GetCurTOX();
                    else
                        pBase = 0;
                }
            }
            pWrtShell->SetReadOnlyAvailable( bOldCrsrInReadOnly );
            pWrtShell->EndAction();
        }
        break;
        case SID_ATTR_BRUSH:
        {
            if(pArgs && SFX_ITEM_SET == pArgs->GetItemState(RES_BACKGROUND, FALSE, &pItem))
            {
                const USHORT nCurIdx = pWrtShell->GetCurPageDesc();
                SwPageDesc aDesc( pWrtShell->GetPageDesc( nCurIdx ));
                SwFrmFmt& rMaster = aDesc.GetMaster();
                rMaster.SetAttr(*pItem);
                pWrtShell->ChgPageDesc( nCurIdx, aDesc);
            }
        }
        break;
        case SID_CLEARHISTORY:
        {
            pWrtShell->DelAllUndoObj();
        }
        break;
        case SID_UNDO:
        {
            pShell->ExecuteSlot(rReq);
        }
        break;
        case FN_INSERT_CTRL:
        case FN_INSERT_OBJ_CTRL:
        {
            if(pArgs && SFX_ITEM_SET == pArgs->GetItemState(nSlot, FALSE, &pItem))
            {
                USHORT nValue = ((SfxUInt16Item*)pItem)->GetValue();
                switch ( nSlot )
                {
                    case FN_INSERT_CTRL:
                    {
                        BOOL bWeb = 0 != PTR_CAST(SwWebView, this);
                        if(bWeb)
                            SwView::nWebInsertCtrlState = nValue;
                        else
                            SwView::nInsertCtrlState = nValue;
                    }
                    break;
                    case FN_INSERT_OBJ_CTRL:    SwView::nInsertObjectCtrlState = nValue  ;break;
                }

            }
            GetViewFrame()->GetBindings().Invalidate( nSlot );
        }
        break;
#if defined WIN || defined WNT || defined UNX
        case SID_TWAIN_SELECT:
        case SID_TWAIN_TRANSFER:
            GetViewImpl()->ExcuteScan(nSlot);
        break;
#endif

        case SID_ATTR_DEFTABSTOP:
        {
            if(pArgs && SFX_ITEM_SET == pArgs->GetItemState(SID_ATTR_DEFTABSTOP, FALSE, &pItem))
            {
                SvxTabStopItem aDefTabs( 0, 0 );
                USHORT nTab = ((const SfxUInt16Item*)pItem)->GetValue();
                MakeDefTabs( nTab, aDefTabs );
                pWrtShell->SetDefault( aDefTabs );
            }
        }
        break;
        case SID_ATTR_LANGUAGE:
        {
            if(pArgs && SFX_ITEM_SET == pArgs->GetItemState(SID_ATTR_LANGUAGE, FALSE, &pItem))
            {
                SvxLanguageItem aLang(((SvxLanguageItem*)pItem)->GetLanguage(), RES_CHRATR_LANGUAGE);
                pWrtShell->SetDefault( aLang );
            }
        }
        break;
        case FN_INSERT_HEADER:
            pWrtShell->SetHeadInBrowse(!pWrtShell->IsHeadInBrowse());
            break;
        case FN_INSERT_FOOTER:
            pWrtShell->SetFootInBrowse(!pWrtShell->IsFootInBrowse());
            break;
        case FN_SCROLL_NEXT_PREV:
            if(pArgs && pArgs->GetItemState(FN_SCROLL_NEXT_PREV, FALSE, &pItem))
            {
                // hier sollen nur die Handler der PageUp/DownButtons gerufen werden
                BOOL bNext = ((const SfxBoolItem*)pItem)->GetValue();
                MoveNavigation(bNext);
            }
            break;
        case SID_JUMPTOMARK:
            if( pArgs && SFX_ITEM_SET == pArgs->GetItemState(SID_JUMPTOMARK, FALSE, &pItem))
                JumpToSwMark( (( const SfxStringItem*)pItem)->GetValue() );
            break;
        case SID_GALLERY :
            GetViewFrame()->ToggleChildWindow(SID_GALLERY);
        break;
        default:
            ASSERT(!this, falscher Dispatcher);
            return;
    }
}

/*--------------------------------------------------------------------
    Beschreibung:   SeitenNr-Feld invalidieren
 --------------------------------------------------------------------*/

void SwView::UpdatePageNums(USHORT nPhyNum, USHORT nVirtNum, const String& rPgStr)
{
    String sTemp(GetPageStr( nPhyNum, nVirtNum, rPgStr ));
    if ( sLstPg != sTemp )
    {
        sLstPg = sTemp;
        const SfxStringItem aTmp( FN_STAT_PAGE,
                                 sLstPg);
        SfxBindings &rBnd = GetViewFrame()->GetBindings();
        rBnd.SetState( aTmp );
        rBnd.Update( FN_STAT_PAGE );
    }
}

/*--------------------------------------------------------------------
    Beschreibung:   Status der Stauszeile
 --------------------------------------------------------------------*/


void SwView::StateStatusLine(SfxItemSet &rSet)
{
    SwWrtShell& rShell = GetWrtShell();

    SfxWhichIter aIter( rSet );
    USHORT nWhich = aIter.FirstWhich();
    ASSERT( nWhich, "leeres Set");

    while( nWhich )
    {
        switch( nWhich )
        {
            case FN_STAT_PAGE:
/*
//JP 07.01.00: is a nice feature - show the selektion of DrawObjects
            if( rShell.IsObjSelected()
//???               || rShell.IsFrmSelected()
                )
            {
                String sDisplay( rShell.GetDrawView()->GetMarkList().
                                    GetMarkDescription() );
                rSet.Put( SfxStringItem( FN_STAT_PAGE, sDisplay ));
            }
            else
*/          {
                // Anzahl der Seiten, log. SeitenNr. SeitenNr ermitteln
                USHORT nPage, nLogPage;
                String sDisplay;
                rShell.GetPageNumber( -1, rShell.IsCrsrVisible(), nPage, nLogPage, sDisplay );
                rSet.Put( SfxStringItem( FN_STAT_PAGE,
                            GetPageStr( nPage, nLogPage, sDisplay) ));

                USHORT nCnt = GetWrtShell().GetPageCnt();
                if (nPageCnt != nCnt)   // Basic benachrichtigen
                {
                    nPageCnt = nCnt;
                    SFX_APP()->NotifyEvent(SfxEventHint(SW_EVENT_PAGE_COUNT, GetViewFrame()->GetObjectShell()), FALSE);
                }
            }
            break;
            case FN_STAT_TEMPLATE:
            {
                rSet.Put(SfxStringItem( FN_STAT_TEMPLATE,
                                        rShell.GetCurPageStyle(FALSE)));

            }
            break;
            case SID_ATTR_ZOOM:
            {
                if ( GetDocShell()->GetCreateMode() != SFX_CREATE_MODE_EMBEDDED )
                {
                    const SwViewOption* pVOpt = rShell.GetViewOptions();
                    SvxZoomType eZoom = (SvxZoomType) pVOpt->GetZoomType();
                    SvxZoomItem aZoom(eZoom,
                                        pVOpt->GetZoom());
                    if(pWrtShell->IsBrowseMode())
                    {
                        aZoom.SetValueSet(
                                SVX_ZOOM_ENABLE_50|
                                SVX_ZOOM_ENABLE_75|
                                SVX_ZOOM_ENABLE_100|
                                SVX_ZOOM_ENABLE_150|
                                SVX_ZOOM_ENABLE_200);
                    }
                    rSet.Put( aZoom );
                }
                else
                    rSet.DisableItem( SID_ATTR_ZOOM );
            }
            break;
            case SID_ATTR_POSITION:
            case SID_ATTR_SIZE:
            {
                if( !rShell.IsFrmSelected() && !rShell.IsObjSelected() )
                    SwBaseShell::_SetFrmMode( FLY_DRAG_END );
                else
                {
                    USHORT nFrameMode = SwBaseShell::GetFrmMode();
                    if ( nFrameMode == FLY_DRAG_START || nFrameMode == FLY_DRAG )
                    {
                        if ( nWhich == SID_ATTR_POSITION )
                            rSet.Put( SfxPointItem( SID_ATTR_POSITION,
                                                    rShell.GetAnchorObjDiff()));
                        else
                            rSet.Put( SvxSizeItem( SID_ATTR_SIZE,
                                                   rShell.GetObjSize()));
                    }
                }
            }
            break;
            case SID_TABLE_CELL:

            if( rShell.IsFrmSelected() || rShell.IsObjSelected() )
            {
                rSet.Put( SvxSizeItem( SID_TABLE_CELL, rShell.GetObjSize()));
            }
            else
            {
                String sStr;
                if( rShell.IsCrsrInTbl() )
                    sStr = rShell.GetBoxNms();
                else
                {
                    const SwSection* pCurrSect = rShell.GetCurrSection();
                    if( pCurrSect )
                    {
                        switch( pCurrSect->GetType() )
                        {
                        case TOX_HEADER_SECTION:
                        case TOX_CONTENT_SECTION:
                            {
                                const SwTOXBase* pTOX = pWrtShell->GetCurTOX();
                                if( pTOX )
                                    sStr = pTOX->GetTOXName();
                                else
                                {
                                    ASSERT( !this,
                                        "was ist das fuer ein Verzeichnis?" );
                                    sStr = pCurrSect->GetName();
                                }
                            }
                            break;
                        default:
                            sStr = pCurrSect->GetName();
                            break;
                        }
                    }
                }
                const SwNumRule* pNumRule = rShell.GetCurNumRule();
                if (pNumRule)   // Cursor in Numerierung
                {
                    BYTE nNumLevel = rShell.GetNumLevel();
                    if( NO_NUM > nNumLevel && MAXLEVEL >
                        ( nNumLevel = GetRealLevel( nNumLevel )) )
                    {
                        if( sStr.Len() )
                            sStr.AppendAscii(sStatusDelim);
                        sStr += SW_RESSTR(STR_NUM_LEVEL);
                        sStr += String::CreateFromInt32( nNumLevel + 1 );
                        if(!pNumRule->IsAutoRule())
                        {
                            SfxItemSet aSet(GetPool(),
                                    RES_PARATR_NUMRULE, RES_PARATR_NUMRULE);
                            rShell.GetAttr(aSet);
                            const SfxPoolItem* pItem;
                            if(SFX_ITEM_AVAILABLE <=
                                aSet.GetItemState(RES_PARATR_NUMRULE, TRUE, &pItem))
                            {
                                const String& rNumStyle =
                                    ((const SfxStringItem*)pItem)->GetValue();
                                if(rNumStyle.Len())
                                {
                                    sStr.AppendAscii(sStatusDelim);
                                    sStr += rNumStyle;
                                }
                            }
                        }
                    }
                }

                if( rShell.HasReadonlySel() )
                {
                    if( sStr.Len() )
                        sStr.InsertAscii( sStatusDelim, 0 );
                    sStr.Insert( SW_RESSTR( STR_READONLY_SEL ), 0 );
                }
                if( sStr.Len() )
                    rSet.Put( SfxStringItem( SID_TABLE_CELL, sStr ));
            }
            break;
            case FN_STAT_SELMODE:
            {
                if(rShell.IsStdMode())
                    rSet.Put(SfxUInt16Item(FN_STAT_SELMODE, 0));
                else if(rShell.IsAddMode())
                    rSet.Put(SfxUInt16Item(FN_STAT_SELMODE, 2));
                else
                    rSet.Put(SfxUInt16Item(FN_STAT_SELMODE, 1));
                break;
            }
            case SID_ATTR_INSERT:
                if( rShell.IsRedlineOn() )
                    rSet.DisableItem( nWhich );
                else
                {
                    rSet.Put(SfxBoolItem(SID_ATTR_INSERT,rShell.IsInsMode()));
                }
                break;
            case FN_STAT_HYPERLINKS:
            {
                rSet.Put(SfxBoolItem(FN_STAT_HYPERLINKS,
                            pWrtShell->GetViewOptions()->IsExecHyperlinks()));
            }
            break;
        }
        nWhich = aIter.NextWhich();
    }
}

/*--------------------------------------------------------------------
    Beschreibung:   Execute fuer die Stauszeile
 --------------------------------------------------------------------*/


void SwView::ExecuteStatusLine(SfxRequest &rReq)
{
    SwWrtShell &rSh = GetWrtShell();
    const SfxItemSet* pArgs = rReq.GetArgs();
    const SfxPoolItem* pItem;
    BOOL bUp = FALSE;
    USHORT nWhich = rReq.GetSlot();
    switch( nWhich )
    {
        case FN_STAT_PAGE:
        {
            GetViewFrame()->GetDispatcher()->Execute( SID_NAVIGATOR,
                                      SFX_CALLMODE_SYNCHRON|SFX_CALLMODE_RECORD );
        }
        break;

        case FN_STAT_BOOKMARK:
        if ( pArgs )
        {
            if (SFX_ITEM_SET == pArgs->GetItemState( nWhich, TRUE, &pItem))
            {
                USHORT nDest;
                nDest = ((const SfxUInt16Item *)pItem)->GetValue();
                rSh.EnterStdMode();
                rSh.GotoBookmark( nDest );
            }
        }
        break;

        case FN_STAT_TEMPLATE:
        {
            GetViewFrame()->GetDispatcher()->Execute(FN_FORMAT_PAGE_DLG,
                                        SFX_CALLMODE_SYNCHRON|SFX_CALLMODE_RECORD );
        }
        break;
        case SID_ATTR_ZOOM:
        {
            if ( GetDocShell()->GetCreateMode() != SFX_CREATE_MODE_EMBEDDED )
            {
                const SfxItemSet *pSet = 0;
                SvxZoomDialog *pDlg = 0;
                if ( pArgs )
                    pSet = pArgs;
                else if ( GetDocShell()->GetCreateMode() != SFX_CREATE_MODE_EMBEDDED )
                {
                    SfxItemSet aCoreSet(pShell->GetPool(), SID_ATTR_ZOOM, SID_ATTR_ZOOM);
                    SvxZoomItem aZoom( (SvxZoomType)rSh.GetViewOptions()->GetZoomType(),
                                                rSh.GetViewOptions()->GetZoom() );

                    if(rSh.IsBrowseMode())
                    {
                        aZoom.SetValueSet(
                                SVX_ZOOM_ENABLE_50|
                                SVX_ZOOM_ENABLE_75|
                                SVX_ZOOM_ENABLE_100|
                                SVX_ZOOM_ENABLE_150|
                                SVX_ZOOM_ENABLE_200);
                    }
                    aCoreSet.Put( aZoom );

                    pDlg = new SvxZoomDialog( &GetViewFrame()->GetWindow(), aCoreSet );
                    pDlg->SetLimits( MINZOOM, MAXZOOM );

                    if( pDlg->Execute() != RET_CANCEL )
                        pSet = pDlg->GetOutputItemSet();
                }

                if ( pSet && SFX_ITEM_SET == pSet->GetItemState(SID_ATTR_ZOOM, TRUE, &pItem))
                {
                    enum SvxZoomType eType = ((const SvxZoomItem *)pItem)->GetType();
                    SetZoom( eType, ((const SvxZoomItem *)pItem)->GetValue() );
                }
                delete pDlg;
                bUp = TRUE;
            }
        }
        break;

        case SID_ATTR_SIZE:
        {
            ULONG nId = FN_INSERT_FIELD;
            if( rSh.IsCrsrInTbl() )
                nId = FN_FORMAT_TABLE_DLG;
            else if( rSh.GetCurTOX() )
                nId = FN_INSERT_MULTI_TOX;
            else if( rSh.GetCurrSection() )
                nId = FN_EDIT_REGION;
            else
            {
                const SwNumRule* pNumRule = rSh.GetCurNumRule();
                if( pNumRule )  // Cursor in Numerierung
                {
                    if( pNumRule->IsAutoRule() )
                        nId = FN_NUMBER_BULLETS;
                    else
                    {
                        // Dialog vom Gestalter starten ;-)
                        nId = 0;
                    }
                }
                else if( rSh.IsFrmSelected() )
                    nId = FN_FORMAT_FRAME_DLG;
                else if( rSh.IsObjSelected() )
                    nId = SID_ATTR_TRANSFORM;
            }
            if( nId )
                GetViewFrame()->GetDispatcher()->Execute( nId, SFX_CALLMODE_SYNCHRON |
                                               SFX_CALLMODE_RECORD );
        }
        break;

        case FN_STAT_SELMODE:
        {
            if ( pArgs )
            {
                if (SFX_ITEM_SET == pArgs->GetItemState( nWhich, TRUE, &pItem))
                {
                    switch ( ((const SfxUInt16Item *)pItem)->GetValue() )
                    {
                        case 0: rSh.EnterStdMode(); break;
                        case 1: rSh.EnterExtMode(); break;
                        case 2: rSh.EnterAddMode(); break;
                    }
                }
            }
            else
            {

                if( !rSh.IsAddMode() && !rSh.IsExtMode() )
                    rSh.ToggleExtMode();
                else if ( rSh.IsExtMode() )
                {
                    rSh.ToggleExtMode();
                    rSh.ToggleAddMode();
                }
                else if ( rSh.IsAddMode() )
                    rSh.ToggleAddMode();
            }
            bUp = TRUE;
            break;
        }
        case FN_SET_ADD_MODE:
            rSh.ToggleAddMode();
            nWhich = FN_STAT_SELMODE;
            bUp = TRUE;
        break;
        case FN_SET_EXT_MODE:
            rSh.ToggleExtMode();
            nWhich = FN_STAT_SELMODE;
            bUp = TRUE;
        break;
        case SID_ATTR_INSERT:
            rSh.ToggleInsMode();
            bUp = TRUE;
        break;

        case FN_STAT_HYPERLINKS:
        {
            SwViewOption aOpt( *rSh.GetViewOptions() );
            BOOL bIs = aOpt.IsExecHyperlinks();
            BOOL bSet;
            if(pArgs && SFX_ITEM_SET == pArgs->GetItemState(nWhich, FALSE, &pItem))
                bSet = ((const SfxBoolItem*)pItem)->GetValue();
            else
                bSet = !bIs;
            if(bIs != bSet)
            {
                aOpt.SetExecHyperlinks(bSet);
                SW_MOD()->ApplyUsrPref(aOpt, this);
                bUp = TRUE;
            }
        }
        break;
    }
    if ( bUp )
    {
        SfxBindings &rBnd = GetViewFrame()->GetBindings();
        rBnd.Invalidate(nWhich);
        rBnd.Update(nWhich);
    }
}




void SwView::InsFrmMode(USHORT nCols)
{
    if ( pWrtShell->HasWholeTabSelection() )
    {
        SwFlyFrmAttrMgr aMgr( TRUE, pWrtShell, FRMMGR_TYPE_TEXT );

        const SwFrmFmt &rPageFmt =
                pWrtShell->GetPageDesc(pWrtShell->GetCurPageDesc()).GetMaster();
        SwTwips lWidth = rPageFmt.GetFrmSize().GetWidth();
        const SvxLRSpaceItem &rLR = rPageFmt.GetLRSpace();
        lWidth -= rLR.GetLeft() + rLR.GetRight();
        aMgr.SetSize(Size(lWidth, aMgr.GetSize().Height()));
        if(nCols > 1)
        {
            SwFmtCol aCol;
            aCol.Init( nCols, aCol.GetGutterWidth(), aCol.GetWishWidth() );
            aMgr.SetCol( aCol );
        }
        aMgr.InsertFlyFrm();
    }
    else
        GetEditWin().InsFrm(nCols);
}

/*--------------------------------------------------------------------
    Beschreibung:   Links bearbeiten
 --------------------------------------------------------------------*/

void SwView::EditLinkDlg()
{
    BOOL bWeb = 0 != PTR_CAST(SwWebView, this);
    SvBaseLinksDialog aSvBaseLinksDialog( &GetViewFrame()->GetWindow(), &GetWrtShell().GetLinkManager()
#if SUPD>355
,bWeb
#endif
     );
    aSvBaseLinksDialog.Execute();
/*
    SwLinkDlg* pDlg = new SwLinkDlg(GetFrameWindow());
    pDlg->SetShell(&GetWrtShell());
    pDlg->Execute();

    DELETEZ(pDlg);
*/
}

BOOL SwView::JumpToSwMark( const String& rMark )
{
    BOOL bRet = FALSE;
    if( rMark.Len() )
    {
        // wir wollen den Bookmark aber am oberen Rand haben
        BOOL bSaveCC = IsCrsrAtCenter();
        BOOL bSaveCT = IsCrsrAtTop();
        SetCrsrAtTop( TRUE );

        //JP 27.04.98: Bug 49786
        // Damit in FrameSet auch gescrollt werden kann, muss die
        // entsprechende Shell auch das Focus-Flag gesetzt haben!
        BOOL bHasShFocus = pWrtShell->HasShFcs();
        if( !bHasShFocus )
            pWrtShell->ShGetFcs( FALSE );

        const SwFmtINetFmt* pINet;
        String sCmp, sMark( INetURLObject::decode( rMark, INET_HEX_ESCAPE,
                                           INetURLObject::DECODE_WITH_CHARSET,
                                        RTL_TEXTENCODING_UTF8 ));

        xub_StrLen nLastPos, nPos = sMark.Search( cMarkSeperator );
        if( STRING_NOTFOUND != nPos )
            while( STRING_NOTFOUND != ( nLastPos =
                sMark.Search( cMarkSeperator, nPos + 1 )) )
                nPos = nLastPos;

        if( STRING_NOTFOUND != nPos &&
            ( sCmp = sMark.Copy( nPos + 1 ) ).EraseAllChars().Len() )
        {
            String sName( sMark.Copy( 0, nPos ) );
            sCmp.ToLowerAscii();
            FlyCntType eFlyType = FLYCNTTYPE_ALL;

            if( COMPARE_EQUAL == sCmp.CompareToAscii( pMarkToRegion ) )
            {
                pWrtShell->EnterStdMode();
                bRet = pWrtShell->GotoRegion( sName );
            }
            else if( COMPARE_EQUAL == sCmp.CompareToAscii( pMarkToOutline ) )
            {
                pWrtShell->EnterStdMode();
                bRet = pWrtShell->GotoOutline( sName );
            }
            else if( COMPARE_EQUAL == sCmp.CompareToAscii( pMarkToFrame ) )
                eFlyType = FLYCNTTYPE_FRM;
            else if( COMPARE_EQUAL == sCmp.CompareToAscii( pMarkToGraphic ) )
                eFlyType = FLYCNTTYPE_GRF;
            else if( COMPARE_EQUAL == sCmp.CompareToAscii( pMarkToOLE ) )
                eFlyType = FLYCNTTYPE_OLE;
            else if( COMPARE_EQUAL == sCmp.CompareToAscii( pMarkToTable ) )
            {
                pWrtShell->EnterStdMode();
                bRet = pWrtShell->GotoTable( sName );
            }
            else if( COMPARE_EQUAL == sCmp.CompareToAscii( pMarkToText ) )
            {
                // Normale Textsuche
                pWrtShell->EnterStdMode();
                SearchParam aParam( sName, SearchParam::SRCH_NORMAL,
                                    TRUE, FALSE, FALSE );

                if( pWrtShell->SearchPattern( aParam, DOCPOS_START, DOCPOS_END ))
                {
                    pWrtShell->EnterStdMode();      // Selektion wieder aufheben
                    bRet = TRUE;
                }
            }
            else if( USHRT_MAX != ( nPos = pWrtShell->FindBookmark( sMark ) ))
                pWrtShell->GotoBookmark( nPos, FALSE, TRUE ), bRet = TRUE;
            else if( 0 != ( pINet = pWrtShell->FindINetAttr( sMark ) ))
                bRet = pWrtShell->GotoINetAttr( *pINet->GetTxtINetFmt() );

            // fuer alle Arten von Flys
            if( FLYCNTTYPE_ALL != eFlyType && pWrtShell->GotoFly( sName, eFlyType ))
            {
                bRet = TRUE;
                if( FLYCNTTYPE_FRM == eFlyType )
                {
                    // TextFrames: Cursor in den Frame setzen
                    pWrtShell->UnSelectFrm();
                    pWrtShell->LeaveSelFrmMode();
                }
                else
                {
                    pWrtShell->HideCrsr();
                    pWrtShell->EnterSelFrmMode();
                }
            }
        }
        else if( USHRT_MAX != ( nPos = pWrtShell->FindBookmark( sMark ) ))
            pWrtShell->GotoBookmark( nPos, FALSE, TRUE ), bRet = TRUE;
        else if( 0 != ( pINet = pWrtShell->FindINetAttr( sMark ) ))
            bRet = pWrtShell->GotoINetAttr( *pINet->GetTxtINetFmt() );

        // ViewStatus wieder zurueck setzen
        SetCrsrAtTop( bSaveCT, bSaveCC );

        if( !bHasShFocus )
            pWrtShell->ShLooseFcs();
    }
    return bRet;
}

/*--------------------------------------------------------------------
    Beschreibung:   Links bearbeiten
 --------------------------------------------------------------------*/

long SwView::InsertDoc( USHORT nSlotId, const String& rFileName,
                        const String& rFilterName, INT16 nVersion )
{
    BOOL bInsert = FALSE, bCompare = FALSE, bMerge = FALSE;
    long nFound = 0;

    switch( nSlotId )
    {
        case SID_DOCUMENT_MERGE:        bMerge = TRUE;      break;
        case SID_DOCUMENT_COMPARE:      bCompare = TRUE;    break;
        case SID_INSERTDOC:             bInsert = TRUE;     break;

        default:
            ASSERT( !this, "Unbekannte SlotId!" );
            bInsert = TRUE;
            nSlotId = SID_INSERTDOC;
            break;
    }

    SfxMedium* pMed = 0;
    SwDocShell* pDocSh = GetDocShell();
    if( rFileName.Len() )
    {
        const SfxFilter* pFilter = 0;

        SfxObjectFactory& rFact = pDocSh->GetFactory();
        const USHORT nCount = rFact.GetFilterCount();
        for( USHORT i = 0; i < nCount; i++ )
        {
            const SfxFilter* pFlt = rFact.GetFilter( i );
            if( pFlt->GetName() == rFilterName ||
                pFlt->GetFilterName() == rFilterName )
            {
                pFilter = pFlt;
                break;
            }
        }
        if ( !pFilter )
        {
            pMed = new SfxMedium(rFileName, STREAM_READ, TRUE, 0, 0 );
            SfxFilterMatcher aMatcher( rFact.GetFilterContainer() );
            ErrCode nErr = aMatcher.GuessFilter( *pMed, &pFilter, FALSE );
            if ( nErr )
                DELETEZ(pMed);
            else
                pMed->SetFilter( pFilter );
        }
        else
            pMed = new SfxMedium(rFileName, STREAM_READ, TRUE, pFilter, 0);
    }
    else
    {
        pMed = SFX_APP()->InsertDocumentDialog(0, pDocSh->GetFactory());
    }
    if( !pMed )
        return -1;

    if( bInsert )
    {
        SfxObjectShellRef aRef( pDocSh );

        pDocSh->RegisterTransfer( *pMed );
        pMed->DownLoad();   // ggfs. den DownLoad anstossen
        if( aRef.Is() && 1 < aRef->GetRefCount() )  // noch gueltige Ref?
        {
            SwReader* pRdr;
            Reader *pRead = pDocSh->StartConvertFrom( *pMed, &pRdr, pWrtShell );
            if( pRead )
            {
                String sTmpBaseURL( INetURLObject::GetBaseURL() );
                INetURLObject::SetBaseURL( pMed->GetName() );

                ULONG nErrno;
                {   //Scope for SwWait-Object, to be able to execute slots
                    //outside this scope.
                    SwWait aWait( *GetDocShell(), TRUE );
                    pWrtShell->StartAllAction();
                    if ( pWrtShell->HasSelection() )
                        pWrtShell->DelRight();      // Selektionen loeschen
                    nErrno = pRdr->Read( *pRead );  // und Dokument einfuegen
                    delete pRdr;

                    INetURLObject::SetBaseURL( sTmpBaseURL );
                }

                // ggfs. alle Verzeichnisse updaten:
                if( pWrtShell->IsUpdateTOX() )
                {
                    SfxRequest aReq( FN_UPDATE_TOX, SFX_CALLMODE_SLOT, GetPool() );
                    Execute( aReq );
                    pWrtShell->SetUpdateTOX( FALSE );       // wieder zurueck setzen
                }

                pWrtShell->EndAllAction();
                if( nErrno )
                {
                    ErrorHandler::HandleError( nErrno );
                    nFound = IsError( nErrno ) ? -1 : 0;
                }
                else
                    nFound = 0;
            }
        }
    }
    else
    {
        SfxObjectShellRef xDocSh;

extern int lcl_FindDocShell( SfxObjectShellRef& xDocSh,
                            const String& rFileName, const String& rPasswd,
                            String& rFilter, INT16 nVersion,
                            SwDocShell* pDestSh );

        String sFltNm;
        int nRet = lcl_FindDocShell( xDocSh, pMed->GetName(), aEmptyStr,
                                    sFltNm, nVersion, pDocSh );
        if( nRet )
        {
            SwWait aWait( *GetDocShell(), TRUE );
            pWrtShell->StartAllAction();

            pWrtShell->EnterStdMode();          // Selektionen loeschen

            if( bCompare )
                nFound = pWrtShell->CompareDoc( *((SwDocShell*)&xDocSh)->GetDoc() );
            else
                nFound = pWrtShell->MergeDoc( *((SwDocShell*)&xDocSh)->GetDoc() );

            pWrtShell->EndAllAction();

            if (!bCompare && !nFound)
            {
                Window* pWindow = &GetEditWin();
                InfoBox(pWindow, SW_RES(MSG_NO_MERGE_ENTRY)).Execute();
            }
        }
        if( 2 == nRet && xDocSh.Is() )
            xDocSh->DoClose();
    }

    delete pMed;
    return nFound;
}


