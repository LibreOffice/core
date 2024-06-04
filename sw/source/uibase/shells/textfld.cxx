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

#include <com/sun/star/beans/PropertyValues.hpp>
#include <AnnotationWin.hxx>
#include <comphelper/lok.hxx>
#include <hintids.hxx>
#include <IDocumentFieldsAccess.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/lnkbase.hxx>
#include <txtfld.hxx>
#include <svl/itempool.hxx>
#include <svl/numformat.hxx>
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
#include <fmthdft.hxx>
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
#include <svtools/strings.hrc>
#include <svtools/svtresid.hxx>

#include <editeng/ulspitem.hxx>
#include <xmloff/odffields.hxx>
#include <IDocumentContentOperations.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <IDocumentUndoRedo.hxx>
#include <svl/zforlist.hxx>
#include <svl/zformat.hxx>
#include <svx/xfillit0.hxx>
#include <svx/pageitem.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <IMark.hxx>
#include <officecfg/Office/Common.hxx>
#include <officecfg/Office/Compatibility.hxx>
#include <ndtxt.hxx>
#include <translatehelper.hxx>
#include <sfx2/dispatch.hxx>


using namespace nsSwDocInfoSubType;

static OUString lcl_BuildTitleWithRedline( const SwRangeRedline *pRedline )
{
    const OUString sTitle(SwResId(STR_REDLINE_COMMENT));

    TranslateId pResId;
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
        pArgs->GetItemState(GetPool().GetWhichIDFromSlotID(nSlot), false, &pItem);

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
                            if (officecfg::Office::Common::Security::Scripting::DisableActiveContent::get())
                            {
                                std::unique_ptr<weld::MessageDialog> xError(
                                    Application::CreateMessageDialog(
                                        nullptr, VclMessageType::Warning, VclButtonsType::Ok,
                                        SvtResId(STR_WARNING_EXTERNAL_LINK_EDIT_DISABLED)));
                                xError->run();
                                break;
                            }

                            SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                            VclPtr<SfxAbstractLinksDialog> pDlg(pFact->CreateLinksDialog(GetView().GetFrameWeld(), &rSh.GetLinkManager(), false, &rLink));
                            pDlg->StartExecuteAsync(
                                [pDlg] (sal_Int32 /*nResult*/)->void
                                {
                                    pDlg->disposeOnce();
                                }
                            );
                        }
                        break;
                    }
                    default:
                    {
                        SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
                        VclPtr<SfxAbstractDialog> pDlg(pFact->CreateSwFieldEditDlg( GetView() ));
                        // without TabPage no dialog
                        if (pDlg)
                            pDlg->StartExecuteAsync(
                                [pDlg] (sal_Int32 /*nResult*/)->void
                                {
                                    pDlg->disposeOnce();
                                }
                            );
                    }
                }
            }
            break;
        }
        case FN_UPDATE_SEL_FIELD:
        {
            SwField *pField = rSh.GetCurField();

            if (pField)
            {
               rSh.UpdateOneField(*pField);
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
                               SwCursorShell::GetTextFieldAtCursor(rSh.GetCursor(), ::sw::GetTextAttrMode::Default))))
                    {
                        rSh.SttSelect();
                        rSh.SelectTextModel(
                            SwCursorShell::StartOfInputFieldAtPos( *(rSh.GetCursor()->Start()) ) + 1,
                            SwCursorShell::EndOfInputFieldAtPos( *(rSh.GetCursor()->Start()) ) - 1 );
                    }
                    else if (SwField* pCurrentField = rSh.GetCurField(true))
                    {
                        rSh.StartInputFieldDlg(pCurrentField, false, false, GetView().GetFrameWeld());
                    }
                    bRet = true;
                }

                rReq.SetReturnValue( SfxBoolItem( nSlot, bRet ));
            }
            break;

        case FN_GOTO_MARK:
        {
            const SfxStringItem* pName = rReq.GetArg<SfxStringItem>(FN_GOTO_MARK);
            if (pName)
            {
                rSh.GotoMark(pName->GetValue());
            }
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

                if( const SfxUInt16Item* pFieldItem = pArgs->GetItemIfSet( FN_PARAM_FIELD_TYPE,
                                                            false ))
                    nType = static_cast<SwFieldTypesEnum>(pFieldItem->GetValue());
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
                if( const SfxStringItem* pContentItem = pArgs->GetItemIfSet(
                                    FN_PARAM_FIELD_CONTENT, false ))
                    aPar2 = pContentItem->GetValue();
                if( const SfxUInt32Item* pFormatItem = pArgs->GetItemIfSet(
                                    FN_PARAM_FIELD_FORMAT, false ))
                    nFormat = pFormatItem->GetValue();
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

                if( const SfxUInt16Item* pTypeItem = pArgs->GetItemIfSet( FN_PARAM_FIELD_TYPE,
                                                            false ))
                    nType = static_cast<SwFieldTypesEnum>(pTypeItem->GetValue());
                else if (pArgs->GetItemState(FN_PARAM_4, false, &pItem) == SfxItemState::SET)
                {
                    const OUString& rTypeName = static_cast<const SfxStringItem *>(pItem)->GetValue();
                    nType = SwFieldTypeFromString(rTypeName);
                }
                if( const SfxUInt16Item* pSubtypeItem = pArgs->GetItemIfSet( FN_PARAM_FIELD_SUBTYPE,
                                                            false ))
                    nSubType = pSubtypeItem->GetValue();
                if( const SfxStringItem* pContentItem = pArgs->GetItemIfSet(
                                    FN_PARAM_FIELD_CONTENT, false ))
                    aPar2 = pContentItem->GetValue();
                if( const SfxUInt32Item* pFormatItem = pArgs->GetItemIfSet(
                                    FN_PARAM_FIELD_FORMAT, false ))
                    nFormat = pFormatItem->GetValue();
                if( SfxItemState::SET == pArgs->GetItemState(
                                    FN_PARAM_3, false, &pItem ))
                {
                    OUString sTmp = static_cast<const SfxStringItem *>(pItem)->GetValue();
                    if(!sTmp.isEmpty())
                        cSeparator = sTmp[0];
                }
                if (pArgs->GetItemState(FN_PARAM_5, false, &pItem) == SfxItemState::SET)
                {
                    // Wrap the field in the requested container instead of inserting it
                    // directly at the cursor position.
                    const OUString& rWrapper = static_cast<const SfxStringItem *>(pItem)->GetValue();
                    if (rWrapper == "Footnote")
                    {
                        GetShellPtr()->InsertFootnote(OUString());
                    }
                    else if (rWrapper == "Endnote")
                    {
                        GetShellPtr()->InsertFootnote(OUString(), /*bEndNote=*/true);
                    }
                }
                SwInsertField_Data aData(nType, nSubType, aPar1, aPar2, nFormat, GetShellPtr(), cSeparator );
                bRes = aFieldMgr.InsertField( aData );
            }
            else
            {
                //#i5788# prevent closing of the field dialog while a modal dialog ( Input field dialog ) is active
                if(!GetView().GetViewFrame().IsInModalMode())
                {
                    SfxViewFrame& rVFrame = GetView().GetViewFrame();
                    rVFrame.ToggleChildWindow(FN_INSERT_FIELD);
                    bRes = rVFrame.GetChildWindow( nSlot ) != nullptr;
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
            SfxViewFrame& rVFrame = GetView().GetViewFrame();
            if (!rVFrame.HasChildWindow(FN_INSERT_FIELD))
                rVFrame.ToggleChildWindow(FN_INSERT_FIELD);    // Show dialog

            // Switch Fielddlg at a new TabPage
            sal_uInt16 nId = SwFieldDlgWrapper::GetChildWindowId();
            SwFieldDlgWrapper *pWrp = static_cast<SwFieldDlgWrapper*>(rVFrame.GetChildWindow(nId));
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

                        SwPostItField* pLatestPostItField = pMgr->GetLatestPostItField();
                        if (pLatestPostItField)
                        {
                            // Set the parent postit id of the reply.
                            pLatestPostItField->SetParentPostItId(pIdItem->GetValue().toUInt32());

                            // If name of the replied comment is empty, we need to set a name in order to connect them in the xml file.
                            pWin->GeneratePostItName(); // Generates a name if the current name is empty.

                            pLatestPostItField->SetParentName(pWin->GetPostItField()->GetName());
                        }
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
        case FN_INSERT_FLD_DATE_VAR:
        {
            nInsertType = SwFieldTypesEnum::Date;
            nInsertSubType = nSlot == FN_INSERT_FLD_DATE ? 0 : 1;
            bIsText = false;
            // use long date format for Hungarian
            SwPaM* pCursorPos = rSh.GetCursor();
            if( pCursorPos )
            {
                LanguageType nLang = pCursorPos->GetPoint()->GetNode().GetTextNode()->GetLang(pCursorPos->GetPoint()->GetContentIndex());
                if (nLang == LANGUAGE_HUNGARIAN)
                    nInsertFormat = rSh.GetNumberFormatter()->GetFormatIndex(NF_DATE_SYSTEM_LONG, nLang);
            }
            goto FIELD_INSERT;
        }
        case FN_INSERT_FLD_TIME    :
        case FN_INSERT_FLD_TIME_VAR:
            nInsertType = SwFieldTypesEnum::Time;
            nInsertSubType = nSlot == FN_INSERT_FLD_TIME ? 0 : 1;
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
            OUString aFieldType(ODF_FORMTEXT);
            const SfxStringItem* pFieldType = rReq.GetArg<SfxStringItem>(FN_PARAM_1);
            if (pFieldType)
            {
                // Allow overwriting the default type.
                aFieldType = pFieldType->GetValue();
            }

            OUString aFieldCode;
            const SfxStringItem* pFieldCode = rReq.GetArg<SfxStringItem>(FN_PARAM_2);
            if (pFieldCode)
            {
                // Allow specifying a field code/command.
                aFieldCode = pFieldCode->GetValue();
            }

            if (rSh.HasReadonlySel())
            {
                // Inform the user that the request has been ignored.
                auto xInfo = std::make_shared<weld::GenericDialogController>(
                    GetView().GetFrameWeld(), "modules/swriter/ui/inforeadonlydialog.ui",
                    "InfoReadonlyDialog");
                weld::DialogController::runAsync(xInfo, [](sal_Int32 /*nResult*/) {});
                break;
            }

            rSh.GetDoc()->GetIDocumentUndoRedo().StartUndo(SwUndoId::INSERT_FORM_FIELD, nullptr);

            SwPaM* pCursorPos = rSh.GetCursor();
            if(pCursorPos)
            {
                // Insert five En Space into the text field so the field has extent
                static constexpr OUStringLiteral vEnSpaces = u"\u2002\u2002\u2002\u2002\u2002";
                OUString aFieldResult(vEnSpaces);
                const SfxStringItem* pFieldResult = rReq.GetArg<SfxStringItem>(FN_PARAM_3);
                if (pFieldResult)
                {
                    // Allow specifying a field result / expanded value.
                    aFieldResult = pFieldResult->GetValue();
                }

                const SfxStringItem* pWrapper = rReq.GetArg<SfxStringItem>(FN_PARAM_4);
                if (pWrapper)
                {
                    // Wrap the fieldmark in the requested container instead of inserting it
                    // directly at the cursor position.
                    OUString aWrapper = pWrapper->GetValue();
                    if (aWrapper == "Footnote")
                    {
                        rSh.InsertFootnote(OUString());
                    }
                    else if (aWrapper == "Endnote")
                    {
                        // It's important that there is no Start/EndAction() around this, so the
                        // inner EndAction() triggers a layout update and the cursor can jump to the
                        // created SwFootnoteFrame.
                        rSh.InsertFootnote(OUString(), /*bEndNote=*/true);
                    }
                }

                // Don't update the layout after inserting content and before deleting temporary
                // text nodes.
                rSh.StartAction();

                // Split node to remember where the start position is.
                bool bSuccess = rSh.GetDoc()->getIDocumentContentOperations().SplitNode(
                    *pCursorPos->GetPoint(), false);
                if(bSuccess)
                {
                    SwPaM aFieldPam(*pCursorPos->GetPoint());
                    aFieldPam.Move(fnMoveBackward, GoInContent);
                    if (pFieldResult)
                    {
                        // Paste HTML content.
                        SwTranslateHelper::PasteHTMLToPaM(rSh, pCursorPos, aFieldResult.toUtf8());
                        if (pCursorPos->GetPoint()->GetContentIndex() == 0)
                        {
                            // The paste created a last empty text node, remove it.
                            SwPaM aPam(*pCursorPos->GetPoint());
                            aPam.SetMark();
                            aPam.Move(fnMoveBackward, GoInContent);
                            rSh.GetDoc()->getIDocumentContentOperations().DeleteAndJoin(aPam);
                        }
                    }
                    else
                    {
                        // Insert default placeholder.
                        rSh.GetDoc()->getIDocumentContentOperations().InsertString(*pCursorPos,
                                                                                   aFieldResult);
                    }
                    // Undo the above SplitNode().
                    aFieldPam.SetMark();
                    aFieldPam.Move(fnMoveForward, GoInContent);
                    rSh.GetDoc()->getIDocumentContentOperations().DeleteAndJoin(aFieldPam);
                    *aFieldPam.GetMark() = *pCursorPos->GetPoint();

                    IDocumentMarkAccess* pMarksAccess = rSh.GetDoc()->getIDocumentMarkAccess();
                    sw::mark::IFieldmark* pFieldmark = pMarksAccess->makeFieldBookmark(
                        aFieldPam, OUString(), aFieldType, aFieldPam.Start());
                    if (pFieldmark && !aFieldCode.isEmpty())
                    {
                        pFieldmark->GetParameters()->insert(
                            std::pair<OUString, uno::Any>(ODF_CODE_PARAM, uno::Any(aFieldCode)));
                    }
                }
                rSh.EndAction();
            }

            rSh.GetDoc()->GetIDocumentUndoRedo().EndUndo(SwUndoId::INSERT_FORM_FIELD, nullptr);
            rSh.GetView().GetViewFrame().GetBindings().Invalidate( SID_UNDO );
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
            rSh.GetView().GetViewFrame().GetBindings().Invalidate( SID_UNDO );
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
            rSh.GetView().GetViewFrame().GetBindings().Invalidate( SID_UNDO );
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
                SwPaM aFieldPam(pCursorPos->GetPoint()->GetNode(), pCursorPos->GetPoint()->GetContentIndex() - ODF_FORMFIELD_DEFAULT_LENGTH,
                                pCursorPos->GetPoint()->GetNode(), pCursorPos->GetPoint()->GetContentIndex());
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
        rSh.GetView().GetViewFrame().GetBindings().Invalidate( SID_UNDO );
    }
    break;
    case FN_UPDATE_TEXT_FORMFIELDS:
    {
        // This updates multiple fieldmarks in a document, based on their field name & field command
        // prefix.
        OUString aFieldType;
        const SfxStringItem* pFieldType = rReq.GetArg<SfxStringItem>(FN_PARAM_1);
        if (pFieldType)
        {
            aFieldType = pFieldType->GetValue();
        }
        OUString aFieldCommandPrefix;
        const SfxStringItem* pFieldCommandPrefix = rReq.GetArg<SfxStringItem>(FN_PARAM_2);
        if (pFieldCommandPrefix)
        {
            aFieldCommandPrefix = pFieldCommandPrefix->GetValue();
        }
        uno::Sequence<beans::PropertyValues> aFields;
        const SfxUnoAnyItem* pFields = rReq.GetArg<SfxUnoAnyItem>(FN_PARAM_3);
        if (pFields)
        {
            pFields->GetValue() >>= aFields;
        }

        rSh.GetDoc()->GetIDocumentUndoRedo().StartUndo(SwUndoId::UPDATE_FORM_FIELDS, nullptr);
        rSh.StartAction();

        IDocumentMarkAccess* pMarkAccess = rSh.GetDoc()->getIDocumentMarkAccess();
        sal_Int32 nFieldIndex = 0;
        for (auto it = pMarkAccess->getFieldmarksBegin(); it != pMarkAccess->getFieldmarksEnd(); ++it)
        {
            auto pFieldmark = dynamic_cast<sw::mark::IFieldmark*>(*it);
            assert(pFieldmark);
            if (pFieldmark->GetFieldname() != aFieldType)
            {
                continue;
            }

            auto itParam = pFieldmark->GetParameters()->find(ODF_CODE_PARAM);
            if (itParam == pFieldmark->GetParameters()->end())
            {
                continue;
            }

            OUString aCommand;
            itParam->second >>= aCommand;
            if (!aCommand.startsWith(aFieldCommandPrefix))
            {
                continue;
            }

            if (aFields.getLength() <= nFieldIndex)
            {
                continue;
            }

            comphelper::SequenceAsHashMap aMap(aFields[nFieldIndex++]);
            itParam->second = aMap[u"FieldCommand"_ustr];
            SwPaM aPaM(pFieldmark->GetMarkPos(), pFieldmark->GetOtherMarkPos());
            aPaM.Normalize();
            // Skip field start & separator.
            aPaM.GetPoint()->AdjustContent(2);
            // Skip field end.
            aPaM.GetMark()->AdjustContent(-1);
            rSh.GetDoc()->getIDocumentContentOperations().DeleteAndJoin(aPaM);
            OUString aFieldResult;
            aMap[u"FieldResult"_ustr] >>= aFieldResult;
            SwTranslateHelper::PasteHTMLToPaM(rSh, &aPaM, aFieldResult.toUtf8());
        }

        rSh.EndAction();
        rSh.GetDoc()->GetIDocumentUndoRedo().EndUndo(SwUndoId::UPDATE_FORM_FIELDS, nullptr);
    }
    break;
    case FN_DELETE_TEXT_FORMFIELDS:
    {
        // This deletes all fieldmarks that match the provided field type & field command prefix.
        OUString aFieldType;
        const SfxStringItem* pFieldType = rReq.GetArg<SfxStringItem>(FN_PARAM_1);
        if (pFieldType)
        {
            aFieldType = pFieldType->GetValue();
        }
        OUString aFieldCommandPrefix;
        const SfxStringItem* pFieldCommandPrefix = rReq.GetArg<SfxStringItem>(FN_PARAM_2);
        if (pFieldCommandPrefix)
        {
            aFieldCommandPrefix = pFieldCommandPrefix->GetValue();
        }
        rSh.GetDoc()->GetIDocumentUndoRedo().StartUndo(SwUndoId::DELETE_FORM_FIELDS, nullptr);
        rSh.StartAction();

        IDocumentMarkAccess* pMarkAccess = rSh.GetDoc()->getIDocumentMarkAccess();
        std::vector<sw::mark::IMark*> aRemovals;
        for (auto it = pMarkAccess->getFieldmarksBegin(); it != pMarkAccess->getFieldmarksEnd(); ++it)
        {
            auto pFieldmark = dynamic_cast<sw::mark::IFieldmark*>(*it);
            assert(pFieldmark);
            if (pFieldmark->GetFieldname() != aFieldType)
            {
                continue;
            }

            if (!aFieldCommandPrefix.isEmpty())
            {
                auto itParam = pFieldmark->GetParameters()->find(ODF_CODE_PARAM);
                if (itParam == pFieldmark->GetParameters()->end())
                {
                    continue;
                }

                OUString aCommand;
                itParam->second >>= aCommand;
                if (!aCommand.startsWith(aFieldCommandPrefix))
                {
                    continue;
                }
            }

            aRemovals.push_back(pFieldmark);
        }

        for (const auto& pMark : aRemovals)
        {
            pMarkAccess->deleteMark(pMark);
        }

        rSh.EndAction();
        rSh.GetDoc()->GetIDocumentUndoRedo().EndUndo(SwUndoId::DELETE_FORM_FIELDS, nullptr);
    }
    break;
    case FN_PGNUMBER_WIZARD:
    {
        SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
        VclPtr<AbstractSwPageNumberDlg> pDlg(
                pFact->CreateSwPageNumberDlg(GetView().GetFrameWeld()));
        auto pShell = GetShellPtr();

        const SwPageDesc& rCurrDesc = rSh.GetPageDesc(rSh.GetCurPageDesc());
        pDlg->SetPageNumberType(rCurrDesc.GetNumType().GetNumberingType());

        pDlg->StartExecuteAsync([pShell, &rSh, pDlg](int nResult) {
            if ( nResult == RET_OK )
            {
                auto rDoc = rSh.GetDoc();

                rSh.LockView(true);
                rSh.StartAllAction();
                rSh.SwCursorShell::Push();
                rDoc->GetIDocumentUndoRedo().StartUndo(SwUndoId::INSERT_PAGE_NUMBER, nullptr);

                const size_t nPageDescIndex = rSh.GetCurPageDesc();
                const SwPageDesc& rDesc = rSh.GetPageDesc(nPageDescIndex);
                const bool bHeader = !pDlg->GetPageNumberPosition();
                const bool bHeaderAlreadyOn = rDesc.GetMaster().GetHeader().IsActive();
                const bool bFooterAlreadyOn = rDesc.GetMaster().GetFooter().IsActive();
                const bool bIsSinglePage = rDesc.GetFollow() != &rDesc;
                const size_t nMirrorPagesNeeded = rDesc.IsFirstShared() ? 2 : 3;
                const OUString sBookmarkName = OUString::Concat("PageNumWizard_")
                    + (bHeader ? "HEADER" : "FOOTER") + "_" + rDesc.GetName();
                IDocumentMarkAccess& rIDMA = *rSh.getIDocumentMarkAccess();

                // Allow wizard to be re-run: delete previously wizard-inserted page number.
                // Try before creating non-shared header: avoid copying ODD bookmark onto EVEN page.
                IDocumentMarkAccess::const_iterator_t ppMark = rIDMA.findMark(
                    sBookmarkName + OUString::number(rSh.GetVirtPageNum()));
                if (ppMark != rIDMA.getAllMarksEnd() && *ppMark)
                {
                    SwPaM aDeleteOldPageNum((*ppMark)->GetMarkStart(), (*ppMark)->GetMarkEnd());
                    rDoc->getIDocumentContentOperations().DeleteAndJoin(aDeleteOldPageNum);
                }

                SwPageDesc aNewDesc(rDesc);
                bool bChangePageDesc = false;
                if (pDlg->GetPageNumberType() != aNewDesc.GetNumType().GetNumberingType())
                {
                    bChangePageDesc = true;
                    SvxNumberType aNewType(rDesc.GetNumType());
                    aNewType.SetNumberingType(pDlg->GetPageNumberType());
                    aNewDesc.SetNumType(aNewType);
                }

                // Insert header/footer
                if ((bHeader && !bHeaderAlreadyOn) || (!bHeader && !bFooterAlreadyOn))
                {
                    bChangePageDesc = true;
                    SwFrameFormat &rMaster = aNewDesc.GetMaster();
                    if (bHeader)
                        rMaster.SetFormatAttr(SwFormatHeader(/*On=*/true));
                    else
                        rMaster.SetFormatAttr(SwFormatFooter(/*On=*/true));

                    // Init copied from ChangeHeaderOrFooter: keep in sync
                    constexpr tools::Long constTwips_5mm = o3tl::toTwips(5, o3tl::Length::mm);
                    const SvxULSpaceItem aUL(bHeader ? 0 : constTwips_5mm,
                                             bHeader ? constTwips_5mm : 0,
                                             RES_UL_SPACE);
                    const XFillStyleItem aFill(drawing::FillStyle_NONE);
                    SwFrameFormat& rFormat
                        = bHeader
                              ? const_cast<SwFrameFormat&>(*rMaster.GetHeader().GetHeaderFormat())
                              : const_cast<SwFrameFormat&>(*rMaster.GetFooter().GetFooterFormat());
                    rFormat.SetFormatAttr(aUL);
                    rFormat.SetFormatAttr(aFill);

                    // Might as well turn on margin mirroring too - if appropriate
                    if (pDlg->GetMirrorOnEvenPages() && !bHeaderAlreadyOn && !bFooterAlreadyOn
                        && !bIsSinglePage
                        && (aNewDesc.ReadUseOn() & UseOnPage::Mirror) == UseOnPage::All)
                    {
                        aNewDesc.WriteUseOn(rDesc.ReadUseOn() | UseOnPage::Mirror);
                    }
                }

                const bool bCreateMirror = !bIsSinglePage && pDlg->GetMirrorOnEvenPages()
                    && nMirrorPagesNeeded <= rSh.GetPageCnt();
                if (bCreateMirror)
                {
                    // Use different left/right header/footer
                    if ((bHeader && rDesc.IsHeaderShared()) || (!bHeader && rDesc.IsFooterShared()))
                    {
                        bChangePageDesc = true;
                        if (bHeader)
                            aNewDesc.ChgHeaderShare(/*Share=*/false);
                        else
                            aNewDesc.ChgFooterShare(/*Share=*/false);
                    }
                }

                if (bChangePageDesc)
                    rSh.ChgPageDesc(nPageDescIndex, aNewDesc);

                // Go to the header or footer insert position
                bool bInHF = false;
                bool bSkipMirror = true;
                size_t nEvenPage = 0;
                if (bCreateMirror || !rSh.GetCurrFrame())
                {
                    // Come here if Currframe can't be found, otherwise Goto*Text will crash.
                    // Get*PageNum will also be invalid (0), so we have no idea where we are.
                    // (Since not asking for mirror, the likelihood is that the bHeader is shared,
                    // in which case it doesn't matter anyway, and we just hope for the best.)
                    // Read the code in this block assuming that bCreateMirror is true.

                    // There are enough pages that there probably is a valid odd page.
                    // However, that is not guaranteed: perhaps the page style switched,
                    // or a blank page was forced, or some other complexity.
                    bInHF = rSh.SetCursorInHdFt(nPageDescIndex, bHeader, /*Even=*/true);
                    if (bInHF)
                    {
                        // Remember valid EVEN page. Mirror it if also a valid ODD or FIRST page
                        nEvenPage = rSh.GetVirtPageNum();
                        assert (nEvenPage && "couldn't find page number. Use a bool instead");
                    }

                    bInHF = rSh.SetCursorInHdFt(nPageDescIndex, bHeader, /*Even=*/false);
                    if (bInHF && nEvenPage)
                    {
                        // Even though the cursor may be on a FIRST page,
                        // the user requested mirrored pages, and we have both ODD and EVEN,
                        // so set page numbers on these two pages, and leave FIRST alone.
                        bSkipMirror = false;
                    }
                    if (!bInHF)
                    {
                        // no ODD page, look for FIRST page
                        bInHF = rSh.SetCursorInHdFt(nPageDescIndex, bHeader, false, /*First=*/true);
                        if (bInHF && nEvenPage)
                        {
                            // Unlikely but valid situation: EVEN and FIRST pages, but no ODD page.
                            // In this case, the first header gets the specified page number
                            // and the even header is mirrored, with an empty odd header,
                            // as the user (somewhat) requested.
                            bSkipMirror = false;
                        }
                    }
                    assert((bInHF || nEvenPage) && "Impossible - why couldn't the move happen?");
                    assert((bInHF || nEvenPage == rSh.GetVirtPageNum()) && "Unexpected move");
                }
                else
                {
                    if (bHeader)
                        bInHF = rSh.GotoHeaderText();
                    else
                        bInHF = rSh.GotoFooterText();
                    assert(bInHF && "shouldn't have a problem going to text when no mirroring");
                }

                // Allow wizard to be re-run: delete previously wizard-inserted page number.
                // Now that the cursor may have moved to a different page, try delete again.
                ppMark = rIDMA.findMark(sBookmarkName + OUString::number(rSh.GetVirtPageNum()));
                if (ppMark != rIDMA.getAllMarksEnd() && *ppMark)
                {
                    SwPaM aDeleteOldPageNum((*ppMark)->GetMarkStart(), (*ppMark)->GetMarkEnd());
                    rDoc->getIDocumentContentOperations().DeleteAndJoin(aDeleteOldPageNum);
                }

                SwTextNode* pTextNode = rSh.GetCursor()->GetPoint()->GetNode().GetTextNode();

                // Insert new line if there is already text in header/footer
                if (pTextNode && !pTextNode->GetText().isEmpty())
                {
                    rDoc->getIDocumentContentOperations().SplitNode(*rSh.GetCursor()->GetPoint(), false);

                    // Go back to start of header/footer
                    if (bHeader)
                        rSh.GotoHeaderText();
                    else
                        rSh.GotoFooterText();
                }

                // Set alignment for the new line
                switch (pDlg->GetPageNumberAlignment())
                {
                    case 0:
                    {
                        SvxAdjustItem aAdjustItem(SvxAdjust::Left, RES_PARATR_ADJUST);
                        rSh.SetAttrItem(aAdjustItem);
                        break;
                    }
                    case 1:
                    {
                        SvxAdjustItem aAdjustItem(SvxAdjust::Center, RES_PARATR_ADJUST);
                        rSh.SetAttrItem(aAdjustItem);
                        break;
                    }
                    case 2:
                    {
                        SvxAdjustItem aAdjustItem(SvxAdjust::Right, RES_PARATR_ADJUST);
                        rSh.SetAttrItem(aAdjustItem);
                        break;
                    }
                }

                sal_Int32 nStartContentIndex = rSh.GetCursor()->Start()->GetContentIndex();
                assert(!nStartContentIndex && "earlier split node if not empty, but not zero?");

                // Insert page number
                SwFieldMgr aMgr(pShell);
                SwInsertField_Data aData(SwFieldTypesEnum::PageNumber, 0,
                            OUString(), OUString(), SVX_NUM_PAGEDESC);
                aMgr.InsertField(aData);
                if (pDlg->GetIncludePageTotal())
                {
                    rDoc->getIDocumentContentOperations().InsertString(*rSh.GetCursor(), u" / "_ustr);
                    SwInsertField_Data aPageTotalData(SwFieldTypesEnum::DocumentStatistics, DS_PAGE,
                                                      OUString(), OUString(), SVX_NUM_PAGEDESC);
                    aMgr.InsertField(aPageTotalData);
                }

                // Mark inserted fields with a bookmark - so it can be found/removed if re-run
                SwPaM aNewBookmarkPaM(*rSh.GetCursor()->Start());
                aNewBookmarkPaM.SetMark();
                assert(aNewBookmarkPaM.GetPointContentNode() && "only SetContent on content node");
                aNewBookmarkPaM.Start()->SetContent(nStartContentIndex);
                rIDMA.makeMark(aNewBookmarkPaM,
                               sBookmarkName + OUString::number(rSh.GetVirtPageNum()),
                               IDocumentMarkAccess::MarkType::BOOKMARK,
                               sw::mark::InsertMode::New);

                // Mirror on the even pages
                if (!bSkipMirror && bCreateMirror
                    && rSh.SetCursorInHdFt(nPageDescIndex, bHeader, /*Even=*/true))
                {
                    assert(nEvenPage && "what? no even page and yet we got here?");
                    ppMark = rIDMA.findMark(sBookmarkName + OUString::number(rSh.GetVirtPageNum()));
                    if (ppMark != rIDMA.getAllMarksEnd() && *ppMark)
                    {
                        SwPaM aDeleteOldPageNum((*ppMark)->GetMarkStart(), (*ppMark)->GetMarkEnd());
                        rDoc->getIDocumentContentOperations().DeleteAndJoin(aDeleteOldPageNum);
                    }

                    pTextNode = rSh.GetCursor()->GetPoint()->GetNode().GetTextNode();

                    // Insert new line if there is already text in header/footer
                    if (pTextNode && !pTextNode->GetText().isEmpty())
                    {
                        rDoc->getIDocumentContentOperations().SplitNode(
                            *rSh.GetCursor()->GetPoint(), false);
                        // Go back to start of header/footer
                        rSh.SetCursorInHdFt(nPageDescIndex, bHeader, /*Even=*/true);
                    }

                    // mirror the adjustment
                    assert(pDlg->GetPageNumberAlignment() != 1 && "cannot have Center and bMirror");
                    SvxAdjust eAdjust = SvxAdjust::Left;
                    if (!pDlg->GetPageNumberAlignment())
                        eAdjust = SvxAdjust::Right;
                    SvxAdjustItem aMirrorAdjustItem(eAdjust, RES_PARATR_ADJUST);
                    rSh.SetAttrItem(aMirrorAdjustItem);

                    nStartContentIndex = rSh.GetCursor()->Start()->GetContentIndex();

                    // Insert page number
                    SwFieldMgr aEvenMgr(pShell);
                    aEvenMgr.InsertField(aData);
                    if (pDlg->GetIncludePageTotal())
                    {
                        rDoc->getIDocumentContentOperations().InsertString(*rSh.GetCursor(), u" / "_ustr);
                        SwInsertField_Data aPageTotalData(SwFieldTypesEnum::DocumentStatistics,
                                                          DS_PAGE, OUString(), OUString(),
                                                          SVX_NUM_PAGEDESC);
                        aMgr.InsertField(aPageTotalData);
                    }

                    // Mark inserted fields with a bookmark - so it can be found/removed if re-run
                    SwPaM aNewEvenBookmarkPaM(*rSh.GetCursor()->Start());
                    aNewEvenBookmarkPaM.SetMark();
                    aNewEvenBookmarkPaM.Start()->SetContent(nStartContentIndex);
                    rIDMA.makeMark(aNewEvenBookmarkPaM,
                                   sBookmarkName + OUString::number(rSh.GetVirtPageNum()),
                                   IDocumentMarkAccess::MarkType::BOOKMARK,
                                   sw::mark::InsertMode::New);
                }

                rSh.SwCursorShell::Pop(SwCursorShell::PopMode::DeleteCurrent);
                rSh.EndAllAction();
                rSh.LockView(false);
                rDoc->GetIDocumentUndoRedo().EndUndo(SwUndoId::INSERT_PAGE_NUMBER, nullptr);
            }
            pDlg->disposeOnce();
        });
        rReq.Done();
    }
    break;
    case FN_UPDATE_TEXT_FORMFIELD:
    {
        // This updates a single fieldmark under the current cursor.
        OUString aFieldType;
        const SfxStringItem* pFieldType = rReq.GetArg<SfxStringItem>(FN_PARAM_1);
        if (pFieldType)
        {
            aFieldType = pFieldType->GetValue();
        }
        OUString aFieldCommandPrefix;
        const SfxStringItem* pFieldCommandPrefix = rReq.GetArg<SfxStringItem>(FN_PARAM_2);
        if (pFieldCommandPrefix)
        {
            aFieldCommandPrefix = pFieldCommandPrefix->GetValue();
        }
        uno::Sequence<beans::PropertyValue> aField;
        const SfxUnoAnyItem* pFields = rReq.GetArg<SfxUnoAnyItem>(FN_PARAM_3);
        if (pFields)
        {
            pFields->GetValue() >>= aField;
        }

        IDocumentMarkAccess& rIDMA = *rSh.getIDocumentMarkAccess();
        SwPosition& rCursor = *rSh.GetCursor()->GetPoint();
        sw::mark::IFieldmark* pFieldmark = rIDMA.getInnerFieldmarkFor(rCursor);
        if (!pFieldmark)
        {
            break;
        }

        if (pFieldmark->GetFieldname() != aFieldType)
        {
            break;
        }

        auto itParam = pFieldmark->GetParameters()->find(ODF_CODE_PARAM);
        if (itParam == pFieldmark->GetParameters()->end())
        {
            break;
        }

        OUString aCommand;
        itParam->second >>= aCommand;
        if (!aCommand.startsWith(aFieldCommandPrefix))
        {
            break;
        }

        rSh.GetDoc()->GetIDocumentUndoRedo().StartUndo(SwUndoId::UPDATE_FORM_FIELD, nullptr);
        rSh.StartAction();
        comphelper::SequenceAsHashMap aMap(aField);
        itParam->second = aMap[u"FieldCommand"_ustr];
        SwPaM aPaM(pFieldmark->GetMarkPos(), pFieldmark->GetOtherMarkPos());
        aPaM.Normalize();
        // Skip field start & separator.
        aPaM.GetPoint()->AdjustContent(2);
        // Skip field end.
        aPaM.GetMark()->AdjustContent(-1);
        rSh.GetDoc()->getIDocumentContentOperations().DeleteAndJoin(aPaM);
        OUString aFieldResult;
        aMap[u"FieldResult"_ustr] >>= aFieldResult;
        SwTranslateHelper::PasteHTMLToPaM(rSh, &aPaM, aFieldResult.toUtf8());

        rSh.EndAction();
        rSh.GetDoc()->GetIDocumentUndoRedo().EndUndo(SwUndoId::UPDATE_FORM_FIELD, nullptr);
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
                // tdf#137568 do not offer comment formatting, if no comments are present
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

        case FN_UPDATE_SEL_FIELD:
            {
                pField = rSh.GetCurField();

                if (!pField)
                    rSet.DisableItem( nWhich );
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
                    SfxViewFrame& rVFrame = GetView().GetViewFrame();
                    //#i5788# prevent closing of the field dialog while a modal dialog ( Input field dialog ) is active
                    if(!rVFrame.IsInModalMode() &&
                        rVFrame.KnowsChildWindow(FN_INSERT_FIELD) && !rVFrame.HasChildWindow(FN_INSERT_FIELD_DATA_ONLY) )
                        rSet.Put(SfxBoolItem( FN_INSERT_FIELD, rVFrame.HasChildWindow(nWhich)));
                    else
                        rSet.DisableItem(FN_INSERT_FIELD);
                }
            }
            break;

        case FN_INSERT_REF_FIELD:
            {
                SfxViewFrame& rVFrame = GetView().GetViewFrame();
                if ( !rVFrame.KnowsChildWindow(FN_INSERT_FIELD)
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
                    rSet.Put(SfxBoolItem( nWhich, GetView().GetViewFrame().HasChildWindow(FN_INSERT_FIELD)));
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
                // tdf#86188, tdf#135794: Allow disabling comment insertion
                // on footnote/endnote/header/frames for better OOXML interoperability
                else if (!officecfg::Office::Compatibility::View::AllowCommentsInFootnotes::get() &&
                         (rSh.IsCursorInFootnote() || rSh.IsInHeaderFooter() ||
                          rSh.GetCurrFlyFrame(/*bCalcFrame=*/false)))
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
                sw::mark::IFieldmark* pFieldBM = GetShell().getIDocumentMarkAccess()->getInnerFieldmarkFor(aCursorPos);
                if ((!pFieldBM || pFieldBM->GetFieldname() != ODF_FORMTEXT)
                    && aCursorPos.GetContentIndex() > 0)
                {
                    SwPosition aPos(*aCursorPos.GetContentNode(), aCursorPos.GetContentIndex() - 1);
                    pFieldBM = GetShell().getIDocumentMarkAccess()->getInnerFieldmarkFor(aPos);
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
    const OUString& rReplacementText = rHlnkItem.GetReplacementText();
    sal_uInt16 nType =  o3tl::narrowing<sal_uInt16>(rHlnkItem.GetInsertMode());
    nType &= ~HLINK_HTMLMODE;
    const SvxMacroTableDtor* pMacroTable = rHlnkItem.GetMacroTable();

    SwWrtShell& rSh = GetShell();

    if( !(rSh.GetSelectionType() & SelectionType::Text) )
        return;

    rSh.StartAction();
    SfxItemSetFixed<RES_TXTATR_INETFMT, RES_TXTATR_INETFMT> aSet(GetPool());
    rSh.GetCurAttr( aSet );

    if(SfxItemState::SET == aSet.GetItemState(RES_TXTATR_INETFMT, false))
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
            // inserting mention
            if (comphelper::LibreOfficeKit::isActive() && !rReplacementText.isEmpty())
            {
                SwPaM* pCursorPos = rSh.GetCursor();
                // move cursor backwards to select @mention
                for(int i=0; i < rReplacementText.getLength(); i++)
                    pCursorPos->Move(fnMoveBackward);
                rSh.InsertURL( aINetFormat, rName, false );
            }
            else
            {
                rSh.InsertURL( aINetFormat, rName, true );
            }
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
