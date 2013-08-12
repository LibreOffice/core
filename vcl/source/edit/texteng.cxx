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

#include <tools/stream.hxx>

#include <vcl/texteng.hxx>
#include <vcl/textview.hxx>
#include <textdoc.hxx>
#include <textdat2.hxx>
#include <textundo.hxx>
#include <textund2.hxx>
#include <svl/ctloptions.hxx>
#include <vcl/window.hxx>

#include <vcl/edit.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/PropertyValues.hpp>

#include <com/sun/star/i18n/XBreakIterator.hpp>

#include <com/sun/star/i18n/CharacterIteratorMode.hpp>

#include <com/sun/star/i18n/WordType.hpp>

#include <com/sun/star/i18n/InputSequenceChecker.hpp>
#include <com/sun/star/i18n/InputSequenceCheckMode.hpp>
#include <com/sun/star/i18n/ScriptType.hpp>

#include <comphelper/processfactory.hxx>

#include <unotools/localedatawrapper.hxx>
#include <vcl/unohelp.hxx>

#include <vcl/svapp.hxx>
#include <vcl/metric.hxx>

#include <unicode/ubidi.h>

#include <set>
#include <vector>
#include <boost/foreach.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::rtl;


TextEngine::TextEngine()
{
    mpDoc = 0;
    mpTEParaPortions = 0;

    mpViews = new TextViews;
    mpActiveView = NULL;

    mbIsFormatting      = sal_False;
    mbFormatted         = sal_False;
    mbUpdate            = sal_True;
    mbModified          = sal_False;
    mbUndoEnabled       = sal_False;
    mbIsInUndo          = sal_False;
    mbDowning           = sal_False;
    mbRightToLeft       = sal_False;
    mbHasMultiLineParas = sal_False;

    meAlign         = TXTALIGN_LEFT;

    mnMaxTextWidth  = 0;
    mnMaxTextLen    = 0;
    mnCurTextWidth  = 0xFFFFFFFF;
    mnCurTextHeight = 0;

    mpUndoManager   = NULL;
       mpIMEInfos       = NULL;
    mpLocaleDataWrapper = NULL;

    mpIdleFormatter = new IdleFormatter;
    mpIdleFormatter->SetTimeoutHdl( LINK( this, TextEngine, IdleFormatHdl ) );

    mpRefDev = new VirtualDevice;

    ImpInitLayoutMode( mpRefDev );

    ImpInitDoc();

    maTextColor = COL_BLACK;
    Font aFont;
    aFont.SetTransparent( sal_False );
    Color aFillColor( aFont.GetFillColor() );
    aFillColor.SetTransparency( 0 );
    aFont.SetFillColor( aFillColor );
    SetFont( aFont );
}

TextEngine::~TextEngine()
{
    mbDowning = sal_True;

    delete mpIdleFormatter;
    delete mpDoc;
    delete mpTEParaPortions;
    delete mpViews; // only the list, not the Views
    delete mpRefDev;
    delete mpUndoManager;
    delete mpIMEInfos;
    delete mpLocaleDataWrapper;
}

void TextEngine::InsertView( TextView* pTextView )
{
    mpViews->push_back( pTextView );
    pTextView->SetSelection( TextSelection() );

    if ( !GetActiveView() )
        SetActiveView( pTextView );
}

void TextEngine::RemoveView( TextView* pTextView )
{
    TextViews::iterator it = std::find( mpViews->begin(), mpViews->end(), pTextView );
    if( it != mpViews->end() )
    {
        pTextView->HideCursor();
        mpViews->erase( it );
        if ( pTextView == GetActiveView() )
            SetActiveView( 0 );
    }
}

sal_uInt16 TextEngine::GetViewCount() const
{
    return mpViews->size();
}

TextView* TextEngine::GetView( sal_uInt16 nView ) const
{
    return (*mpViews)[ nView ];
}

TextView* TextEngine::GetActiveView() const
{
    return mpActiveView;
}

void TextEngine::SetActiveView( TextView* pTextView )
{
    if ( pTextView != mpActiveView )
    {
        if ( mpActiveView )
            mpActiveView->HideSelection();

        mpActiveView = pTextView;

        if ( mpActiveView )
            mpActiveView->ShowSelection();
    }
}

void TextEngine::SetFont( const Font& rFont )
{
    if ( rFont != maFont )
    {
        maFont = rFont;
        // #i40221# As the font's color now defaults to transparent (since i35764)
        //  we have to choose a useful textcolor in this case.
        // Otherwise maTextColor and maFont.GetColor() are both transparent....
        if( rFont.GetColor() == COL_TRANSPARENT )
            maTextColor = COL_BLACK;
        else
            maTextColor = rFont.GetColor();

        // Do not allow transparent fonts because of selection
        // (otherwise delete the background in ImplPaint later differently)
        maFont.SetTransparent( sal_False );
        // Tell VCL not to use the font color, use text color from OutputDevice
        maFont.SetColor( COL_TRANSPARENT );
        Color aFillColor( maFont.GetFillColor() );
        aFillColor.SetTransparency( 0 );
        maFont.SetFillColor( aFillColor );

        maFont.SetAlign( ALIGN_TOP );
        mpRefDev->SetFont( maFont);
        Size aTextSize;
        aTextSize.Width() = mpRefDev->GetTextWidth(OUString("    "));
        aTextSize.Height() = mpRefDev->GetTextHeight();
        if ( !aTextSize.Width() )
            aTextSize.Width() = mpRefDev->GetTextWidth(OUString("XXXX"));

        mnDefTab = (sal_uInt16)aTextSize.Width();
        if ( !mnDefTab )
            mnDefTab = 1;
        mnCharHeight = (sal_uInt16)aTextSize.Height();
        mnFixCharWidth100 = 0;

        FormatFullDoc();
        UpdateViews();

        for ( sal_uInt16 nView = mpViews->size(); nView; )
        {
            TextView* pView = (*mpViews)[ --nView ];
            pView->GetWindow()->SetInputContext( InputContext( GetFont(), !pView->IsReadOnly() ? INPUTCONTEXT_TEXT|INPUTCONTEXT_EXTTEXTINPUT : 0 ) );
        }
    }
}

void TextEngine::SetMaxTextLen( sal_uLong nLen )
{
    mnMaxTextLen = nLen;
}

void TextEngine::SetMaxTextWidth( sal_uLong nMaxWidth )
{
    if ( nMaxWidth != mnMaxTextWidth )
    {
        mnMaxTextWidth = std::min( nMaxWidth, (sal_uLong)0x7FFFFFFF );
        FormatFullDoc();
        UpdateViews();
    }
}

static sal_Unicode static_aLFText[] = { '\n', 0 };
static sal_Unicode static_aCRText[] = { '\r', 0 };
static sal_Unicode static_aCRLFText[] = { '\r', '\n', 0 };

static inline const sal_Unicode* static_getLineEndText( LineEnd aLineEnd )
{
    const sal_Unicode* pRet = NULL;

    switch( aLineEnd )
    {
    case LINEEND_LF: pRet = static_aLFText;break;
    case LINEEND_CR: pRet = static_aCRText;break;
    case LINEEND_CRLF: pRet = static_aCRLFText;break;
    }
    return pRet;
}

void  TextEngine::ReplaceText(const TextSelection& rSel, const OUString& rText)
{
    ImpInsertText( rSel, rText );
}

OUString TextEngine::GetText( LineEnd aSeparator ) const
{
    return mpDoc->GetText( static_getLineEndText( aSeparator ) );
}

OUString TextEngine::GetTextLines( LineEnd aSeparator ) const
{
    OUString aText;
    sal_uLong nParas = mpTEParaPortions->Count();
    const sal_Unicode* pSep = static_getLineEndText( aSeparator );
    for ( sal_uLong nP = 0; nP < nParas; nP++ )
    {
        TEParaPortion* pTEParaPortion = mpTEParaPortions->GetObject( nP );

        sal_uInt16 nLines = pTEParaPortion->GetLines().size();
        for ( sal_uInt16 nL = 0; nL < nLines; nL++ )
        {
            TextLine* pLine = pTEParaPortion->GetLines()[nL];
            aText += pTEParaPortion->GetNode()->GetText().copy( pLine->GetStart(), pLine->GetEnd() - pLine->GetStart() );
            if ( pSep && ( ( (nP+1) < nParas ) || ( (nL+1) < nLines ) ) )
                aText += pSep;
        }
    }
    return aText;
}

OUString TextEngine::GetText( sal_uLong nPara ) const
{
    return mpDoc->GetText( nPara );
}

sal_uLong TextEngine::GetTextLen( LineEnd aSeparator ) const
{
    return mpDoc->GetTextLen( static_getLineEndText( aSeparator ) );
}

sal_uLong TextEngine::GetTextLen( const TextSelection& rSel, LineEnd aSeparator ) const
{
    TextSelection aSel( rSel );
    aSel.Justify();
    ValidateSelection( aSel );
    return mpDoc->GetTextLen( static_getLineEndText( aSeparator ), &aSel );
}

sal_uInt16 TextEngine::GetTextLen( sal_uLong nPara ) const
{
    return mpDoc->GetNodes().GetObject( nPara )->GetText().getLength();
}

void TextEngine::SetUpdateMode( sal_Bool bUpdate )
{
    if ( bUpdate != mbUpdate )
    {
        mbUpdate = bUpdate;
        if ( mbUpdate )
        {
            FormatAndUpdate( GetActiveView() );
            if ( GetActiveView() )
                GetActiveView()->ShowCursor();
        }
    }
}

sal_Bool TextEngine::DoesKeyChangeText( const KeyEvent& rKeyEvent )
{
    sal_Bool bDoesChange = sal_False;

    KeyFuncType eFunc = rKeyEvent.GetKeyCode().GetFunction();
    if ( eFunc != KEYFUNC_DONTKNOW )
    {
        switch ( eFunc )
        {
            case KEYFUNC_UNDO:
            case KEYFUNC_REDO:
            case KEYFUNC_CUT:
            case KEYFUNC_PASTE: bDoesChange = sal_True;
            break;
            default:    // might get handled below
                        eFunc = KEYFUNC_DONTKNOW;
        }
    }
    if ( eFunc == KEYFUNC_DONTKNOW )
    {
        switch ( rKeyEvent.GetKeyCode().GetCode() )
        {
            case KEY_DELETE:
            case KEY_BACKSPACE:
            {
                if ( !rKeyEvent.GetKeyCode().IsMod2() )
                    bDoesChange = sal_True;
            }
            break;
            case KEY_RETURN:
            case KEY_TAB:
            {
                if ( !rKeyEvent.GetKeyCode().IsMod1() && !rKeyEvent.GetKeyCode().IsMod2() )
                    bDoesChange = sal_True;
            }
            break;
            default:
            {
                bDoesChange = TextEngine::IsSimpleCharInput( rKeyEvent );
            }
        }
    }
    return bDoesChange;
}

sal_Bool TextEngine::IsSimpleCharInput( const KeyEvent& rKeyEvent )
{
    if( rKeyEvent.GetCharCode() >= 32 && rKeyEvent.GetCharCode() != 127 &&
        KEY_MOD1 != (rKeyEvent.GetKeyCode().GetModifier() & ~KEY_SHIFT) && // (ssa) #i45714#:
        KEY_MOD2 != (rKeyEvent.GetKeyCode().GetModifier() & ~KEY_SHIFT) )  // check for Ctrl and Alt separately
    {
        return sal_True;
    }
    return sal_False;
}

void TextEngine::ImpInitDoc()
{
    if ( mpDoc )
        mpDoc->Clear();
    else
        mpDoc = new TextDoc;

    delete mpTEParaPortions;
    mpTEParaPortions = new TEParaPortions;

    TextNode* pNode = new TextNode( OUString() );
    mpDoc->GetNodes().Insert( pNode, 0 );

    TEParaPortion* pIniPortion = new TEParaPortion( pNode );
    mpTEParaPortions->Insert( pIniPortion, (sal_uLong)0 );

    mbFormatted = sal_False;

    ImpParagraphRemoved( TEXT_PARA_ALL );
    ImpParagraphInserted( 0 );
}

OUString TextEngine::GetText( const TextSelection& rSel, LineEnd aSeparator ) const
{
    OUString aText;

    if ( !rSel.HasRange() )
        return aText;

    TextSelection aSel( rSel );
    aSel.Justify();

    sal_uLong nStartPara = aSel.GetStart().GetPara();
    sal_uLong nEndPara = aSel.GetEnd().GetPara();
    const sal_Unicode* pSep = static_getLineEndText( aSeparator );
    for ( sal_uLong nNode = aSel.GetStart().GetPara(); nNode <= nEndPara; nNode++ )
    {
        TextNode* pNode = mpDoc->GetNodes().GetObject( nNode );

        sal_uInt16 nStartPos = 0;
        sal_Int32 nEndPos = pNode->GetText().getLength();
        if ( nNode == nStartPara )
            nStartPos = aSel.GetStart().GetIndex();
        if ( nNode == nEndPara ) // may also be == nStart!
            nEndPos = aSel.GetEnd().GetIndex();

        aText += pNode->GetText().copy( nStartPos, nEndPos-nStartPos );
        if ( nNode < nEndPara )
            aText += pSep;
    }
    return aText;
}

void TextEngine::ImpRemoveText()
{
    ImpInitDoc();

    TextPaM aStartPaM( 0, 0 );
    TextSelection aEmptySel( aStartPaM, aStartPaM );
    for ( sal_uInt16 nView = 0; nView < mpViews->size(); nView++ )
    {
        TextView* pView = (*mpViews)[ nView ];
        pView->ImpSetSelection( aEmptySel );
    }
    ResetUndo();
}

void TextEngine::SetText( const OUString& rText )
{
    ImpRemoveText();

    sal_Bool bUndoCurrentlyEnabled = IsUndoEnabled();
    // the manually inserted text cannot be reversed by the user
    EnableUndo( sal_False );

    TextPaM aStartPaM( 0, 0 );
    TextSelection aEmptySel( aStartPaM, aStartPaM );

    TextPaM aPaM = aStartPaM;
    if ( !rText.isEmpty() )
        aPaM = ImpInsertText( aEmptySel, rText );

    for ( sal_uInt16 nView = 0; nView < mpViews->size(); nView++ )
    {
        TextView* pView = (*mpViews)[ nView ];
        pView->ImpSetSelection( aEmptySel );

        // if no text, then no Format&Update => the text remains
        if ( rText.isEmpty() && GetUpdateMode() )
            pView->Invalidate();
    }

    if( rText.isEmpty() )  // otherwise needs invalidation later; !bFormatted is sufficient
        mnCurTextHeight = 0;

    FormatAndUpdate();

    EnableUndo( bUndoCurrentlyEnabled );
    DBG_ASSERT( !HasUndoManager() || !GetUndoManager().GetUndoActionCount(), "SetText: Undo!" );
}


void TextEngine::CursorMoved( sal_uLong nNode )
{
    // delete empty attribute; but only if paragraph is not empty!
    TextNode* pNode = mpDoc->GetNodes().GetObject( nNode );
    if ( pNode && pNode->GetCharAttribs().HasEmptyAttribs() && !pNode->GetText().isEmpty() )
        pNode->GetCharAttribs().DeleteEmptyAttribs();
}

void TextEngine::ImpRemoveChars( const TextPaM& rPaM, sal_uInt16 nChars, SfxUndoAction* )
{
    DBG_ASSERT( nChars, "ImpRemoveChars: 0 Chars?!" );
    if ( IsUndoEnabled() && !IsInUndo() )
    {
        // attributes have to be saved for UNDO before RemoveChars!
        TextNode* pNode = mpDoc->GetNodes().GetObject( rPaM.GetPara() );
        OUString aStr( pNode->GetText().copy( rPaM.GetIndex(), nChars ) );

        // check if attributes are being deleted or changed
        sal_uInt16 nStart = rPaM.GetIndex();
        sal_uInt16 nEnd = nStart + nChars;
        for ( sal_uInt16 nAttr = pNode->GetCharAttribs().Count(); nAttr; )
        {
            TextCharAttrib* pAttr = pNode->GetCharAttribs().GetAttrib( --nAttr );
            if ( ( pAttr->GetEnd() >= nStart ) && ( pAttr->GetStart() < nEnd ) )
            {
                break;  // for
            }
        }
            InsertUndo( new TextUndoRemoveChars( this, rPaM, aStr ) );
    }

    mpDoc->RemoveChars( rPaM, nChars );
    ImpCharsRemoved( rPaM.GetPara(), rPaM.GetIndex(), nChars );
}

TextPaM TextEngine::ImpConnectParagraphs( sal_uLong nLeft, sal_uLong nRight )
{
    DBG_ASSERT( nLeft != nRight, "ImpConnectParagraphs: connect the very same paragraph ?" );

    TextNode* pLeft = mpDoc->GetNodes().GetObject( nLeft );
    TextNode* pRight = mpDoc->GetNodes().GetObject( nRight );

    if ( IsUndoEnabled() && !IsInUndo() )
        InsertUndo( new TextUndoConnectParas( this, nLeft, pLeft->GetText().getLength() ) );

    // first lookup Portions, as pRight is gone after ConnectParagraphs
    TEParaPortion* pLeftPortion = mpTEParaPortions->GetObject( nLeft );
    TEParaPortion* pRightPortion = mpTEParaPortions->GetObject( nRight );
    DBG_ASSERT( pLeft && pLeftPortion, "ImpConnectParagraphs(1): Hidden Portion" );
    DBG_ASSERT( pRight && pRightPortion, "ImpConnectParagraphs(2): Hidden Portion" );

    TextPaM aPaM = mpDoc->ConnectParagraphs( pLeft, pRight );
    ImpParagraphRemoved( nRight );

    pLeftPortion->MarkSelectionInvalid( aPaM.GetIndex(), pLeft->GetText().getLength() );

    mpTEParaPortions->Remove( nRight );
    delete pRightPortion;
    // the right Node is deleted by EditDoc::ConnectParagraphs()

    return aPaM;
}

TextPaM TextEngine::ImpDeleteText( const TextSelection& rSel )
{
    if ( !rSel.HasRange() )
        return rSel.GetStart();

    TextSelection aSel( rSel );
    aSel.Justify();
    TextPaM aStartPaM( aSel.GetStart() );
    TextPaM aEndPaM( aSel.GetEnd() );

    CursorMoved( aStartPaM.GetPara() ); // so that newly-adjusted attributes vanish
    CursorMoved( aEndPaM.GetPara() );   // so that newly-adjusted attributes vanish

    DBG_ASSERT( mpDoc->IsValidPaM( aStartPaM ), "ImpDeleteText(1): bad Index" );
    DBG_ASSERT( mpDoc->IsValidPaM( aEndPaM ), "ImpDeleteText(2): bad Index" );

    sal_uLong nStartNode = aStartPaM.GetPara();
    sal_uLong nEndNode = aEndPaM.GetPara();

    // remove all Nodes inbetween
    for ( sal_uLong z = nStartNode+1; z < nEndNode; z++ )
    {
        // always nStartNode+1, because of Remove()!
        ImpRemoveParagraph( nStartNode+1 );
    }

    if ( nStartNode != nEndNode )
    {
        // the remainder of StartNodes...
        TextNode* pLeft = mpDoc->GetNodes().GetObject( nStartNode );
        sal_Int32 nChars = pLeft->GetText().getLength() - aStartPaM.GetIndex();
        if ( nChars )
        {
            ImpRemoveChars( aStartPaM, nChars );
            TEParaPortion* pPortion = mpTEParaPortions->GetObject( nStartNode );
            DBG_ASSERT( pPortion, "ImpDeleteText(3): bad Index" );
            pPortion->MarkSelectionInvalid( aStartPaM.GetIndex(), pLeft->GetText().getLength() );
        }

        // the beginning of EndNodes....
        nEndNode = nStartNode+1;    // the other paragraphs were deleted
        nChars = aEndPaM.GetIndex();
        if ( nChars )
        {
            aEndPaM.GetPara() = nEndNode;
            aEndPaM.GetIndex() = 0;
            ImpRemoveChars( aEndPaM, nChars );
            TEParaPortion* pPortion = mpTEParaPortions->GetObject( nEndNode );
            DBG_ASSERT( pPortion, "ImpDeleteText(4): bad Index" );
            pPortion->MarkSelectionInvalid( 0, pPortion->GetNode()->GetText().getLength() );
        }

        // connect....
        aStartPaM = ImpConnectParagraphs( nStartNode, nEndNode );
    }
    else
    {
        sal_uInt16 nChars;
        nChars = aEndPaM.GetIndex() - aStartPaM.GetIndex();
        ImpRemoveChars( aStartPaM, nChars );
        TEParaPortion* pPortion = mpTEParaPortions->GetObject( nStartNode );
        DBG_ASSERT( pPortion, "ImpDeleteText(5): bad Index" );
        pPortion->MarkInvalid( aEndPaM.GetIndex(), aStartPaM.GetIndex() - aEndPaM.GetIndex() );
    }

//  UpdateSelections();
    TextModified();
    return aStartPaM;
}

void TextEngine::ImpRemoveParagraph( sal_uLong nPara )
{
    TextNode* pNode = mpDoc->GetNodes().GetObject( nPara );
    TEParaPortion* pPortion = mpTEParaPortions->GetObject( nPara );

    // the Node is handled by Undo and is deleted if appropriate
    mpDoc->GetNodes().Remove( nPara );
    if ( IsUndoEnabled() && !IsInUndo() )
        InsertUndo( new TextUndoDelPara( this, pNode, nPara ) );
    else
        delete pNode;

    mpTEParaPortions->Remove( nPara );
    delete pPortion;

    ImpParagraphRemoved( nPara );
}

uno::Reference < i18n::XExtendedInputSequenceChecker > TextEngine::GetInputSequenceChecker() const
{
    uno::Reference < i18n::XExtendedInputSequenceChecker > xISC;
//    if ( !xISC.is() )
    {
        uno::Reference< uno::XComponentContext > xContext = ::comphelper::getProcessComponentContext();
        xISC = i18n::InputSequenceChecker::create(xContext);
    }
    return xISC;
}

sal_Bool TextEngine::IsInputSequenceCheckingRequired( sal_Unicode c, const TextSelection& rCurSel ) const
{
    uno::Reference< i18n::XBreakIterator > xBI = ((TextEngine *) this)->GetBreakIterator();
    SvtCTLOptions aCTLOptions;

    // get the index that really is first
    sal_uInt16 nFirstPos = rCurSel.GetStart().GetIndex();
    sal_uInt16 nMaxPos   = rCurSel.GetEnd().GetIndex();
    if (nMaxPos < nFirstPos)
        nFirstPos = nMaxPos;

    sal_Bool bIsSequenceChecking =
        aCTLOptions.IsCTLFontEnabled() &&
        aCTLOptions.IsCTLSequenceChecking() &&
        nFirstPos != 0 && /* first char needs not to be checked */
        xBI.is() && i18n::ScriptType::COMPLEX == xBI->getScriptType( OUString( c ), 0 );

    return bIsSequenceChecking;
}

TextPaM TextEngine::ImpInsertText( const TextSelection& rCurSel, sal_Unicode c, sal_Bool bOverwrite )
{
    return ImpInsertText( c, rCurSel, bOverwrite, sal_False );
}

TextPaM TextEngine::ImpInsertText( sal_Unicode c, const TextSelection& rCurSel, sal_Bool bOverwrite, sal_Bool bIsUserInput )
{
    DBG_ASSERT( c != '\n', "InsertText: NewLine!" );
    DBG_ASSERT( c != '\r', "InsertText: NewLine!" );

    TextPaM aPaM( rCurSel.GetStart() );
    TextNode* pNode = mpDoc->GetNodes().GetObject( aPaM.GetPara() );

    if ( pNode->GetText().getLength() < STRING_MAXLEN )
    {
        bool bDoOverwrite = ( bOverwrite &&
                ( aPaM.GetIndex() < pNode->GetText().getLength() ) );

        bool bUndoAction = ( rCurSel.HasRange() || bDoOverwrite );

        if ( bUndoAction )
            UndoActionStart();

        if ( rCurSel.HasRange() )
        {
            aPaM = ImpDeleteText( rCurSel );
        }
        else if ( bDoOverwrite )
        {
            // if selection, then don't overwrite a character
            TextSelection aTmpSel( aPaM );
            aTmpSel.GetEnd().GetIndex()++;
            ImpDeleteText( aTmpSel );
        }

        if (bIsUserInput && IsInputSequenceCheckingRequired( c, rCurSel ))
        {
            uno::Reference < i18n::XExtendedInputSequenceChecker > xISC = GetInputSequenceChecker();
            SvtCTLOptions aCTLOptions;

            if (xISC.is())
            {
                xub_StrLen nTmpPos = aPaM.GetIndex();
                sal_Int16 nCheckMode = aCTLOptions.IsCTLSequenceCheckingRestricted() ?
                        i18n::InputSequenceCheckMode::STRICT : i18n::InputSequenceCheckMode::BASIC;

                // the text that needs to be checked is only the one
                // before the current cursor position
                OUString aOldText( mpDoc->GetText( aPaM.GetPara() ).copy(0, nTmpPos) );
                OUString aNewText( aOldText );
                if (aCTLOptions.IsCTLSequenceCheckingTypeAndReplace())
                {
                    xISC->correctInputSequence( aNewText, nTmpPos - 1, c, nCheckMode );

                    // find position of first character that has changed
                    sal_Int32 nOldLen = aOldText.getLength();
                    sal_Int32 nNewLen = aNewText.getLength();
                    const sal_Unicode *pOldTxt = aOldText.getStr();
                    const sal_Unicode *pNewTxt = aNewText.getStr();
                    sal_Int32 nChgPos = 0;
                    while ( nChgPos < nOldLen && nChgPos < nNewLen &&
                            pOldTxt[nChgPos] == pNewTxt[nChgPos] )
                        ++nChgPos;

                    OUString aChgText( aNewText.copy( nChgPos ) );

                    // select text from first pos to be changed to current pos
                    TextSelection aSel( TextPaM( aPaM.GetPara(), (sal_uInt16) nChgPos ), aPaM );

                    if (!aChgText.isEmpty())
                        // ImpInsertText implicitly handles undo...
                        return ImpInsertText( aSel, aChgText );
                    else
                        return aPaM;
                }
                else
                {
                    // should the character be ignored (i.e. not get inserted) ?
                    if (!xISC->checkInputSequence( aOldText, nTmpPos - 1, c, nCheckMode ))
                        return aPaM;    // nothing to be done -> no need for undo
                }
            }

            // at this point now we will insert the character 'normally' some lines below...
        }


        if ( IsUndoEnabled() && !IsInUndo() )
        {
            TextUndoInsertChars* pNewUndo = new TextUndoInsertChars( this, aPaM, OUString(c) );
            sal_Bool bTryMerge = ( !bDoOverwrite && ( c != ' ' ) ) ? sal_True : sal_False;
            InsertUndo( pNewUndo, bTryMerge );
        }

        TEParaPortion* pPortion = mpTEParaPortions->GetObject( aPaM.GetPara() );
        pPortion->MarkInvalid( aPaM.GetIndex(), 1 );
        if ( c == '\t' )
            pPortion->SetNotSimpleInvalid();
        aPaM = mpDoc->InsertText( aPaM, c );
        ImpCharsInserted( aPaM.GetPara(), aPaM.GetIndex()-1, 1 );

        TextModified();

        if ( bUndoAction )
            UndoActionEnd();
    }

    return aPaM;
}


TextPaM TextEngine::ImpInsertText( const TextSelection& rCurSel, const OUString& rStr )
{
    UndoActionStart();

    TextPaM aPaM;

    if ( rCurSel.HasRange() )
        aPaM = ImpDeleteText( rCurSel );
    else
        aPaM = rCurSel.GetEnd();

    OUString aText(convertLineEnd(rStr, LINEEND_LF));

    sal_Int32 nStart = 0;
    while ( nStart < aText.getLength() )
    {
        sal_Int32 nEnd = aText.indexOf( LINE_SEP, nStart );
        if (nEnd == -1)
            nEnd = aText.getLength(); // do not dereference!

        // Start == End => empty line
        if ( nEnd > nStart )
        {
            OUString aLine(aText.copy(nStart, nEnd-nStart));
            if ( IsUndoEnabled() && !IsInUndo() )
                InsertUndo( new TextUndoInsertChars( this, aPaM, aLine ) );

            TEParaPortion* pPortion = mpTEParaPortions->GetObject( aPaM.GetPara() );
            pPortion->MarkInvalid( aPaM.GetIndex(), aLine.getLength() );
            if (aLine.indexOf( '\t' ) != -1)
                pPortion->SetNotSimpleInvalid();

            aPaM = mpDoc->InsertText( aPaM, aLine );
            ImpCharsInserted( aPaM.GetPara(), aPaM.GetIndex()-aLine.getLength(), aLine.getLength() );

        }
        if ( nEnd < aText.getLength() )
            aPaM = ImpInsertParaBreak( aPaM );

        if ( nEnd == aText.getLength() )    // #108611# prevent overflow in "nStart = nEnd+1" calculation
            break;

        nStart = nEnd+1;
    }

    UndoActionEnd();

    TextModified();
    return aPaM;
}

TextPaM TextEngine::ImpInsertParaBreak( const TextSelection& rCurSel, sal_Bool bKeepEndingAttribs )
{
    TextPaM aPaM;
    if ( rCurSel.HasRange() )
        aPaM = ImpDeleteText( rCurSel );
    else
        aPaM = rCurSel.GetEnd();

    return ImpInsertParaBreak( aPaM, bKeepEndingAttribs );
}

TextPaM TextEngine::ImpInsertParaBreak( const TextPaM& rPaM, sal_Bool bKeepEndingAttribs )
{
    if ( IsUndoEnabled() && !IsInUndo() )
        InsertUndo( new TextUndoSplitPara( this, rPaM.GetPara(), rPaM.GetIndex() ) );

    TextNode* pNode = mpDoc->GetNodes().GetObject( rPaM.GetPara() );
    bool bFirstParaContentChanged = rPaM.GetIndex() < pNode->GetText().getLength();

    TextPaM aPaM( mpDoc->InsertParaBreak( rPaM, bKeepEndingAttribs ) );

    TEParaPortion* pPortion = mpTEParaPortions->GetObject( rPaM.GetPara() );
    DBG_ASSERT( pPortion, "ImpInsertParaBreak: Hidden Portion" );
    pPortion->MarkInvalid( rPaM.GetIndex(), 0 );

    TextNode* pNewNode = mpDoc->GetNodes().GetObject( aPaM.GetPara() );
    TEParaPortion* pNewPortion = new TEParaPortion( pNewNode );
    mpTEParaPortions->Insert( pNewPortion, aPaM.GetPara() );
    ImpParagraphInserted( aPaM.GetPara() );

    CursorMoved( rPaM.GetPara() );  // if empty attribute created
    TextModified();

    if ( bFirstParaContentChanged )
        Broadcast( TextHint( TEXT_HINT_PARACONTENTCHANGED, rPaM.GetPara() ) );

    return aPaM;
}

Rectangle TextEngine::PaMtoEditCursor( const TextPaM& rPaM, sal_Bool bSpecial )
{
    DBG_ASSERT( GetUpdateMode(), "PaMtoEditCursor: GetUpdateMode()" );

    Rectangle aEditCursor;
    long nY = 0;

    if ( !mbHasMultiLineParas )
    {
        nY = rPaM.GetPara() * mnCharHeight;
    }
    else
    {
        for ( sal_uLong nPortion = 0; nPortion < rPaM.GetPara(); nPortion++ )
        {
            TEParaPortion* pPortion = mpTEParaPortions->GetObject(nPortion);
            nY += pPortion->GetLines().size() * mnCharHeight;
        }
    }

    aEditCursor = GetEditCursor( rPaM, bSpecial );
    aEditCursor.Top() += nY;
    aEditCursor.Bottom() += nY;
    return aEditCursor;
}

Rectangle TextEngine::GetEditCursor( const TextPaM& rPaM, sal_Bool bSpecial, sal_Bool bPreferPortionStart )
{
    if ( !IsFormatted() && !IsFormatting() )
        FormatAndUpdate();

    TEParaPortion* pPortion = mpTEParaPortions->GetObject( rPaM.GetPara() );
    //TextNode* pNode = mpDoc->GetNodes().GetObject( rPaM.GetPara() );

    /*
      bSpecial: If behind the last character of a made up line, stay at the
                  end of the line, not at the start of the next line.
      Purpose:  - really END = > behind the last character
                - to selection...

    */

    long nY = 0;
    sal_uInt16 nCurIndex = 0;
    TextLine* pLine = 0;
    for ( sal_uInt16 nLine = 0; nLine < pPortion->GetLines().size(); nLine++ )
    {
        TextLine* pTmpLine = pPortion->GetLines()[ nLine ];
        if ( ( pTmpLine->GetStart() == rPaM.GetIndex() ) || ( pTmpLine->IsIn( rPaM.GetIndex(), bSpecial ) ) )
        {
            pLine = pTmpLine;
            break;
        }

        nCurIndex = nCurIndex + pTmpLine->GetLen();
        nY += mnCharHeight;
    }
    if ( !pLine )
    {
        // Cursor at end of paragraph
        DBG_ASSERT( rPaM.GetIndex() == nCurIndex, "GetEditCursor: Bad Index!" );

        pLine = pPortion->GetLines().back();
        nY -= mnCharHeight;
        nCurIndex = nCurIndex - pLine->GetLen();
    }

    Rectangle aEditCursor;

    aEditCursor.Top() = nY;
    nY += mnCharHeight;
    aEditCursor.Bottom() = nY-1;

    // search within the line
    long nX = ImpGetXPos( rPaM.GetPara(), pLine, rPaM.GetIndex(), bPreferPortionStart );
    aEditCursor.Left() = aEditCursor.Right() = nX;
    return aEditCursor;
}

long TextEngine::ImpGetXPos( sal_uLong nPara, TextLine* pLine, sal_uInt16 nIndex, sal_Bool bPreferPortionStart )
{
    DBG_ASSERT( ( nIndex >= pLine->GetStart() ) && ( nIndex <= pLine->GetEnd() ) , "ImpGetXPos: Bad parameters!" );

    sal_Bool bDoPreferPortionStart = bPreferPortionStart;
    // Assure that the portion belongs to this line
    if ( nIndex == pLine->GetStart() )
        bDoPreferPortionStart = sal_True;
    else if ( nIndex == pLine->GetEnd() )
        bDoPreferPortionStart = sal_False;

    TEParaPortion* pParaPortion = mpTEParaPortions->GetObject( nPara );

    sal_uInt16 nTextPortionStart = 0;
    size_t nTextPortion = pParaPortion->GetTextPortions().FindPortion( nIndex, nTextPortionStart, bDoPreferPortionStart );

    DBG_ASSERT( ( nTextPortion >= pLine->GetStartPortion() ) && ( nTextPortion <= pLine->GetEndPortion() ), "GetXPos: Portion not in current line!" );

    TETextPortion* pPortion = pParaPortion->GetTextPortions()[ nTextPortion ];

    long nX = ImpGetPortionXOffset( nPara, pLine, nTextPortion );

    long nPortionTextWidth = pPortion->GetWidth();

    if ( nTextPortionStart != nIndex )
    {
        // Search within portion...
        if ( nIndex == ( nTextPortionStart + pPortion->GetLen() ) )
        {
            // End of Portion
            if ( ( pPortion->GetKind() == PORTIONKIND_TAB ) ||
                 ( !IsRightToLeft() && !pPortion->IsRightToLeft() ) ||
                 ( IsRightToLeft() && pPortion->IsRightToLeft() ) )
            {
                nX += nPortionTextWidth;
                if ( ( pPortion->GetKind() == PORTIONKIND_TAB ) && ( (nTextPortion+1) < pParaPortion->GetTextPortions().size() ) )
                {
                    TETextPortion* pNextPortion = pParaPortion->GetTextPortions()[ nTextPortion+1 ];
                    if ( ( pNextPortion->GetKind() != PORTIONKIND_TAB ) && (
                              ( !IsRightToLeft() && pNextPortion->IsRightToLeft() ) ||
                              ( IsRightToLeft() && !pNextPortion->IsRightToLeft() ) ) )
                    {
//                        nX += pNextPortion->GetWidth();
                        // End of the tab portion, use start of next for cursor pos
                        DBG_ASSERT( !bPreferPortionStart, "ImpGetXPos: How can we get here!" );
                        nX = ImpGetXPos( nPara, pLine, nIndex, sal_True );
                    }

                }
            }
        }
        else if ( pPortion->GetKind() == PORTIONKIND_TEXT )
        {
            DBG_ASSERT( nIndex != pLine->GetStart(), "ImpGetXPos: Strange behavior" );

            long nPosInPortion = (long)CalcTextWidth( nPara, nTextPortionStart, nIndex-nTextPortionStart );

            if ( ( !IsRightToLeft() && !pPortion->IsRightToLeft() ) ||
                 ( IsRightToLeft() && pPortion->IsRightToLeft() ) )
            {
                nX += nPosInPortion;
            }
            else
            {
                nX += nPortionTextWidth - nPosInPortion;
            }
        }
    }
    else // if ( nIndex == pLine->GetStart() )
    {
        if ( ( pPortion->GetKind() != PORTIONKIND_TAB ) &&
                ( ( !IsRightToLeft() && pPortion->IsRightToLeft() ) ||
                ( IsRightToLeft() && !pPortion->IsRightToLeft() ) ) )
        {
            nX += nPortionTextWidth;
        }
    }

    return nX;
}

const TextAttrib* TextEngine::FindAttrib( const TextPaM& rPaM, sal_uInt16 nWhich ) const
{
    const TextAttrib* pAttr = NULL;
    const TextCharAttrib* pCharAttr = FindCharAttrib( rPaM, nWhich );
    if ( pCharAttr )
        pAttr = &pCharAttr->GetAttr();
    return pAttr;
}

const TextCharAttrib* TextEngine::FindCharAttrib( const TextPaM& rPaM, sal_uInt16 nWhich ) const
{
    const TextCharAttrib* pAttr = NULL;
    TextNode* pNode = mpDoc->GetNodes().GetObject( rPaM.GetPara() );
    if ( pNode && ( rPaM.GetIndex() < pNode->GetText().getLength() ) )
        pAttr = pNode->GetCharAttribs().FindAttrib( nWhich, rPaM.GetIndex() );
    return pAttr;
}

sal_Bool TextEngine::HasAttrib( sal_uInt16 nWhich ) const
{
    sal_Bool bAttr = sal_False;
    for ( sal_uLong n = mpDoc->GetNodes().Count(); --n && !bAttr; )
    {
        TextNode* pNode = mpDoc->GetNodes().GetObject( n );
        bAttr = pNode->GetCharAttribs().HasAttrib( nWhich );
    }
    return bAttr;
}

TextPaM TextEngine::GetPaM( const Point& rDocPos, sal_Bool bSmart )
{
    DBG_ASSERT( GetUpdateMode(), "GetPaM: GetUpdateMode()" );

    long nY = 0;
    for ( sal_uLong nPortion = 0; nPortion < mpTEParaPortions->Count(); nPortion++ )
    {
        TEParaPortion* pPortion = mpTEParaPortions->GetObject( nPortion );
        long nTmpHeight = pPortion->GetLines().size() * mnCharHeight;
        nY += nTmpHeight;
        if ( nY > rDocPos.Y() )
        {
            nY -= nTmpHeight;
            Point aPosInPara( rDocPos );
            aPosInPara.Y() -= nY;

            TextPaM aPaM( nPortion, 0 );
            aPaM.GetIndex() = ImpFindIndex( nPortion, aPosInPara, bSmart );
            return aPaM;
        }
    }

    // not found - go to last visible
    sal_uLong nLastNode = mpDoc->GetNodes().Count() - 1;
    TextNode* pLast = mpDoc->GetNodes().GetObject( nLastNode );
    return TextPaM( nLastNode, pLast->GetText().getLength() );
}

sal_uInt16 TextEngine::ImpFindIndex( sal_uLong nPortion, const Point& rPosInPara, sal_Bool bSmart )
{
    DBG_ASSERT( IsFormatted(), "GetPaM: Not formatted" );
    TEParaPortion* pPortion = mpTEParaPortions->GetObject( nPortion );

    sal_uInt16 nCurIndex = 0;

    long nY = 0;
    TextLine* pLine = 0;
    sal_uInt16 nLine;
    for ( nLine = 0; nLine < pPortion->GetLines().size(); nLine++ )
    {
        TextLine* pTmpLine = pPortion->GetLines()[ nLine ];
        nY += mnCharHeight;
        if ( nY > rPosInPara.Y() )  // that's it
        {
            pLine = pTmpLine;
            break;                  // correct Y-Position not needed
        }
    }
    DBG_ASSERT( pLine, "ImpFindIndex: pLine ?" );

    nCurIndex = GetCharPos( nPortion, nLine, rPosInPara.X(), bSmart );

    if ( nCurIndex && ( nCurIndex == pLine->GetEnd() ) &&
         ( pLine != pPortion->GetLines().back() ) )
    {
        uno::Reference < i18n::XBreakIterator > xBI = GetBreakIterator();
        sal_Int32 nCount = 1;
        nCurIndex = (sal_uInt16)xBI->previousCharacters( pPortion->GetNode()->GetText(), nCurIndex, GetLocale(), i18n::CharacterIteratorMode::SKIPCELL, nCount, nCount );
    }
    return nCurIndex;
}

sal_uInt16 TextEngine::GetCharPos( sal_uLong nPortion, sal_uInt16 nLine, long nXPos, sal_Bool )
{

    TEParaPortion* pPortion = mpTEParaPortions->GetObject( nPortion );
    TextLine* pLine = pPortion->GetLines()[ nLine ];

    sal_uInt16 nCurIndex = pLine->GetStart();

    long nTmpX = pLine->GetStartX();
    if ( nXPos <= nTmpX )
        return nCurIndex;

    for ( sal_uInt16 i = pLine->GetStartPortion(); i <= pLine->GetEndPortion(); i++ )
    {
        TETextPortion* pTextPortion = pPortion->GetTextPortions()[ i ];
        nTmpX += pTextPortion->GetWidth();

        if ( nTmpX > nXPos )
        {
            if( pTextPortion->GetLen() > 1 )
            {
                nTmpX -= pTextPortion->GetWidth();  // position before Portion
                // TODO: Optimize: no GetTextBreak if fixed-width Font
                Font aFont;
                SeekCursor( nPortion, nCurIndex+1, aFont, NULL );
                mpRefDev->SetFont( aFont);
                long nPosInPortion = nXPos-nTmpX;
                if ( IsRightToLeft() != pTextPortion->IsRightToLeft() )
                    nPosInPortion = pTextPortion->GetWidth() - nPosInPortion;
                nCurIndex = mpRefDev->GetTextBreak( pPortion->GetNode()->GetText(), nPosInPortion, nCurIndex );
                // MT: GetTextBreak should assure that we are not withing a CTL cell...
            }
            return nCurIndex;
        }
        nCurIndex = nCurIndex + pTextPortion->GetLen();
    }
    return nCurIndex;
}


sal_uLong TextEngine::GetTextHeight() const
{
    DBG_ASSERT( GetUpdateMode(), "GetTextHeight: GetUpdateMode()" );

    if ( !IsFormatted() && !IsFormatting() )
        ((TextEngine*)this)->FormatAndUpdate();

    return mnCurTextHeight;
}

sal_uLong TextEngine::GetTextHeight( sal_uLong nParagraph ) const
{
    DBG_ASSERT( GetUpdateMode(), "GetTextHeight: GetUpdateMode()" );

      if ( !IsFormatted() && !IsFormatting() )
        ((TextEngine*)this)->FormatAndUpdate();

    return CalcParaHeight( nParagraph );
}

sal_uLong TextEngine::CalcTextWidth( sal_uLong nPara )
{
    sal_uLong nParaWidth = 0;
    TEParaPortion* pPortion = mpTEParaPortions->GetObject( nPara );
    for ( sal_uInt16 nLine = pPortion->GetLines().size(); nLine; )
    {
        sal_uLong nLineWidth = 0;
        TextLine* pLine = pPortion->GetLines()[ --nLine ];
        for ( sal_uInt16 nTP = pLine->GetStartPortion(); nTP <= pLine->GetEndPortion(); nTP++ )
        {
            TETextPortion* pTextPortion = pPortion->GetTextPortions()[ nTP ];
            nLineWidth += pTextPortion->GetWidth();
        }
        if ( nLineWidth > nParaWidth )
            nParaWidth = nLineWidth;
    }
    return nParaWidth;
}

sal_uLong TextEngine::CalcTextWidth()
{
    if ( !IsFormatted() && !IsFormatting() )
        FormatAndUpdate();

    if ( mnCurTextWidth == 0xFFFFFFFF )
    {
        mnCurTextWidth = 0;
        for ( sal_uLong nPara = mpTEParaPortions->Count(); nPara; )
        {
            sal_uLong nParaWidth = CalcTextWidth( --nPara );
            if ( nParaWidth > mnCurTextWidth )
                mnCurTextWidth = nParaWidth;
        }
    }
    return mnCurTextWidth+1;// wider by 1, as CreateLines breaks at >=
}

sal_uLong TextEngine::CalcTextHeight()
{
    DBG_ASSERT( GetUpdateMode(), "CalcTextHeight: GetUpdateMode()" );

    sal_uLong nY = 0;
    for ( sal_uLong nPortion = mpTEParaPortions->Count(); nPortion; )
        nY += CalcParaHeight( --nPortion );
    return nY;
}

sal_uLong TextEngine::CalcTextWidth( sal_uLong nPara, sal_uInt16 nPortionStart, sal_uInt16 nLen, const Font* pFont )
{
    // within the text there must not be a Portion change (attribute/tab)!
    DBG_ASSERT( mpDoc->GetNodes().GetObject( nPara )->GetText().indexOf( '\t', nPortionStart ) >= (nPortionStart+nLen), "CalcTextWidth: Tab!" );

    sal_uLong nWidth;
    if ( mnFixCharWidth100 )
    {
        nWidth = (sal_uLong)nLen*mnFixCharWidth100/100;
    }
    else
    {
        if ( pFont )
        {
            if ( !mpRefDev->GetFont().IsSameInstance( *pFont ) )
                mpRefDev->SetFont( *pFont );
        }
        else
        {
            Font aFont;
            SeekCursor( nPara, nPortionStart+1, aFont, NULL );
            mpRefDev->SetFont( aFont );
        }
        TextNode* pNode = mpDoc->GetNodes().GetObject( nPara );
        nWidth = (sal_uLong)mpRefDev->GetTextWidth( pNode->GetText(), nPortionStart, nLen );

    }
    return nWidth;
}

sal_uInt16 TextEngine::GetLineCount( sal_uLong nParagraph ) const
{
    DBG_ASSERT( nParagraph < mpTEParaPortions->Count(), "GetLineCount: Out of range" );

    TEParaPortion* pPPortion = mpTEParaPortions->GetObject( nParagraph );
    if ( pPPortion )
        return pPPortion->GetLines().size();

    return 0xFFFF;
}

sal_uInt16 TextEngine::GetLineLen( sal_uLong nParagraph, sal_uInt16 nLine ) const
{
    DBG_ASSERT( nParagraph < mpTEParaPortions->Count(), "GetLineCount: Out of range" );

    TEParaPortion* pPPortion = mpTEParaPortions->GetObject( nParagraph );
    if ( pPPortion && ( nLine < pPPortion->GetLines().size() ) )
    {
        TextLine* pLine = pPPortion->GetLines()[ nLine ];
        return pLine->GetLen();
    }

    return 0xFFFF;
}

sal_uLong TextEngine::CalcParaHeight( sal_uLong nParagraph ) const
{
    sal_uLong nHeight = 0;

    TEParaPortion* pPPortion = mpTEParaPortions->GetObject( nParagraph );
    DBG_ASSERT( pPPortion, "GetParaHeight: paragraph not found" );
    if ( pPPortion )
        nHeight = pPPortion->GetLines().size() * mnCharHeight;

    return nHeight;
}

void TextEngine::UpdateSelections()
{
}

Range TextEngine::GetInvalidYOffsets( sal_uLong nPortion )
{
    TEParaPortion* pTEParaPortion = mpTEParaPortions->GetObject( nPortion );
    sal_uInt16 nLines = pTEParaPortion->GetLines().size();
    sal_uInt16 nLastInvalid, nFirstInvalid = 0;
    sal_uInt16 nLine;
    for ( nLine = 0; nLine < nLines; nLine++ )
    {
        TextLine* pL = pTEParaPortion->GetLines()[ nLine ];
        if ( pL->IsInvalid() )
        {
            nFirstInvalid = nLine;
            break;
        }
    }

    for ( nLastInvalid = nFirstInvalid; nLastInvalid < nLines; nLastInvalid++ )
    {
        TextLine* pL = pTEParaPortion->GetLines()[ nLine ];
        if ( pL->IsValid() )
            break;
    }

    if ( nLastInvalid >= nLines )
        nLastInvalid = nLines-1;

    return Range( nFirstInvalid*mnCharHeight, ((nLastInvalid+1)*mnCharHeight)-1 );
}

sal_uLong TextEngine::GetParagraphCount() const
{
    return mpDoc->GetNodes().Count();
}

void TextEngine::EnableUndo( sal_Bool bEnable )
{
    // delete list when switching mode
    if ( bEnable != IsUndoEnabled() )
        ResetUndo();

    mbUndoEnabled = bEnable;
}

::svl::IUndoManager& TextEngine::GetUndoManager()
{
    if ( !mpUndoManager )
        mpUndoManager = new TextUndoManager( this );
    return *mpUndoManager;
}

void TextEngine::UndoActionStart( sal_uInt16 nId )
{
    if ( IsUndoEnabled() && !IsInUndo() )
    {
        OUString aComment;
        GetUndoManager().EnterListAction( aComment, OUString(), nId );
    }
}

void TextEngine::UndoActionEnd()
{
    if ( IsUndoEnabled() && !IsInUndo() )
        GetUndoManager().LeaveListAction();
}

void TextEngine::InsertUndo( TextUndo* pUndo, sal_Bool bTryMerge )
{
    DBG_ASSERT( !IsInUndo(), "InsertUndo: in Undo mode!" );
    GetUndoManager().AddUndoAction( pUndo, bTryMerge );
}

void TextEngine::ResetUndo()
{
    if ( mpUndoManager )
        mpUndoManager->Clear();
}

void TextEngine::InsertContent( TextNode* pNode, sal_uLong nPara )
{
    DBG_ASSERT( pNode, "InsertContent: NULL-Pointer!" );
    DBG_ASSERT( IsInUndo(), "InsertContent: only in Undo()!" );
    TEParaPortion* pNew = new TEParaPortion( pNode );
    mpTEParaPortions->Insert( pNew, nPara );
    mpDoc->GetNodes().Insert( pNode, nPara );
    ImpParagraphInserted( nPara );
}

TextPaM TextEngine::SplitContent( sal_uLong nNode, sal_uInt16 nSepPos )
{
    #ifdef DBG_UTIL
    TextNode* pNode = mpDoc->GetNodes().GetObject( nNode );
    DBG_ASSERT( pNode, "SplitContent: Invalid Node!" );
    DBG_ASSERT( IsInUndo(), "SplitContent: only in Undo()!" );
    DBG_ASSERT( nSepPos <= pNode->GetText().getLength(), "SplitContent: Bad index" );
    #endif
    TextPaM aPaM( nNode, nSepPos );
    return ImpInsertParaBreak( aPaM );
}

TextPaM TextEngine::ConnectContents( sal_uLong nLeftNode )
{
    DBG_ASSERT( IsInUndo(), "ConnectContent: only in Undo()!" );
    return ImpConnectParagraphs( nLeftNode, nLeftNode+1 );
}

void TextEngine::SeekCursor( sal_uLong nPara, sal_uInt16 nPos, Font& rFont, OutputDevice* pOutDev )
{
    rFont = maFont;
    if ( pOutDev )
        pOutDev->SetTextColor( maTextColor );

    TextNode* pNode = mpDoc->GetNodes().GetObject( nPara );
    sal_uInt16 nAttribs = pNode->GetCharAttribs().Count();
    for ( sal_uInt16 nAttr = 0; nAttr < nAttribs; nAttr++ )
    {
        TextCharAttrib* pAttrib = pNode->GetCharAttribs().GetAttrib( nAttr );
        if ( pAttrib->GetStart() > nPos )
            break;

        // When seeking don't use Attr that start there!
        // Do not use empty attributes:
        // - If just being setup and empty => no effect on Font
        // - Characters that are setup in an empty paragraph become visible right away.
        if ( ( ( pAttrib->GetStart() < nPos ) && ( pAttrib->GetEnd() >= nPos ) )
                    || pNode->GetText().isEmpty() )
        {
            if ( pAttrib->Which() != TEXTATTR_FONTCOLOR )
            {
                pAttrib->GetAttr().SetFont(rFont);
            }
            else
            {
                if ( pOutDev )
                    pOutDev->SetTextColor( ((TextAttribFontColor&)pAttrib->GetAttr()).GetColor() );
            }
        }
    }

    if ( mpIMEInfos && mpIMEInfos->pAttribs && ( mpIMEInfos->aPos.GetPara() == nPara ) &&
        ( nPos > mpIMEInfos->aPos.GetIndex() ) && ( nPos <= ( mpIMEInfos->aPos.GetIndex() + mpIMEInfos->nLen ) ) )
    {
        sal_uInt16 nAttr = mpIMEInfos->pAttribs[ nPos - mpIMEInfos->aPos.GetIndex() - 1 ];
        if ( nAttr & EXTTEXTINPUT_ATTR_UNDERLINE )
            rFont.SetUnderline( UNDERLINE_SINGLE );
        else if ( nAttr & EXTTEXTINPUT_ATTR_BOLDUNDERLINE )
            rFont.SetUnderline( UNDERLINE_BOLD );
        else if ( nAttr & EXTTEXTINPUT_ATTR_DOTTEDUNDERLINE )
            rFont.SetUnderline( UNDERLINE_DOTTED );
        else if ( nAttr & EXTTEXTINPUT_ATTR_DASHDOTUNDERLINE )
            rFont.SetUnderline( UNDERLINE_DOTTED );
        if ( nAttr & EXTTEXTINPUT_ATTR_REDTEXT )
            rFont.SetColor( Color( COL_RED ) );
        else if ( nAttr & EXTTEXTINPUT_ATTR_HALFTONETEXT )
            rFont.SetColor( Color( COL_LIGHTGRAY ) );
        if ( nAttr & EXTTEXTINPUT_ATTR_HIGHLIGHT )
        {
            const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
            rFont.SetColor( rStyleSettings.GetHighlightTextColor() );
            rFont.SetFillColor( rStyleSettings.GetHighlightColor() );
            rFont.SetTransparent( sal_False );
        }
        else if ( nAttr & EXTTEXTINPUT_ATTR_GRAYWAVELINE )
        {
            rFont.SetUnderline( UNDERLINE_WAVE );
//          if( pOut )
//              pOut->SetTextLineColor( Color( COL_LIGHTGRAY ) );
        }
    }
}

void TextEngine::FormatAndUpdate( TextView* pCurView )
{
    if ( mbDowning )
        return ;

    if ( IsInUndo() )
        IdleFormatAndUpdate( pCurView );
    else
    {
        FormatDoc();
        UpdateViews( pCurView );
    }
}


void TextEngine::IdleFormatAndUpdate( TextView* pCurView, sal_uInt16 nMaxTimerRestarts )
{
    mpIdleFormatter->DoIdleFormat( pCurView, nMaxTimerRestarts );
}

void TextEngine::TextModified()
{
    mbFormatted = sal_False;
    mbModified = sal_True;
}

void TextEngine::UpdateViews( TextView* pCurView )
{
    if ( !GetUpdateMode() || IsFormatting() || maInvalidRect.IsEmpty() )
        return;

    DBG_ASSERT( IsFormatted(), "UpdateViews: Doc not formatted!" );

    for ( sal_uInt16 nView = 0; nView < mpViews->size(); nView++ )
    {
        TextView* pView = (*mpViews)[ nView ];
        pView->HideCursor();

        Rectangle aClipRect( maInvalidRect );
        Size aOutSz = pView->GetWindow()->GetOutputSizePixel();
        Rectangle aVisArea( pView->GetStartDocPos(), aOutSz );
        aClipRect.Intersection( aVisArea );
        if ( !aClipRect.IsEmpty() )
        {
            // translate into window coordinates
            Point aNewPos = pView->GetWindowPos( aClipRect.TopLeft() );
            if ( IsRightToLeft() )
                aNewPos.X() -= aOutSz.Width() - 1;
            aClipRect.SetPos( aNewPos );

            if ( pView == pCurView )
                pView->ImpPaint( aClipRect, !pView->GetWindow()->IsPaintTransparent() );
            else
                pView->GetWindow()->Invalidate( aClipRect );
        }
    }

    if ( pCurView )
    {
        pCurView->ShowCursor( pCurView->IsAutoScroll() );
    }

    maInvalidRect = Rectangle();
}

IMPL_LINK_NOARG(TextEngine, IdleFormatHdl)
{
    FormatAndUpdate( mpIdleFormatter->GetView() );
    return 0;
}

void TextEngine::CheckIdleFormatter()
{
    mpIdleFormatter->ForceTimeout();
}

void TextEngine::FormatFullDoc()
{
    for ( sal_uLong nPortion = 0; nPortion < mpTEParaPortions->Count(); nPortion++ )
    {
        TEParaPortion* pTEParaPortion = mpTEParaPortions->GetObject( nPortion );
        sal_Int32 nLen = pTEParaPortion->GetNode()->GetText().getLength();
        pTEParaPortion->MarkSelectionInvalid( 0, nLen );
    }
    mbFormatted = sal_False;
    FormatDoc();
}

void TextEngine::FormatDoc()
{
    if ( IsFormatted() || !GetUpdateMode() || IsFormatting() )
        return;

    mbIsFormatting = sal_True;
    mbHasMultiLineParas = sal_False;

    long nY = 0;
    bool bGrow = false;

    maInvalidRect = Rectangle(); // clear
    for ( sal_uLong nPara = 0; nPara < mpTEParaPortions->Count(); nPara++ )
    {
        TEParaPortion* pTEParaPortion = mpTEParaPortions->GetObject( nPara );
        if ( pTEParaPortion->IsInvalid() )
        {
            sal_uLong nOldParaWidth = 0xFFFFFFFF;
            if ( mnCurTextWidth != 0xFFFFFFFF )
                nOldParaWidth = CalcTextWidth( nPara );

            ImpFormattingParagraph( nPara );

            if ( CreateLines( nPara ) )
                bGrow = true;

            // set InvalidRect only once
            if ( maInvalidRect.IsEmpty() )
            {
                // otherwise remains Empty() for Paperwidth 0 (AutoPageSize)
                long nWidth = (long)mnMaxTextWidth;
                if ( !nWidth )
                    nWidth = 0x7FFFFFFF;
                Range aInvRange( GetInvalidYOffsets( nPara ) );
                maInvalidRect = Rectangle( Point( 0, nY+aInvRange.Min() ),
                    Size( nWidth, aInvRange.Len() ) );
            }
            else
            {
                maInvalidRect.Bottom() = nY + CalcParaHeight( nPara );
            }

            if ( mnCurTextWidth != 0xFFFFFFFF )
            {
                sal_uLong nNewParaWidth = CalcTextWidth( nPara );
                if ( nNewParaWidth >= mnCurTextWidth )
                    mnCurTextWidth = nNewParaWidth;
                else if ( ( nOldParaWidth != 0xFFFFFFFF ) && ( nOldParaWidth >= mnCurTextWidth ) )
                    mnCurTextWidth = 0xFFFFFFFF;
            }
        }
        else if ( bGrow )
        {
            maInvalidRect.Bottom() = nY + CalcParaHeight( nPara );
        }
        nY += CalcParaHeight( nPara );
        if ( !mbHasMultiLineParas && pTEParaPortion->GetLines().size() > 1 )
            mbHasMultiLineParas = sal_True;
    }

    if ( !maInvalidRect.IsEmpty() )
    {
        sal_uLong nNewHeight = CalcTextHeight();
        long nDiff = nNewHeight - mnCurTextHeight;
        if ( nNewHeight < mnCurTextHeight )
        {
            maInvalidRect.Bottom() = (long)std::max( nNewHeight, mnCurTextHeight );
            if ( maInvalidRect.IsEmpty() )
            {
                maInvalidRect.Top() = 0;
                // Left and Right are not evaluated, but set because of IsEmpty
                maInvalidRect.Left() = 0;
                maInvalidRect.Right() = mnMaxTextWidth;
            }
        }

        mnCurTextHeight = nNewHeight;
        if ( nDiff )
        {
            mbFormatted = sal_True;
            ImpTextHeightChanged();
        }
    }

    mbIsFormatting = sal_False;
    mbFormatted = sal_True;

    ImpTextFormatted();
}

void TextEngine::CreateAndInsertEmptyLine( sal_uLong nPara )
{
    TextNode* pNode = mpDoc->GetNodes().GetObject( nPara );
    TEParaPortion* pTEParaPortion = mpTEParaPortions->GetObject( nPara );

    TextLine* pTmpLine = new TextLine;
    pTmpLine->SetStart( pNode->GetText().getLength() );
    pTmpLine->SetEnd( pTmpLine->GetStart() );
    pTEParaPortion->GetLines().push_back( pTmpLine );

    if ( ImpGetAlign() == TXTALIGN_CENTER )
        pTmpLine->SetStartX( (short)(mnMaxTextWidth / 2) );
    else if ( ImpGetAlign() == TXTALIGN_RIGHT )
        pTmpLine->SetStartX( (short)mnMaxTextWidth );
    else
        pTmpLine->SetStartX( mpDoc->GetLeftMargin() );

    bool bLineBreak = !pNode->GetText().isEmpty();

    TETextPortion* pDummyPortion = new TETextPortion( 0 );
    pDummyPortion->GetWidth() = 0;
    pTEParaPortion->GetTextPortions().push_back( pDummyPortion );

    if ( bLineBreak )
    {
        // -2: The new one is already inserted.
        OSL_ENSURE(
            pTEParaPortion->GetLines()[pTEParaPortion->GetLines().size()-2],
            "CreateAndInsertEmptyLine: Soft Break, no Line?!");
        sal_uInt16 nPos = (sal_uInt16) pTEParaPortion->GetTextPortions().size() - 1 ;
        pTmpLine->SetStartPortion( nPos );
        pTmpLine->SetEndPortion( nPos );
    }
}

void TextEngine::ImpBreakLine( sal_uLong nPara, TextLine* pLine, TETextPortion*, sal_uInt16 nPortionStart, long nRemainingWidth )
{
    TextNode* pNode = mpDoc->GetNodes().GetObject( nPara );

    // Font still should be adjusted
    sal_Int32 nMaxBreakPos = mpRefDev->GetTextBreak( pNode->GetText(), nRemainingWidth, nPortionStart );

    DBG_ASSERT( nMaxBreakPos < pNode->GetText().getLength(), "ImpBreakLine: Break?!" );

    if ( nMaxBreakPos == STRING_LEN )   // GetTextBreak() != GetTextSize()
        nMaxBreakPos = pNode->GetText().getLength() - 1;

    uno::Reference < i18n::XBreakIterator > xBI = GetBreakIterator();
    i18n::LineBreakHyphenationOptions aHyphOptions( NULL, uno::Sequence< beans::PropertyValue >(), 1 );

    i18n::LineBreakUserOptions aUserOptions;
    aUserOptions.forbiddenBeginCharacters = ImpGetLocaleDataWrapper()->getForbiddenCharacters().beginLine;
    aUserOptions.forbiddenEndCharacters = ImpGetLocaleDataWrapper()->getForbiddenCharacters().endLine;
    aUserOptions.applyForbiddenRules = sal_True;
    aUserOptions.allowPunctuationOutsideMargin = sal_False;
    aUserOptions.allowHyphenateEnglish = sal_False;

    static const com::sun::star::lang::Locale aDefLocale;
    i18n::LineBreakResults aLBR = xBI->getLineBreak( pNode->GetText(), nMaxBreakPos, aDefLocale, pLine->GetStart(), aHyphOptions, aUserOptions );
    sal_uInt16 nBreakPos = (sal_uInt16)aLBR.breakIndex;
    if ( nBreakPos <= pLine->GetStart() )
    {
        nBreakPos = nMaxBreakPos;
        if ( nBreakPos <= pLine->GetStart() )
            nBreakPos = pLine->GetStart() + 1;  // infinite loop otherwise!
    }

    // the damaged Portion is the End Portion
    pLine->SetEnd( nBreakPos );
    sal_uInt16 nEndPortion = SplitTextPortion( nPara, nBreakPos );

    bool bBlankSeparator = ( ( nBreakPos >= pLine->GetStart() ) &&
                             ( pNode->GetText()[ nBreakPos ] == ' ' ) );
    if ( bBlankSeparator )
    {
        // generally suppress blanks at the end of line
        TEParaPortion* pTEParaPortion = mpTEParaPortions->GetObject( nPara );
        TETextPortion* pTP = pTEParaPortion->GetTextPortions()[ nEndPortion ];
        DBG_ASSERT( nBreakPos > pLine->GetStart(), "ImpBreakLine: SplitTextPortion at beginning of line?" );
        pTP->GetWidth() = (long)CalcTextWidth( nPara, nBreakPos-pTP->GetLen(), pTP->GetLen()-1 );
    }
    pLine->SetEndPortion( nEndPortion );
}

sal_uInt16 TextEngine::SplitTextPortion( sal_uLong nPara, sal_uInt16 nPos )
{

    // the Portion at nPos is being split, unless there is already a switch at nPos
    if ( nPos == 0 )
        return 0;

    sal_uInt16 nSplitPortion;
    sal_uInt16 nTmpPos = 0;
    TETextPortion* pTextPortion = 0;
    TEParaPortion* pTEParaPortion = mpTEParaPortions->GetObject( nPara );
    sal_uInt16 nPortions = pTEParaPortion->GetTextPortions().size();
    for ( nSplitPortion = 0; nSplitPortion < nPortions; nSplitPortion++ )
    {
        TETextPortion* pTP = pTEParaPortion->GetTextPortions()[nSplitPortion];
        nTmpPos = nTmpPos + pTP->GetLen();
        if ( nTmpPos >= nPos )
        {
            if ( nTmpPos == nPos )  // nothing needs splitting
                return nSplitPortion;
            pTextPortion = pTP;
            break;
        }
    }

    DBG_ASSERT( pTextPortion, "SplitTextPortion: position outside of region!" );

    sal_uInt16 nOverlapp = nTmpPos - nPos;
    pTextPortion->GetLen() = pTextPortion->GetLen() - nOverlapp;
    TETextPortion* pNewPortion = new TETextPortion( nOverlapp );
    pTEParaPortion->GetTextPortions().insert( pTEParaPortion->GetTextPortions().begin() + nSplitPortion + 1, pNewPortion );
    pTextPortion->GetWidth() = (long)CalcTextWidth( nPara, nPos-pTextPortion->GetLen(), pTextPortion->GetLen() );

    return nSplitPortion;
}

void TextEngine::CreateTextPortions( sal_uLong nPara, sal_uInt16 nStartPos )
{
    TEParaPortion* pTEParaPortion = mpTEParaPortions->GetObject( nPara );
    TextNode* pNode = pTEParaPortion->GetNode();
    DBG_ASSERT( !pNode->GetText().isEmpty(), "CreateTextPortions: should not be used for empty paragraphs!" );

    std::set<sal_uInt16> aPositions;
    std::set<sal_uInt16>::iterator aPositionsIt;
    aPositions.insert(0);

    sal_uInt16 nAttribs = pNode->GetCharAttribs().Count();
    for ( sal_uInt16 nAttr = 0; nAttr < nAttribs; nAttr++ )
    {
        TextCharAttrib* pAttrib = pNode->GetCharAttribs().GetAttrib( nAttr );

        aPositions.insert( pAttrib->GetStart() );
        aPositions.insert( pAttrib->GetEnd() );
    }
    aPositions.insert( pNode->GetText().getLength() );

    const std::vector<TEWritingDirectionInfo>& rWritingDirections = pTEParaPortion->GetWritingDirectionInfos();
    for ( std::vector<TEWritingDirectionInfo>::const_iterator it = rWritingDirections.begin(); it != rWritingDirections.end(); ++it )
        aPositions.insert( (*it).nStartPos );

    if ( mpIMEInfos && mpIMEInfos->pAttribs && ( mpIMEInfos->aPos.GetPara() == nPara ) )
    {
        sal_uInt16 nLastAttr = 0xFFFF;
        for( sal_uInt16 n = 0; n < mpIMEInfos->nLen; n++ )
        {
            if ( mpIMEInfos->pAttribs[n] != nLastAttr )
            {
                aPositions.insert( mpIMEInfos->aPos.GetIndex() + n );
                nLastAttr = mpIMEInfos->pAttribs[n];
            }
        }
    }

    sal_Int32 nTabPos = pNode->GetText().indexOf( '\t' );
    while ( nTabPos != -1 )
    {
        aPositions.insert( nTabPos );
        aPositions.insert( nTabPos + 1 );
        nTabPos = pNode->GetText().indexOf( '\t', nTabPos+1 );
    }

    // Delete starting with...
    // Unfortunately, the number of TextPortions does not have to be
    // equal to aPositions.Count(), because of linebreaks
    sal_uInt16 nPortionStart = 0;
    sal_uInt16 nInvPortion = 0;
    sal_uInt16 nP;
    for ( nP = 0; nP < pTEParaPortion->GetTextPortions().size(); nP++ )
    {
        TETextPortion* pTmpPortion = pTEParaPortion->GetTextPortions()[nP];
        nPortionStart = nPortionStart + pTmpPortion->GetLen();
        if ( nPortionStart >= nStartPos )
        {
            nPortionStart = nPortionStart - pTmpPortion->GetLen();
            nInvPortion = nP;
            break;
        }
    }
    OSL_ENSURE(nP < pTEParaPortion->GetTextPortions().size()
            || pTEParaPortion->GetTextPortions().empty(),
            "CreateTextPortions: Nothing to delete!");
    if ( nInvPortion && ( nPortionStart+pTEParaPortion->GetTextPortions()[nInvPortion]->GetLen() > nStartPos ) )
    {
        // better one before...
        // But only if it was within the Portion; otherwise it might be
        // the only one in the previous line!
        nInvPortion--;
        nPortionStart = nPortionStart - pTEParaPortion->GetTextPortions()[nInvPortion]->GetLen();
    }
    pTEParaPortion->GetTextPortions().DeleteFromPortion( nInvPortion );

    // a Portion might have been created by a line break
    aPositions.insert( nPortionStart );

    aPositionsIt = aPositions.find( nPortionStart );
    DBG_ASSERT( aPositionsIt != aPositions.end(), "CreateTextPortions: nPortionStart not found" );

    if ( aPositionsIt != aPositions.end() )
    {
        std::set<sal_uInt16>::iterator nextIt = aPositionsIt;
        for ( ++nextIt; nextIt != aPositions.end(); ++aPositionsIt, ++nextIt )
        {
            TETextPortion* pNew = new TETextPortion( *nextIt - *aPositionsIt );
            pTEParaPortion->GetTextPortions().push_back( pNew );
        }
    }
    OSL_ENSURE(pTEParaPortion->GetTextPortions().size(), "CreateTextPortions: No Portions?!");
}

void TextEngine::RecalcTextPortion( sal_uLong nPara, sal_uInt16 nStartPos, short nNewChars )
{
    TEParaPortion* pTEParaPortion = mpTEParaPortions->GetObject( nPara );
    OSL_ENSURE(pTEParaPortion->GetTextPortions().size(), "RecalcTextPortion: no Portions!");
    OSL_ENSURE(nNewChars, "RecalcTextPortion: Diff == 0");

    TextNode* const pNode = pTEParaPortion->GetNode();
    if ( nNewChars > 0 )
    {
        // If an Attribute is starting/ending at nStartPos, or there is a tab
        // before nStartPos => a new Portion starts.
        // Otherwise the Portion is extended at nStartPos.
        // Or if at the very beginning ( StartPos 0 ) followed by a tab...
        if ( ( pNode->GetCharAttribs().HasBoundingAttrib( nStartPos ) ) ||
             ( nStartPos && ( pNode->GetText()[ nStartPos - 1 ] == '\t' ) ) ||
             ( ( !nStartPos && ( nNewChars < pNode->GetText().getLength() ) && pNode->GetText()[ nNewChars ] == '\t' ) ) )
        {
            sal_uInt16 nNewPortionPos = 0;
            if ( nStartPos )
                nNewPortionPos = SplitTextPortion( nPara, nStartPos ) + 1;

           // Here could be an empty Portion if the paragraph was empty,
            // or a new line was created by a hard line-break.
            if ( ( nNewPortionPos < pTEParaPortion->GetTextPortions().size() ) &&
                    !pTEParaPortion->GetTextPortions()[nNewPortionPos]->GetLen() )
            {
                // use the empty Portion
                sal_uInt16 & r =
                    pTEParaPortion->GetTextPortions()[nNewPortionPos]->GetLen();
                r = r + nNewChars;
            }
            else
            {
                TETextPortion* pNewPortion = new TETextPortion( nNewChars );
                pTEParaPortion->GetTextPortions().insert( pTEParaPortion->GetTextPortions().begin() + nNewPortionPos, pNewPortion );
            }
        }
        else
        {
            sal_uInt16 nPortionStart;
            const sal_uInt16 nTP = pTEParaPortion->GetTextPortions().
                FindPortion( nStartPos, nPortionStart );
            TETextPortion* const pTP = pTEParaPortion->GetTextPortions()[ nTP ];
            DBG_ASSERT( pTP, "RecalcTextPortion: Portion not found!"  );
            pTP->GetLen() = pTP->GetLen() + nNewChars;
            pTP->GetWidth() = (-1);
        }
    }
    else
    {
        // Shrink or remove Portion
        // Before calling this function, ensure that no Portions were in the deleted range!

        // There must be no Portion reaching into or starting within,
        // thus: nStartPos <= nPos <= nStartPos - nNewChars(neg.)
        sal_uInt16 nPortion = 0;
        sal_uInt16 nPos = 0;
        sal_uInt16 nEnd = nStartPos-nNewChars;
        sal_uInt16 nPortions = pTEParaPortion->GetTextPortions().size();
        TETextPortion* pTP = 0;
        for ( nPortion = 0; nPortion < nPortions; nPortion++ )
        {
            pTP = pTEParaPortion->GetTextPortions()[ nPortion ];
            if ( ( nPos+pTP->GetLen() ) > nStartPos )
            {
                DBG_ASSERT( nPos <= nStartPos, "RecalcTextPortion: Bad Start!" );
                DBG_ASSERT( nPos+pTP->GetLen() >= nEnd, "RecalcTextPortion: Bad End!" );
                break;
            }
            nPos = nPos + pTP->GetLen();
        }
        DBG_ASSERT( pTP, "RecalcTextPortion: Portion not found!" );
        if ( ( nPos == nStartPos ) && ( (nPos+pTP->GetLen()) == nEnd ) )
        {
            // remove Portion
            pTEParaPortion->GetTextPortions().erase( pTEParaPortion->GetTextPortions().begin() + nPortion );
            delete pTP;
        }
        else
        {
            DBG_ASSERT( pTP->GetLen() > (-nNewChars), "RecalcTextPortion: Portion too small to shrink!" );
            pTP->GetLen() = pTP->GetLen() + nNewChars;
        }
        OSL_ENSURE( pTEParaPortion->GetTextPortions().size(),
                "RecalcTextPortion: none are left!" );
    }
}

void TextEngine::ImpPaint( OutputDevice* pOutDev, const Point& rStartPos, Rectangle const* pPaintArea, TextSelection const* pPaintRange, TextSelection const* pSelection )
{
    if ( !GetUpdateMode() )
        return;

    if ( !IsFormatted() )
        FormatDoc();

    Window* pOutWin = dynamic_cast<Window*>(pOutDev);
    bool bTransparent = (pOutWin && pOutWin->IsPaintTransparent());

    long nY = rStartPos.Y();

    TextPaM const* pSelStart = 0;
    TextPaM const* pSelEnd = 0;
    if ( pSelection && pSelection->HasRange() )
    {
        bool bInvers = pSelection->GetEnd() < pSelection->GetStart();
        pSelStart = !bInvers ? &pSelection->GetStart() : &pSelection->GetEnd();
        pSelEnd = bInvers ? &pSelection->GetStart() : &pSelection->GetEnd();
    }
    DBG_ASSERT( !pPaintRange || ( pPaintRange->GetStart() < pPaintRange->GetEnd() ), "ImpPaint: Paint-Range?!" );

    const StyleSettings& rStyleSettings = pOutDev->GetSettings().GetStyleSettings();

    // for all paragraphs
    for ( sal_uLong nPara = 0; nPara < mpTEParaPortions->Count(); nPara++ )
    {
        TEParaPortion* pPortion = mpTEParaPortions->GetObject( nPara );
        // in case while typing Idle-Formatting, asynchronous Paint
        if ( pPortion->IsInvalid() )
            return;

        sal_uLong nParaHeight = CalcParaHeight( nPara );
        if ( ( !pPaintArea || ( ( nY + (long)nParaHeight ) > pPaintArea->Top() ) )
                && ( !pPaintRange || ( ( nPara >= pPaintRange->GetStart().GetPara() ) && ( nPara <= pPaintRange->GetEnd().GetPara() ) ) ) )
        {
            // for all lines of the paragraph
            sal_uInt16 nLines = pPortion->GetLines().size();
            sal_uInt16 nIndex = 0;
            for ( sal_uInt16 nLine = 0; nLine < nLines; nLine++ )
            {
                TextLine* pLine = pPortion->GetLines()[nLine];
                Point aTmpPos( rStartPos.X() + pLine->GetStartX(), nY );

                if ( ( !pPaintArea || ( ( nY + mnCharHeight ) > pPaintArea->Top() ) )
                    && ( !pPaintRange || (
                        ( TextPaM( nPara, pLine->GetStart() ) < pPaintRange->GetEnd() ) &&
                        ( TextPaM( nPara, pLine->GetEnd() ) > pPaintRange->GetStart() ) ) ) )
                {
                    // for all Portions of the line
                    nIndex = pLine->GetStart();
                    for ( sal_uInt16 y = pLine->GetStartPortion(); y <= pLine->GetEndPortion(); y++ )
                    {
                        OSL_ENSURE(pPortion->GetTextPortions().size(),
                                "ImpPaint: Line without Textportion!");
                        TETextPortion* pTextPortion = pPortion->GetTextPortions()[ y ];
                        DBG_ASSERT( pTextPortion, "ImpPaint: Bad pTextPortion!" );

                        ImpInitLayoutMode( pOutDev /*, pTextPortion->IsRightToLeft() */);

                        long nTxtWidth = pTextPortion->GetWidth();
                        aTmpPos.X() = rStartPos.X() + ImpGetOutputOffset( nPara, pLine, nIndex, nIndex );

                        // only print if starting in the visible region
                        if ( ( ( aTmpPos.X() + nTxtWidth ) >= 0 )
                            && ( !pPaintRange || (
                                ( TextPaM( nPara, nIndex ) < pPaintRange->GetEnd() ) &&
                                    ( TextPaM( nPara, nIndex + pTextPortion->GetLen() ) > pPaintRange->GetStart() ) ) ) )
                        {
                            switch ( pTextPortion->GetKind() )
                            {
                                case PORTIONKIND_TEXT:
                                {
                                    {
                                        Font aFont;
                                        SeekCursor( nPara, nIndex+1, aFont, pOutDev );
                                        if( bTransparent )
                                            aFont.SetTransparent( sal_True );
                                        else if ( pSelection )
                                            aFont.SetTransparent( sal_False );
                                        pOutDev->SetFont( aFont );

                                        sal_uInt16 nTmpIndex = nIndex;
                                        sal_uInt16 nEnd = nTmpIndex + pTextPortion->GetLen();
                                        Point aPos = aTmpPos;
                                        if ( pPaintRange )
                                        {
                                            // maybe not print all of it
                                            if ( ( pPaintRange->GetStart().GetPara() == nPara )
                                                    && ( nTmpIndex < pPaintRange->GetStart().GetIndex() ) )
                                            {
                                                nTmpIndex = pPaintRange->GetStart().GetIndex();
                                            }
                                            if ( ( pPaintRange->GetEnd().GetPara() == nPara )
                                                    && ( nEnd > pPaintRange->GetEnd().GetIndex() ) )
                                            {
                                                nEnd = pPaintRange->GetEnd().GetIndex();
                                            }
                                        }

                                        bool bDone = false;
                                        if ( pSelStart )
                                        {
                                            // is a part of it in the selection?
                                            TextPaM aTextStart( nPara, nTmpIndex );
                                            TextPaM aTextEnd( nPara, nEnd );
                                            if ( ( aTextStart < *pSelEnd ) && ( aTextEnd > *pSelStart ) )
                                            {
                                                sal_uInt16 nL;

                                                // 1) Region before Selection
                                                if ( aTextStart < *pSelStart )
                                                {
                                                    nL = pSelStart->GetIndex() - nTmpIndex;
                                                    pOutDev->SetFont( aFont);
                                                    aPos.X() = rStartPos.X() + ImpGetOutputOffset( nPara, pLine, nTmpIndex, nTmpIndex+nL );
                                                    pOutDev->DrawText( aPos, pPortion->GetNode()->GetText(), nTmpIndex, nL );
                                                    nTmpIndex = nTmpIndex + nL;

                                                }
                                                // 2) Region with Selection
                                                nL = nEnd-nTmpIndex;
                                                if ( aTextEnd > *pSelEnd )
                                                    nL = pSelEnd->GetIndex() - nTmpIndex;
                                                if ( nL )
                                                {
                                                    Color aOldTextColor = pOutDev->GetTextColor();
                                                    pOutDev->SetTextColor( rStyleSettings.GetHighlightTextColor() );
                                                    pOutDev->SetTextFillColor( rStyleSettings.GetHighlightColor() );
                                                    aPos.X() = rStartPos.X() + ImpGetOutputOffset( nPara, pLine, nTmpIndex, nTmpIndex+nL );
                                                    pOutDev->DrawText( aPos, pPortion->GetNode()->GetText(), nTmpIndex, nL );
                                                    pOutDev->SetTextColor( aOldTextColor );
                                                    pOutDev->SetTextFillColor();
                                                    nTmpIndex = nTmpIndex + nL;
                                                }

                                                // 3) Region after Selection
                                                if ( nTmpIndex < nEnd )
                                                {
                                                    nL = nEnd-nTmpIndex;
                                                    aPos.X() = rStartPos.X() + ImpGetOutputOffset( nPara, pLine, nTmpIndex, nTmpIndex+nL );
                                                    pOutDev->DrawText( aPos, pPortion->GetNode()->GetText(), nTmpIndex, nEnd-nTmpIndex );
                                                }
                                                bDone = true;
                                            }
                                        }
                                        if ( !bDone )
                                        {
                                            aPos.X() = rStartPos.X() + ImpGetOutputOffset( nPara, pLine, nTmpIndex, nEnd );
                                            pOutDev->DrawText( aPos, pPortion->GetNode()->GetText(), nTmpIndex, nEnd-nTmpIndex );
                                        }
                                    }

                                }
                                break;
                                case PORTIONKIND_TAB:
                                {
                                    // for HideSelection() only Range, pSelection = 0.
                                    if ( pSelStart || pPaintRange )
                                    {
                                        Rectangle aTabArea( aTmpPos, Point( aTmpPos.X()+nTxtWidth, aTmpPos.Y()+mnCharHeight-1 ) );
                                        bool bDone = false;
                                        if ( pSelStart )
                                        {
                                            // is the Tab in the Selection???
                                            TextPaM aTextStart( nPara, nIndex );
                                            TextPaM aTextEnd( nPara, nIndex+1 );
                                            if ( ( aTextStart < *pSelEnd ) && ( aTextEnd > *pSelStart ) )
                                            {
                                                Color aOldColor = pOutDev->GetFillColor();
                                                pOutDev->SetFillColor( rStyleSettings.GetHighlightColor() );
                                                pOutDev->DrawRect( aTabArea );
                                                pOutDev->SetFillColor( aOldColor );
                                                bDone = true;
                                            }
                                        }
                                        if ( !bDone )
                                        {
                                            pOutDev->Erase( aTabArea );
                                        }
                                    }
                                }
                                break;
                                default:    OSL_FAIL( "ImpPaint: Unknown Portion-Type !" );
                            }
                        }

                        nIndex = nIndex + pTextPortion->GetLen();
                    }
                }

                nY += mnCharHeight;

                if ( pPaintArea && ( nY >= pPaintArea->Bottom() ) )
                    break;  // no more visible actions
            }
        }
        else
        {
            nY += nParaHeight;
        }

        if ( pPaintArea && ( nY > pPaintArea->Bottom() ) )
            break;  // no more visible actions
    }
}

sal_Bool TextEngine::CreateLines( sal_uLong nPara )
{
    // sal_Bool: changing Height of Paragraph Yes/No - sal_True/sal_False

    TextNode* pNode = mpDoc->GetNodes().GetObject( nPara );
    TEParaPortion* pTEParaPortion = mpTEParaPortions->GetObject( nPara );
    DBG_ASSERT( pTEParaPortion->IsInvalid(), "CreateLines: Portion not invalid!" );

    sal_uInt16 nOldLineCount = pTEParaPortion->GetLines().size();

    // fast special case for empty paragraphs
    if ( pTEParaPortion->GetNode()->GetText().isEmpty() )
    {
        if ( !pTEParaPortion->GetTextPortions().empty() )
            pTEParaPortion->GetTextPortions().Reset();
        if ( !pTEParaPortion->GetLines().empty() )
        {
            BOOST_FOREACH(TextLine* pLine, pTEParaPortion->GetLines())
                delete pLine;
            pTEParaPortion->GetLines().clear();
        }
        CreateAndInsertEmptyLine( nPara );
        pTEParaPortion->SetValid();
        return nOldLineCount != pTEParaPortion->GetLines().size();
    }

    // initialization
    if ( pTEParaPortion->GetLines().empty() )
    {
        TextLine* pL = new TextLine;
        pTEParaPortion->GetLines().push_back( pL );
    }

    const short nInvalidDiff = pTEParaPortion->GetInvalidDiff();
    const sal_uInt16 nInvalidStart = pTEParaPortion->GetInvalidPosStart();
    const sal_uInt16 nInvalidEnd =  nInvalidStart + std::abs( nInvalidDiff );
    bool bQuickFormat = false;

    if ( pTEParaPortion->GetWritingDirectionInfos().empty() )
        ImpInitWritingDirections( nPara );

    if ( pTEParaPortion->GetWritingDirectionInfos().size() == 1 )
    {
        if ( pTEParaPortion->IsSimpleInvalid() && ( nInvalidDiff > 0 ) )
        {
            bQuickFormat = true;
        }
        else if ( ( pTEParaPortion->IsSimpleInvalid() ) && ( nInvalidDiff < 0 ) )
        {
            // check if deleting across Portion border
            sal_uInt16 nStart = nInvalidStart;  // duplicate!!!
            sal_uInt16 nEnd = nStart - nInvalidDiff;  // neg.
            bQuickFormat = true;
            sal_uInt16 nPos = 0;
            sal_uInt16 nPortions = pTEParaPortion->GetTextPortions().size();
            for ( sal_uInt16 nTP = 0; nTP < nPortions; nTP++ )
            {
                // there must be no Start/End in the deleted region
                TETextPortion* const pTP = pTEParaPortion->GetTextPortions()[ nTP ];
                nPos = nPos + pTP->GetLen();
                if ( ( nPos > nStart ) && ( nPos < nEnd ) )
                {
                    bQuickFormat = false;
                    break;
                }
            }
        }
    }

    if ( bQuickFormat )
        RecalcTextPortion( nPara, nInvalidStart, nInvalidDiff );
    else
        CreateTextPortions( nPara, nInvalidStart );

    // search for line with InvalidPos; start a line prior
    // flag lines => do not remove!

    sal_uInt16 nLine = pTEParaPortion->GetLines().size()-1;
    for ( sal_uInt16 nL = 0; nL <= nLine; nL++ )
    {
        TextLine* pLine = pTEParaPortion->GetLines()[ nL ];
        if ( pLine->GetEnd() > nInvalidStart )
        {
            nLine = nL;
            break;
        }
        pLine->SetValid();
    }
    // start a line before...
    // if typing at the end, the line before cannot change
    if ( nLine && ( !pTEParaPortion->IsSimpleInvalid() || ( nInvalidEnd < pNode->GetText().getLength() ) || ( nInvalidDiff <= 0 ) ) )
        nLine--;

    TextLine* pLine = pTEParaPortion->GetLines()[ nLine ];

    // format all lines starting here
    size_t nDelFromLine = std::numeric_limits<size_t>::max();
    bool bLineBreak = false;

    sal_uInt16 nIndex = pLine->GetStart();
    TextLine aSaveLine( *pLine );

    Font aFont;

    bool bCalcPortion = true;

    while ( nIndex < pNode->GetText().getLength() )
    {
        bool bEOL = false;
        sal_uInt16 nPortionStart = 0;
        sal_uInt16 nPortionEnd = 0;

        sal_uInt16 nTmpPos = nIndex;
        sal_uInt16 nTmpPortion = pLine->GetStartPortion();
        long nTmpWidth = mpDoc->GetLeftMargin();
        // do not subtract margin; it is included in TmpWidth
        long nXWidth = mnMaxTextWidth ? mnMaxTextWidth : 0x7FFFFFFF;
        if ( nXWidth < nTmpWidth )
            nXWidth = nTmpWidth;

        // search for Portion that does not fit anymore into line
        TETextPortion* pPortion = 0;
        bool bBrokenLine = false;
        bLineBreak = false;

        while ( ( nTmpWidth <= nXWidth ) && !bEOL && ( nTmpPortion < pTEParaPortion->GetTextPortions().size() ) )
        {
            nPortionStart = nTmpPos;
            pPortion = pTEParaPortion->GetTextPortions()[ nTmpPortion ];
            DBG_ASSERT( pPortion->GetLen(), "CreateLines: Empty Portion!" );
            if ( pNode->GetText()[ nTmpPos ] == '\t' )
            {
                long nCurPos = nTmpWidth-mpDoc->GetLeftMargin();
                nTmpWidth = ((nCurPos/mnDefTab)+1)*mnDefTab+mpDoc->GetLeftMargin();
                pPortion->GetWidth() = nTmpWidth - nCurPos - mpDoc->GetLeftMargin();
                // infinite loop, if this is the first token of the line and nTmpWidth > aPaperSize.Width !!!
                if ( ( nTmpWidth >= nXWidth ) && ( nTmpPortion == pLine->GetStartPortion() ) )
                {
                    // adjust Tab
                    pPortion->GetWidth() = nXWidth-1;
                    nTmpWidth = pPortion->GetWidth();
                    bEOL = true;
                    bBrokenLine = true;
                }
                pPortion->GetKind() = PORTIONKIND_TAB;
            }
            else
            {

                if ( bCalcPortion || !pPortion->HasValidSize() )
                    pPortion->GetWidth() = (long)CalcTextWidth( nPara, nTmpPos, pPortion->GetLen() );
                nTmpWidth += pPortion->GetWidth();

                pPortion->GetRightToLeft() = ImpGetRightToLeft( nPara, nTmpPos+1 );
                pPortion->GetKind() = PORTIONKIND_TEXT;
            }

            nTmpPos = nTmpPos + pPortion->GetLen();
            nPortionEnd = nTmpPos;
            nTmpPortion++;
        }

        // this was perhaps one Portion too far
        bool bFixedEnd = false;
        if ( nTmpWidth > nXWidth )
        {
            nPortionEnd = nTmpPos;
            nTmpPos = nTmpPos - pPortion->GetLen();
            nPortionStart = nTmpPos;
            nTmpPortion--;
            bEOL = false;

            nTmpWidth -= pPortion->GetWidth();
            if ( pPortion->GetKind() == PORTIONKIND_TAB )
            {
                bEOL = true;
                bFixedEnd = true;
            }
        }
        else
        {
            bEOL = true;
            pLine->SetEnd( nPortionEnd );
            OSL_ENSURE(pTEParaPortion->GetTextPortions().size(),
                    "CreateLines: No TextPortions?");
            pLine->SetEndPortion( (sal_uInt16)pTEParaPortion->GetTextPortions().size() - 1 );
        }

        if ( bFixedEnd )
        {
            pLine->SetEnd( nPortionStart );
            pLine->SetEndPortion( nTmpPortion-1 );
        }
        else if ( bLineBreak || bBrokenLine )
        {
            pLine->SetEnd( nPortionStart+1 );
            pLine->SetEndPortion( nTmpPortion-1 );
        }
        else if ( !bEOL )
        {
            DBG_ASSERT( (nPortionEnd-nPortionStart) == pPortion->GetLen(), "CreateLines: There is a Portion after all?!" );
            long nRemainingWidth = mnMaxTextWidth - nTmpWidth;
            ImpBreakLine( nPara, pLine, pPortion, nPortionStart, nRemainingWidth );
        }

        if ( ( ImpGetAlign() == TXTALIGN_CENTER ) || ( ImpGetAlign() == TXTALIGN_RIGHT ) )
        {
            // adjust
            long nTextWidth = 0;
            for ( sal_uInt16 nTP = pLine->GetStartPortion(); nTP <= pLine->GetEndPortion(); nTP++ )
            {
                TETextPortion* pTextPortion = pTEParaPortion->GetTextPortions()[ nTP ];
                nTextWidth += pTextPortion->GetWidth();
            }
            long nSpace = mnMaxTextWidth - nTextWidth;
            if ( nSpace > 0 )
            {
                if ( ImpGetAlign() == TXTALIGN_CENTER )
                    pLine->SetStartX( (sal_uInt16)(nSpace / 2) );
                else    // TXTALIGN_RIGHT
                    pLine->SetStartX( (sal_uInt16)nSpace );
            }
        }
        else
        {
            pLine->SetStartX( mpDoc->GetLeftMargin() );
        }

         // check if the line has to be printed again
        pLine->SetInvalid();

        if ( pTEParaPortion->IsSimpleInvalid() )
        {
            // Change due to simple TextChange...
            // Do not abort formatting, as Portions might have to be split!
            // Once it is ok to abort, then validate the following lines!
            // But mark as valid, thus reduce printing...
            if ( pLine->GetEnd() < nInvalidStart )
            {
                if ( *pLine == aSaveLine )
                {
                    pLine->SetValid();
                }
            }
            else
            {
                sal_uInt16 nStart = pLine->GetStart();
                sal_uInt16 nEnd = pLine->GetEnd();

                if ( nStart > nInvalidEnd )
                {
                    if ( ( ( nStart-nInvalidDiff ) == aSaveLine.GetStart() ) &&
                            ( ( nEnd-nInvalidDiff ) == aSaveLine.GetEnd() ) )
                    {
                        pLine->SetValid();
                        if ( bCalcPortion && bQuickFormat )
                        {
                            bCalcPortion = false;
                            pTEParaPortion->CorrectValuesBehindLastFormattedLine( nLine );
                            break;
                        }
                    }
                }
                else if ( bQuickFormat && ( nEnd > nInvalidEnd) )
                {
                    // If the invalid line ends such that the next line starts
                    // at the 'same' position as before (no change in line breaks),
                    // the text width does not have to be recalculated.
                    if ( nEnd == ( aSaveLine.GetEnd() + nInvalidDiff ) )
                    {
                        bCalcPortion = false;
                        pTEParaPortion->CorrectValuesBehindLastFormattedLine( nLine );
                        break;
                    }
                }
            }
        }

        nIndex = pLine->GetEnd();   // next line Start = previous line End
                                    // because nEnd is past the last char!

        sal_uInt16 nEndPortion = pLine->GetEndPortion();

        // next line or new line
        pLine = 0;
        if ( nLine < pTEParaPortion->GetLines().size()-1 )
            pLine = pTEParaPortion->GetLines()[ ++nLine ];
        if ( pLine && ( nIndex >= pNode->GetText().getLength() ) )
        {
            nDelFromLine = nLine;
            break;
        }
        if ( !pLine && ( nIndex < pNode->GetText().getLength() )  )
        {
            pLine = new TextLine;
            pTEParaPortion->GetLines().insert( pTEParaPortion->GetLines().begin() + ++nLine, pLine );
        }
        if ( pLine )
        {
            aSaveLine = *pLine;
            pLine->SetStart( nIndex );
            pLine->SetEnd( nIndex );
            pLine->SetStartPortion( nEndPortion+1 );
            pLine->SetEndPortion( nEndPortion+1 );
        }
    }   // while ( Index < Len )

    if (nDelFromLine != std::numeric_limits<size_t>::max())
    {
        for( TextLines::iterator it = pTEParaPortion->GetLines().begin() + nDelFromLine;
             it != pTEParaPortion->GetLines().end(); ++it )
        {
            delete *it;
        }
        pTEParaPortion->GetLines().erase( pTEParaPortion->GetLines().begin() + nDelFromLine,
                                          pTEParaPortion->GetLines().end() );
    }

    DBG_ASSERT( pTEParaPortion->GetLines().size(), "CreateLines: No Line!" );

    if ( bLineBreak )
        CreateAndInsertEmptyLine( nPara );

    pTEParaPortion->SetValid();

    return nOldLineCount != pTEParaPortion->GetLines().size();
}

OUString TextEngine::GetWord( const TextPaM& rCursorPos, TextPaM* pStartOfWord )
{
    OUString aWord;
    if ( rCursorPos.GetPara() < mpDoc->GetNodes().Count() )
    {
        TextSelection aSel( rCursorPos );
        TextNode* pNode = mpDoc->GetNodes().GetObject(  rCursorPos.GetPara() );
        uno::Reference < i18n::XBreakIterator > xBI = GetBreakIterator();
        i18n::Boundary aBoundary = xBI->getWordBoundary( pNode->GetText(), rCursorPos.GetIndex(), GetLocale(), i18n::WordType::ANYWORD_IGNOREWHITESPACES, sal_True );
        aSel.GetStart().GetIndex() = (sal_uInt16)aBoundary.startPos;
        aSel.GetEnd().GetIndex() = (sal_uInt16)aBoundary.endPos;
        aWord = pNode->GetText().copy( aSel.GetStart().GetIndex(), aSel.GetEnd().GetIndex() - aSel.GetStart().GetIndex() );
        if ( pStartOfWord )
            *pStartOfWord = aSel.GetStart();
    }
    return aWord;
}

sal_Bool TextEngine::Read( SvStream& rInput, const TextSelection* pSel )
{
    sal_Bool bUpdate = GetUpdateMode();
    SetUpdateMode( sal_False );

    UndoActionStart();
    TextSelection aSel;
    if ( pSel )
        aSel = *pSel;
    else
    {
        sal_uLong nParas = mpDoc->GetNodes().Count();
        TextNode* pNode = mpDoc->GetNodes().GetObject( nParas - 1 );
        aSel = TextPaM( nParas-1 , pNode->GetText().getLength() );
    }

    if ( aSel.HasRange() )
        aSel = ImpDeleteText( aSel );

    OString aLine;
    sal_Bool bDone = rInput.ReadLine( aLine );
    OUString aTmpStr(OStringToOUString(aLine, rInput.GetStreamCharSet()));
    while ( bDone )
    {
        aSel = ImpInsertText( aSel, aTmpStr );
        bDone = rInput.ReadLine( aLine );
        aTmpStr = OStringToOUString(aLine, rInput.GetStreamCharSet());
        if ( bDone )
            aSel = ImpInsertParaBreak( aSel.GetEnd() );
    }

    UndoActionEnd();

    TextSelection aNewSel( aSel.GetEnd(), aSel.GetEnd() );

    // so that FormatAndUpdate does not access the invalid selection
    if ( GetActiveView() )
        GetActiveView()->ImpSetSelection( aNewSel );

    SetUpdateMode( bUpdate );
    FormatAndUpdate( GetActiveView() );

    return rInput.GetError() ? sal_False : sal_True;
}

sal_Bool TextEngine::Write( SvStream& rOutput, const TextSelection* pSel, sal_Bool bHTML )
{
    TextSelection aSel;
    if ( pSel )
        aSel = *pSel;
    else
    {
        sal_uLong nParas = mpDoc->GetNodes().Count();
        TextNode* pNode = mpDoc->GetNodes().GetObject( nParas - 1 );
        aSel.GetStart() = TextPaM( 0, 0 );
        aSel.GetEnd() = TextPaM( nParas-1, pNode->GetText().getLength() );
    }

    if ( bHTML )
    {
        rOutput.WriteLine( "<HTML>" );
        rOutput.WriteLine( "<BODY>" );
    }

    for ( sal_uLong nPara = aSel.GetStart().GetPara(); nPara <= aSel.GetEnd().GetPara(); nPara++  )
    {
        TextNode* pNode = mpDoc->GetNodes().GetObject( nPara );

        sal_uInt16 nStartPos = 0;
        sal_Int32  nEndPos = pNode->GetText().getLength();
        if ( nPara == aSel.GetStart().GetPara() )
            nStartPos = aSel.GetStart().GetIndex();
        if ( nPara == aSel.GetEnd().GetPara() )
            nEndPos = aSel.GetEnd().GetIndex();

        OUStringBuffer aText;
        if ( !bHTML )
        {
            aText = OUString( pNode->GetText().copy( nStartPos, nEndPos-nStartPos ) );
        }
        else
        {
            aText = "<P STYLE=\"margin-bottom: 0cm\">";

            if ( nStartPos == nEndPos )
            {
                // Empty lines will be removed by Writer
                aText.append( "<BR>" );
            }
            else
            {
                sal_uInt16 nTmpStart = nStartPos;
                sal_uInt16 nTmpEnd = nEndPos;
                do
                {
                    TextCharAttrib* pAttr = pNode->GetCharAttribs().FindNextAttrib( TEXTATTR_HYPERLINK, nTmpStart, nEndPos );
                    nTmpEnd = pAttr ? pAttr->GetStart() : nEndPos;

                    // Text before Attribute
                    aText.append( OUString( pNode->GetText().copy( nTmpStart, nTmpEnd-nTmpStart ) ) );

                    if ( pAttr )
                    {
                        nTmpEnd = std::min( pAttr->GetEnd(), (sal_uInt16) nEndPos );

                        // e.g. <A HREF="http://www.mopo.de/">Morgenpost</A>
                        aText.append( "<A HREF=\"" );
                        aText.append( ((const TextAttribHyperLink&) pAttr->GetAttr() ).GetURL() );
                        aText.append( "\">" );
                        nTmpStart = pAttr->GetStart();
                        aText.append( pNode->GetText().copy( nTmpStart, nTmpEnd-nTmpStart ) );
                        aText.append( "</A>" );

                        nTmpStart = pAttr->GetEnd();
                    }
                } while ( nTmpEnd < nEndPos );
            }

            aText.append( "</P>" );
        }
        rOutput.WriteLine(OUStringToOString(aText.makeStringAndClear(),
            rOutput.GetStreamCharSet()));
    }

    if ( bHTML )
    {
        rOutput.WriteLine( "</BODY>" );
        rOutput.WriteLine( "</HTML>" );
    }

    return rOutput.GetError() ? sal_False : sal_True;
}

void TextEngine::RemoveAttribs( sal_uLong nPara, sal_Bool bIdleFormatAndUpdate )
{
    if ( nPara < mpDoc->GetNodes().Count() )
    {
        TextNode* pNode = mpDoc->GetNodes().GetObject( nPara );
        if ( pNode->GetCharAttribs().Count() )
        {
            pNode->GetCharAttribs().Clear( sal_True );

            TEParaPortion* pTEParaPortion = mpTEParaPortions->GetObject( nPara );
            pTEParaPortion->MarkSelectionInvalid( 0, pNode->GetText().getLength() );

            mbFormatted = sal_False;

            if ( bIdleFormatAndUpdate )
                IdleFormatAndUpdate( NULL, 0xFFFF );
            else
                FormatAndUpdate( NULL );
        }
    }
}
void TextEngine::RemoveAttribs( sal_uLong nPara, sal_uInt16 nWhich, sal_Bool bIdleFormatAndUpdate )
{
    if ( nPara < mpDoc->GetNodes().Count() )
    {
        TextNode* pNode = mpDoc->GetNodes().GetObject( nPara );
        if ( pNode->GetCharAttribs().Count() )
        {
            TextCharAttribList& rAttribs = pNode->GetCharAttribs();
            sal_uInt16 nAttrCount = rAttribs.Count();
            for(sal_uInt16 nAttr = nAttrCount; nAttr; --nAttr)
            {
                if(rAttribs.GetAttrib( nAttr - 1 )->Which() == nWhich)
                    rAttribs.RemoveAttrib( nAttr -1 );
            }
            TEParaPortion* pTEParaPortion = mpTEParaPortions->GetObject( nPara );
            pTEParaPortion->MarkSelectionInvalid( 0, pNode->GetText().getLength() );
            mbFormatted = sal_False;
            if(bIdleFormatAndUpdate)
                IdleFormatAndUpdate( NULL, 0xFFFF );
            else
                FormatAndUpdate( NULL );
        }
    }
}
void TextEngine::RemoveAttrib( sal_uLong nPara, const TextCharAttrib& rAttrib )
{
    if ( nPara < mpDoc->GetNodes().Count() )
    {
        TextNode* pNode = mpDoc->GetNodes().GetObject( nPara );
        if ( pNode->GetCharAttribs().Count() )
        {
            TextCharAttribList& rAttribs = pNode->GetCharAttribs();
            sal_uInt16 nAttrCount = rAttribs.Count();
            for(sal_uInt16 nAttr = nAttrCount; nAttr; --nAttr)
            {
                if(rAttribs.GetAttrib( nAttr - 1 ) == &rAttrib)
                {
                    rAttribs.RemoveAttrib( nAttr -1 );
                    break;
                }
            }
            TEParaPortion* pTEParaPortion = mpTEParaPortions->GetObject( nPara );
            pTEParaPortion->MarkSelectionInvalid( 0, pNode->GetText().getLength() );
            mbFormatted = sal_False;
            FormatAndUpdate( NULL );
        }
    }
}

void TextEngine::SetAttrib( const TextAttrib& rAttr, sal_uLong nPara, sal_uInt16 nStart, sal_uInt16 nEnd, sal_Bool bIdleFormatAndUpdate )
{

    // For now do not check if Attributes overlap!
    // This function is for TextEditors that want to _quickly_ generate the Syntax-Highlight

    // As TextEngine is currently intended only for TextEditors, there is no Undo for Attributes!

    if ( nPara < mpDoc->GetNodes().Count() )
    {
        TextNode* pNode = mpDoc->GetNodes().GetObject( nPara );
        TEParaPortion* pTEParaPortion = mpTEParaPortions->GetObject( nPara );

        sal_Int32 nMax = pNode->GetText().getLength();
        if ( nStart > nMax )
            nStart = nMax;
        if ( nEnd > nMax )
            nEnd = nMax;

        pNode->GetCharAttribs().InsertAttrib( new TextCharAttrib( rAttr, nStart, nEnd ) );
        pTEParaPortion->MarkSelectionInvalid( nStart, nEnd );

        mbFormatted = sal_False;
        if ( bIdleFormatAndUpdate )
            IdleFormatAndUpdate( NULL, 0xFFFF );
        else
            FormatAndUpdate( NULL );
    }
}

void TextEngine::SetTextAlign( TxtAlign eAlign )
{
    if ( eAlign != meAlign )
    {
        meAlign = eAlign;
        FormatFullDoc();
        UpdateViews();
    }
}


void TextEngine::ValidateSelection( TextSelection& rSel ) const
{
    ValidatePaM( rSel.GetStart() );
    ValidatePaM( rSel.GetEnd() );
}

void TextEngine::ValidatePaM( TextPaM& rPaM ) const
{
    sal_uLong nMaxPara = mpDoc->GetNodes().Count() - 1;
    if ( rPaM.GetPara() > nMaxPara )
    {
        rPaM.GetPara() = nMaxPara;
        rPaM.GetIndex() = 0xFFFF;
    }

    sal_uInt16 nMaxIndex = GetTextLen( rPaM.GetPara() );
    if ( rPaM.GetIndex() > nMaxIndex )
        rPaM.GetIndex() = nMaxIndex;
}


// adjust State & Selection

void TextEngine::ImpParagraphInserted( sal_uLong nPara )
{
    // No adjustment needed for the active View;
    // but for all passive Views the Selection needs adjusting.
    if ( mpViews->size() > 1 )
    {
        for ( sal_uInt16 nView = mpViews->size(); nView; )
        {
            TextView* pView = (*mpViews)[ --nView ];
            if ( pView != GetActiveView() )
            {
                for ( int n = 0; n <= 1; n++ )
                {
                    TextPaM& rPaM = n ? pView->GetSelection().GetStart(): pView->GetSelection().GetEnd();
                    if ( rPaM.GetPara() >= nPara )
                        rPaM.GetPara()++;
                }
            }
        }
    }
    Broadcast( TextHint( TEXT_HINT_PARAINSERTED, nPara ) );
}

void TextEngine::ImpParagraphRemoved( sal_uLong nPara )
{
    if ( mpViews->size() > 1 )
    {
        for ( sal_uInt16 nView = mpViews->size(); nView; )
        {
            TextView* pView = (*mpViews)[ --nView ];
            if ( pView != GetActiveView() )
            {
                sal_uLong nParas = mpDoc->GetNodes().Count();
                for ( int n = 0; n <= 1; n++ )
                {
                    TextPaM& rPaM = n ? pView->GetSelection().GetStart(): pView->GetSelection().GetEnd();
                    if ( rPaM.GetPara() > nPara )
                        rPaM.GetPara()--;
                    else if ( rPaM.GetPara() == nPara )
                    {
                        rPaM.GetIndex() = 0;
                        if ( rPaM.GetPara() >= nParas )
                            rPaM.GetPara()--;
                    }
                }
            }
        }
    }
    Broadcast( TextHint( TEXT_HINT_PARAREMOVED, nPara ) );
}

void TextEngine::ImpCharsRemoved( sal_uLong nPara, sal_uInt16 nPos, sal_uInt16 nChars )
{
    if ( mpViews->size() > 1 )
    {
        for ( sal_uInt16 nView = mpViews->size(); nView; )
        {
            TextView* pView = (*mpViews)[ --nView ];
            if ( pView != GetActiveView() )
            {
                sal_uInt16 nEnd = nPos+nChars;
                for ( int n = 0; n <= 1; n++ )
                {
                    TextPaM& rPaM = n ? pView->GetSelection().GetStart(): pView->GetSelection().GetEnd();
                    if ( rPaM.GetPara() == nPara )
                    {
                        if ( rPaM.GetIndex() > nEnd )
                            rPaM.GetIndex() = rPaM.GetIndex() - nChars;
                        else if ( rPaM.GetIndex() > nPos )
                            rPaM.GetIndex() = nPos;
                    }
                }
            }
        }
    }
    Broadcast( TextHint( TEXT_HINT_PARACONTENTCHANGED, nPara ) );
}

void TextEngine::ImpCharsInserted( sal_uLong nPara, sal_uInt16 nPos, sal_uInt16 nChars )
{
    if ( mpViews->size() > 1 )
    {
        for ( sal_uInt16 nView = mpViews->size(); nView; )
        {
            TextView* pView = (*mpViews)[ --nView ];
            if ( pView != GetActiveView() )
            {
                for ( int n = 0; n <= 1; n++ )
                {
                    TextPaM& rPaM = n ? pView->GetSelection().GetStart(): pView->GetSelection().GetEnd();
                    if ( rPaM.GetPara() == nPara )
                    {
                        if ( rPaM.GetIndex() >= nPos )
                            rPaM.GetIndex() = rPaM.GetIndex() + nChars;
                    }
                }
            }
        }
    }
    Broadcast( TextHint( TEXT_HINT_PARACONTENTCHANGED, nPara ) );
}

void TextEngine::ImpFormattingParagraph( sal_uLong nPara )
{
    Broadcast( TextHint( TEXT_HINT_FORMATPARA, nPara ) );
}

void TextEngine::ImpTextHeightChanged()
{
    Broadcast( TextHint( TEXT_HINT_TEXTHEIGHTCHANGED ) );
}

void TextEngine::ImpTextFormatted()
{
    Broadcast( TextHint( TEXT_HINT_TEXTFORMATTED ) );
}

void TextEngine::Draw( OutputDevice* pDev, const Point& rPos )
{
    ImpPaint( pDev, rPos, NULL );
}

void TextEngine::SetLeftMargin( sal_uInt16 n )
{
    mpDoc->SetLeftMargin( n );
}

sal_uInt16 TextEngine::GetLeftMargin() const
{
    return mpDoc->GetLeftMargin();
}

uno::Reference< i18n::XBreakIterator > TextEngine::GetBreakIterator()
{
    if ( !mxBreakIterator.is() )
        mxBreakIterator = vcl::unohelper::CreateBreakIterator();
    DBG_ASSERT( mxBreakIterator.is(), "BreakIterator: Failed to create!" );
    return mxBreakIterator;
}

void TextEngine::SetLocale( const ::com::sun::star::lang::Locale& rLocale )
{
    maLocale = rLocale;
    delete mpLocaleDataWrapper;
    mpLocaleDataWrapper = NULL;
}

::com::sun::star::lang::Locale TextEngine::GetLocale()
{
    if ( maLocale.Language.isEmpty() )
    {
        maLocale = Application::GetSettings().GetUILanguageTag().getLocale();   // TODO: why UI locale?
    }
    return maLocale;
}

LocaleDataWrapper* TextEngine::ImpGetLocaleDataWrapper()
{
    if ( !mpLocaleDataWrapper )
        mpLocaleDataWrapper = new LocaleDataWrapper( LanguageTag( GetLocale()) );

    return mpLocaleDataWrapper;
}

void TextEngine::SetRightToLeft( sal_Bool bR2L )
{
    if ( mbRightToLeft != bR2L )
    {
        mbRightToLeft = bR2L;
        meAlign = bR2L ? TXTALIGN_RIGHT : TXTALIGN_LEFT;
        FormatFullDoc();
        UpdateViews();
    }
}

void TextEngine::ImpInitWritingDirections( sal_uLong nPara )
{
    TEParaPortion* pParaPortion = mpTEParaPortions->GetObject( nPara );
    std::vector<TEWritingDirectionInfo>& rInfos = pParaPortion->GetWritingDirectionInfos();
    rInfos.clear();

    if ( !pParaPortion->GetNode()->GetText().isEmpty() )
    {
        const UBiDiLevel nBidiLevel = IsRightToLeft() ? 1 /*RTL*/ : 0 /*LTR*/;
        OUString aText( pParaPortion->GetNode()->GetText() );

        //
        // Bidi functions from icu 2.0
        //
        UErrorCode nError = U_ZERO_ERROR;
        UBiDi* pBidi = ubidi_openSized( aText.getLength(), 0, &nError );
        nError = U_ZERO_ERROR;

        ubidi_setPara( pBidi, reinterpret_cast<const UChar *>(aText.getStr()), aText.getLength(), nBidiLevel, NULL, &nError ); // UChar != sal_Unicode in MinGW
        nError = U_ZERO_ERROR;

        long nCount = ubidi_countRuns( pBidi, &nError );

        int32_t nStart = 0;
        int32_t nEnd;
        UBiDiLevel nCurrDir;

        for ( sal_uInt16 nIdx = 0; nIdx < nCount; ++nIdx )
        {
            ubidi_getLogicalRun( pBidi, nStart, &nEnd, &nCurrDir );
            rInfos.push_back( TEWritingDirectionInfo( nCurrDir, (sal_uInt16)nStart, (sal_uInt16)nEnd ) );
            nStart = nEnd;
        }

        ubidi_close( pBidi );
    }

    // No infos mean no CTL and default dir is L2R...
    if ( rInfos.empty() )
        rInfos.push_back( TEWritingDirectionInfo( 0, 0, (sal_uInt16)pParaPortion->GetNode()->GetText().getLength() ) );

}

sal_uInt8 TextEngine::ImpGetRightToLeft( sal_uLong nPara, sal_uInt16 nPos, sal_uInt16* pStart, sal_uInt16* pEnd )
{
    sal_uInt8 nRightToLeft = 0;

    TextNode* pNode = mpDoc->GetNodes().GetObject( nPara );
    if ( pNode && !pNode->GetText().isEmpty() )
    {
        TEParaPortion* pParaPortion = mpTEParaPortions->GetObject( nPara );
        if ( pParaPortion->GetWritingDirectionInfos().empty() )
            ImpInitWritingDirections( nPara );

        std::vector<TEWritingDirectionInfo>& rDirInfos = pParaPortion->GetWritingDirectionInfos();
        for ( std::vector<TEWritingDirectionInfo>::const_iterator rDirInfosIt = rDirInfos.begin(); rDirInfosIt != rDirInfos.end(); ++rDirInfosIt )
        {
            if ( ( (*rDirInfosIt).nStartPos <= nPos ) && ( (*rDirInfosIt).nEndPos >= nPos ) )
               {
                nRightToLeft = (*rDirInfosIt).nType;
                if ( pStart )
                    *pStart = (*rDirInfosIt).nStartPos;
                if ( pEnd )
                    *pEnd = (*rDirInfosIt).nEndPos;
                break;
            }
        }
    }
    return nRightToLeft;
}

long TextEngine::ImpGetPortionXOffset( sal_uLong nPara, TextLine* pLine, sal_uInt16 nTextPortion )
{
    long nX = pLine->GetStartX();

    TEParaPortion* pParaPortion = mpTEParaPortions->GetObject( nPara );

    for ( sal_uInt16 i = pLine->GetStartPortion(); i < nTextPortion; i++ )
    {
        TETextPortion* pPortion = pParaPortion->GetTextPortions()[ i ];
        nX += pPortion->GetWidth();
    }

    TETextPortion* pDestPortion = pParaPortion->GetTextPortions()[ nTextPortion ];
    if ( pDestPortion->GetKind() != PORTIONKIND_TAB )
    {
        if ( !IsRightToLeft() && pDestPortion->GetRightToLeft() )
        {
            // Portions behind must be added, visual before this portion
            sal_uInt16 nTmpPortion = nTextPortion+1;
            while ( nTmpPortion <= pLine->GetEndPortion() )
            {
                TETextPortion* pNextTextPortion = pParaPortion->GetTextPortions()[ nTmpPortion ];
                if ( pNextTextPortion->GetRightToLeft() && ( pNextTextPortion->GetKind() != PORTIONKIND_TAB ) )
                    nX += pNextTextPortion->GetWidth();
                else
                    break;
                nTmpPortion++;
            }
            // Portions before must be removed, visual behind this portion
            nTmpPortion = nTextPortion;
            while ( nTmpPortion > pLine->GetStartPortion() )
            {
                --nTmpPortion;
                TETextPortion* pPrevTextPortion = pParaPortion->GetTextPortions()[ nTmpPortion ];
                if ( pPrevTextPortion->GetRightToLeft() && ( pPrevTextPortion->GetKind() != PORTIONKIND_TAB ) )
                    nX -= pPrevTextPortion->GetWidth();
                else
                    break;
            }
        }
        else if ( IsRightToLeft() && !pDestPortion->IsRightToLeft() )
        {
            // Portions behind must be removed, visual behind this portion
            sal_uInt16 nTmpPortion = nTextPortion+1;
            while ( nTmpPortion <= pLine->GetEndPortion() )
            {
                TETextPortion* pNextTextPortion = pParaPortion->GetTextPortions()[ nTmpPortion ];
                if ( !pNextTextPortion->IsRightToLeft() && ( pNextTextPortion->GetKind() != PORTIONKIND_TAB ) )
                    nX += pNextTextPortion->GetWidth();
                else
                    break;
                nTmpPortion++;
            }
            // Portions before must be added, visual before this portion
            nTmpPortion = nTextPortion;
            while ( nTmpPortion > pLine->GetStartPortion() )
            {
                --nTmpPortion;
                TETextPortion* pPrevTextPortion = pParaPortion->GetTextPortions()[ nTmpPortion ];
                if ( !pPrevTextPortion->IsRightToLeft() && ( pPrevTextPortion->GetKind() != PORTIONKIND_TAB ) )
                    nX -= pPrevTextPortion->GetWidth();
                else
                    break;
            }
        }
    }

    return nX;
}

void TextEngine::ImpInitLayoutMode( OutputDevice* pOutDev, sal_Bool bDrawingR2LPortion )
{
    sal_uLong nLayoutMode = pOutDev->GetLayoutMode();

    nLayoutMode &= ~(TEXT_LAYOUT_BIDI_RTL | TEXT_LAYOUT_COMPLEX_DISABLED | TEXT_LAYOUT_BIDI_STRONG );
    if ( bDrawingR2LPortion )
        nLayoutMode |= TEXT_LAYOUT_BIDI_RTL;

    pOutDev->SetLayoutMode( nLayoutMode );
}

TxtAlign TextEngine::ImpGetAlign() const
{
    TxtAlign eAlign = meAlign;
    if ( IsRightToLeft() )
    {
        if ( eAlign == TXTALIGN_LEFT )
            eAlign = TXTALIGN_RIGHT;
        else if ( eAlign == TXTALIGN_RIGHT )
            eAlign = TXTALIGN_LEFT;
    }
    return eAlign;
}

long TextEngine::ImpGetOutputOffset( sal_uLong nPara, TextLine* pLine, sal_uInt16 nIndex, sal_uInt16 nIndex2 )
{
    TEParaPortion* pPortion = mpTEParaPortions->GetObject( nPara );

    sal_uInt16 nPortionStart;
    sal_uInt16 nPortion = pPortion->GetTextPortions().FindPortion( nIndex, nPortionStart, sal_True );

    TETextPortion* pTextPortion = pPortion->GetTextPortions()[ nPortion ];

    long nX;

    if ( ( nIndex == nPortionStart ) && ( nIndex == nIndex2 )  )
    {
        // Output of full portion, so we need portion x offset.
        // Use ImpGetPortionXOffset, because GetXPos may deliver left or right position from portioon, depending on R2L, L2R
        nX = ImpGetPortionXOffset( nPara, pLine, nPortion );
        if ( IsRightToLeft() )
        {
            nX = -nX -pTextPortion->GetWidth();
        }
    }
    else
    {
        nX = ImpGetXPos( nPara, pLine, nIndex, nIndex == nPortionStart );
        if ( nIndex2 != nIndex )
        {
            long nX2 = ImpGetXPos( nPara, pLine, nIndex2, sal_False );
            if ( ( !IsRightToLeft() && ( nX2 < nX ) ) ||
                 ( IsRightToLeft() && ( nX2 > nX ) ) )
            {
                nX = nX2;
            }
        }
        if ( IsRightToLeft() )
        {
            nX = -nX;
        }
    }

    return nX;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
