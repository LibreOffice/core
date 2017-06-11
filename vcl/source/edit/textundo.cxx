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

#include "textundo.hxx"
#include "textund2.hxx"
#include "strings.hrc"

#include <vcl/texteng.hxx>
#include <vcl/textview.hxx>
#include <vcl/textdata.hxx>
#include <textdoc.hxx>
#include <textdat2.hxx>
#include <svdata.hxx>
#include <tools/resid.hxx>

namespace
{

// Shorten() -- inserts ellipsis (...) in the middle of a long text
void Shorten (OUString& rString)
{
    unsigned nLen = rString.getLength();
    if (nLen > 48)
    {
        // If possible, we don't break a word, hence first we look for a space.
        // Space before the ellipsis:
        int iFirst = rString.lastIndexOf(' ', 32);
        if (iFirst == -1 || unsigned(iFirst) < 16)
            iFirst = 24; // not possible
        // Space after the ellipsis:
        int iLast = rString.indexOf(' ', nLen - 16);
        if (iLast == -1 || unsigned(iLast) > nLen - 4)
            iLast = nLen - 8; // not possible
        // finally:
        rString =
            rString.copy(0, iFirst + 1) +
            "..." +
            rString.copy(iLast);
    }
}

} // namespace

TextUndoManager::TextUndoManager( TextEngine* p )
{
    mpTextEngine = p;
}

TextUndoManager::~TextUndoManager()
{
}

bool TextUndoManager::Undo()
{
    if ( GetUndoActionCount() == 0 )
        return false;

    UndoRedoStart();

    mpTextEngine->SetIsInUndo( true );
    bool bDone = SfxUndoManager::Undo();
    mpTextEngine->SetIsInUndo( false );

    UndoRedoEnd();

    return bDone;
}

bool TextUndoManager::Redo()
{
    if ( GetRedoActionCount() == 0 )
        return false;

    UndoRedoStart();

    mpTextEngine->SetIsInUndo( true );
    bool bDone = SfxUndoManager::Redo();
    mpTextEngine->SetIsInUndo( false );

    UndoRedoEnd();

    return bDone;
}

void TextUndoManager::UndoRedoStart()
{
    SAL_WARN_IF( !GetView(), "vcl", "Undo/Redo: Active View?" );
}

void TextUndoManager::UndoRedoEnd()
{
    if ( GetView() )
    {
        TextSelection aNewSel( GetView()->GetSelection() );
        aNewSel.GetStart() = aNewSel.GetEnd();
        GetView()->ImpSetSelection( aNewSel );
    }

    mpTextEngine->FormatAndUpdate( GetView() );
}

TextUndo::TextUndo( TextEngine* p )
{
    mpTextEngine = p;
}

TextUndo::~TextUndo()
{
}

OUString TextUndo::GetComment() const
{
    return OUString();
}

void TextUndo::SetSelection( const TextSelection& rSel )
{
    if ( GetView() )
        GetView()->ImpSetSelection( rSel );
}

TextUndoDelPara::TextUndoDelPara( TextEngine* pTextEngine, TextNode* pNode, sal_uInt32 nPara )
                    : TextUndo( pTextEngine )
{
    mpNode = pNode;
    mnPara = nPara;
    mbDelObject = true;
}

TextUndoDelPara::~TextUndoDelPara()
{
    if ( mbDelObject )
        delete mpNode;
}

void TextUndoDelPara::Undo()
{
    GetTextEngine()->InsertContent( mpNode, mnPara );
    mbDelObject = false;    // belongs again to the engine

    if ( GetView() )
    {
        TextSelection aSel( TextPaM( mnPara, 0 ), TextPaM( mnPara, mpNode->GetText().getLength() ) );
        SetSelection( aSel );
    }
}

void TextUndoDelPara::Redo()
{
    // pNode is not valid anymore in case an Undo joined paragraphs
    mpNode = GetDoc()->GetNodes()[ mnPara ];

    delete GetTEParaPortions()->GetObject( mnPara );
    GetTEParaPortions()->Remove( mnPara );

    // do not delete Node because of Undo!
    GetDoc()->GetNodes().erase( ::std::find( GetDoc()->GetNodes().begin(), GetDoc()->GetNodes().end(), mpNode ) );
    GetTextEngine()->ImpParagraphRemoved( mnPara );

    mbDelObject = true; // belongs again to the Undo

    const sal_uInt32 nParas = static_cast<sal_uInt32>(GetDoc()->GetNodes().size());
    const sal_uInt32 n = mnPara < nParas ? mnPara : nParas-1;
    TextNode* pN = GetDoc()->GetNodes()[ n ];
    TextPaM aPaM( n, pN->GetText().getLength() );
    SetSelection( aPaM );
}

OUString TextUndoDelPara::GetComment () const
{
    return VclResId(STR_TEXTUNDO_DELPARA);
}

TextUndoConnectParas::TextUndoConnectParas( TextEngine* pTextEngine, sal_uInt32 nPara, sal_Int32 nPos )
                    :   TextUndo( pTextEngine )
{
    mnPara = nPara;
    mnSepPos = nPos;
}

TextUndoConnectParas::~TextUndoConnectParas()
{
}

void TextUndoConnectParas::Undo()
{
    TextPaM aPaM = GetTextEngine()->SplitContent( mnPara, mnSepPos );
    SetSelection( aPaM );
}

void TextUndoConnectParas::Redo()
{
    TextPaM aPaM = GetTextEngine()->ConnectContents( mnPara );
    SetSelection( aPaM );
}

OUString TextUndoConnectParas::GetComment () const
{
    return VclResId(STR_TEXTUNDO_CONNECTPARAS);
}

TextUndoSplitPara::TextUndoSplitPara( TextEngine* pTextEngine, sal_uInt32 nPara, sal_Int32 nPos )
                    : TextUndo( pTextEngine )
{
    mnPara = nPara;
    mnSepPos = nPos;
}

TextUndoSplitPara::~TextUndoSplitPara()
{
}

void TextUndoSplitPara::Undo()
{
    TextPaM aPaM = GetTextEngine()->ConnectContents( mnPara );
    SetSelection( aPaM );
}

void TextUndoSplitPara::Redo()
{
    TextPaM aPaM = GetTextEngine()->SplitContent( mnPara, mnSepPos );
    SetSelection( aPaM );
}

OUString TextUndoSplitPara::GetComment () const
{
    return VclResId(STR_TEXTUNDO_SPLITPARA);
}

TextUndoInsertChars::TextUndoInsertChars( TextEngine* pTextEngine, const TextPaM& rTextPaM, const OUString& rStr )
                    : TextUndo( pTextEngine ),
                        maTextPaM( rTextPaM ), maText( rStr )
{
}

void TextUndoInsertChars::Undo()
{
    TextSelection aSel( maTextPaM, maTextPaM );
    aSel.GetEnd().GetIndex() += maText.getLength();
    TextPaM aPaM = GetTextEngine()->ImpDeleteText( aSel );
    SetSelection( aPaM );
}

void TextUndoInsertChars::Redo()
{
    TextSelection aSel( maTextPaM, maTextPaM );
    GetTextEngine()->ImpInsertText( aSel, maText );
    TextPaM aNewPaM( maTextPaM );
    aNewPaM.GetIndex() += maText.getLength();
    SetSelection( TextSelection( aSel.GetStart(), aNewPaM ) );
}

bool TextUndoInsertChars::Merge( SfxUndoAction* pNextAction )
{
    if ( nullptr == dynamic_cast< const TextUndoInsertChars*>( pNextAction ) )
        return false;

    TextUndoInsertChars* pNext = static_cast<TextUndoInsertChars*>(pNextAction);

    if ( maTextPaM.GetPara() != pNext->maTextPaM.GetPara() )
        return false;

    if ( ( maTextPaM.GetIndex() + maText.getLength() ) == pNext->maTextPaM.GetIndex() )
    {
        maText += pNext->maText;
        return true;
    }
    return false;
}

OUString TextUndoInsertChars::GetComment () const
{
    // multiple lines?
    OUString sText(maText);
    Shorten(sText);
    return VclResId(STR_TEXTUNDO_INSERTCHARS).replaceAll("$1", sText);
}

TextUndoRemoveChars::TextUndoRemoveChars( TextEngine* pTextEngine, const TextPaM& rTextPaM, const OUString& rStr )
                    : TextUndo( pTextEngine ),
                        maTextPaM( rTextPaM ), maText( rStr )
{
}

void TextUndoRemoveChars::Undo()
{
    TextSelection aSel( maTextPaM, maTextPaM );
    GetTextEngine()->ImpInsertText( aSel, maText );
    aSel.GetEnd().GetIndex() += maText.getLength();
    SetSelection( aSel );
}

void TextUndoRemoveChars::Redo()
{
    TextSelection aSel( maTextPaM, maTextPaM );
    aSel.GetEnd().GetIndex() += maText.getLength();
    TextPaM aPaM = GetTextEngine()->ImpDeleteText( aSel );
    SetSelection( aPaM );
}

OUString TextUndoRemoveChars::GetComment () const
{
    // multiple lines?
    OUString sText(maText);
    Shorten(sText);
    return VclResId(STR_TEXTUNDO_REMOVECHARS).replaceAll("$1", sText);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
