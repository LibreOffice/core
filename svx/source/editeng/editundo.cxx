/*************************************************************************
 *
 *  $RCSfile: editundo.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:14 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <eeng_pch.hxx>

#pragma hdrstop

#include <impedit.hxx>
#include <editundo.hxx>
#include <editview.hxx>
#include <editeng.hxx>

DBG_NAME( EditUndo );

#define MAX_UNDOS   100     // ab dieser Menge darf geloescht werden....
#define MIN_UNDOS   50      // soviel muss stehen bleiben...

#define NO_UNDO         0xFFFF
#define GROUP_NOTFOUND  0xFFFF

TYPEINIT1( EditUndo, SfxUndoAction );
TYPEINIT1( EditUndoDelContent, EditUndo );
TYPEINIT1( EditUndoConnectParas, EditUndo );
TYPEINIT1( EditUndoSplitPara, EditUndo );
TYPEINIT1( EditUndoInsertChars, EditUndo );
TYPEINIT1( EditUndoRemoveChars, EditUndo );
TYPEINIT1( EditUndoInsertFeature, EditUndo );
TYPEINIT1( EditUndoMoveParagraphs, EditUndo );
TYPEINIT1( EditUndoSetStyleSheet, EditUndo );
TYPEINIT1( EditUndoSetParaAttribs, EditUndo );
TYPEINIT1( EditUndoSetAttribs, EditUndo );
TYPEINIT1( EditUndoMarkSelection, EditUndo );

void lcl_DoSetSelection( EditView* pView, USHORT nPara )
{
    EPaM aEPaM( nPara, 0 );
    EditPaM aPaM( pView->GetImpEditEngine()->CreateEditPaM( aEPaM ) );
    aPaM.SetIndex( aPaM.GetNode()->Len() );
    EditSelection aSel( aPaM, aPaM );
    pView->GetImpEditView()->SetEditSelection( aSel );
}

// -----------------------------------------------------------------------
// EditUndoManager
// ------------------------------------------------------------------------
EditUndoManager::EditUndoManager( ImpEditEngine* p )
{
    pImpEE = p;
}

BOOL __EXPORT EditUndoManager::Undo( USHORT nCount )
{
    if ( GetUndoActionCount() == 0 )
        return FALSE;

    DBG_ASSERT( pImpEE->GetActiveView(), "Active View?" );

    if ( !pImpEE->GetActiveView() )
    {
        if ( pImpEE->GetEditViews().Count() )
            pImpEE->SetActiveView( pImpEE->GetEditViews().GetObject(0) );
        else
        {
            DBG_ERROR( "Undo in Engine ohne View nicht moeglich!" );
            return FALSE;
        }
    }

    pImpEE->GetActiveView()->GetImpEditView()->DrawSelection(); // alte Selektion entfernen

    pImpEE->SetUndoMode( TRUE );
    BOOL bDone = SfxUndoManager::Undo( nCount );
    pImpEE->SetUndoMode( FALSE );

    EditSelection aNewSel( pImpEE->GetActiveView()->GetImpEditView()->GetEditSelection() );
    DBG_ASSERT( !aNewSel.IsInvalid(), "Ungueltige Selektion nach Undo()" );
    DBG_ASSERT( !aNewSel.DbgIsBuggy( pImpEE->GetEditDoc() ), "Kaputte Selektion nach Undo()" );

    aNewSel.Min() = aNewSel.Max();
    pImpEE->GetActiveView()->GetImpEditView()->SetEditSelection( aNewSel );
    pImpEE->FormatAndUpdate( pImpEE->GetActiveView() );

    return bDone;
}

BOOL __EXPORT EditUndoManager::Redo( USHORT nCount )
{
    if ( GetRedoActionCount() == 0 )
        return FALSE;

    DBG_ASSERT( pImpEE->GetActiveView(), "Active View?" );

    if ( !pImpEE->GetActiveView() )
    {
        if ( pImpEE->GetEditViews().Count() )
            pImpEE->SetActiveView( pImpEE->GetEditViews().GetObject(0) );
        else
        {
            DBG_ERROR( "Redo in Engine ohne View nicht moeglich!" );
            return FALSE;
        }
    }

    pImpEE->GetActiveView()->GetImpEditView()->DrawSelection(); // alte Selektion entfernen

    pImpEE->SetUndoMode( TRUE );
    BOOL bDone = SfxUndoManager::Redo( nCount );
    pImpEE->SetUndoMode( FALSE );

    EditSelection aNewSel( pImpEE->GetActiveView()->GetImpEditView()->GetEditSelection() );
    DBG_ASSERT( !aNewSel.IsInvalid(), "Ungueltige Selektion nach Undo()" );
    DBG_ASSERT( !aNewSel.DbgIsBuggy( pImpEE->GetEditDoc() ), "Kaputte Selektion nach Redo()" );

    aNewSel.Min() = aNewSel.Max();
    pImpEE->GetActiveView()->GetImpEditView()->SetEditSelection( aNewSel );
    pImpEE->FormatAndUpdate( pImpEE->GetActiveView() );

    return bDone;
}

// -----------------------------------------------------------------------
// EditUndo
// ------------------------------------------------------------------------
EditUndo::EditUndo( USHORT nI, ImpEditEngine* p )
{
    DBG_CTOR( EditUndo, 0 );
    nId = nI;
    pImpEE = p;
}

EditUndo::~EditUndo()
{
    DBG_DTOR( EditUndo, 0 );
}

USHORT __EXPORT EditUndo::GetId() const
{
    DBG_CHKTHIS( EditUndo, 0 );
    return nId;
}

BOOL __EXPORT EditUndo::CanRepeat(SfxRepeatTarget&) const
{
    return FALSE;
}

XubString __EXPORT EditUndo::GetComment() const
{
    XubString aComment;
    if ( pImpEE )
    {
        EditEngine* pEditEng = pImpEE->GetEditEnginePtr();
        aComment = pEditEng->GetUndoComment( GetId() );
    }
    return aComment;
}

// -----------------------------------------------------------------------
// EditUndoDelContent
// ------------------------------------------------------------------------
EditUndoDelContent::EditUndoDelContent( ImpEditEngine* pImpEE, ContentNode* pNode, USHORT n )
                    : EditUndo( EDITUNDO_DELCONTENT, pImpEE )
{
    pContentNode = pNode;
    nNode = n;
    bDelObject = TRUE;
}

EditUndoDelContent::~EditUndoDelContent()
{
    if ( bDelObject )
        delete pContentNode;
}

void __EXPORT EditUndoDelContent::Undo()
{
    DBG_ASSERT( GetImpEditEngine()->GetActiveView(), "Undo/Redo: Keine Active View!" );
    GetImpEditEngine()->InsertContent( pContentNode, nNode );
    bDelObject = FALSE; // gehoert wieder der Engine
    EditSelection aSel( EditPaM( pContentNode, 0 ), EditPaM( pContentNode, pContentNode->Len() ) );
    GetImpEditEngine()->GetActiveView()->GetImpEditView()->SetEditSelection( aSel );
}

void __EXPORT EditUndoDelContent::Redo()
{
    DBG_ASSERT( GetImpEditEngine()->GetActiveView(), "Undo/Redo: Keine Active View!" );

    ImpEditEngine* pImpEE = GetImpEditEngine();

    // pNode stimmt nicht mehr, falls zwischendurch Undos, in denen
    // Absaetze verschmolzen sind.
    pContentNode = pImpEE->GetEditDoc().SaveGetObject( nNode );
    DBG_ASSERT( pContentNode, "EditUndoDelContent::Redo(): Node?!" );

    delete pImpEE->GetParaPortions()[nNode];
    pImpEE->GetParaPortions().Remove( nNode );

    // Node nicht loeschen, haengt im Undo!
    pImpEE->GetEditDoc().Remove( nNode );
    if( pImpEE->IsCallParaInsertedOrDeleted() )
        pImpEE->GetEditEnginePtr()->ParagraphDeleted( nNode );

    DeletedNodeInfo* pInf = new DeletedNodeInfo( (ULONG)pContentNode, nNode );
    pImpEE->aDeletedNodes.Insert( pInf, pImpEE->aDeletedNodes.Count() );
    pImpEE->UpdateSelections();

    ContentNode* pN = ( nNode < pImpEE->GetEditDoc().Count() )
        ? pImpEE->GetEditDoc().SaveGetObject( nNode )
        : pImpEE->GetEditDoc().SaveGetObject( nNode-1 );
    DBG_ASSERT( pN && ( pN != pContentNode ), "?! RemoveContent !? " );
    EditPaM aPaM( pN, pN->Len() );

    bDelObject = TRUE;  // gehoert wieder dem Undo

    pImpEE->GetActiveView()->GetImpEditView()->SetEditSelection( EditSelection( aPaM, aPaM ) );
}

void __EXPORT EditUndoDelContent::Repeat()
{
    DBG_ERROR( "EditUndoDelContent::Repeat nicht implementiert!" );
}

// -----------------------------------------------------------------------
// EditUndoConnectParas
// ------------------------------------------------------------------------
EditUndoConnectParas::EditUndoConnectParas( ImpEditEngine* pImpEE, USHORT nN, USHORT nSP,
                                            const SfxItemSet& rLeftParaAttribs, const SfxItemSet& rRightParaAttribs,
                                            const SfxStyleSheet* pLeftStyle, const SfxStyleSheet* pRightStyle, BOOL bBkwrd )
                    :   EditUndo( EDITUNDO_CONNECTPARAS, pImpEE ),
                        aLeftParaAttribs( rLeftParaAttribs ),
                        aRightParaAttribs( rRightParaAttribs )
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

    bBackward = bBkwrd;
}

EditUndoConnectParas::~EditUndoConnectParas()
{
}

void __EXPORT EditUndoConnectParas::Undo()
{
    DBG_ASSERT( GetImpEditEngine()->GetActiveView(), "Undo/Redo: Keine Active View!" );

    // Bei SplitContent darf noch kein ParagraphInserted gerufen werden,
    // weil der Outliner sich auf die Attribute verlaesst um die Tiefe
    // des Absatzes zu initialisieren

    BOOL bCall = GetImpEditEngine()->IsCallParaInsertedOrDeleted();
    GetImpEditEngine()->SetCallParaInsertedOrDeleted( FALSE );

    EditPaM aPaM = GetImpEditEngine()->SplitContent( nNode, nSepPos );
    GetImpEditEngine()->SetParaAttribs( nNode, aLeftParaAttribs );
    GetImpEditEngine()->SetParaAttribs( nNode+1, aRightParaAttribs );

    GetImpEditEngine()->SetCallParaInsertedOrDeleted( bCall );
    if ( GetImpEditEngine()->IsCallParaInsertedOrDeleted() )
        GetImpEditEngine()->GetEditEnginePtr()->ParagraphInserted( nNode+1 );

    if ( GetImpEditEngine()->GetStyleSheetPool() )
    {
        if ( aLeftStyleName.Len() )
            GetImpEditEngine()->SetStyleSheet( (USHORT)nNode, (SfxStyleSheet*)GetImpEditEngine()->GetStyleSheetPool()->Find( aLeftStyleName, eLeftStyleFamily ) );
        if ( aRightStyleName.Len() )
            GetImpEditEngine()->SetStyleSheet( nNode+1, (SfxStyleSheet*)GetImpEditEngine()->GetStyleSheetPool()->Find( aRightStyleName, eRightStyleFamily ) );
    }

    GetImpEditEngine()->GetActiveView()->GetImpEditView()->SetEditSelection( EditSelection( aPaM, aPaM ) );
}

void __EXPORT EditUndoConnectParas::Redo()
{
    DBG_ASSERT( GetImpEditEngine()->GetActiveView(), "Undo/Redo: Keine Active View!" );
    EditPaM aPaM = GetImpEditEngine()->ConnectContents( nNode, bBackward );

    GetImpEditEngine()->GetActiveView()->GetImpEditView()->SetEditSelection( EditSelection( aPaM, aPaM ) );
}

void __EXPORT EditUndoConnectParas::Repeat()
{
    DBG_ERROR( "EditUndoConnectParas::Repeat nicht implementiert!" );
}

// -----------------------------------------------------------------------
// EditUndoSplitPara
// ------------------------------------------------------------------------
EditUndoSplitPara::EditUndoSplitPara( ImpEditEngine* pImpEE, USHORT nN, USHORT nSP )
                    : EditUndo( EDITUNDO_SPLITPARA, pImpEE )
{
    nNode   = nN;
    nSepPos = nSP;
}

EditUndoSplitPara::~EditUndoSplitPara()
{
}

void __EXPORT EditUndoSplitPara::Undo()
{
    DBG_ASSERT( GetImpEditEngine()->GetActiveView(), "Undo/Redo: Keine Active View!" );
    EditPaM aPaM = GetImpEditEngine()->ConnectContents( nNode, FALSE );
    GetImpEditEngine()->GetActiveView()->GetImpEditView()->SetEditSelection( EditSelection( aPaM, aPaM ) );
}

void __EXPORT EditUndoSplitPara::Redo()
{
    DBG_ASSERT( GetImpEditEngine()->GetActiveView(), "Undo/Redo: Keine Active View!" );
    EditPaM aPaM = GetImpEditEngine()->SplitContent( nNode, nSepPos );
    GetImpEditEngine()->GetActiveView()->GetImpEditView()->SetEditSelection( EditSelection( aPaM, aPaM ) );
}

void __EXPORT EditUndoSplitPara::Repeat()
{
    DBG_ERROR( "EditUndoSplitPara::Repeat nicht implementiert!" );
}

// -----------------------------------------------------------------------
// EditUndoInsertChars
// ------------------------------------------------------------------------
EditUndoInsertChars::EditUndoInsertChars( ImpEditEngine* pImpEE, const EPaM& rEPaM, const XubString& rStr )
                    : EditUndo( EDITUNDO_INSERTCHARS, pImpEE ),
                        aEPaM( rEPaM ), aText( rStr )
{
}

void __EXPORT EditUndoInsertChars::Undo()
{
    DBG_ASSERT( GetImpEditEngine()->GetActiveView(), "Undo/Redo: Keine Active View!" );
    EditPaM aPaM( GetImpEditEngine()->CreateEditPaM( aEPaM ) );
    EditSelection aSel( aPaM, aPaM );
    aSel.Max().GetIndex() += aText.Len();
    EditPaM aNewPaM( GetImpEditEngine()->ImpDeleteSelection( aSel ) );
    GetImpEditEngine()->GetActiveView()->GetImpEditView()->SetEditSelection( EditSelection( aNewPaM, aNewPaM ) );
}

void __EXPORT EditUndoInsertChars::Redo()
{
    DBG_ASSERT( GetImpEditEngine()->GetActiveView(), "Undo/Redo: Keine Active View!" );
    EditPaM aPaM( GetImpEditEngine()->CreateEditPaM( aEPaM ) );
    GetImpEditEngine()->ImpInsertText( EditSelection( aPaM, aPaM ), aText );
    EditPaM aNewPaM( aPaM );
    aNewPaM.GetIndex() += aText.Len();
    GetImpEditEngine()->GetActiveView()->GetImpEditView()->SetEditSelection( EditSelection( aPaM, aNewPaM ) );
}

BOOL __EXPORT EditUndoInsertChars::Merge( SfxUndoAction* pNextAction )
{
    if ( !pNextAction->ISA( EditUndoInsertChars ) )
        return FALSE;

    EditUndoInsertChars* pNext = (EditUndoInsertChars*)pNextAction;

    if ( aEPaM.nPara != pNext->aEPaM.nPara )
        return FALSE;

    if ( ( aEPaM.nIndex + aText.Len() ) == pNext->aEPaM.nIndex )
    {
        aText += pNext->aText;
        return TRUE;
    }
    return FALSE;
}

void __EXPORT EditUndoInsertChars::Repeat()
{
    DBG_ERROR( "EditUndoInsertChars::Repeat nicht implementiert!" );
}

// -----------------------------------------------------------------------
// EditUndoRemoveChars
// ------------------------------------------------------------------------
EditUndoRemoveChars::EditUndoRemoveChars( ImpEditEngine* pImpEE, const EPaM& rEPaM, const XubString& rStr )
                    : EditUndo( EDITUNDO_REMOVECHARS, pImpEE ),
                        aEPaM( rEPaM ), aText( rStr )
{
}

void __EXPORT EditUndoRemoveChars::Undo()
{
    DBG_ASSERT( GetImpEditEngine()->GetActiveView(), "Undo/Redo: Keine Active View!" );
    EditPaM aPaM( GetImpEditEngine()->CreateEditPaM( aEPaM ) );
    EditSelection aSel( aPaM, aPaM );
    GetImpEditEngine()->ImpInsertText( aSel, aText );
    aSel.Max().GetIndex() += aText.Len();
    GetImpEditEngine()->GetActiveView()->GetImpEditView()->SetEditSelection( aSel );
}

void __EXPORT EditUndoRemoveChars::Redo()
{
    DBG_ASSERT( GetImpEditEngine()->GetActiveView(), "Undo/Redo: Keine Active View!" );
    EditPaM aPaM( GetImpEditEngine()->CreateEditPaM( aEPaM ) );
    EditSelection aSel( aPaM, aPaM );
    aSel.Max().GetIndex() += aText.Len();
    EditPaM aNewPaM = GetImpEditEngine()->ImpDeleteSelection( aSel );
    GetImpEditEngine()->GetActiveView()->GetImpEditView()->SetEditSelection( aNewPaM );
}

void __EXPORT EditUndoRemoveChars::Repeat()
{
    DBG_ERROR( "EditUndoRemoveChars::Repeat nicht implementiert!" );
}

// -----------------------------------------------------------------------
// EditUndoInsertFeature
// ------------------------------------------------------------------------
EditUndoInsertFeature::EditUndoInsertFeature( ImpEditEngine* pImpEE, const EPaM& rEPaM, const SfxPoolItem& rFeature)
                    : EditUndo( EDITUNDO_INSERTFEATURE, pImpEE ), aEPaM( rEPaM )
{
    pFeature = rFeature.Clone();
    DBG_ASSERT( pFeature, "Feature konnte nicht dupliziert werden: EditUndoInsertFeature" );
}

EditUndoInsertFeature::~EditUndoInsertFeature()
{
    delete pFeature;
}

void __EXPORT EditUndoInsertFeature::Undo()
{
    DBG_ASSERT( GetImpEditEngine()->GetActiveView(), "Undo/Redo: Keine Active View!" );
    EditPaM aPaM( GetImpEditEngine()->CreateEditPaM( aEPaM ) );
    EditSelection aSel( aPaM, aPaM );
    // Attribute werden dort implizit vom Dokument korrigiert...
    aSel.Max().GetIndex()++;
    EditPaM aNewPaM = GetImpEditEngine()->ImpDeleteSelection( aSel );
    aSel.Max().GetIndex()--;    // Fuer Selektion
    GetImpEditEngine()->GetActiveView()->GetImpEditView()->SetEditSelection( aSel );
}

void __EXPORT EditUndoInsertFeature::Redo()
{
    DBG_ASSERT( GetImpEditEngine()->GetActiveView(), "Undo/Redo: Keine Active View!" );
    EditPaM aPaM( GetImpEditEngine()->CreateEditPaM( aEPaM ) );
    EditSelection aSel( aPaM, aPaM );
    GetImpEditEngine()->ImpInsertFeature( aSel, *pFeature );
    if ( pFeature->Which() == EE_FEATURE_FIELD )
        GetImpEditEngine()->UpdateFields();
    aSel.Max().GetIndex()++;
    GetImpEditEngine()->GetActiveView()->GetImpEditView()->SetEditSelection( aSel );
}

void __EXPORT EditUndoInsertFeature::Repeat()
{
    DBG_ERROR( "EditUndoInsertFeature::Repeat nicht implementiert!" );
}

// -----------------------------------------------------------------------
// EditUndoMoveParagraphs
// ------------------------------------------------------------------------
EditUndoMoveParagraphs::EditUndoMoveParagraphs
                            ( ImpEditEngine* pImpEE, const Range& rParas, USHORT n )
                            :   EditUndo( EDITUNDO_MOVEPARAGRAPHS, pImpEE ),
                                nParagraphs( rParas )
{
    nDest = n;
}

EditUndoMoveParagraphs::~EditUndoMoveParagraphs()
{
}

void __EXPORT EditUndoMoveParagraphs::Undo()
{
    DBG_ASSERT( GetImpEditEngine()->GetActiveView(), "Undo/Redo: Keine Active View!" );
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

    EditSelection aNewSel( GetImpEditEngine()->MoveParagraphs( aTmpRange, (USHORT)nTmpDest, 0 ) );
    GetImpEditEngine()->GetActiveView()->GetImpEditView()->SetEditSelection( aNewSel );
}

void __EXPORT EditUndoMoveParagraphs::Redo()
{
    DBG_ASSERT( GetImpEditEngine()->GetActiveView(), "Undo/Redo: Keine Active View!" );
    EditSelection aNewSel( GetImpEditEngine()->MoveParagraphs( nParagraphs, nDest, 0 ) );
    GetImpEditEngine()->GetActiveView()->GetImpEditView()->SetEditSelection( aNewSel );
}

void __EXPORT EditUndoMoveParagraphs::Repeat()
{
    DBG_ERROR( "EditUndoMoveParagraphs::Repeat nicht implementiert!" );
}

// -----------------------------------------------------------------------
// EditUndoSetStyleSheet
// ------------------------------------------------------------------------
EditUndoSetStyleSheet::EditUndoSetStyleSheet( ImpEditEngine* pImpEE, USHORT nP,
                        const XubString& rPrevName, SfxStyleFamily ePrevFam,
                        const XubString& rNewName, SfxStyleFamily eNewFam,
                        const SfxItemSet& rPrevParaAttribs )
    : EditUndo( EDITUNDO_STYLESHEET, pImpEE ), aPrevName( rPrevName ), aNewName( rNewName ),
      aPrevParaAttribs( rPrevParaAttribs )
{
    ePrevFamily = ePrevFam;
    eNewFamily = eNewFam;
    nPara = nP;
}

EditUndoSetStyleSheet::~EditUndoSetStyleSheet()
{
}

void __EXPORT EditUndoSetStyleSheet::Undo()
{
    DBG_ASSERT( GetImpEditEngine()->GetActiveView(), "Undo/Redo: Keine Active View!" );
    GetImpEditEngine()->SetStyleSheet( nPara, (SfxStyleSheet*)GetImpEditEngine()->GetStyleSheetPool()->Find( aPrevName, ePrevFamily ) );
    GetImpEditEngine()->SetParaAttribs( nPara, aPrevParaAttribs );
    lcl_DoSetSelection( GetImpEditEngine()->GetActiveView(), nPara );
}

void __EXPORT EditUndoSetStyleSheet::Redo()
{
    DBG_ASSERT( GetImpEditEngine()->GetActiveView(), "Undo/Redo: Keine Active View!" );
    GetImpEditEngine()->SetStyleSheet( nPara, (SfxStyleSheet*)GetImpEditEngine()->GetStyleSheetPool()->Find( aNewName, eNewFamily ) );
    lcl_DoSetSelection( GetImpEditEngine()->GetActiveView(), nPara );
}

void __EXPORT EditUndoSetStyleSheet::Repeat()
{
    DBG_ERROR( "EditUndoSetStyleSheet::Repeat nicht implementiert!" );
}

// -----------------------------------------------------------------------
// EditUndoSetParaAttribs
// ------------------------------------------------------------------------
EditUndoSetParaAttribs::EditUndoSetParaAttribs( ImpEditEngine* pImpEE, USHORT nP, const SfxItemSet& rPrevItems, const SfxItemSet& rNewItems )
    : EditUndo( EDITUNDO_PARAATTRIBS, pImpEE ),
      aPrevItems( rPrevItems ),
      aNewItems(rNewItems )
{
    nPara = nP;
}

EditUndoSetParaAttribs::~EditUndoSetParaAttribs()
{
}

void __EXPORT EditUndoSetParaAttribs::Undo()
{
    DBG_ASSERT( GetImpEditEngine()->GetActiveView(), "Undo/Redo: Keine Active View!" );
    GetImpEditEngine()->SetParaAttribs( nPara, aPrevItems );
    lcl_DoSetSelection( GetImpEditEngine()->GetActiveView(), nPara );
}

void __EXPORT EditUndoSetParaAttribs::Redo()
{
    DBG_ASSERT( GetImpEditEngine()->GetActiveView(), "Undo/Redo: Keine Active View!" );
    GetImpEditEngine()->SetParaAttribs( nPara, aNewItems );
    lcl_DoSetSelection( GetImpEditEngine()->GetActiveView(), nPara );
}

void __EXPORT EditUndoSetParaAttribs::Repeat()
{
    DBG_ERROR( "EditUndoSetParaAttribs::Repeat nicht implementiert!" );
}

// -----------------------------------------------------------------------
// EditUndoSetAttribs
// ------------------------------------------------------------------------
EditUndoSetAttribs::EditUndoSetAttribs( ImpEditEngine* pImpEE, const ESelection& rESel, const SfxItemSet& rNewItems )
    : EditUndo( EDITUNDO_ATTRIBS, pImpEE ),
      aNewAttribs( rNewItems ),
      aESel( rESel )
{
    // Wenn das EditUndoSetAttribs eigentlich ein RemoveAttribs ist, koennte
    // man das eigentlich an einem leeren ItemSet erkennen, aber dann muesste
    // an einigen Stellen abgefangen werden, das ggf. ein SetAttribs mit einem
    // leeren ItemSet gemacht wird.
    // => Ich habe lieber diesen Member spendiert...
    bSetIsRemove = FALSE;
    bRemoveParaAttribs = FALSE;
    nRemoveWhich = 0;
    nSpecial = 0;
}

EditUndoSetAttribs::~EditUndoSetAttribs()
{
    // Items aus Pool holen...
    SfxItemPool* pPool = aNewAttribs.GetPool();
    USHORT nContents = aPrevAttribs.Count();
    for ( USHORT n = 0; n < nContents; n++ )
    {
        ContentAttribsInfo* pInf = aPrevAttribs[n];
        DBG_ASSERT( pInf, "Undo_DTOR (SetAttribs): pInf = NULL!" );
        for ( USHORT nAttr = 0; nAttr < pInf->GetPrevCharAttribs().Count(); nAttr++ )
        {
            EditCharAttrib* pX = pInf->GetPrevCharAttribs()[nAttr];
            DBG_ASSERT( pX, "Undo_DTOR (SetAttribs): pX = NULL!" );
            pPool->Remove( *pX->GetItem() );
            delete pX;
        }
        delete pInf;
    }
}

void __EXPORT EditUndoSetAttribs::Undo()
{
    DBG_ASSERT( GetImpEditEngine()->GetActiveView(), "Undo/Redo: Keine Active View!" );
    ImpEditEngine* pImpEE = GetImpEditEngine();
    BOOL bFields = FALSE;
    for ( USHORT nPara = aESel.nStartPara; nPara <= aESel.nEndPara; nPara++ )
    {
        ContentAttribsInfo* pInf = aPrevAttribs[ (USHORT)(nPara-aESel.nStartPara) ];
        DBG_ASSERT( pInf, "Undo (SetAttribs): pInf = NULL!" );

        // erstmal die Absatzattribute...
        pImpEE->SetParaAttribs( nPara, pInf->GetPrevParaAttribs() );

        // Dann die Zeichenattribute...
        // Alle Attribute inkl. Features entfernen, werden wieder neu eingestellt.
        pImpEE->RemoveCharAttribs( nPara, 0, TRUE );
        DBG_ASSERT( pImpEE->GetEditDoc().SaveGetObject( nPara ), "Undo (SetAttribs): pNode = NULL!" );
        ContentNode* pNode = pImpEE->GetEditDoc().GetObject( nPara );
        for ( USHORT nAttr = 0; nAttr < pInf->GetPrevCharAttribs().Count(); nAttr++ )
        {
            EditCharAttrib* pX = pInf->GetPrevCharAttribs()[nAttr];
            DBG_ASSERT( pX, "Redo (SetAttribs): pX = NULL!" );
            // wird autom. 'eingepoolt'.
            pImpEE->GetEditDoc().InsertAttrib( pNode, pX->GetStart(), pX->GetEnd(), *pX->GetItem() );
            if ( pX->Which() == EE_FEATURE_FIELD )
                bFields = TRUE;
        }
    }
    if ( bFields )
        pImpEE->UpdateFields();
    ImpSetSelection( GetImpEditEngine()->GetActiveView() );
}

void __EXPORT EditUndoSetAttribs::Redo()
{
    DBG_ASSERT( GetImpEditEngine()->GetActiveView(), "Undo/Redo: Keine Active View!" );
    ImpEditEngine* pImpEE = GetImpEditEngine();

    EditSelection aSel( pImpEE->CreateSel( aESel ) );
    if ( !bSetIsRemove )
        pImpEE->SetAttribs( aSel, aNewAttribs, nSpecial );
    else
        pImpEE->RemoveCharAttribs( aSel, bRemoveParaAttribs, nRemoveWhich );

    ImpSetSelection( GetImpEditEngine()->GetActiveView() );
}

void __EXPORT EditUndoSetAttribs::Repeat()
{
    DBG_ERROR( "EditUndoSetAttribs::Repeat nicht implementiert!" );
}

void EditUndoSetAttribs::ImpSetSelection( EditView* pView )
{
    ImpEditEngine* pImpEE = GetImpEditEngine();
    EditSelection aSel( pImpEE->CreateSel( aESel ) );
    GetImpEditEngine()->GetActiveView()->GetImpEditView()->SetEditSelection( aSel );
}

// -----------------------------------------------------------------------
// EditUndoMarkSelection
// ------------------------------------------------------------------------
EditUndoMarkSelection::EditUndoMarkSelection( ImpEditEngine* pImpEE, const ESelection& rSel )
    : EditUndo( EDITUNDO_MARKSELECTION, pImpEE ), aSelection( rSel )
{
}

EditUndoMarkSelection::~EditUndoMarkSelection()
{
}

void __EXPORT EditUndoMarkSelection::Undo()
{
    DBG_ASSERT( GetImpEditEngine()->GetActiveView(), "Undo/Redo: Keine Active View!" );
    if ( GetImpEditEngine()->GetActiveView() )
        GetImpEditEngine()->GetActiveView()->SetSelection( aSelection );
}

void __EXPORT EditUndoMarkSelection::Redo()
{
    // Fuer Redo unwichtig, weil am Anfang der Undo-Klammerung
}

void __EXPORT EditUndoMarkSelection::Repeat()
{
    DBG_ERROR( "EditUndoMarkSelection::Repeat nicht implementiert!" );
}

