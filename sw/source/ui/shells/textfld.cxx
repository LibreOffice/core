/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/************ *************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


#include <crsskip.hxx>
#include <hintids.hxx>  //_immer_ vor den solar-Items

#include <sfx2/lnkbase.hxx>
#include <fmtfld.hxx>
#include <vcl/msgbox.hxx>
#include <svl/itempool.hxx>
#include <unotools/useroptions.hxx>
#include <svl/whiter.hxx>
#include <svl/eitem.hxx>
#include <svl/macitem.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/request.hxx>
#include <svx/postattr.hxx>
#include <svx/hlnkitem.hxx>

#include <svx/svxdlg.hxx>
#include <sfx2/linkmgr.hxx>
#include <unotools/localedatawrapper.hxx>
#include <sfx2/dispatch.hxx>
#include <fmtinfmt.hxx>
#include <fldwrap.hxx>
#include <redline.hxx>
#include <view.hxx>
#include <wrtsh.hxx>
#include <basesh.hxx>
#include <flddat.hxx>
#include <numrule.hxx>
#include <textsh.hxx>
#include <docsh.hxx>
#include <docufld.hxx>
#include <usrfld.hxx>
#include <ddefld.hxx>
#include <expfld.hxx>
#include <fldmgr.hxx>
#include <uitool.hxx>

#include <cmdid.h>
#include <shells.hrc>

#include <sfx2/app.hxx>
#include <svx/dialogs.hrc>
#include "swabstdlg.hxx"
#include "dialog.hrc"
#include <fldui.hrc>
#include <doc.hxx>

#include <app.hrc>

#include <PostItMgr.hxx>
#include <switerator.hxx>

using namespace nsSwDocInfoSubType;

extern sal_Bool bNoInterrupt;       // in mainwn.cxx

static String& lcl_AppendRedlineStr( String& rStr, sal_uInt16 nRedlId )
{
    sal_uInt16 nResId = 0;
    switch( nRedlId )
    {
    case nsRedlineType_t::REDLINE_INSERT:   nResId = STR_REDLINE_INSERTED;      break;
    case nsRedlineType_t::REDLINE_DELETE:   nResId = STR_REDLINE_DELETED;       break;
    case nsRedlineType_t::REDLINE_FORMAT:   nResId = STR_REDLINE_FORMATED;      break;
    case nsRedlineType_t::REDLINE_TABLE:        nResId = STR_REDLINE_TABLECHG;      break;
    case nsRedlineType_t::REDLINE_FMTCOLL:  nResId = STR_REDLINE_FMTCOLLSET;    break;
    }
    if( nResId )
        rStr += SW_RESSTR( nResId );
    return rStr;
}

// STATIC DATA -----------------------------------------------------------
void SwTextShell::ExecField(SfxRequest &rReq)
{
    SwWrtShell& rSh = GetShell();
    const SfxPoolItem* pItem = 0;

    sal_uInt16 nSlot = rReq.GetSlot();
    const SfxItemSet* pArgs = rReq.GetArgs();
    if(pArgs)
        pArgs->GetItemState(GetPool().GetWhich(nSlot), sal_False, &pItem);

    Window *pMDI = &GetView().GetViewFrame()->GetWindow();
    sal_Bool bMore = sal_False;
    sal_Bool bIsText = sal_True;
    sal_uInt16 nInsertType = 0;
    sal_uInt16 nInsertSubType = 0;
    sal_uLong nInsertFormat = 0;

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
                        ::sfx2::SvBaseLink& rLink = ((SwDDEFieldType*)pFld->GetTyp())->
                                                GetBaseLink();
                        if(rLink.IsVisible())
                        {
                            SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                            SfxAbstractLinksDialog* pDlg = pFact->CreateLinksDialog( pMDI, &rSh.GetLinkManager(), sal_False, &rLink );
                            if ( pDlg )
                            {
                                pDlg->Execute();
                                delete pDlg;
                            }
                        }
                        break;
                    }
                    default:
                    {
                        SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
                        OSL_ENSURE(pFact, "SwAbstractDialogFactory fail!");

                        SfxAbstractDialog* pDlg = pFact->CreateSwFldEditDlg( GetView(),RC_DLG_SWFLDEDITDLG );
                        OSL_ENSURE(pDlg, "Dialogdiet fail!");
                        pDlg->Execute();
                        delete pDlg;
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
                sal_uInt16 nPos = rMacro.Search('.', 0);
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
                sal_Bool bRet = sal_False;
                SwFieldType* pFld = rSh.GetFldType( 0, RES_INPUTFLD );
                if( pFld && rSh.MoveFldType( pFld,
                            FN_GOTO_NEXT_INPUTFLD == nSlot ))
                {
                    rSh.ClearMark();
                    rSh.StartInputFldDlg( rSh.GetCurFld(), sal_False );
                    bRet = sal_True;
                }

                rReq.SetReturnValue( SfxBoolItem( nSlot, bRet ));
            }
            break;

        default:
            bMore = sal_True;
    }
    if(bMore)
    {
        // hier kommen die Slots mit FldMgr
        SwFldMgr aFldMgr(GetShellPtr());
        switch(nSlot)
        {
            case FN_INSERT_DBFIELD:
            {
                sal_Bool bRes = sal_False;
                if( pItem )
                {
                    sal_uLong  nFormat = 0;
                    sal_uInt16 nType = 0;
                    String aPar1 = ((SfxStringItem *)pItem)->GetValue();
                    String aPar2;
                    String aPar3;
                    sal_Int32 nCommand = 0;

                    if( SFX_ITEM_SET == pArgs->GetItemState( FN_PARAM_FIELD_TYPE,
                                                                sal_False, &pItem ))
                        nType = ((SfxUInt16Item *)pItem)->GetValue();
                    aPar1 += DB_DELIM;
                    if( SFX_ITEM_SET == pArgs->GetItemState(
                                        FN_PARAM_1, sal_False, &pItem ))
                    {
                        aPar1 += ((SfxStringItem *)pItem)->GetValue();
                    }
                    if( SFX_ITEM_SET == pArgs->GetItemState(
                                        FN_PARAM_3, sal_False, &pItem ))
                        nCommand = ((SfxInt32Item*)pItem)->GetValue();
                    aPar1 += DB_DELIM;
                    aPar1 += String::CreateFromInt32(nCommand);
                    aPar1 += DB_DELIM;
                    if( SFX_ITEM_SET == pArgs->GetItemState(
                                        FN_PARAM_2, sal_False, &pItem ))
                    {
                        aPar1 += ((SfxStringItem *)pItem)->GetValue();
                    }
                    if( SFX_ITEM_SET == pArgs->GetItemState(
                                        FN_PARAM_FIELD_CONTENT, sal_False, &pItem ))
                        aPar2 = ((SfxStringItem *)pItem)->GetValue();
                    if( SFX_ITEM_SET == pArgs->GetItemState(
                                        FN_PARAM_FIELD_FORMAT, sal_False, &pItem ))
                        nFormat = ((SfxUInt32Item *)pItem)->GetValue();
                    OSL_FAIL("Command is not yet used");
                    sal_Unicode cSeparator = ' ';
                    SwInsertFld_Data aData(nType, 0, aPar1, aPar2, nFormat, GetShellPtr(), cSeparator );
                    bRes = aFldMgr.InsertFld(aData);
                }
                rReq.SetReturnValue(SfxBoolItem( nSlot, bRes ));
            }
            break;
            case FN_INSERT_FIELD_CTRL:
            case FN_INSERT_FIELD:
            {
                sal_Bool bRes = sal_False;
                if( pItem && nSlot != FN_INSERT_FIELD_CTRL)
                {
                    sal_uLong  nFormat = 0;
                    sal_uInt16 nType = 0;
                    sal_uInt16 nSubType = 0;
                    String aPar1 = ((SfxStringItem *)pItem)->GetValue();
                    String aPar2;
                    sal_Unicode cSeparator = ' ';

                    if( SFX_ITEM_SET == pArgs->GetItemState( FN_PARAM_FIELD_TYPE,
                                                                sal_False, &pItem ))
                        nType = ((SfxUInt16Item *)pItem)->GetValue();
                    if( SFX_ITEM_SET == pArgs->GetItemState( FN_PARAM_FIELD_SUBTYPE,
                                                                sal_False, &pItem ))
                        nSubType = ((SfxUInt16Item *)pItem)->GetValue();
                    if( SFX_ITEM_SET == pArgs->GetItemState(
                                        FN_PARAM_FIELD_CONTENT, sal_False, &pItem ))
                        aPar2 = ((SfxStringItem *)pItem)->GetValue();
                    if( SFX_ITEM_SET == pArgs->GetItemState(
                                        FN_PARAM_FIELD_FORMAT, sal_False, &pItem ))
                        nFormat = ((SfxUInt32Item *)pItem)->GetValue();
                    if( SFX_ITEM_SET == pArgs->GetItemState(
                                        FN_PARAM_3, sal_False, &pItem ))
                    {
                        String sTmp = ((SfxStringItem *)pItem)->GetValue();
                        if(sTmp.Len())
                            cSeparator = sTmp.GetChar(0);
                    }
                    SwInsertFld_Data aData(nType, nSubType, aPar1, aPar2, nFormat, GetShellPtr(), cSeparator );
                    bRes = aFldMgr.InsertFld( aData );
                }
                else
                        //#i5788# prevent closing of the field dialog while a modal dialog ( Input field dialog ) is active
                        if(!GetView().GetViewFrame()->IsInModalMode())
                {
                    SfxViewFrame* pVFrame = GetView().GetViewFrame();
                    pVFrame->ToggleChildWindow(FN_INSERT_FIELD);
                    bRes = pVFrame->GetChildWindow( nSlot ) != 0;
                    Invalidate(rReq.GetSlot());
                    Invalidate(FN_INSERT_FIELD_CTRL);
                    rReq.Ignore();
                }
                rReq.SetReturnValue(SfxBoolItem( nSlot, bRes ));
            }
            break;

            case FN_INSERT_REF_FIELD:
            {
                SfxViewFrame* pVFrame = GetView().GetViewFrame();
                if (!pVFrame->HasChildWindow(FN_INSERT_FIELD))
                    pVFrame->ToggleChildWindow(FN_INSERT_FIELD);    // Dialog anzeigen

                // Flddlg auf neue TabPage umschalten
                sal_uInt16 nId = SwFldDlgWrapper::GetChildWindowId();
                SwFldDlgWrapper *pWrp = (SwFldDlgWrapper*)pVFrame->GetChildWindow(nId);
                if (pWrp)
                    pWrp->ShowPage();
                rReq.Ignore();
            }
            break;
            case FN_DELETE_COMMENT:
                if ( GetView().GetPostItMgr() &&
                     GetView().GetPostItMgr()->HasActiveSidebarWin() )
                {
                    GetView().GetPostItMgr()->DeleteActiveSidebarWin();
                }
            break;
            case FN_DELETE_ALL_NOTES:
                if ( GetView().GetPostItMgr() )
                    GetView().GetPostItMgr()->Delete();
            break;
            case FN_DELETE_NOTE_AUTHOR:
            {
                SFX_REQUEST_ARG( rReq, pNoteItem, SfxStringItem, nSlot, sal_False);
                if ( pNoteItem && GetView().GetPostItMgr() )
                    GetView().GetPostItMgr()->Delete( pNoteItem->GetValue() );
            }
            break;
            case FN_HIDE_NOTE:
                if ( GetView().GetPostItMgr() &&
                     GetView().GetPostItMgr()->HasActiveSidebarWin() )
                {
                    GetView().GetPostItMgr()->HideActiveSidebarWin();
                }
            break;
            case FN_HIDE_ALL_NOTES:
                if ( GetView().GetPostItMgr() )
                    GetView().GetPostItMgr()->Hide();
            break;
            case FN_HIDE_NOTE_AUTHOR:
            {
                SFX_REQUEST_ARG( rReq, pNoteItem, SfxStringItem, nSlot, sal_False);
                if ( pNoteItem && GetView().GetPostItMgr() )
                    GetView().GetPostItMgr()->Hide( pNoteItem->GetValue() );
            }
            break;
            case FN_POSTIT:
            {
                SwPostItField* pPostIt = (SwPostItField*)aFldMgr.GetCurFld();
                  sal_Bool bNew = !(pPostIt && pPostIt->GetTyp()->Which() == RES_POSTITFLD);
                if (bNew || GetView().GetPostItMgr()->IsAnswer())
                {
                    SvtUserOptions aUserOpt;
                    String sAuthor;
                    if( !(sAuthor = aUserOpt.GetFullName()).Len())
                        if( !(sAuthor = aUserOpt.GetID()).Len() )
                            sAuthor = String( SW_RES( STR_REDLINE_UNKNOWN_AUTHOR ));

                    // Save the current selection, it will be required later for fieldmark insertion.
                    SwPaM& rCurrPam = rSh.GetCurrentShellCursor();
                    SwPaM aSaved(*rCurrPam.GetPoint(), *rCurrPam.GetMark());
                    if( rSh.HasSelection() )
                    {
                        rSh.NormalizePam(false);
                        rSh.KillPams();
                        rSh.ClearMark();
                    }
                    SwInsertFld_Data aData(TYP_POSTITFLD, 0, sAuthor, aEmptyStr, 0);
                    aFldMgr.InsertFld(aData, &aSaved);

                    rSh.Push();
                    rSh.SwCrsrShell::Left(1, CRSR_SKIP_CHARS, sal_False);
                    pPostIt = (SwPostItField*)aFldMgr.GetCurFld();
                    rSh.Pop(sal_False); // Cursorpos restaurieren
                 }

                if (pPostIt)
                {
                    SwFieldType* pType = rSh.GetDoc()->GetFldType(RES_POSTITFLD, aEmptyStr,false);
                    SwIterator<SwFmtFld,SwFieldType> aIter( *pType );
                    SwFmtFld* pSwFmtFld = aIter.First();
                    while( pSwFmtFld )
                    {
                        if ( pSwFmtFld->GetFld() == pPostIt )
                        {
                            pSwFmtFld->Broadcast( SwFmtFldHint( 0, SWFMTFLD_FOCUS, &GetView() ) );
                            break;
                        }
                        pSwFmtFld = aIter.Next();
                    }
                }
            }
            break;
            case FN_REDLINE_COMMENT:
            {
                /*  this code can be used once we want redline comments in the margin, all other stuff can
                    then be deleted
                String sComment;
                const SwRedline *pRedline = rSh.GetCurrRedline();

                if (pRedline)
                {
                    sComment = pRedline->GetComment();
                    if ( !sComment.Len() )
                        GetView().GetDocShell()->Broadcast(SwRedlineHint(pRedline,SWREDLINE_INSERTED));
                    const_cast<SwRedline*>(pRedline)->Broadcast(SwRedlineHint(pRedline,SWREDLINE_FOCUS,&GetView()));
                }
                */

                const SwRedline *pRedline = rSh.GetCurrRedline();

                if (pRedline)
                {
                    rtl::OUString sComment = convertLineEnd(pRedline->GetComment(), GetSystemLineEnd());

                    sal_Bool bTravel = sal_False;

                    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                    OSL_ENSURE(pFact, "Dialogdiet fail!");
                    ::DialogGetRanges fnGetRange = pFact->GetDialogGetRangesFunc( RID_SVXDLG_POSTIT );
                    OSL_ENSURE(fnGetRange, "Dialogdiet fail! GetRanges()");
                    SfxItemSet aSet(GetPool(), fnGetRange());
                    aSet.Put(SvxPostItTextItem(sComment, SID_ATTR_POSTIT_TEXT));
                    aSet.Put(SvxPostItAuthorItem(pRedline->GetAuthorString(), SID_ATTR_POSTIT_AUTHOR));

                    aSet.Put( SvxPostItDateItem( GetAppLangDateTimeString(
                                pRedline->GetRedlineData().GetTimeStamp() ),
                                SID_ATTR_POSTIT_DATE ));

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

                    sal_Bool bPrev = pActRed != 0;
                    rSh.Pop(sal_False);
                    rSh.EndAction();

                    rSh.ClearMark();
                    rSh.SelNextRedline();   // Aktueller Redline wird selektiert

                    rSh.StartAction();
                    rSh.Push();
                    pActRed = rSh.SelNextRedline();
                    sal_Bool bNext = pActRed != 0;
                    rSh.Pop(sal_False); // Cursorpos restaurieren

                    if( rSh.IsCrsrPtAtEnd() )
                        rSh.SwapPam();

                    rSh.EndAction();

                    bTravel |= bNext|bPrev;

                    SvxAbstractDialogFactory* pFact2 = SvxAbstractDialogFactory::Create();
                    OSL_ENSURE(pFact2, "Dialogdiet fail!");
                    AbstractSvxPostItDialog* pDlg = pFact2->CreateSvxPostItDialog( pMDI, aSet, bTravel, sal_True );
                    OSL_ENSURE(pDlg, "Dialogdiet fail!");
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

                    rSh.SetCareWin(pDlg->GetWindow());
                    bNoInterrupt = sal_True;

                    if ( pDlg->Execute() == RET_OK )
                    {
                        const SfxItemSet* pOutSet = pDlg->GetOutputItemSet();
                        String sMsg(((const SvxPostItTextItem&)pOutSet->Get(SID_ATTR_POSTIT_TEXT)).GetValue());

                        // Kommentar einfuegen bzw aendern
                        rSh.SetRedlineComment(sMsg);
                    }

                    delete pDlg;
                    rSh.SetCareWin(NULL);
                    bNoInterrupt = sal_False;
                    rSh.ClearMark();
                    GetView().AttrChangedNotify(GetShellPtr());
                }
            }
            break;

            case FN_JAVAEDIT:
            {
                rtl::OUString aType, aText;
                sal_Bool bIsUrl=sal_False;
                sal_Bool bNew=sal_False, bUpdate=sal_False;
                SwFldMgr* pMgr = new SwFldMgr;
                if ( pItem )
                {
                    aText = ((SfxStringItem*)pItem)->GetValue();
                    SFX_REQUEST_ARG( rReq, pType, SfxStringItem, FN_PARAM_2 , sal_False );
                    SFX_REQUEST_ARG( rReq, pIsUrl, SfxBoolItem, FN_PARAM_1 , sal_False );
                    if ( pType )
                        aType = pType->GetValue();
                    if ( pIsUrl )
                        bIsUrl = pIsUrl->GetValue();

                    SwScriptField* pFld = (SwScriptField*)pMgr->GetCurFld();
                    bNew = !pFld || !(pFld->GetTyp()->Which() == RES_SCRIPTFLD);
                    bUpdate = pFld && ( bIsUrl != pFld->GetFormat() || pFld->GetPar2() != aType || pFld->GetPar1() != aText );
                }
                else
                {
                    SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
                    OSL_ENSURE(pFact, "Dialogdiet fail!");
                    AbstractJavaEditDialog* pDlg = pFact->CreateJavaEditDialog( DLG_JAVAEDIT,
                                                            pMDI, &rSh);
                    OSL_ENSURE(pDlg, "Dialogdiet fail!");
                    if ( pDlg->Execute() )
                    {
                        aType = pDlg->GetType();
                        aText = pDlg->GetText();
                        bIsUrl = pDlg->IsUrl();
                        bNew = pDlg->IsNew();
                        bUpdate = pDlg->IsUpdate();
                        rReq.AppendItem( SfxStringItem( FN_JAVAEDIT, aText ) );
                        rReq.AppendItem( SfxStringItem( FN_PARAM_2, aType ) );
                        rReq.AppendItem( SfxBoolItem( FN_PARAM_1, bIsUrl ) );
                    }

                    delete pDlg;
                }

                if( bNew )
                {
                    SwInsertFld_Data aData(TYP_SCRIPTFLD, 0, aType, aText, bIsUrl);
                    pMgr->InsertFld(aData);
                    rReq.Done();
                }
                else if( bUpdate )
                {
                    pMgr->UpdateCurFld( bIsUrl, aType, aText );
                    rSh.SetUndoNoResetModified();
                    rReq.Done();
                }
                else
                    rReq.Ignore();
            }
            break;

            case FN_INSERT_FLD_DATE    :
                nInsertType = TYP_DATEFLD;
                bIsText = sal_False;
                goto FIELD_INSERT;
            case FN_INSERT_FLD_TIME    :
                nInsertType = TYP_TIMEFLD;
                bIsText = sal_False;
                goto FIELD_INSERT;
            case FN_INSERT_FLD_PGNUMBER:
                nInsertType = TYP_PAGENUMBERFLD;
                nInsertFormat = SVX_NUM_PAGEDESC; // wie Seitenvorlage
                bIsText = sal_False;
                goto FIELD_INSERT;
            case FN_INSERT_FLD_PGCOUNT :
                nInsertType = TYP_DOCSTATFLD;
                nInsertSubType = 0;
                bIsText = sal_False;
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
                nInsertType = TYP_DOCINFOFLD;
                nInsertSubType = DI_CREATE|DI_SUB_AUTHOR;

FIELD_INSERT:
            {
                //format conversion should only be done for number formatter formats
                if(!nInsertFormat)
                    nInsertFormat = aFldMgr.GetDefaultFormat(nInsertType, bIsText, rSh.GetNumberFormatter());
                SwInsertFld_Data aData(nInsertType, nInsertSubType,
                                    aEmptyStr, aEmptyStr, nInsertFormat);
                aFldMgr.InsertFld(aData);
                rReq.Done();
            }
            break;
            default:
                OSL_FAIL("wrong dispatcher");
                return;
        }
    }
}

void SwTextShell::StateField( SfxItemSet &rSet )
{
    SwWrtShell& rSh = GetShell();
    SfxWhichIter aIter( rSet );
    const SwField* pField = 0;
    int bGetField = sal_False;
    sal_uInt16 nWhich = aIter.FirstWhich();

    while (nWhich)
    {
        switch (nWhich)
        {
            case FN_DELETE_COMMENT:
            case FN_DELETE_NOTE_AUTHOR:
            case FN_DELETE_ALL_NOTES:
            case FN_HIDE_NOTE:
            case FN_HIDE_NOTE_AUTHOR:
            case FN_HIDE_ALL_NOTES:
                {
                    SwPostItMgr* pPostItMgr = GetView().GetPostItMgr();
                    if ( !pPostItMgr )
                        rSet.InvalidateItem( nWhich );
                    else if ( !pPostItMgr->HasActiveSidebarWin() )
                    {
                        rSet.InvalidateItem( FN_DELETE_COMMENT );
                        rSet.InvalidateItem( FN_HIDE_NOTE );
                    }
                }
            break;
            case FN_EDIT_FIELD:
            {

                if( !bGetField )
                {
                    pField = rSh.GetCurFld();
                    bGetField = sal_True;
                }

                sal_uInt16 nTempWhich = pField ? pField->GetTyp()->Which() : USHRT_MAX;
                if( USHRT_MAX == nTempWhich ||
                    RES_POSTITFLD == nTempWhich ||
                    RES_SCRIPTFLD == nTempWhich ||
                    RES_AUTHORITY == nTempWhich )
                    rSet.DisableItem( nWhich );
                else if( RES_DDEFLD == nTempWhich &&
                        !((SwDDEFieldType*)pField->GetTyp())->GetBaseLink().IsVisible())
                {
                    // nested links cannot be edited
                    rSet.DisableItem( nWhich );
                }
            }
            break;
            case FN_EXECUTE_MACROFIELD:
            {
                if(!bGetField)
                {
                    pField = rSh.GetCurFld();
                    bGetField = sal_True;
                }
                if(!pField || pField->GetTyp()->Which() != RES_MACROFLD)
                    rSet.DisableItem(nWhich);
            }
            break;

            case FN_INSERT_FIELD:
            {
                SfxViewFrame* pVFrame = GetView().GetViewFrame();
                //#i5788# prevent closing of the field dialog while a modal dialog ( Input field dialog ) is active
                if(!pVFrame->IsInModalMode() &&
                        pVFrame->KnowsChildWindow(FN_INSERT_FIELD) && !pVFrame->HasChildWindow(FN_INSERT_FIELD_DATA_ONLY) )
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
                sal_Bool bCurField = sal_False;
                pField = rSh.GetCurFld();
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

void SwTextShell::InsertHyperlink(const SvxHyperlinkItem& rHlnkItem)
{
    const String& rName   = rHlnkItem.GetName();
    const String& rURL    = rHlnkItem.GetURL();
    const String& rTarget = rHlnkItem.GetTargetFrame();
    sal_uInt16 nType =  (sal_uInt16)rHlnkItem.GetInsertMode();
    nType &= ~HLINK_HTMLMODE;
    const SvxMacroTableDtor* pMacroTbl = rHlnkItem.GetMacroTbl();

    SwWrtShell& rSh = GetShell();

    if( rSh.GetSelectionType() & nsSelectionType::SEL_TXT )
    {
        rSh.StartAction();
        SfxItemSet aSet(GetPool(), RES_TXTATR_INETFMT, RES_TXTATR_INETFMT);
        rSh.GetCurAttr( aSet );

        const SfxPoolItem* pItem;
        if(SFX_ITEM_SET == aSet.GetItemState(RES_TXTATR_INETFMT, sal_False, &pItem))
        {
            // Links selektieren
            rSh.SwCrsrShell::SelectTxtAttr(RES_TXTATR_INETFMT, sal_False);
        }
        switch (nType)
        {
        case HLINK_DEFAULT:
        case HLINK_FIELD:
            {
                SwFmtINetFmt aINetFmt( rURL, rTarget );
                aINetFmt.SetName(rHlnkItem.GetIntName());
                if(pMacroTbl)
                {
                    const SvxMacro *pMacro = pMacroTbl->Get( SFX_EVENT_MOUSEOVER_OBJECT );
                    if( pMacro )
                        aINetFmt.SetMacro(SFX_EVENT_MOUSEOVER_OBJECT, *pMacro);
                    pMacro = pMacroTbl->Get( SFX_EVENT_MOUSECLICK_OBJECT );
                    if( pMacro )
                        aINetFmt.SetMacro(SFX_EVENT_MOUSECLICK_OBJECT, *pMacro);
                    pMacro = pMacroTbl->Get( SFX_EVENT_MOUSEOUT_OBJECT );
                    if( pMacro )
                        aINetFmt.SetMacro(SFX_EVENT_MOUSEOUT_OBJECT, *pMacro);
                }
                rSh.SttSelect();
                rSh.InsertURL( aINetFmt, rName, sal_True );
                rSh.EndSelect();
            }
            break;

        case HLINK_BUTTON:
            sal_Bool bSel = rSh.HasSelection();
            if(bSel)
                rSh.DelRight();
            InsertURLButton( rURL, rTarget, rName );
            rSh.EnterStdMode();
            break;
        }
        rSh.EndAction();
    }
}

IMPL_LINK( SwTextShell, RedlineNextHdl, AbstractSvxPostItDialog *, pBtn )
{
    SwWrtShell* pSh = GetShellPtr();
    AbstractSvxPostItDialog *pDlg = (AbstractSvxPostItDialog*)pBtn;

    // Kommentar einfuegen bzw aendern
    pSh->SetRedlineComment(pDlg->GetNote());

    const SwRedline *pRedline = pSh->GetCurrRedline();

    if (pRedline)
    {
        // Traveling nur bei mehr als einem Feld
        if( !pSh->IsCrsrPtAtEnd() )
            pSh->SwapPam(); // Cursor hinter den Redline stellen

        pSh->Push();
        const SwRedline *pActRed = pSh->SelNextRedline();
        pSh->Pop(pActRed != 0);

        sal_Bool bEnable = sal_False;

        if (pActRed)
        {
            pSh->StartAction();
            pSh->Push();
            bEnable = pSh->SelNextRedline() != 0;
            pSh->Pop(sal_False);
            pSh->EndAction();
        }

        pDlg->EnableTravel(bEnable, sal_True);

        if( pSh->IsCrsrPtAtEnd() )
            pSh->SwapPam();

        pRedline = pSh->GetCurrRedline();
        rtl::OUString sComment = convertLineEnd(pRedline->GetComment(), GetSystemLineEnd());

        pDlg->SetNote(sComment);
        pDlg->ShowLastAuthor( pRedline->GetAuthorString(),
                    GetAppLangDateTimeString(
                                pRedline->GetRedlineData().GetTimeStamp() ));

        String sTitle(SW_RES(STR_REDLINE_COMMENT));
        ::lcl_AppendRedlineStr( sTitle, pRedline->GetType() );

        pDlg->SetText(sTitle);
    }

    return 0;
}

IMPL_LINK( SwTextShell, RedlinePrevHdl, AbstractSvxPostItDialog *, pBtn )
{
    SwWrtShell* pSh = GetShellPtr();
    AbstractSvxPostItDialog *pDlg = (AbstractSvxPostItDialog*)pBtn;

    // Kommentar einfuegen bzw aendern
    pSh->SetRedlineComment(pDlg->GetNote());

    const SwRedline *pRedline = pSh->GetCurrRedline();

    if (pRedline)
    {
        // Traveling nur bei mehr als einem Feld
        pSh->Push();
        const SwRedline *pActRed = pSh->SelPrevRedline();
        pSh->Pop(pActRed != 0);

        sal_Bool bEnable = sal_False;

        if (pActRed)
        {
            pSh->StartAction();
            pSh->Push();
            bEnable = pSh->SelPrevRedline() != 0;
            pSh->Pop(sal_False);
            pSh->EndAction();
        }

        pDlg->EnableTravel(sal_True, bEnable);

        pRedline = pSh->GetCurrRedline();
        rtl::OUString sComment = convertLineEnd(pRedline->GetComment(), GetSystemLineEnd());

        pDlg->SetNote(sComment);
        pDlg->ShowLastAuthor(pRedline->GetAuthorString(),
                GetAppLangDateTimeString(
                                pRedline->GetRedlineData().GetTimeStamp() ));

        String sTitle(SW_RES(STR_REDLINE_COMMENT));
        ::lcl_AppendRedlineStr( sTitle, pRedline->GetType() );

        pDlg->SetText(sTitle);
    }

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
