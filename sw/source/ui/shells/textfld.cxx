/*************************************************************************
 *
 *  $RCSfile: textfld.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: os $ $Date: 2000-10-10 07:05:00 $
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
#include <hintids.hxx>  //_immer_ vor den solar-Items
#endif
#ifndef _UIPARAM_HXX
#include <uiparam.hxx>
#endif

#ifndef _URLOBJ_HXX //autogen
#include <tools/urlobj.hxx>
#endif
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _LINKDLG_HXX //autogen
#include <so3/linkdlg.hxx>
#endif
#ifndef _LINKDLG_HXX //autogen
#include <so3/linkdlg.hxx>
#endif
#ifndef _SFXITEMPOOL_HXX //autogen
#include <svtools/itempool.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_USEROPTIONS_HXX
#include <svtools/useroptions.hxx>
#endif
#ifndef _SFX_WHITER_HXX //autogen
#include <svtools/whiter.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _SFXMACITEM_HXX //autogen
#include <svtools/macitem.hxx>
#endif
#ifndef _SFXVIEWFRM_HXX //autogen
#include <sfx2/viewfrm.hxx>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifndef _SVX_POSTATTR_HXX //autogen
#include <svx/postattr.hxx>
#endif
#ifndef _SVX_HLNKITEM_HXX //autogen
#include <svx/hlnkitem.hxx>
#endif
#ifndef _SVXLINKMGR_HXX
#include <svx/linkmgr.hxx>
#endif
#ifndef _SVX_POSTDLG_HXX //autogen
#include <svx/postdlg.hxx>
#endif
#ifndef _SVX_ADRITEM_HXX //autogen
#include <svx/adritem.hxx>
#endif
#ifndef _OFF_APP_HXX //autogen
#include <offmgr/app.hxx>
#endif

#ifndef _FMTINFMT_HXX //autogen
#include <fmtinfmt.hxx>
#endif
#ifndef _FLDWRAP_HXX //autogen
#include <fldwrap.hxx>
#endif
#ifndef _REDLINE_HXX
#include <redline.hxx>
#endif
#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _BASESH_HXX
#include <basesh.hxx>
#endif
#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _FLDDAT_HXX
#include <flddat.hxx>
#endif
#ifndef _JAVAEDIT_HXX
#include <javaedit.hxx>
#endif
#ifndef _NUMRULE_HXX
#include <numrule.hxx>
#endif
#ifndef _TEXTSH_HXX
#include <textsh.hxx>
#endif
#ifndef _DOCUFLD_HXX
#include <docufld.hxx>
#endif
#ifndef _USRFLD_HXX
#include <usrfld.hxx>
#endif
#ifndef _DDEFLD_HXX
#include <ddefld.hxx>
#endif
#ifndef _EXPFLD_HXX
#include <expfld.hxx>
#endif
#ifndef _FLDMGR_HXX
#include <fldmgr.hxx>
#endif
#ifndef _FLDEDT_HXX
#include <fldedt.hxx>
#endif

#ifndef _CMDID_H
#include <cmdid.h>
#endif
#ifndef _SHELLS_HRC
#include <shells.hrc>
#endif


extern BOOL bNoInterrupt;       // in mainwn.cxx

String& lcl_AppendRedlineStr( String& rStr, USHORT nRedlId )
{
    USHORT nResId = 0;
    switch( nRedlId )
    {
    case REDLINE_INSERT:    nResId = STR_REDLINE_INSERTED;      break;
    case REDLINE_DELETE:    nResId = STR_REDLINE_DELETED;       break;
    case REDLINE_FORMAT:    nResId = STR_REDLINE_FORMATED;      break;
    case REDLINE_TABLE:     nResId = STR_REDLINE_TABLECHG;      break;
    case REDLINE_FMTCOLL:   nResId = STR_REDLINE_FMTCOLLSET;    break;
    }
    if( nResId )
        rStr += SW_RESSTR( nResId );
    return rStr;
}

// STATIC DATA -----------------------------------------------------------

void SwTextShell::ExecField(SfxRequest &rReq)
{
    SwWrtShell& rSh = GetShell();
    OfficeApplication* pOffApp = OFF_APP();
    const SfxPoolItem* pItem = 0;

    USHORT nSlot = rReq.GetSlot();
    const SfxItemSet* pArgs = rReq.GetArgs();
    if(pArgs)
        pArgs->GetItemState(GetPool().GetWhich(nSlot), FALSE, &pItem);

    Window *pMDI = &GetView().GetViewFrame()->GetWindow();
    BOOL bMore = FALSE;
    USHORT nInsertType = 0;
    USHORT nInsertSubType = 0;
    USHORT nInsertFormat = 0;

    switch(nSlot)
    {
        case FN_EDIT_FIELD:
        {
            SwField* pFld = rSh.GetCurFld();
            if( pFld )
            {
                switch ( pFld->GetTypeId() )
                {
                    case TYP_DDEFLD:
                    {
#if !defined(DDE_AVAILABLE)
                        return;
#endif
                        SvBaseLinksDialog aDlg( pMDI,
                                        &rSh.GetLinkManager() );
                        aDlg.SetActLink( ((SwDDEFieldType*)pFld->GetTyp())->
                                                GetBaseLink() );
                        aDlg.Execute();
                        break;
                    }
                    default:
                    {
                        SwFldEditDlg *pDlg = new SwFldEditDlg(GetView());
                        // SetCareWin geht nicht, da Feld nicht selektiert wird
                        // rSh.SetCareWin(pDlg);
                        pDlg->Execute();
                        delete pDlg;
                        //rSh.SetCareWin(NULL);
                    }
                }
            }
            break;
        }
        case FN_EXECUTE_MACROFIELD:
        {
            SwField* pFld = rSh.GetCurFld();
            if(pFld && pFld->GetTyp()->Which() == RES_MACROFLD)
            {

                const String& rMacro = ((SwMacroField*)pFld)->GetMacro();
                USHORT nPos = rMacro.Search('.', 0);
                if(nPos != STRING_NOTFOUND)
                {
                    SvxMacro aMacro( rMacro.Copy(nPos + 1), rMacro.Copy(0,nPos), STARBASIC );
                    rSh.ExecMacro(aMacro);
                }
            }
        }
        break;

        case FN_GOTO_NEXT_INPUTFLD:
        case FN_GOTO_PREV_INPUTFLD:
            {
                BOOL bRet = FALSE;
                SwFieldType* pFld = rSh.GetFldType( 0, RES_INPUTFLD );
                if( pFld && rSh.MoveFldType( pFld,
                            FN_GOTO_NEXT_INPUTFLD == nSlot ))
                {
                    rSh.ClearMark();
                    rSh.StartInputFldDlg( rSh.GetCurFld(), FALSE );
                    bRet = TRUE;
                }

                rReq.SetReturnValue( SfxBoolItem( nSlot, bRet ));
            }
            break;

        default:
            bMore = TRUE;
    }
    if(bMore)
    {
        // hier kommen die Slots mit FldMgr
        SwFldMgr aFldMgr(GetShellPtr());
        switch(nSlot)
        {
            case FN_INSERT_FIELD_CTRL:
            case FN_INSERT_FIELD:
            {
                BOOL bRes = FALSE;
                SfxViewFrame* pVFrame = GetView().GetViewFrame();
                pVFrame->ToggleChildWindow(FN_INSERT_FIELD);
                bRes = pVFrame->GetChildWindow( nSlot ) != 0;
                Invalidate(rReq.GetSlot());
                Invalidate(FN_INSERT_FIELD_CTRL);
                rReq.SetReturnValue(SfxBoolItem( nSlot, bRes ));
            }
            break;

            case FN_INSERT_REF_FIELD:
            {
                SfxViewFrame* pVFrame = GetView().GetViewFrame();
                if (!pVFrame->HasChildWindow(FN_INSERT_FIELD))
                    pVFrame->ToggleChildWindow(FN_INSERT_FIELD);    // Dialog anzeigen

                // Flddlg auf neue TabPage umschalten
                USHORT nId = SwFldDlgWrapper::GetChildWindowId();
                SwFldDlgWrapper *pWrp = (SwFldDlgWrapper*)pVFrame->GetChildWindow(nId);
                if (pWrp)
                    pWrp->ShowPage();
            }
            break;


            case FN_POSTIT:
            {
                pPostItFldMgr = new SwFldMgr;
                SwPostItField* pPostIt = (SwPostItField*)pPostItFldMgr->GetCurFld();
                BOOL bNew = !(pPostIt && pPostIt->GetTyp()->Which() == RES_POSTITFLD);
                BOOL bTravel = FALSE;
                BOOL bNext, bPrev;

                SfxItemSet aSet(GetPool(), SvxPostItDialog::GetRanges());

                if(!bNew)
                {
                    aSet.Put(SvxPostItTextItem(pPostIt->GetPar2().ConvertLineEnd(), SID_ATTR_POSTIT_TEXT));
                    aSet.Put(SvxPostItAuthorItem(pPostIt->GetPar1(), SID_ATTR_POSTIT_AUTHOR));
                    aSet.Put(SvxPostItDateItem(
                        Application::GetAppInternational().
                        GetDate(pPostIt->GetDate()),
                        SID_ATTR_POSTIT_DATE));

                    // Traveling nur bei mehr als einem Feld
                    rSh.StartAction();

                    bNext = pPostItFldMgr->GoNext();
                    if( bNext )
                        pPostItFldMgr->GoPrev();

                    if( 0 != ( bPrev = pPostItFldMgr->GoPrev() ) )
                        pPostItFldMgr->GoNext();
                    bTravel |= bNext|bPrev;

                    rSh.EndAction();
                }
                else
                {
                    SvtUserOptions aUserOpt;
                    aSet.Put(SvxPostItTextItem( aEmptyStr,
                                                    SID_ATTR_POSTIT_TEXT));
                    aSet.Put(SvxPostItAuthorItem( aUserOpt.GetID(),
                                                    SID_ATTR_POSTIT_AUTHOR));
                    aSet.Put(SvxPostItDateItem(
                            Application::GetAppInternational().
                            GetDate(Date()), SID_ATTR_POSTIT_DATE));
                }

                SvxPostItDialog *pDlg = new SvxPostItDialog( pMDI, aSet, bTravel);
                pDlg->SetReadonlyPostIt(rSh.IsReadOnlyAvailable() && rSh.HasReadonlySel());

                if (bTravel)
                {
                    pDlg->EnableTravel(bNext, bPrev);
                    pDlg->SetPrevHdl(LINK(this, SwTextShell, PostItPrevHdl));
                    pDlg->SetNextHdl(LINK(this, SwTextShell, PostItNextHdl));
                }

                if (bNew)
                    pDlg->SetText(SW_RESSTR(STR_NOTIZ_INSERT));

                bNoInterrupt = TRUE;
                if ( pDlg->Execute() == RET_OK )
                {
                    const SfxItemSet* pOutSet = pDlg->GetOutputItemSet();
                    String sMsg(((const SvxPostItTextItem&)pOutSet->Get(SID_ATTR_POSTIT_TEXT)).GetValue());
                    String sAuthor(((const SvxPostItAuthorItem&)pOutSet->Get(SID_ATTR_POSTIT_AUTHOR)).GetValue());

                    if(bNew)
                        // neues PostIt anlegen
                        pPostItFldMgr->InsertFld(TYP_POSTITFLD, 0, sAuthor, sMsg, 0);
                    else
                        // altes PostIt updaten
                        pPostItFldMgr->UpdateCurFld(0, sAuthor, sMsg);
                }

                delete pDlg;
                delete pPostItFldMgr;
                bNoInterrupt = FALSE;
                GetView().AttrChangedNotify(GetShellPtr());
            }
            break;

            case FN_REDLINE_COMMENT:
            {
                BOOL bNew = TRUE;
                String sComment;
                const SwRedline *pRedline = rSh.GetCurrRedline();

                if (pRedline)
                {
                    sComment = pRedline->GetComment();


                    BOOL bTravel = FALSE;

                    SfxItemSet aSet(GetPool(), SvxPostItDialog::GetRanges());
                    const International& rIntl = Application::GetAppInternational();

                    aSet.Put(SvxPostItTextItem(sComment.ConvertLineEnd(), SID_ATTR_POSTIT_TEXT));
                    aSet.Put(SvxPostItAuthorItem(pRedline->GetAuthorString(), SID_ATTR_POSTIT_AUTHOR));

                    const DateTime &rDT = pRedline->GetRedlineData().GetTimeStamp();

                    String sDate(rIntl.GetDate( rDT ));
                    (sDate += ' ' ) += rIntl.GetTime( rDT, FALSE, FALSE );

                    aSet.Put(SvxPostItDateItem(sDate, SID_ATTR_POSTIT_DATE));

                    // Traveling nur bei mehr als einem Feld
                    rSh.StartAction();

                    rSh.Push();
                    const SwRedline *pActRed = rSh.SelPrevRedline();

                    if (pActRed == pRedline)
                    {   // Neuer Cursor steht am Anfang des Current Redlines
                        rSh.Pop();  // Alten Cursor wegwerfen
                        rSh.Push();
                        pActRed = rSh.SelPrevRedline();
                    }

                    BOOL bPrev = pActRed != 0;
                    rSh.Pop(FALSE);
                    rSh.EndAction();

                    rSh.ClearMark();
                    rSh.SelNextRedline();   // Aktueller Redline wird selektiert

                    rSh.StartAction();
                    rSh.Push();
                    pActRed = rSh.SelNextRedline();
                    BOOL bNext = pActRed != 0;
                    rSh.Pop(FALSE); // Cursorpos restaurieren

                    if( rSh.IsCrsrPtAtEnd() )
                        rSh.SwapPam();

                    rSh.EndAction();

                    bTravel |= bNext|bPrev;

                    SvxPostItDialog *pDlg = new SvxPostItDialog( pMDI, aSet, bTravel, TRUE);
                    pDlg->HideAuthor();

                    String sTitle(SW_RES(STR_REDLINE_COMMENT));
                    ::lcl_AppendRedlineStr( sTitle, pRedline->GetType() );

                    pDlg->SetText(sTitle);

                    if (bTravel)
                    {
                        pDlg->EnableTravel(bNext, bPrev);
                        pDlg->SetPrevHdl(LINK(this, SwTextShell, RedlinePrevHdl));
                        pDlg->SetNextHdl(LINK(this, SwTextShell, RedlineNextHdl));
                    }

                    rSh.SetCareWin(pDlg);
                    bNoInterrupt = TRUE;

                    if ( pDlg->Execute() == RET_OK )
                    {
                        const SfxItemSet* pOutSet = pDlg->GetOutputItemSet();
                        String sMsg(((const SvxPostItTextItem&)pOutSet->Get(SID_ATTR_POSTIT_TEXT)).GetValue());

                        // Kommentar einfuegen bzw aendern
                        rSh.SetRedlineComment(sMsg);
                    }

                    delete pDlg;
                    rSh.SetCareWin(NULL);
                    bNoInterrupt = FALSE;
                    rSh.ClearMark();
                    GetView().AttrChangedNotify(GetShellPtr());
                }
            }
            break;

            case FN_JAVAEDIT:
            {
                SwJavaEditDialog *pDlg = new SwJavaEditDialog( pMDI, &rSh);
                pDlg->Execute();
                delete pDlg;
            }
            break;

            case FN_INSERT_FLD_DATE    :
                nInsertType = TYP_DATEFLD;
                goto FIELD_INSERT;
            case FN_INSERT_FLD_TIME    :
                nInsertType = TYP_TIMEFLD;
                goto FIELD_INSERT;
            case FN_INSERT_FLD_PGNUMBER:
                nInsertType = TYP_PAGENUMBERFLD;
                nInsertFormat = SVX_NUM_PAGEDESC; // wie Seitenvorlage
                goto FIELD_INSERT;
            case FN_INSERT_FLD_PGCOUNT :
                nInsertType = TYP_DOCSTATFLD;
                nInsertSubType = 0;
                nInsertFormat = SVX_NUM_PAGEDESC;
                goto FIELD_INSERT;
            case FN_INSERT_FLD_TOPIC   :
                nInsertType = TYP_DOCINFOFLD;
                nInsertSubType = DI_THEMA;
                goto FIELD_INSERT;
            case FN_INSERT_FLD_TITLE   :
                nInsertType = TYP_DOCINFOFLD;
                nInsertSubType = DI_TITEL;
                goto FIELD_INSERT;
            case FN_INSERT_FLD_AUTHOR  :
                nInsertType = TYP_AUTHORFLD;

FIELD_INSERT:
                aFldMgr.InsertFld(nInsertType, nInsertSubType,
                                    aEmptyStr, aEmptyStr, nInsertFormat);
            break;
            default:
                ASSERT(FALSE, falscher Dispatcher);
                return;
        }
    }
}


void SwTextShell::StateField( SfxItemSet &rSet )
{
    SwWrtShell& rSh = GetShell();
    SfxWhichIter aIter( rSet );
    const SwField* pField = 0;
    int bGetField = FALSE;
    USHORT nWhich = aIter.FirstWhich();

    while (nWhich)
    {
        switch (nWhich)
        {
            case FN_EDIT_FIELD:
            {
                if( rSh.HasSelection() )
                    rSet.DisableItem(nWhich);
                else
                {
                    if( !bGetField )
                    {
                        pField = rSh.GetCurFld();
                        bGetField = TRUE;
                    }

                    USHORT nTempWhich = pField ? pField->GetTyp()->Which() : USHRT_MAX;
                    if( USHRT_MAX == nTempWhich ||
                        RES_POSTITFLD == nTempWhich ||
                        RES_SCRIPTFLD == nTempWhich ||
                        RES_AUTHORITY == nTempWhich )
                        rSet.DisableItem( nWhich );
                }
            }
            break;
            case FN_EXECUTE_MACROFIELD:
            {
                if(!bGetField)
                {
                    pField = rSh.GetCurFld();
                    bGetField = TRUE;
                }
                if(!pField || pField->GetTyp()->Which() != RES_MACROFLD)
                    rSet.DisableItem(nWhich);
            }
            break;

            case FN_INSERT_FIELD:
            {
                SfxViewFrame* pVFrame = GetView().GetViewFrame();
                if (pVFrame->KnowsChildWindow(FN_INSERT_FIELD))
                    rSet.Put(SfxBoolItem( FN_INSERT_FIELD, pVFrame->HasChildWindow(nWhich)));
                else
                    rSet.DisableItem(FN_INSERT_FIELD);
            }
            break;
            case FN_INSERT_REF_FIELD:
            {
                SfxViewFrame* pVFrame = GetView().GetViewFrame();
                if (!pVFrame->KnowsChildWindow(FN_INSERT_FIELD))
                    rSet.DisableItem(FN_INSERT_REF_FIELD);
            }
            break;
            case FN_INSERT_FIELD_CTRL:
                rSet.Put(SfxBoolItem( nWhich, GetView().GetViewFrame()->HasChildWindow(FN_INSERT_FIELD)));
            break;
            case FN_REDLINE_COMMENT:
                if (!rSh.GetCurrRedline())
                    rSet.DisableItem(nWhich);
                break;
            case FN_POSTIT :
            case FN_JAVAEDIT :
                BOOL bCurField = FALSE;
                SwField* pField = rSh.GetCurFld();
                if(nWhich == FN_POSTIT)
                    bCurField = pField && pField->GetTyp()->Which() == RES_POSTITFLD;
                else
                    bCurField = pField && pField->GetTyp()->Which() == RES_SCRIPTFLD;

                if(!bCurField && rSh.IsReadOnlyAvailable() && rSh.HasReadonlySel() )
                    rSet.DisableItem(nWhich);
            break;
        }
        nWhich = aIter.NextWhich();
    }
}

/*---------------------------------------------------------------------------
    Beschreibung:
 ----------------------------------------------------------------------------*/


void SwTextShell::InsertHyperlink(const SvxHyperlinkItem& rHlnkItem)
{
    const String& rName   = rHlnkItem.GetName();
    const String& rURL    = rHlnkItem.GetURL();
    const String& rTarget = rHlnkItem.GetTargetFrame();
    USHORT nType =  (USHORT)rHlnkItem.GetInsertMode();
    const SvxMacroTableDtor* pMacroTbl = rHlnkItem.GetMacroTbl();

    SwWrtShell& rSh = GetShell();

    if( rSh.GetSelectionType() & SwWrtShell::SEL_TXT )
    {
        switch (nType)
        {
        case HLINK_DEFAULT:
        case HLINK_FIELD:
            {
                rSh.StartAction();
                BOOL bSel = rSh.HasSelection();
                SfxItemSet aSet(GetPool(), RES_TXTATR_INETFMT, RES_TXTATR_INETFMT);
                rSh.GetAttr( aSet );

                const SfxPoolItem* pItem;
                if(SFX_ITEM_SET == aSet.GetItemState(RES_TXTATR_INETFMT, FALSE, &pItem))
                {
                    const SwFmtINetFmt* pINetFmt = (const SwFmtINetFmt*)pItem;

                    // Links selektieren
                    rSh.SwCrsrShell::SelectTxtAttr(RES_TXTATR_INETFMT);
                }
                SwFmtINetFmt aINetFmt( rURL, rTarget );
                aINetFmt.SetName(rHlnkItem.GetIntName());
                if(pMacroTbl)
                {
                    SvxMacro *pMacro = pMacroTbl->Get( SFX_EVENT_MOUSEOVER_OBJECT );
                    if( pMacro )
                        aINetFmt.SetMacro(SFX_EVENT_MOUSEOVER_OBJECT, *pMacro);
                    pMacro = pMacroTbl->Get( SFX_EVENT_MOUSECLICK_OBJECT );
                    if( pMacro )
                        aINetFmt.SetMacro(SFX_EVENT_MOUSECLICK_OBJECT, *pMacro);
                    pMacro = pMacroTbl->Get( SFX_EVENT_MOUSEOUT_OBJECT );
                    if( pMacro )
                        aINetFmt.SetMacro(SFX_EVENT_MOUSEOUT_OBJECT, *pMacro);
                }
                rSh.InsertURL( aINetFmt, rName, TRUE );
                rSh.EndAction();
            }
            break;

        case HLINK_BUTTON:
            InsertURLButton( rURL, rTarget, rName );
            rSh.EnterStdMode();
            break;
        }
    }
}

/*---------------------------------------------------------------------------
    Beschreibung:
 ----------------------------------------------------------------------------*/


void SwTextShell::InsertDBFld(const String& sFldName)
{
    SwFldMgr aFldMgr(GetShellPtr());

    aFldMgr.InsertFld(TYP_DBFLD, 0, sFldName, aEmptyStr, 0, FALSE, TRUE);
}

/*--------------------------------------------------------------------
    Beschreibung: Traveling zwishen PostIts
 --------------------------------------------------------------------*/


IMPL_LINK( SwTextShell, PostItNextHdl, Button *, pBtn )
{
    SvxPostItDialog *pDlg = (SvxPostItDialog*)pBtn;

    if( pDlg->IsOkEnabled() )
    {
        SvtUserOptions aUserOpt;
        pPostItFldMgr->UpdateCurFld( 0, aUserOpt.GetID(), pDlg->GetNote() );
    }
    pPostItFldMgr->GoNext();
    SwPostItField* pPostIt = (SwPostItField*)pPostItFldMgr->GetCurFld();
    pDlg->SetNote(pPostIt->GetPar2().ConvertLineEnd());
    pDlg->ShowLastAuthor(pPostIt->GetPar1(), Application::GetAppInternational().GetDate(pPostIt->GetDate()));

    // Traveling nur bei mehr als einem Feld
    SwWrtShell* pSh = GetShellPtr();
    pDlg->SetReadonlyPostIt(pSh->IsReadOnlyAvailable() && pSh->HasReadonlySel());
    pSh->StartAction();

    BOOL bEnable = FALSE;
    if( pPostItFldMgr->GoNext() )
    {
        bEnable = TRUE;
        pPostItFldMgr->GoPrev();
    }
    pDlg->EnableTravel(bEnable, TRUE);

    pSh->EndAction();

    return 0;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/


IMPL_LINK( SwTextShell, PostItPrevHdl, Button *, pBtn )
{
    SvxPostItDialog *pDlg = (SvxPostItDialog*)pBtn;

    if( pDlg->IsOkEnabled() )
    {
        SvtUserOptions aUserOpt;
        pPostItFldMgr->UpdateCurFld( 0, aUserOpt.GetID(), pDlg->GetNote() );
    }
    pPostItFldMgr->GoPrev();
    SwPostItField* pPostIt = (SwPostItField*)pPostItFldMgr->GetCurFld();
    pDlg->SetNote(pPostIt->GetPar2().ConvertLineEnd());
    pDlg->ShowLastAuthor(pPostIt->GetPar1(), Application::GetAppInternational().GetDate(pPostIt->GetDate()));

    // Traveling nur bei mehr als einem Feld
    SwWrtShell* pSh = GetShellPtr();
    pDlg->SetReadonlyPostIt(pSh->IsReadOnlyAvailable() && pSh->HasReadonlySel());
    pSh->StartAction();

    BOOL bEnable = FALSE;
    if( pPostItFldMgr->GoPrev() )
    {
        bEnable = TRUE;
        pPostItFldMgr->GoNext();
    }
    pDlg->EnableTravel(TRUE, bEnable);

    pSh->EndAction();

    return 0;
}

/*--------------------------------------------------------------------
    Beschreibung: Traveling zwischen Redlines
 --------------------------------------------------------------------*/


IMPL_LINK( SwTextShell, RedlineNextHdl, Button *, pBtn )
{
    SwWrtShell* pSh = GetShellPtr();
    SvxPostItDialog *pDlg = (SvxPostItDialog*)pBtn;

    // Kommentar einfuegen bzw aendern
    pSh->SetRedlineComment(pDlg->GetNote());

    const SwRedline *pRedline = pSh->GetCurrRedline();

    BOOL bNew = TRUE;
    String sComment;

    if (pRedline)
    {
        // Traveling nur bei mehr als einem Feld
        if( !pSh->IsCrsrPtAtEnd() )
            pSh->SwapPam(); // Cursor hinter den Redline stellen

        pSh->Push();
        const SwRedline *pActRed = pSh->SelNextRedline();
        pSh->Pop(pActRed != 0);

        BOOL bEnable = FALSE;

        if (pActRed)
        {
            pSh->StartAction();
            pSh->Push();
            bEnable = pSh->SelNextRedline() != 0;
            pSh->Pop(FALSE);
            pSh->EndAction();
        }

        pDlg->EnableTravel(bEnable, TRUE);

        if( pSh->IsCrsrPtAtEnd() )
            pSh->SwapPam();

        pRedline = pSh->GetCurrRedline();
        sComment = pRedline->GetComment();

        const International& rIntl = Application::GetAppInternational();

        const DateTime &rDT = pRedline->GetRedlineData().GetTimeStamp();

        String sDate(rIntl.GetDate( rDT ));
        (sDate += ' ' ) += rIntl.GetTime( rDT, FALSE, FALSE );

        pDlg->SetNote(sComment.ConvertLineEnd());
        pDlg->ShowLastAuthor(pRedline->GetAuthorString(), sDate);

        String sTitle(SW_RES(STR_REDLINE_COMMENT));
        ::lcl_AppendRedlineStr( sTitle, pRedline->GetType() );

        pDlg->SetText(sTitle);
    }

    return 0;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/


IMPL_LINK( SwTextShell, RedlinePrevHdl, Button *, pBtn )
{
    SwWrtShell* pSh = GetShellPtr();
    SvxPostItDialog *pDlg = (SvxPostItDialog*)pBtn;

    // Kommentar einfuegen bzw aendern
    pSh->SetRedlineComment(pDlg->GetNote());

    const SwRedline *pRedline = pSh->GetCurrRedline();

    BOOL bNew = TRUE;
    String sComment;

    if (pRedline)
    {
        // Traveling nur bei mehr als einem Feld
        pSh->Push();
        const SwRedline *pActRed = pSh->SelPrevRedline();
        pSh->Pop(pActRed != 0);

        BOOL bEnable = FALSE;

        if (pActRed)
        {
            pSh->StartAction();
            pSh->Push();
            bEnable = pSh->SelPrevRedline() != 0;
            pSh->Pop(FALSE);
            pSh->EndAction();
        }

        pDlg->EnableTravel(TRUE, bEnable);

        pRedline = pSh->GetCurrRedline();
        sComment = pRedline->GetComment();

        const International& rIntl = Application::GetAppInternational();

        const DateTime &rDT = pRedline->GetRedlineData().GetTimeStamp();

        String sDate(rIntl.GetDate( rDT ));
        (sDate += ' ' ) += rIntl.GetTime( rDT, FALSE, FALSE );

        pDlg->SetNote(sComment.ConvertLineEnd());
        pDlg->ShowLastAuthor(pRedline->GetAuthorString(), sDate);

        String sTitle(SW_RES(STR_REDLINE_COMMENT));
        ::lcl_AppendRedlineStr( sTitle, pRedline->GetType() );

        pDlg->SetText(sTitle);
    }

    return 0;
}



