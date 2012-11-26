/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svtools.hxx"

#include <svtools/texteng.hxx>
#include <svtools/textview.hxx>
#include <textundo.hxx>
#include <textund2.hxx>
#include <svtools/textdata.hxx>
#include <textdoc.hxx>
#include <textdat2.hxx>

TextUndoManager::TextUndoManager( TextEngine* p )
{
    mpTextEngine = p;
}

TextUndoManager::~TextUndoManager()
{
}

sal_Bool __EXPORT TextUndoManager::Undo()
{
    if ( GetUndoActionCount() == 0 )
        return sal_False;

    UndoRedoStart();

    mpTextEngine->SetIsInUndo( sal_True );
    sal_Bool bDone = SfxUndoManager::Undo();
    mpTextEngine->SetIsInUndo( sal_False );

    UndoRedoEnd();

    return bDone;
}

sal_Bool __EXPORT TextUndoManager::Redo()
{
    if ( GetRedoActionCount() == 0 )
        return sal_False;


    UndoRedoStart();

    mpTextEngine->SetIsInUndo( sal_True );
    sal_Bool bDone = SfxUndoManager::Redo();
    mpTextEngine->SetIsInUndo( sal_False );

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


TextUndoDelPara::TextUndoDelPara( TextEngine* pTextEngine, TextNode* pNode, sal_uLong nPara )
                    : TextUndo( pTextEngine )
{
    mpNode = pNode;
    mnPara = nPara;
    mbDelObject = sal_True;
}

TextUndoDelPara::~TextUndoDelPara()
{
    if ( mbDelObject )
        delete mpNode;
}

void __EXPORT TextUndoDelPara::Undo()
{
    GetTextEngine()->InsertContent( mpNode, mnPara );
    mbDelObject = sal_False;    // gehoert wieder der Engine

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

    mbDelObject = sal_True; // gehoert wieder dem Undo

    sal_uLong nParas = GetDoc()->GetNodes().Count();
    sal_uLong n = mnPara < nParas ? mnPara : (nParas-1);
    TextNode* pN = GetDoc()->GetNodes().GetObject( n );
    TextPaM aPaM( n, pN->GetText().Len() );
    SetSelection( aPaM );
}

// -----------------------------------------------------------------------
// TextUndoConnectParas
// ------------------------------------------------------------------------
TextUndoConnectParas::TextUndoConnectParas( TextEngine* pTextEngine, sal_uLong nPara, sal_uInt16 nPos )
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


TextUndoSplitPara::TextUndoSplitPara( TextEngine* pTextEngine, sal_uLong nPara, sal_uInt16 nPos )
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

sal_Bool __EXPORT TextUndoInsertChars::Merge( SfxUndoAction* pNextAction )
{
    if ( !dynamic_cast< TextUndoInsertChars* >(pNextAction) )
        return sal_False;

    TextUndoInsertChars* pNext = (TextUndoInsertChars*)pNextAction;

    if ( maTextPaM.GetPara() != pNext->maTextPaM.GetPara() )
        return sal_False;

    if ( ( maTextPaM.GetIndex() + maText.Len() ) == pNext->maTextPaM.GetIndex() )
    {
        maText += pNext->maText;
        return sal_True;
    }
    return sal_False;
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
//  mbSetIsRemove = sal_False;
//  mnRemoveWhich = 0;
//  mnSpecial = 0;
}

TextUndoSetAttribs::~TextUndoSetAttribs()
{
    // ...............
}

void __EXPORT TextUndoSetAttribs::Undo()
{
    for ( sal_uLong nPara = maSelection.GetStart().GetPara(); nPara <= maSelection.GetEnd().GetPara(); nPara++ )
    {
//      ContentAttribsInfo* pInf = aPrevAttribs[ (sal_uInt16)(nPara-aESel.nStartPara) ];
//      GetTextEngine()->RemoveCharAttribs( nPara );
//      TextNode* pNode = GetTextEngine()->GetTextDoc().GetObject( nPara );
//      for ( sal_uInt16 nAttr = 0; nAttr < pInf->GetPrevCharAttribs().Count(); nAttr++ )
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
