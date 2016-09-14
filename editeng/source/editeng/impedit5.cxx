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

#include <vcl/wrkwin.hxx>
#include <vcl/dialog.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/svapp.hxx>
#include <impedit.hxx>
#include <editeng/editeng.hxx>
#include <editdbg.hxx>
#include <svl/hint.hxx>
#include <editeng/lrspitem.hxx>

void ImpEditEngine::SetStyleSheetPool( SfxStyleSheetPool* pSPool )
{
    if ( pStylePool != pSPool )
    {
        pStylePool = pSPool;
    }
}

const SfxStyleSheet* ImpEditEngine::GetStyleSheet( sal_Int32 nPara ) const
{
    const ContentNode* pNode = aEditDoc.GetObject( nPara );
    return pNode ? pNode->GetContentAttribs().GetStyleSheet() : nullptr;
}

SfxStyleSheet* ImpEditEngine::GetStyleSheet( sal_Int32 nPara )
{
    ContentNode* pNode = aEditDoc.GetObject( nPara );
    return pNode ? pNode->GetContentAttribs().GetStyleSheet() : nullptr;
}

void ImpEditEngine::SetStyleSheet( EditSelection aSel, SfxStyleSheet* pStyle )
{
    aSel.Adjust( aEditDoc );

    sal_Int32 nStartPara = aEditDoc.GetPos( aSel.Min().GetNode() );
    sal_Int32 nEndPara = aEditDoc.GetPos( aSel.Max().GetNode() );

    bool _bUpdate = GetUpdateMode();
    SetUpdateMode( false );

    for ( sal_Int32 n = nStartPara; n <= nEndPara; n++ )
        SetStyleSheet( n, pStyle );

    SetUpdateMode( _bUpdate );
}

void ImpEditEngine::SetStyleSheet( sal_Int32 nPara, SfxStyleSheet* pStyle )
{
    DBG_ASSERT( GetStyleSheetPool() || !pStyle, "SetStyleSheet: No StyleSheetPool registered!" );
    ContentNode* pNode = aEditDoc.GetObject( nPara );
    SfxStyleSheet* pCurStyle = pNode->GetStyleSheet();
    if ( pStyle != pCurStyle )
    {
        if ( IsUndoEnabled() && !IsInUndo() && aStatus.DoUndoAttribs() )
        {
            OUString aPrevStyleName;
            if ( pCurStyle )
                aPrevStyleName = pCurStyle->GetName();

            OUString aNewStyleName;
            if ( pStyle )
                aNewStyleName = pStyle->GetName();

            InsertUndo(
                new EditUndoSetStyleSheet(pEditEngine, aEditDoc.GetPos( pNode ),
                        aPrevStyleName, pCurStyle ? pCurStyle->GetFamily() : SfxStyleFamily::Para,
                        aNewStyleName, pStyle ? pStyle->GetFamily() : SfxStyleFamily::Para,
                        pNode->GetContentAttribs().GetItems() ) );
        }
        if ( pCurStyle )
            EndListening( *pCurStyle );
        pNode->SetStyleSheet( pStyle, aStatus.UseCharAttribs() );
        if ( pStyle )
            StartListening( *pStyle );
        ParaAttribsChanged( pNode );
    }
    FormatAndUpdate();
}

void ImpEditEngine::UpdateParagraphsWithStyleSheet( SfxStyleSheet* pStyle )
{
    SvxFont aFontFromStyle;
    CreateFont( aFontFromStyle, pStyle->GetItemSet() );

    bool bUsed = false;
    for ( sal_Int32 nNode = 0; nNode < aEditDoc.Count(); nNode++ )
    {
        ContentNode* pNode = aEditDoc.GetObject( nNode );
        if ( pNode->GetStyleSheet() == pStyle )
        {
            bUsed = true;
            if ( aStatus.UseCharAttribs() )
                pNode->SetStyleSheet( pStyle, aFontFromStyle );
            else
                pNode->SetStyleSheet( pStyle, false );

            ParaAttribsChanged( pNode );
        }
    }
    if ( bUsed )
    {
        GetEditEnginePtr()->StyleSheetChanged( pStyle );
        FormatAndUpdate();
    }
}

void ImpEditEngine::RemoveStyleFromParagraphs( SfxStyleSheet* pStyle )
{
    for ( sal_Int32 nNode = 0; nNode < aEditDoc.Count(); nNode++ )
    {
        ContentNode* pNode = aEditDoc.GetObject(nNode);
        if ( pNode->GetStyleSheet() == pStyle )
        {
            pNode->SetStyleSheet( nullptr );
            ParaAttribsChanged( pNode );
        }
    }
    FormatAndUpdate();
}

void ImpEditEngine::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    // So that not a lot of unnecessary formatting is done when destructing:
    if ( !bDowning )
    {

        SfxStyleSheet* pStyle = nullptr;
        sal_uInt32 nId = 0;

        const SfxStyleSheetHint* pStyleSheetHint = dynamic_cast<const SfxStyleSheetHint*>(&rHint);
        if ( pStyleSheetHint )
        {
            DBG_ASSERT( dynamic_cast< const SfxStyleSheet* >(pStyleSheetHint->GetStyleSheet()) != nullptr, "No SfxStyleSheet!" );
            pStyle = static_cast<SfxStyleSheet*>( pStyleSheetHint->GetStyleSheet() );
            nId = pStyleSheetHint->GetHint();
        }
        else if ( dynamic_cast< const SfxStyleSheet* >(&rBC) !=  nullptr )
        {
            pStyle = static_cast<SfxStyleSheet*>(&rBC);
            nId = rHint.GetId();
        }

        if ( pStyle )
        {
            if ( ( nId == SFX_HINT_DYING ) ||
                 ( nId == SfxStyleSheetHintId::INDESTRUCTION ) ||
                 ( nId == SfxStyleSheetHintId::ERASED ) )
            {
                RemoveStyleFromParagraphs( pStyle );
            }
            else if ( ( nId == SFX_HINT_DATACHANGED ) ||
                      ( nId == SfxStyleSheetHintId::MODIFIED ) )
            {
                UpdateParagraphsWithStyleSheet( pStyle );
            }
        }
    }
}

EditUndoSetAttribs* ImpEditEngine::CreateAttribUndo( EditSelection aSel, const SfxItemSet& rSet )
{
    DBG_ASSERT( !aSel.DbgIsBuggy( aEditDoc ), "CreateAttribUndo: Incorrect selection ");
    aSel.Adjust( aEditDoc );

    ESelection aESel( CreateESel( aSel ) );

    sal_Int32 nStartNode = aEditDoc.GetPos( aSel.Min().GetNode() );
    sal_Int32 nEndNode = aEditDoc.GetPos( aSel.Max().GetNode() );

    DBG_ASSERT( nStartNode <= nEndNode, "CreateAttribUndo: Start > End ?!" );

    EditUndoSetAttribs* pUndo = nullptr;
    if ( rSet.GetPool() != &aEditDoc.GetItemPool() )
    {
        SfxItemSet aTmpSet( GetEmptyItemSet() );
        aTmpSet.Put( rSet );
        pUndo = new EditUndoSetAttribs(pEditEngine, aESel, aTmpSet);
    }
    else
    {
        pUndo = new EditUndoSetAttribs(pEditEngine, aESel, rSet);
    }

    SfxItemPool* pPool = pUndo->GetNewAttribs().GetPool();

    for ( sal_Int32 nPara = nStartNode; nPara <= nEndNode; nPara++ )
    {
        ContentNode* pNode = aEditDoc.GetObject( nPara );
        DBG_ASSERT( aEditDoc.GetObject( nPara ), "Node not found: CreateAttribUndo" );
        ContentAttribsInfo* pInf = new ContentAttribsInfo( pNode->GetContentAttribs().GetItems() );
        pUndo->AppendContentInfo(pInf);

        for ( sal_Int32 nAttr = 0; nAttr < pNode->GetCharAttribs().Count(); nAttr++ )
        {
            const EditCharAttrib& rAttr = *pNode->GetCharAttribs().GetAttribs()[nAttr].get();
            if (rAttr.GetLen())
            {
                EditCharAttrib* pNew = MakeCharAttrib(*pPool, *rAttr.GetItem(), rAttr.GetStart(), rAttr.GetEnd());
                pInf->AppendCharAttrib(pNew);
            }
        }
    }
    return pUndo;
}

sal_Int32 ImpEditEngine::CreateViewShellId()
{
    sal_Int32 nRet = -1;

    const EditView* pEditView = pEditEngine ? pEditEngine->GetActiveView() : nullptr;
    const OutlinerViewShell* pViewShell = pEditView ? pEditView->GetImpEditView()->GetViewShell() : nullptr;
    if (pViewShell)
        nRet = pViewShell->GetViewShellId();

    return nRet;
}

void ImpEditEngine::UndoActionStart( sal_uInt16 nId, const ESelection& aSel )
{
    if ( IsUndoEnabled() && !IsInUndo() )
    {
        GetUndoManager().EnterListAction( GetEditEnginePtr()->GetUndoComment( nId ), OUString(), nId, CreateViewShellId() );
        DBG_ASSERT( !pUndoMarkSelection, "UndoAction SelectionMarker?" );
        pUndoMarkSelection = new ESelection( aSel );
    }
}

void ImpEditEngine::UndoActionStart( sal_uInt16 nId )
{
    if ( IsUndoEnabled() && !IsInUndo() )
    {
        GetUndoManager().EnterListAction( GetEditEnginePtr()->GetUndoComment( nId ), OUString(), nId, CreateViewShellId() );
        DBG_ASSERT( !pUndoMarkSelection, "UndoAction SelectionMarker?" );
    }
}

void ImpEditEngine::UndoActionEnd( sal_uInt16 )
{
    if ( IsUndoEnabled() && !IsInUndo() )
    {
        GetUndoManager().LeaveListAction();
        delete pUndoMarkSelection;
        pUndoMarkSelection = nullptr;
    }
}

void ImpEditEngine::InsertUndo( EditUndo* pUndo, bool bTryMerge )
{
    DBG_ASSERT( !IsInUndo(), "InsertUndo in Undomodus!" );
    if ( pUndoMarkSelection )
    {
        EditUndoMarkSelection* pU = new EditUndoMarkSelection(pEditEngine, *pUndoMarkSelection);
        GetUndoManager().AddUndoAction( pU );
        delete pUndoMarkSelection;
        pUndoMarkSelection = nullptr;
    }
    GetUndoManager().AddUndoAction( pUndo, bTryMerge );

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

    bUndoEnabled = bEnable;
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

    aSel.Adjust( aEditDoc );

    SfxItemSet aCurSet( GetEmptyItemSet() );

    sal_Int32 nStartNode = aEditDoc.GetPos( aSel.Min().GetNode() );
    sal_Int32 nEndNode = aEditDoc.GetPos( aSel.Max().GetNode() );

    // iterate over the paragraphs ...
    for ( sal_Int32 nNode = nStartNode; nNode <= nEndNode; nNode++ )
    {
        ContentNode* pNode = aEditDoc.GetObject( nNode );
        DBG_ASSERT( aEditDoc.GetObject( nNode ), "Node not found: GetAttrib" );

        const sal_Int32 nStartPos = nNode==nStartNode ? aSel.Min().GetIndex() : 0;
        const sal_Int32 nEndPos = nNode==nEndNode ? aSel.Max().GetIndex() : pNode->Len(); // Can also be == nStart!

        // Problem: Templates ....
        // =>  Other way:
        // 1) Hard character attributes, as usual ...
        // 2) Examine Style and paragraph attributes only when OFF ...

        // First the very hard formatting ...
        EditDoc::FindAttribs( pNode, nStartPos, nEndPos, aCurSet );

        if( nOnlyHardAttrib != EditEngineAttribs_OnlyHard )
        {
            // and then paragraph formatting and template...
            for ( sal_uInt16 nWhich = EE_ITEMS_START; nWhich <= EE_CHAR_END; nWhich++)
            {
                if ( aCurSet.GetItemState( nWhich ) == SfxItemState::DEFAULT )
                {
                    if ( nOnlyHardAttrib == EditEngineAttribs_All )
                    {
                        const SfxPoolItem& rItem = pNode->GetContentAttribs().GetItem( nWhich );
                        aCurSet.Put( rItem );
                    }
                    else if ( pNode->GetContentAttribs().GetItems().GetItemState( nWhich ) == SfxItemState::SET )
                    {
                        const SfxPoolItem& rItem = pNode->GetContentAttribs().GetItems().Get( nWhich );
                        aCurSet.Put( rItem );
                    }
                }
                else if ( aCurSet.GetItemState( nWhich ) == SfxItemState::SET )
                {
                    const SfxPoolItem* pItem = nullptr;
                    if ( nOnlyHardAttrib == EditEngineAttribs_All )
                    {
                        pItem = &pNode->GetContentAttribs().GetItem( nWhich );
                    }
                    else if ( pNode->GetContentAttribs().GetItems().GetItemState( nWhich ) == SfxItemState::SET )
                    {
                        pItem = &pNode->GetContentAttribs().GetItems().Get( nWhich );
                    }
                    // pItem can only be NULL when nOnlyHardAttrib...
                    if ( !pItem || ( *pItem != aCurSet.Get( nWhich ) ) )
                    {
                        // Problem: When Paragraph style with for example font,
                        // but the Font is hard and completely different,
                        // wrong in selection  if invalidated....
                        // => better not invalidate, instead CHANGE!
                        // It would be better to fill each paragraph with
                        // a itemset and compare this in large.
                        if ( nWhich <= EE_PARA_END )
                            aCurSet.InvalidateItem( nWhich );
                    }
                }
            }
        }
    }

    // fill empty slots with defaults ...
    if ( nOnlyHardAttrib == EditEngineAttribs_All )
    {
        for ( sal_uInt16 nWhich = EE_ITEMS_START; nWhich <= EE_CHAR_END; nWhich++ )
        {
            if ( aCurSet.GetItemState( nWhich ) == SfxItemState::DEFAULT )
            {
                aCurSet.Put( aEditDoc.GetItemPool().GetDefaultItem( nWhich ) );
            }
        }
    }
    return aCurSet;
}


SfxItemSet ImpEditEngine::GetAttribs( sal_Int32 nPara, sal_Int32 nStart, sal_Int32 nEnd, GetAttribsFlags nFlags ) const
{
    // Optimized function with less Puts(), which cause unnecessary cloning from default items.
    // If this works, change GetAttribs( EditSelection ) to use this for each paragraph and merge the results!


    ContentNode* pNode = const_cast<ContentNode*>(aEditDoc.GetObject(nPara));
    DBG_ASSERT( pNode, "GetAttribs - unknown paragraph!" );
    DBG_ASSERT( nStart <= nEnd, "getAttribs: Start > End not supported!" );

    SfxItemSet aAttribs( const_cast<ImpEditEngine*>(this)->GetEmptyItemSet() );

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
            const SfxItemPool& rPool = GetEditDoc().GetItemPool();
            pNode->GetCharAttribs().OptimizeRanges(const_cast<SfxItemPool&>(rPool));

            const CharAttribList::AttribsType& rAttrs = pNode->GetCharAttribs().GetAttribs();
            for (const auto & nAttr : rAttrs)
            {
                const EditCharAttrib& rAttr = *nAttr.get();

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
                            // OptimizeRagnge() assures that not the same attr can follow for full coverage
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


void ImpEditEngine::SetAttribs( EditSelection aSel, const SfxItemSet& rSet, sal_uInt8 nSpecial )
{
    aSel.Adjust( aEditDoc );

    // When no selection => use the Attribute on the word.
    // ( the RTF-parser should actually never call the Method without a Range )
    if ( ( nSpecial == ATTRSPECIAL_WHOLEWORD ) && !aSel.HasRange() )
        aSel = SelectWord( aSel, css::i18n::WordType::ANYWORD_IGNOREWHITESPACES, false );

    sal_Int32 nStartNode = aEditDoc.GetPos( aSel.Min().GetNode() );
    sal_Int32 nEndNode = aEditDoc.GetPos( aSel.Max().GetNode() );

    if ( IsUndoEnabled() && !IsInUndo() && aStatus.DoUndoAttribs() )
    {
        EditUndoSetAttribs* pUndo = CreateAttribUndo( aSel, rSet );
        pUndo->SetSpecial( nSpecial );
        InsertUndo( pUndo );
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

        DBG_ASSERT( aEditDoc.GetObject( nNode ), "Node not founden: SetAttribs" );
        DBG_ASSERT( GetParaPortions().SafeGetObject( nNode ), "Portion not found: SetAttribs" );

        ContentNode* pNode = aEditDoc.GetObject( nNode );
        ParaPortion* pPortion = GetParaPortions()[nNode];

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
                }
                else
                {
                    aEditDoc.InsertAttrib( pNode, nStartPos, nEndPos, rItem );
                    bCharAttribFound = true;
                    if ( nSpecial == ATTRSPECIAL_EDGE )
                    {
                        CharAttribList::AttribsType& rAttribs = pNode->GetCharAttribs().GetAttribs();
                        for (std::unique_ptr<EditCharAttrib> & rAttrib : rAttribs)
                        {
                            EditCharAttrib& rAttr = *rAttrib.get();
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
            ParaAttribsChanged( pPortion->GetNode() );
        }
        else if ( bCharAttribFound )
        {
            bFormatted = false;
            if ( !pNode->Len() || ( nStartPos != nEndPos  ) )
            {
                pPortion->MarkSelectionInvalid( nStartPos, nEndPos-nStartPos );
                if ( bCheckLanguage )
                    pNode->GetWrongList()->SetInvalidRange(nStartPos, nEndPos);
            }
        }
    }
}

void ImpEditEngine::RemoveCharAttribs( EditSelection aSel, bool bRemoveParaAttribs, sal_uInt16 nWhich )
{
    aSel.Adjust( aEditDoc );

    sal_Int32 nStartNode = aEditDoc.GetPos( aSel.Min().GetNode() );
    sal_Int32 nEndNode = aEditDoc.GetPos( aSel.Max().GetNode() );

    const SfxItemSet* _pEmptyItemSet = bRemoveParaAttribs ? &GetEmptyItemSet() : nullptr;

    if ( IsUndoEnabled() && !IsInUndo() && aStatus.DoUndoAttribs() )
    {
        // Possibly a special Undo, or itemset*
        EditUndoSetAttribs* pUndo = CreateAttribUndo( aSel, GetEmptyItemSet() );
        pUndo->SetRemoveAttribs( true );
        pUndo->SetRemoveParaAttribs( bRemoveParaAttribs );
        pUndo->SetRemoveWhich( nWhich );
        InsertUndo( pUndo );
    }

    // iterate over the paragraphs ...
    for ( sal_Int32 nNode = nStartNode; nNode <= nEndNode; nNode++ )
    {
        ContentNode* pNode = aEditDoc.GetObject( nNode );
        ParaPortion* pPortion = GetParaPortions()[nNode];

        DBG_ASSERT( aEditDoc.GetObject( nNode ), "Node not found: SetAttribs" );
        DBG_ASSERT( GetParaPortions().SafeGetObject( nNode ), "Portion not found: SetAttribs" );

        const sal_Int32 nStartPos = nNode==nStartNode ? aSel.Min().GetIndex() : 0;
        const sal_Int32 nEndPos = nNode==nEndNode ? aSel.Max().GetIndex() : pNode->Len(); // can also be == nStart!

        // Optimize: If whole paragraph, then RemoveCharAttribs (nPara)?
        bool bChanged = aEditDoc.RemoveAttribs( pNode, nStartPos, nEndPos, nWhich );
        if ( bRemoveParaAttribs )
        {
            SetParaAttribs( nNode, *_pEmptyItemSet );   // Invalidated
        }
        else
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
            bFormatted = false;
            pPortion->MarkSelectionInvalid( nStartPos, nEndPos-nStartPos );
        }
    }
}

void ImpEditEngine::RemoveCharAttribs( sal_Int32 nPara, sal_uInt16 nWhich, bool bRemoveFeatures )
{
    ContentNode* pNode = aEditDoc.GetObject( nPara );
    ParaPortion* pPortion = GetParaPortions().SafeGetObject( nPara );

    DBG_ASSERT( pNode, "Node not found: RemoveCharAttribs" );
    DBG_ASSERT( pPortion, "Portion not found: RemoveCharAttribs" );

    if ( !pNode )
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
            nAttr--;
        }
        nAttr++;
        pAttr = GetAttrib(rAttrs, nAttr);
    }

#if OSL_DEBUG_LEVEL > 0
    CharAttribList::DbgCheckAttribs(pNode->GetCharAttribs());
#endif

    pPortion->MarkSelectionInvalid( 0, pNode->Len() );
}

void ImpEditEngine::SetParaAttribs( sal_Int32 nPara, const SfxItemSet& rSet )
{
    ContentNode* pNode = aEditDoc.GetObject( nPara );

    if ( !pNode )
        return;

    if ( !( pNode->GetContentAttribs().GetItems() == rSet ) )
    {
        if ( IsUndoEnabled() && !IsInUndo() && aStatus.DoUndoAttribs() )
        {
            if ( rSet.GetPool() != &aEditDoc.GetItemPool() )
            {
                SfxItemSet aTmpSet( GetEmptyItemSet() );
                aTmpSet.Put( rSet );
                InsertUndo(new EditUndoSetParaAttribs(pEditEngine, nPara, pNode->GetContentAttribs().GetItems(), aTmpSet));
            }
            else
            {
                InsertUndo(new EditUndoSetParaAttribs(pEditEngine, nPara, pNode->GetContentAttribs().GetItems(), rSet));
            }
        }
        pNode->GetContentAttribs().GetItems().Set( rSet );
        if ( aStatus.UseCharAttribs() )
            pNode->CreateDefFont();

        ParaAttribsChanged( pNode );
    }
}

const SfxItemSet& ImpEditEngine::GetParaAttribs( sal_Int32 nPara ) const
{
    const ContentNode* pNode = aEditDoc.GetObject( nPara );
    assert(pNode && "Node not found: GetParaAttribs");
    return pNode->GetContentAttribs().GetItems();
}

bool ImpEditEngine::HasParaAttrib( sal_Int32 nPara, sal_uInt16 nWhich ) const
{
    const ContentNode* pNode = aEditDoc.GetObject( nPara );
    assert(pNode && "Node not found: HasParaAttrib");
    return pNode->GetContentAttribs().HasItem( nWhich );
}

const SfxPoolItem& ImpEditEngine::GetParaAttrib( sal_Int32 nPara, sal_uInt16 nWhich ) const
{
    const ContentNode* pNode = aEditDoc.GetObject(nPara);
    assert(pNode && "Node not found: GetParaAttrib");
    return pNode->GetContentAttribs().GetItem(nWhich);
}

void ImpEditEngine::GetCharAttribs( sal_Int32 nPara, std::vector<EECharAttrib>& rLst ) const
{
    rLst.clear();
    const ContentNode* pNode = aEditDoc.GetObject( nPara );
    if ( pNode )
    {
        rLst.reserve(pNode->GetCharAttribs().Count());
        const CharAttribList::AttribsType& rAttrs = pNode->GetCharAttribs().GetAttribs();
        for (const auto & i : rAttrs)
        {
            const EditCharAttrib& rAttr = *i.get();
            EECharAttrib aEEAttr;
            aEEAttr.pAttr = rAttr.GetItem();
            aEEAttr.nPara = nPara;
            aEEAttr.nStart = rAttr.GetStart();
            aEEAttr.nEnd = rAttr.GetEnd();
            rLst.push_back(aEEAttr);
        }
    }
}

void ImpEditEngine::ParaAttribsToCharAttribs( ContentNode* pNode )
{
    pNode->GetCharAttribs().DeleteEmptyAttribs( GetEditDoc().GetItemPool() );
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
                    aEditDoc.InsertAttrib( pNode, nLastEnd, pAttr->GetStart(), rItem );
                // #112831# Last Attr might go from 0xffff to 0x0000
                pAttr = nLastEnd ? pNode->GetCharAttribs().FindNextAttrib( nWhich, nLastEnd ) : nullptr;
            }

            // And the Rest:
            if ( nLastEnd < nEndPos )
                aEditDoc.InsertAttrib( pNode, nLastEnd, nEndPos, rItem );
        }
    }
    bFormatted = false;
    // Portion does not need to be invalidated here, happens elsewhere.
}

IdleFormattter::IdleFormattter()
{
    pView = nullptr;
    nRestarts = 0;
}

IdleFormattter::~IdleFormattter()
{
    pView = nullptr;
}

void IdleFormattter::DoIdleFormat( EditView* pV )
{
    pView = pV;

    if ( IsActive() )
        nRestarts++;

    if ( nRestarts > 4 )
        ForceTimeout();
    else
        Start();
}

void IdleFormattter::ForceTimeout()
{
    if ( IsActive() )
    {
        Stop();
        Invoke();
    }
}

ImplIMEInfos::ImplIMEInfos( const EditPaM& rPos, const OUString& rOldTextAfterStartPos )
 : aOldTextAfterStartPos( rOldTextAfterStartPos )
{
    aPos = rPos;
    nLen = 0;
    bCursor = true;
    pAttribs = nullptr;
    bWasCursorOverwrite = false;
}

ImplIMEInfos::~ImplIMEInfos()
{
    delete[] pAttribs;
}

void ImplIMEInfos::CopyAttribs( const ExtTextInputAttr* pA, sal_uInt16 nL )
{
    nLen = nL;
    delete[] pAttribs;
    pAttribs = new ExtTextInputAttr[ nL ];
    memcpy( pAttribs, pA, nL*sizeof(ExtTextInputAttr) );
}

void ImplIMEInfos::DestroyAttribs()
{
    delete[] pAttribs;
    pAttribs = nullptr;
    nLen = 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
