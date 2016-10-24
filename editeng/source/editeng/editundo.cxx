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
#include <editundo.hxx>
#include <editeng/editview.hxx>
#include <editeng/editeng.hxx>
#include <editeng/outliner.hxx>


static void lcl_DoSetSelection( EditView* pView, sal_uInt16 nPara )
{
    EPaM aEPaM( nPara, 0 );
    EditPaM aPaM( pView->GetImpEditEngine()->CreateEditPaM( aEPaM ) );
    aPaM.SetIndex( aPaM.GetNode()->Len() );
    EditSelection aSel( aPaM, aPaM );
    pView->GetImpEditView()->SetEditSelection( aSel );
}

EditUndoManager::EditUndoManager(sal_uInt16 nMaxUndoActionCount )
:   SfxUndoManager(nMaxUndoActionCount),
    mpEditEngine(nullptr)
{
}

void EditUndoManager::SetEditEngine(EditEngine* pNew)
{
    mpEditEngine = pNew;
}

bool EditUndoManager::Undo()
{
    if ( !mpEditEngine || GetUndoActionCount() == 0 )
        return false;

    DBG_ASSERT( mpEditEngine->GetActiveView(), "Active View?" );

    if ( !mpEditEngine->GetActiveView() )
    {
        if (!mpEditEngine->GetEditViews().empty())
            mpEditEngine->SetActiveView(mpEditEngine->GetEditViews()[0]);
        else
        {
            OSL_FAIL("Undo in engine is not possible without a View! ");
            return false;
        }
    }

    mpEditEngine->GetActiveView()->GetImpEditView()->DrawSelection(); // Remove the old selection

    mpEditEngine->SetUndoMode( true );
    bool bDone = SfxUndoManager::Undo();
    mpEditEngine->SetUndoMode( false );

    EditSelection aNewSel( mpEditEngine->GetActiveView()->GetImpEditView()->GetEditSelection() );
    DBG_ASSERT( !aNewSel.IsInvalid(), "Invalid selection after Undo () ");
    DBG_ASSERT( !aNewSel.DbgIsBuggy( mpEditEngine->GetEditDoc() ), "Broken selection afte Undo () ");

    aNewSel.Min() = aNewSel.Max();
    mpEditEngine->GetActiveView()->GetImpEditView()->SetEditSelection( aNewSel );
    mpEditEngine->FormatAndUpdate( mpEditEngine->GetActiveView(), true );

    return bDone;
}

bool EditUndoManager::Redo()
{
    if ( !mpEditEngine || GetRedoActionCount() == 0 )
        return false;

    DBG_ASSERT( mpEditEngine->GetActiveView(), "Active View?" );

    if ( !mpEditEngine->GetActiveView() )
    {
        if (!mpEditEngine->GetEditViews().empty())
            mpEditEngine->SetActiveView(mpEditEngine->GetEditViews()[0]);
        else
        {
            OSL_FAIL( "Redo in Engine ohne View nicht moeglich!" );
            return false;
        }
    }

    mpEditEngine->GetActiveView()->GetImpEditView()->DrawSelection(); // Remove the old selection

    mpEditEngine->SetUndoMode( true );
    bool bDone = SfxUndoManager::Redo();
    mpEditEngine->SetUndoMode( false );

    EditSelection aNewSel( mpEditEngine->GetActiveView()->GetImpEditView()->GetEditSelection() );
    DBG_ASSERT( !aNewSel.IsInvalid(), "Invalid selection after Undo () ");
    DBG_ASSERT( !aNewSel.DbgIsBuggy( mpEditEngine->GetEditDoc() ), "Broken selection afte Undo () ");

    aNewSel.Min() = aNewSel.Max();
    mpEditEngine->GetActiveView()->GetImpEditView()->SetEditSelection( aNewSel );
    mpEditEngine->FormatAndUpdate( mpEditEngine->GetActiveView() );

    return bDone;
}

EditUndo::EditUndo(sal_uInt16 nI, EditEngine* pEE) :
    nId(nI), mnViewShellId(-1), mpEditEngine(pEE)
{
    const EditView* pEditView = mpEditEngine ? mpEditEngine->GetActiveView() : nullptr;
    const OutlinerViewShell* pViewShell = pEditView ? pEditView->GetImpEditView()->GetViewShell() : nullptr;
    if (pViewShell)
        mnViewShellId = pViewShell->GetViewShellId();
}

EditUndo::~EditUndo()
{
}


sal_uInt16 EditUndo::GetId() const
{
    return nId;
}

bool EditUndo::CanRepeat(SfxRepeatTarget&) const
{
    return false;
}

OUString EditUndo::GetComment() const
{
    OUString aComment;

    if (mpEditEngine)
        aComment = mpEditEngine->GetUndoComment( GetId() );

    return aComment;
}

sal_Int32 EditUndo::GetViewShellId() const
{
    return mnViewShellId;
}

EditUndoDelContent::EditUndoDelContent(
    EditEngine* pEE, ContentNode* pNode, sal_Int32 nPortion) :
    EditUndo(EDITUNDO_DELCONTENT, pEE),
    bDelObject(true),
    nNode(nPortion),
    pContentNode(pNode) {}

EditUndoDelContent::~EditUndoDelContent()
{
    if ( bDelObject )
        delete pContentNode;
}

void EditUndoDelContent::Undo()
{
    DBG_ASSERT( GetEditEngine()->GetActiveView(), "Undo/Redo: No Active View!" );
    GetEditEngine()->InsertContent( pContentNode, nNode );
    bDelObject = false; // belongs to the Engine again
    EditSelection aSel( EditPaM( pContentNode, 0 ), EditPaM( pContentNode, pContentNode->Len() ) );
    GetEditEngine()->GetActiveView()->GetImpEditView()->SetEditSelection(aSel);
}

void EditUndoDelContent::Redo()
{
    DBG_ASSERT( GetEditEngine()->GetActiveView(), "Undo/Redo: No Active View!" );

    EditEngine* pEE = GetEditEngine();

    // pNode is no longer correct, if the paragraphs where merged
    // in between Undos
    pContentNode = pEE->GetEditDoc().GetObject( nNode );
    DBG_ASSERT( pContentNode, "EditUndoDelContent::Redo(): Node?!" );

    pEE->RemoveParaPortion(nNode);

    // Do not delete node, depends on the undo!
    pEE->GetEditDoc().Release( nNode );
    if (pEE->IsCallParaInsertedOrDeleted())
        pEE->ParagraphDeleted( nNode );

    DeletedNodeInfo* pInf = new DeletedNodeInfo( pContentNode, nNode );
    pEE->AppendDeletedNodeInfo(pInf);
    pEE->UpdateSelections();

    ContentNode* pN = ( nNode < pEE->GetEditDoc().Count() )
        ? pEE->GetEditDoc().GetObject( nNode )
        : pEE->GetEditDoc().GetObject( nNode-1 );
    DBG_ASSERT( pN && ( pN != pContentNode ), "?! RemoveContent !? " );
    EditPaM aPaM( pN, pN->Len() );

    bDelObject = true;  // belongs to the Engine again

    pEE->GetActiveView()->GetImpEditView()->SetEditSelection( EditSelection( aPaM, aPaM ) );
}

EditUndoConnectParas::EditUndoConnectParas(
    EditEngine* pEE, sal_Int32 nN, sal_uInt16 nSP,
    const SfxItemSet& rLeftParaAttribs, const SfxItemSet& rRightParaAttribs,
    const SfxStyleSheet* pLeftStyle, const SfxStyleSheet* pRightStyle, bool bBkwrd) :
    EditUndo(EDITUNDO_CONNECTPARAS, pEE),
    aLeftParaAttribs(rLeftParaAttribs),
    aRightParaAttribs(rRightParaAttribs),
    eLeftStyleFamily(SFX_STYLE_FAMILY_ALL),
    eRightStyleFamily(SFX_STYLE_FAMILY_ALL),
    bBackward(bBkwrd)
{
    nNode   = nN;
    nSepPos = nSP;

    if ( pLeftStyle )
    {
        aLeftStyleName = pLeftStyle->GetName();
        eLeftStyleFamily = pLeftStyle->GetFamily();
    }
    if ( pRightStyle )
    {
        aRightStyleName = pRightStyle->GetName();
        eRightStyleFamily = pRightStyle->GetFamily();
    }
}

EditUndoConnectParas::~EditUndoConnectParas()
{
}

void EditUndoConnectParas::Undo()
{
    DBG_ASSERT( GetEditEngine()->GetActiveView(), "Undo/Redo: No Active View!" );

    // For SplitContent ParagraphInserted can not be called yet because the
    // Outliner relies on the attributes to initialize the depth

    bool bCall = GetEditEngine()->IsCallParaInsertedOrDeleted();
    GetEditEngine()->SetCallParaInsertedOrDeleted(false);

    EditPaM aPaM = GetEditEngine()->SplitContent(nNode, nSepPos);
    GetEditEngine()->SetParaAttribs( nNode, aLeftParaAttribs );
    GetEditEngine()->SetParaAttribs( nNode+1, aRightParaAttribs );

    GetEditEngine()->SetCallParaInsertedOrDeleted( bCall );
    if (GetEditEngine()->IsCallParaInsertedOrDeleted())
        GetEditEngine()->ParagraphInserted( nNode+1 );

    if (GetEditEngine()->GetStyleSheetPool())
    {
        if ( !aLeftStyleName.isEmpty() )
            GetEditEngine()->SetStyleSheet( nNode, static_cast<SfxStyleSheet*>(GetEditEngine()->GetStyleSheetPool()->Find( aLeftStyleName, eLeftStyleFamily )) );
        if ( !aRightStyleName.isEmpty() )
            GetEditEngine()->SetStyleSheet( nNode+1, static_cast<SfxStyleSheet*>(GetEditEngine()->GetStyleSheetPool()->Find( aRightStyleName, eRightStyleFamily )) );
    }

    GetEditEngine()->GetActiveView()->GetImpEditView()->SetEditSelection( EditSelection( aPaM, aPaM ) );
}

void EditUndoConnectParas::Redo()
{
    DBG_ASSERT( GetEditEngine()->GetActiveView(), "Undo/Redo: Np Active View!" );
    EditPaM aPaM = GetEditEngine()->ConnectContents( nNode, bBackward );

    GetEditEngine()->GetActiveView()->GetImpEditView()->SetEditSelection( EditSelection( aPaM, aPaM ) );
}

EditUndoSplitPara::EditUndoSplitPara(
    EditEngine* pEE, sal_Int32 nN, sal_uInt16 nSP) :
    EditUndo(EDITUNDO_SPLITPARA, pEE),
    nNode(nN), nSepPos(nSP) {}

EditUndoSplitPara::~EditUndoSplitPara() {}

void EditUndoSplitPara::Undo()
{
    DBG_ASSERT( GetEditEngine()->GetActiveView(), "Undo/Redo: No Active View!" );
    EditPaM aPaM = GetEditEngine()->ConnectContents(nNode, false);
    GetEditEngine()->GetActiveView()->GetImpEditView()->SetEditSelection( EditSelection( aPaM, aPaM ) );
}

void EditUndoSplitPara::Redo()
{
    DBG_ASSERT( GetEditEngine()->GetActiveView(), "Undo/Redo: No Active View!" );
    EditPaM aPaM = GetEditEngine()->SplitContent(nNode, nSepPos);
    GetEditEngine()->GetActiveView()->GetImpEditView()->SetEditSelection( EditSelection( aPaM, aPaM ) );
}

EditUndoInsertChars::EditUndoInsertChars(
    EditEngine* pEE, const EPaM& rEPaM, const OUString& rStr) :
    EditUndo(EDITUNDO_INSERTCHARS, pEE),
    aEPaM(rEPaM),
    aText(rStr) {}

void EditUndoInsertChars::Undo()
{
    DBG_ASSERT( GetEditEngine()->GetActiveView(), "Undo/Redo: No Active View!" );
    EditPaM aPaM = GetEditEngine()->CreateEditPaM(aEPaM);
    EditSelection aSel( aPaM, aPaM );
    aSel.Max().SetIndex( aSel.Max().GetIndex() + aText.getLength() );
    EditPaM aNewPaM( GetEditEngine()->DeleteSelection(aSel) );
    GetEditEngine()->GetActiveView()->GetImpEditView()->SetEditSelection( EditSelection( aNewPaM, aNewPaM ) );
}

void EditUndoInsertChars::Redo()
{
    DBG_ASSERT( GetEditEngine()->GetActiveView(), "Undo/Redo: Keine Active View!" );
    EditPaM aPaM = GetEditEngine()->CreateEditPaM(aEPaM);
    GetEditEngine()->InsertText(EditSelection(aPaM, aPaM), aText);
    EditPaM aNewPaM( aPaM );
    aNewPaM.SetIndex( aNewPaM.GetIndex() + aText.getLength() );
    GetEditEngine()->GetActiveView()->GetImpEditView()->SetEditSelection( EditSelection( aPaM, aNewPaM ) );
}

bool EditUndoInsertChars::Merge( SfxUndoAction* pNextAction )
{
    EditUndoInsertChars* pNext = dynamic_cast<EditUndoInsertChars*>(pNextAction);
    if (!pNext)
        return false;

    if ( aEPaM.nPara != pNext->aEPaM.nPara )
        return false;

    if ( ( aEPaM.nIndex + aText.getLength() ) == pNext->aEPaM.nIndex )
    {
        aText += pNext->aText;
        return true;
    }
    return false;
}

EditUndoRemoveChars::EditUndoRemoveChars(
    EditEngine* pEE, const EPaM& rEPaM, const OUString& rStr) :
    EditUndo(EDITUNDO_REMOVECHARS, pEE),
    aEPaM(rEPaM), aText(rStr) {}

void EditUndoRemoveChars::Undo()
{
    DBG_ASSERT( GetEditEngine()->GetActiveView(), "Undo/Redo: Keine Active View!" );
    EditPaM aPaM = GetEditEngine()->CreateEditPaM(aEPaM);
    EditSelection aSel( aPaM, aPaM );
    GetEditEngine()->InsertText(aSel, aText);
    aSel.Max().SetIndex( aSel.Max().GetIndex() + aText.getLength() );
    GetEditEngine()->GetActiveView()->GetImpEditView()->SetEditSelection(aSel);
}

void EditUndoRemoveChars::Redo()
{
    DBG_ASSERT( GetEditEngine()->GetActiveView(), "Undo/Redo: No Active View!" );
    EditPaM aPaM = GetEditEngine()->CreateEditPaM(aEPaM);
    EditSelection aSel( aPaM, aPaM );
    aSel.Max().SetIndex( aSel.Max().GetIndex() + aText.getLength() );
    EditPaM aNewPaM = GetEditEngine()->DeleteSelection(aSel);
    GetEditEngine()->GetActiveView()->GetImpEditView()->SetEditSelection(aNewPaM);
}

EditUndoInsertFeature::EditUndoInsertFeature(
    EditEngine* pEE, const EPaM& rEPaM, const SfxPoolItem& rFeature) :
    EditUndo(EDITUNDO_INSERTFEATURE, pEE), aEPaM(rEPaM)
{
    pFeature = rFeature.Clone();
    DBG_ASSERT( pFeature, "Feature could not be duplicated: EditUndoInsertFeature" );
}

EditUndoInsertFeature::~EditUndoInsertFeature()
{
    delete pFeature;
}

void EditUndoInsertFeature::Undo()
{
    DBG_ASSERT( GetEditEngine()->GetActiveView(), "Undo/Redo: No Active View!" );
    EditPaM aPaM = GetEditEngine()->CreateEditPaM(aEPaM);
    EditSelection aSel( aPaM, aPaM );
    // Attributes are then corrected implicitly by the document ...
    aSel.Max().SetIndex( aSel.Max().GetIndex()+1 );
    GetEditEngine()->DeleteSelection(aSel);
    aSel.Max().SetIndex( aSel.Max().GetIndex()-1 ); // For Selection
    GetEditEngine()->GetActiveView()->GetImpEditView()->SetEditSelection(aSel);
}

void EditUndoInsertFeature::Redo()
{
    DBG_ASSERT( GetEditEngine()->GetActiveView(), "Undo/Redo: No Active View!" );
    EditPaM aPaM = GetEditEngine()->CreateEditPaM(aEPaM);
    EditSelection aSel( aPaM, aPaM );
    GetEditEngine()->InsertFeature(aSel, *pFeature);
    if ( pFeature->Which() == EE_FEATURE_FIELD )
        GetEditEngine()->UpdateFieldsOnly();
    aSel.Max().SetIndex( aSel.Max().GetIndex()+1 );
    GetEditEngine()->GetActiveView()->GetImpEditView()->SetEditSelection(aSel);
}

EditUndoMoveParagraphs::EditUndoMoveParagraphs(
    EditEngine* pEE, const Range& rParas, sal_Int32 n) :
    EditUndo(EDITUNDO_MOVEPARAGRAPHS, pEE), nParagraphs(rParas), nDest(n) {}

EditUndoMoveParagraphs::~EditUndoMoveParagraphs() {}

void EditUndoMoveParagraphs::Undo()
{
    DBG_ASSERT( GetEditEngine()->GetActiveView(), "Undo/Redo: No Active View!" );
    Range aTmpRange( nParagraphs );
    long nTmpDest = aTmpRange.Min();

    long nDiff = ( nDest - aTmpRange.Min() );
    aTmpRange.Min() += nDiff;
    aTmpRange.Max() += nDiff;

    if ( nParagraphs.Min() < (long)nDest )
    {
        long nLen = aTmpRange.Len();
        aTmpRange.Min() -= nLen;
        aTmpRange.Max() -= nLen;
    }
    else
        nTmpDest += aTmpRange.Len();

    EditSelection aNewSel = GetEditEngine()->MoveParagraphs(aTmpRange, nTmpDest, nullptr);
    GetEditEngine()->GetActiveView()->GetImpEditView()->SetEditSelection( aNewSel );
}

void EditUndoMoveParagraphs::Redo()
{
    DBG_ASSERT( GetEditEngine()->GetActiveView(), "Undo/Redo: No Active View!" );
    EditSelection aNewSel = GetEditEngine()->MoveParagraphs(nParagraphs, nDest, nullptr);
    GetEditEngine()->GetActiveView()->GetImpEditView()->SetEditSelection( aNewSel );
}

EditUndoSetStyleSheet::EditUndoSetStyleSheet(
    EditEngine* pEE, sal_Int32 nP, const OUString& rPrevName, SfxStyleFamily ePrevFam,
    const OUString& rNewName, SfxStyleFamily eNewFam, const SfxItemSet& rPrevParaAttribs) :
    EditUndo(EDITUNDO_STYLESHEET, pEE),
    aPrevName(rPrevName),
    aNewName(rNewName),
    aPrevParaAttribs(rPrevParaAttribs)
{
    ePrevFamily = ePrevFam;
    eNewFamily = eNewFam;
    nPara = nP;
}

EditUndoSetStyleSheet::~EditUndoSetStyleSheet()
{
}

void EditUndoSetStyleSheet::Undo()
{
    DBG_ASSERT( GetEditEngine()->GetActiveView(), "Undo/Redo: No Active View!" );
    GetEditEngine()->SetStyleSheet( nPara, static_cast<SfxStyleSheet*>(GetEditEngine()->GetStyleSheetPool()->Find( aPrevName, ePrevFamily )) );
    GetEditEngine()->SetParaAttribsOnly( nPara, aPrevParaAttribs );
    lcl_DoSetSelection( GetEditEngine()->GetActiveView(), nPara );
}

void EditUndoSetStyleSheet::Redo()
{
    DBG_ASSERT( GetEditEngine()->GetActiveView(), "Undo/Redo: No Active View!" );
    GetEditEngine()->SetStyleSheet( nPara, static_cast<SfxStyleSheet*>(GetEditEngine()->GetStyleSheetPool()->Find( aNewName, eNewFamily )) );
    lcl_DoSetSelection( GetEditEngine()->GetActiveView(), nPara );
}

EditUndoSetParaAttribs::EditUndoSetParaAttribs(
    EditEngine* pEE, sal_Int32 nP, const SfxItemSet& rPrevItems, const SfxItemSet& rNewItems) :
    EditUndo(EDITUNDO_PARAATTRIBS, pEE),
    nPara(nP),
    aPrevItems(rPrevItems),
    aNewItems(rNewItems) {}

EditUndoSetParaAttribs::~EditUndoSetParaAttribs() {}

void EditUndoSetParaAttribs::Undo()
{
    DBG_ASSERT( GetEditEngine()->GetActiveView(), "Undo/Redo: No Active View!" );
    GetEditEngine()->SetParaAttribsOnly( nPara, aPrevItems );
    lcl_DoSetSelection( GetEditEngine()->GetActiveView(), nPara );
}

void EditUndoSetParaAttribs::Redo()
{
    DBG_ASSERT( GetEditEngine()->GetActiveView(), "Undo/Redo: No Active View!" );
    GetEditEngine()->SetParaAttribsOnly( nPara, aNewItems );
    lcl_DoSetSelection( GetEditEngine()->GetActiveView(), nPara );
}

EditUndoSetAttribs::EditUndoSetAttribs(EditEngine* pEE, const ESelection& rESel, const SfxItemSet& rNewItems) :
    EditUndo(EDITUNDO_ATTRIBS, pEE),
    aESel(rESel), aNewAttribs(rNewItems)
{
    // When EditUndoSetAttribs actually is a RemoveAttribs this could be
    // recognize by the empty itemset, but then it would have to be caught in
    // its own place, which possible a setAttribs does with an empty itemset.
    bSetIsRemove = false;
    bRemoveParaAttribs = false;
    nRemoveWhich = 0;
    nSpecial = 0;
}

namespace {

struct RemoveAttribsFromPool : std::unary_function<std::unique_ptr<ContentAttribsInfo>, void>
{
    SfxItemPool& mrPool;
public:
    explicit RemoveAttribsFromPool(SfxItemPool& rPool) : mrPool(rPool) {}
    void operator() (std::unique_ptr<ContentAttribsInfo>& rInfo)
    {
        rInfo->RemoveAllCharAttribsFromPool(mrPool);
    }
};

}

EditUndoSetAttribs::~EditUndoSetAttribs()
{
    // Get Items from Pool...
    SfxItemPool* pPool = aNewAttribs.GetPool();
    std::for_each(aPrevAttribs.begin(), aPrevAttribs.end(), RemoveAttribsFromPool(*pPool));
}

void EditUndoSetAttribs::Undo()
{
    DBG_ASSERT( GetEditEngine()->GetActiveView(), "Undo/Redo: No Active View!" );
    EditEngine* pEE = GetEditEngine();
    bool bFields = false;
    for ( sal_Int32 nPara = aESel.nStartPara; nPara <= aESel.nEndPara; nPara++ )
    {
        const ContentAttribsInfo& rInf = *aPrevAttribs[nPara-aESel.nStartPara].get();

        // first the paragraph attributes ...
        pEE->SetParaAttribsOnly(nPara, rInf.GetPrevParaAttribs());

        // Then the character attributes ...
        // Remove all attributes including features, are later re-established.
        pEE->RemoveCharAttribs(nPara, 0, true);
        DBG_ASSERT( pEE->GetEditDoc().GetObject( nPara ), "Undo (SetAttribs): pNode = NULL!" );
        ContentNode* pNode = pEE->GetEditDoc().GetObject( nPara );
        for (size_t nAttr = 0; nAttr < rInf.GetPrevCharAttribs().size(); ++nAttr)
        {
            const EditCharAttrib& rX = *rInf.GetPrevCharAttribs()[nAttr].get();
            // is automatically "poolsized"
            pEE->GetEditDoc().InsertAttrib(pNode, rX.GetStart(), rX.GetEnd(), *rX.GetItem());
            if (rX.Which() == EE_FEATURE_FIELD)
                bFields = true;
        }
    }
    if ( bFields )
        pEE->UpdateFieldsOnly();
    ImpSetSelection(pEE->GetActiveView());
}

void EditUndoSetAttribs::Redo()
{
    DBG_ASSERT( GetEditEngine()->GetActiveView(), "Undo/Redo: No Active View!" );
    EditEngine* pEE = GetEditEngine();

    EditSelection aSel = pEE->CreateSelection(aESel);
    if ( !bSetIsRemove )
        pEE->SetAttribs( aSel, aNewAttribs, nSpecial );
    else
        pEE->RemoveCharAttribs( aSel, bRemoveParaAttribs, nRemoveWhich );

    ImpSetSelection( GetEditEngine()->GetActiveView() );
}

void EditUndoSetAttribs::AppendContentInfo(ContentAttribsInfo* pNew)
{
    aPrevAttribs.push_back(std::unique_ptr<ContentAttribsInfo>(pNew));
}

void EditUndoSetAttribs::ImpSetSelection( EditView* /*pView*/ )
{
    EditEngine* pEE = GetEditEngine();
    EditSelection aSel = pEE->CreateSelection(aESel);
    pEE->GetActiveView()->GetImpEditView()->SetEditSelection(aSel);
}

EditUndoTransliteration::EditUndoTransliteration(EditEngine* pEE, const ESelection& rESel, sal_Int32 nM) :
    EditUndo(EDITUNDO_TRANSLITERATE, pEE),
    aOldESel(rESel), nMode(nM), pTxtObj(nullptr) {}

EditUndoTransliteration::~EditUndoTransliteration()
{
    delete pTxtObj;
}

void EditUndoTransliteration::Undo()
{
    DBG_ASSERT( GetEditEngine()->GetActiveView(), "Undo/Redo: No Active View!" );

    EditEngine* pEE = GetEditEngine();

    EditSelection aSel = pEE->CreateSelection(aNewESel);

    // Insert text, but don't expand Attribs at the current position:
    aSel = pEE->DeleteSelected( aSel );
    EditSelection aDelSel( aSel );
    aSel = pEE->InsertParaBreak( aSel );
    aDelSel.Max() = aSel.Min();
    aDelSel.Max().GetNode()->GetCharAttribs().DeleteEmptyAttribs( pEE->GetEditDoc().GetItemPool() );
    EditSelection aNewSel;
    if ( pTxtObj )
    {
        aNewSel = pEE->InsertText( *pTxtObj, aSel );
    }
    else
    {
        aNewSel = pEE->InsertText( aSel, aText );
    }
    if ( aNewSel.Min().GetNode() == aDelSel.Max().GetNode() )
    {
        aNewSel.Min().SetNode( aDelSel.Min().GetNode() );
        aNewSel.Min().SetIndex( aNewSel.Min().GetIndex() + aDelSel.Min().GetIndex() );
    }
    if ( aNewSel.Max().GetNode() == aDelSel.Max().GetNode() )
    {
        aNewSel.Max().SetNode( aDelSel.Min().GetNode() );
        aNewSel.Max().SetIndex( aNewSel.Max().GetIndex() + aDelSel.Min().GetIndex() );
    }
    pEE->DeleteSelected( aDelSel );
    pEE->GetActiveView()->GetImpEditView()->SetEditSelection( aNewSel );
}

void EditUndoTransliteration::Redo()
{
    DBG_ASSERT( GetEditEngine()->GetActiveView(), "Undo/Redo: No Active View!" );
    EditEngine* pEE = GetEditEngine();

    EditSelection aSel = pEE->CreateSelection(aOldESel);
    EditSelection aNewSel = pEE->TransliterateText( aSel, nMode );
    pEE->GetActiveView()->GetImpEditView()->SetEditSelection( aNewSel );
}

EditUndoMarkSelection::EditUndoMarkSelection(EditEngine* pEE, const ESelection& rSel) :
    EditUndo(EDITUNDO_MARKSELECTION, pEE), aSelection(rSel) {}

EditUndoMarkSelection::~EditUndoMarkSelection() {}

void EditUndoMarkSelection::Undo()
{
    DBG_ASSERT( GetEditEngine()->GetActiveView(), "Undo/Redo: No Active View!" );
    if ( GetEditEngine()->GetActiveView() )
    {
        if ( GetEditEngine()->IsFormatted() )
            GetEditEngine()->GetActiveView()->SetSelection( aSelection );
        else
            GetEditEngine()->GetActiveView()->GetImpEditView()->SetEditSelection( GetEditEngine()->CreateSelection(aSelection) );
    }
}

void EditUndoMarkSelection::Redo()
{
    // For redo unimportant, because at the beginning of the undo parentheses
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
