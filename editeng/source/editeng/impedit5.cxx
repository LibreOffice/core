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
#include <svl/smplhint.hxx>
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
    return pNode ? pNode->GetContentAttribs().GetStyleSheet() : NULL;
}

SfxStyleSheet* ImpEditEngine::GetStyleSheet( sal_Int32 nPara )
{
    ContentNode* pNode = aEditDoc.GetObject( nPara );
    return pNode ? pNode->GetContentAttribs().GetStyleSheet() : NULL;
}

void ImpEditEngine::SetStyleSheet( EditSelection aSel, SfxStyleSheet* pStyle )
{
    aSel.Adjust( aEditDoc );

    sal_Int32 nStartPara = aEditDoc.GetPos( aSel.Min().GetNode() );
    sal_Int32 nEndPara = aEditDoc.GetPos( aSel.Max().GetNode() );

    sal_Bool _bUpdate = GetUpdateMode();
    SetUpdateMode( sal_False );

    for ( sal_Int32 n = nStartPara; n <= nEndPara; n++ )
        SetStyleSheet( n, pStyle );

    SetUpdateMode( _bUpdate, 0 );
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
            XubString aPrevStyleName;
            if ( pCurStyle )
                aPrevStyleName = pCurStyle->GetName();

            XubString aNewStyleName;
            if ( pStyle )
                aNewStyleName = pStyle->GetName();

            InsertUndo(
                new EditUndoSetStyleSheet(pEditEngine, aEditDoc.GetPos( pNode ),
                        aPrevStyleName, pCurStyle ? pCurStyle->GetFamily() : SFX_STYLE_FAMILY_PARA,
                        aNewStyleName, pStyle ? pStyle->GetFamily() : SFX_STYLE_FAMILY_PARA,
                        pNode->GetContentAttribs().GetItems() ) );
        }
        if ( pCurStyle )
            EndListening( *pCurStyle, sal_False );
        pNode->SetStyleSheet( pStyle, aStatus.UseCharAttribs() );
        if ( pStyle )
            StartListening( *pStyle, sal_False );
        ParaAttribsChanged( pNode );
    }
    FormatAndUpdate();
}

void ImpEditEngine::UpdateParagraphsWithStyleSheet( SfxStyleSheet* pStyle )
{
    SvxFont aFontFromStyle;
    CreateFont( aFontFromStyle, pStyle->GetItemSet() );

    sal_Bool bUsed = sal_False;
    for ( sal_Int32 nNode = 0; nNode < aEditDoc.Count(); nNode++ )
    {
        ContentNode* pNode = aEditDoc.GetObject( nNode );
        if ( pNode->GetStyleSheet() == pStyle )
        {
            bUsed = sal_True;
            if ( aStatus.UseCharAttribs() )
                pNode->SetStyleSheet( pStyle, aFontFromStyle );
            else
                pNode->SetStyleSheet( pStyle, sal_False );

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
            pNode->SetStyleSheet( NULL );
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
        DBG_CHKOBJ( GetEditEnginePtr(), EditEngine, 0 );

        SfxStyleSheet* pStyle = NULL;
        sal_uLong nId = 0;

        if ( rHint.ISA( SfxStyleSheetHint ) )
        {
            const SfxStyleSheetHint& rH = (const SfxStyleSheetHint&) rHint;
            DBG_ASSERT( rH.GetStyleSheet()->ISA( SfxStyleSheet ), "No SfxStyleSheet!" );
            pStyle = (SfxStyleSheet*) rH.GetStyleSheet();
            nId = rH.GetHint();
        }
        else if ( ( rHint.Type() == TYPE(SfxSimpleHint ) ) && ( rBC.ISA( SfxStyleSheet ) ) )
        {
            pStyle = (SfxStyleSheet*)&rBC;
            nId = ((SfxSimpleHint&)rHint).GetId();
        }

        if ( pStyle )
        {
            if ( ( nId == SFX_HINT_DYING ) ||
                 ( nId == SFX_STYLESHEET_INDESTRUCTION ) ||
                 ( nId == SFX_STYLESHEET_ERASED ) )
            {
                RemoveStyleFromParagraphs( pStyle );
            }
            else if ( ( nId == SFX_HINT_DATACHANGED ) ||
                      ( nId == SFX_STYLESHEET_MODIFIED ) )
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

    EditUndoSetAttribs* pUndo = NULL;
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

        for ( sal_uInt16 nAttr = 0; nAttr < pNode->GetCharAttribs().Count(); nAttr++ )
        {
            const EditCharAttrib& rAttr = pNode->GetCharAttribs().GetAttribs()[nAttr];
            if (rAttr.GetLen())
            {
                EditCharAttrib* pNew = MakeCharAttrib(*pPool, *rAttr.GetItem(), rAttr.GetStart(), rAttr.GetEnd());
                pInf->AppendCharAttrib(pNew);
            }
        }
    }
    return pUndo;
}

void ImpEditEngine::UndoActionStart( sal_uInt16 nId, const ESelection& aSel )
{
    if ( IsUndoEnabled() && !IsInUndo() )
    {
        GetUndoManager().EnterListAction( GetEditEnginePtr()->GetUndoComment( nId ), OUString(), nId );
        DBG_ASSERT( !pUndoMarkSelection, "UndoAction SelectionMarker?" );
        pUndoMarkSelection = new ESelection( aSel );
    }
}

void ImpEditEngine::UndoActionStart( sal_uInt16 nId )
{
    if ( IsUndoEnabled() && !IsInUndo() )
    {
        GetUndoManager().EnterListAction( GetEditEnginePtr()->GetUndoComment( nId ), OUString(), nId );
        DBG_ASSERT( !pUndoMarkSelection, "UndoAction SelectionMarker?" );
    }
}

void ImpEditEngine::UndoActionEnd( sal_uInt16 )
{
    if ( IsUndoEnabled() && !IsInUndo() )
    {
        GetUndoManager().LeaveListAction();
        delete pUndoMarkSelection;
        pUndoMarkSelection = NULL;
    }
}

void ImpEditEngine::InsertUndo( EditUndo* pUndo, bool bTryMerge )
{
    DBG_ASSERT( !IsInUndo(), "InsertUndo in Undomodus!" );
    if ( pUndoMarkSelection )
    {
        EditUndoMarkSelection* pU = new EditUndoMarkSelection(pEditEngine, *pUndoMarkSelection);
        GetUndoManager().AddUndoAction( pU, false );
        delete pUndoMarkSelection;
        pUndoMarkSelection = NULL;
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

sal_Bool ImpEditEngine::Undo( EditView* pView )
{
    if ( HasUndoManager() && GetUndoManager().GetUndoActionCount() )
    {
        SetActiveView( pView );
        GetUndoManager().Undo();
        return sal_True;
    }
    return sal_False;
}

sal_Bool ImpEditEngine::Redo( EditView* pView )
{
    if ( HasUndoManager() && GetUndoManager().GetRedoActionCount() )
    {
        SetActiveView( pView );
        GetUndoManager().Redo();
        return sal_True;
    }
    return sal_False;
}

SfxItemSet ImpEditEngine::GetAttribs( EditSelection aSel, sal_Bool bOnlyHardAttrib )
{
    DBG_CHKOBJ( GetEditEnginePtr(), EditEngine, 0 );

    aSel.Adjust( aEditDoc );

    SfxItemSet aCurSet( GetEmptyItemSet() );

    sal_Int32 nStartNode = aEditDoc.GetPos( aSel.Min().GetNode() );
    sal_Int32 nEndNode = aEditDoc.GetPos( aSel.Max().GetNode() );

    // iterate over the paragraphs ...
    for ( sal_Int32 nNode = nStartNode; nNode <= nEndNode; nNode++ )
    {
        ContentNode* pNode = aEditDoc.GetObject( nNode );
        DBG_ASSERT( aEditDoc.GetObject( nNode ), "Node not found: GetAttrib" );

        xub_StrLen nStartPos = 0;
        xub_StrLen nEndPos = pNode->Len();
        if ( nNode == nStartNode )
            nStartPos = aSel.Min().GetIndex();
        if ( nNode == nEndNode ) // Can also be == nStart!
            nEndPos = aSel.Max().GetIndex();

        // Problem: Templates ....
        // =>  Other way:
        // 1) Hard character attributes, as usual ...
        // 2) Examine Style and paragraph attributes only when OFF ...

        // First the very hard formatting ...
        aEditDoc.FindAttribs( pNode, nStartPos, nEndPos, aCurSet );

        if( bOnlyHardAttrib != EditEngineAttribs_OnlyHard )
        {
            // and then paragraph formatting and template...
            for ( sal_uInt16 nWhich = EE_ITEMS_START; nWhich <= EE_CHAR_END; nWhich++)
            {
                if ( aCurSet.GetItemState( nWhich ) == SFX_ITEM_OFF )
                {
                    if ( bOnlyHardAttrib == EditEngineAttribs_All )
                    {
                        const SfxPoolItem& rItem = pNode->GetContentAttribs().GetItem( nWhich );
                        aCurSet.Put( rItem );
                    }
                    else if ( pNode->GetContentAttribs().GetItems().GetItemState( nWhich ) == SFX_ITEM_ON )
                    {
                        const SfxPoolItem& rItem = pNode->GetContentAttribs().GetItems().Get( nWhich );
                        aCurSet.Put( rItem );
                    }
                }
                else if ( aCurSet.GetItemState( nWhich ) == SFX_ITEM_ON )
                {
                    const SfxPoolItem* pItem = NULL;
                    if ( bOnlyHardAttrib == EditEngineAttribs_All )
                    {
                        pItem = &pNode->GetContentAttribs().GetItem( nWhich );
                    }
                    else if ( pNode->GetContentAttribs().GetItems().GetItemState( nWhich ) == SFX_ITEM_ON )
                    {
                        pItem = &pNode->GetContentAttribs().GetItems().Get( nWhich );
                    }
                    // pItem can only be NULL when bOnlyHardAttrib...
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
    if ( bOnlyHardAttrib == EditEngineAttribs_All )
    {
        for ( sal_uInt16 nWhich = EE_ITEMS_START; nWhich <= EE_CHAR_END; nWhich++ )
        {
            if ( aCurSet.GetItemState( nWhich ) == SFX_ITEM_OFF )
            {
                aCurSet.Put( aEditDoc.GetItemPool().GetDefaultItem( nWhich ) );
            }
        }
    }
    return aCurSet;
}


SfxItemSet ImpEditEngine::GetAttribs( sal_Int32 nPara, sal_uInt16 nStart, sal_uInt16 nEnd, sal_uInt8 nFlags ) const
{
    // Optimized function with less Puts(), which cause unnecessary cloning from default items.
    // If this works, change GetAttribs( EditSelection ) to use this for each paragraph and merge the results!

    DBG_CHKOBJ( GetEditEnginePtr(), EditEngine, 0 );

    ContentNode* pNode = const_cast<ContentNode*>(aEditDoc.GetObject(nPara));
    DBG_ASSERT( pNode, "GetAttribs - unknown paragraph!" );
    DBG_ASSERT( nStart <= nEnd, "getAttribs: Start > End not supported!" );

    SfxItemSet aAttribs( ((ImpEditEngine*)this)->GetEmptyItemSet() );

    if ( pNode )
    {
        if ( nEnd > pNode->Len() )
            nEnd = pNode->Len();

        if ( nStart > nEnd )
            nStart = nEnd;

        // StyleSheet / Parattribs...

        if ( pNode->GetStyleSheet() && ( nFlags & GETATTRIBS_STYLESHEET ) )
            aAttribs.Set(pNode->GetStyleSheet()->GetItemSet(), true);

        if ( nFlags & GETATTRIBS_PARAATTRIBS )
            aAttribs.Put( pNode->GetContentAttribs().GetItems() );

        // CharAttribs...

        if ( nFlags & GETATTRIBS_CHARATTRIBS )
        {
            // Make testing easier...
            const SfxItemPool& rPool = GetEditDoc().GetItemPool();
            pNode->GetCharAttribs().OptimizeRanges(const_cast<SfxItemPool&>(rPool));

            const CharAttribList::AttribsType& rAttrs = pNode->GetCharAttribs().GetAttribs();
            for (size_t nAttr = 0; nAttr < rAttrs.size(); ++nAttr)
            {
                const EditCharAttrib& rAttr = rAttrs[nAttr];

                if ( nStart == nEnd )
                {
                    sal_uInt16 nCursorPos = nStart;
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
                            // only partial, check with current, when using para/styhe, otherwise invalid.
                            if ( !( nFlags & (GETATTRIBS_PARAATTRIBS|GETATTRIBS_STYLESHEET) ) ||
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
        aSel = SelectWord( aSel, ::com::sun::star::i18n::WordType::ANYWORD_IGNOREWHITESPACES, sal_False );

    sal_Int32 nStartNode = aEditDoc.GetPos( aSel.Min().GetNode() );
    sal_Int32 nEndNode = aEditDoc.GetPos( aSel.Max().GetNode() );

    if ( IsUndoEnabled() && !IsInUndo() && aStatus.DoUndoAttribs() )
    {
        EditUndoSetAttribs* pUndo = CreateAttribUndo( aSel, rSet );
        pUndo->SetSpecial( nSpecial );
        InsertUndo( pUndo );
    }

    sal_Bool bCheckLanguage = sal_False;
    if ( GetStatus().DoOnlineSpelling() )
    {
        bCheckLanguage = ( rSet.GetItemState( EE_CHAR_LANGUAGE ) == SFX_ITEM_ON ) ||
                         ( rSet.GetItemState( EE_CHAR_LANGUAGE_CJK ) == SFX_ITEM_ON ) ||
                         ( rSet.GetItemState( EE_CHAR_LANGUAGE_CTL ) == SFX_ITEM_ON );
    }

    // iterate over the paragraphs ...
    for ( sal_Int32 nNode = nStartNode; nNode <= nEndNode; nNode++ )
    {
        sal_Bool bParaAttribFound = sal_False;
        sal_Bool bCharAttribFound = sal_False;

        DBG_ASSERT( aEditDoc.GetObject( nNode ), "Node not founden: SetAttribs" );
        DBG_ASSERT( GetParaPortions().SafeGetObject( nNode ), "Portion not found: SetAttribs" );

        ContentNode* pNode = aEditDoc.GetObject( nNode );
        ParaPortion* pPortion = GetParaPortions()[nNode];

        xub_StrLen nStartPos = 0;
        xub_StrLen nEndPos = pNode->Len();
        if ( nNode == nStartNode )
            nStartPos = aSel.Min().GetIndex();
        if ( nNode == nEndNode ) // can also be == nStart!
            nEndPos = aSel.Max().GetIndex();

        // Iterate over the Items...
        for ( sal_uInt16 nWhich = EE_ITEMS_START; nWhich <= EE_CHAR_END; nWhich++)
        {
            if ( rSet.GetItemState( nWhich ) == SFX_ITEM_ON )
            {
                const SfxPoolItem& rItem = rSet.Get( nWhich );
                if ( nWhich <= EE_PARA_END )
                {
                    pNode->GetContentAttribs().GetItems().Put( rItem );
                    bParaAttribFound = sal_True;
                }
                else
                {
                    aEditDoc.InsertAttrib( pNode, nStartPos, nEndPos, rItem );
                    bCharAttribFound = sal_True;
                    if ( nSpecial == ATTRSPECIAL_EDGE )
                    {
                        CharAttribList::AttribsType& rAttribs = pNode->GetCharAttribs().GetAttribs();
                        for (size_t i = 0, n = rAttribs.size(); i < n; ++i)
                        {
                            EditCharAttrib& rAttr = rAttribs[i];
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
                    pNode->GetWrongList()->MarkInvalid( nStartPos, nEndPos );
            }
        }
    }
}

void ImpEditEngine::RemoveCharAttribs( EditSelection aSel, sal_Bool bRemoveParaAttribs, sal_uInt16 nWhich )
{
    aSel.Adjust( aEditDoc );

    sal_Int32 nStartNode = aEditDoc.GetPos( aSel.Min().GetNode() );
    sal_Int32 nEndNode = aEditDoc.GetPos( aSel.Max().GetNode() );

    const SfxItemSet* _pEmptyItemSet = bRemoveParaAttribs ? &GetEmptyItemSet() : 0;

    if ( IsUndoEnabled() && !IsInUndo() && aStatus.DoUndoAttribs() )
    {
        // Possibly a special Undo, or itemset*
        EditUndoSetAttribs* pUndo = CreateAttribUndo( aSel, GetEmptyItemSet() );
        pUndo->SetRemoveAttribs( sal_True );
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

        xub_StrLen nStartPos = 0;
        xub_StrLen nEndPos = pNode->Len();
        if ( nNode == nStartNode )
            nStartPos = aSel.Min().GetIndex();
        if ( nNode == nEndNode ) // can also be == nStart!
            nEndPos = aSel.Max().GetIndex();

        // Optimize: If whole paragraph, then RemoveCharAttribs (nPara)?
        sal_Bool bChanged = aEditDoc.RemoveAttribs( pNode, nStartPos, nEndPos, nWhich );
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

void ImpEditEngine::RemoveCharAttribs( sal_Int32 nPara, sal_uInt16 nWhich, sal_Bool bRemoveFeatures )
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
    DBG_ASSERT( pNode, "Node not found: GetParaAttribs" );
    return pNode->GetContentAttribs().GetItems();
}

bool ImpEditEngine::HasParaAttrib( sal_Int32 nPara, sal_uInt16 nWhich ) const
{
    const ContentNode* pNode = aEditDoc.GetObject( nPara );
    DBG_ASSERT( pNode, "Node not found: HasParaAttrib" );

    return pNode->GetContentAttribs().HasItem( nWhich );
}

const SfxPoolItem& ImpEditEngine::GetParaAttrib( sal_Int32 nPara, sal_uInt16 nWhich ) const
{
    const ContentNode* pNode = aEditDoc.GetObject( nPara );
    DBG_ASSERT( pNode, "Node not found: GetParaAttrib" );

    return pNode->GetContentAttribs().GetItem( nWhich );
}

void ImpEditEngine::GetCharAttribs( sal_Int32 nPara, std::vector<EECharAttrib>& rLst ) const
{
    rLst.clear();
    const ContentNode* pNode = aEditDoc.GetObject( nPara );
    if ( pNode )
    {
        rLst.reserve(pNode->GetCharAttribs().Count());
        const CharAttribList::AttribsType& rAttrs = pNode->GetCharAttribs().GetAttribs();
        for (size_t i = 0; i < rAttrs.size(); ++i)
        {
            const EditCharAttrib& rAttr = rAttrs[i];
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
    xub_StrLen nEndPos = pNode->Len();
    for ( sal_uInt16 nWhich = EE_CHAR_START; nWhich <= EE_CHAR_END; nWhich++ )
    {
        if ( pNode->GetContentAttribs().HasItem( nWhich ) )
        {
            const SfxPoolItem& rItem = pNode->GetContentAttribs().GetItem( nWhich );
            // Fill the gap:
            sal_uInt16 nLastEnd = 0;
            const EditCharAttrib* pAttr = pNode->GetCharAttribs().FindNextAttrib( nWhich, nLastEnd );
            while ( pAttr )
            {
                nLastEnd = pAttr->GetEnd();
                if ( pAttr->GetStart() > nLastEnd )
                    aEditDoc.InsertAttrib( pNode, nLastEnd, pAttr->GetStart(), rItem );
                // #112831# Last Attr might go from 0xffff to 0x0000
                pAttr = nLastEnd ? pNode->GetCharAttribs().FindNextAttrib( nWhich, nLastEnd ) : NULL;
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
    pView = 0;
    nRestarts = 0;
}

IdleFormattter::~IdleFormattter()
{
    pView = 0;
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
        ((Link&)GetTimeoutHdl()).Call( this );
    }
}

ImplIMEInfos::ImplIMEInfos( const EditPaM& rPos, const String& rOldTextAfterStartPos )
 : aOldTextAfterStartPos( rOldTextAfterStartPos )
{
    aPos = rPos;
    nLen = 0;
    bCursor = sal_True;
    pAttribs = NULL;
    bWasCursorOverwrite = sal_False;
}

ImplIMEInfos::~ImplIMEInfos()
{
    delete[] pAttribs;
}

void ImplIMEInfos::CopyAttribs( const sal_uInt16* pA, sal_uInt16 nL )
{
    nLen = nL;
    delete[] pAttribs;
    pAttribs = new sal_uInt16[ nL ];
    memcpy( pAttribs, pA, nL*sizeof(sal_uInt16) );
}

void ImplIMEInfos::DestroyAttribs()
{
    delete[] pAttribs;
    pAttribs = NULL;
    nLen = 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
