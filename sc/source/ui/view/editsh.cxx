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

#include <comphelper/string.hxx>
#include <comphelper/lok.hxx>
#include <scitems.hxx>
#include <editeng/eeitem.hxx>
#include <i18nutil/unicode.hxx>
#include <i18nutil/transliteration.hxx>

#include <svx/clipfmtitem.hxx>
#include <svx/svxdlg.hxx>
#include <editeng/contouritem.hxx>
#include <editeng/outliner.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/editeng.hxx>
#include <editeng/editview.hxx>
#include <editeng/escapementitem.hxx>
#include <editeng/flditem.hxx>
#include <editeng/flstitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/urlfieldhelper.hxx>
#include <editeng/editund2.hxx>
#include <svx/hlnkitem.hxx>
#include <vcl/EnumContext.hxx>
#include <editeng/postitem.hxx>
#include <editeng/scripttypeitem.hxx>
#include <editeng/shdditem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/wghtitem.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/msg.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/request.hxx>
#include <sfx2/viewfrm.hxx>
#include <svtools/cliplistener.hxx>
#include <svl/whiter.hxx>
#include <sot/formats.hxx>
#include <vcl/transfer.hxx>
#include <vcl/unohelp2.hxx>
#include <svl/stritem.hxx>
#include <editeng/colritem.hxx>

#include <editsh.hxx>
#include <global.hxx>
#include <appoptio.hxx>
#include <scmod.hxx>
#include <sc.hrc>
#include <inputhdl.hxx>
#include <viewutil.hxx>
#include <viewdata.hxx>
#include <document.hxx>
#include <reffind.hxx>
#include <tabvwsh.hxx>
#include <editutil.hxx>
#include <globstr.hrc>
#include <scresid.hxx>
#include <gridwin.hxx>

#define ShellClass_ScEditShell
#include <scslots.hxx>

#include <scui_def.hxx>
#include <scabstdlg.hxx>
#include <memory>

using namespace ::com::sun::star;


SFX_IMPL_INTERFACE(ScEditShell, SfxShell)

void ScEditShell::InitInterface_Impl()
{
    GetStaticInterface()->RegisterPopupMenu(u"celledit"_ustr);
}

ScEditShell::ScEditShell(EditView* pView, ScViewData& rData) :
    pEditView       (pView),
    rViewData       (rData),
    bPastePossible  (false),
    bIsInsertMode   (true)
{
    SetPool( pEditView->getEditEngine().GetEmptyItemSet().GetPool() );
    SetUndoManager( &pEditView->getEditEngine().GetUndoManager() );
    SetName(u"EditCell"_ustr);
    SfxShell::SetContextName(vcl::EnumContext::GetContextName(vcl::EnumContext::Context::EditCell));
}

ScEditShell::~ScEditShell()
{
    if ( mxClipEvtLstnr.is() )
    {
        mxClipEvtLstnr->RemoveListener( rViewData.GetActiveWin() );

        //  The listener may just now be waiting for the SolarMutex and call the link
        //  afterwards, in spite of RemoveListener. So the link has to be reset, too.
        mxClipEvtLstnr->ClearCallbackLink();
    }
}

ScInputHandler* ScEditShell::GetMyInputHdl()
{
    return SC_MOD()->GetInputHdl( rViewData.GetViewShell() );
}

void ScEditShell::SetEditView(EditView* pView)
{
    pEditView = pView;
    pEditView->SetInsertMode( bIsInsertMode );
    SetPool( pEditView->getEditEngine().GetEmptyItemSet().GetPool() );
    SetUndoManager( &pEditView->getEditEngine().GetUndoManager() );
}

static void lcl_RemoveAttribs( EditView& rEditView )
{
    ScEditEngineDefaulter* pEngine = static_cast<ScEditEngineDefaulter*>(&rEditView.getEditEngine());

    bool bOld = pEngine->SetUpdateLayout(false);

    OUString aName = ScResId( STR_UNDO_DELETECONTENTS );
    ViewShellId nViewShellId(-1);
    if (ScTabViewShell* pViewSh = ScTabViewShell::GetActiveViewShell())
        nViewShellId = pViewSh->GetViewShellId();
    pEngine->GetUndoManager().EnterListAction( aName, aName, 0, nViewShellId );

    rEditView.RemoveAttribs(true);
    pEngine->RepeatDefaults();      // paragraph attributes from cell formats must be preserved

    pEngine->GetUndoManager().LeaveListAction();

    pEngine->SetUpdateLayout(bOld);
}

static void lclInsertCharacter( EditView* pTableView, EditView* pTopView, sal_Unicode cChar )
{
    OUString aString( cChar );
    if( pTableView )
        pTableView->InsertText( aString );
    if( pTopView )
        pTopView->InsertText( aString );
}

void ScEditShell::Execute( SfxRequest& rReq )
{
    const SfxItemSet*   pReqArgs    = rReq.GetArgs();
    sal_uInt16              nSlot   = rReq.GetSlot();
    SfxBindings&        rBindings   = rViewData.GetBindings();

    ScInputHandler* pHdl = GetMyInputHdl();
    OSL_ENSURE(pHdl,"no ScInputHandler");

    EditView* pTopView   = pHdl->GetTopView();      // Has thee input cell the focus?
    EditView* pTableView = pHdl->GetTableView();

    OSL_ENSURE(pTableView,"no EditView :-(");
    /* #i91683# No EditView if spell-check dialog is active and positioned on
     * an error and user immediately (without double click or F2) selected a
     * text portion of that cell with the mouse and wanted to modify it. */
    /* FIXME: Bailing out only cures the symptom and prevents a crash, no edit
     * action is possible. A real fix somehow would need to create a valid
     * EditView from the spell-check view. */
    if (!pTableView)
        return;

    EditEngine& rEngine = pTableView->getEditEngine();

    pHdl->DataChanging();
    bool bSetSelIsRef = false;
    bool bSetModified = true;

    switch ( nSlot )
    {
        case SID_ATTR_INSERT:
        case FID_INS_CELL_CONTENTS: // Insert taste, while defined as Acc
            bIsInsertMode = !pTableView->IsInsertMode();
            pTableView->SetInsertMode( bIsInsertMode );
            if (pTopView)
                pTopView->SetInsertMode( bIsInsertMode );
            rBindings.Invalidate( SID_ATTR_INSERT );
            break;

        case SID_THES:
            {
                OUString aReplaceText;
                const SfxStringItem* pItem2 = rReq.GetArg(FN_PARAM_THES_WORD_REPLACE);
                if (pItem2)
                    aReplaceText = pItem2->GetValue();
                if (!aReplaceText.isEmpty())
                    ReplaceTextWithSynonym( *pEditView, aReplaceText );
            }
            break;

        case SID_COPY:
            pTableView->Copy();
            bSetModified = false;
            break;

        case SID_CUT:
            pTableView->Cut();
            if (pTopView)
                pTopView->DeleteSelected();
            break;

        case SID_PASTE:
        {
            EVControlBits nControl = pTableView->GetControlWord();
            if (pTopView)
            {
                pTopView->Paste();
                pTableView->SetControlWord(nControl | EVControlBits::SINGLELINEPASTE);
            }

            pTableView->PasteSpecial();
            pTableView->SetControlWord(nControl);
        }
        break;

        case SID_DELETE:
            pTableView->DeleteSelected();
            if (pTopView)
                pTopView->DeleteSelected();
            break;

        case SID_CELL_FORMAT_RESET:                 // "Standard"
            lcl_RemoveAttribs( *pTableView );
            if ( pTopView )
                lcl_RemoveAttribs( *pTopView );
            break;

        case SID_CLIPBOARD_FORMAT_ITEMS:
            {
                SotClipboardFormatId nFormat = SotClipboardFormatId::NONE;
                const SfxPoolItem* pItem;
                if ( pReqArgs && pReqArgs->GetItemState(nSlot, true, &pItem) == SfxItemState::SET )
                    if (auto pIntItem = dynamic_cast<const SfxUInt32Item*>( pItem))
                        nFormat = static_cast<SotClipboardFormatId>(pIntItem->GetValue());

                if ( nFormat != SotClipboardFormatId::NONE )
                {
                    if (SotClipboardFormatId::STRING == nFormat)
                        pTableView->Paste();
                    else
                        pTableView->PasteSpecial();

                    if (pTopView)
                        pTopView->Paste();
                }
            }
            break;

        case SID_PASTE_SPECIAL:
            {
                SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                ScopedVclPtr<SfxAbstractPasteDialog> pDlg(pFact->CreatePasteDialog(rViewData.GetDialogParent()));
                SotClipboardFormatId nFormat = SotClipboardFormatId::NONE;
                pDlg->Insert( SotClipboardFormatId::STRING, OUString() );
                pDlg->Insert( SotClipboardFormatId::RTF,    OUString() );
                pDlg->Insert( SotClipboardFormatId::RICHTEXT,    OUString() );
                // Do not offer SotClipboardFormatId::STRING_TSVC for
                // in-cell paste.

                TransferableDataHelper aDataHelper(
                    TransferableDataHelper::CreateFromSystemClipboard( rViewData.GetActiveWin() ) );

                nFormat = pDlg->GetFormat( aDataHelper.GetTransferable() );
                pDlg.disposeAndClear();

                // while the dialog was open, edit mode may have been stopped
                if (!SC_MOD()->IsInputMode())
                    return;

                if (nFormat != SotClipboardFormatId::NONE)
                {
                    if (SotClipboardFormatId::STRING == nFormat)
                        pTableView->Paste();
                    else
                        pTableView->PasteSpecial();

                    if (pTopView)
                        pTopView->Paste();
                }

                if (vcl::Window* pViewWindow = pTopView ? pTopView->GetWindow() : nullptr)
                    pViewWindow->GrabFocus();
            }
            break;

        case SID_PASTE_UNFORMATTED:
            {
                pTableView->Paste();

                if (pTopView)
                {
                    pTopView->Paste();
                    if (vcl::Window* pViewWindow = pTopView->GetWindow())
                        pViewWindow->GrabFocus();
                }
            }
            break;

        case SID_SELECTALL:
            {
                sal_Int32 nPar = rEngine.GetParagraphCount();
                if (nPar)
                {
                    sal_Int32 nLen = rEngine.GetTextLen(nPar-1);
                    pTableView->SetSelection(ESelection(0,0,nPar-1,nLen));
                    if (pTopView)
                        pTopView->SetSelection(ESelection(0,0,nPar-1,nLen));
                    rBindings.Invalidate( SID_ATTR_CHAR_FONT );
                    rBindings.Invalidate( SID_ATTR_CHAR_FONTHEIGHT );
                    rBindings.Invalidate( SID_ATTR_CHAR_WEIGHT );
                    rBindings.Invalidate( SID_ATTR_CHAR_POSTURE );
                    rBindings.Invalidate( SID_ATTR_CHAR_UNDERLINE );
                    rBindings.Invalidate( SID_ATTR_CHAR_STRIKEOUT );
                    rBindings.Invalidate( SID_ATTR_CHAR_SHADOWED );
                    rBindings.Invalidate( SID_ATTR_CHAR_KERNING );
                    rBindings.Invalidate( SID_ATTR_CHAR_COLOR );
                    rBindings.Invalidate( SID_SET_SUPER_SCRIPT );
                    rBindings.Invalidate( SID_SET_SUB_SCRIPT );
                }
            }
            return;
        case SID_UNICODE_NOTATION_TOGGLE:
            {
                EditView* pActiveView = pHdl->GetActiveView();
                if( pActiveView )
                {
                    OUString sInput = rEngine.GetText();
                    ESelection aSel( pActiveView->GetSelection() );
                    if( aSel.HasRange() )
                        sInput = pActiveView->GetSelected();

                    if( aSel.nStartPos > aSel.nEndPos )
                        aSel.nEndPos = aSel.nStartPos;

                    //calculate a valid end-position by reading logical characters
                    sal_Int32 nUtf16Pos=0;
                    while( (nUtf16Pos < sInput.getLength()) && (nUtf16Pos < aSel.nEndPos) )
                    {
                        sInput.iterateCodePoints(&nUtf16Pos);
                        if( nUtf16Pos > aSel.nEndPos )
                            aSel.nEndPos = nUtf16Pos;
                    }

                    ToggleUnicodeCodepoint aToggle;
                    while( nUtf16Pos && aToggle.AllowMoreInput( sInput[nUtf16Pos-1]) )
                        --nUtf16Pos;
                    OUString sReplacement = aToggle.ReplacementString();
                    if( !sReplacement.isEmpty() )
                    {
                        aSel.nStartPos = aSel.nEndPos - aToggle.StringToReplace().getLength();
                        pTableView->SetSelection( aSel );
                        pTableView->InsertText(sReplacement, true);
                        if( pTopView )
                        {
                            pTopView->SetSelection( aSel );
                            pTopView->InsertText(sReplacement, true);
                        }
                    }
                }
            }
            break;

        case SID_CHARMAP:
            {
                SvtScriptType nScript = pTableView->GetSelectedScriptType();
                sal_uInt16 nFontWhich = ( nScript == SvtScriptType::ASIAN ) ? EE_CHAR_FONTINFO_CJK :
                                ( ( nScript == SvtScriptType::COMPLEX ) ? EE_CHAR_FONTINFO_CTL :
                                                                        EE_CHAR_FONTINFO );
                auto const attribs = pTableView->GetAttribs();
                const SvxFontItem& rItem = static_cast<const SvxFontItem&>(
                            attribs.Get(nFontWhich));

                OUString aString;
                std::shared_ptr<SvxFontItem> aNewItem(std::make_shared<SvxFontItem>(EE_CHAR_FONTINFO));

                const SfxItemSet *pArgs = rReq.GetArgs();
                const SfxPoolItem* pItem = nullptr;
                if( pArgs )
                    pArgs->GetItemState(SID_CHARMAP, false, &pItem);

                if ( pItem )
                {
                    aString = static_cast<const SfxStringItem*>(pItem)->GetValue();
                    const SfxStringItem* pFontItem = pArgs->GetItemIfSet( SID_ATTR_SPECIALCHAR, false);
                    if ( pFontItem )
                    {
                        const OUString& aFontName(pFontItem->GetValue());
                        vcl::Font aFont(aFontName, Size(1,1)); // Size just because CTOR
                        // tdf#125054 see comment in drtxob.cxx, same ID
                        aNewItem = std::make_shared<SvxFontItem>(
                            aFont.GetFamilyType(), aFont.GetFamilyName(),
                            aFont.GetStyleName(), aFont.GetPitch(),
                            aFont.GetCharSet(), ATTR_FONT);
                    }
                    else
                    {
                        aNewItem.reset(rItem.Clone());
                    }

                    // tdf#125054 force Item to correct intended ID
                    aNewItem->SetWhich(EE_CHAR_FONTINFO);
                }
                else
                {
                    ScViewUtil::ExecuteCharMap(rItem, *rViewData.GetViewShell());

                    // while the dialog was open, edit mode may have been stopped
                    if (!SC_MOD()->IsInputMode())
                        return;
                }

                if ( !aString.isEmpty() )
                {
                    //  if string contains WEAK characters, set all fonts
                    SvtScriptType nSetScript;
                    ScDocument& rDoc = rViewData.GetDocument();
                    if ( rDoc.HasStringWeakCharacters( aString ) )
                        nSetScript = SvtScriptType::LATIN | SvtScriptType::ASIAN | SvtScriptType::COMPLEX;
                    else
                        nSetScript = rDoc.GetStringScriptType( aString );

                    SfxItemSet aSet( pTableView->GetEmptyItemSet() );
                    SvxScriptSetItem aSetItem( SID_ATTR_CHAR_FONT, GetPool() );
                    aSetItem.PutItemForScriptType( nSetScript, *aNewItem );
                    aSet.Put( aSetItem.GetItemSet(), false );

                    // SetAttribs on the View selects a word, when nothing is selected
                    pTableView->getEditEngine().QuickSetAttribs( aSet, pTableView->GetSelection() );
                    pTableView->InsertText(aString);
                    if (pTopView)
                        pTopView->InsertText(aString);

                    SfxStringItem aStringItem( SID_CHARMAP, aString );
                    SfxStringItem aFontItem( SID_ATTR_SPECIALCHAR, aNewItem->GetFamilyName() );
                    rReq.AppendItem( aFontItem );
                    rReq.AppendItem( aStringItem );
                    rReq.Done();

                }

                if (vcl::Window* pViewWindow = pTopView ? pTopView->GetWindow() : nullptr)
                    pViewWindow->GrabFocus();
            }
            break;

        case FID_INSERT_NAME:
            {
                ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();

                ScopedVclPtr<AbstractScNamePasteDlg> pDlg(pFact->CreateScNamePasteDlg(rViewData.GetDialogParent(), rViewData.GetDocShell()));
                short nRet = pDlg->Execute();
                // pDlg is needed below

                // while the dialog was open, edit mode may have been stopped
                if (!SC_MOD()->IsInputMode())
                    return;

                if ( nRet == BTN_PASTE_NAME )
                {
                    std::vector<OUString> aNames = pDlg->GetSelectedNames();
                    if (!aNames.empty())
                    {
                        OUStringBuffer aBuffer;
                        for (const auto& rName : aNames)
                        {
                            aBuffer.append(rName + " ");
                        }
                        const OUString s = aBuffer.makeStringAndClear();
                        pTableView->InsertText(s);
                        if (pTopView)
                            pTopView->InsertText(s);
                    }
                }
                pDlg.disposeAndClear();

                if (vcl::Window* pViewWindow = pTopView ? pTopView->GetWindow() : nullptr)
                    pViewWindow->GrabFocus();
            }
            break;

        case SID_CHAR_DLG_EFFECT:
        case SID_CHAR_DLG:
            {
                SfxItemSet aAttrs( pTableView->GetAttribs() );

                SfxObjectShell* pObjSh = rViewData.GetSfxDocShell();

                ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();

                ScopedVclPtr<SfxAbstractTabDialog> pDlg(pFact->CreateScCharDlg(
                    rViewData.GetDialogParent(), &aAttrs, pObjSh, false));
                if (nSlot == SID_CHAR_DLG_EFFECT)
                {
                    pDlg->SetCurPageId(u"fonteffects"_ustr);
                }
                short nRet = pDlg->Execute();
                // pDlg is needed below

                // while the dialog was open, edit mode may have been stopped
                if (!SC_MOD()->IsInputMode())
                    return;

                if ( nRet == RET_OK )
                {
                    const SfxItemSet* pOut = pDlg->GetOutputItemSet();
                    pTableView->SetAttribs( *pOut );
                }
            }
            break;

        case SID_TOGGLE_REL:
            {
                /* TODO: MLFORMULA: this should work also with multi-line formulas. */
                if (rEngine.GetParagraphCount() == 1)
                {
                    OUString aText = rEngine.GetText();
                    ESelection aSel = pEditView->GetSelection();    // current View

                    ScDocument& rDoc = rViewData.GetDocument();
                    ScRefFinder aFinder(aText, rViewData.GetCurPos(), rDoc, rDoc.GetAddressConvention());
                    aFinder.ToggleRel( aSel.nStartPos, aSel.nEndPos );
                    if (aFinder.GetFound())
                    {
                        const OUString& aNew = aFinder.GetText();
                        ESelection aNewSel( 0,aFinder.GetSelStart(), 0,aFinder.GetSelEnd() );
                        rEngine.SetText( aNew );
                        pTableView->SetSelection( aNewSel );
                        if ( pTopView )
                        {
                            pTopView->getEditEngine().SetText( aNew );
                            pTopView->SetSelection( aNewSel );
                        }

                        // reference is being selected -> do not overwrite when typing
                        bSetSelIsRef = true;
                    }
                }
            }
            break;

        case SID_HYPERLINK_SETLINK:
            if( pReqArgs )
            {
                const SfxPoolItem* pItem;
                if ( pReqArgs->GetItemState( SID_HYPERLINK_SETLINK, true, &pItem ) == SfxItemState::SET )
                {
                    const SvxHyperlinkItem* pHyper = static_cast<const SvxHyperlinkItem*>(pItem);
                    const OUString& rName     = pHyper->GetName();
                    const OUString& rURL      = pHyper->GetURL();
                    const OUString& rTarget   = pHyper->GetTargetFrame();
                    SvxLinkInsertMode eMode = pHyper->GetInsertMode();

                    bool bCellLinksOnly
                        = (SC_MOD()->GetAppOptions().GetLinksInsertedLikeMSExcel()
                          && rViewData.GetSfxDocShell()->GetMedium()->GetFilter()->IsMSOFormat())
                          || comphelper::LibreOfficeKit::isActive();

                    bool bDone = false;
                    if ( (eMode == HLINK_DEFAULT || eMode == HLINK_FIELD) && !bCellLinksOnly )
                    {
                        std::unique_ptr<const SvxFieldData> aSvxFieldDataPtr(GetURLField());
                        const SvxURLField* pURLField(static_cast<const SvxURLField*>(aSvxFieldDataPtr.get()));
                        if ( pURLField )
                        {
                            // select old field

                            ESelection aSel = pTableView->GetSelection();
                            aSel.Adjust();
                            aSel.nEndPara = aSel.nStartPara;
                            aSel.nEndPos = aSel.nStartPos + 1;
                            pTableView->SetSelection( aSel );

                            // insert new field

                            SvxURLField aURLField( rURL, rName, SvxURLFormat::Repr );
                            aURLField.SetTargetFrame( rTarget );
                            SvxFieldItem aURLItem( aURLField, EE_FEATURE_FIELD );
                            pTableView->InsertField( aURLItem );
                            pTableView->SetSelection( aSel );       // select inserted field

                            // now also fields in the Top-View

                            if ( pTopView )
                            {
                                aSel = pTopView->GetSelection();
                                aSel.nEndPara = aSel.nStartPara;
                                aSel.nEndPos = aSel.nStartPos + 1;
                                pTopView->SetSelection( aSel );
                                pTopView->InsertField( aURLItem );
                                pTopView->SetSelection( aSel );     // select inserted field
                            }

                            bDone = true;
                        }
                    }

                    if (!bDone)
                    {
                        if (bCellLinksOnly)
                        {
                            sal_Int32 nPar = rEngine.GetParagraphCount();
                            if (nPar)
                            {
                                sal_Int32 nLen = rEngine.GetTextLen(nPar - 1);
                                pTableView->SetSelection(ESelection(0, 0, nPar - 1, nLen));
                                if (pTopView)
                                    pTopView->SetSelection(ESelection(0, 0, nPar - 1, nLen));
                            }
                        }
                        rViewData.GetViewShell()->
                            InsertURL( rName, rURL, rTarget, static_cast<sal_uInt16>(eMode) );

                        // when "Button", the InsertURL in ViewShell turns the EditShell  off
                        // thus the immediate return statement
                        return;
                    }
                }
            }
            break;
        case SID_EDIT_HYPERLINK:
            {
                // Ensure the field is selected first
                pEditView->SelectFieldAtCursor();
                rViewData.GetViewShell()->GetViewFrame().GetDispatcher()->Execute(
                    SID_HYPERLINK_DIALOG);
            }
        break;
        case SID_COPY_HYPERLINK_LOCATION:
            {
                const SvxFieldItem* pFieldItem
                    = pEditView->GetFieldAtSelection(/*AlsoCheckBeforeCursor=*/true);
                const SvxFieldData* pField = pFieldItem ? pFieldItem->GetField() : nullptr;
                if (const SvxURLField* pURLField = dynamic_cast<const SvxURLField*>(pField))
                {
                    uno::Reference<datatransfer::clipboard::XClipboard> xClipboard
                        = pEditView->GetClipboard();
                    vcl::unohelper::TextDataObject::CopyStringTo(pURLField->GetURL(), xClipboard, SfxViewShell::Current());
                }
            }
        break;
        case SID_REMOVE_HYPERLINK:
            {
                URLFieldHelper::RemoveURLField(*pEditView);
            }
        break;

        case FN_INSERT_SOFT_HYPHEN:
            lclInsertCharacter( pTableView, pTopView, CHAR_SHY );
        break;
        case FN_INSERT_HARDHYPHEN:
            lclInsertCharacter( pTableView, pTopView, CHAR_NBHY );
        break;
        case FN_INSERT_HARD_SPACE:
            lclInsertCharacter( pTableView, pTopView, CHAR_NBSP );
        break;
        case FN_INSERT_NNBSP:
            lclInsertCharacter( pTableView, pTopView, CHAR_NNBSP );
        break;
        case SID_INSERT_RLM:
            lclInsertCharacter( pTableView, pTopView, CHAR_RLM );
        break;
        case SID_INSERT_LRM:
            lclInsertCharacter( pTableView, pTopView, CHAR_LRM );
        break;
        case SID_INSERT_ZWSP:
            lclInsertCharacter( pTableView, pTopView, CHAR_ZWSP );
        break;
        case SID_INSERT_WJ:
            lclInsertCharacter( pTableView, pTopView, CHAR_WJ );
        break;
        case SID_INSERT_FIELD_SHEET:
        {
            SvxTableField aField(rViewData.GetTabNo());
            SvxFieldItem aItem(aField, EE_FEATURE_FIELD);
            pTableView->InsertField(aItem);
        }
        break;
        case SID_INSERT_FIELD_TITLE:
        {
            SvxFileField aField;
            SvxFieldItem aItem(aField, EE_FEATURE_FIELD);
            pTableView->InsertField(aItem);
        }
        break;
        case SID_INSERT_FIELD_DATE_VAR:
        {
            SvxDateField aField;
            SvxFieldItem aItem(aField, EE_FEATURE_FIELD);
            pTableView->InsertField(aItem);
        }
        break;
    }

    pHdl->DataChanged(false, bSetModified);
    if (bSetSelIsRef)
        pHdl->SetSelIsRef(true);
}

static void lcl_DisableAll( SfxItemSet& rSet )    // disable all slots
{
    SfxWhichIter aIter( rSet );
    sal_uInt16 nWhich = aIter.FirstWhich();
    while (nWhich)
    {
        rSet.DisableItem( nWhich );
        nWhich = aIter.NextWhich();
    }
}

void ScEditShell::GetState( SfxItemSet& rSet )
{
    // When deactivating the view, edit mode is stopped, but the EditShell is left active
    // (a shell can't be removed from within Deactivate). In that state, the EditView isn't inserted
    // into the EditEngine, so it can have an invalid selection and must not be used.
    ScInputHandler* pHdl = GetMyInputHdl();
    if ( !rViewData.HasEditView( rViewData.GetActivePart() ) )
    {
        lcl_DisableAll( rSet );

        // Some items are actually useful and still applicable when in formula building mode: enable
        if (pHdl && pHdl->IsFormulaMode())
        {
            rSet.ClearItem(SID_TOGGLE_REL); //  F4 Cycle Cell Reference Types
            rSet.ClearItem(SID_CHARMAP); // Insert Special Characters
        }
        return;
    }

    EditView* pActiveView = pHdl ? pHdl->GetActiveView() : pEditView;

    SfxWhichIter aIter( rSet );
    sal_uInt16 nWhich = aIter.FirstWhich();
    while (nWhich)
    {
        switch (nWhich)
        {
            case SID_ATTR_INSERT:   // Status row
                {
                    if ( pActiveView )
                        rSet.Put( SfxBoolItem( nWhich, pActiveView->IsInsertMode() ) );
                    else
                    {
                        // Here the code used to pass the value 42 and it used
                        // to "work" without warnings because the SfxBoolItem
                        // was based on 'sal_Bool', which is actually 'unsigned
                        // char'. But now it uses actual 'bool', and passing 42
                        // for a 'bool' parameter causes a warning at least with
                        // MSVC.  So use 'true'. I really really hope there is
                        // not code somewhere that retrieves this "boolean" item
                        // and checks it value for the magic value 42...
                        rSet.Put( SfxBoolItem( nWhich,  true) );
                    }
                }
                break;

            case SID_HYPERLINK_GETLINK:
                {
                    SvxHyperlinkItem aHLinkItem;
                    bool bCellLinksOnly
                        = (SC_MOD()->GetAppOptions().GetLinksInsertedLikeMSExcel()
                          && rViewData.GetSfxDocShell()->GetMedium()->GetFilter()->IsMSOFormat())
                          || comphelper::LibreOfficeKit::isActive();
                    std::unique_ptr<const SvxFieldData> aSvxFieldDataPtr(GetURLField());
                    const SvxURLField* pURLField(static_cast<const SvxURLField*>(aSvxFieldDataPtr.get()));
                    if (!bCellLinksOnly)
                    {
                        if (pURLField)
                        {
                            aHLinkItem.SetName(pURLField->GetRepresentation());
                            aHLinkItem.SetURL(pURLField->GetURL());
                            aHLinkItem.SetTargetFrame(pURLField->GetTargetFrame());
                        }
                        else if (pActiveView)
                        {
                            // use selected text as name for urls
                            OUString sReturn = pActiveView->GetSelected();
                            sReturn = sReturn.copy(
                                0, std::min(sReturn.getLength(), static_cast<sal_Int32>(255)));
                            aHLinkItem.SetName(comphelper::string::stripEnd(sReturn, ' '));
                        }
                    }
                    else
                    {
                        if (!pURLField)
                        {
                            aSvxFieldDataPtr = GetFirstURLFieldFromCell();
                            pURLField = static_cast<const SvxURLField*>(aSvxFieldDataPtr.get());
                        }
                        if (pURLField)
                        {
                            aHLinkItem.SetURL(pURLField->GetURL());
                            aHLinkItem.SetTargetFrame(pURLField->GetTargetFrame());
                        }
                        ScDocument& rDoc = rViewData.GetDocument();
                        SCCOL nPosX = rViewData.GetCurX();
                        SCROW nPosY = rViewData.GetCurY();
                        SCTAB nTab = rViewData.GetTabNo();
                        aHLinkItem.SetName(rDoc.GetString(nPosX, nPosY, nTab));
                    }
                    rSet.Put(aHLinkItem);
                }
                break;

            case SID_EDIT_HYPERLINK:
            case SID_COPY_HYPERLINK_LOCATION:
            case SID_REMOVE_HYPERLINK:
                {
                    if (!URLFieldHelper::IsCursorAtURLField(*pEditView,
                                                            /*AlsoCheckBeforeCursor=*/true))
                        rSet.DisableItem (nWhich);
                }
                break;

            case SID_TRANSLITERATE_HALFWIDTH:
            case SID_TRANSLITERATE_FULLWIDTH:
            case SID_TRANSLITERATE_HIRAGANA:
            case SID_TRANSLITERATE_KATAKANA:
            case SID_INSERT_RLM:
            case SID_INSERT_LRM:
                ScViewUtil::HideDisabledSlot( rSet, rViewData.GetBindings(), nWhich );
            break;

            case SID_THES:
                {
                    OUString        aStatusVal;
                    LanguageType    nLang = LANGUAGE_NONE;
                    bool bIsLookUpWord = pActiveView &&
                        GetStatusValueForThesaurusFromContext(aStatusVal, nLang, *pActiveView);
                    rSet.Put( SfxStringItem( SID_THES, aStatusVal ) );

                    // disable thesaurus context menu entry if there is nothing to look up
                    bool bCanDoThesaurus = ScModule::HasThesaurusLanguage( nLang );
                    if (!bIsLookUpWord || !bCanDoThesaurus)
                        rSet.DisableItem( SID_THES );
                }
                break;
            case SID_INSERT_FIELD_SHEET:
            case SID_INSERT_FIELD_TITLE:
            case SID_INSERT_FIELD_DATE_VAR:
            break;
            case SID_COPY:
            case SID_CUT:
                if (GetObjectShell() && GetObjectShell()->isContentExtractionLocked())
                {
                    rSet.DisableItem(SID_COPY);
                    rSet.DisableItem(SID_CUT);
                }
                break;

        }
        nWhich = aIter.NextWhich();
    }
}

std::unique_ptr<const SvxFieldData> ScEditShell::GetURLField()
{
    ScInputHandler* pHdl = GetMyInputHdl();
    EditView* pActiveView = pHdl ? pHdl->GetActiveView() : pEditView;
    if (!pActiveView)
        return std::unique_ptr<const SvxFieldData>();

    const SvxFieldData* pField = pActiveView->GetFieldUnderMouseOrInSelectionOrAtCursor();
    if (auto pURLField = dynamic_cast<const SvxURLField*>(pField))
        return pURLField->Clone();

    return std::unique_ptr<const SvxFieldData>();
}

std::unique_ptr<const SvxFieldData> ScEditShell::GetFirstURLFieldFromCell()
{
    EditEngine& rEditEngine = GetEditView()->getEditEngine();
    sal_Int32 nParaCount = rEditEngine.GetParagraphCount();
    for (sal_Int32 nPara = 0; nPara < nParaCount; ++nPara)
    {
        ESelection aSel(nPara, 0);
        std::vector<sal_Int32> aPosList;
        rEditEngine.GetPortions(nPara, aPosList);
        for (const auto& rPos : aPosList)
        {
            aSel.nEndPos = rPos;

            SfxItemSet aEditSet(rEditEngine.GetAttribs(aSel));
            if (aSel.nStartPos + 1 == aSel.nEndPos)
            {
                // test if the character is a text field
                if (const SvxFieldItem* pItem = aEditSet.GetItemIfSet(EE_FEATURE_FIELD, false))
                {
                    const SvxFieldData* pField = pItem->GetField();
                    if (const SvxURLField* pUrlField = dynamic_cast<const SvxURLField*>(pField))
                    {
                        return pUrlField->Clone();
                    }
                }
            }
            aSel.nStartPos = aSel.nEndPos;
        }
    }

    return std::unique_ptr<const SvxFieldData>();
}

IMPL_LINK( ScEditShell, ClipboardChanged, TransferableDataHelper*, pDataHelper, void )
{
    bPastePossible = ( pDataHelper->HasFormat( SotClipboardFormatId::STRING )
            || pDataHelper->HasFormat( SotClipboardFormatId::RTF )
            || pDataHelper->HasFormat( SotClipboardFormatId::RICHTEXT ));

    SfxBindings& rBindings = rViewData.GetBindings();
    rBindings.Invalidate( SID_PASTE );
    rBindings.Invalidate( SID_PASTE_SPECIAL );
    rBindings.Invalidate( SID_PASTE_UNFORMATTED );
    rBindings.Invalidate( SID_CLIPBOARD_FORMAT_ITEMS );
}

void ScEditShell::GetClipState( SfxItemSet& rSet )
{
    // Do not offer SotClipboardFormatId::STRING_TSVC for in-cell paste.

    if ( !mxClipEvtLstnr.is() )
    {
        // create listener
        mxClipEvtLstnr = new TransferableClipboardListener( LINK( this, ScEditShell, ClipboardChanged ) );
        vcl::Window* pWin = rViewData.GetActiveWin();
        mxClipEvtLstnr->AddListener( pWin );

        // get initial state
        TransferableDataHelper aDataHelper( TransferableDataHelper::CreateFromSystemClipboard( rViewData.GetActiveWin() ) );
        bPastePossible = ( aDataHelper.HasFormat( SotClipboardFormatId::STRING )
                || aDataHelper.HasFormat( SotClipboardFormatId::RTF )
                || aDataHelper.HasFormat( SotClipboardFormatId::RICHTEXT ) );
    }

    SfxWhichIter aIter( rSet );
    sal_uInt16 nWhich = aIter.FirstWhich();
    while (nWhich)
    {
        switch (nWhich)
        {
            case SID_PASTE:
            case SID_PASTE_SPECIAL:
            case SID_PASTE_UNFORMATTED:
                if( !bPastePossible )
                    rSet.DisableItem( nWhich );
                break;
            case SID_CLIPBOARD_FORMAT_ITEMS:
                if( bPastePossible )
                {
                    SvxClipboardFormatItem aFormats( SID_CLIPBOARD_FORMAT_ITEMS );
                    TransferableDataHelper aDataHelper(
                            TransferableDataHelper::CreateFromSystemClipboard( rViewData.GetActiveWin() ) );

                    if ( aDataHelper.HasFormat( SotClipboardFormatId::STRING ) )
                        aFormats.AddClipbrdFormat( SotClipboardFormatId::STRING );
                    if ( aDataHelper.HasFormat( SotClipboardFormatId::RTF ) )
                        aFormats.AddClipbrdFormat( SotClipboardFormatId::RTF );

                    rSet.Put( aFormats );
                }
                else
                    rSet.DisableItem( nWhich );
                break;
        }
        nWhich = aIter.NextWhich();
    }
}

static void lcl_InvalidateUnder( SfxBindings& rBindings )
{
    rBindings.Invalidate( SID_ATTR_CHAR_UNDERLINE );
    rBindings.Invalidate( SID_ULINE_VAL_NONE );
    rBindings.Invalidate( SID_ULINE_VAL_SINGLE );
    rBindings.Invalidate( SID_ULINE_VAL_DOUBLE );
    rBindings.Invalidate( SID_ULINE_VAL_DOTTED );
}

void ScEditShell::ExecuteAttr(SfxRequest& rReq)
{
    SfxItemSet          aSet( pEditView->GetEmptyItemSet() );
    SfxBindings&        rBindings   = rViewData.GetBindings();
    const SfxItemSet*   pArgs       = rReq.GetArgs();
    sal_uInt16              nSlot       = rReq.GetSlot();

    switch ( nSlot )
    {
        case SID_ATTR_CHAR_FONTHEIGHT:
        case SID_ATTR_CHAR_FONT:
            {
                if (pArgs)
                {
                    // #i78017 establish the same behaviour as in Writer
                    SvtScriptType nScript = SvtScriptType::LATIN | SvtScriptType::ASIAN | SvtScriptType::COMPLEX;
                    if (nSlot == SID_ATTR_CHAR_FONT)
                    {
                        nScript = pEditView->GetSelectedScriptType();
                        if (nScript == SvtScriptType::NONE) nScript = ScGlobal::GetDefaultScriptType();
                    }

                    SfxItemPool& rPool = GetPool();
                    SvxScriptSetItem aSetItem( nSlot, rPool );
                    sal_uInt16 nWhich = rPool.GetWhichIDFromSlotID( nSlot );
                    aSetItem.PutItemForScriptType( nScript, pArgs->Get( nWhich ) );

                    aSet.Put( aSetItem.GetItemSet(), false );
                }
            }
            break;

        case SID_ATTR_CHAR_COLOR:
            {
                if (pArgs)
                {
                    aSet.Put( pArgs->Get( pArgs->GetPool()->GetWhichIDFromSlotID( nSlot ) ) );
                    rBindings.Invalidate( nSlot );
                }
            }
            break;

        //  Toggles

        case SID_ATTR_CHAR_WEIGHT:
            {
                // #i78017 establish the same behaviour as in Writer
                SvtScriptType nScript = SvtScriptType::LATIN | SvtScriptType::ASIAN | SvtScriptType::COMPLEX;

                SfxItemPool& rPool = GetPool();

                bool bOld = false;
                SvxScriptSetItem aOldSetItem( nSlot, rPool );
                aOldSetItem.GetItemSet().Put( pEditView->GetAttribs(), false );
                const SfxPoolItem* pCore = aOldSetItem.GetItemOfScript( nScript );
                if ( pCore && static_cast<const SvxWeightItem*>(pCore)->GetWeight() > WEIGHT_NORMAL )
                    bOld = true;

                SvxScriptSetItem aSetItem( nSlot, rPool );
                aSetItem.PutItemForScriptType( nScript,
                            SvxWeightItem( bOld ? WEIGHT_NORMAL : WEIGHT_BOLD, EE_CHAR_WEIGHT ) );
                aSet.Put( aSetItem.GetItemSet(), false );

                rBindings.Invalidate( nSlot );
            }
            break;

        case SID_ATTR_CHAR_POSTURE:
            {
                // #i78017 establish the same behaviour as in Writer
                SvtScriptType nScript = SvtScriptType::LATIN | SvtScriptType::ASIAN | SvtScriptType::COMPLEX;

                SfxItemPool& rPool = GetPool();

                bool bOld = false;
                SvxScriptSetItem aOldSetItem( nSlot, rPool );
                aOldSetItem.GetItemSet().Put( pEditView->GetAttribs(), false );
                const SfxPoolItem* pCore = aOldSetItem.GetItemOfScript( nScript );
                if ( pCore && static_cast<const SvxPostureItem*>(pCore)->GetValue() != ITALIC_NONE )
                    bOld = true;

                SvxScriptSetItem aSetItem( nSlot, rPool );
                aSetItem.PutItemForScriptType( nScript,
                            SvxPostureItem( bOld ? ITALIC_NONE : ITALIC_NORMAL, EE_CHAR_ITALIC ) );
                aSet.Put( aSetItem.GetItemSet(), false );

                rBindings.Invalidate( nSlot );
            }
            break;

        case SID_ULINE_VAL_NONE:
            aSet.Put( SvxUnderlineItem( LINESTYLE_NONE, EE_CHAR_UNDERLINE ) );
            lcl_InvalidateUnder( rBindings );
            break;

        case SID_ATTR_CHAR_UNDERLINE:
        case SID_ULINE_VAL_SINGLE:
        case SID_ULINE_VAL_DOUBLE:
        case SID_ULINE_VAL_DOTTED:
            {
                FontLineStyle eOld = pEditView->GetAttribs().Get(EE_CHAR_UNDERLINE).GetLineStyle();
                FontLineStyle eNew = eOld;
                switch (nSlot)
                {
                    case SID_ATTR_CHAR_UNDERLINE:
                        if ( pArgs )
                        {
                            const SvxTextLineItem& rTextLineItem = static_cast< const SvxTextLineItem& >( pArgs->Get( pArgs->GetPool()->GetWhichIDFromSlotID(nSlot) ) );
                            eNew = rTextLineItem.GetLineStyle();
                        }
                        else
                        {
                            eNew = ( eOld != LINESTYLE_NONE ) ? LINESTYLE_NONE : LINESTYLE_SINGLE;
                        }
                        break;
                    case SID_ULINE_VAL_SINGLE:
                        eNew = ( eOld == LINESTYLE_SINGLE ) ? LINESTYLE_NONE : LINESTYLE_SINGLE;
                        break;
                    case SID_ULINE_VAL_DOUBLE:
                        eNew = ( eOld == LINESTYLE_DOUBLE ) ? LINESTYLE_NONE : LINESTYLE_DOUBLE;
                        break;
                    case SID_ULINE_VAL_DOTTED:
                        eNew = ( eOld == LINESTYLE_DOTTED ) ? LINESTYLE_NONE : LINESTYLE_DOTTED;
                        break;
                }
                aSet.Put( SvxUnderlineItem( eNew, EE_CHAR_UNDERLINE ) );
                lcl_InvalidateUnder( rBindings );
            }
            break;

        case SID_ATTR_CHAR_OVERLINE:
            {
                FontLineStyle eOld = pEditView->GetAttribs().Get(EE_CHAR_OVERLINE).GetLineStyle();
                FontLineStyle eNew = ( eOld != LINESTYLE_NONE ) ? LINESTYLE_NONE : LINESTYLE_SINGLE;
                aSet.Put( SvxOverlineItem( eNew, EE_CHAR_OVERLINE ) );
                rBindings.Invalidate( nSlot );
            }
            break;

        case SID_ATTR_CHAR_STRIKEOUT:
            {
                bool bOld = pEditView->GetAttribs().Get(EE_CHAR_STRIKEOUT).GetValue() != STRIKEOUT_NONE;
                aSet.Put( SvxCrossedOutItem( bOld ? STRIKEOUT_NONE : STRIKEOUT_SINGLE, EE_CHAR_STRIKEOUT ) );
                rBindings.Invalidate( nSlot );
            }
            break;

        case SID_ATTR_CHAR_SHADOWED:
            {
                bool bOld = pEditView->GetAttribs().Get(EE_CHAR_SHADOW).GetValue();
                aSet.Put( SvxShadowedItem( !bOld, EE_CHAR_SHADOW ) );
                rBindings.Invalidate( nSlot );
            }
            break;

        case SID_ATTR_CHAR_CONTOUR:
            {
                bool bOld = pEditView->GetAttribs().Get(EE_CHAR_OUTLINE).GetValue();
                aSet.Put( SvxContourItem( !bOld, EE_CHAR_OUTLINE ) );
                rBindings.Invalidate( nSlot );
            }
            break;

        case SID_SET_SUPER_SCRIPT:
            {
                SvxEscapement eOld = static_cast<SvxEscapement>(pEditView->GetAttribs().Get(EE_CHAR_ESCAPEMENT).GetEnumValue());
                SvxEscapement eNew = (eOld == SvxEscapement::Superscript) ?
                                        SvxEscapement::Off : SvxEscapement::Superscript;
                aSet.Put( SvxEscapementItem( eNew, EE_CHAR_ESCAPEMENT ) );
                rBindings.Invalidate( nSlot );
            }
            break;
        case SID_SET_SUB_SCRIPT:
            {
                SvxEscapement eOld = static_cast<SvxEscapement>(pEditView->GetAttribs().Get(EE_CHAR_ESCAPEMENT).GetEnumValue());
                SvxEscapement eNew = (eOld == SvxEscapement::Subscript) ?
                                        SvxEscapement::Off : SvxEscapement::Subscript;
                aSet.Put( SvxEscapementItem( eNew, EE_CHAR_ESCAPEMENT ) );
                rBindings.Invalidate( nSlot );
            }
            break;
        case SID_ATTR_CHAR_KERNING:
            {
                if(pArgs)
                {
                    aSet.Put ( pArgs->Get(pArgs->GetPool()->GetWhichIDFromSlotID(nSlot)));
                    rBindings.Invalidate( nSlot );
                }
            }
            break;

        case SID_GROW_FONT_SIZE:
        case SID_SHRINK_FONT_SIZE:
            {
                SfxObjectShell* pObjSh = SfxObjectShell::Current();
                const SvxFontListItem* pFontListItem = static_cast<const SvxFontListItem*>
                        (pObjSh ? pObjSh->GetItem(SID_ATTR_CHAR_FONTLIST) : nullptr);
                const FontList* pFontList = pFontListItem ? pFontListItem->GetFontList() : nullptr;
                pEditView->ChangeFontSize( nSlot == SID_GROW_FONT_SIZE, pFontList );
                rBindings.Invalidate( SID_ATTR_CHAR_FONTHEIGHT );
            }
            break;
    }

    // apply

    EditEngine& rEngine = pEditView->getEditEngine();
    bool bOld = rEngine.SetUpdateLayout(false);

    pEditView->SetAttribs( aSet );

    rEngine.SetUpdateLayout(bOld);
    pEditView->Invalidate();

    ScInputHandler* pHdl = GetMyInputHdl();
    pHdl->SetModified();

    rReq.Done();
}

void ScEditShell::GetAttrState(SfxItemSet &rSet)
{
    if ( !rViewData.HasEditView( rViewData.GetActivePart() ) )
    {
        lcl_DisableAll( rSet );
        return;
    }

    SfxItemSet aAttribs = pEditView->GetAttribs();
    rSet.Put( aAttribs );

    //  choose font info according to selection script type

    SvtScriptType nScript = pEditView->GetSelectedScriptType();
    if (nScript == SvtScriptType::NONE) nScript = ScGlobal::GetDefaultScriptType();

    // #i55929# input-language-dependent script type (depends on input language if nothing selected)
    SvtScriptType nInputScript = nScript;
    if ( !pEditView->GetSelection().HasRange() )
    {
        LanguageType nInputLang = rViewData.GetActiveWin()->GetInputLanguage();
        if (nInputLang != LANGUAGE_DONTKNOW && nInputLang != LANGUAGE_SYSTEM)
            nInputScript = SvtLanguageOptions::GetScriptTypeOfLanguage( nInputLang );
    }

    // #i55929# according to spec, nInputScript is used for font and font height only
    if ( rSet.GetItemState( EE_CHAR_FONTINFO ) != SfxItemState::UNKNOWN )
        ScViewUtil::PutItemScript( rSet, aAttribs, EE_CHAR_FONTINFO, nInputScript );
    if ( rSet.GetItemState( EE_CHAR_FONTHEIGHT ) != SfxItemState::UNKNOWN )
        ScViewUtil::PutItemScript( rSet, aAttribs, EE_CHAR_FONTHEIGHT, nInputScript );
    if ( rSet.GetItemState( EE_CHAR_WEIGHT ) != SfxItemState::UNKNOWN )
        ScViewUtil::PutItemScript( rSet, aAttribs, EE_CHAR_WEIGHT, nScript );
    if ( rSet.GetItemState( EE_CHAR_ITALIC ) != SfxItemState::UNKNOWN )
        ScViewUtil::PutItemScript( rSet, aAttribs, EE_CHAR_ITALIC, nScript );

    // underline
    SfxItemState eState = aAttribs.GetItemState( EE_CHAR_UNDERLINE );
    if ( eState == SfxItemState::INVALID )
    {
        rSet.InvalidateItem( SID_ULINE_VAL_NONE );
        rSet.InvalidateItem( SID_ULINE_VAL_SINGLE );
        rSet.InvalidateItem( SID_ULINE_VAL_DOUBLE );
        rSet.InvalidateItem( SID_ULINE_VAL_DOTTED );
    }
    else
    {
        FontLineStyle eUnderline = aAttribs.Get(EE_CHAR_UNDERLINE).GetLineStyle();
        rSet.Put(SfxBoolItem(SID_ULINE_VAL_SINGLE, eUnderline == LINESTYLE_SINGLE));
        rSet.Put(SfxBoolItem(SID_ULINE_VAL_DOUBLE, eUnderline == LINESTYLE_DOUBLE));
        rSet.Put(SfxBoolItem(SID_ULINE_VAL_DOTTED, eUnderline == LINESTYLE_DOTTED));
        rSet.Put(SfxBoolItem(SID_ULINE_VAL_NONE, eUnderline == LINESTYLE_NONE));
    }

    //! Testing whether brace highlighting is active !!!!
    ScInputHandler* pHdl = GetMyInputHdl();
    if ( pHdl && pHdl->IsFormulaMode() )
        rSet.ClearItem( EE_CHAR_WEIGHT );   // Highlighted brace not here

    SvxEscapement eEsc = static_cast<SvxEscapement>(aAttribs.Get( EE_CHAR_ESCAPEMENT ).GetEnumValue());
    rSet.Put(SfxBoolItem(SID_SET_SUPER_SCRIPT, eEsc == SvxEscapement::Superscript));
    rSet.Put(SfxBoolItem(SID_SET_SUB_SCRIPT, eEsc == SvxEscapement::Subscript));
    rViewData.GetBindings().Invalidate( SID_SET_SUPER_SCRIPT );
    rViewData.GetBindings().Invalidate( SID_SET_SUB_SCRIPT );

    eState = aAttribs.GetItemState( EE_CHAR_KERNING );
    rViewData.GetBindings().Invalidate( SID_ATTR_CHAR_KERNING );
    if ( eState == SfxItemState::INVALID )
    {
        rSet.InvalidateItem(EE_CHAR_KERNING);
    }
}

OUString ScEditShell::GetSelectionText( bool bWholeWord )
{
    OUString aStrSelection;

    if ( rViewData.HasEditView( rViewData.GetActivePart() ) )
    {
        if ( bWholeWord )
        {
            EditEngine& rEngine = pEditView->getEditEngine();
            ESelection  aSel = pEditView->GetSelection();
            OUString    aStrCurrentDelimiters = rEngine.GetWordDelimiters();

            rEngine.SetWordDelimiters(u" .,;\"'"_ustr);
            aStrSelection = rEngine.GetWord( aSel.nEndPara, aSel.nEndPos );
            rEngine.SetWordDelimiters( aStrCurrentDelimiters );
        }
        else
        {
            aStrSelection = pEditView->GetSelected();
        }
    }

    return aStrSelection;
}

void ScEditShell::ExecuteUndo(const SfxRequest& rReq)
{
    //  Undo must be handled here because it's called for both EditViews

    ScInputHandler* pHdl = GetMyInputHdl();
    OSL_ENSURE(pHdl,"no ScInputHandler");
    EditView* pTopView   = pHdl->GetTopView();
    EditView* pTableView = pHdl->GetTableView();
    OSL_ENSURE(pTableView,"no EditView");

    pHdl->DataChanging();

    const SfxItemSet* pReqArgs = rReq.GetArgs();
    sal_uInt16 nSlot = rReq.GetSlot();
    switch ( nSlot )
    {
        case SID_UNDO:
        case SID_REDO:
            {
                bool bIsUndo = ( nSlot == SID_UNDO );

                sal_uInt16 nCount = 1;
                const SfxPoolItem* pItem;
                if ( pReqArgs && pReqArgs->GetItemState( nSlot, true, &pItem ) == SfxItemState::SET )
                    nCount = static_cast<const SfxUInt16Item*>(pItem)->GetValue();

                for (sal_uInt16 i=0; i<nCount; i++)
                {
                    if ( bIsUndo )
                    {
                        pTableView->Undo();
                        if (pTopView)
                            pTopView->Undo();
                    }
                    else
                    {
                        pTableView->Redo();
                        if (pTopView)
                            pTopView->Redo();
                    }
                }
            }
            break;
    }
    rViewData.GetBindings().InvalidateAll(false);

    pHdl->DataChanged();
}

void ScEditShell::GetUndoState(SfxItemSet &rSet)
{
    //  Undo state is taken from normal ViewFrame state function

    SfxViewFrame& rViewFrm = rViewData.GetViewShell()->GetViewFrame();
    if ( GetUndoManager() )
    {
        SfxWhichIter aIter(rSet);
        sal_uInt16 nWhich = aIter.FirstWhich();
        while( nWhich )
        {
            rViewFrm.GetSlotState( nWhich, nullptr, &rSet );
            nWhich = aIter.NextWhich();
        }
    }

    //  disable if no action in input line EditView

    ScInputHandler* pHdl = GetMyInputHdl();
    OSL_ENSURE(pHdl,"no ScInputHandler");
    EditView* pTopView = pHdl->GetTopView();
    if (pTopView)
    {
        SfxUndoManager& rTopMgr = pTopView->getEditEngine().GetUndoManager();
        if ( rTopMgr.GetUndoActionCount() == 0 )
            rSet.DisableItem( SID_UNDO );
        if ( rTopMgr.GetRedoActionCount() == 0 )
            rSet.DisableItem( SID_REDO );
    }
}

void ScEditShell::ExecuteTrans( const SfxRequest& rReq )
{
    TransliterationFlags nType = ScViewUtil::GetTransliterationType( rReq.GetSlot() );
    if ( nType == TransliterationFlags::NONE )
        return;

    ScInputHandler* pHdl = GetMyInputHdl();
    assert(pHdl && "no ScInputHandler");

    EditView* pTopView   = pHdl->GetTopView();
    EditView* pTableView = pHdl->GetTableView();
    assert(pTableView && "no EditView");

    pHdl->DataChanging();

    pTableView->TransliterateText( nType );
    if (pTopView)
        pTopView->TransliterateText( nType );

    pHdl->DataChanged();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
