/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
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

#include <vcl/wrkwin.hxx>
#include <vcl/dialog.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/svapp.hxx>
#include <impedit.hxx>
#include <editundo.hxx>
#include <editeng/editview.hxx>
#include <editeng/editeng.hxx>

DBG_NAME( EditUndo )

#define NO_UNDO         0xFFFF
#define GROUP_NOTFOUND  0xFFFF

static void lcl_DoSetSelection( EditView* pView, sal_uInt16 nPara )
{
    EPaM aEPaM( nPara, 0 );
    EditPaM aPaM( pView->GetImpEditEngine()->CreateEditPaM( aEPaM ) );
    aPaM.SetIndex( aPaM.GetNode()->Len() );
    EditSelection aSel( aPaM, aPaM );
    pView->GetImpEditView()->SetEditSelection( aSel );
}

EditUndoManager::EditUndoManager(EditEngine* pEE) : mpEditEngine(pEE) {}

sal_Bool EditUndoManager::Undo()
{
    if ( GetUndoActionCount() == 0 )
        return sal_False;

    DBG_ASSERT( mpEditEngine->GetActiveView(), "Active View?" );

    if ( !mpEditEngine->GetActiveView() )
    {
        if (!mpEditEngine->GetEditViews().empty())
            mpEditEngine->SetActiveView(mpEditEngine->GetEditViews()[0]);
        else
        {
            OSL_FAIL("Undo in engine is not possible without a View! ");
            return sal_False;
        }
    }

    mpEditEngine->GetActiveView()->GetImpEditView()->DrawSelection(); // Remove the old selection

    mpEditEngine->SetUndoMode( sal_True );
    sal_Bool bDone = SfxUndoManager::Undo();
    mpEditEngine->SetUndoMode( sal_False );

    EditSelection aNewSel( mpEditEngine->GetActiveView()->GetImpEditView()->GetEditSelection() );
    DBG_ASSERT( !aNewSel.IsInvalid(), "Invalid selection after Undo () ");
    DBG_ASSERT( !aNewSel.DbgIsBuggy( mpEditEngine->GetEditDoc() ), "Broken selection afte Undo () ");

    aNewSel.Min() = aNewSel.Max();
    mpEditEngine->GetActiveView()->GetImpEditView()->SetEditSelection( aNewSel );
    mpEditEngine->FormatAndUpdate( mpEditEngine->GetActiveView() );

    return bDone;
}

sal_Bool EditUndoManager::Redo()
{
    if ( GetRedoActionCount() == 0 )
        return sal_False;

    DBG_ASSERT( mpEditEngine->GetActiveView(), "Active View?" );

    if ( !mpEditEngine->GetActiveView() )
    {
        if (!mpEditEngine->GetEditViews().empty())
            mpEditEngine->SetActiveView(mpEditEngine->GetEditViews()[0]);
        else
        {
            OSL_FAIL( "Redo in Engine ohne View nicht moeglich!" );
            return sal_False;
        }
    }

    mpEditEngine->GetActiveView()->GetImpEditView()->DrawSelection(); // Remove the old selection

    mpEditEngine->SetUndoMode( sal_True );
    sal_Bool bDone = SfxUndoManager::Redo();
    mpEditEngine->SetUndoMode( sal_False );

    EditSelection aNewSel( mpEditEngine->GetActiveView()->GetImpEditView()->GetEditSelection() );
    DBG_ASSERT( !aNewSel.IsInvalid(), "Invalid selection after Undo () ");
    DBG_ASSERT( !aNewSel.DbgIsBuggy( mpEditEngine->GetEditDoc() ), "Broken selection afte Undo () ");

    aNewSel.Min() = aNewSel.Max();
    mpEditEngine->GetActiveView()->GetImpEditView()->SetEditSelection( aNewSel );
    mpEditEngine->FormatAndUpdate( mpEditEngine->GetActiveView() );

    return bDone;
}

EditUndo::EditUndo(sal_uInt16 nI, EditEngine* pEE) :
    nId(nI), mpEditEngine(pEE)
{
}

EditUndo::~EditUndo()
{
}

EditEngine* EditUndo::GetEditEngine()
{
    return mpEditEngine;
}

sal_uInt16 EditUndo::GetId() const
{
    DBG_CHKTHIS( EditUndo, 0 );
    return nId;
}

sal_Bool EditUndo::CanRepeat(SfxRepeatTarget&) const
{
    return sal_False;
}

rtl::OUString EditUndo::GetComment() const
{
    rtl::OUString aComment;

    if (mpEditEngine)
        aComment = mpEditEngine->GetUndoComment( GetId() );

    return aComment;
}

EditUndoDelContent::EditUndoDelContent(
    EditEngine* pEE, ContentNode* pNode, size_t nPortion) :
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
    pEE->GetEditDoc().Remove( nNode );
    if (pEE->IsCallParaInsertedOrDeleted())
        pEE->ParagraphDeleted( nNode );

    DeletedNodeInfo* pInf = new DeletedNodeInfo( (sal_uLong)pContentNode, nNode );
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
    EditEngine* pEE, sal_uInt16 nN, sal_uInt16 nSP,
    const SfxItemSet& rLeftParaAttribs, const SfxItemSet& rRightParaAttribs,
    const SfxStyleSheet* pLeftStyle, const SfxStyleSheet* pRightStyle, bool bBkwrd) :
    EditUndo(EDITUNDO_CONNECTPARAS, pEE),
    aLeftParaAttribs(rLeftParaAttribs),
    aRightParaAttribs(rRightParaAttribs),
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

    sal_Bool bCall = GetEditEngine()->IsCallParaInsertedOrDeleted();
    GetEditEngine()->SetCallParaInsertedOrDeleted(false);

    EditPaM aPaM = GetEditEngine()->SplitContent(nNode, nSepPos);
    GetEditEngine()->SetParaAttribs( nNode, aLeftParaAttribs );
    GetEditEngine()->SetParaAttribs( nNode+1, aRightParaAttribs );

    GetEditEngine()->SetCallParaInsertedOrDeleted( bCall );
    if (GetEditEngine()->IsCallParaInsertedOrDeleted())
        GetEditEngine()->ParagraphInserted( nNode+1 );

    if (GetEditEngine()->GetStyleSheetPool())
    {
        if ( aLeftStyleName.Len() )
            GetEditEngine()->SetStyleSheet( (sal_uInt16)nNode, (SfxStyleSheet*)GetEditEngine()->GetStyleSheetPool()->Find( aLeftStyleName, eLeftStyleFamily ) );
        if ( aRightStyleName.Len() )
            GetEditEngine()->SetStyleSheet( nNode+1, (SfxStyleSheet*)GetEditEngine()->GetStyleSheetPool()->Find( aRightStyleName, eRightStyleFamily ) );
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
    EditEngine* pEE, sal_uInt16 nN, sal_uInt16 nSP) :
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
    EditEngine* pEE, const EPaM& rEPaM, const String& rStr) :
    EditUndo(EDITUNDO_INSERTCHARS, pEE),
    aEPaM(rEPaM),
    aText(rStr) {}

void EditUndoInsertChars::Undo()
{
    DBG_ASSERT( GetEditEngine()->GetActiveView(), "Undo/Redo: No Active View!" );
    EditPaM aPaM = GetEditEngine()->CreateEditPaM(aEPaM);
    EditSelection aSel( aPaM, aPaM );
    aSel.Max().GetIndex() = aSel.Max().GetIndex() + aText.Len();
    EditPaM aNewPaM( GetEditEngine()->DeleteSelection(aSel) );
    GetEditEngine()->GetActiveView()->GetImpEditView()->SetEditSelection( EditSelection( aNewPaM, aNewPaM ) );
}

void EditUndoInsertChars::Redo()
{
    DBG_ASSERT( GetEditEngine()->GetActiveView(), "Undo/Redo: Keine Active View!" );
    EditPaM aPaM = GetEditEngine()->CreateEditPaM(aEPaM);
    GetEditEngine()->InsertText(EditSelection(aPaM, aPaM), aText);
    EditPaM aNewPaM( aPaM );
    aNewPaM.GetIndex() = aNewPaM.GetIndex() + aText.Len();
    GetEditEngine()->GetActiveView()->GetImpEditView()->SetEditSelection( EditSelection( aPaM, aNewPaM ) );
}

sal_Bool EditUndoInsertChars::Merge( SfxUndoAction* pNextAction )
{
    EditUndoInsertChars* pNext = dynamic_cast<EditUndoInsertChars*>(pNextAction);
    if (!pNext)
        return false;

    if ( aEPaM.nPara != pNext->aEPaM.nPara )
        return sal_False;

    if ( ( aEPaM.nIndex + aText.Len() ) == pNext->aEPaM.nIndex )
    {
        aText += pNext->aText;
        return sal_True;
    }
    return sal_False;
}

EditUndoRemoveChars::EditUndoRemoveChars(
    EditEngine* pEE, const EPaM& rEPaM, const String& rStr) :
    EditUndo(EDITUNDO_REMOVECHARS, pEE),
    aEPaM(rEPaM), aText(rStr) {}

void EditUndoRemoveChars::Undo()
{
    DBG_ASSERT( GetEditEngine()->GetActiveView(), "Undo/Redo: Keine Active View!" );
    EditPaM aPaM = GetEditEngine()->CreateEditPaM(aEPaM);
    EditSelection aSel( aPaM, aPaM );
    GetEditEngine()->InsertText(aSel, aText);
    aSel.Max().GetIndex() = aSel.Max().GetIndex() + aText.Len();
    GetEditEngine()->GetActiveView()->GetImpEditView()->SetEditSelection(aSel);
}

void EditUndoRemoveChars::Redo()
{
    DBG_ASSERT( GetEditEngine()->GetActiveView(), "Undo/Redo: No Active View!" );
    EditPaM aPaM = GetEditEngine()->CreateEditPaM(aEPaM);
    EditSelection aSel( aPaM, aPaM );
    aSel.Max().GetIndex() = aSel.Max().GetIndex() + aText.Len();
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
    aSel.Max().GetIndex()++;
    GetEditEngine()->DeleteSelection(aSel);
    aSel.Max().GetIndex()--;    // For Selection
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
    aSel.Max().GetIndex()++;
    GetEditEngine()->GetActiveView()->GetImpEditView()->SetEditSelection(aSel);
}

EditUndoMoveParagraphs::EditUndoMoveParagraphs(
    EditEngine* pEE, const Range& rParas, sal_uInt16 n) :
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

    EditSelection aNewSel = GetEditEngine()->MoveParagraphs(aTmpRange, (sal_uInt16)nTmpDest, 0);
    GetEditEngine()->GetActiveView()->GetImpEditView()->SetEditSelection( aNewSel );
}

void EditUndoMoveParagraphs::Redo()
{
    DBG_ASSERT( GetEditEngine()->GetActiveView(), "Undo/Redo: No Active View!" );
    EditSelection aNewSel = GetEditEngine()->MoveParagraphs(nParagraphs, nDest, 0);
    GetEditEngine()->GetActiveView()->GetImpEditView()->SetEditSelection( aNewSel );
}

EditUndoSetStyleSheet::EditUndoSetStyleSheet(
    EditEngine* pEE, sal_uInt16 nP, const String& rPrevName, SfxStyleFamily ePrevFam,
    const String& rNewName, SfxStyleFamily eNewFam, const SfxItemSet& rPrevParaAttribs) :
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
    GetEditEngine()->SetStyleSheet( nPara, (SfxStyleSheet*)GetEditEngine()->GetStyleSheetPool()->Find( aPrevName, ePrevFamily ) );
    GetEditEngine()->SetParaAttribsOnly( nPara, aPrevParaAttribs );
    lcl_DoSetSelection( GetEditEngine()->GetActiveView(), nPara );
}

void EditUndoSetStyleSheet::Redo()
{
    DBG_ASSERT( GetEditEngine()->GetActiveView(), "Undo/Redo: No Active View!" );
    GetEditEngine()->SetStyleSheet( nPara, (SfxStyleSheet*)GetEditEngine()->GetStyleSheetPool()->Find( aNewName, eNewFamily ) );
    lcl_DoSetSelection( GetEditEngine()->GetActiveView(), nPara );
}

EditUndoSetParaAttribs::EditUndoSetParaAttribs(
    EditEngine* pEE, sal_uInt16 nP, const SfxItemSet& rPrevItems, const SfxItemSet& rNewItems) :
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
    // /recognize by the empty itemset, but then it would have to be caught in
    // its own place, which possible a setAttribs does with an empty itemset.
    bSetIsRemove = sal_False;
    bRemoveParaAttribs = sal_False;
    nRemoveWhich = 0;
    nSpecial = 0;
}

namespace {

struct RemoveAttribsFromPool : std::unary_function<ContentAttribsInfo, void>
{
    SfxItemPool& mrPool;
public:
    RemoveAttribsFromPool(SfxItemPool& rPool) : mrPool(rPool) {}
    void operator() (ContentAttribsInfo& rInfo)
    {
        rInfo.RemoveAllCharAttribsFromPool(mrPool);
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
    for ( sal_uInt16 nPara = aESel.nStartPara; nPara <= aESel.nEndPara; nPara++ )
    {
        const ContentAttribsInfo& rInf = aPrevAttribs[nPara-aESel.nStartPara];

        // first the paragraph attributes ...
        pEE->SetParaAttribsOnly(nPara, rInf.GetPrevParaAttribs());

        // Then the character attributes ...
        // Remove all attributes including features, are later re-established.
        pEE->RemoveCharAttribs(nPara, 0, true);
        DBG_ASSERT( pEE->GetEditDoc().GetObject( nPara ), "Undo (SetAttribs): pNode = NULL!" );
        ContentNode* pNode = pEE->GetEditDoc().GetObject( nPara );
        for (size_t nAttr = 0; nAttr < rInf.GetPrevCharAttribs().size(); ++nAttr)
        {
            const EditCharAttrib& rX = rInf.GetPrevCharAttribs()[nAttr];
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
    aPrevAttribs.push_back(pNew);
}

void EditUndoSetAttribs::ImpSetSelection( EditView* /*pView*/ )
{
    EditEngine* pEE = GetEditEngine();
    EditSelection aSel = pEE->CreateSelection(aESel);
    pEE->GetActiveView()->GetImpEditView()->SetEditSelection(aSel);
}

EditUndoTransliteration::EditUndoTransliteration(EditEngine* pEE, const ESelection& rESel, sal_Int32 nM) :
    EditUndo(EDITUNDO_TRANSLITERATE, pEE),
    aOldESel(rESel), nMode(nM), pTxtObj(NULL) {}

EditUndoTransliteration::~EditUndoTransliteration()
{
    delete pTxtObj;
}

void EditUndoTransliteration::Undo()
{
    DBG_ASSERT( GetEditEngine()->GetActiveView(), "Undo/Redo: No Active View!" );

    EditEngine* pEE = GetEditEngine();

    EditSelection aSel = pEE->CreateSelection(aNewESel);

    // Insert text, but don't expand Atribs at the current position:
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
        aNewSel.Min().GetIndex() =
            aNewSel.Min().GetIndex() + aDelSel.Min().GetIndex();
    }
    if ( aNewSel.Max().GetNode() == aDelSel.Max().GetNode() )
    {
        aNewSel.Max().SetNode( aDelSel.Min().GetNode() );
        aNewSel.Max().GetIndex() =
            aNewSel.Max().GetIndex() + aDelSel.Min().GetIndex();
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
