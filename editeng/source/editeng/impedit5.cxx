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

#include <memory>
#include "impedit.hxx"
#include <comphelper/lok.hxx>
#include <editeng/editeng.hxx>
#include <editeng/txtrange.hxx>
#include <svl/eitem.hxx>
#include <svl/hint.hxx>
#include <sfx2/app.hxx>
#include <vcl/help.hxx>
#include <vcl/svapp.hxx>
#include <vcl/transfer.hxx>
#include <vcl/window.hxx>
#include <utility>

void ImpEditEngine::SetStyleSheetPool( SfxStyleSheetPool* pSPool )
{
    if (mpStylePool != pSPool)
        mpStylePool = pSPool;
}

const SfxStyleSheet* ImpEditEngine::GetStyleSheet( sal_Int32 nPara ) const
{
    const ContentNode* pNode = maEditDoc.GetObject( nPara );
    return pNode ? pNode->GetContentAttribs().GetStyleSheet() : nullptr;
}

SfxStyleSheet* ImpEditEngine::GetStyleSheet( sal_Int32 nPara )
{
    ContentNode* pNode = maEditDoc.GetObject( nPara );
    return pNode ? pNode->GetContentAttribs().GetStyleSheet() : nullptr;
}

void ImpEditEngine::SetStyleSheet( EditSelection aSel, SfxStyleSheet* pStyle )
{
    aSel.Adjust( maEditDoc );

    sal_Int32 nStartPara = maEditDoc.GetPos( aSel.Min().GetNode() );
    sal_Int32 nEndPara = maEditDoc.GetPos( aSel.Max().GetNode() );

    bool _bUpdate = SetUpdateLayout( false );

    for ( sal_Int32 n = nStartPara; n <= nEndPara; n++ )
        SetStyleSheet( n, pStyle );

    SetUpdateLayout( _bUpdate );
}

void ImpEditEngine::SetStyleSheet( sal_Int32 nPara, SfxStyleSheet* pStyle )
{
    DBG_ASSERT( GetStyleSheetPool() || !pStyle, "SetStyleSheet: No StyleSheetPool registered!" );
    ContentNode* pNode = maEditDoc.GetObject( nPara );
    SfxStyleSheet* pCurStyle = pNode->GetStyleSheet();
    if ( pStyle != pCurStyle )
    {
        if ( IsUndoEnabled() && !IsInUndo() && maStatus.DoUndoAttribs() )
        {
            OUString aPrevStyleName;
            if ( pCurStyle )
                aPrevStyleName = pCurStyle->GetName();

            OUString aNewStyleName;
            if ( pStyle )
                aNewStyleName = pStyle->GetName();

            InsertUndo(
                std::make_unique<EditUndoSetStyleSheet>(mpEditEngine, maEditDoc.GetPos( pNode ),
                        aPrevStyleName, pCurStyle ? pCurStyle->GetFamily() : SfxStyleFamily::Para,
                        aNewStyleName, pStyle ? pStyle->GetFamily() : SfxStyleFamily::Para,
                        pNode->GetContentAttribs().GetItems() ) );
        }
        if ( pCurStyle )
            EndListening( *pCurStyle );
        pNode->SetStyleSheet( pStyle, maStatus.UseCharAttribs() );
#if defined(YRS)
        maEditDoc.YrsSetStyle(nPara, pStyle ? pStyle->GetName() : OUString());
#endif
        if ( pStyle )
            StartListening(*pStyle, DuplicateHandling::Allow);

        if (pNode->GetWrongList())
            pNode->GetWrongList()->ResetInvalidRange(0, pNode->Len());
        ParaAttribsChanged( pNode );
    }
    if (IsUpdateLayout())
        FormatAndLayout();
}

void ImpEditEngine::UpdateParagraphsWithStyleSheet( SfxStyleSheet* pStyle )
{
    SvxFont aFontFromStyle;
    CreateFont( aFontFromStyle, pStyle->GetItemSet() );

    bool bUsed = false;
    for ( sal_Int32 nNode = 0; nNode < maEditDoc.Count(); nNode++ )
    {
        ContentNode* pNode = maEditDoc.GetObject( nNode );
        if ( pNode->GetStyleSheet() == pStyle )
        {
            bUsed = true;
            if (maStatus.UseCharAttribs())
                pNode->SetStyleSheet( pStyle, aFontFromStyle );
            else
                pNode->SetStyleSheet( pStyle, false );

            if (pNode->GetWrongList())
                pNode->GetWrongList()->ResetInvalidRange(0, pNode->Len());
            ParaAttribsChanged( pNode );
        }
    }
    if ( bUsed )
    {
        GetEditEnginePtr()->StyleSheetChanged( pStyle );
        if (IsUpdateLayout())
            FormatAndLayout();
    }
}

void ImpEditEngine::RemoveStyleFromParagraphs( SfxStyleSheet const * pStyle )
{
    for ( sal_Int32 nNode = 0; nNode < maEditDoc.Count(); nNode++ )
    {
        ContentNode* pNode = maEditDoc.GetObject(nNode);
        if ( pNode->GetStyleSheet() == pStyle )
        {
            pNode->SetStyleSheet( nullptr );
            ParaAttribsChanged( pNode );
        }
    }
    if (IsUpdateLayout())
        FormatAndLayout();
}

void ImpEditEngine::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    // So that not a lot of unnecessary formatting is done when destructing:
    if (!mbDowning)
    {
        SfxHintId nId = rHint.GetId();
        if ( ( nId == SfxHintId::StyleSheetInDestruction ) ||
             ( nId == SfxHintId::StyleSheetErased ) )
        {
            const SfxStyleSheetHint* pStyleSheetHint = static_cast<const SfxStyleSheetHint*>(&rHint);
            SfxStyleSheet* pStyle = static_cast<SfxStyleSheet*>( pStyleSheetHint->GetStyleSheet() );
            RemoveStyleFromParagraphs( pStyle );
        }
        else if ( nId == SfxHintId::StyleSheetModified || nId == SfxHintId::StyleSheetModifiedExtended )
        {
            const SfxStyleSheetHint* pStyleSheetHint = static_cast<const SfxStyleSheetHint*>(&rHint);
            SfxStyleSheet* pStyle = static_cast<SfxStyleSheet*>( pStyleSheetHint->GetStyleSheet() );
            UpdateParagraphsWithStyleSheet( pStyle );
        }
        else if ( nId == SfxHintId::Dying && rBC.IsSfxStyleSheet() )
        {
            auto pStyle = static_cast< SfxStyleSheet* >(&rBC);
            RemoveStyleFromParagraphs( pStyle );
        }
        else if ( nId == SfxHintId::DataChanged && rBC.IsSfxStyleSheet())
        {
            auto pStyle = static_cast< SfxStyleSheet* >(&rBC);
            UpdateParagraphsWithStyleSheet( pStyle );
        }
    }
    if (rHint.GetId() == SfxHintId::Dying && dynamic_cast<const SfxApplication*>(&rBC))
        Dispose();
}

std::unique_ptr<EditUndoSetAttribs> ImpEditEngine::CreateAttribUndo( EditSelection aSel, const SfxItemSet& rSet )
{
    DBG_ASSERT( !aSel.DbgIsBuggy( maEditDoc ), "CreateAttribUndo: Incorrect selection ");
    aSel.Adjust( maEditDoc );

    ESelection aESel( CreateESel( aSel ) );

    sal_Int32 nStartNode = maEditDoc.GetPos( aSel.Min().GetNode() );
    sal_Int32 nEndNode = maEditDoc.GetPos( aSel.Max().GetNode() );

    DBG_ASSERT( nStartNode <= nEndNode, "CreateAttribUndo: Start > End ?!" );

    std::unique_ptr<EditUndoSetAttribs> pUndo;
    if ( rSet.GetPool() != &maEditDoc.GetItemPool() )
    {
        SfxItemSet aTmpSet( GetEmptyItemSet() );
        aTmpSet.Put( rSet );
        pUndo.reset( new EditUndoSetAttribs(mpEditEngine, aESel, std::move(aTmpSet)) );
    }
    else
    {
        pUndo.reset( new EditUndoSetAttribs(mpEditEngine, aESel, rSet) );
    }

    SfxItemPool* pPool = pUndo->GetNewAttribs().GetPool();

    for ( sal_Int32 nPara = nStartNode; nPara <= nEndNode; nPara++ )
    {
        ContentNode* pNode = maEditDoc.GetObject( nPara );
        DBG_ASSERT( maEditDoc.GetObject( nPara ), "Node not found: CreateAttribUndo" );
        ContentAttribsInfo* pInf = new ContentAttribsInfo( pNode->GetContentAttribs().GetItems() );
        pUndo->AppendContentInfo(pInf);

        for ( sal_Int32 nAttr = 0; nAttr < pNode->GetCharAttribs().Count(); nAttr++ )
        {
            const EditCharAttrib& rAttr = *pNode->GetCharAttribs().GetAttribs()[nAttr];
            if (rAttr.GetLen())
            {
                EditCharAttrib* pNew = MakeCharAttrib(*pPool, *rAttr.GetItem(), rAttr.GetStart(), rAttr.GetEnd());
                pInf->AppendCharAttrib(pNew);
            }
        }
    }
    return pUndo;
}

ViewShellId ImpEditEngine::CreateViewShellId()
{
    ViewShellId nRet(-1);

    const EditView* pEditView = mpEditEngine ? mpEditEngine->GetActiveView() : nullptr;
    const OutlinerViewShell* pViewShell = pEditView ? pEditView->getImpl().GetViewShell() : nullptr;
    if (pViewShell)
        nRet = pViewShell->GetViewShellId();

    return nRet;
}

void ImpEditEngine::UndoActionStart( sal_uInt16 nId, const ESelection& aSel )
{
    DBG_ASSERT(IsInUndo(), "Calling UndoActionStart in Undomode!");
    if ( IsUndoEnabled() && !IsInUndo() )
    {
        GetUndoManager().EnterListAction( GetEditEnginePtr()->GetUndoComment( nId ), OUString(), nId, CreateViewShellId() );
        DBG_ASSERT( !moUndoMarkSelection, "UndoAction SelectionMarker?" );
        moUndoMarkSelection = aSel;
    }
}

void ImpEditEngine::UndoActionStart( sal_uInt16 nId )
{
    if ( IsUndoEnabled() && !IsInUndo() )
    {
        GetUndoManager().EnterListAction( GetEditEnginePtr()->GetUndoComment( nId ), OUString(), nId, CreateViewShellId() );
        DBG_ASSERT( !moUndoMarkSelection, "UndoAction SelectionMarker?" );
    }
}

void ImpEditEngine::UndoActionEnd()
{
    DBG_ASSERT(!IsInUndo(), "Calling UndoActionEnd in Undomode!");
    if ( IsUndoEnabled() && !IsInUndo() )
    {
        GetUndoManager().LeaveListAction();
        moUndoMarkSelection.reset();
    }
}

void ImpEditEngine::InsertUndo( std::unique_ptr<EditUndo> pUndo, bool bTryMerge )
{
    DBG_ASSERT( !IsInUndo(), "InsertUndo in Undo mode!" );
    if ( moUndoMarkSelection )
    {
        GetUndoManager().AddUndoAction( std::make_unique<EditUndoMarkSelection>(mpEditEngine, *moUndoMarkSelection) );
        moUndoMarkSelection.reset();
    }
    GetUndoManager().AddUndoAction( std::move(pUndo), bTryMerge );

    mbLastTryMerge = bTryMerge;
}

void ImpEditEngine::ResetUndoManager()
{
    if ( HasUndoManager() )
        GetUndoManager().Clear();
}

void ImpEditEngine::EnableUndo( bool bEnable )
{
    // When switching the mode Delete list:
    if ( bEnable != IsUndoEnabled() )
        ResetUndoManager();

    mbUndoEnabled = bEnable;
}

void ImpEditEngine::Undo( EditView* pView )
{
    if ( HasUndoManager() && GetUndoManager().GetUndoActionCount() )
    {
        SetActiveView( pView );
        GetUndoManager().Undo();
    }
}

void ImpEditEngine::Redo( EditView* pView )
{
    if ( HasUndoManager() && GetUndoManager().GetRedoActionCount() )
    {
        SetActiveView( pView );
        GetUndoManager().Redo();
    }
}

SfxItemSet ImpEditEngine::GetAttribs( EditSelection aSel, EditEngineAttribs nOnlyHardAttrib )
{

    aSel.Adjust( maEditDoc );

    SfxItemSet aCurSet( GetEmptyItemSet() );

    sal_Int32 nStartNode = maEditDoc.GetPos( aSel.Min().GetNode() );
    sal_Int32 nEndNode = maEditDoc.GetPos( aSel.Max().GetNode() );

    // iterate over the paragraphs ...
    for ( sal_Int32 nNode = nStartNode; nNode <= nEndNode; nNode++ )
    {
        ContentNode* pNode = maEditDoc.GetObject( nNode );
        assert( pNode && "Node not found: GetAttrib" );

        const sal_Int32 nStartPos = nNode==nStartNode ? aSel.Min().GetIndex() : 0;
        const sal_Int32 nEndPos = nNode==nEndNode ? aSel.Max().GetIndex() : pNode->Len(); // Can also be == nStart!

        // Problem: Templates...
        // =>  Other way:
        // 1) Hard character attributes, as usual...
        // 2) Examine Style and paragraph attributes only when OFF...

        // First the very hard formatting...
        if (pNode)
            EditDoc::FindAttribs( pNode, nStartPos, nEndPos, aCurSet );

        if( nOnlyHardAttrib != EditEngineAttribs::OnlyHard )
        {
            // and then paragraph formatting and template...
            for ( sal_uInt16 nWhich = EE_ITEMS_START; nWhich <= EE_CHAR_END; nWhich++)
            {
                if ( aCurSet.GetItemState( nWhich ) == SfxItemState::DEFAULT )
                {
                    const SfxPoolItem* pItem = nullptr;
                    if ( nOnlyHardAttrib == EditEngineAttribs::All )
                    {
                        const SfxPoolItem& rItem = pNode->GetContentAttribs().GetItem( nWhich );
                        aCurSet.Put( rItem );
                    }
                    else if ( pNode->GetContentAttribs().GetItems().GetItemState( nWhich, true, &pItem ) == SfxItemState::SET )
                    {
                        aCurSet.Put( *pItem );
                    }
                }
                else if ( aCurSet.GetItemState( nWhich ) == SfxItemState::SET )
                {
                    const SfxPoolItem* pItem = nullptr;
                    const SfxPoolItem* pTmpItem = nullptr;
                    if ( nOnlyHardAttrib == EditEngineAttribs::All )
                    {
                        pItem = &pNode->GetContentAttribs().GetItem( nWhich );
                    }
                    else if ( pNode->GetContentAttribs().GetItems().GetItemState( nWhich, true, &pTmpItem ) == SfxItemState::SET )
                    {
                        pItem = pTmpItem;
                    }
                    // pItem can only be NULL when nOnlyHardAttrib...
                    if ( !pItem || ( *pItem != aCurSet.Get( nWhich ) ) )
                    {
                        // Problem: When Paragraph style with for example font,
                        // but the Font is hard and completely different,
                        // wrong in selection  if invalidated....
                        // => better not invalidate, instead CHANGE!
                        // It would be better to fill each paragraph with
                        // an itemset and compare this in large.
                        if ( nWhich <= EE_PARA_END )
                            aCurSet.InvalidateItem( nWhich );
                    }
                }
            }
        }
    }

    // fill empty slots with defaults ...
    if ( nOnlyHardAttrib == EditEngineAttribs::All )
    {
        for ( sal_uInt16 nWhich = EE_ITEMS_START; nWhich <= EE_CHAR_END; nWhich++ )
        {
            if ( aCurSet.GetItemState( nWhich ) == SfxItemState::DEFAULT )
            {
                aCurSet.Put( maEditDoc.GetItemPool().GetUserOrPoolDefaultItem( nWhich ) );
            }
        }
    }
    return aCurSet;
}


SfxItemSet ImpEditEngine::GetAttribs( sal_Int32 nPara, sal_Int32 nStart, sal_Int32 nEnd, GetAttribsFlags nFlags ) const
{
    // Optimized function with fewer Puts(), which cause unnecessary cloning from default items.
    // If this works, change GetAttribs( EditSelection ) to use this for each paragraph and merge the results!


    ContentNode* pNode = const_cast<ContentNode*>(maEditDoc.GetObject(nPara));
    DBG_ASSERT( pNode, "GetAttribs - unknown paragraph!" );
    DBG_ASSERT( nStart <= nEnd, "getAttribs: Start > End not supported!" );

    SfxItemSet aAttribs(GetEmptyItemSet());

    if ( pNode )
    {
        if ( nEnd > pNode->Len() )
            nEnd = pNode->Len();

        if ( nStart > nEnd )
            nStart = nEnd;

        // StyleSheet / Parattribs...

        if ( pNode->GetStyleSheet() && ( nFlags & GetAttribsFlags::STYLESHEET ) )
            aAttribs.Set(pNode->GetStyleSheet()->GetItemSet());

        if ( nFlags & GetAttribsFlags::PARAATTRIBS )
            aAttribs.Put( pNode->GetContentAttribs().GetItems() );

        // CharAttribs...

        if ( nFlags & GetAttribsFlags::CHARATTRIBS )
        {
            // Make testing easier...
            pNode->GetCharAttribs().OptimizeRanges();

            const CharAttribList::AttribsType& rAttrs = pNode->GetCharAttribs().GetAttribs();
            for (const auto & nAttr : rAttrs)
            {
                const EditCharAttrib& rAttr = *nAttr;

                if ( nStart == nEnd )
                {
                    sal_Int32 nCursorPos = nStart;
                    if ( ( rAttr.GetStart() <= nCursorPos ) && ( rAttr.GetEnd() >= nCursorPos ) )
                    {
                        // To be used the attribute has to start BEFORE the position, or it must be a
                        // new empty attr AT the position, or we are on position 0.
                        if ( ( rAttr.GetStart() < nCursorPos ) || rAttr.IsEmpty() || !nCursorPos )
                        {
                            // maybe this attrib ends here and a new attrib with 0 Len may follow and be valid here,
                            // but that s no problem, the empty item will come later and win.
                            aAttribs.Put( *rAttr.GetItem() );
                        }
                    }
                }
                else
                {
                    // Check every attribute covering the area, partial or full.
                    if ( ( rAttr.GetStart() < nEnd ) && ( rAttr.GetEnd() > nStart ) )
                    {
                        if ( ( rAttr.GetStart() <= nStart ) && ( rAttr.GetEnd() >= nEnd ) )
                        {
                            // full coverage
                            aAttribs.Put( *rAttr.GetItem() );
                        }
                        else
                        {
                            // OptimizeRanges() assures that not the same attr can follow for full coverage
                            // only partial, check with current, when using para/style, otherwise invalid.
                            if ( !( nFlags & (GetAttribsFlags::PARAATTRIBS|GetAttribsFlags::STYLESHEET) ) ||
                                ( *rAttr.GetItem() != aAttribs.Get( rAttr.Which() ) ) )
                            {
                                aAttribs.InvalidateItem( rAttr.Which() );
                            }
                        }
                    }
                }

                if ( rAttr.GetStart() > nEnd )
                {
                    break;
                }
            }
        }
    }

    return aAttribs;
}


void ImpEditEngine::SetAttribs( EditSelection aSel, const SfxItemSet& rSet, SetAttribsMode nSpecial, bool bSetSelection )
{
    aSel.Adjust( maEditDoc );

    // When no selection => use the Attribute on the word.
    // ( the RTF-parser should actually never call the Method without a Range )
    if ( nSpecial == SetAttribsMode::WholeWord && !aSel.HasRange() )
        aSel = SelectWord( aSel, css::i18n::WordType::ANYWORD_IGNOREWHITESPACES, false );

    sal_Int32 nStartNode = maEditDoc.GetPos( aSel.Min().GetNode() );
    sal_Int32 nEndNode = maEditDoc.GetPos( aSel.Max().GetNode() );

    if (IsUndoEnabled() && !IsInUndo() && maStatus.DoUndoAttribs())
    {
        std::unique_ptr<EditUndoSetAttribs> pUndo = CreateAttribUndo( aSel, rSet );
        pUndo->SetSpecial( nSpecial );
        pUndo->SetUpdateSelection(bSetSelection);
        InsertUndo( std::move(pUndo) );
    }

    bool bCheckLanguage = false;
    if ( GetStatus().DoOnlineSpelling() )
    {
        bCheckLanguage = ( rSet.GetItemState( EE_CHAR_LANGUAGE ) == SfxItemState::SET ) ||
                         ( rSet.GetItemState( EE_CHAR_LANGUAGE_CJK ) == SfxItemState::SET ) ||
                         ( rSet.GetItemState( EE_CHAR_LANGUAGE_CTL ) == SfxItemState::SET );
    }

    // iterate over the paragraphs ...
    for ( sal_Int32 nNode = nStartNode; nNode <= nEndNode; nNode++ )
    {
        bool bParaAttribFound = false;
        bool bCharAttribFound = false;

        DBG_ASSERT( maEditDoc.GetObject( nNode ), "Node not found: SetAttribs" );
        DBG_ASSERT(GetParaPortions().exists(nNode), "Portion not found: SetAttribs");

        if (!GetParaPortions().exists(nNode))
            continue;

        ContentNode* pNode = maEditDoc.GetObject(nNode);
        ParaPortion& rPortion = GetParaPortions().getRef(nNode);

        const sal_Int32 nStartPos = nNode==nStartNode ? aSel.Min().GetIndex() : 0;
        const sal_Int32 nEndPos = nNode==nEndNode ? aSel.Max().GetIndex() : pNode->Len(); // can also be == nStart!

        // Iterate over the Items...
        for ( sal_uInt16 nWhich = EE_ITEMS_START; nWhich <= EE_CHAR_END; nWhich++)
        {
            if ( rSet.GetItemState( nWhich ) == SfxItemState::SET )
            {
                const SfxPoolItem& rItem = rSet.Get( nWhich );
                if ( nWhich <= EE_PARA_END )
                {
                    pNode->GetContentAttribs().GetItems().Put( rItem );
                    bParaAttribFound = true;
#if defined(YRS)
                    maEditDoc.YrsSetParaAttr(nNode, rItem);
#endif
                }
                else
                {
                    maEditDoc.InsertAttrib( pNode, nStartPos, nEndPos, rItem );
                    bCharAttribFound = true;
                    if ( nSpecial == SetAttribsMode::Edge )
                    {
                        CharAttribList::AttribsType& rAttribs = pNode->GetCharAttribs().GetAttribs();
                        for (std::unique_ptr<EditCharAttrib> & rAttrib : rAttribs)
                        {
                            EditCharAttrib& rAttr = *rAttrib;
                            if (rAttr.GetStart() > nEndPos)
                                break;

                            if (rAttr.GetEnd() == nEndPos && rAttr.Which() == nWhich)
                            {
                                rAttr.SetEdge(true);
                                break;
                            }
                        }
                    }
                }
            }
        }

        if ( bParaAttribFound )
        {
            ParaAttribsChanged(rPortion.GetNode());
        }
        else if ( bCharAttribFound )
        {
            mbFormatted = false;
            if ( !pNode->Len() || ( nStartPos != nEndPos  ) )
            {
                rPortion.MarkSelectionInvalid(nStartPos);
                if ( bCheckLanguage )
                    pNode->GetWrongList()->SetInvalidRange(nStartPos, nEndPos);
            }
        }
    }
}

void ImpEditEngine::RemoveAttribs( const ESelection& rSelection, bool bRemoveParaAttribs, sal_uInt16 nWhich )
{
    const EERemoveParaAttribsMode eMode = bRemoveParaAttribs?
        EERemoveParaAttribsMode::RemoveAll :
        EERemoveParaAttribsMode::RemoveCharItems;

    UndoActionStart(EDITUNDO_RESETATTRIBS);
    RemoveCharAttribs(CreateNormalizedSel(rSelection), eMode, nWhich);
    UndoActionEnd();
    if (IsUpdateLayout())
        FormatAndLayout();
}

void ImpEditEngine::RemoveCharAttribs( EditSelection aSel, EERemoveParaAttribsMode eMode, sal_uInt16 nWhich )
{
    aSel.Adjust( maEditDoc );

    sal_Int32 nStartNode = maEditDoc.GetPos( aSel.Min().GetNode() );
    sal_Int32 nEndNode = maEditDoc.GetPos( aSel.Max().GetNode() );
    bool bRemoveParaAttribs = eMode == EERemoveParaAttribsMode::RemoveAll;
    const SfxItemSet* _pEmptyItemSet = bRemoveParaAttribs ? &GetEmptyItemSet() : nullptr;

    if (IsUndoEnabled() && !IsInUndo() && maStatus.DoUndoAttribs())
    {
        // Possibly a special Undo, or itemset*
        std::unique_ptr<EditUndoSetAttribs> pUndo = CreateAttribUndo( aSel, GetEmptyItemSet() );
        pUndo->SetRemoveAttribs( true );
        pUndo->SetRemoveParaAttribs( bRemoveParaAttribs );
        pUndo->SetRemoveWhich( nWhich );
        InsertUndo( std::move(pUndo) );
    }

    // iterate over the paragraphs ...
    for ( sal_Int32 nNode = nStartNode; nNode <= nEndNode; nNode++ )
    {
        ContentNode* pNode = maEditDoc.GetObject( nNode );

        DBG_ASSERT( maEditDoc.GetObject( nNode ), "Node not found: SetAttribs" );
        DBG_ASSERT(GetParaPortions().exists(nNode), "Portion not found: SetAttribs");

        if (!GetParaPortions().exists(nNode))
            continue;

        ParaPortion& rPortion = GetParaPortions().getRef(nNode);

        const sal_Int32 nStartPos = nNode==nStartNode ? aSel.Min().GetIndex() : 0;
        const sal_Int32 nEndPos = nNode==nEndNode ? aSel.Max().GetIndex() : pNode->Len(); // can also be == nStart!

        // Optimize: If whole paragraph, then RemoveCharAttribs (nPara)?
        bool bChanged = maEditDoc.RemoveAttribs( pNode, nStartPos, nEndPos, nWhich );
        if ( bRemoveParaAttribs )
        {
            SetParaAttribs( nNode, *_pEmptyItemSet );   // Invalidated
        }
        else if (eMode == EERemoveParaAttribsMode::RemoveCharItems)
        {
            // For 'Format-Standard' also the character attributes should
            // disappear, which were set as paragraph attributes by the
            // DrawingEngine. These could not have been set by the user anyway.

            // #106871# Not when nWhich
            // Would have been better to offer a separate method for format/standard...
            if ( !nWhich )
            {
                SfxItemSet aAttribs( GetParaAttribs( nNode ) );
                for ( sal_uInt16 nW = EE_CHAR_START; nW <= EE_CHAR_END; nW++ )
                    aAttribs.ClearItem( nW );
                SetParaAttribs( nNode, aAttribs );
            }
        }

        if ( bChanged && !bRemoveParaAttribs )
        {
            mbFormatted = false;
            rPortion.MarkSelectionInvalid(nStartPos);
        }
    }
}

void ImpEditEngine::RemoveCharAttribs( sal_Int32 nPara, sal_uInt16 nWhich, bool bRemoveFeatures )
{
    ContentNode* pNode = maEditDoc.GetObject( nPara );
    ParaPortion* pPortion = GetParaPortions().SafeGetObject( nPara );

    DBG_ASSERT( pNode, "Node not found: RemoveCharAttribs" );
    DBG_ASSERT( pPortion, "Portion not found: RemoveCharAttribs" );

    if ( !pNode || !pPortion )
        return;

    size_t nAttr = 0;
    CharAttribList::AttribsType& rAttrs = pNode->GetCharAttribs().GetAttribs();
    EditCharAttrib* pAttr = GetAttrib(rAttrs, nAttr);
    while ( pAttr )
    {
        if ( ( !pAttr->IsFeature() || bRemoveFeatures ) &&
             ( !nWhich || ( pAttr->GetItem()->Which() == nWhich ) ) )
        {
            pNode->GetCharAttribs().Remove(nAttr);
        }
        else
        {
            nAttr++;
        }
        pAttr = GetAttrib(rAttrs, nAttr);
    }

#if OSL_DEBUG_LEVEL > 0 && !defined NDEBUG
    CharAttribList::DbgCheckAttribs(pNode->GetCharAttribs());
#endif

    pPortion->MarkSelectionInvalid( 0 );
}

void ImpEditEngine::SetParaAttribs( sal_Int32 nPara, const SfxItemSet& rSet )
{
    ContentNode* pNode = maEditDoc.GetObject( nPara );

    if ( !pNode )
        return;

    if ( pNode->GetContentAttribs().GetItems() == rSet )
        return;

    if (IsUndoEnabled() && !IsInUndo() && maStatus.DoUndoAttribs())
    {
        if ( rSet.GetPool() != &maEditDoc.GetItemPool() )
        {
            SfxItemSet aTmpSet( GetEmptyItemSet() );
            aTmpSet.Put( rSet );
            InsertUndo(std::make_unique<EditUndoSetParaAttribs>(mpEditEngine, nPara, pNode->GetContentAttribs().GetItems(), aTmpSet));
        }
        else
        {
            InsertUndo(std::make_unique<EditUndoSetParaAttribs>(mpEditEngine, nPara, pNode->GetContentAttribs().GetItems(), rSet));
        }
    }

    pNode->GetContentAttribs().GetItems().Set( rSet );

    if ( auto pWrongList = pNode->GetWrongList() )
    {
        bool bCheckLanguage = ( rSet.GetItemState( EE_CHAR_LANGUAGE ) == SfxItemState::SET ) ||
                     ( rSet.GetItemState( EE_CHAR_LANGUAGE_CJK ) == SfxItemState::SET ) ||
                     ( rSet.GetItemState( EE_CHAR_LANGUAGE_CTL ) == SfxItemState::SET );
        if (bCheckLanguage)
            pWrongList->ResetInvalidRange(0, pNode->Len());
    }

    if (maStatus.UseCharAttribs())
        pNode->CreateDefFont();

    ParaAttribsChanged( pNode );
}

const SfxItemSet& ImpEditEngine::GetParaAttribs( sal_Int32 nPara ) const
{
    const ContentNode* pNode = maEditDoc.GetObject( nPara );
    assert(pNode && "Node not found: GetParaAttribs");
    return pNode->GetContentAttribs().GetItems();
}

bool ImpEditEngine::HasParaAttrib( sal_Int32 nPara, sal_uInt16 nWhich ) const
{
    const ContentNode* pNode = maEditDoc.GetObject( nPara );
    assert(pNode && "Node not found: HasParaAttrib");
    return pNode->GetContentAttribs().HasItem( nWhich );
}

const SfxPoolItem& ImpEditEngine::GetParaAttrib( sal_Int32 nPara, sal_uInt16 nWhich ) const
{
    const ContentNode* pNode = maEditDoc.GetObject(nPara);
    assert(pNode && "Node not found: GetParaAttrib");
    return pNode->GetContentAttribs().GetItem(nWhich);
}

void ImpEditEngine::GetCharAttribs( sal_Int32 nPara, std::vector<EECharAttrib>& rLst ) const
{
    rLst.clear();
    const ContentNode* pNode = maEditDoc.GetObject( nPara );
    if ( !pNode )
        return;

    rLst.reserve(pNode->GetCharAttribs().Count());
    const CharAttribList::AttribsType& rAttrs = pNode->GetCharAttribs().GetAttribs();
    for (const auto & i : rAttrs)
    {
        const EditCharAttrib& rAttr = *i;
        EECharAttrib aEEAttr(rAttr.GetStart(), rAttr.GetEnd(), rAttr.GetItem());
        rLst.push_back(aEEAttr);
    }
}

void ImpEditEngine::ParaAttribsToCharAttribs( ContentNode* pNode )
{
    pNode->GetCharAttribs().DeleteEmptyAttribs();
    sal_Int32 nEndPos = pNode->Len();
    for ( sal_uInt16 nWhich = EE_CHAR_START; nWhich <= EE_CHAR_END; nWhich++ )
    {
        if ( pNode->GetContentAttribs().HasItem( nWhich ) )
        {
            const SfxPoolItem& rItem = pNode->GetContentAttribs().GetItem( nWhich );
            // Fill the gap:
            sal_Int32 nLastEnd = 0;
            const EditCharAttrib* pAttr = pNode->GetCharAttribs().FindNextAttrib( nWhich, nLastEnd );
            while ( pAttr )
            {
                nLastEnd = pAttr->GetEnd();
                if ( pAttr->GetStart() > nLastEnd )
                    maEditDoc.InsertAttrib( pNode, nLastEnd, pAttr->GetStart(), rItem );
                // #112831# Last Attr might go from 0xffff to 0x0000
                pAttr = nLastEnd ? pNode->GetCharAttribs().FindNextAttrib( nWhich, nLastEnd ) : nullptr;
            }

            // And the Rest:
            if ( nLastEnd < nEndPos )
                maEditDoc.InsertAttrib( pNode, nLastEnd, nEndPos, rItem );
        }
    }
    mbFormatted = false;
    // Portion does not need to be invalidated here, happens elsewhere.
}

void ImpEditEngine::SetPolygon(const basegfx::B2DPolyPolygon& rPolyPolygon, const basegfx::B2DPolyPolygon* pLinePolyPolygon)
{
    bool bSimple(false);

    if(pLinePolyPolygon && 1 == rPolyPolygon.count())
    {
        if(rPolyPolygon.getB2DPolygon(0).isClosed())
        {
            // open polygon
            bSimple = true;
        }
    }

    TextRanger* pRanger = new TextRanger( rPolyPolygon, pLinePolyPolygon, 30, 2, 2, bSimple, true );
    SetTextRanger( std::unique_ptr<TextRanger>(pRanger) );
    maPaperSize = pRanger->GetBoundRect().GetSize();
}

void ImpEditEngine::InsertView(EditView* pEditView, size_t nIndex)
{
    if (nIndex > maEditViews.size())
        nIndex = maEditViews.size();

    maEditViews.insert(maEditViews.begin()+nIndex, pEditView);

    EditSelection aStartSel( maEditDoc.GetStartPaM() );
    pEditView->getImpl().SetEditSelection( aStartSel );
    if (!mpActiveView)
        SetActiveView(pEditView);

    pEditView->getImpl().AddDragAndDropListeners();
}

EditView* ImpEditEngine::RemoveView( EditView* pView )
{
    pView->HideCursor();
    EditView* pRemoved = nullptr;
    ImpEditEngine::ViewsType::iterator it = std::find(maEditViews.begin(), maEditViews.end(), pView);

    DBG_ASSERT( it != maEditViews.end(), "RemoveView with invalid index" );
    if (it != maEditViews.end())
    {
        pRemoved = *it;
        maEditViews.erase(it);
        if (mpActiveView == pView)
        {
            SetActiveView(nullptr);
            GetSelEngine().SetCurView(nullptr);
        }
        pView->getImpl().RemoveDragAndDropListeners();

    }
    return pRemoved;
}

void ImpEditEngine::RemoveView(size_t nIndex)
{
    if (nIndex >= maEditViews.size())
        return;

    EditView* pView = maEditViews[nIndex];
    if ( pView )
        RemoveView( pView );
}

bool ImpEditEngine::HasView( EditView* pView ) const
{
    return std::find(maEditViews.begin(), maEditViews.end(), pView) != maEditViews.end();
}

void ImpEditEngine::SetDefTab( sal_uInt16 nDefTab )
{
    maEditDoc.SetDefTab(nDefTab);
    if (IsFormatted())
    {
        FormatFullDoc();
        UpdateViews();
    }
}

bool ImpEditEngine::PostKeyEvent( const KeyEvent& rKeyEvent, EditView* pEditView, vcl::Window const * pFrameWin )
{
    DBG_ASSERT( pEditView, "no View - no cookie !" );

    bool bDone = true;

    bool bModified  = false;
    bool bMoved     = false;
    bool bAllowIdle = true;
    bool bReadOnly  = pEditView->IsReadOnly();

    CursorFlags aNewCursorFlags;
    bool bSetCursorFlags = true;

    EditSelection aCurSel( pEditView->getImpl().GetEditSelection() );
    DBG_ASSERT( !aCurSel.IsInvalid(), "Blinde Selection in EditEngine::PostKeyEvent" );

    OUString aAutoText(maAutoCompleteText);
    if (!maAutoCompleteText.isEmpty())
        SetAutoCompleteText(OUString(), true);

    sal_uInt16 nCode = rKeyEvent.GetKeyCode().GetCode();
    KeyFuncType eFunc = rKeyEvent.GetKeyCode().GetFunction();
    if ( eFunc != KeyFuncType::DONTKNOW )
    {
        switch ( eFunc )
        {
            case KeyFuncType::UNDO:
            {
                if ( !bReadOnly )
                    pEditView->Undo();
                return true;
            }
            case KeyFuncType::REDO:
            {
                if ( !bReadOnly )
                    pEditView->Redo();
                return true;
            }

            default:    // is then possible edited below.
                break;
        }
    }

    {
        switch ( nCode )
        {
#if defined( DBG_UTIL ) || (OSL_DEBUG_LEVEL > 1)
            case KEY_F1:
            {
                if ( rKeyEvent.GetKeyCode().IsMod1() && rKeyEvent.GetKeyCode().IsMod2() )
                {
                    sal_Int32 nParas = maEditDoc.Count();
                    Point aPos;
                    Point aViewStart( pEditView->GetOutputArea().TopLeft() );
                    tools::Long n20 = 40 * mnOnePixelInRef;
                    for ( sal_Int32 n = 0; n < nParas; n++ )
                    {
                        if (!IsFormatted())
                            FormatDoc();
                        tools::Long nH = GetParaHeight(n);
                        Point P1( aViewStart.X() + n20 + n20*(n%2), aViewStart.Y() + aPos.Y() );
                        Point P2( P1 );
                        P2.AdjustX(n20 );
                        P2.AdjustY(nH );
                        pEditView->GetWindow()->GetOutDev()->SetLineColor();
                        pEditView->GetWindow()->GetOutDev()->SetFillColor( (n%2) ? COL_YELLOW : COL_LIGHTGREEN );
                        pEditView->GetWindow()->GetOutDev()->DrawRect( tools::Rectangle( P1, P2 ) );
                        aPos.AdjustY(nH );
                    }
                }
                bDone = false;
            }
            break;
            case KEY_F11:
            {
                if ( rKeyEvent.GetKeyCode().IsMod1() && rKeyEvent.GetKeyCode().IsMod2() )
                {
                    ImpEditEngine::bDebugPaint = !ImpEditEngine::bDebugPaint;
                    OStringBuffer aInfo("DebugPaint: ");
                    aInfo.append(ImpEditEngine::bDebugPaint ? "On" : "Off");
                    std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(pEditView->GetWindow()->GetFrameWeld(),
                                                                  VclMessageType::Info, VclButtonsType::Ok,
                                                                  OStringToOUString(aInfo, RTL_TEXTENCODING_ASCII_US)));
                    xInfoBox->run();

                }
                bDone = false;
            }
            break;
            case KEY_F12:
            {
                if ( rKeyEvent.GetKeyCode().IsMod1() && rKeyEvent.GetKeyCode().IsMod2() )
                    DumpData(true);
                bDone = false;
            }
            break;
#endif
            case KEY_UP:
            case KEY_DOWN:
            case KEY_LEFT:
            case KEY_RIGHT:
            case KEY_HOME:
            case KEY_END:
            case KEY_PAGEUP:
            case KEY_PAGEDOWN:
            case css::awt::Key::MOVE_WORD_FORWARD:
            case css::awt::Key::SELECT_WORD_FORWARD:
            case css::awt::Key::MOVE_WORD_BACKWARD:
            case css::awt::Key::SELECT_WORD_BACKWARD:
            case css::awt::Key::MOVE_TO_BEGIN_OF_LINE:
            case css::awt::Key::MOVE_TO_END_OF_LINE:
            case css::awt::Key::SELECT_TO_BEGIN_OF_LINE:
            case css::awt::Key::SELECT_TO_END_OF_LINE:
            case css::awt::Key::MOVE_TO_BEGIN_OF_PARAGRAPH:
            case css::awt::Key::MOVE_TO_END_OF_PARAGRAPH:
            case css::awt::Key::SELECT_TO_BEGIN_OF_PARAGRAPH:
            case css::awt::Key::SELECT_TO_END_OF_PARAGRAPH:
            case css::awt::Key::MOVE_TO_BEGIN_OF_DOCUMENT:
            case css::awt::Key::MOVE_TO_END_OF_DOCUMENT:
            case css::awt::Key::SELECT_TO_BEGIN_OF_DOCUMENT:
            case css::awt::Key::SELECT_TO_END_OF_DOCUMENT:
            {
                if ( !rKeyEvent.GetKeyCode().IsMod2() || ( nCode == KEY_LEFT ) || ( nCode == KEY_RIGHT ) )
                {
                    if ( ImpEditEngine::DoVisualCursorTraveling() && ( ( nCode == KEY_LEFT ) || ( nCode == KEY_RIGHT ) /* || ( nCode == KEY_HOME ) || ( nCode == KEY_END ) */ ) )
                        bSetCursorFlags = false;    // Will be manipulated within visual cursor move

                    aCurSel = MoveCursor( rKeyEvent, pEditView );

                    if ( aCurSel.HasRange() ) {
                        css::uno::Reference<css::datatransfer::clipboard::XClipboard> aSelection(GetSystemPrimarySelection());
                        pEditView->getImpl().CutCopy( aSelection, false );
                    }

                    bMoved = true;
                    if ( nCode == KEY_HOME )
                        aNewCursorFlags.bStartOfLine = true;
                    else if ( nCode == KEY_END )
                        aNewCursorFlags.bEndOfLine = true;

                }
#if OSL_DEBUG_LEVEL > 1
                GetLanguage(getImpl().GetEditDoc().GetPos( aCurSel.Max().GetNode() ), aCurSel.Max().GetIndex());
#endif
            }
            break;
            case KEY_BACKSPACE:
            case KEY_DELETE:
            case css::awt::Key::DELETE_WORD_BACKWARD:
            case css::awt::Key::DELETE_WORD_FORWARD:
            case css::awt::Key::DELETE_TO_BEGIN_OF_PARAGRAPH:
            case css::awt::Key::DELETE_TO_END_OF_PARAGRAPH:
            {
                if ( !bReadOnly && !rKeyEvent.GetKeyCode().IsMod2() )
                {
                    // check if we are behind a bullet and using the backspace key
                    ContentNode *pNode = aCurSel.Min().GetNode();
                    const SvxNumberFormat *pFmt = GetNumberFormat( pNode );
                    if (pFmt && nCode == KEY_BACKSPACE &&
                        !aCurSel.HasRange() && aCurSel.Min().GetIndex() == 0)
                    {
                        // if the bullet is still visible, just make it invisible.
                        // Otherwise continue as usual.


                        sal_Int32 nPara = maEditDoc.GetPos( pNode );
                        SfxBoolItem aBulletState(GetParaAttrib(nPara, EE_PARA_BULLETSTATE));

                        if ( aBulletState.GetValue() )
                        {

                            aBulletState.SetValue( false );
                            SfxItemSet aSet( GetParaAttribs( nPara ) );
                            aSet.Put( aBulletState );
                            SetParaAttribs( nPara, aSet );

                            // have this and the following paragraphs formatted and repainted.
                            // (not painting a numbering in the list may cause the following
                            // numberings to have different numbers than before and thus the
                            // length may have changed as well )
                            FormatAndLayout(mpActiveView);

                            break;
                        }
                    }

                    sal_uInt8 nDel = 0;
                    DeleteMode nMode = DeleteMode::Simple;
                    switch( nCode )
                    {
                    case css::awt::Key::DELETE_WORD_BACKWARD:
                        nMode = DeleteMode::RestOfWord;
                        nDel = DEL_LEFT;
                        break;
                    case css::awt::Key::DELETE_WORD_FORWARD:
                        nMode = DeleteMode::RestOfWord;
                        nDel = DEL_RIGHT;
                        break;
                    case css::awt::Key::DELETE_TO_BEGIN_OF_PARAGRAPH:
                        nMode = DeleteMode::RestOfContent;
                        nDel = DEL_LEFT;
                        break;
                    case css::awt::Key::DELETE_TO_END_OF_PARAGRAPH:
                        nMode = DeleteMode::RestOfContent;
                        nDel = DEL_RIGHT;
                        break;
                    default:
                        nDel = ( nCode == KEY_DELETE ) ? DEL_RIGHT : DEL_LEFT;
                        nMode = rKeyEvent.GetKeyCode().IsMod1() ? DeleteMode::RestOfWord : DeleteMode::Simple;
                        if ( ( nMode == DeleteMode::RestOfWord ) && rKeyEvent.GetKeyCode().IsShift() )
                            nMode = DeleteMode::RestOfContent;
                        break;
                    }

                    pEditView->getImpl().DrawSelectionXOR();
                    UndoActionStart( EDITUNDO_DELETE );
                    aCurSel = DeleteLeftOrRight( aCurSel, nDel, nMode );
                    UndoActionEnd();
                    bModified = true;
                    bAllowIdle = false;
                }
            }
            break;
            case KEY_TAB:
            {
                if ( !bReadOnly && !rKeyEvent.GetKeyCode().IsMod1() && !rKeyEvent.GetKeyCode().IsMod2() )
                {
                    bool bShift = rKeyEvent.GetKeyCode().IsShift();
                    if ( !bShift )
                    {
                        bool bSel = pEditView->HasSelection();
                        if ( bSel )
                            UndoActionStart( EDITUNDO_INSERT );
                        if ( GetStatus().DoAutoCorrect() )
                            aCurSel = AutoCorrect( aCurSel, 0, !pEditView->IsInsertMode(), pFrameWin );
                        aCurSel = InsertTab( aCurSel );
                        if ( bSel )
                            UndoActionEnd();
                        bModified = true;
                    }
                }
                else
                    bDone = false;
            }
            break;
            case KEY_RETURN:
            {
                if ( !bReadOnly )
                {
                    pEditView->getImpl().DrawSelectionXOR();
                    if ( !rKeyEvent.GetKeyCode().IsMod1() && !rKeyEvent.GetKeyCode().IsMod2() )
                    {
                        UndoActionStart( EDITUNDO_INSERT );
                        if ( rKeyEvent.GetKeyCode().IsShift() )
                        {
                            aCurSel = AutoCorrect( aCurSel, 0, !pEditView->IsInsertMode(), pFrameWin );
                            aCurSel = InsertLineBreak( aCurSel );
                        }
                        else
                        {
                            if (aAutoText.isEmpty())
                            {
                                if (GetStatus().DoAutoCorrect())
                                    aCurSel = AutoCorrect( aCurSel, 0, !pEditView->IsInsertMode(), pFrameWin );
                                aCurSel = InsertParaBreak( aCurSel );
                            }
                            else
                            {
                                DBG_ASSERT( !aCurSel.HasRange(), "Selection on complete?!" );
                                EditPaM aStart = WordLeft(aCurSel.Max());
                                EditSelection aSelection(aStart, aCurSel.Max());
                                aCurSel = InsertText(aSelection, aAutoText);
                                SetAutoCompleteText( OUString(), true );
                            }
                        }
                        UndoActionEnd();
                        bModified = true;
                    }
                }
            }
            break;
            case KEY_INSERT:
            {
                if ( !rKeyEvent.GetKeyCode().IsMod1() && !rKeyEvent.GetKeyCode().IsMod2() )
                    pEditView->SetInsertMode( !pEditView->IsInsertMode() );
            }
            break;
            default:
            {
                #if (OSL_DEBUG_LEVEL > 1) && defined(DBG_UTIL)
                    if ( ( nCode == KEY_W ) && rKeyEvent.GetKeyCode().IsMod1() && rKeyEvent.GetKeyCode().IsMod2() )
                    {
                        SfxItemSet aAttribs = pEditView->GetAttribs();
                        const SvxFrameDirectionItem& rCurrentWritingMode = (const SvxFrameDirectionItem&)aAttribs.Get( EE_PARA_WRITINGDIR );
                        SvxFrameDirectionItem aNewItem( SvxFrameDirection::Horizontal_LR_TB, EE_PARA_WRITINGDIR );
                        if ( rCurrentWritingMode.GetValue() != SvxFrameDirection::Horizontal_RL_TB )
                            aNewItem.SetValue( SvxFrameDirection::Horizontal_RL_TB );
                        aAttribs.Put( aNewItem );
                        pEditView->SetAttribs( aAttribs );
                    }
                #endif
                if ( !bReadOnly && IsSimpleCharInput( rKeyEvent ) )
                {
                    sal_Unicode nCharCode = rKeyEvent.GetCharCode();
                    pEditView->getImpl().DrawSelectionXOR();
                    // Autocorrection?
                    if (GetStatus().DoAutoCorrect() &&
                        (SvxAutoCorrect::IsAutoCorrectChar(nCharCode) ||
                            IsNbspRunNext()))
                    {
                        aCurSel = AutoCorrect(
                            aCurSel, nCharCode, !pEditView->IsInsertMode(), pFrameWin );
                    }
                    else
                    {
                        aCurSel = InsertTextUserInput( aCurSel, nCharCode, !pEditView->IsInsertMode() );
                    }
                    // AutoComplete ???
                    if ( GetStatus().DoAutoComplete() && ( nCharCode != ' ' ) )
                    {
                        // Only at end of word...
                        sal_Int32 nIndex = aCurSel.Max().GetIndex();
                        if ((nIndex >= aCurSel.Max().GetNode()->Len()) ||
                             (maWordDelimiters.indexOf(aCurSel.Max().GetNode()->GetChar(nIndex)) != -1))
                        {
                            EditPaM aStart(WordLeft(aCurSel.Max()));
                            OUString aWord = GetSelected(EditSelection(aStart, aCurSel.Max()));
                            if ( aWord.getLength() >= 3 )
                            {
                                OUString aComplete;

                                LanguageType eLang = GetLanguage(EditPaM( aStart.GetNode(), aStart.GetIndex()+1)).nLang;
                                LanguageTag aLanguageTag( eLang);

                                if (!mxLocaleDataWrapper.isInitialized())
                                    mxLocaleDataWrapper.init( SvtSysLocale().GetLocaleData().getComponentContext(), aLanguageTag);
                                else
                                    mxLocaleDataWrapper.changeLocale( aLanguageTag);

                                if (!mxTransliterationWrapper.isInitialized())
                                    mxTransliterationWrapper.init( SvtSysLocale().GetLocaleData().getComponentContext(), eLang);
                                else
                                    mxTransliterationWrapper.changeLocale( eLang);

                                const ::utl::TransliterationWrapper* pTransliteration = mxTransliterationWrapper.get();
                                css::uno::Sequence< css::i18n::CalendarItem2 > xItem = mxLocaleDataWrapper->getDefaultCalendarDays();
                                sal_Int32 nCount = xItem.getLength();
                                const css::i18n::CalendarItem2* pArr = xItem.getConstArray();
                                for( sal_Int32 n = 0; n <= nCount; ++n )
                                {
                                    const OUString& rDay = pArr[n].FullName;
                                    if( pTransliteration->isMatch( aWord, rDay) )
                                    {
                                        aComplete = rDay;
                                        break;
                                    }
                                }

                                if ( aComplete.isEmpty() )
                                {
                                    xItem = mxLocaleDataWrapper->getDefaultCalendarMonths();
                                    sal_Int32 nMonthCount = xItem.getLength();
                                    const css::i18n::CalendarItem2* pMonthArr = xItem.getConstArray();
                                    for( sal_Int32 n = 0; n <= nMonthCount; ++n )
                                    {
                                        const OUString& rMon = pMonthArr[n].FullName;
                                        if( pTransliteration->isMatch( aWord, rMon) )
                                        {
                                            aComplete = rMon;
                                            break;
                                        }
                                    }
                                }

                                if( !aComplete.isEmpty() && ( ( aWord.getLength() + 1 ) < aComplete.getLength() ) )
                                {
                                    SetAutoCompleteText( aComplete, false );
                                    Point aPos = PaMtoEditCursor( aCurSel.Max() ).TopLeft();
                                    aPos = pEditView->getImpl().GetWindowPos( aPos );
                                    aPos = pEditView->getImpl().GetWindow()->LogicToPixel( aPos );
                                    aPos = pEditView->GetWindow()->OutputToScreenPixel( aPos );
                                    aPos.AdjustY( -3 );
                                    Help::ShowQuickHelp( pEditView->GetWindow(), tools::Rectangle( aPos, Size( 1, 1 ) ), aComplete, QuickHelpFlags::Bottom|QuickHelpFlags::Left );
                                }
                            }
                        }
                    }
                    bModified = true;
                }
                else
                    bDone = false;
            }
        }
    }

    pEditView->getImpl().SetEditSelection( aCurSel );
    if (comphelper::LibreOfficeKit::isActive())
    {
        pEditView->getImpl().DrawSelectionXOR();
    }
    UpdateSelections();

    if ( ( !IsEffectivelyVertical() && ( nCode != KEY_UP ) && ( nCode != KEY_DOWN ) ) ||
         ( IsEffectivelyVertical() && ( nCode != KEY_LEFT ) && ( nCode != KEY_RIGHT ) ))
    {
        pEditView->getImpl().mnTravelXPos = TRAVEL_X_DONTKNOW;
    }

    if ( /* ( nCode != KEY_HOME ) && ( nCode != KEY_END ) && */
        ( !IsEffectivelyVertical() && ( nCode != KEY_LEFT ) && ( nCode != KEY_RIGHT ) ) ||
         ( IsEffectivelyVertical() && ( nCode != KEY_UP ) && ( nCode != KEY_DOWN ) ))
    {
        pEditView->getImpl().SetCursorBidiLevel( CURSOR_BIDILEVEL_DONTKNOW );
    }

    if (bSetCursorFlags)
        pEditView->getImpl().maExtraCursorFlags = aNewCursorFlags;

    if ( bModified )
    {
        DBG_ASSERT( !bReadOnly, "ReadOnly but modified???" );
        // Idle-Formatter only when AnyInput.
        if ( bAllowIdle && GetStatus().UseIdleFormatter()
                && Application::AnyInput( VclInputFlags::KEYBOARD) )
            IdleFormatAndLayout( pEditView );
        else
            FormatAndLayout( pEditView );
    }
    else if ( bMoved )
    {
        bool bGotoCursor = pEditView->getImpl().DoAutoScroll();
        pEditView->getImpl().ShowCursor( bGotoCursor, true );
        CallStatusHdl();
    }

    return bDone;
}

bool ImpEditEngine::IsSimpleCharInput( const KeyEvent& rKeyEvent )
{
    return EditEngine::IsPrintable( rKeyEvent.GetCharCode() ) &&
        ( KEY_MOD2 != (rKeyEvent.GetKeyCode().GetModifier() & ~KEY_SHIFT ) ) &&
        ( KEY_MOD1 != (rKeyEvent.GetKeyCode().GetModifier() & ~KEY_SHIFT ) );
}

void ImpEditEngine::SetControlWord( EEControlBits nWord )
{

    if (nWord == maStatus.GetControlWord())
        return;

    EEControlBits nPrev = maStatus.GetControlWord();
    maStatus.GetControlWord() = nWord;

    EEControlBits nChanges = nPrev ^ nWord;
    if (IsFormatted())
    {
        // possibly reformat:
        if ( ( nChanges & EEControlBits::USECHARATTRIBS ) ||
             ( nChanges & EEControlBits::ONECHARPERLINE ) ||
             ( nChanges & EEControlBits::STRETCHING ) ||
             ( nChanges & EEControlBits::OUTLINER ) ||
             ( nChanges & EEControlBits::NOCOLORS ) ||
             ( nChanges & EEControlBits::OUTLINER2 ) )
        {
            if ( nChanges & EEControlBits::USECHARATTRIBS )
            {
                maEditDoc.CreateDefFont(true);
            }

            FormatFullDoc();
            UpdateViews(mpActiveView);
        }
    }

    bool bSpellingChanged = bool(nChanges & EEControlBits::ONLINESPELLING);

    if ( !bSpellingChanged )
        return;

    StopOnlineSpellTimer();
    if (nWord & EEControlBits::ONLINESPELLING)
    {
        // Create WrongList, start timer...
        sal_Int32 nNodes = maEditDoc.Count();
        for (sal_Int32 nNode = 0; nNode < nNodes; nNode++)
        {
            ContentNode* pNode = maEditDoc.GetObject(nNode);
            pNode->CreateWrongList();
        }
        if (IsFormatted())
            StartOnlineSpellTimer();
    }
    else
    {
        tools::Long nY = 0;
        sal_Int32 nNodes = maEditDoc.Count();
        for ( sal_Int32 nNode = 0; nNode < nNodes; nNode++)
        {
            ContentNode* pNode = maEditDoc.GetObject(nNode);
            ParaPortion const& rPortion = maParaPortionList.getRef(nNode);
            bool bWrongs = false;
            if (pNode->GetWrongList() != nullptr)
                bWrongs = !pNode->GetWrongList()->empty();
            pNode->DestroyWrongList();
            if ( bWrongs )
            {
                maInvalidRect.SetLeft(0);
                maInvalidRect.SetRight(GetPaperSize().Width());
                maInvalidRect.SetTop(nY + 1);
                maInvalidRect.SetBottom(nY + rPortion.GetHeight() - 1);
                UpdateViews(mpActiveView);
            }
            nY += rPortion.GetHeight();
        }
    }
}

IdleFormattter::IdleFormattter()
    : Idle("editeng::ImpEditEngine aIdleFormatter")
{
}

IdleFormattter::~IdleFormattter()
{
    mpView = nullptr;
}

void IdleFormattter::DoIdleFormat(EditView* pView)
{
    mpView = pView;

    if (IsActive())
        mnRestarts++;

    if (mnRestarts > 4)
        ForceTimeout();
    else
        Start();
}

void IdleFormattter::ForceTimeout()
{
    if (IsActive())
    {
        Stop();
        Invoke();
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
