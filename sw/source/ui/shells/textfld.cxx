/*************************************************************************
 *
 *  $RCSfile: textfld.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:47 $
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

#include "hintids.hxx"  //_immer_ vor den solar-Items
#include "uiparam.hxx"

#ifndef _SVX_POSTATTR_HXX //autogen
#include <svx/postattr.hxx>
#endif
#ifndef _SVX_HLNKITEM_HXX //autogen
#include <svx/hlnkitem.hxx>
#endif
#ifndef _SVXLINKMGR_HXX
#include <svx/linkmgr.hxx>
#endif
#ifndef _LINKDLG_HXX //autogen
#include <so3/linkdlg.hxx>
#endif
#ifndef _SVX_POSTDLG_HXX //autogen
#include <svx/postdlg.hxx>
#endif
#ifndef _SFXITEMPOOL_HXX //autogen
#include <svtools/itempool.hxx>
#endif
#ifndef _SFXINIMGR_HXX //autogen
#include <svtools/iniman.hxx>
#endif
#ifndef _OFF_APP_HXX //autogen
#include <offmgr/app.hxx>
#endif
#ifndef _SFX_INIMGR_HXX //autogen
#include <sfx2/inimgr.hxx>
#endif
#ifndef _SFX_WHITER_HXX //autogen
#include <svtools/whiter.hxx>
#endif
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _URLOBJ_HXX //autogen
#include <tools/urlobj.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _SFXMACITEM_HXX //autogen
#include <svtools/macitem.hxx>
#endif
#ifndef _SVX_ADRITEM_HXX //autogen
#include <svx/adritem.hxx>
#endif
#ifndef _LINKDLG_HXX //autogen
#include <so3/linkdlg.hxx>
#endif
#ifndef _SFXVIEWFRM_HXX //autogen
#include <sfx2/viewfrm.hxx>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif


#ifndef _FMTINFMT_HXX //autogen
#include <fmtinfmt.hxx>
#endif
#ifndef _FLDWRAP_HXX //autogen
#include <fldwrap.hxx>
#endif
#include "redline.hxx"
#include "view.hxx"
#include "wrtsh.hxx"
#include "cmdid.h"
#include "basesh.hxx"
#include "wrtsh.hxx"
#include "finder.hxx"
#include "flddat.hxx"
#include "javaedit.hxx"
#include "numrule.hxx"
#include "textsh.hxx"
#include "docufld.hxx"
#include "usrfld.hxx"
#include "ddefld.hxx"
#include "expfld.hxx"
#include "fldmgr.hxx"
#include "fldedt.hxx"
//#include "addrdlg.hxx"
#include "shells.hrc"


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
                    SvxAddressItem aAdr( pPathFinder->GetAddress() );
                    aSet.Put(SvxPostItTextItem(aEmptyStr, SID_ATTR_POSTIT_TEXT));
                    aSet.Put(SvxPostItAuthorItem(aAdr.GetShortName(), SID_ATTR_POSTIT_AUTHOR));
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
        pPostItFldMgr->UpdateCurFld( 0,
                                    SFX_INIMANAGER()->Get(SFX_KEY_USER_ID),
                                    pDlg->GetNote() );
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
        pPostItFldMgr->UpdateCurFld( 0,
                                    SFX_INIMANAGER()->Get(SFX_KEY_USER_ID),
                                    pDlg->GetNote() );
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

// -----------------------------------------------------------------------
/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.117  2000/09/18 16:06:05  willem.vandorp
    OpenOffice header added.

    Revision 1.116  2000/09/08 08:12:52  os
    Change: Set/Toggle/Has/Knows/Show/GetChildWindow

    Revision 1.115  2000/06/30 08:52:04  os
    #76541# string assertions removed

    Revision 1.114  2000/05/26 07:21:33  os
    old SW Basic API Slots removed

    Revision 1.113  2000/04/18 14:58:24  os
    UNICODE

    Revision 1.112  2000/04/11 11:32:34  os
    #74939# select inserted URLs

    Revision 1.111  2000/02/25 09:26:37  hr
    #73447#: removed temporary

    Revision 1.110  2000/02/11 14:58:18  hr
    #70473# changes for unicode ( patched by automated patchtool )

    Revision 1.109  2000/01/20 10:59:55  os
    #72115# set name of inserted hyperlink

    Revision 1.108  2000/01/18 12:10:40  pw
    #71932# Use SFX_EVENT_-constants to get a macro from a macrotable

    Revision 1.107  1999/10/27 06:37:49  os
    HyperLink dialog interface, events

    Revision 1.106  1999/10/20 16:33:19  jp
    Bug #69239#: return correct state of FN_EDIT_FIELD

    Revision 1.105  1999/09/24 14:38:32  os
    hlnkitem.hxx now in SVX

    Revision 1.104  1999/09/15 14:15:43  os
    AuthorityField(-Type), RES_AUTHORITY

    Revision 1.103  1999/07/16 14:04:08  JP
    Bug #67677#: PostIdNext-/-PrevHdl - if OkButton is enabled the field can changed


      Rev 1.102   16 Jul 1999 16:04:08   JP
   Bug #67677#: PostIdNext-/-PrevHdl - if OkButton is enabled the field can changed

      Rev 1.101   10 Jun 1999 13:16:36   JP
   have to change: no AppWin from SfxApp

      Rev 1.100   10 May 1999 13:50:12   OS
   #64780# Notizen in gesch. Bereichen

      Rev 1.99   19 Apr 1999 13:42:20   OS
   #63005# Notizen und Scripts in gesch. Bereichen

      Rev 1.98   17 Nov 1998 10:58:32   OS
   #58263# NumType durch SvxExtNumType ersetzt

      Rev 1.97   21 Oct 1998 16:19:32   OM
   #58157# Querverweise einfuegen

      Rev 1.96   09 Oct 1998 17:06:04   JP
   Bug #57741#: neue ResourceIds, optimiert

      Rev 1.95   17 Jul 1998 17:26:14   OM
   #52865# Neue HelpIDs fuer Redline-bearbeiten Dlg

      Rev 1.94   14 Jul 1998 14:19:10   OM
   #52859# Autor-Button nicht anzeigen

      Rev 1.93   09 Jul 1998 09:53:22   JP
   EmptyStr benutzen

      Rev 1.92   13 Jun 1998 16:08:48   OS
   FN_INSERT_FIELD_CONTROL ruft auch Feldbefehl-Dialog

      Rev 1.91   19 May 1998 12:45:50   OM
   SvxMacro-Umstellung

      Rev 1.90   24 Mar 1998 13:43:34   JP
   neu: Redline fuer harte Attributierung

      Rev 1.89   18 Mar 1998 18:20:10   OM
   Redlines schneller selektieren

      Rev 1.88   18 Mar 1998 18:06:26   OM
   Redlines schneller selektieren

      Rev 1.87   28 Feb 1998 15:13:54   OM
   Accept / reject changes

      Rev 1.86   26 Feb 1998 11:10:02   OM
   Redlining-Travelling anzeigen

      Rev 1.85   25 Feb 1998 16:49:46   OM
   Fixe Author- und ExtUser-Felder

      Rev 1.84   12 Feb 1998 13:22:48   OM
   Traveling zwischen Redline-Objekten

      Rev 1.83   10 Feb 1998 17:08:58   OM
   Redlining kommentieren

      Rev 1.82   27 Jan 1998 22:42:18   JP
   GetNumDepend durch GetDepends ersetzt

      Rev 1.81   23 Jan 1998 16:07:14   MA
   includes

      Rev 1.80   13 Jan 1998 16:57:54   OM
   #46549# Notiz einfuegen

      Rev 1.79   12 Jan 1998 17:09:32   OM
   Zu TYP_SETREFFLD per Basic springen

      Rev 1.78   19 Dec 1997 18:25:34   OM
   Feldbefehl-bearbeiten Dlg

      Rev 1.77   10 Dec 1997 10:08:28   OM
   Alten Feldbefehl-Dlg entfernt

      Rev 1.76   05 Dec 1997 13:16:56   OM
   #45750# DDE-Field Delimiter fuer Basic wandeln

      Rev 1.75   04 Dec 1997 08:31:50   OS
   NextFieldByType: SubType fuer SETFLD nicht setzen #45912#

      Rev 1.74   29 Nov 1997 15:52:12   MA
   includes

      Rev 1.73   24 Nov 1997 09:47:08   MA
   includes

      Rev 1.72   10 Nov 1997 15:40:04   OS
   MoveFldType mit SubType #45425#

      Rev 1.71   03 Nov 1997 13:55:48   MA
   precomp entfernt

      Rev 1.70   28 Oct 1997 15:07:16   OM
   Neuer Feldbefehl-Dialog angefangen

      Rev 1.69   20 Oct 1997 15:41:24   OS
   GotoNext/PrevInputField: Selektion aufheben #44846#

      Rev 1.68   24 Sep 1997 15:20:30   OM
   Feldumstellung

      Rev 1.67   03 Sep 1997 16:02:10   OS
   include

      Rev 1.66   01 Sep 1997 13:23:58   OS
   DLL-Umstellung

      Rev 1.65   15 Aug 1997 11:48:38   OS
   chartar/frmatr/txtatr aufgeteilt

      Rev 1.64   08 Aug 1997 17:28:48   OM
   Headerfile-Umstellung

      Rev 1.63   12 Jul 1997 12:11:02   OS
   FN_JAVAEDIT: Parameter fuer URL und ScriptType nachgetragen

      Rev 1.62   10 Jul 1997 16:07:22   OM
   Aufgeraeumt

      Rev 1.61   26 Jun 1997 09:43:40   OM
   Basic: Datenbank in Feldbefehlen ersetzen

      Rev 1.60   25 Jun 1997 15:42:26   OS
   CurField(Sub)Type: DATE und TIME mappen  #40967#

      Rev 1.59   21 Jun 1997 14:52:24   OS
   neu: FieldPrompt

      Rev 1.58   20 Jun 1997 15:17:38   OS
   GetShell()s reduziert

      Rev 1.57   18 Jun 1997 18:09:38   OM
   Adressfeld: Focus je nach Typ im Dialog setzen

      Rev 1.56   12 Jun 1997 15:34:08   JP
   Feld-Slots aus der TextShell in die BaseShell verschoben

      Rev 1.55   09 Jun 1997 11:39:46   JP
   fuer TYP_SETREFFLD gibt es keinen FeldTypen

      Rev 1.54   04 Jun 1997 14:08:06   OM
   #40431# Scriptfeld: Felbefehle bearbeiten disablen

      Rev 1.53   27 May 1997 13:36:24   OS
   neu: CurFieldType, CurFieldSubType

      Rev 1.52   12 May 1997 16:09:26   OM
   #39823# GPF bei blaettern zwischen Notizen behoben

      Rev 1.51   23 Apr 1997 15:15:44   OS
   FieldType anlegen

      Rev 1.50   08 Apr 1997 10:52:22   OM
   Fehlende Includes

      Rev 1.49   07 Apr 1997 16:45:24   OM
   HyperlinkItem recorden

      Rev 1.48   18 Mar 1997 15:31:16   OM
   Segmentiert

      Rev 1.47   18 Mar 1997 15:30:56   OM
   Datenbanktrenner wandeln

      Rev 1.46   23 Feb 1997 15:05:20   OS
   SetUserFieldValue liefert BOOL

      Rev 1.45   21 Feb 1997 16:08:38   OM
   Eingefuegten Hyperlink aufzeichnen

      Rev 1.44   13 Feb 1997 17:38:22   OM
   Script-Dlg

      Rev 1.43   11 Feb 1997 16:52:24   OM
   Eingabefeld ueber Basic ohne Dialog einfuegen

      Rev 1.42   05 Feb 1997 12:39:32   OM
   Feldbefehl Seitennummer auf Standardvorlage defaulten

      Rev 1.41   09 Dec 1996 14:43:16   OM
   SwPostItDlg durch SvxPostItDlg ersetzt

      Rev 1.40   06 Nov 1996 07:10:16   OS
   FN_GET_FIELD_VALUE optimiert

      Rev 1.39   01 Oct 1996 16:54:02   OM
   Hyperlinks editieren

      Rev 1.38   26 Sep 1996 14:40:28   OM
   Datenbankfelder ueber Basic einfuegen

      Rev 1.37   24 Sep 1996 16:42:32   OS
   neue Slots fuer FieldTypes

      Rev 1.36   24 Sep 1996 13:49:58   OM
   Neuer Datenbanktrenner

      Rev 1.35   20 Sep 1996 12:01:42   OS
   Formate fuer Seitennummer/Seitenzahl-Felder korrigiert

      Rev 1.34   19 Sep 1996 16:08:34   OS
   neue Slots zum direkten Feldeinfuegen

      Rev 1.33   19 Sep 1996 10:58:56   OM
   neue Parameter fuer GetDatabaseFieldValue

      Rev 1.32   02 Sep 1996 18:43:08   JP
   INetFeld entfernt

      Rev 1.31   30 Aug 1996 12:41:24   OS
   InputFldDlg mit Next-Button

      Rev 1.30   28 Aug 1996 15:54:56   OS
   includes

      Rev 1.29   23 Aug 1996 14:49:20   OM
   Neue Segs

      Rev 1.28   23 Aug 1996 14:48:00   OM
   Datenbank: Drag&Drop

      Rev 1.27   21 Aug 1996 13:09:24   OS
   FldMgr mit Shell anlegen -> richtige view benutzen

      Rev 1.26   20 Aug 1996 17:23:00   JP
   InsertURL: Shell wieder in StdMode schalten

      Rev 1.25   20 Aug 1996 15:39:10   JP
   InsertURL: statt FeldManager direkt an die Shell gehen

      Rev 1.24   24 Jun 1996 18:29:14   HJS
   includes

      Rev 1.23   12 Jun 1996 15:16:30   OM
   Optimiert

      Rev 1.22   12 Jun 1996 14:56:50   OM
   Target aus Hyperlinkleiste uebernehmen

      Rev 1.21   04 Jun 1996 18:43:46   JP
   neu: ab CursorPosition Next/Prev-InputField suchen und Dialog hochziehem

      Rev 1.20   31 May 1996 15:27:08   OM
   Ueberfluessigen Include entfernt

      Rev 1.19   28 May 1996 15:23:38   OM
   Neue Basic-Cmds fuer Datenbanken

------------------------------------------------------------------------*/


