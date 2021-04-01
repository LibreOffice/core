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

#include <AnnotationWin.hxx>
#include <comphelper/lok.hxx>
#include <hintids.hxx>
#include <IDocumentFieldsAccess.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/lnkbase.hxx>
#include <txtfld.hxx>
#include <svl/itempool.hxx>
#include <tools/lineend.hxx>
#include <svl/whiter.hxx>
#include <svl/eitem.hxx>
#include <svl/macitem.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/request.hxx>
#include <svx/postattr.hxx>
#include <svx/hlnkitem.hxx>
#include <svx/svxdlg.hxx>
#include <osl/diagnose.h>
#include <fmtinfmt.hxx>
#include <fldwrap.hxx>
#include <redline.hxx>
#include <view.hxx>
#include <viewopt.hxx>
#include <wrtsh.hxx>
#include <textsh.hxx>
#include <docufld.hxx>
#include <ddefld.hxx>
#include <fldmgr.hxx>
#include <uitool.hxx>
#include <cmdid.h>
#include <strings.hrc>
#include <sfx2/event.hxx>
#include <swabstdlg.hxx>
#include <doc.hxx>
#include <PostItMgr.hxx>
#include <swmodule.hxx>
#include <xmloff/odffields.hxx>
#include <IDocumentContentOperations.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <IDocumentUndoRedo.hxx>
#include <svl/zforlist.hxx>
#include <svl/zformat.hxx>
#include <IMark.hxx>
#include <officecfg/Office/Compatibility.hxx>
#include <ndtxt.hxx>


using namespace nsSwDocInfoSubType;

static OUString lcl_BuildTitleWithRedline( const SwRangeRedline *pRedline )
{
    const OUString sTitle(SwResId(STR_REDLINE_COMMENT));

    const char* pResId = nullptr;
    switch( pRedline->GetType() )
    {
        case RedlineType::Insert:
            pResId = STR_REDLINE_INSERTED;
            break;
        case RedlineType::Delete:
            pResId = STR_REDLINE_DELETED;
            break;
        case RedlineType::Format:
        case RedlineType::ParagraphFormat:
            pResId = STR_REDLINE_FORMATTED;
            break;
        case RedlineType::Table:
            pResId = STR_REDLINE_TABLECHG;
            break;
        case RedlineType::FmtColl:
            pResId = STR_REDLINE_FMTCOLLSET;
            break;
        default:
            return sTitle;
    }

    return sTitle + SwResId(pResId);
}

void SwTextShell::ExecField(SfxRequest &rReq)
{
    SwWrtShell& rSh = GetShell();
    const SfxPoolItem* pItem = nullptr;

    sal_uInt16 nSlot = rReq.GetSlot();
    const SfxItemSet* pArgs = rReq.GetArgs();
    if(pArgs)
        pArgs->GetItemState(GetPool().GetWhich(nSlot), false, &pItem);

    bool bMore = false;
    bool bIsText = true;
    SwFieldTypesEnum nInsertType = SwFieldTypesEnum::Date;
    sal_uInt16 nInsertSubType = 0;
    sal_uInt32 nInsertFormat = 0;

    switch(nSlot)
    {
        case FN_EDIT_FIELD:
        {
            SwField* pField = rSh.GetCurField(true);
            if( pField )
            {
                switch ( pField->GetTypeId() )
                {
                    case SwFieldTypesEnum::DDE:
                    {
                        ::sfx2::SvBaseLink& rLink = static_cast<SwDDEFieldType*>(pField->GetTyp())->
                                                GetBaseLink();
                        if(rLink.IsVisible())
                        {
                            SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                            ScopedVclPtr<SfxAbstractLinksDialog> pDlg(pFact->CreateLinksDialog(GetView().GetFrameWeld(), &rSh.GetLinkManager(), false, &rLink));
                            pDlg->Execute();
                        }
                        break;
                    }
                    default:
                    {
                        SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
                        ScopedVclPtr<SfxAbstractDialog> pDlg(pFact->CreateSwFieldEditDlg( GetView() ));
                        pDlg->Execute();
                    }
                }
            }
            break;
        }
        case FN_EXECUTE_MACROFIELD:
        {
            SwField* pField = rSh.GetCurField();
            if(pField && pField->GetTyp()->Which() == SwFieldIds::Macro)
            {

                const OUString& rMacro = static_cast<SwMacroField*>(pField)->GetMacro();
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
                bool bRet = false;
                SwFieldType* pField = rSh.GetFieldType( 0, SwFieldIds::Input );
                const bool bAddSetExpressionFields = !( rSh.GetViewOptions()->IsReadonly() );
                if ( pField != nullptr
                     && rSh.MoveFieldType(
                            pField,
                            FN_GOTO_NEXT_INPUTFLD == nSlot,
                            SwFieldIds::Unknown,
                            bAddSetExpressionFields ) )
                {
                    rSh.ClearMark();
                    if (!rSh.IsMultiSelection()
                        && (nullptr != dynamic_cast<const SwTextInputField*>(
                               SwCursorShell::GetTextFieldAtCursor(rSh.GetCursor(), true))))
                    {
                        rSh.SttSelect();
                        rSh.SelectText(
                            SwCursorShell::StartOfInputFieldAtPos( *(rSh.GetCursor()->Start()) ) + 1,
                            SwCursorShell::EndOfInputFieldAtPos( *(rSh.GetCursor()->Start()) ) - 1 );
                    }
                    else
                    {
                        rSh.StartInputFieldDlg(rSh.GetCurField(true), false, false, GetView().GetFrameWeld());
                    }
                    bRet = true;
                }

                rReq.SetReturnValue( SfxBoolItem( nSlot, bRet ));
            }
            break;

        default:
            bMore = true;
    }
    if(!bMore)
        return;

    // Here come the slots with FieldMgr.
    SwFieldMgr aFieldMgr(GetShellPtr());
    switch(nSlot)
    {
        case FN_INSERT_DBFIELD:
        {
            bool bRes = false;
            if( pItem )
            {
                sal_uInt32 nFormat = 0;
                SwFieldTypesEnum nType = SwFieldTypesEnum::Date;
                OUString aPar1 = static_cast<const SfxStringItem *>(pItem)->GetValue();
                OUString aPar2;
                sal_Int32 nCommand = 0;

                if( SfxItemState::SET == pArgs->GetItemState( FN_PARAM_FIELD_TYPE,
                                                            false, &pItem ))
                    nType = static_cast<SwFieldTypesEnum>(static_cast<const SfxUInt16Item *>(pItem)->GetValue());
                aPar1 += OUStringChar(DB_DELIM);
                if( SfxItemState::SET == pArgs->GetItemState(
                                    FN_PARAM_1, false, &pItem ))
                {
                    aPar1 += static_cast<const SfxStringItem *>(pItem)->GetValue();
                }
                if( SfxItemState::SET == pArgs->GetItemState(
                                    FN_PARAM_3, false, &pItem ))
                    nCommand = static_cast<const SfxInt32Item*>(pItem)->GetValue();
                aPar1 += OUStringChar(DB_DELIM)
                    + OUString::number(nCommand)
                    + OUStringChar(DB_DELIM);
                if( SfxItemState::SET == pArgs->GetItemState(
                                    FN_PARAM_2, false, &pItem ))
                {
                    aPar1 += static_cast<const SfxStringItem *>(pItem)->GetValue();
                }
                if( SfxItemState::SET == pArgs->GetItemState(
                                    FN_PARAM_FIELD_CONTENT, false, &pItem ))
                    aPar2 = static_cast<const SfxStringItem *>(pItem)->GetValue();
                if( SfxItemState::SET == pArgs->GetItemState(
                                    FN_PARAM_FIELD_FORMAT, false, &pItem ))
                    nFormat = static_cast<const SfxUInt32Item *>(pItem)->GetValue();
                OSL_FAIL("Command is not yet used");
                SwInsertField_Data aData(nType, 0, aPar1, aPar2, nFormat, GetShellPtr(), ' '/*separator*/ );
                bRes = aFieldMgr.InsertField(aData);
            }
            rReq.SetReturnValue(SfxBoolItem( nSlot, bRes ));
        }
        break;
        case FN_INSERT_FIELD_CTRL:
        case FN_INSERT_FIELD:
        {
            bool bRes = false;
            if( pItem && nSlot != FN_INSERT_FIELD_CTRL)
            {
                sal_uInt32 nFormat = 0;
                SwFieldTypesEnum nType = SwFieldTypesEnum::Date;
                sal_uInt16 nSubType = 0;
                OUString aPar1 = static_cast<const SfxStringItem *>(pItem)->GetValue();
                OUString aPar2;
                sal_Unicode cSeparator = ' ';

                if( SfxItemState::SET == pArgs->GetItemState( FN_PARAM_FIELD_TYPE,
                                                            false, &pItem ))
                    nType = static_cast<SwFieldTypesEnum>(static_cast<const SfxUInt16Item *>(pItem)->GetValue());
                if( SfxItemState::SET == pArgs->GetItemState( FN_PARAM_FIELD_SUBTYPE,
                                                            false, &pItem ))
                    nSubType = static_cast<const SfxUInt16Item *>(pItem)->GetValue();
                if( SfxItemState::SET == pArgs->GetItemState(
                                    FN_PARAM_FIELD_CONTENT, false, &pItem ))
                    aPar2 = static_cast<const SfxStringItem *>(pItem)->GetValue();
                if( SfxItemState::SET == pArgs->GetItemState(
                                    FN_PARAM_FIELD_FORMAT, false, &pItem ))
                    nFormat = static_cast<const SfxUInt32Item *>(pItem)->GetValue();
                if( SfxItemState::SET == pArgs->GetItemState(
                                    FN_PARAM_3, false, &pItem ))
                {
                    OUString sTmp = static_cast<const SfxStringItem *>(pItem)->GetValue();
                    if(!sTmp.isEmpty())
                        cSeparator = sTmp[0];
                }
                SwInsertField_Data aData(nType, nSubType, aPar1, aPar2, nFormat, GetShellPtr(), cSeparator );
                bRes = aFieldMgr.InsertField( aData );
            }
            else
            {
                //#i5788# prevent closing of the field dialog while a modal dialog ( Input field dialog ) is active
                if(!GetView().GetViewFrame()->IsInModalMode())
                {
                    SfxViewFrame* pVFrame = GetView().GetViewFrame();
                    pVFrame->ToggleChildWindow(FN_INSERT_FIELD);
                    bRes = pVFrame->GetChildWindow( nSlot ) != nullptr;
                    Invalidate(rReq.GetSlot());
                    Invalidate(FN_INSERT_FIELD_CTRL);
                    rReq.Ignore();
                }
            }
            rReq.SetReturnValue(SfxBoolItem( nSlot, bRes ));
        }
        break;

        case FN_INSERT_REF_FIELD:
        {
            SfxViewFrame* pVFrame = GetView().GetViewFrame();
            if (!pVFrame->HasChildWindow(FN_INSERT_FIELD))
                pVFrame->ToggleChildWindow(FN_INSERT_FIELD);    // Show dialog

            // Switch Fielddlg at a new TabPage
            sal_uInt16 nId = SwFieldDlgWrapper::GetChildWindowId();
            SwFieldDlgWrapper *pWrp = static_cast<SwFieldDlgWrapper*>(pVFrame->GetChildWindow(nId));
            if (pWrp)
                pWrp->ShowReferencePage();
            rReq.Ignore();
        }
        break;
        case FN_DELETE_COMMENT:
        {
            const SvxPostItIdItem* pIdItem = rReq.GetArg<SvxPostItIdItem>(SID_ATTR_POSTIT_ID);
            if (pIdItem && !pIdItem->GetValue().isEmpty() && GetView().GetPostItMgr())
            {
                GetView().GetPostItMgr()->Delete(pIdItem->GetValue().toUInt32());
            }
            else if ( GetView().GetPostItMgr() &&
                      GetView().GetPostItMgr()->HasActiveSidebarWin() )
            {
                GetView().GetPostItMgr()->DeleteActiveSidebarWin();
            }
            break;
        }
        case FN_DELETE_COMMENT_THREAD:
        {
            const SvxPostItIdItem* pIdItem = rReq.GetArg<SvxPostItIdItem>(SID_ATTR_POSTIT_ID);
            if (pIdItem && !pIdItem->GetValue().isEmpty() && GetView().GetPostItMgr())
            {
                GetView().GetPostItMgr()->DeleteCommentThread(pIdItem->GetValue().toUInt32());
            }
            else if ( GetView().GetPostItMgr() &&
                        GetView().GetPostItMgr()->HasActiveSidebarWin() )
            {
                GetView().GetPostItMgr()->DeleteActiveSidebarWin();
            }
            break;
        }
        case FN_RESOLVE_NOTE:
        {
            const SvxPostItIdItem* pIdItem = rReq.GetArg<SvxPostItIdItem>(SID_ATTR_POSTIT_ID);
            if (pIdItem && !pIdItem->GetValue().isEmpty() && GetView().GetPostItMgr())
            {
                GetView().GetPostItMgr()->ToggleResolved(pIdItem->GetValue().toUInt32());
            }
            break;
        }
        case FN_RESOLVE_NOTE_THREAD:
        {
            const SvxPostItIdItem* pIdItem = rReq.GetArg<SvxPostItIdItem>(SID_ATTR_POSTIT_ID);
            if (pIdItem && !pIdItem->GetValue().isEmpty() && GetView().GetPostItMgr())
            {
                GetView().GetPostItMgr()->ToggleResolvedForThread(pIdItem->GetValue().toUInt32());
            }
            break;
        }
        case FN_DELETE_ALL_NOTES:
            if ( GetView().GetPostItMgr() )
                GetView().GetPostItMgr()->Delete();
        break;
        case FN_FORMAT_ALL_NOTES:
        {
            SwPostItMgr* pPostItMgr = GetView().GetPostItMgr();
            if (pPostItMgr)
                pPostItMgr->ExecuteFormatAllDialog(GetView());
        }
        break;
        case FN_DELETE_NOTE_AUTHOR:
        {
            const SfxStringItem* pNoteItem = rReq.GetArg<SfxStringItem>(nSlot);
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
            const SfxStringItem* pNoteItem = rReq.GetArg<SfxStringItem>(nSlot);
            if ( pNoteItem && GetView().GetPostItMgr() )
                GetView().GetPostItMgr()->Hide( pNoteItem->GetValue() );
        }
        break;
        case FN_REPLY:
        {
            const SvxPostItIdItem* pIdItem = rReq.GetArg<SvxPostItIdItem>(SID_ATTR_POSTIT_ID);
            if (pIdItem && !pIdItem->GetValue().isEmpty())
            {
                SwFieldType* pType = rSh.GetDoc()->getIDocumentFieldsAccess().GetFieldType(SwFieldIds::Postit, OUString(), false);
                if(pType->FindFormatForPostItId(pIdItem->GetValue().toUInt32()))
                {
                    auto pMgr = GetView().GetPostItMgr();
                    auto pWin = pMgr->GetAnnotationWin(pIdItem->GetValue().toUInt32());
                    if(pWin)
                    {
                        OUString sText;
                        if(const auto pTextItem = rReq.GetArg<SvxPostItTextItem>(SID_ATTR_POSTIT_TEXT))
                            sText = pTextItem->GetValue();
                        pMgr->RegisterAnswerText(sText);
                        pWin->ExecuteCommand(nSlot);
                    }
                }
            }
        }
        break;
        case FN_POSTIT:
        {
            rSh.InsertPostIt(aFieldMgr, rReq);
        }
        break;
        case SID_EDIT_POSTIT:
        {
            const SvxPostItIdItem* pIdItem = rReq.GetArg<SvxPostItIdItem>(SID_ATTR_POSTIT_ID);
            if (pIdItem && !pIdItem->GetValue().isEmpty())
            {
                const SvxPostItTextItem* pTextItem = rReq.GetArg<SvxPostItTextItem>(SID_ATTR_POSTIT_TEXT);
                OUString sText;
                if ( pTextItem )
                    sText = pTextItem->GetValue();

                sw::annotation::SwAnnotationWin* pAnnotationWin = GetView().GetPostItMgr()->GetAnnotationWin(pIdItem->GetValue().toUInt32());
                if (pAnnotationWin)
                {
                    pAnnotationWin->UpdateText(sText);

                    // explicit state update to get the Undo state right
                    GetView().AttrChangedNotify(nullptr);
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
            SwDoc *pDoc = rSh.GetDoc();
            // If index is specified, goto and select the appropriate redline
            if (pArgs && pArgs->GetItemState(nSlot, false, &pItem) == SfxItemState::SET)
            {
                const sal_uInt32 nChangeId = static_cast<const SfxUInt32Item*>(pItem)->GetValue();
                const SwRedlineTable& rRedlineTable = pDoc->getIDocumentRedlineAccess().GetRedlineTable();
                for (SwRedlineTable::size_type nRedline = 0; nRedline < rRedlineTable.size(); ++nRedline)
                {
                    if (nChangeId == rRedlineTable[nRedline]->GetId())
                        pRedline = rSh.GotoRedline(nRedline, true);
                }
            }

            OUString sCommentText;
            const SfxStringItem* pTextItem = rReq.GetArg<SvxPostItTextItem>(SID_ATTR_POSTIT_TEXT);
            if (pTextItem)
                sCommentText = pTextItem->GetValue();

            if (pRedline)
            {
                // In case of LOK and comment text is already provided, skip
                // dialog creation and just change the redline comment directly
                if (comphelper::LibreOfficeKit::isActive() && !sCommentText.isEmpty())
                {
                    rSh.SetRedlineComment(sCommentText);
                    GetView().AttrChangedNotify(nullptr);
                    MaybeNotifyRedlineModification(const_cast<SwRangeRedline&>(*pRedline), pRedline->GetDoc());
                    break;
                }

                OUString sComment = convertLineEnd(pRedline->GetComment(), GetSystemLineEnd());

                bool bTravel = false;

                SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                ::DialogGetRanges fnGetRange = pFact->GetDialogGetRangesFunc();
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

                bool bPrev = pActRed != nullptr;
                rSh.Pop(SwCursorShell::PopMode::DeleteCurrent);
                rSh.EndAction();

                rSh.ClearMark();
                // Select current redline.
                pActRed = rSh.SelNextRedline();
                if (pActRed != pRedline)
                    rSh.SelPrevRedline();

                rSh.StartAction();
                rSh.Push();
                pActRed = rSh.SelNextRedline();
                bool bNext = pActRed != nullptr;
                rSh.Pop(SwCursorShell::PopMode::DeleteCurrent); // Restore cursor position

                if( rSh.IsCursorPtAtEnd() )
                    rSh.SwapPam();

                rSh.EndAction();

                bTravel |= bNext || bPrev;

                SvxAbstractDialogFactory* pFact2 = SvxAbstractDialogFactory::Create();
                ScopedVclPtr<AbstractSvxPostItDialog> pDlg(pFact2->CreateSvxPostItDialog(GetView().GetFrameWeld(), aSet, bTravel));
                pDlg->HideAuthor();

                pDlg->SetText(lcl_BuildTitleWithRedline(pRedline));

                if (bTravel)
                {
                    pDlg->EnableTravel(bNext, bPrev);
                    pDlg->SetPrevHdl(LINK(this, SwTextShell, RedlinePrevHdl));
                    pDlg->SetNextHdl(LINK(this, SwTextShell, RedlineNextHdl));
                }

                SwViewShell::SetCareDialog(pDlg->GetDialog());
                g_bNoInterrupt = true;

                if ( pDlg->Execute() == RET_OK )
                {
                    const SfxItemSet* pOutSet = pDlg->GetOutputItemSet();
                    OUString sMsg(pOutSet->Get(SID_ATTR_POSTIT_TEXT).GetValue());

                    // Insert or change a comment
                    rSh.SetRedlineComment(sMsg);
                }

                SwViewShell::SetCareDialog(nullptr);
                pDlg.disposeAndClear();
                g_bNoInterrupt = false;
                rSh.ClearMark();
                GetView().AttrChangedNotify(nullptr);
            }
        }
        break;

        case FN_JAVAEDIT:
        {
            OUString aType, aText;
            bool bIsUrl=false;
            bool bNew=false;
            bool bUpdate = false;
            SwFieldMgr aMgr;
            if ( pItem )
            {
                aText = static_cast<const SfxStringItem*>(pItem)->GetValue();
                const SfxStringItem* pType = rReq.GetArg<SfxStringItem>(FN_PARAM_2);
                const SfxBoolItem* pIsUrl = rReq.GetArg<SfxBoolItem>(FN_PARAM_1);
                if ( pType )
                    aType = pType->GetValue();
                if ( pIsUrl )
                    bIsUrl = pIsUrl->GetValue();

                SwScriptField* pField = static_cast<SwScriptField*>(aMgr.GetCurField());
                bNew = !pField || (pField->GetTyp()->Which() != SwFieldIds::Script);
                bUpdate = pField && ( bIsUrl != static_cast<bool>(pField->GetFormat()) || pField->GetPar2() != aType || pField->GetPar1() != aText );
            }
            else
            {
                SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
                ScopedVclPtr<AbstractJavaEditDialog> pDlg(pFact->CreateJavaEditDialog(GetView().GetFrameWeld(), &rSh));
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
            }

            if( bNew )
            {
                SwInsertField_Data aData(SwFieldTypesEnum::Script, 0, aType, aText, bIsUrl ? 1 : 0);
                aMgr.InsertField(aData);
                rReq.Done();
            }
            else if( bUpdate )
            {
                aMgr.UpdateCurField( bIsUrl ? 1 : 0, aType, aText );
                rSh.SetUndoNoResetModified();
                rReq.Done();
            }
            else
                rReq.Ignore();
        }
        break;

        case FN_INSERT_FLD_DATE    :
        {
            nInsertType = SwFieldTypesEnum::Date;
            bIsText = false;
            // use long date format for Hungarian
            SwPaM* pCursorPos = rSh.GetCursor();
            if( pCursorPos )
            {
                LanguageType nLang = pCursorPos->GetPoint()->nNode.GetNode().GetTextNode()->GetLang(pCursorPos->GetPoint()->nContent.GetIndex());
                if (nLang == LANGUAGE_HUNGARIAN)
                    nInsertFormat = rSh.GetNumberFormatter()->GetFormatIndex(NF_DATE_SYSTEM_LONG, nLang);
            }
            goto FIELD_INSERT;
        }
        case FN_INSERT_FLD_TIME    :
            nInsertType = SwFieldTypesEnum::Time;
            bIsText = false;
            goto FIELD_INSERT;
        case FN_INSERT_FLD_PGNUMBER:
            nInsertType = SwFieldTypesEnum::PageNumber;
            nInsertFormat = SVX_NUM_PAGEDESC; // Like page template
            bIsText = false;
            goto FIELD_INSERT;
        case FN_INSERT_FLD_PGCOUNT :
            nInsertType = SwFieldTypesEnum::DocumentStatistics;
            nInsertSubType = 0;
            bIsText = false;
            nInsertFormat = SVX_NUM_PAGEDESC;
            goto FIELD_INSERT;
        case FN_INSERT_FLD_TOPIC   :
            nInsertType = SwFieldTypesEnum::DocumentInfo;
            nInsertSubType = DI_SUBJECT;
            goto FIELD_INSERT;
        case FN_INSERT_FLD_TITLE   :
            nInsertType = SwFieldTypesEnum::DocumentInfo;
            nInsertSubType = DI_TITLE;
            goto FIELD_INSERT;
        case FN_INSERT_FLD_AUTHOR  :
            nInsertType = SwFieldTypesEnum::DocumentInfo;
            nInsertSubType = DI_CREATE|DI_SUB_AUTHOR;

FIELD_INSERT:
        {
            //format conversion should only be done for number formatter formats
            if(!nInsertFormat)
                nInsertFormat = aFieldMgr.GetDefaultFormat(nInsertType, bIsText, rSh.GetNumberFormatter());
            SwInsertField_Data aData(nInsertType, nInsertSubType,
                                OUString(), OUString(), nInsertFormat);
            aFieldMgr.InsertField(aData);
            rReq.Done();
        }
        break;

        case FN_INSERT_TEXT_FORMFIELD:
        {
            rSh.GetDoc()->GetIDocumentUndoRedo().StartUndo(SwUndoId::INSERT_FORM_FIELD, nullptr);

            SwPaM* pCursorPos = rSh.GetCursor();
            if(pCursorPos)
            {
                // Insert five En Space into the text field so the field has extent
                static constexpr OUStringLiteral vEnSpaces = u"\u2002\u2002\u2002\u2002\u2002";
                bool bSuccess = rSh.GetDoc()->getIDocumentContentOperations().InsertString(*pCursorPos, vEnSpaces);
                if(bSuccess)
                {
                    IDocumentMarkAccess* pMarksAccess = rSh.GetDoc()->getIDocumentMarkAccess();
                    SwPaM aFieldPam(pCursorPos->GetPoint()->nNode, pCursorPos->GetPoint()->nContent.GetIndex() - vEnSpaces.getLength(),
                                    pCursorPos->GetPoint()->nNode, pCursorPos->GetPoint()->nContent.GetIndex());
                    pMarksAccess->makeFieldBookmark(aFieldPam, OUString(), ODF_FORMTEXT,
                            aFieldPam.Start());
                }
            }

            rSh.GetDoc()->GetIDocumentUndoRedo().EndUndo(SwUndoId::INSERT_FORM_FIELD, nullptr);
            rSh.GetView().GetViewFrame()->GetBindings().Invalidate( SID_UNDO );
        }
        break;
        case FN_INSERT_CHECKBOX_FORMFIELD:
        {
            rSh.GetDoc()->GetIDocumentUndoRedo().StartUndo(SwUndoId::INSERT_FORM_FIELD, nullptr);

            SwPaM* pCursorPos = rSh.GetCursor();
            if(pCursorPos)
            {
                IDocumentMarkAccess* pMarksAccess = rSh.GetDoc()->getIDocumentMarkAccess();
                pMarksAccess->makeNoTextFieldBookmark(*pCursorPos, OUString(), ODF_FORMCHECKBOX);
            }

            rSh.GetDoc()->GetIDocumentUndoRedo().EndUndo(SwUndoId::INSERT_FORM_FIELD, nullptr);
            rSh.GetView().GetViewFrame()->GetBindings().Invalidate( SID_UNDO );
        }
        break;
        case FN_INSERT_DROPDOWN_FORMFIELD:
        {
            rSh.GetDoc()->GetIDocumentUndoRedo().StartUndo(SwUndoId::INSERT_FORM_FIELD, nullptr);

            SwPaM* pCursorPos = rSh.GetCursor();
            if(pCursorPos)
            {
                IDocumentMarkAccess* pMarksAccess = rSh.GetDoc()->getIDocumentMarkAccess();
                pMarksAccess->makeNoTextFieldBookmark(*pCursorPos, OUString(), ODF_FORMDROPDOWN);
            }

            rSh.GetDoc()->GetIDocumentUndoRedo().EndUndo(SwUndoId::INSERT_FORM_FIELD, nullptr);
            rSh.GetView().GetViewFrame()->GetBindings().Invalidate( SID_UNDO );
        }
        break;
    case FN_INSERT_DATE_FORMFIELD:
    {
        rSh.GetDoc()->GetIDocumentUndoRedo().StartUndo(SwUndoId::INSERT_FORM_FIELD, nullptr);

        SwPaM* pCursorPos = rSh.GetCursor();
        if(pCursorPos)
        {
            // Insert five enspaces into the text field so the field has extent
            sal_Unicode vEnSpaces[ODF_FORMFIELD_DEFAULT_LENGTH] = {8194, 8194, 8194, 8194, 8194};
            bool bSuccess = rSh.GetDoc()->getIDocumentContentOperations().InsertString(*pCursorPos, OUString(vEnSpaces, ODF_FORMFIELD_DEFAULT_LENGTH));
            if(bSuccess)
            {
                IDocumentMarkAccess* pMarksAccess = rSh.GetDoc()->getIDocumentMarkAccess();
                SwPaM aFieldPam(pCursorPos->GetPoint()->nNode, pCursorPos->GetPoint()->nContent.GetIndex() - ODF_FORMFIELD_DEFAULT_LENGTH,
                                pCursorPos->GetPoint()->nNode, pCursorPos->GetPoint()->nContent.GetIndex());
                sw::mark::IFieldmark* pFieldBM = pMarksAccess->makeFieldBookmark(aFieldPam, OUString(), ODF_FORMDATE,
                            aFieldPam.Start());

                // Use a default date format and language
                sw::mark::IFieldmark::parameter_map_t* pParameters = pFieldBM->GetParameters();
                SvNumberFormatter* pFormatter = rSh.GetDoc()->GetNumberFormatter();
                sal_uInt32 nStandardFormat = pFormatter->GetStandardFormat(SvNumFormatType::DATE);
                const SvNumberformat* pFormat = pFormatter->GetEntry(nStandardFormat);

                (*pParameters)[ODF_FORMDATE_DATEFORMAT] <<= pFormat->GetFormatstring();
                (*pParameters)[ODF_FORMDATE_DATEFORMAT_LANGUAGE] <<= LanguageTag(pFormat->GetLanguage()).getBcp47();
            }
        }

        rSh.GetDoc()->GetIDocumentUndoRedo().EndUndo(SwUndoId::INSERT_FORM_FIELD, nullptr);
        rSh.GetView().GetViewFrame()->GetBindings().Invalidate( SID_UNDO );
    }
    break;
        default:
            OSL_FAIL("wrong dispatcher");
            return;
    }
}

void SwTextShell::StateField( SfxItemSet &rSet )
{
    SwWrtShell& rSh = GetShell();
    SfxWhichIter aIter( rSet );
    const SwField* pField = nullptr;
    bool bGetField = false;
    sal_uInt16 nWhich = aIter.FirstWhich();

    while (nWhich)
    {
        switch (nWhich)
        {
        case FN_DELETE_COMMENT:
        case FN_DELETE_NOTE_AUTHOR:
        case FN_DELETE_ALL_NOTES:
        case FN_FORMAT_ALL_NOTES:
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
                // tdf#137568 do not offer comment formating, if no comments are present
                if (!pPostItMgr || !pPostItMgr->HasNotes())
                    rSet.DisableItem( FN_FORMAT_ALL_NOTES );
            }
            break;

        case FN_EDIT_FIELD:
            {
                if( !bGetField )
                {
                    pField = rSh.GetCurField(true);
                    bGetField = true;
                }

                SwFieldIds nTempWhich = pField ? pField->GetTyp()->Which() : SwFieldIds::Unknown;
                if( SwFieldIds::Unknown == nTempWhich ||
                    SwFieldIds::Postit == nTempWhich ||
                    SwFieldIds::Script == nTempWhich ||
                    SwFieldIds::TableOfAuthorities == nTempWhich )
                    rSet.DisableItem( nWhich );
                else if( SwFieldIds::Dde == nTempWhich &&
                    !static_cast<SwDDEFieldType*>(pField->GetTyp())->GetBaseLink().IsVisible())
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
                    pField = rSh.GetCurField();
                    bGetField = true;
                }
                if(!pField || pField->GetTyp()->Which() != SwFieldIds::Macro)
                    rSet.DisableItem(nWhich);
            }
            break;

        case FN_INSERT_FIELD:
            {
                if ( rSh.CursorInsideInputField() )
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
                     || rSh.CursorInsideInputField() )
                {
                    rSet.DisableItem(FN_INSERT_REF_FIELD);
                }
            }
            break;

        case FN_INSERT_FIELD_CTRL:
                if ( rSh.CursorInsideInputField() )
                {
                    rSet.DisableItem(nWhich);
                }
                else
                {
                    rSet.Put(SfxBoolItem( nWhich, GetView().GetViewFrame()->HasChildWindow(FN_INSERT_FIELD)));
                }
            break;

        case FN_REDLINE_COMMENT:
            if (!comphelper::LibreOfficeKit::isActive() && !rSh.GetCurrRedline())
                rSet.DisableItem(nWhich);
            break;

        case FN_REPLY:
            if (!comphelper::LibreOfficeKit::isActive())
                rSet.DisableItem(nWhich);
            break;

        case FN_POSTIT :
        case FN_JAVAEDIT :
            {
                bool bCurField = false;
                pField = rSh.GetCurField();
                if(nWhich == FN_POSTIT)
                    bCurField = pField && pField->GetTyp()->Which() == SwFieldIds::Postit;
                else
                    bCurField = pField && pField->GetTyp()->Which() == SwFieldIds::Script;

                if( !bCurField && rSh.IsReadOnlyAvailable() && rSh.HasReadonlySel() )
                {
                    rSet.DisableItem(nWhich);
                }
                else if ( rSh.CursorInsideInputField() )
                {
                    rSet.DisableItem(nWhich);
                }
                // tdf#86188 Allow disabling comment insertion on footnote/endnote for better OOXML interoperability
                else if ( rSh.IsCursorInFootnote() && !officecfg::Office::Compatibility::View::AllowCommentsInFootnotes::get() )
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
            if ( rSh.CursorInsideInputField() )
            {
                rSet.DisableItem(nWhich);
            }
            break;

        case FN_INSERT_TEXT_FORMFIELD:
        case FN_INSERT_CHECKBOX_FORMFIELD:
        case FN_INSERT_DROPDOWN_FORMFIELD:
        case FN_INSERT_DATE_FORMFIELD:
            if ( rSh.CursorInsideInputField() )
            {
                rSet.DisableItem(nWhich);
            }
            else
            {
                // Check whether we are in a text form field
                SwPosition aCursorPos(*rSh.GetCursor()->GetPoint());
                sw::mark::IFieldmark* pFieldBM = GetShell().getIDocumentMarkAccess()->getFieldmarkFor(aCursorPos);
                if ((!pFieldBM || pFieldBM->GetFieldname() != ODF_FORMTEXT)
                    && aCursorPos.nContent.GetIndex() > 0)
                {
                    SwPosition aPos(aCursorPos);
                    --aPos.nContent;
                    pFieldBM = GetShell().getIDocumentMarkAccess()->getFieldmarkFor(aPos);
                }
                if (pFieldBM && pFieldBM->GetFieldname() == ODF_FORMTEXT &&
                    (aCursorPos > pFieldBM->GetMarkStart() && aCursorPos < pFieldBM->GetMarkEnd() ))
                {
                    rSet.DisableItem(nWhich);
                }
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
    sal_uInt16 nType =  static_cast<sal_uInt16>(rHlnkItem.GetInsertMode());
    nType &= ~HLINK_HTMLMODE;
    const SvxMacroTableDtor* pMacroTable = rHlnkItem.GetMacroTable();

    SwWrtShell& rSh = GetShell();

    if( !(rSh.GetSelectionType() & SelectionType::Text) )
        return;

    rSh.StartAction();
    SfxItemSet aSet(GetPool(), svl::Items<RES_TXTATR_INETFMT, RES_TXTATR_INETFMT>{});
    rSh.GetCurAttr( aSet );

    const SfxPoolItem* pItem;
    if(SfxItemState::SET == aSet.GetItemState(RES_TXTATR_INETFMT, false, &pItem))
    {
        // Select links
        rSh.SwCursorShell::SelectTextAttr(RES_TXTATR_INETFMT, false);
    }
    switch (nType)
    {
    case HLINK_DEFAULT:
    case HLINK_FIELD:
        {
            SwFormatINetFormat aINetFormat( rURL, rTarget );
            aINetFormat.SetName(rHlnkItem.GetIntName());
            if(pMacroTable)
            {
                const SvxMacro *pMacro = pMacroTable->Get( SvMacroItemId::OnMouseOver );
                if( pMacro )
                    aINetFormat.SetMacro(SvMacroItemId::OnMouseOver, *pMacro);
                pMacro = pMacroTable->Get( SvMacroItemId::OnClick );
                if( pMacro )
                    aINetFormat.SetMacro(SvMacroItemId::OnClick, *pMacro);
                pMacro = pMacroTable->Get( SvMacroItemId::OnMouseOut );
                if( pMacro )
                    aINetFormat.SetMacro(SvMacroItemId::OnMouseOut, *pMacro);
            }
            rSh.SttSelect();
            rSh.InsertURL( aINetFormat, rName, true );
            rSh.EndSelect();
        }
        break;

    case HLINK_BUTTON:
        bool bSel = rSh.HasSelection();
        if(bSel)
            rSh.DelRight();
        InsertURLButton( rURL, rTarget, rName );
        rSh.EnterStdMode();
        break;
    }
    rSh.EndAction();
}

IMPL_LINK( SwTextShell, RedlineNextHdl, AbstractSvxPostItDialog&, rDlg, void )
{
    SwWrtShell* pSh = GetShellPtr();

    // Insert or change a comment.
    pSh->SetRedlineComment(rDlg.GetNote());

    const SwRangeRedline *pRedline = pSh->GetCurrRedline();

    if (!pRedline)
        return;

    // Traveling only if more than one field.
    if( !pSh->IsCursorPtAtEnd() )
        pSh->SwapPam(); // Move the cursor behind the Redline.

    pSh->Push();
    const SwRangeRedline *pActRed = pSh->SelNextRedline();
    pSh->Pop((pActRed != nullptr) ? SwCursorShell::PopMode::DeleteStack : SwCursorShell::PopMode::DeleteCurrent);

    bool bEnable = false;

    if (pActRed)
    {
        pSh->StartAction();
        pSh->Push();
        bEnable = pSh->SelNextRedline() != nullptr;
        pSh->Pop(SwCursorShell::PopMode::DeleteCurrent);
        pSh->EndAction();
    }

    rDlg.EnableTravel(bEnable, true);

    if( pSh->IsCursorPtAtEnd() )
        pSh->SwapPam();

    pRedline = pSh->GetCurrRedline();
    OUString sComment = convertLineEnd(pRedline->GetComment(), GetSystemLineEnd());

    rDlg.SetNote(sComment);
    rDlg.ShowLastAuthor( pRedline->GetAuthorString(),
                GetAppLangDateTimeString(
                            pRedline->GetRedlineData().GetTimeStamp() ));

    rDlg.SetText(lcl_BuildTitleWithRedline(pRedline));

}

IMPL_LINK( SwTextShell, RedlinePrevHdl, AbstractSvxPostItDialog&, rDlg, void )
{
    SwWrtShell* pSh = GetShellPtr();

    // Insert or change a comment.
    pSh->SetRedlineComment(rDlg.GetNote());

    const SwRangeRedline *pRedline = pSh->GetCurrRedline();

    if (!pRedline)
        return;

    // Traveling only if more than one field.
    pSh->Push();
    const SwRangeRedline *pActRed = pSh->SelPrevRedline();
    pSh->Pop((pActRed != nullptr) ? SwCursorShell::PopMode::DeleteStack : SwCursorShell::PopMode::DeleteCurrent);

    bool bEnable = false;

    if (pActRed)
    {
        pSh->StartAction();
        pSh->Push();
        bEnable = pSh->SelPrevRedline() != nullptr;
        pSh->Pop(SwCursorShell::PopMode::DeleteCurrent);
        pSh->EndAction();
    }

    rDlg.EnableTravel(true, bEnable);

    pRedline = pSh->GetCurrRedline();
    OUString sComment = convertLineEnd(pRedline->GetComment(), GetSystemLineEnd());

    rDlg.SetNote(sComment);
    rDlg.ShowLastAuthor(pRedline->GetAuthorString(),
            GetAppLangDateTimeString(
                            pRedline->GetRedlineData().GetTimeStamp() ));

    rDlg.SetText(lcl_BuildTitleWithRedline(pRedline));

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
