/*************************************************************************
 *
 *  $RCSfile: textundo.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:58:58 $
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

#include <texteng.hxx>
#include <textview.hxx>
#include <textundo.hxx>
#include <textund2.hxx>
#include <textdata.hxx>
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

BOOL __EXPORT TextUndoManager::Undo( USHORT nCount )
{
    if ( GetUndoActionCount() == 0 )
        return FALSE;

    UndoRedoStart();

    mpTextEngine->SetIsInUndo( TRUE );
    BOOL bDone = SfxUndoManager::Undo( nCount );
    mpTextEngine->SetIsInUndo( FALSE );

    UndoRedoEnd();

    return bDone;
}

BOOL __EXPORT TextUndoManager::Redo( USHORT nCount )
{
    if ( GetRedoActionCount() == 0 )
        return FALSE;


    UndoRedoStart();

    mpTextEngine->SetIsInUndo( TRUE );
    BOOL bDone = SfxUndoManager::Redo( nCount );
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
        GetView()->maSelection = aNewSel;
    }

    mpTextEngine->UpdateSelections();

    mpTextEngine->FormatAndUpdate( GetView() );
}


TextUndo::TextUndo( USHORT nI, TextEngine* p )
{
    mnId = nI;
    mpTextEngine = p;
}

TextUndo::~TextUndo()
{
}

USHORT __EXPORT TextUndo::GetId() const
{
    //nId sollte mal entfallen => GetId ueberall ueberladen...
    return mnId;
}

XubString __EXPORT TextUndo::GetComment() const
{
//  return mpTextEngine->GetUndoComment( this );
    return String();
}

void TextUndo::SetSelection( const TextSelection& rSel )
{
    if ( GetView() )
        GetView()->maSelection = rSel;
}


TextUndoDelPara::TextUndoDelPara( TextEngine* pTextEngine, TextNode* pNode, ULONG nPara )
                    : TextUndo( TEXTUNDO_DELCONTENT, pTextEngine )
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
                    :   TextUndo( TEXTUNDO_CONNECTPARAS, pTextEngine )
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
                    : TextUndo( TEXTUNDO_SPLITPARA, pTextEngine )
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
                    : TextUndo( TEXTUNDO_INSERTCHARS, pTextEngine ),
                        maTextPaM( rTextPaM ), maText( rStr )
{
}

void __EXPORT TextUndoInsertChars::Undo()
{
    TextSelection aSel( maTextPaM, maTextPaM );
    aSel.GetEnd().GetIndex() += maText.Len();
    TextPaM aPaM = GetTextEngine()->ImpDeleteText( aSel );
    SetSelection( aPaM );
}

void __EXPORT TextUndoInsertChars::Redo()
{
    TextSelection aSel( maTextPaM, maTextPaM );
    GetTextEngine()->ImpInsertText( aSel, maText );
    TextPaM aNewPaM( maTextPaM );
    aNewPaM.GetIndex() += maText.Len();
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
                    : TextUndo( TEXTUNDO_REMOVECHARS, pTextEngine ),
                        maTextPaM( rTextPaM ), maText( rStr )
{
}

void __EXPORT TextUndoRemoveChars::Undo()
{
    TextSelection aSel( maTextPaM, maTextPaM );
    GetTextEngine()->ImpInsertText( aSel, maText );
    aSel.GetEnd().GetIndex() += maText.Len();
    SetSelection( aSel );
}

void __EXPORT TextUndoRemoveChars::Redo()
{
    TextSelection aSel( maTextPaM, maTextPaM );
    aSel.GetEnd().GetIndex() += maText.Len();
    TextPaM aPaM = GetTextEngine()->ImpDeleteText( aSel );
    SetSelection( aPaM );
}


TextUndoSetAttribs::TextUndoSetAttribs( TextEngine* pTextEngine, const TextSelection& rSel )
    : TextUndo( TEXTUNDO_ATTRIBS, pTextEngine ), maSelection( rSel )
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
