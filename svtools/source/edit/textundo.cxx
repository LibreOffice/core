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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svtools.hxx"

#include <svtools/texteng.hxx>
#include <svtools/textview.hxx>
#include <textundo.hxx>
#include <textund2.hxx>
#include <svtools/textdata.hxx>
#include <textdoc.hxx>
#include <textdat2.hxx>

TYPEINIT1( TextUndo, SfxUndoAction );
TYPEINIT1( TextUndoDelPara, TextUndo );
TYPEINIT1( TextUndoConnectParas, TextUndo );
TYPEINIT1( TextUndoSplitPara, TextUndo );
TYPEINIT1( TextUndoInsertChars, TextUndo );
TYPEINIT1( TextUndoRemoveChars, TextUndo );
TYPEINIT1( TextUndoSetAttribs, TextUndo );


TextUndoManager::TextUndoManager( TextEngine* p )
{
    mpTextEngine = p;
}

TextUndoManager::~TextUndoManager()
{
}

BOOL __EXPORT TextUndoManager::Undo()
{
    if ( GetUndoActionCount() == 0 )
        return FALSE;

    UndoRedoStart();

    mpTextEngine->SetIsInUndo( TRUE );
    BOOL bDone = SfxUndoManager::Undo();
    mpTextEngine->SetIsInUndo( FALSE );

    UndoRedoEnd();

    return bDone;
}

BOOL __EXPORT TextUndoManager::Redo()
{
    if ( GetRedoActionCount() == 0 )
        return FALSE;


    UndoRedoStart();

    mpTextEngine->SetIsInUndo( TRUE );
    BOOL bDone = SfxUndoManager::Redo();
    mpTextEngine->SetIsInUndo( FALSE );

    UndoRedoEnd();

    return bDone;
}

void TextUndoManager::UndoRedoStart()
{
    DBG_ASSERT( GetView(), "Undo/Redo: Active View?" );

//  if ( GetView() )
//      GetView()->HideSelection();
}

void TextUndoManager::UndoRedoEnd()
{
    if ( GetView() )
    {
        TextSelection aNewSel( GetView()->GetSelection() );
        aNewSel.GetStart() = aNewSel.GetEnd();
        GetView()->ImpSetSelection( aNewSel );
    }

    mpTextEngine->UpdateSelections();

    mpTextEngine->FormatAndUpdate( GetView() );
}


TextUndo::TextUndo( TextEngine* p )
{
    mpTextEngine = p;
}

TextUndo::~TextUndo()
{
}

XubString __EXPORT TextUndo::GetComment() const
{
//  return mpTextEngine->GetUndoComment( this );
    return String();
}

void TextUndo::SetSelection( const TextSelection& rSel )
{
    if ( GetView() )
        GetView()->ImpSetSelection( rSel );
}


TextUndoDelPara::TextUndoDelPara( TextEngine* pTextEngine, TextNode* pNode, ULONG nPara )
                    : TextUndo( pTextEngine )
{
    mpNode = pNode;
    mnPara = nPara;
    mbDelObject = TRUE;
}

TextUndoDelPara::~TextUndoDelPara()
{
    if ( mbDelObject )
        delete mpNode;
}

void __EXPORT TextUndoDelPara::Undo()
{
    GetTextEngine()->InsertContent( mpNode, mnPara );
    mbDelObject = FALSE;    // gehoert wieder der Engine

    if ( GetView() )
    {
        TextSelection aSel( TextPaM( mnPara, 0 ), TextPaM( mnPara, mpNode->GetText().Len() ) );
        SetSelection( aSel );
    }
}

void __EXPORT TextUndoDelPara::Redo()
{
    // pNode stimmt nicht mehr, falls zwischendurch Undos, in denen
    // Absaetze verschmolzen sind.
    mpNode = GetDoc()->GetNodes().GetObject( mnPara );

    delete GetTEParaPortions()->GetObject( mnPara );
    GetTEParaPortions()->Remove( mnPara );

    // Node nicht loeschen, haengt im Undo!
    GetDoc()->GetNodes().Remove( mnPara );
    GetTextEngine()->ImpParagraphRemoved( mnPara );

    mbDelObject = TRUE; // gehoert wieder dem Undo

    ULONG nParas = GetDoc()->GetNodes().Count();
    ULONG n = mnPara < nParas ? mnPara : (nParas-1);
    TextNode* pN = GetDoc()->GetNodes().GetObject( n );
    TextPaM aPaM( n, pN->GetText().Len() );
    SetSelection( aPaM );
}

// -----------------------------------------------------------------------
// TextUndoConnectParas
// ------------------------------------------------------------------------
TextUndoConnectParas::TextUndoConnectParas( TextEngine* pTextEngine, ULONG nPara, USHORT nPos )
                    :   TextUndo( pTextEngine )
{
    mnPara = nPara;
    mnSepPos = nPos;
}

TextUndoConnectParas::~TextUndoConnectParas()
{
}

void __EXPORT TextUndoConnectParas::Undo()
{
    TextPaM aPaM = GetTextEngine()->SplitContent( mnPara, mnSepPos );
    SetSelection( aPaM );
}

void __EXPORT TextUndoConnectParas::Redo()
{
    TextPaM aPaM = GetTextEngine()->ConnectContents( mnPara );
    SetSelection( aPaM );
}


TextUndoSplitPara::TextUndoSplitPara( TextEngine* pTextEngine, ULONG nPara, USHORT nPos )
                    : TextUndo( pTextEngine )
{
    mnPara = nPara;
    mnSepPos = nPos;
}

TextUndoSplitPara::~TextUndoSplitPara()
{
}

void __EXPORT TextUndoSplitPara::Undo()
{
    TextPaM aPaM = GetTextEngine()->ConnectContents( mnPara );
    SetSelection( aPaM );
}

void __EXPORT TextUndoSplitPara::Redo()
{
    TextPaM aPaM = GetTextEngine()->SplitContent( mnPara, mnSepPos );
    SetSelection( aPaM );
}


TextUndoInsertChars::TextUndoInsertChars( TextEngine* pTextEngine, const TextPaM& rTextPaM, const XubString& rStr )
                    : TextUndo( pTextEngine ),
                        maTextPaM( rTextPaM ), maText( rStr )
{
}

void __EXPORT TextUndoInsertChars::Undo()
{
    TextSelection aSel( maTextPaM, maTextPaM );
    aSel.GetEnd().GetIndex() = aSel.GetEnd().GetIndex() + maText.Len();
    TextPaM aPaM = GetTextEngine()->ImpDeleteText( aSel );
    SetSelection( aPaM );
}

void __EXPORT TextUndoInsertChars::Redo()
{
    TextSelection aSel( maTextPaM, maTextPaM );
    GetTextEngine()->ImpInsertText( aSel, maText );
    TextPaM aNewPaM( maTextPaM );
    aNewPaM.GetIndex() = aNewPaM.GetIndex() + maText.Len();
    SetSelection( TextSelection( aSel.GetStart(), aNewPaM ) );
}

BOOL __EXPORT TextUndoInsertChars::Merge( SfxUndoAction* pNextAction )
{
    if ( !pNextAction->ISA( TextUndoInsertChars ) )
        return FALSE;

    TextUndoInsertChars* pNext = (TextUndoInsertChars*)pNextAction;

    if ( maTextPaM.GetPara() != pNext->maTextPaM.GetPara() )
        return FALSE;

    if ( ( maTextPaM.GetIndex() + maText.Len() ) == pNext->maTextPaM.GetIndex() )
    {
        maText += pNext->maText;
        return TRUE;
    }
    return FALSE;
}


TextUndoRemoveChars::TextUndoRemoveChars( TextEngine* pTextEngine, const TextPaM& rTextPaM, const XubString& rStr )
                    : TextUndo( pTextEngine ),
                        maTextPaM( rTextPaM ), maText( rStr )
{
}

void __EXPORT TextUndoRemoveChars::Undo()
{
    TextSelection aSel( maTextPaM, maTextPaM );
    GetTextEngine()->ImpInsertText( aSel, maText );
    aSel.GetEnd().GetIndex() = aSel.GetEnd().GetIndex() + maText.Len();
    SetSelection( aSel );
}

void __EXPORT TextUndoRemoveChars::Redo()
{
    TextSelection aSel( maTextPaM, maTextPaM );
    aSel.GetEnd().GetIndex() = aSel.GetEnd().GetIndex() + maText.Len();
    TextPaM aPaM = GetTextEngine()->ImpDeleteText( aSel );
    SetSelection( aPaM );
}


TextUndoSetAttribs::TextUndoSetAttribs( TextEngine* pTextEngine, const TextSelection& rSel )
    : TextUndo( pTextEngine ), maSelection( rSel )
{
    maSelection.Justify();
//  aNewAttribs.Set( rNewItems );
//  mbSetIsRemove = FALSE;
//  mnRemoveWhich = 0;
//  mnSpecial = 0;
}

TextUndoSetAttribs::~TextUndoSetAttribs()
{
    // ...............
}

void __EXPORT TextUndoSetAttribs::Undo()
{
    for ( ULONG nPara = maSelection.GetStart().GetPara(); nPara <= maSelection.GetEnd().GetPara(); nPara++ )
    {
//      ContentAttribsInfo* pInf = aPrevAttribs[ (USHORT)(nPara-aESel.nStartPara) ];
//      GetTextEngine()->RemoveCharAttribs( nPara );
//      TextNode* pNode = GetTextEngine()->GetTextDoc().GetObject( nPara );
//      for ( USHORT nAttr = 0; nAttr < pInf->GetPrevCharAttribs().Count(); nAttr++ )
//      {
//          GetTextEngine()->GetTextDoc().InsertAttrib( pNode, pX->GetStart(), pX->GetEnd(), *pX->GetItem() );
//      }
    }
    SetSelection( maSelection );
}

void __EXPORT TextUndoSetAttribs::Redo()
{
//  if ( !bSetIsRemove )
//      GetTextEngine()->SetAttribs( aSel, aNewAttribs, nSpecial );
//  else
//      GetTextEngine()->RemoveCharAttribs( aSel, bRemoveParaAttribs, nRemoveWhich );
    SetSelection( maSelection );
}
