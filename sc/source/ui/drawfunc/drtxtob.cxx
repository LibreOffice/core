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
#include <scitems.hxx>

#include <editeng/adjustitem.hxx>
#include <svx/clipfmtitem.hxx>
#include <editeng/contouritem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/editeng.hxx>
#include <editeng/editview.hxx>
#include <editeng/escapementitem.hxx>
#include <editeng/flditem.hxx>
#include <editeng/flstitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/lspcitem.hxx>
#include <editeng/ulspitem.hxx>
#include <svx/hlnkitem.hxx>
#include <svx/svdoutl.hxx>
#include <editeng/postitem.hxx>
#include <editeng/scripttypeitem.hxx>
#include <editeng/shdditem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/writingmodeitem.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/request.hxx>
#include <sfx2/viewfrm.hxx>
#include <svtools/cliplistener.hxx>
#include <vcl/transfer.hxx>
#include <svl/whiter.hxx>
#include <svl/languageoptions.hxx>

#include <svx/svxdlg.hxx>
#include <vcl/EnumContext.hxx>

#include <sc.hrc>
#include <globstr.hrc>
#include <scresid.hxx>
#include <scmod.hxx>
#include <drtxtob.hxx>
#include <viewdata.hxx>
#include <document.hxx>
#include <drawview.hxx>
#include <viewutil.hxx>
#include <tabvwsh.hxx>
#include <gridwin.hxx>

#define ShellClass_ScDrawTextObjectBar
#include <scslots.hxx>

using namespace ::com::sun::star;

SFX_IMPL_INTERFACE(ScDrawTextObjectBar, SfxShell)

void ScDrawTextObjectBar::InitInterface_Impl()
{
    GetStaticInterface()->RegisterObjectBar(SFX_OBJECTBAR_OBJECT,
                                            SfxVisibilityFlags::Standard | SfxVisibilityFlags::Server,
                                            ToolbarId::Text_Toolbox_Sc);

    GetStaticInterface()->RegisterPopupMenu("drawtext");

    GetStaticInterface()->RegisterChildWindow(ScGetFontWorkId());
}


// disable not wanted accelerators

void ScDrawTextObjectBar::StateDisableItems( SfxItemSet &rSet )
{
    SfxWhichIter aIter(rSet);
    sal_uInt16 nWhich = aIter.FirstWhich();

    while (nWhich)
    {
        rSet.DisableItem( nWhich );
        nWhich = aIter.NextWhich();
    }
}

ScDrawTextObjectBar::ScDrawTextObjectBar(ScViewData* pData) :
    SfxShell(pData->GetViewShell()),
    pViewData(pData),
    bPastePossible(false)
{
    SetPool( pViewData->GetScDrawView()->GetDefaultAttr().GetPool() );

    //  At the switching-over the UndoManager is changed to edit mode
    SfxUndoManager* pMgr = pViewData->GetSfxDocShell()->GetUndoManager();
    SetUndoManager( pMgr );
    if ( !pViewData->GetDocument()->IsUndoEnabled() )
    {
        pMgr->SetMaxUndoActionCount( 0 );
    }

    SetName("DrawText");
    SfxShell::SetContextName(vcl::EnumContext::GetContextName(vcl::EnumContext::Context::DrawText));
}

ScDrawTextObjectBar::~ScDrawTextObjectBar()
{
    if ( mxClipEvtLstnr.is() )
    {
        mxClipEvtLstnr->RemoveListener( pViewData->GetActiveWin() );

        //  The listener may just now be waiting for the SolarMutex and call the link
        //  afterwards, in spite of RemoveListener. So the link has to be reset, too.
        mxClipEvtLstnr->ClearCallbackLink();
    }
}

//          Functions

void ScDrawTextObjectBar::Execute( SfxRequest &rReq )
{
    ScDrawView* pView = pViewData->GetScDrawView();
    OutlinerView* pOutView = pView->GetTextEditOutlinerView();
    Outliner* pOutliner = pView->GetTextEditOutliner();

    if (!pOutView || !pOutliner)
    {
        ExecuteGlobal( rReq );              // on whole objects
        return;
    }

    const SfxItemSet* pReqArgs = rReq.GetArgs();
    sal_uInt16 nSlot = rReq.GetSlot();
    switch ( nSlot )
    {
        case SID_COPY:
            pOutView->Copy();
            break;

        case SID_CUT:
            pOutView->Cut();
            break;

        case SID_PASTE:
            pOutView->PasteSpecial();
            break;

        case SID_CLIPBOARD_FORMAT_ITEMS:
            {
                SotClipboardFormatId nFormat = SotClipboardFormatId::NONE;
                const SfxPoolItem* pItem;
                if ( pReqArgs &&
                     pReqArgs->GetItemState(nSlot, true, &pItem) == SfxItemState::SET &&
                     dynamic_cast<const SfxUInt32Item*>( pItem) !=  nullptr )
                {
                    nFormat = static_cast<SotClipboardFormatId>(static_cast<const SfxUInt32Item*>(pItem)->GetValue());
                }

                if ( nFormat != SotClipboardFormatId::NONE )
                {
                    if (nFormat == SotClipboardFormatId::STRING)
                        pOutView->Paste();
                    else
                        pOutView->PasteSpecial();
                }
            }
            break;

        case SID_PASTE_SPECIAL:
            ExecutePasteContents( rReq );
            break;

        case SID_PASTE_UNFORMATTED:
            pOutView->Paste();
            break;

        case SID_SELECTALL:
            {
                sal_Int32 nCount = pOutliner->GetParagraphCount();
                ESelection aSel( 0,0,nCount,0 );
                pOutView->SetSelection( aSel );
            }
            break;

        case SID_CHARMAP:
            {
                const SvxFontItem& rItem = pOutView->GetAttribs().Get(EE_CHAR_FONTINFO);

                OUString aString;
                SvxFontItem aNewItem( EE_CHAR_FONTINFO );

                const SfxItemSet *pArgs = rReq.GetArgs();
                const SfxPoolItem* pItem = nullptr;
                if( pArgs )
                    pArgs->GetItemState(GetPool().GetWhich(SID_CHARMAP), false, &pItem);

                if ( pItem )
                {
                    aString = static_cast<const SfxStringItem*>(pItem)->GetValue();
                    const SfxPoolItem* pFtItem = nullptr;
                    pArgs->GetItemState( GetPool().GetWhich(SID_ATTR_SPECIALCHAR), false, &pFtItem);
                    const SfxStringItem* pFontItem = dynamic_cast<const SfxStringItem*>( pFtItem  );
                    if ( pFontItem )
                    {
                        const OUString& aFontName(pFontItem->GetValue());
                        vcl::Font aFont(aFontName, Size(1,1)); // Size only because of CTOR
                        aNewItem = SvxFontItem( aFont.GetFamilyType(), aFont.GetFamilyName(),
                                    aFont.GetStyleName(), aFont.GetPitch(),
                                    aFont.GetCharSet(), ATTR_FONT  );
                    }
                    else
                        aNewItem = rItem;
                }
                else
                    ScViewUtil::ExecuteCharMap( rItem, *pViewData->GetViewShell()->GetViewFrame() );

                if ( !aString.isEmpty() )
                {
                    SfxItemSet aSet( pOutliner->GetEmptyItemSet() );
                    aSet.Put( aNewItem );
                    //  If nothing is selected, then SetAttribs of the View selects a word
                    pOutView->GetOutliner()->QuickSetAttribs( aSet, pOutView->GetSelection() );
                    pOutView->InsertText(aString);
                }

                Invalidate( SID_ATTR_CHAR_FONT );
            }
            break;

        case SID_HYPERLINK_SETLINK:
            if( pReqArgs )
            {
                const SfxPoolItem* pItem;
                if ( pReqArgs->GetItemState( SID_HYPERLINK_SETLINK, true, &pItem ) == SfxItemState::SET )
                {
                    const SvxHyperlinkItem* pHyper = static_cast<const SvxHyperlinkItem*>(pItem);
                    const OUString& rName = pHyper->GetName();
                    const OUString& rURL      = pHyper->GetURL();
                    const OUString& rTarget   = pHyper->GetTargetFrame();
                    SvxLinkInsertMode eMode = pHyper->GetInsertMode();

                    bool bDone = false;
                    if (eMode == HLINK_DEFAULT || eMode == HLINK_FIELD)
                    {
                        const SvxFieldItem* pFieldItem = pOutView->GetFieldAtSelection();
                        if (pFieldItem)
                        {
                            const SvxFieldData* pField = pFieldItem->GetField();
                            if ( dynamic_cast<const SvxURLField*>( pField) )
                            {
                                //  select old field

                                ESelection aSel = pOutView->GetSelection();
                                aSel.Adjust();
                                aSel.nEndPara = aSel.nStartPara;
                                aSel.nEndPos = aSel.nStartPos + 1;
                                pOutView->SetSelection( aSel );
                            }
                        }

                        //  insert new field

                        SvxURLField aURLField( rURL, rName, SvxURLFormat::Repr );
                        aURLField.SetTargetFrame( rTarget );
                        SvxFieldItem aURLItem( aURLField, EE_FEATURE_FIELD );
                        pOutView->InsertField( aURLItem );

                        //  select new field

                        ESelection aSel = pOutView->GetSelection();
                        if ( aSel.nStartPos == aSel.nEndPos && aSel.nStartPos > 0 )
                        {
                            //  Cursor is behind the inserted field -> extend selection to the left

                            --aSel.nStartPos;
                            pOutView->SetSelection( aSel );
                        }

                        bDone = true;
                    }

                    if (!bDone)
                        ExecuteGlobal( rReq );      // normal at View

                    //  If "text" is received by InsertURL of ViewShell, then the DrawShell is turned off !!!
                }
            }
            break;

        case SID_OPEN_HYPERLINK:
            if (const SvxFieldItem* pFieldItem = pOutView->GetFieldAtSelection())
            {
                const SvxFieldData* pField = pFieldItem->GetField();
                if (const SvxURLField* pURLField = dynamic_cast<const SvxURLField*>(pField))
                {
                    ScGlobal::OpenURL(pURLField->GetURL(), pURLField->GetTargetFrame());
                }
            }
            break;

        case SID_ENABLE_HYPHENATION:
        case SID_TEXTDIRECTION_LEFT_TO_RIGHT:
        case SID_TEXTDIRECTION_TOP_TO_BOTTOM:
            pView->ScEndTextEdit(); // end text edit before switching direction
            ExecuteGlobal( rReq );
            // restore consistent state between shells and functions:
            pViewData->GetDispatcher().Execute(SID_OBJECT_SELECT, SfxCallMode::SLOT | SfxCallMode::RECORD);
            break;

        case SID_THES:
            {
                OUString aReplaceText;
                const SfxStringItem* pItem2 = rReq.GetArg<SfxStringItem>(SID_THES);
                if (pItem2)
                    aReplaceText = pItem2->GetValue();
                if (!aReplaceText.isEmpty())
                    ReplaceTextWithSynonym( pOutView->GetEditView(), aReplaceText );
            }
            break;

        case SID_THESAURUS:
            {
                pOutView->StartThesaurus();
            }
            break;

    }
}

void ScDrawTextObjectBar::GetState( SfxItemSet& rSet )
{
    SfxViewFrame* pViewFrm = pViewData->GetViewShell()->GetViewFrame();
    bool bHasFontWork = pViewFrm->HasChildWindow(SID_FONTWORK);
    bool bDisableFontWork = false;

    if (IsNoteEdit())
    {
        // #i21255# notes now support rich text formatting (#i74140# but not fontwork)
        bDisableFontWork = true;
    }

    if ( bDisableFontWork )
        rSet.DisableItem( SID_FONTWORK  );
    else
        rSet.Put(SfxBoolItem(SID_FONTWORK, bHasFontWork));

    if ( rSet.GetItemState( SID_HYPERLINK_GETLINK ) != SfxItemState::UNKNOWN )
    {
        SvxHyperlinkItem aHLinkItem;
        SdrView* pView = pViewData->GetScDrawView();
        OutlinerView* pOutView = pView->GetTextEditOutlinerView();
        if ( pOutView )
        {
            bool bField = false;
            const SvxFieldItem* pFieldItem = pOutView->GetFieldAtSelection();
            if (pFieldItem)
            {
                const SvxFieldData* pField = pFieldItem->GetField();
                if (const SvxURLField* pURLField = dynamic_cast<const SvxURLField*>(pField))
                {
                    aHLinkItem.SetName( pURLField->GetRepresentation() );
                    aHLinkItem.SetURL( pURLField->GetURL() );
                    aHLinkItem.SetTargetFrame( pURLField->GetTargetFrame() );
                    bField = true;
                }
            }
            if (!bField)
            {
                // use selected text as name for urls
                OUString sReturn = pOutView->GetSelected();
                sal_Int32 nLen = std::min<sal_Int32>(sReturn.getLength(), 255);
                sReturn = sReturn.copy(0, nLen);
                aHLinkItem.SetName(comphelper::string::stripEnd(sReturn, ' '));
            }
        }
        rSet.Put(aHLinkItem);
    }

    if ( rSet.GetItemState( SID_OPEN_HYPERLINK ) != SfxItemState::UNKNOWN )
    {
        SdrView* pView = pViewData->GetScDrawView();
        OutlinerView* pOutView = pView->GetTextEditOutlinerView();
        bool bEnable = false;
        if ( pOutView )
        {
            const SvxFieldItem* pFieldItem = pOutView->GetFieldAtSelection();
            if ( pFieldItem )
            {
                const SvxFieldData* pField = pFieldItem->GetField();
                bEnable = dynamic_cast<const SvxURLField*>( pField) !=  nullptr;
            }
        }
        if( !bEnable )
            rSet.DisableItem( SID_OPEN_HYPERLINK );
    }

    if( rSet.GetItemState( SID_TRANSLITERATE_HALFWIDTH ) != SfxItemState::UNKNOWN )
        ScViewUtil::HideDisabledSlot( rSet, pViewFrm->GetBindings(), SID_TRANSLITERATE_HALFWIDTH );
    if( rSet.GetItemState( SID_TRANSLITERATE_FULLWIDTH ) != SfxItemState::UNKNOWN )
        ScViewUtil::HideDisabledSlot( rSet, pViewFrm->GetBindings(), SID_TRANSLITERATE_FULLWIDTH );
    if( rSet.GetItemState( SID_TRANSLITERATE_HIRAGANA ) != SfxItemState::UNKNOWN )
        ScViewUtil::HideDisabledSlot( rSet, pViewFrm->GetBindings(), SID_TRANSLITERATE_HIRAGANA );
    if( rSet.GetItemState( SID_TRANSLITERATE_KATAKANA ) != SfxItemState::UNKNOWN )
        ScViewUtil::HideDisabledSlot( rSet, pViewFrm->GetBindings(), SID_TRANSLITERATE_KATAKANA );

    if ( rSet.GetItemState( SID_ENABLE_HYPHENATION ) != SfxItemState::UNKNOWN )
    {
        SdrView* pView = pViewData->GetScDrawView();
        SfxItemSet aAttrs( pView->GetModel()->GetItemPool() );
        pView->GetAttributes( aAttrs );
        if( aAttrs.GetItemState( EE_PARA_HYPHENATE ) >= SfxItemState::DEFAULT )
        {
            bool bValue = aAttrs.Get( EE_PARA_HYPHENATE ).GetValue();
            rSet.Put( SfxBoolItem( SID_ENABLE_HYPHENATION, bValue ) );
        }
    }

    if ( rSet.GetItemState( SID_THES ) != SfxItemState::UNKNOWN  ||
         rSet.GetItemState( SID_THESAURUS ) != SfxItemState::UNKNOWN )
    {
        SdrView * pView = pViewData->GetScDrawView();
        OutlinerView* pOutView = pView->GetTextEditOutlinerView();

        OUString        aStatusVal;
        LanguageType    nLang = LANGUAGE_NONE;
        bool bIsLookUpWord = false;
        if ( pOutView )
        {
            EditView& rEditView = pOutView->GetEditView();
            bIsLookUpWord = GetStatusValueForThesaurusFromContext( aStatusVal, nLang, rEditView );
        }
        rSet.Put( SfxStringItem( SID_THES, aStatusVal ) );

        // disable thesaurus main menu and context menu entry if there is nothing to look up
        bool bCanDoThesaurus = ScModule::HasThesaurusLanguage( nLang );
        if (!bIsLookUpWord || !bCanDoThesaurus)
            rSet.DisableItem( SID_THES );
        if (!bCanDoThesaurus)
            rSet.DisableItem( SID_THESAURUS );
    }
}

IMPL_LINK( ScDrawTextObjectBar, ClipboardChanged, TransferableDataHelper*, pDataHelper, void )
{
    bPastePossible = ( pDataHelper->HasFormat( SotClipboardFormatId::STRING ) || pDataHelper->HasFormat( SotClipboardFormatId::RTF )
        || pDataHelper->HasFormat( SotClipboardFormatId::RICHTEXT ) );

    SfxBindings& rBindings = pViewData->GetBindings();
    rBindings.Invalidate( SID_PASTE );
    rBindings.Invalidate( SID_PASTE_SPECIAL );
    rBindings.Invalidate( SID_PASTE_UNFORMATTED );
    rBindings.Invalidate( SID_CLIPBOARD_FORMAT_ITEMS );
}

void ScDrawTextObjectBar::GetClipState( SfxItemSet& rSet )
{
    SdrView* pView = pViewData->GetScDrawView();
    if ( !pView->GetTextEditOutlinerView() )
    {
        GetGlobalClipState( rSet );
        return;
    }

    if ( !mxClipEvtLstnr.is() )
    {
        // create listener
        mxClipEvtLstnr = new TransferableClipboardListener( LINK( this, ScDrawTextObjectBar, ClipboardChanged ) );
        vcl::Window* pWin = pViewData->GetActiveWin();
        mxClipEvtLstnr->AddListener( pWin );

        // get initial state
        TransferableDataHelper aDataHelper( TransferableDataHelper::CreateFromSystemClipboard( pViewData->GetActiveWin() ) );
        bPastePossible = ( aDataHelper.HasFormat( SotClipboardFormatId::STRING ) || aDataHelper.HasFormat( SotClipboardFormatId::RTF )
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
                if ( bPastePossible )
                {
                    SvxClipboardFormatItem aFormats( SID_CLIPBOARD_FORMAT_ITEMS );
                    TransferableDataHelper aDataHelper(
                            TransferableDataHelper::CreateFromSystemClipboard( pViewData->GetActiveWin() ) );

                    if ( aDataHelper.HasFormat( SotClipboardFormatId::STRING ) )
                        aFormats.AddClipbrdFormat( SotClipboardFormatId::STRING );
                    if ( aDataHelper.HasFormat( SotClipboardFormatId::RTF ) )
                        aFormats.AddClipbrdFormat( SotClipboardFormatId::RTF );
                    if ( aDataHelper.HasFormat( SotClipboardFormatId::RICHTEXT ) )
                        aFormats.AddClipbrdFormat( SotClipboardFormatId::RICHTEXT );

                    rSet.Put( aFormats );
                }
                else
                    rSet.DisableItem( nWhich );
                break;
        }
        nWhich = aIter.NextWhich();
    }
}

//          Attributes

void ScDrawTextObjectBar::ExecuteToggle( SfxRequest &rReq )
{
    //  Underline

    SdrView* pView = pViewData->GetScDrawView();

    sal_uInt16 nSlot = rReq.GetSlot();

    SfxItemSet aSet( pView->GetDefaultAttr() );

    SfxItemSet aViewAttr(pView->GetModel()->GetItemPool());
    pView->GetAttributes(aViewAttr);

    //  Underline
    FontLineStyle eOld = aViewAttr.Get(EE_CHAR_UNDERLINE).GetLineStyle();
    FontLineStyle eNew = eOld;
    switch (nSlot)
    {
        case SID_ULINE_VAL_NONE:
            eNew = LINESTYLE_NONE;
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
        default:
            break;
    }
    aSet.Put( SvxUnderlineItem( eNew, EE_CHAR_UNDERLINE ) );

    pView->SetAttributes( aSet );
    rReq.Done();
    pViewData->GetScDrawView()->InvalidateDrawTextAttrs();
}

static void lcl_RemoveFields( OutlinerView& rOutView )
{
    //! Outliner should have RemoveFields with a selection

    Outliner* pOutliner = rOutView.GetOutliner();
    if (!pOutliner) return;

    ESelection aOldSel = rOutView.GetSelection();
    ESelection aSel = aOldSel;
    aSel.Adjust();
    sal_Int32 nNewEnd = aSel.nEndPos;

    bool bUpdate = pOutliner->GetUpdateMode();
    bool bChanged = false;

    //! GetPortions and GetAttribs should be const!
    EditEngine& rEditEng = const_cast<EditEngine&>(pOutliner->GetEditEngine());

    sal_Int32 nParCount = pOutliner->GetParagraphCount();
    for (sal_Int32 nPar=0; nPar<nParCount; nPar++)
        if ( nPar >= aSel.nStartPara && nPar <= aSel.nEndPara )
        {
            std::vector<sal_Int32> aPortions;
            rEditEng.GetPortions( nPar, aPortions );

            for ( size_t nPos = aPortions.size(); nPos; )
            {
                --nPos;
                sal_Int32 nEnd = aPortions[ nPos ];
                sal_Int32 nStart = nPos ? aPortions[ nPos - 1 ] : 0;
                // fields are single characters
                if ( nEnd == nStart+1 &&
                     ( nPar > aSel.nStartPara || nStart >= aSel.nStartPos ) &&
                     ( nPar < aSel.nEndPara   || nEnd   <= aSel.nEndPos ) )
                {
                    ESelection aFieldSel( nPar, nStart, nPar, nEnd );
                    SfxItemSet aSet = rEditEng.GetAttribs( aFieldSel );
                    if ( aSet.GetItemState( EE_FEATURE_FIELD ) == SfxItemState::SET )
                    {
                        if (!bChanged)
                        {
                            if (bUpdate)
                                pOutliner->SetUpdateMode( false );
                            OUString aName = ScResId( STR_UNDO_DELETECONTENTS );
                            ViewShellId nViewShellId(-1);
                            if (ScTabViewShell* pViewSh = ScTabViewShell::GetActiveViewShell())
                                nViewShellId = pViewSh->GetViewShellId();
                            pOutliner->GetUndoManager().EnterListAction( aName, aName, 0, nViewShellId );
                            bChanged = true;
                        }

                        OUString aFieldText = rEditEng.GetText( aFieldSel );
                        pOutliner->QuickInsertText( aFieldText, aFieldSel );
                        if ( nPar == aSel.nEndPara )
                        {
                            nNewEnd = nNewEnd + aFieldText.getLength();
                            --nNewEnd;
                        }
                    }
                }
            }
        }

    if (bUpdate && bChanged)
    {
        pOutliner->GetUndoManager().LeaveListAction();
        pOutliner->SetUpdateMode( true );
    }

    if ( aOldSel == aSel )          // aSel is adjusted
        aOldSel.nEndPos = nNewEnd;
    else
        aOldSel.nStartPos = nNewEnd;        // if aOldSel is backwards
    rOutView.SetSelection( aOldSel );
}

void ScDrawTextObjectBar::ExecuteAttr( SfxRequest &rReq )
{
    SdrView*            pView = pViewData->GetScDrawView();
    const SfxItemSet*   pArgs = rReq.GetArgs();
    sal_uInt16          nSlot = rReq.GetSlot();

    SfxItemSet aEditAttr( pView->GetModel()->GetItemPool() );
    pView->GetAttributes( aEditAttr );
    SfxItemSet  aNewAttr( *aEditAttr.GetPool(), aEditAttr.GetRanges() );

    bool bSet = true;
    switch ( nSlot )
    {
        case SID_ALIGNLEFT:
        case SID_ALIGN_ANY_LEFT:
        case SID_ATTR_PARA_ADJUST_LEFT:
            aNewAttr.Put( SvxAdjustItem( SvxAdjust::Left, EE_PARA_JUST ) );
            break;

        case SID_ALIGNCENTERHOR:
        case SID_ALIGN_ANY_HCENTER:
        case SID_ATTR_PARA_ADJUST_CENTER:
            aNewAttr.Put( SvxAdjustItem( SvxAdjust::Center, EE_PARA_JUST ) );
            break;

        case SID_ALIGNRIGHT:
        case SID_ALIGN_ANY_RIGHT:
        case SID_ATTR_PARA_ADJUST_RIGHT:
            aNewAttr.Put( SvxAdjustItem( SvxAdjust::Right, EE_PARA_JUST ) );
            break;

        case SID_ALIGNBLOCK:
        case SID_ALIGN_ANY_JUSTIFIED:
        case SID_ATTR_PARA_ADJUST_BLOCK:
            aNewAttr.Put( SvxAdjustItem( SvxAdjust::Block, EE_PARA_JUST ) );
            break;

        case SID_ATTR_PARA_LINESPACE_10:
            {
                SvxLineSpacingItem aItem( LINE_SPACE_DEFAULT_HEIGHT, EE_PARA_SBL );
                aItem.SetPropLineSpace( 100 );
                aNewAttr.Put( aItem );
            }
            break;

        case SID_ATTR_PARA_LINESPACE_15:
            {
                SvxLineSpacingItem aItem( LINE_SPACE_DEFAULT_HEIGHT, EE_PARA_SBL );
                aItem.SetPropLineSpace( 150 );
                aNewAttr.Put( aItem );
            }
            break;

        case SID_ATTR_PARA_LINESPACE_20:
            {
                SvxLineSpacingItem aItem( LINE_SPACE_DEFAULT_HEIGHT, EE_PARA_SBL );
                aItem.SetPropLineSpace( 200 );
                aNewAttr.Put( aItem );
            }
            break;

        case SID_SET_SUPER_SCRIPT:
            {
                SvxEscapementItem aItem(EE_CHAR_ESCAPEMENT);
                SvxEscapement eEsc = static_cast<SvxEscapement>(aEditAttr.Get( EE_CHAR_ESCAPEMENT ).GetEnumValue());

                if( eEsc == SvxEscapement::Superscript )
                    aItem.SetEscapement( SvxEscapement::Off );
                else
                    aItem.SetEscapement( SvxEscapement::Superscript );
                aNewAttr.Put( aItem );
            }
            break;

        case SID_SET_SUB_SCRIPT:
            {
                SvxEscapementItem aItem(EE_CHAR_ESCAPEMENT);
                SvxEscapement eEsc = static_cast<SvxEscapement>(aEditAttr.Get( EE_CHAR_ESCAPEMENT ).GetEnumValue());

                if( eEsc == SvxEscapement::Subscript )
                    aItem.SetEscapement( SvxEscapement::Off );
                else
                    aItem.SetEscapement( SvxEscapement::Subscript );
                aNewAttr.Put( aItem );
            }
            break;

        case SID_TABLE_VERT_NONE:
        case SID_TABLE_VERT_CENTER:
        case SID_TABLE_VERT_BOTTOM:
            {
                SdrTextVertAdjust eTVA = SDRTEXTVERTADJUST_TOP;
                if (nSlot == SID_TABLE_VERT_CENTER)
                    eTVA = SDRTEXTVERTADJUST_CENTER;
                else if (nSlot == SID_TABLE_VERT_BOTTOM)
                    eTVA = SDRTEXTVERTADJUST_BOTTOM;
                aNewAttr.Put(SdrTextVertAdjustItem(eTVA));
            }
            break;

        case SID_PARASPACE_INCREASE:
        case SID_PARASPACE_DECREASE:
        {
            SvxULSpaceItem aULSpace( aEditAttr.Get( EE_PARA_ULSPACE ) );
            sal_uInt16 nUpper = aULSpace.GetUpper();
            sal_uInt16 nLower = aULSpace.GetLower();

            if ( nSlot == SID_PARASPACE_INCREASE )
            {
                nUpper += 100;
                nLower += 100;
            }
            else
            {
                nUpper = std::max< sal_Int16 >( nUpper - 100, 0 );
                nLower = std::max< sal_Int16 >( nLower - 100, 0 );
            }

            aULSpace.SetUpper( nUpper );
            aULSpace.SetLower( nLower );
            aNewAttr.Put( aULSpace );
        }
        break;

        default:
            bSet = false;
    }

    bool bDone = true;
    bool bArgsInReq = ( pArgs != nullptr );

    if ( !bArgsInReq )
    {
        switch ( nSlot )
        {
            case SID_TEXT_STANDARD: // delete hard text attributes
            {
                OutlinerView* pOutView = pView->IsTextEdit() ?
                                pView->GetTextEditOutlinerView() : nullptr;
                if ( pOutView )
                    pOutView->Paint( tools::Rectangle() );

                SfxItemSet aEmptyAttr( *aEditAttr.GetPool(), svl::Items<EE_ITEMS_START, EE_ITEMS_END>{} );
                pView->SetAttributes( aEmptyAttr, true );

                if ( pOutView )
                {
                    lcl_RemoveFields( *pOutView );
                    pOutView->ShowCursor();
                }

                rReq.Done( aEmptyAttr );
                pViewData->GetScDrawView()->InvalidateDrawTextAttrs();
                bDone = false; // already happened here
            }
            break;

            case SID_GROW_FONT_SIZE:
            case SID_SHRINK_FONT_SIZE:
            {
                OutlinerView* pOutView = pView->IsTextEdit() ?
                    pView->GetTextEditOutlinerView() : nullptr;
                if ( pOutView )
                {
                    const SvxFontListItem* pFontListItem = static_cast< const SvxFontListItem* >
                            ( SfxObjectShell::Current()->GetItem( SID_ATTR_CHAR_FONTLIST ) );
                    const FontList* pFontList = pFontListItem ? pFontListItem->GetFontList() : nullptr;
                    pOutView->GetEditView().ChangeFontSize( nSlot == SID_GROW_FONT_SIZE, pFontList );
                    pViewData->GetBindings().Invalidate( SID_ATTR_CHAR_FONTHEIGHT );
                    bDone = false;
                }
            }
            break;

            case SID_CHAR_DLG_EFFECT:
            case SID_CHAR_DLG:                      // dialog button
            case SID_ATTR_CHAR_FONT:                // Controller not shown
            case SID_ATTR_CHAR_FONTHEIGHT:
                bDone = ExecuteCharDlg( aEditAttr, aNewAttr , nSlot);
                break;

            case SID_PARA_DLG:
                bDone = ExecuteParaDlg( aEditAttr, aNewAttr );
                break;

            case SID_ATTR_CHAR_WEIGHT:
                aNewAttr.Put( aEditAttr.Get( EE_CHAR_WEIGHT ) );
                break;

            case SID_ATTR_CHAR_POSTURE:
                aNewAttr.Put( aEditAttr.Get( EE_CHAR_ITALIC ) );
                break;

            case SID_ATTR_CHAR_UNDERLINE:
                aNewAttr.Put( aEditAttr.Get( EE_CHAR_UNDERLINE ) );
                break;

            case SID_ATTR_CHAR_OVERLINE:
                aNewAttr.Put( aEditAttr.Get( EE_CHAR_OVERLINE ) );
                break;

            case SID_ATTR_CHAR_CONTOUR:
                aNewAttr.Put( aEditAttr.Get( EE_CHAR_OUTLINE ) );
                break;

            case SID_ATTR_CHAR_SHADOWED:
                aNewAttr.Put( aEditAttr.Get( EE_CHAR_SHADOW ) );
                break;

            case SID_ATTR_CHAR_STRIKEOUT:
                aNewAttr.Put( aEditAttr.Get( EE_CHAR_STRIKEOUT ) );
                break;

            case SID_DRAWTEXT_ATTR_DLG:
                {
                    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                    vcl::Window* pWin = pViewData->GetDialogParent();
                    ScopedVclPtr<SfxAbstractTabDialog> pDlg(pFact->CreateTextTabDialog( pWin ? pWin->GetFrameWeld() : nullptr, &aEditAttr, pView ));

                    bDone = ( RET_OK == pDlg->Execute() );

                    if ( bDone )
                        aNewAttr.Put( *pDlg->GetOutputItemSet() );

                    pDlg.disposeAndClear();

                    SfxBindings& rBindings = pViewData->GetBindings();
                    rBindings.Invalidate( SID_TABLE_VERT_NONE );
                    rBindings.Invalidate( SID_TABLE_VERT_CENTER );
                    rBindings.Invalidate( SID_TABLE_VERT_BOTTOM );
                }
                break;
        }
    }

    if ( bSet || bDone )
    {
        rReq.Done( aNewAttr );
        pArgs = rReq.GetArgs();
    }

    if ( pArgs )
    {
        if ( bArgsInReq &&
            ( nSlot == SID_ATTR_CHAR_FONT || nSlot == SID_ATTR_CHAR_FONTHEIGHT ||
              nSlot == SID_ATTR_CHAR_WEIGHT || nSlot == SID_ATTR_CHAR_POSTURE ) )
        {
            // font items from toolbox controller have to be applied for the right script type

            // #i78017 establish the same behaviour as in Writer
            SvtScriptType nScript = SvtScriptType::LATIN | SvtScriptType::ASIAN | SvtScriptType::COMPLEX;
            if (nSlot == SID_ATTR_CHAR_FONT)
                nScript = pView->GetScriptType();

            SfxItemPool& rPool = GetPool();
            SvxScriptSetItem aSetItem( nSlot, rPool );
            sal_uInt16 nWhich = rPool.GetWhich( nSlot );
            aSetItem.PutItemForScriptType( nScript, pArgs->Get( nWhich ) );

            pView->SetAttributes( aSetItem.GetItemSet() );
        }
        else if( nSlot == SID_ATTR_PARA_LRSPACE )
        {
            sal_uInt16 nId = SID_ATTR_PARA_LRSPACE;
            const SvxLRSpaceItem& rItem = static_cast<const SvxLRSpaceItem&>(
                pArgs->Get( nId ));
            SfxItemSet aAttr( GetPool(), svl::Items<EE_PARA_LRSPACE, EE_PARA_LRSPACE>{} );
            nId = EE_PARA_LRSPACE;
            SvxLRSpaceItem aLRSpaceItem( rItem.GetLeft(),
                rItem.GetRight(), rItem.GetTextLeft(),
                rItem.GetTextFirstLineOfst(), nId );
            aAttr.Put( aLRSpaceItem );
            pView->SetAttributes( aAttr );
        }
        else if( nSlot == SID_ATTR_PARA_LINESPACE )
        {
            SvxLineSpacingItem aLineSpaceItem = static_cast<const SvxLineSpacingItem&>(pArgs->Get(
                                                                GetPool().GetWhich(nSlot)));
            SfxItemSet aAttr( GetPool(), svl::Items<EE_PARA_SBL, EE_PARA_SBL>{} );
            aAttr.Put( aLineSpaceItem );
            pView->SetAttributes( aAttr );
        }
        else if( nSlot == SID_ATTR_PARA_ULSPACE )
        {
            SvxULSpaceItem aULSpaceItem = static_cast<const SvxULSpaceItem&>(pArgs->Get(
                                                                GetPool().GetWhich(nSlot)));
            SfxItemSet aAttr( GetPool(), svl::Items<EE_PARA_ULSPACE, EE_PARA_ULSPACE>{} );
            aULSpaceItem.SetWhich(EE_PARA_ULSPACE);
            aAttr.Put( aULSpaceItem );
            pView->SetAttributes( aAttr );
        }
        else
        {
            // use args directly
            pView->SetAttributes( *pArgs );
        }
        pViewData->GetScDrawView()->InvalidateDrawTextAttrs();
    }
}

void ScDrawTextObjectBar::GetAttrState( SfxItemSet& rDestSet )
{
    if ( IsNoteEdit() )
    {
        // issue 21255 - Notes now support rich text formatting.
    }

    SvtLanguageOptions  aLangOpt;
    bool bDisableCTLFont = !aLangOpt.IsCTLFontEnabled();
    bool bDisableVerticalText = !aLangOpt.IsVerticalTextEnabled();

    SdrView* pView = pViewData->GetScDrawView();
    SfxItemSet aAttrSet(pView->GetModel()->GetItemPool());
    pView->GetAttributes(aAttrSet);

    //  direct attributes

    rDestSet.Put( aAttrSet );

    //  choose font info according to selection script type

    SvtScriptType nScript = pView->GetScriptType();

    // #i55929# input-language-dependent script type (depends on input language if nothing selected)
    SvtScriptType nInputScript = nScript;
    OutlinerView* pOutView = pView->GetTextEditOutlinerView();
    if (pOutView && !pOutView->GetSelection().HasRange())
    {
        LanguageType nInputLang = pViewData->GetActiveWin()->GetInputLanguage();
        if (nInputLang != LANGUAGE_DONTKNOW && nInputLang != LANGUAGE_SYSTEM)
            nInputScript = SvtLanguageOptions::GetScriptTypeOfLanguage( nInputLang );
    }

    // #i55929# according to spec, nInputScript is used for font and font height only
    if ( rDestSet.GetItemState( EE_CHAR_FONTINFO ) != SfxItemState::UNKNOWN )
        ScViewUtil::PutItemScript( rDestSet, aAttrSet, EE_CHAR_FONTINFO, nInputScript );
    if ( rDestSet.GetItemState( EE_CHAR_FONTHEIGHT ) != SfxItemState::UNKNOWN )
        ScViewUtil::PutItemScript( rDestSet, aAttrSet, EE_CHAR_FONTHEIGHT, nInputScript );
    if ( rDestSet.GetItemState( EE_CHAR_WEIGHT ) != SfxItemState::UNKNOWN )
        ScViewUtil::PutItemScript( rDestSet, aAttrSet, EE_CHAR_WEIGHT, nScript );
    if ( rDestSet.GetItemState( EE_CHAR_ITALIC ) != SfxItemState::UNKNOWN )
        ScViewUtil::PutItemScript( rDestSet, aAttrSet, EE_CHAR_ITALIC, nScript );
    //  Alignment

    SvxAdjust eAdj = aAttrSet.Get(EE_PARA_JUST).GetAdjust();
    switch( eAdj )
    {
    case SvxAdjust::Left:
        {
            rDestSet.Put( SfxBoolItem( SID_ALIGNLEFT, true ) );
            rDestSet.Put( SfxBoolItem( SID_ATTR_PARA_ADJUST_LEFT, true ) );
        }
        break;
    case SvxAdjust::Center:
        {
            rDestSet.Put( SfxBoolItem( SID_ALIGNCENTERHOR, true ) );
            rDestSet.Put( SfxBoolItem( SID_ATTR_PARA_ADJUST_CENTER, true ) );
        }
        break;
    case SvxAdjust::Right:
        {
            rDestSet.Put( SfxBoolItem( SID_ALIGNRIGHT, true ) );
            rDestSet.Put( SfxBoolItem( SID_ATTR_PARA_ADJUST_RIGHT, true ) );
        }
        break;
    case SvxAdjust::Block:
        {
            rDestSet.Put( SfxBoolItem( SID_ALIGNBLOCK, true ) );
            rDestSet.Put( SfxBoolItem( SID_ATTR_PARA_ADJUST_BLOCK, true ) );
        }
        break;
        default:
        {
            // added to avoid warnings
        }
    }
    // pseudo slots for Format menu
    rDestSet.Put( SfxBoolItem( SID_ALIGN_ANY_LEFT,      eAdj == SvxAdjust::Left ) );
    rDestSet.Put( SfxBoolItem( SID_ALIGN_ANY_HCENTER,   eAdj == SvxAdjust::Center ) );
    rDestSet.Put( SfxBoolItem( SID_ALIGN_ANY_RIGHT,     eAdj == SvxAdjust::Right ) );
    rDestSet.Put( SfxBoolItem( SID_ALIGN_ANY_JUSTIFIED, eAdj == SvxAdjust::Block ) );

    SvxLRSpaceItem aLR = aAttrSet.Get( EE_PARA_LRSPACE );
    aLR.SetWhich(SID_ATTR_PARA_LRSPACE);
    rDestSet.Put(aLR);
    Invalidate( SID_ATTR_PARA_LRSPACE );
    SfxItemState eState = aAttrSet.GetItemState( EE_PARA_LRSPACE );
    if ( eState == SfxItemState::DONTCARE )
        rDestSet.InvalidateItem(SID_ATTR_PARA_LRSPACE);
    //xuxu for Line Space
    SvxLineSpacingItem aLineSP = aAttrSet.Get( EE_PARA_SBL );
    aLineSP.SetWhich(SID_ATTR_PARA_LINESPACE);
    rDestSet.Put(aLineSP);
    Invalidate(SID_ATTR_PARA_LINESPACE);
    eState = aAttrSet.GetItemState( EE_PARA_SBL );
    if ( eState == SfxItemState::DONTCARE )
        rDestSet.InvalidateItem(SID_ATTR_PARA_LINESPACE);
    //xuxu for UL Space
    SvxULSpaceItem aULSP = aAttrSet.Get( EE_PARA_ULSPACE );
    aULSP.SetWhich(SID_ATTR_PARA_ULSPACE);
    rDestSet.Put(aULSP);
    Invalidate(SID_ATTR_PARA_ULSPACE);
    Invalidate(SID_PARASPACE_INCREASE);
    Invalidate(SID_PARASPACE_DECREASE);
    eState = aAttrSet.GetItemState( EE_PARA_ULSPACE );
    if( eState >= SfxItemState::DEFAULT )
    {
        if ( !aULSP.GetUpper() && !aULSP.GetLower() )
            rDestSet.DisableItem( SID_PARASPACE_DECREASE );
    }
    else
    {
        rDestSet.DisableItem( SID_PARASPACE_INCREASE );
        rDestSet.DisableItem( SID_PARASPACE_DECREASE );
        rDestSet.InvalidateItem(SID_ATTR_PARA_ULSPACE);
    }

    //  Line spacing

    sal_uInt16 nLineSpace = aAttrSet.Get( EE_PARA_SBL ).GetPropLineSpace();
    switch( nLineSpace )
    {
        case 100:
            rDestSet.Put( SfxBoolItem( SID_ATTR_PARA_LINESPACE_10, true ) );
            break;
        case 150:
            rDestSet.Put( SfxBoolItem( SID_ATTR_PARA_LINESPACE_15, true ) );
            break;
        case 200:
            rDestSet.Put( SfxBoolItem( SID_ATTR_PARA_LINESPACE_20, true ) );
            break;
    }

    //  super-/subscript

    SvxEscapement eEsc = static_cast<SvxEscapement>(aAttrSet.Get( EE_CHAR_ESCAPEMENT ).GetEnumValue());
    if( eEsc == SvxEscapement::Superscript )
        rDestSet.Put( SfxBoolItem( SID_SET_SUPER_SCRIPT, true ) );
    else if( eEsc == SvxEscapement::Subscript )
        rDestSet.Put( SfxBoolItem( SID_SET_SUB_SCRIPT, true ) );

    //  Underline

    eState = aAttrSet.GetItemState( EE_CHAR_UNDERLINE );
    if ( eState == SfxItemState::DONTCARE )
    {
        rDestSet.InvalidateItem( SID_ULINE_VAL_NONE );
        rDestSet.InvalidateItem( SID_ULINE_VAL_SINGLE );
        rDestSet.InvalidateItem( SID_ULINE_VAL_DOUBLE );
        rDestSet.InvalidateItem( SID_ULINE_VAL_DOTTED );
    }
    else
    {
        FontLineStyle eUnderline = aAttrSet.Get(EE_CHAR_UNDERLINE).GetLineStyle();
        sal_uInt16 nId = SID_ULINE_VAL_NONE;
        switch (eUnderline)
        {
            case LINESTYLE_SINGLE:  nId = SID_ULINE_VAL_SINGLE; break;
            case LINESTYLE_DOUBLE:  nId = SID_ULINE_VAL_DOUBLE; break;
            case LINESTYLE_DOTTED:  nId = SID_ULINE_VAL_DOTTED; break;
            default:
                break;
        }
        rDestSet.Put( SfxBoolItem( nId, true ) );
    }

    //  horizontal / vertical

    bool bLeftToRight = true;

    SdrOutliner* pOutl = pView->GetTextEditOutliner();
    if( pOutl )
    {
        if( pOutl->IsVertical() )
            bLeftToRight = false;
    }
    else
        bLeftToRight = aAttrSet.Get( SDRATTR_TEXTDIRECTION ).GetValue() == css::text::WritingMode_LR_TB;

    if ( bDisableVerticalText )
    {
        rDestSet.DisableItem( SID_TEXTDIRECTION_LEFT_TO_RIGHT );
        rDestSet.DisableItem( SID_TEXTDIRECTION_TOP_TO_BOTTOM );
    }
    else
    {
        rDestSet.Put( SfxBoolItem( SID_TEXTDIRECTION_LEFT_TO_RIGHT, bLeftToRight ) );
        rDestSet.Put( SfxBoolItem( SID_TEXTDIRECTION_TOP_TO_BOTTOM, !bLeftToRight ) );
    }

    //  left-to-right or right-to-left

    if ( !bLeftToRight || bDisableCTLFont )
    {
        //  disabled if vertical
        rDestSet.DisableItem( SID_ATTR_PARA_LEFT_TO_RIGHT );
        rDestSet.DisableItem( SID_ATTR_PARA_RIGHT_TO_LEFT );
    }
    else if ( aAttrSet.GetItemState( EE_PARA_WRITINGDIR ) == SfxItemState::DONTCARE )
    {
        rDestSet.InvalidateItem( SID_ATTR_PARA_LEFT_TO_RIGHT );
        rDestSet.InvalidateItem( SID_ATTR_PARA_RIGHT_TO_LEFT );
    }
    else
    {
        SvxFrameDirection eAttrDir = aAttrSet.Get( EE_PARA_WRITINGDIR ).GetValue();
        if ( eAttrDir == SvxFrameDirection::Environment )
        {
            //  get "environment" direction from page style
            if ( pViewData->GetDocument()->GetEditTextDirection( pViewData->GetTabNo() ) == EEHorizontalTextDirection::R2L )
                eAttrDir = SvxFrameDirection::Horizontal_RL_TB;
            else
                eAttrDir = SvxFrameDirection::Horizontal_LR_TB;
        }
        rDestSet.Put( SfxBoolItem( SID_ATTR_PARA_LEFT_TO_RIGHT, ( eAttrDir == SvxFrameDirection::Horizontal_LR_TB ) ) );
        rDestSet.Put( SfxBoolItem( SID_ATTR_PARA_RIGHT_TO_LEFT, ( eAttrDir == SvxFrameDirection::Horizontal_RL_TB ) ) );
    }
}

void ScDrawTextObjectBar::ExecuteTrans( const SfxRequest& rReq )
{
    TransliterationFlags nType = ScViewUtil::GetTransliterationType( rReq.GetSlot() );
    if ( nType != TransliterationFlags::NONE )
    {
        ScDrawView* pView = pViewData->GetScDrawView();
        OutlinerView* pOutView = pView->GetTextEditOutlinerView();
        if ( pOutView )
        {
            //  change selected text in object
            pOutView->TransliterateText( nType );
        }
        else
        {
            //! apply to whole objects?
        }
    }
}

void ScDrawTextObjectBar::GetStatePropPanelAttr(SfxItemSet &rSet)
{
    SfxWhichIter    aIter( rSet );
    sal_uInt16          nWhich = aIter.FirstWhich();

    SdrView*            pView = pViewData->GetScDrawView();

    SfxItemSet aEditAttr(pView->GetModel()->GetItemPool());
    pView->GetAttributes(aEditAttr);
    //SfxItemSet    aAttrs( *aEditAttr.GetPool(), aEditAttr.GetRanges() );

    while ( nWhich )
    {
        sal_uInt16 nSlotId = SfxItemPool::IsWhich(nWhich)
            ? GetPool().GetSlotId(nWhich)
            : nWhich;
        switch ( nSlotId )
        {
            case SID_TABLE_VERT_NONE:
            case SID_TABLE_VERT_CENTER:
            case SID_TABLE_VERT_BOTTOM:
                bool bContour = false;
                SfxItemState eConState = aEditAttr.GetItemState( SDRATTR_TEXT_CONTOURFRAME );
                if( eConState != SfxItemState::DONTCARE )
                {
                    bContour = aEditAttr.Get( SDRATTR_TEXT_CONTOURFRAME ).GetValue();
                }
                if (bContour) break;

                SfxItemState eVState = aEditAttr.GetItemState( SDRATTR_TEXT_VERTADJUST );
                //SfxItemState eHState = aAttrs.GetItemState( SDRATTR_TEXT_HORZADJUST );

                //if(SfxItemState::DONTCARE != eVState && SfxItemState::DONTCARE != eHState)
                if(SfxItemState::DONTCARE != eVState)
                {
                    SdrTextVertAdjust eTVA = aEditAttr.Get(SDRATTR_TEXT_VERTADJUST).GetValue();
                    bool bSet = (nSlotId == SID_TABLE_VERT_NONE && eTVA == SDRTEXTVERTADJUST_TOP) ||
                            (nSlotId == SID_TABLE_VERT_CENTER && eTVA == SDRTEXTVERTADJUST_CENTER) ||
                            (nSlotId == SID_TABLE_VERT_BOTTOM && eTVA == SDRTEXTVERTADJUST_BOTTOM);
                    rSet.Put(SfxBoolItem(nSlotId, bSet));
                }
                else
                {
                    rSet.Put(SfxBoolItem(nSlotId, false));
                }
                break;
        }
        nWhich = aIter.NextWhich();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
