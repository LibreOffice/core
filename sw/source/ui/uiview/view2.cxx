/*************************************************************************
 *
 *  $RCSfile: view2.cxx,v $
 *
 *  $Revision: 1.48 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 15:52:49 $
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

#pragma hdrstop

#ifndef _COM_SUN_STAR_UTIL_SEARCHOPTIONS_HPP_
#include <com/sun/star/util/SearchOptions.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_SEARCHFLAGS_HPP_
#include <com/sun/star/util/SearchFlags.hpp>
#endif
#ifndef _COM_SUN_STAR_I18N_TRANSLITERATIONMODULES_HPP_
#include <com/sun/star/i18n/TransliterationModules.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_LOCALE_HPP_
#include <com/sun/star/lang/Locale.hpp>
#endif

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif
#ifndef _UIPARAM_HXX
#include <uiparam.hxx>
#endif

#ifndef _SVTOOLS_PASSWORDHELPER_HXX
#include <svtools/PasswordHelper.hxx>
#endif
#ifndef _SFX_PASSWD_HXX
#include <sfx2/passwd.hxx>
#endif
#ifndef _SFX_HELP_HXX
#include <sfx2/sfxhelp.hxx>
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
//CHINA001 #ifndef _SVX_ZOOM_HXX //autogen
//CHINA001 #include <svx/zoom.hxx>
//CHINA001 #endif
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
#ifndef _UNOTOOLS_TEXTSEARCH_HXX
#include <unotools/testsearch.hxx>
#endif

#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
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
//CHINA001 #ifndef _LINENUM_HXX
//CHINA001 #include <linenum.hxx>
//CHINA001 #endif
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
#ifndef _APP_HRC
#include <app.hrc>
#endif
#ifndef _FMTCLDS_HXX //autogen
#include <fmtclds.hxx>
#endif
#include <helpid.h>
#ifndef _SVTOOLS_TEMPLDLG_HXX
#include <svtools/templdlg.hxx>
#endif
#ifndef _DBCONFIG_HXX
#include <dbconfig.hxx>
#endif
#ifndef _DBMGR_HXX
#include <dbmgr.hxx>
#endif
//CHINA001 #ifndef _MAILMRGE_HXX
//CHINA001 #include "mailmrge.hxx"
//CHINA001 #endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif

#include <svx/svxdlg.hxx> //CHINA001
#include <svx/dialogs.hrc> //CHINA001
#include "swabstdlg.hxx" //CHINA001
#include "globals.hrc" //CHINA001
#include <envelp.hrc> //CHINA001

//Damit die Seitenanzeige in der Statusleiste nicht unnoetig erfolgt.
static String sLstPg;
static USHORT nPageCnt = 0;
const char __FAR_DATA sStatusDelim[] = " : ";

using namespace ::rtl;
using namespace com::sun::star;
using namespace com::sun::star::i18n;
using namespace com::sun::star::util;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::scanner;
using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
#define C2U(char) rtl::OUString::createFromAscii(char)

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
    BOOL bIgnore = FALSE;
    switch( nSlot )
    {
        case SID_CREATE_SW_DRAWVIEW:
            pWrtShell->GetDoc()->MakeDrawModel();
            break;

        case FN_LINE_NUMBERING_DLG:
        {
            //CHINA001 SwLineNumberingDlg *pDlg = new SwLineNumberingDlg(this);
            SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
            DBG_ASSERT(pFact, "Dialogdiet fail!");//CHINA001
            VclAbstractDialog* pDlg = pFact->CreateVclSwViewDialog( ResId(DLG_LINE_NUMBERING),  *this);
            DBG_ASSERT(pDlg, "Dialogdiet fail!");//CHINA001
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
            if( pArgs &&
                SFX_ITEM_SET == pArgs->GetItemState(nSlot, FALSE, &pItem ))
            {
                USHORT nOn = ((const SfxBoolItem*)pItem)->GetValue() ? REDLINE_ON : 0;
                USHORT nMode = pWrtShell->GetRedlineMode();
                pWrtShell->SetRedlineMode( (nMode & ~REDLINE_ON) | nOn);
            }
        }
        break;
        case FN_REDLINE_PROTECT :
        {
            BOOL bMode = (( pWrtShell->GetRedlineMode() & REDLINE_ON ) != 0);
            if( pArgs && SFX_ITEM_SET == pArgs->GetItemState(nSlot, FALSE, &pItem ) && ((SfxBoolItem*)pItem)->GetValue() == bMode )
                break;
            Sequence <sal_Int8> aPasswd =
                        pWrtShell->GetDoc()->GetRedlinePasswd();
            SfxPasswordDialog aPasswdDlg(&GetViewFrame()->GetWindow());
            if(!aPasswd.getLength())
                aPasswdDlg.ShowExtras(SHOWEXTRAS_CONFIRM);
            if (aPasswdDlg.Execute())
            {
                USHORT nOn = REDLINE_ON;;
                String sNewPasswd( aPasswdDlg.GetPassword() );
                Sequence <sal_Int8> aNewPasswd =
                        pWrtShell->GetDoc()->GetRedlinePasswd();
                SvPasswordHelper::GetHashPassword( aNewPasswd, sNewPasswd );
                if(!aPasswd.getLength())
                {
                    pWrtShell->GetDoc()->SetRedlinePasswd(aNewPasswd);
                }
                else if(SvPasswordHelper::CompareHashPassword(aPasswd, sNewPasswd))
                {
                    pWrtShell->GetDoc()->SetRedlinePasswd(Sequence <sal_Int8> ());
                    nOn = 0;
                }
                USHORT nMode = pWrtShell->GetRedlineMode();
                pWrtShell->SetRedlineMode( (nMode & ~REDLINE_ON) | nOn);
                rReq.AppendItem( SfxBoolItem( FN_REDLINE_PROTECT, ((nMode&REDLINE_ON)==0) ) );
            }
            else
                bIgnore = TRUE;
        }
        break;
        case FN_REDLINE_SHOW:

            if( pArgs &&
                SFX_ITEM_SET == pArgs->GetItemState(nSlot, FALSE, &pItem))
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
        {
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
                SdrView *pSdrView = pWrtShell->HasDrawView() ? pWrtShell->GetDrawView() : 0;
                if(pSdrView && pSdrView->AreObjectsMarked() &&
                    pSdrView->GetHdlList().GetFocusHdl())
                {
                    ((SdrHdlList&)pSdrView->GetHdlList()).ResetFocusHdl();
                }
                else
                {
                    if(pSdrView)
                    {
                        LeaveDrawCreate();
                        Point aPt(LONG_MIN, LONG_MIN);
                        //go out of the frame
                        pWrtShell->SelectObj(aPt, SW_LEAVE_FRAME);
                        SfxBindings& rBind = GetViewFrame()->GetBindings();
                        rBind.Invalidate( SID_ATTR_SIZE );
                    }
                    pWrtShell->EnterStdMode();
                    AttrChangedNotify(pWrtShell); // ggf Shellwechsel...
                }
            }
            else if ( GetEditWin().GetApplyTemplate() )
            {
                GetEditWin().SetApplyTemplate(SwApplyTemplate());
            }
            else if( ((SfxObjectShell*)GetDocShell())->GetInPlaceObject() &&
                        ((SfxObjectShell*)GetDocShell())->GetInPlaceObject()->GetIPClient() )
            {
                ErrCode nErr = GetDocShell()->DoInPlaceActivate( FALSE );
                if ( nErr )
                    ErrorHandler::HandleError( nErr );
            }
            else if ( GetEditWin().IsChainMode() )
            {
                GetEditWin().SetChainMode( FALSE );
            }
            else if( pWrtShell->GetFlyFrmFmt() )
            {
                const SwFrmFmt* pFmt = pWrtShell->GetFlyFrmFmt();
                if(pWrtShell->GotoFly( pFmt->GetName(), FLYCNTTYPE_FRM ))
                {
                    pWrtShell->HideCrsr();
                    pWrtShell->EnterSelFrmMode();
                }
            }
            else
            {
                SfxBoolItem aItem( SID_WIN_FULLSCREEN, FALSE );
                GetViewFrame()->GetDispatcher()->Execute( SID_WIN_FULLSCREEN, SFX_CALLMODE_RECORD, &aItem, 0L );
                bIgnore = TRUE;
            }
        }
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
                ::ItemSetToPageDesc( *pArgs, aPageDesc );
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
            GetViewImpl()->ExecuteScan( rReq );
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
        if(pArgs && SFX_ITEM_SET == pArgs->GetItemState(SID_ATTR_LANGUAGE, FALSE, &pItem))
        {
            SvxLanguageItem aLang(((SvxLanguageItem*)pItem)->GetLanguage(), RES_CHRATR_LANGUAGE);
            pWrtShell->SetDefault( aLang );
        }
        break;
        case  SID_ATTR_CHAR_CTL_LANGUAGE:
        if(pArgs && SFX_ITEM_SET == pArgs->GetItemState(RES_CHRATR_CTL_LANGUAGE, FALSE, &pItem))
        {
            pWrtShell->SetDefault( *pItem );
        }
        break;
        case  SID_ATTR_CHAR_CJK_LANGUAGE:
        if(pArgs && SFX_ITEM_SET == pArgs->GetItemState(RES_CHRATR_CJK_LANGUAGE, FALSE, &pItem))
        {
            pWrtShell->SetDefault( *pItem );
        }
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
            GetViewFrame()->ChildWindowExecute(rReq);//ToggleChildWindow(SID_GALLERY);
        break;
        case SID_VIEW_DATA_SOURCE_BROWSER:
        {
            SfxViewFrame* pVFrame = GetViewFrame();
            pVFrame->ChildWindowExecute(rReq);
            if(pVFrame->HasChildWindow(SID_BROWSER))
            {
                const SwDBData& rData = GetWrtShell().GetDBDesc();
                SW_MOD()->ShowDBObj(*this, rData, FALSE);
            }
        }
        break;
        case FN_INSERT_FIELD_DATA_ONLY :
        {
            BOOL bShow = FALSE;
            if( pArgs &&
                SFX_ITEM_SET == pArgs->GetItemState(nSlot, FALSE, &pItem ))
                bShow = ((const SfxBoolItem*)pItem)->GetValue();
            //GetViewFrame()->ShowChildWindow(nSlot, bShow && bInMailMerge);
            if((bShow && bInMailMerge) != GetViewFrame()->HasChildWindow(nSlot))
                GetViewFrame()->ToggleChildWindow(nSlot);
            //if fields have been succesfully inserted call the "real"
            //mail merge dialog
            SwWrtShell &rSh = GetWrtShell();
            if(bInMailMerge && rSh.IsAnyDatabaseFieldInDoc())
            {
                SwNewDBMgr* pNewDBMgr = rSh.GetNewDBMgr();
                if (pNewDBMgr)
                {
                    SwDBData aData;
                    aData = rSh.GetDBData();
                    rSh.EnterStdMode(); // Wechsel in Textshell erzwingen; ist fuer
                                        // das Mischen von DB-Feldern notwendig.
                    AttrChangedNotify( &rSh );
                    pNewDBMgr->SetMergeType( DBMGR_MERGE );

                    Sequence<PropertyValue> aProperties(3);
                    PropertyValue* pValues = aProperties.getArray();
                    pValues[0].Name = C2U("DataSourceName");
                    pValues[1].Name = C2U("Command");
                    pValues[2].Name = C2U("CommandType");
                    pValues[0].Value <<= aData.sDataSource;
                    pValues[1].Value <<= aData.sCommand;
                    pValues[2].Value <<= aData.nCommandType;
                    pNewDBMgr->ExecuteFormLetter(rSh, aProperties, TRUE);
                }
            }
            bInMailMerge &= bShow;
            GetViewFrame()->GetBindings().Invalidate(FN_INSERT_FIELD);
        }
        break;
        case FN_QRY_MERGE:
        {
            BOOL bUseCurrentDocument = TRUE;
            BOOL bQuery = !pArgs||SFX_ITEM_SET != pArgs->GetItemState(nSlot);
            if(bQuery)
            {
                SfxViewFrame* pFrame = GetViewFrame();
                SfxHelp::OpenHelpAgent( pFrame->GetFrame(), HID_MAIL_MERGE_SELECT );
//CHINA001                 SwMailMergeCreateFromDlg* pDlg = new SwMailMergeCreateFromDlg(
//CHINA001              &pFrame->GetWindow());
                SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
                DBG_ASSERT(pFact, "Dialogdiet fail!");//CHINA001
                AbstractMailMergeCreateFromDlg* pDlg = pFact->CreateMailMergeCreateFromDlg( ResId(DLG_MERGE_CREATE),
                                                        &pFrame->GetWindow());
                DBG_ASSERT(pDlg, "Dialogdiet fail!");//CHINA001
                if(RET_OK == pDlg->Execute())
                    bUseCurrentDocument = pDlg->IsThisDocument();
                else
                    break;
            }
            GenerateFormLetter(bUseCurrentDocument);
        }
        break;
        default:
            ASSERT(!this, falscher Dispatcher);
            return;
    }
    if(!bIgnore)
        rReq.Done();
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
                String sDisplay( rShell.GetDrawView()->GetMarkedObjectList().
                                    GetDescriptionOfMarkedObjects() );
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
                {
                    // table name + cell coordinate
                    sStr = rShell.GetTableFmt()->GetName();
                    sStr += ':';
                    sStr += rShell.GetBoxNms();
                }
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
                    if( IsShowNum(nNumLevel) && MAXLEVEL >
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
                            /* const SfxPoolItem* pItem; */
                            if(SFX_ITEM_AVAILABLE <=
                               aSet.GetItemState(RES_PARATR_NUMRULE, TRUE
                                                 /*, &pItem */ ))
                            {
                                const String& rNumStyle =
                                    ((const SfxStringItem &)
                                     aSet.Get(RES_PARATR_NUMRULE)).GetValue();
                                /* #i5116# GetItemState does not necessarily
                                   change pItem */
                                // ((const SfxStringItem*)pItem)->GetValue();
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
    const SfxPoolItem* pItem=NULL;
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
                //CHINA001 SvxZoomDialog *pDlg = 0;
                AbstractSvxZoomDialog *pDlg = 0;
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

                    //CHINA001 pDlg = new SvxZoomDialog( &GetViewFrame()->GetWindow(), aCoreSet );
                    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                    if(pFact)
                    {
                        pDlg = pFact->CreateSvxZoomDialog(&GetViewFrame()->GetWindow(), aCoreSet, ResId(RID_SVXDLG_ZOOM));
                        DBG_ASSERT(pDlg, "Dialogdiet fail!");//CHINA001
                    }

                    pDlg->SetLimits( MINZOOM, MAXZOOM );

                    if( pDlg->Execute() != RET_CANCEL )
                        pSet = pDlg->GetOutputItemSet();
                }

                if ( pSet && SFX_ITEM_SET == pSet->GetItemState(SID_ATTR_ZOOM, TRUE, &pItem))
                {
                    enum SvxZoomType eType = ((const SvxZoomItem *)pItem)->GetType();
                    SetZoom( eType, ((const SvxZoomItem *)pItem)->GetValue() );
                }
                bUp = TRUE;
                if ( pItem )
                    rReq.AppendItem( *pItem );
                rReq.Done();
                delete pDlg;
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
                GetViewFrame()->GetDispatcher()->Execute(
                    static_cast< USHORT >( nId ), SFX_CALLMODE_SYNCHRON | SFX_CALLMODE_RECORD );
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
    ::so3::SvBaseLinksDialog aSvBaseLinksDialog( &GetViewFrame()->GetWindow(),
                                    &GetWrtShell().GetLinkManager(), bWeb );
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

                SearchOptions aSearchOpt(
                                    SearchAlgorithms_ABSOLUTE, 0,
                                    sName, rtl::OUString(),
                                    CreateLocale( LANGUAGE_SYSTEM ),
                                    0,0,0,
                                    TransliterationModules_IGNORE_CASE );

                if( pWrtShell->SearchPattern( aSearchOpt, DOCPOS_START, DOCPOS_END ))
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
        SfxObjectFactory& rFact = pDocSh->GetFactory();
        const SfxFilter* pFilter = rFact.GetFilterContainer()->GetFilter4FilterName( rFilterName );
        if ( !pFilter )
        {
            pMed = new SfxMedium(rFileName, STREAM_READ, TRUE, 0, 0 );
            SfxFilterMatcher aMatcher( rFact.GetFilterContainer()->GetName() );
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
        pMed = SFX_APP()->InsertDocumentDialog(0, String::CreateFromAscii(pDocSh->GetFactory().GetShortName()), HID_INSERT_FILE );
    }
    if( !pMed )
        return -1;

    if( bInsert )
    {
        com::sun::star::uno::Reference< com::sun::star::frame::XDispatchRecorder > xRecorder =
                GetViewFrame()->GetBindings().GetRecorder();
        if ( xRecorder.is() )
        {
            SfxRequest aRequest(GetViewFrame(), SID_INSERTDOC);
            aRequest.AppendItem(SfxStringItem(SID_INSERTDOC, pMed->GetOrigURL()));
            if(pMed->GetFilter())
            aRequest.AppendItem(SfxStringItem(FN_PARAM_1, pMed->GetFilter()->GetName()));
            aRequest.Done();
        }

        SfxObjectShellRef aRef( pDocSh );

        sal_uInt32 nError = SfxObjectShell::HandleFilter( pMed, pDocSh );
        // #i16722# aborted?
        if(nError != ERRCODE_NONE)
        {
            delete pMed;
            return -1;
        }
        pDocSh->RegisterTransfer( *pMed );
        pMed->DownLoad();   // ggfs. den DownLoad anstossen
        if( aRef.Is() && 1 < aRef->GetRefCount() )  // noch gueltige Ref?
        {
            SwReader* pRdr;
            Reader *pRead = pDocSh->StartConvertFrom( *pMed, &pRdr, pWrtShell );
            if( pRead ||
                (pMed->GetFilter()->GetFilterFlags() & SFX_FILTER_STARONEFILTER) != 0 )
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
                    if( pRead )
                    {
                        nErrno = pRdr->Read( *pRead );  // und Dokument einfuegen
                        delete pRdr;
                    }
                    else
                    {
                        nErrno = pDocSh->ImportFrom( *pMed ) ? 0
                                                : ERR_SWG_READ_ERROR;
                    }

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
/* -----------------05.02.2003 12:06-----------------
 *
 * --------------------------------------------------*/
void SwView::EnableMailMerge(BOOL bEnable )
{
    bInMailMerge = bEnable;
    SfxBindings& rBind = GetViewFrame()->GetBindings();
    rBind.Invalidate(FN_INSERT_FIELD_DATA_ONLY);
    rBind.Update(FN_INSERT_FIELD_DATA_ONLY);
}
/*
*/
namespace
{
    sal_Bool lcl_NeedAdditionalDataSource( const Reference< XNameAccess >& _rDatasourceContext )
    {
        Sequence < OUString > aNames = _rDatasourceContext->getElementNames();

        return  (   !aNames.getLength()
                ||  (   ( 1 == aNames.getLength() )
                    &&  aNames.getConstArray()[0] == SW_MOD()->GetDBConfig()->GetBibliographySource().sDataSource
                    )
                );
    }
}

/* -----------------27.11.2002 12:12-----------------
 *
 * --------------------------------------------------*/
void SwView::GenerateFormLetter(BOOL bUseCurrentDocument)
{
    if(bUseCurrentDocument)
    {
        if(!GetWrtShell().IsAnyDatabaseFieldInDoc())
        {
            //check availability of data sources (except biblio source)
            Reference< XMultiServiceFactory > xMgr( ::comphelper::getProcessServiceFactory() );
            Reference<XNameAccess>  xDBContext;
            if( xMgr.is() )
            {
                Reference<XInterface> xInstance = xMgr->createInstance(
                    OUString::createFromAscii( "com.sun.star.sdb.DatabaseContext" ));
                xDBContext = Reference<XNameAccess>(xInstance, UNO_QUERY) ;
            }
            if(!xDBContext.is())
                return ;
            BOOL bCallAddressPilot = FALSE;
            if ( lcl_NeedAdditionalDataSource( xDBContext ) )
            {
                // no data sources are available - create a new one
                WarningBox aWarning(
                            &GetViewFrame()->GetWindow(),
                            SW_RES(MSG_DATA_SOURCES_UNAVAILABLE));
                // no cancel allowed
                if ( RET_OK != aWarning.Execute() )
                    return;
                bCallAddressPilot = TRUE;
            }
            else
            {
                //take an existing data source or create a new one?
//CHINA001                     SwMailMergeFieldConnectionsDlg* pConnectionsDlg = new SwMailMergeFieldConnectionsDlg(
//CHINA001              &GetViewFrame()->GetWindow());
                    SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
                    DBG_ASSERT(pFact, "Dialogdiet fail!");//CHINA001
                    AbstractMailMergeFieldConnectionsDlg* pConnectionsDlg = pFact->CreateMailMergeFieldConnectionsDlg(
                                                        ResId(DLG_MERGE_FIELD_CONNECTIONS),
                                                        &GetViewFrame()->GetWindow());
                    DBG_ASSERT(pConnectionsDlg, "Dialogdiet fail!");//CHINA001
                    if(RET_OK == pConnectionsDlg->Execute())
                        bCallAddressPilot = !pConnectionsDlg->IsUseExistingConnections();
                    else
                        return;

            }
            if(bCallAddressPilot)
            {
                GetViewFrame()->GetDispatcher()->Execute(
                                SID_ADDRESS_DATA_SOURCE, SFX_CALLMODE_SYNCHRON);
                if ( lcl_NeedAdditionalDataSource( xDBContext ) )
                    // no additional data source has been created
                    // -> assume that the user has cancelled the pilot
                    return;
            }

            //call insert fields with database field page available, only
            SfxViewFrame* pVFrame = GetViewFrame();
            //at first hide the default field dialog if currently visible
            pVFrame->SetChildWindow(FN_INSERT_FIELD, FALSE);
            //enable the status of the db field dialog - it is disabled in the status method
            //to prevent creation of the dialog without mail merge active
            EnableMailMerge();
            //then show the "Data base only" field dialog
            SfxBoolItem aOn(FN_INSERT_FIELD_DATA_ONLY, TRUE);
            pVFrame->GetDispatcher()->Execute(FN_INSERT_FIELD_DATA_ONLY,
                                                SFX_CALLMODE_SYNCHRON, &aOn, 0);
            return;
        }
        else
        {
            // check whether the
            String sSource;
            if(!GetWrtShell().IsFieldDataSourceAvailable(sSource))
            {
                WarningBox aWarning( &GetViewFrame()->GetWindow(),
                            SW_RES(MSG_MERGE_SOURCE_UNAVAILABLE));
                String sTmp(aWarning.GetMessText());
                sTmp.SearchAndReplaceAscii("%1", sSource);
                aWarning.SetMessText(sTmp);
                aWarning.Execute();
                return ;
            }
        }
        SwNewDBMgr* pNewDBMgr = GetWrtShell().GetNewDBMgr();

        SwDBData aData;
        SwWrtShell &rSh = GetWrtShell();
        aData = rSh.GetDBData();
        rSh.EnterStdMode(); // Wechsel in Textshell erzwingen; ist fuer
                            // das Mischen von DB-Feldern notwendig.
        AttrChangedNotify( &rSh );
        pNewDBMgr->SetMergeType( DBMGR_MERGE );

        if (pNewDBMgr)
        {
            Sequence<PropertyValue> aProperties(3);
            PropertyValue* pValues = aProperties.getArray();
            pValues[0].Name = C2U("DataSourceName");
            pValues[1].Name = C2U("Command");
            pValues[2].Name = C2U("CommandType");
            pValues[0].Value <<= aData.sDataSource;
            pValues[1].Value <<= aData.sCommand;
            pValues[2].Value <<= aData.nCommandType;
            pNewDBMgr->ExecuteFormLetter(GetWrtShell(), aProperties, TRUE);
        }
    }
    else
    {
        //call documents and template dialog
        SfxApplication* pSfxApp = SFX_APP();
        Window* pTopWin = pSfxApp->GetTopWindow();
        SvtDocumentTemplateDialog* pDocTemplDlg = new SvtDocumentTemplateDialog( pTopWin );
        pDocTemplDlg->SelectTemplateFolder();

        int nRet = pDocTemplDlg->Execute();
        sal_Bool bNewWin = sal_False;
        if ( nRet == RET_OK )
        {
            if ( pTopWin != pSfxApp->GetTopWindow() )
            {
                // the dialogue opens a document -> a new TopWindow appears
                pTopWin = pSfxApp->GetTopWindow();
                bNewWin = sal_True;
            }
        }

        delete pDocTemplDlg;
        if ( bNewWin )
            // after the destruction of the dialogue its parent comes to top,
            // but we want that the new document is on top
            pTopWin->ToTop();

//        return;
    }
}


