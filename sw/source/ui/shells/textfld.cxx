/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include <crsskip.hxx>
#include <hintids.hxx>

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
#include <viewopt.hxx>
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
#include <edtwin.hxx>
#include <PostItMgr.hxx>
#include <switerator.hxx>

using namespace nsSwDocInfoSubType;

extern bool bNoInterrupt;       // in mainwn.cxx

static OUString& lcl_AppendRedlineStr( OUString& rStr, sal_uInt16 nRedlId )
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

void SwTextShell::ExecField(SfxRequest &rReq)
{
    SwWrtShell& rSh = GetShell();
    const SfxPoolItem* pItem = 0;

    sal_uInt16 nSlot = rReq.GetSlot();
    const SfxItemSet* pArgs = rReq.GetArgs();
    if(pArgs)
        pArgs->GetItemState(GetPool().GetWhich(nSlot), sal_False, &pItem);

    Window *pMDI = &GetView().GetViewFrame()->GetWindow();
    bool bMore = false;
    bool bIsText = true;
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

                const OUString& rMacro = ((SwMacroField*)pFld)->GetMacro();
                sal_Int32 nPos = rMacro.indexOf('.');
                if(nPos != -1)
                {
                    SvxMacro aMacro( rMacro.copy(nPos + 1), rMacro.copy(0,nPos), STARBASIC );
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
                const bool bAddSetExpressionFlds = !( rSh.GetViewOptions()->IsReadonly() );
                if ( pFld != NULL
                     && rSh.MoveFldType(
                            pFld,
                            FN_GOTO_NEXT_INPUTFLD == nSlot,
                            USHRT_MAX,
                            bAddSetExpressionFlds ) )
                {
                    rSh.ClearMark();
                    if ( dynamic_cast<SwInputField*>(rSh.GetCurFld( true )) != NULL )
                    {
                        rSh.SttSelect();
                        rSh.SelectTxt(
                            rSh.StartOfInputFldAtPos( *(rSh.GetCrsr()->Start()) ) + 1,
                            rSh.EndOfInputFldAtPos( *(rSh.GetCrsr()->Start()) ) - 1 );
                    }
                    else
                    {
                        rSh.StartInputFldDlg( rSh.GetCurFld( true ), sal_False );
                    }
                    bRet = sal_True;
                }

                rReq.SetReturnValue( SfxBoolItem( nSlot, bRet ));
            }
            break;

        default:
            bMore = true;
    }
    if(bMore)
    {
        // Here come the slots with FldMgr.
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
                    OUString aPar1 = ((SfxStringItem *)pItem)->GetValue();
                    OUString aPar2;
                    sal_Int32 nCommand = 0;

                    if( SFX_ITEM_SET == pArgs->GetItemState( FN_PARAM_FIELD_TYPE,
                                                                sal_False, &pItem ))
                        nType = ((SfxUInt16Item *)pItem)->GetValue();
                    aPar1 += OUString(DB_DELIM);
                    if( SFX_ITEM_SET == pArgs->GetItemState(
                                        FN_PARAM_1, sal_False, &pItem ))
                    {
                        aPar1 += ((SfxStringItem *)pItem)->GetValue();
                    }
                    if( SFX_ITEM_SET == pArgs->GetItemState(
                                        FN_PARAM_3, sal_False, &pItem ))
                        nCommand = ((SfxInt32Item*)pItem)->GetValue();
                    aPar1 += OUString(DB_DELIM);
                    aPar1 += OUString::number(nCommand);
                    aPar1 += OUString(DB_DELIM);
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
                    OUString aPar1 = ((SfxStringItem *)pItem)->GetValue();
                    OUString aPar2;
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
                        OUString sTmp = ((SfxStringItem *)pItem)->GetValue();
                        if(!sTmp.isEmpty())
                            cSeparator = sTmp[0];
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
                    pVFrame->ToggleChildWindow(FN_INSERT_FIELD);    // Show dialog

                // Switch Flddlg at a new TabPage
                sal_uInt16 nId = SwFldDlgWrapper::GetChildWindowId();
                SwFldDlgWrapper *pWrp = (SwFldDlgWrapper*)pVFrame->GetChildWindow(nId);
                if (pWrp)
                    pWrp->ShowReferencePage();
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
                SwPostItField* pPostIt = dynamic_cast<SwPostItField*>(aFldMgr.GetCurFld());
                bool bNew = !(pPostIt && pPostIt->GetTyp()->Which() == RES_POSTITFLD);
                if (bNew || GetView().GetPostItMgr()->IsAnswer())
                {
                    SvtUserOptions aUserOpt;
                    OUString sAuthor;
                    if( (sAuthor = aUserOpt.GetFullName()).isEmpty())
                        if( (sAuthor = aUserOpt.GetID()).isEmpty() )
                            sAuthor = SW_RES( STR_REDLINE_UNKNOWN_AUTHOR );

                    if ( rSh.HasSelection() && !rSh.IsTableMode() )
                    {
                        rSh.KillPams();
                    }

                    // #i120513# Inserting a comment into an autocompletion crashes
                    // --> suggestion has to be removed before
                    GetView().GetEditWin().StopQuickHelp();

                    SwInsertFld_Data aData(TYP_POSTITFLD, 0, sAuthor, aEmptyOUStr, 0);
                    aFldMgr.InsertFld( aData );

                    rSh.Push();
                    rSh.SwCrsrShell::Left(1, CRSR_SKIP_CHARS, sal_False);
                    pPostIt = (SwPostItField*)aFldMgr.GetCurFld();
                    rSh.Pop(sal_False); // Restore cursor position
                 }

                if (pPostIt)
                {
                    SwFieldType* pType = rSh.GetDoc()->GetFldType(RES_POSTITFLD, aEmptyOUStr,false);
                    SwIterator<SwFmtFld,SwFieldType> aIter( *pType );
                    SwFmtFld* pSwFmtFld = aIter.First();
                    while( pSwFmtFld )
                    {
                        if ( pSwFmtFld->GetField() == pPostIt )
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
                const SwRangeRedline *pRedline = rSh.GetCurrRedline();

                if (pRedline)
                {
                    sComment = pRedline->GetComment();
                    if ( !sComment.Len() )
                        GetView().GetDocShell()->Broadcast(SwRedlineHint(pRedline,SWREDLINE_INSERTED));
                    const_cast<SwRangeRedline*>(pRedline)->Broadcast(SwRedlineHint(pRedline,SWREDLINE_FOCUS,&GetView()));
                }
                */

                const SwRangeRedline *pRedline = rSh.GetCurrRedline();

                if (pRedline)
                {
                    OUString sComment = convertLineEnd(pRedline->GetComment(), GetSystemLineEnd());

                    sal_Bool bTravel = sal_False;

                    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                    OSL_ENSURE(pFact, "Dialogdiet fail!");
                    ::DialogGetRanges fnGetRange = pFact->GetDialogGetRangesFunc();
                    OSL_ENSURE(fnGetRange, "Dialogdiet fail! GetRanges()");
                    SfxItemSet aSet(GetPool(), fnGetRange());
                    aSet.Put(SvxPostItTextItem(sComment, SID_ATTR_POSTIT_TEXT));
                    aSet.Put(SvxPostItAuthorItem(pRedline->GetAuthorString(), SID_ATTR_POSTIT_AUTHOR));

                    aSet.Put( SvxPostItDateItem( GetAppLangDateTimeString(
                                pRedline->GetRedlineData().GetTimeStamp() ),
                                SID_ATTR_POSTIT_DATE ));

                    // Traveling only if more than one field.
                    rSh.StartAction();

                    rSh.Push();
                    const SwRangeRedline *pActRed = rSh.SelPrevRedline();

                    if (pActRed == pRedline)
                    {   // New cursor is at the beginning of the current redlines.
                        rSh.Pop();  // Throw old cursor away
                        rSh.Push();
                        pActRed = rSh.SelPrevRedline();
                    }

                    sal_Bool bPrev = pActRed != 0;
                    rSh.Pop(sal_False);
                    rSh.EndAction();

                    rSh.ClearMark();
                    rSh.SelNextRedline();   // Select current redline.

                    rSh.StartAction();
                    rSh.Push();
                    pActRed = rSh.SelNextRedline();
                    sal_Bool bNext = pActRed != 0;
                    rSh.Pop(sal_False); // Restore cursor position

                    if( rSh.IsCrsrPtAtEnd() )
                        rSh.SwapPam();

                    rSh.EndAction();

                    bTravel |= bNext|bPrev;

                    SvxAbstractDialogFactory* pFact2 = SvxAbstractDialogFactory::Create();
                    OSL_ENSURE(pFact2, "Dialogdiet fail!");
                    AbstractSvxPostItDialog* pDlg = pFact2->CreateSvxPostItDialog( pMDI, aSet, bTravel );
                    OSL_ENSURE(pDlg, "Dialogdiet fail!");
                    pDlg->HideAuthor();

                    OUString sTitle(SW_RES(STR_REDLINE_COMMENT));
                    ::lcl_AppendRedlineStr( sTitle, pRedline->GetType() );

                    pDlg->SetText(sTitle);

                    if (bTravel)
                    {
                        pDlg->EnableTravel(bNext, bPrev);
                        pDlg->SetPrevHdl(LINK(this, SwTextShell, RedlinePrevHdl));
                        pDlg->SetNextHdl(LINK(this, SwTextShell, RedlineNextHdl));
                    }

                    rSh.SetCareWin(pDlg->GetWindow());
                    bNoInterrupt = true;

                    if ( pDlg->Execute() == RET_OK )
                    {
                        const SfxItemSet* pOutSet = pDlg->GetOutputItemSet();
                        OUString sMsg(((const SvxPostItTextItem&)pOutSet->Get(SID_ATTR_POSTIT_TEXT)).GetValue());

                        // Insert or change a comment
                        rSh.SetRedlineComment(sMsg);
                    }

                    delete pDlg;
                    rSh.SetCareWin(NULL);
                    bNoInterrupt = false;
                    rSh.ClearMark();
                    GetView().AttrChangedNotify(GetShellPtr());
                }
            }
            break;

            case FN_JAVAEDIT:
            {
                OUString aType, aText;
                sal_Bool bIsUrl=sal_False;
                sal_Bool bNew=sal_False;
                bool bUpdate = false;
                SwFldMgr aMgr;
                if ( pItem )
                {
                    aText = ((SfxStringItem*)pItem)->GetValue();
                    SFX_REQUEST_ARG( rReq, pType, SfxStringItem, FN_PARAM_2 , sal_False );
                    SFX_REQUEST_ARG( rReq, pIsUrl, SfxBoolItem, FN_PARAM_1 , sal_False );
                    if ( pType )
                        aType = pType->GetValue();
                    if ( pIsUrl )
                        bIsUrl = pIsUrl->GetValue();

                    SwScriptField* pFld = (SwScriptField*)aMgr.GetCurFld();
                    bNew = !pFld || !(pFld->GetTyp()->Which() == RES_SCRIPTFLD);
                    bUpdate = pFld && ( bIsUrl != pFld->GetFormat() || pFld->GetPar2() != aType || pFld->GetPar1() != aText );
                }
                else
                {
                    SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
                    OSL_ENSURE(pFact, "Dialogdiet fail!");
                    AbstractJavaEditDialog* pDlg = pFact->CreateJavaEditDialog(pMDI, &rSh);
                    OSL_ENSURE(pDlg, "Dialogdiet fail!");
                    if ( pDlg->Execute() )
                    {
                        aType = pDlg->GetScriptType();
                        aText = pDlg->GetScriptText();
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
                    aMgr.InsertFld(aData);
                    rReq.Done();
                }
                else if( bUpdate )
                {
                    aMgr.UpdateCurFld( bIsUrl, aType, aText );
                    rSh.SetUndoNoResetModified();
                    rReq.Done();
                }
                else
                    rReq.Ignore();
            }
            break;

            case FN_INSERT_FLD_DATE    :
                nInsertType = TYP_DATEFLD;
                bIsText = false;
                goto FIELD_INSERT;
            case FN_INSERT_FLD_TIME    :
                nInsertType = TYP_TIMEFLD;
                bIsText = false;
                goto FIELD_INSERT;
            case FN_INSERT_FLD_PGNUMBER:
                nInsertType = TYP_PAGENUMBERFLD;
                nInsertFormat = SVX_NUM_PAGEDESC; // Like page template
                bIsText = false;
                goto FIELD_INSERT;
            case FN_INSERT_FLD_PGCOUNT :
                nInsertType = TYP_DOCSTATFLD;
                nInsertSubType = 0;
                bIsText = false;
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
                                    aEmptyOUStr, aEmptyOUStr, nInsertFormat);
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
                if ( rSh.CrsrInsideInputFld() )
                {
                    rSet.DisableItem(nWhich);
                }
                else
                {
                    SfxViewFrame* pVFrame = GetView().GetViewFrame();
                    //#i5788# prevent closing of the field dialog while a modal dialog ( Input field dialog ) is active
                    if(!pVFrame->IsInModalMode() &&
                        pVFrame->KnowsChildWindow(FN_INSERT_FIELD) && !pVFrame->HasChildWindow(FN_INSERT_FIELD_DATA_ONLY) )
                        rSet.Put(SfxBoolItem( FN_INSERT_FIELD, pVFrame->HasChildWindow(nWhich)));
                    else
                        rSet.DisableItem(FN_INSERT_FIELD);
                }
            }
            break;

        case FN_INSERT_REF_FIELD:
            {
                SfxViewFrame* pVFrame = GetView().GetViewFrame();
                if ( !pVFrame->KnowsChildWindow(FN_INSERT_FIELD)
                     || rSh.CrsrInsideInputFld() )
                {
                    rSet.DisableItem(FN_INSERT_REF_FIELD);
                }
            }
            break;

        case FN_INSERT_FIELD_CTRL:
                if ( rSh.CrsrInsideInputFld() )
                {
                    rSet.DisableItem(nWhich);
                }
                else
                {
                    rSet.Put(SfxBoolItem( nWhich, GetView().GetViewFrame()->HasChildWindow(FN_INSERT_FIELD)));
                }
            break;

        case FN_REDLINE_COMMENT:
            if (!rSh.GetCurrRedline())
                rSet.DisableItem(nWhich);
            break;

        case FN_POSTIT :
        case FN_JAVAEDIT :
            {
                sal_Bool bCurField = sal_False;
                pField = rSh.GetCurFld();
                if(nWhich == FN_POSTIT)
                    bCurField = pField && pField->GetTyp()->Which() == RES_POSTITFLD;
                else
                    bCurField = pField && pField->GetTyp()->Which() == RES_SCRIPTFLD;

                if( !bCurField && rSh.IsReadOnlyAvailable() && rSh.HasReadonlySel() )
                {
                    rSet.DisableItem(nWhich);
                }
                else if ( rSh.CrsrInsideInputFld() )
                {
                    rSet.DisableItem(nWhich);
                }
            }

            break;

        case FN_INSERT_FLD_AUTHOR:
        case FN_INSERT_FLD_DATE:
        case FN_INSERT_FLD_PGCOUNT:
        case FN_INSERT_FLD_PGNUMBER:
        case FN_INSERT_FLD_TIME:
        case FN_INSERT_FLD_TITLE:
        case FN_INSERT_FLD_TOPIC:
        case FN_INSERT_DBFIELD:
            if ( rSh.CrsrInsideInputFld() )
            {
                rSet.DisableItem(nWhich);
            }
            break;

        }
        nWhich = aIter.NextWhich();
    }
}

void SwTextShell::InsertHyperlink(const SvxHyperlinkItem& rHlnkItem)
{
    const OUString& rName   = rHlnkItem.GetName();
    const OUString& rURL    = rHlnkItem.GetURL();
    const OUString& rTarget = rHlnkItem.GetTargetFrame();
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
            // Select links
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

    // Insert or change a comment.
    pSh->SetRedlineComment(pDlg->GetNote());

    const SwRangeRedline *pRedline = pSh->GetCurrRedline();

    if (pRedline)
    {
        // Traveling only if more than one field.
        if( !pSh->IsCrsrPtAtEnd() )
            pSh->SwapPam(); // Move the cursor behind the Redline.

        pSh->Push();
        const SwRangeRedline *pActRed = pSh->SelNextRedline();
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
        OUString sComment = convertLineEnd(pRedline->GetComment(), GetSystemLineEnd());

        pDlg->SetNote(sComment);
        pDlg->ShowLastAuthor( pRedline->GetAuthorString(),
                    GetAppLangDateTimeString(
                                pRedline->GetRedlineData().GetTimeStamp() ));

        OUString sTitle(SW_RES(STR_REDLINE_COMMENT));
        ::lcl_AppendRedlineStr( sTitle, pRedline->GetType() );

        pDlg->SetText(sTitle);
    }

    return 0;
}

IMPL_LINK( SwTextShell, RedlinePrevHdl, AbstractSvxPostItDialog *, pBtn )
{
    SwWrtShell* pSh = GetShellPtr();
    AbstractSvxPostItDialog *pDlg = (AbstractSvxPostItDialog*)pBtn;

    // Insert or change a comment.
    pSh->SetRedlineComment(pDlg->GetNote());

    const SwRangeRedline *pRedline = pSh->GetCurrRedline();

    if (pRedline)
    {
        // Traveling only if more than one field.
        pSh->Push();
        const SwRangeRedline *pActRed = pSh->SelPrevRedline();
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
        OUString sComment = convertLineEnd(pRedline->GetComment(), GetSystemLineEnd());

        pDlg->SetNote(sComment);
        pDlg->ShowLastAuthor(pRedline->GetAuthorString(),
                GetAppLangDateTimeString(
                                pRedline->GetRedlineData().GetTimeStamp() ));

        OUString sTitle(SW_RES(STR_REDLINE_COMMENT));
        ::lcl_AppendRedlineStr( sTitle, pRedline->GetType() );

        pDlg->SetText(sTitle);
    }

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
