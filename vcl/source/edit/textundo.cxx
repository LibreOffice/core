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

#include "textundo.hxx"
#include "textund2.hxx"
#include "textundo.hrc"

#include <vcl/texteng.hxx>
#include <vcl/textview.hxx>
#include <vcl/textdata.hxx>
#include <textdoc.hxx>
#include <textdat2.hxx>
#include <svdata.hxx> // ImplGetResMgr()
#include <tools/resid.hxx>

TYPEINIT1( TextUndo, SfxUndoAction );
TYPEINIT1( TextUndoDelPara, TextUndo );
TYPEINIT1( TextUndoConnectParas, TextUndo );
TYPEINIT1( TextUndoSplitPara, TextUndo );
TYPEINIT1( TextUndoInsertChars, TextUndo );
TYPEINIT1( TextUndoRemoveChars, TextUndo );


namespace
{

// Shorten() -- inserts ellipsis (...) in the middle of a long text
void Shorten (rtl::OUString& rString)
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

//
// TextUndoManager
// ===============
//

TextUndoManager::TextUndoManager( TextEngine* p )
{
    mpTextEngine = p;
}

TextUndoManager::~TextUndoManager()
{
}

sal_Bool TextUndoManager::Undo()
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

sal_Bool TextUndoManager::Redo()
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


//
// TextUndo
// ========
//

TextUndo::TextUndo( TextEngine* p )
{
    mpTextEngine = p;
}

TextUndo::~TextUndo()
{
}

rtl::OUString TextUndo::GetComment() const
{
    return rtl::OUString();
}

void TextUndo::SetSelection( const TextSelection& rSel )
{
    if ( GetView() )
        GetView()->ImpSetSelection( rSel );
}


//
// TextUndoDelPara
// ===============
//

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

void TextUndoDelPara::Undo()
{
    GetTextEngine()->InsertContent( mpNode, mnPara );
    mbDelObject = sal_False;    // gehoert wieder der Engine

    if ( GetView() )
    {
        TextSelection aSel( TextPaM( mnPara, 0 ), TextPaM( mnPara, mpNode->GetText().Len() ) );
        SetSelection( aSel );
    }
}

void TextUndoDelPara::Redo()
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

rtl::OUString TextUndoDelPara::GetComment () const
{
    return ResId(STR_TEXTUNDO_DELPARA, *ImplGetResMgr());
}


//
// TextUndoConnectParas
// ====================
//

TextUndoConnectParas::TextUndoConnectParas( TextEngine* pTextEngine, sal_uLong nPara, sal_uInt16 nPos )
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

rtl::OUString TextUndoConnectParas::GetComment () const
{
    return ResId(STR_TEXTUNDO_CONNECTPARAS, *ImplGetResMgr());
}


//
// TextUndoSplitPara
// =================
//

TextUndoSplitPara::TextUndoSplitPara( TextEngine* pTextEngine, sal_uLong nPara, sal_uInt16 nPos )
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

rtl::OUString TextUndoSplitPara::GetComment () const
{
    return ResId(STR_TEXTUNDO_SPLITPARA, *ImplGetResMgr());
}


//
// TextUndoInsertChars
// ===================
//

TextUndoInsertChars::TextUndoInsertChars( TextEngine* pTextEngine, const TextPaM& rTextPaM, const XubString& rStr )
                    : TextUndo( pTextEngine ),
                        maTextPaM( rTextPaM ), maText( rStr )
{
}

void TextUndoInsertChars::Undo()
{
    TextSelection aSel( maTextPaM, maTextPaM );
    aSel.GetEnd().GetIndex() = aSel.GetEnd().GetIndex() + maText.Len();
    TextPaM aPaM = GetTextEngine()->ImpDeleteText( aSel );
    SetSelection( aPaM );
}

void TextUndoInsertChars::Redo()
{
    TextSelection aSel( maTextPaM, maTextPaM );
    GetTextEngine()->ImpInsertText( aSel, maText );
    TextPaM aNewPaM( maTextPaM );
    aNewPaM.GetIndex() = aNewPaM.GetIndex() + maText.Len();
    SetSelection( TextSelection( aSel.GetStart(), aNewPaM ) );
}

sal_Bool TextUndoInsertChars::Merge( SfxUndoAction* pNextAction )
{
    if ( !pNextAction->ISA( TextUndoInsertChars ) )
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

rtl::OUString TextUndoInsertChars::GetComment () const
{
    // multiple lines?
    rtl::OUString sText(maText);
    Shorten(sText);
    return rtl::OUString(ResId(STR_TEXTUNDO_INSERTCHARS, *ImplGetResMgr())).replaceAll("$1", sText);
}



//
// TextUndoRemoveChars
// ===================
//

TextUndoRemoveChars::TextUndoRemoveChars( TextEngine* pTextEngine, const TextPaM& rTextPaM, const XubString& rStr )
                    : TextUndo( pTextEngine ),
                        maTextPaM( rTextPaM ), maText( rStr )
{
}

void TextUndoRemoveChars::Undo()
{
    TextSelection aSel( maTextPaM, maTextPaM );
    GetTextEngine()->ImpInsertText( aSel, maText );
    aSel.GetEnd().GetIndex() = aSel.GetEnd().GetIndex() + maText.Len();
    SetSelection( aSel );
}

void TextUndoRemoveChars::Redo()
{
    TextSelection aSel( maTextPaM, maTextPaM );
    aSel.GetEnd().GetIndex() = aSel.GetEnd().GetIndex() + maText.Len();
    TextPaM aPaM = GetTextEngine()->ImpDeleteText( aSel );
    SetSelection( aPaM );
}

rtl::OUString TextUndoRemoveChars::GetComment () const
{
    // multiple lines?
    rtl::OUString sText(maText);
    Shorten(sText);
    return rtl::OUString(ResId(STR_TEXTUNDO_REMOVECHARS, *ImplGetResMgr())).replaceAll("$1", sText);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
