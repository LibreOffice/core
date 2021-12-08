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

#include <memory>
#include <i18nutil/searchopt.hxx>
#include <o3tl/deleter.hxx>
#include <vcl/textview.hxx>
#include <vcl/texteng.hxx>
#include <vcl/settings.hxx>
#include "textdoc.hxx"
#include <vcl/textdata.hxx>
#include <vcl/transfer.hxx>
#include <vcl/xtextedt.hxx>
#include "textdat2.hxx"
#include <vcl/commandevent.hxx>
#include <vcl/inputctx.hxx>

#include <svl/undo.hxx>
#include <vcl/cursor.hxx>
#include <vcl/weld.hxx>
#include <vcl/window.hxx>
#include <vcl/svapp.hxx>
#include <tools/stream.hxx>

#include <sal/log.hxx>
#include <sot/formats.hxx>

#include <cppuhelper/weak.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <com/sun/star/i18n/XBreakIterator.hpp>
#include <com/sun/star/i18n/CharacterIteratorMode.hpp>
#include <com/sun/star/i18n/CharType.hpp>
#include <com/sun/star/i18n/WordType.hpp>
#include <com/sun/star/datatransfer/UnsupportedFlavorException.hpp>
#include <com/sun/star/datatransfer/XTransferable.hpp>
#include <com/sun/star/datatransfer/clipboard/XClipboard.hpp>
#include <com/sun/star/datatransfer/clipboard/XFlushableClipboard.hpp>
#include <com/sun/star/datatransfer/dnd/DNDConstants.hpp>
#include <com/sun/star/datatransfer/dnd/XDragGestureRecognizer.hpp>
#include <com/sun/star/datatransfer/dnd/XDropTarget.hpp>
#include <com/sun/star/util/SearchFlags.hpp>

#include <vcl/toolkit/edit.hxx>

#include <sot/exchange.hxx>

#include <algorithm>
#include <cstddef>

TETextDataObject::TETextDataObject( const OUString& rText ) : maText( rText )
{
}

// css::uno::XInterface
css::uno::Any TETextDataObject::queryInterface( const css::uno::Type & rType )
{
    css::uno::Any aRet = ::cppu::queryInterface( rType, static_cast< css::datatransfer::XTransferable* >(this) );
    return (aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType ));
}

// css::datatransfer::XTransferable
css::uno::Any TETextDataObject::getTransferData( const css::datatransfer::DataFlavor& rFlavor )
{
    css::uno::Any aAny;

    SotClipboardFormatId nT = SotExchange::GetFormat( rFlavor );
    if ( nT == SotClipboardFormatId::STRING )
    {
        aAny <<= maText;
    }
    else if ( nT == SotClipboardFormatId::HTML )
    {
        sal_uLong nLen = GetHTMLStream().TellEnd();
        GetHTMLStream().Seek(0);

        css::uno::Sequence< sal_Int8 > aSeq( nLen );
        memcpy( aSeq.getArray(), GetHTMLStream().GetData(), nLen );
        aAny <<= aSeq;
    }
    else
    {
        throw css::datatransfer::UnsupportedFlavorException();
    }
    return aAny;
}

css::uno::Sequence< css::datatransfer::DataFlavor > TETextDataObject::getTransferDataFlavors(  )
{
    GetHTMLStream().Seek( STREAM_SEEK_TO_END );
    bool bHTML = GetHTMLStream().Tell() > 0;
    css::uno::Sequence< css::datatransfer::DataFlavor > aDataFlavors( bHTML ? 2 : 1 );
    SotExchange::GetFormatDataFlavor( SotClipboardFormatId::STRING, aDataFlavors.getArray()[0] );
    if ( bHTML )
        SotExchange::GetFormatDataFlavor( SotClipboardFormatId::HTML, aDataFlavors.getArray()[1] );
    return aDataFlavors;
}

sal_Bool TETextDataObject::isDataFlavorSupported( const css::datatransfer::DataFlavor& rFlavor )
{
    SotClipboardFormatId nT = SotExchange::GetFormat( rFlavor );
    return ( nT == SotClipboardFormatId::STRING );
}

struct ImpTextView
{
    ExtTextEngine*      mpTextEngine;

    VclPtr<vcl::Window> mpWindow;
    TextSelection       maSelection;
    Point               maStartDocPos;

    std::unique_ptr<vcl::Cursor, o3tl::default_delete<vcl::Cursor>> mpCursor;

    std::unique_ptr<TextDDInfo, o3tl::default_delete<TextDDInfo>> mpDDInfo;

    std::unique_ptr<SelectionEngine> mpSelEngine;
    std::unique_ptr<TextSelFunctionSet> mpSelFuncSet;

    css::uno::Reference< css::datatransfer::dnd::XDragSourceListener > mxDnDListener;

    sal_uInt16              mnTravelXPos;

    bool                mbAutoScroll            : 1;
    bool                mbInsertMode            : 1;
    bool                mbReadOnly              : 1;
    bool                mbPaintSelection        : 1;
    bool                mbAutoIndent            : 1;
    bool                mbCursorEnabled         : 1;
    bool                mbClickedInSelection    : 1;
    bool                mbCursorAtEndOfLine;
};

TextView::TextView( ExtTextEngine* pEng, vcl::Window* pWindow ) :
    mpImpl(new ImpTextView)
{
    pWindow->EnableRTL( false );

    mpImpl->mpWindow = pWindow;
    mpImpl->mpTextEngine = pEng;

    mpImpl->mbPaintSelection = true;
    mpImpl->mbAutoScroll = true;
    mpImpl->mbInsertMode = true;
    mpImpl->mbReadOnly = false;
    mpImpl->mbAutoIndent = false;
    mpImpl->mbCursorEnabled = true;
    mpImpl->mbClickedInSelection = false;
//  mbInSelection = false;

    mpImpl->mnTravelXPos = TRAVEL_X_DONTKNOW;

    mpImpl->mpSelFuncSet = std::make_unique<TextSelFunctionSet>( this );
    mpImpl->mpSelEngine = std::make_unique<SelectionEngine>( mpImpl->mpWindow, mpImpl->mpSelFuncSet.get() );
    mpImpl->mpSelEngine->SetSelectionMode( SelectionMode::Range );
    mpImpl->mpSelEngine->EnableDrag( true );

    mpImpl->mpCursor.reset(new vcl::Cursor);
    mpImpl->mpCursor->Show();
    pWindow->SetCursor( mpImpl->mpCursor.get() );
    pWindow->SetInputContext( InputContext( pEng->GetFont(), InputContextFlags::Text|InputContextFlags::ExtText ) );

    pWindow->GetOutDev()->SetLineColor();

    if ( pWindow->GetDragGestureRecognizer().is() )
    {
        mpImpl->mxDnDListener = new vcl::unohelper::DragAndDropWrapper( this );

        css::uno::Reference< css::datatransfer::dnd::XDragGestureListener> xDGL( mpImpl->mxDnDListener, css::uno::UNO_QUERY );
        pWindow->GetDragGestureRecognizer()->addDragGestureListener( xDGL );
        css::uno::Reference< css::datatransfer::dnd::XDropTargetListener> xDTL( xDGL, css::uno::UNO_QUERY );
        pWindow->GetDropTarget()->addDropTargetListener( xDTL );
        pWindow->GetDropTarget()->setActive( true );
        pWindow->GetDropTarget()->setDefaultActions( css::datatransfer::dnd::DNDConstants::ACTION_COPY_OR_MOVE );
    }
}

TextView::~TextView()
{
    mpImpl->mpSelEngine.reset();
    mpImpl->mpSelFuncSet.reset();

    if ( mpImpl->mpWindow->GetCursor() == mpImpl->mpCursor.get() )
        mpImpl->mpWindow->SetCursor( nullptr );

    mpImpl->mpCursor.reset();
    mpImpl->mpDDInfo.reset();
}

void TextView::Invalidate()
{
    mpImpl->mpWindow->Invalidate();
}

void TextView::SetSelection( const TextSelection& rTextSel, bool bGotoCursor )
{
    // if someone left an empty attribute and then the Outliner manipulated the selection
    if ( !mpImpl->maSelection.HasRange() )
        mpImpl->mpTextEngine->CursorMoved( mpImpl->maSelection.GetStart().GetPara() );

    // if the selection is manipulated after a KeyInput
    mpImpl->mpTextEngine->CheckIdleFormatter();

    HideSelection();
    TextSelection aNewSel( rTextSel );
    mpImpl->mpTextEngine->ValidateSelection( aNewSel );
    ImpSetSelection( aNewSel );
    ShowSelection();
    ShowCursor( bGotoCursor );
}

void TextView::SetSelection( const TextSelection& rTextSel )
{
    SetSelection( rTextSel, mpImpl->mbAutoScroll );
}

const TextSelection& TextView::GetSelection() const
{
    return mpImpl->maSelection;
}
TextSelection&      TextView::GetSelection()
{
    return mpImpl->maSelection;
}

void TextView::DeleteSelected()
{
//  HideSelection();

    mpImpl->mpTextEngine->UndoActionStart();
    TextPaM aPaM = mpImpl->mpTextEngine->ImpDeleteText( mpImpl->maSelection );
    mpImpl->mpTextEngine->UndoActionEnd();

    ImpSetSelection( aPaM );
    mpImpl->mpTextEngine->FormatAndUpdate( this );
    ShowCursor();
}

void TextView::ImpPaint(vcl::RenderContext& rRenderContext, const Point& rStartPos, tools::Rectangle const* pPaintArea, TextSelection const* pSelection)
{
    if (!mpImpl->mbPaintSelection)
    {
        pSelection = nullptr;
    }
    else
    {
        // set correct background color;
        // unfortunately we cannot detect if it has changed
        vcl::Font aFont = mpImpl->mpTextEngine->GetFont();
        Color aColor = rRenderContext.GetBackground().GetColor();
        aColor.SetAlpha(255);
        if (aColor != aFont.GetFillColor())
        {
            if (aFont.IsTransparent())
                aColor = COL_TRANSPARENT;
            aFont.SetFillColor(aColor);
            mpImpl->mpTextEngine->maFont = aFont;
        }
    }

    mpImpl->mpTextEngine->ImpPaint(&rRenderContext, rStartPos, pPaintArea, pSelection);
}

void TextView::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect)
{
    ImpPaint(rRenderContext, rRect);
}

void TextView::ImpPaint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect)
{
    if ( !mpImpl->mpTextEngine->GetUpdateMode() || mpImpl->mpTextEngine->IsInUndo() )
        return;

    TextSelection *pDrawSelection = nullptr;
    if (mpImpl->maSelection.HasRange())
        pDrawSelection = &mpImpl->maSelection;

    Point aStartPos = ImpGetOutputStartPos(mpImpl->maStartDocPos);
    ImpPaint(rRenderContext, aStartPos, &rRect, pDrawSelection);
}

void TextView::ImpSetSelection( const TextSelection& rSelection )
{
    if (rSelection == mpImpl->maSelection)
        return;

    bool bCaret = false, bSelection = false;
    const TextPaM &rEnd = rSelection.GetEnd();
    const TextPaM &rOldEnd = mpImpl->maSelection.GetEnd();
    bool bGap = rSelection.HasRange(), bOldGap = mpImpl->maSelection.HasRange();
    if (rEnd != rOldEnd)
        bCaret = true;
    if (bGap || bOldGap)
        bSelection = true;

    mpImpl->maSelection = rSelection;

    if (bSelection)
        mpImpl->mpTextEngine->Broadcast(TextHint(SfxHintId::TextViewSelectionChanged));

    if (bCaret)
        mpImpl->mpTextEngine->Broadcast(TextHint(SfxHintId::TextViewCaretChanged));
}

void TextView::ShowSelection()
{
    ImpShowHideSelection();
}

void TextView::HideSelection()
{
    ImpShowHideSelection();
}

void TextView::ShowSelection( const TextSelection& rRange )
{
    ImpShowHideSelection( &rRange );
}

void TextView::ImpShowHideSelection(const TextSelection* pRange)
{
    const TextSelection* pRangeOrSelection = pRange ? pRange : &mpImpl->maSelection;

    if ( !pRangeOrSelection->HasRange() )
        return;

    if( mpImpl->mpWindow->IsPaintTransparent() )
        mpImpl->mpWindow->Invalidate();
    else
    {
        TextSelection aRange( *pRangeOrSelection );
        aRange.Justify();
        bool bVisCursor = mpImpl->mpCursor->IsVisible();
        mpImpl->mpCursor->Hide();
        Invalidate();
        if (bVisCursor)
            mpImpl->mpCursor->Show();
    }
}

bool TextView::KeyInput( const KeyEvent& rKeyEvent )
{
    bool bDone      = true;
    bool bModified  = false;
    bool bMoved     = false;
    bool bEndKey    = false;    // special CursorPosition
    bool bAllowIdle = true;

    // check mModified;
    // the local bModified is not set e.g. by Cut/Paste, as here
    // the update happens somewhere else
    bool bWasModified = mpImpl->mpTextEngine->IsModified();
    mpImpl->mpTextEngine->SetModified( false );

    TextSelection aCurSel( mpImpl->maSelection );
    TextSelection aOldSel( aCurSel );

    sal_uInt16 nCode = rKeyEvent.GetKeyCode().GetCode();
    KeyFuncType eFunc = rKeyEvent.GetKeyCode().GetFunction();
    if ( eFunc != KeyFuncType::DONTKNOW )
    {
        switch ( eFunc )
        {
            case KeyFuncType::CUT:
            {
                if ( !mpImpl->mbReadOnly )
                    Cut();
            }
            break;
            case KeyFuncType::COPY:
            {
                Copy();
            }
            break;
            case KeyFuncType::PASTE:
            {
                if ( !mpImpl->mbReadOnly )
                    Paste();
            }
            break;
            case KeyFuncType::UNDO:
            {
                if ( !mpImpl->mbReadOnly )
                    Undo();
            }
            break;
            case KeyFuncType::REDO:
            {
                if ( !mpImpl->mbReadOnly )
                    Redo();
            }
            break;

            default:    // might get processed below
                        eFunc = KeyFuncType::DONTKNOW;
        }
    }
    if ( eFunc == KeyFuncType::DONTKNOW )
    {
        switch ( nCode )
        {
            case KEY_UP:
            case KEY_DOWN:
            case KEY_LEFT:
            case KEY_RIGHT:
            case KEY_HOME:
            case KEY_END:
            case KEY_PAGEUP:
            case KEY_PAGEDOWN:
            case css::awt::Key::MOVE_WORD_FORWARD:
            case css::awt::Key::SELECT_WORD_FORWARD:
            case css::awt::Key::MOVE_WORD_BACKWARD:
            case css::awt::Key::SELECT_WORD_BACKWARD:
            case css::awt::Key::MOVE_TO_BEGIN_OF_LINE:
            case css::awt::Key::MOVE_TO_END_OF_LINE:
            case css::awt::Key::SELECT_TO_BEGIN_OF_LINE:
            case css::awt::Key::SELECT_TO_END_OF_LINE:
            case css::awt::Key::MOVE_TO_BEGIN_OF_PARAGRAPH:
            case css::awt::Key::MOVE_TO_END_OF_PARAGRAPH:
            case css::awt::Key::SELECT_TO_BEGIN_OF_PARAGRAPH:
            case css::awt::Key::SELECT_TO_END_OF_PARAGRAPH:
            case css::awt::Key::MOVE_TO_BEGIN_OF_DOCUMENT:
            case css::awt::Key::MOVE_TO_END_OF_DOCUMENT:
            case css::awt::Key::SELECT_TO_BEGIN_OF_DOCUMENT:
            case css::awt::Key::SELECT_TO_END_OF_DOCUMENT:
            {
                if ( ( !rKeyEvent.GetKeyCode().IsMod2() || ( nCode == KEY_LEFT ) || ( nCode == KEY_RIGHT ) )
                      && !( rKeyEvent.GetKeyCode().IsMod1() && ( nCode == KEY_PAGEUP || nCode == KEY_PAGEDOWN ) ) )
                {
                    aCurSel = ImpMoveCursor( rKeyEvent );
                    if ( aCurSel.HasRange() ) {
                        css::uno::Reference<css::datatransfer::clipboard::XClipboard> aSelection(GetSystemPrimarySelection());
                        Copy( aSelection );
                    }
                    bMoved = true;
                    if ( nCode == KEY_END )
                        bEndKey = true;
                }
                else
                    bDone = false;
            }
            break;
            case KEY_BACKSPACE:
            case KEY_DELETE:
            case css::awt::Key::DELETE_WORD_BACKWARD:
            case css::awt::Key::DELETE_WORD_FORWARD:
            case css::awt::Key::DELETE_TO_BEGIN_OF_LINE:
            case css::awt::Key::DELETE_TO_END_OF_LINE:
            {
                if ( !mpImpl->mbReadOnly && !rKeyEvent.GetKeyCode().IsMod2() )
                {
                    sal_uInt8 nDel = ( nCode == KEY_DELETE ) ? DEL_RIGHT : DEL_LEFT;
                    sal_uInt8 nMode = rKeyEvent.GetKeyCode().IsMod1() ? DELMODE_RESTOFWORD : DELMODE_SIMPLE;
                    if ( ( nMode == DELMODE_RESTOFWORD ) && rKeyEvent.GetKeyCode().IsShift() )
                        nMode = DELMODE_RESTOFCONTENT;

                    switch( nCode )
                    {
                    case css::awt::Key::DELETE_WORD_BACKWARD:
                        nDel = DEL_LEFT;
                        nMode = DELMODE_RESTOFWORD;
                        break;
                    case css::awt::Key::DELETE_WORD_FORWARD:
                        nDel = DEL_RIGHT;
                        nMode = DELMODE_RESTOFWORD;
                        break;
                    case css::awt::Key::DELETE_TO_BEGIN_OF_LINE:
                        nDel = DEL_LEFT;
                        nMode = DELMODE_RESTOFCONTENT;
                        break;
                    case css::awt::Key::DELETE_TO_END_OF_LINE:
                        nDel = DEL_RIGHT;
                        nMode = DELMODE_RESTOFCONTENT;
                        break;
                    default: break;
                    }

                    mpImpl->mpTextEngine->UndoActionStart();
                    aCurSel = ImpDelete( nDel, nMode );
                    mpImpl->mpTextEngine->UndoActionEnd();
                    bModified = true;
                    bAllowIdle = false;
                }
                else
                    bDone = false;
            }
            break;
            case KEY_TAB:
            {
                if ( !mpImpl->mbReadOnly && !rKeyEvent.GetKeyCode().IsShift() &&
                        !rKeyEvent.GetKeyCode().IsMod1() && !rKeyEvent.GetKeyCode().IsMod2() &&
                        ImplCheckTextLen( OUString('x') ) )
                {
                    aCurSel = mpImpl->mpTextEngine->ImpInsertText( aCurSel, '\t', !IsInsertMode() );
                    bModified = true;
                }
                else
                    bDone = false;
            }
            break;
            case KEY_RETURN:
            {
                // do not swallow Shift-RETURN, as this would disable multi-line entries
                // in dialogs & property editors
                if ( !mpImpl->mbReadOnly && !rKeyEvent.GetKeyCode().IsMod1() &&
                        !rKeyEvent.GetKeyCode().IsMod2() && ImplCheckTextLen( OUString('x') ) )
                {
                    mpImpl->mpTextEngine->UndoActionStart();
                    aCurSel = mpImpl->mpTextEngine->ImpInsertParaBreak( aCurSel );
                    if ( mpImpl->mbAutoIndent )
                    {
                        TextNode* pPrev = mpImpl->mpTextEngine->mpDoc->GetNodes()[ aCurSel.GetEnd().GetPara() - 1 ].get();
                        sal_Int32 n = 0;
                        while ( ( n < pPrev->GetText().getLength() ) && (
                                    ( pPrev->GetText()[ n ] == ' ' ) ||
                                    ( pPrev->GetText()[ n ] == '\t' ) ) )
                        {
                            n++;
                        }
                        if ( n )
                            aCurSel = mpImpl->mpTextEngine->ImpInsertText( aCurSel, pPrev->GetText().copy( 0, n ) );
                    }
                    mpImpl->mpTextEngine->UndoActionEnd();
                    bModified = true;
                }
                else
                    bDone = false;
            }
            break;
            case KEY_INSERT:
            {
                if ( !mpImpl->mbReadOnly )
                    SetInsertMode( !IsInsertMode() );
            }
            break;
            default:
            {
                if ( TextEngine::IsSimpleCharInput( rKeyEvent ) )
                {
                    sal_Unicode nCharCode = rKeyEvent.GetCharCode();
                    if ( !mpImpl->mbReadOnly && ImplCheckTextLen( OUString(nCharCode) ) )    // otherwise swallow the character anyway
                    {
                        aCurSel = mpImpl->mpTextEngine->ImpInsertText( nCharCode, aCurSel, !IsInsertMode(), true );
                        bModified = true;
                    }
                }
                else
                    bDone = false;
            }
        }
    }

    if ( aCurSel != aOldSel )   // Check if changed, maybe other method already changed mpImpl->maSelection, don't overwrite that!
        ImpSetSelection( aCurSel );

    if ( ( nCode != KEY_UP ) && ( nCode != KEY_DOWN ) )
        mpImpl->mnTravelXPos = TRAVEL_X_DONTKNOW;

    if ( bModified )
    {
        // Idle-Formatter only if AnyInput
        if ( bAllowIdle && Application::AnyInput( VclInputFlags::KEYBOARD) )
            mpImpl->mpTextEngine->IdleFormatAndUpdate( this );
        else
            mpImpl->mpTextEngine->FormatAndUpdate( this);
    }
    else if ( bMoved )
    {
        // selection is painted now in ImpMoveCursor
        ImpShowCursor( mpImpl->mbAutoScroll, true, bEndKey );
    }

    if ( mpImpl->mpTextEngine->IsModified() )
        mpImpl->mpTextEngine->Broadcast( TextHint( SfxHintId::TextModified ) );
    else if ( bWasModified )
        mpImpl->mpTextEngine->SetModified( true );

    return bDone;
}

void TextView::MouseButtonUp( const MouseEvent& rMouseEvent )
{
    mpImpl->mbClickedInSelection = false;
    mpImpl->mnTravelXPos = TRAVEL_X_DONTKNOW;
    mpImpl->mpSelEngine->SelMouseButtonUp( rMouseEvent );
    if ( rMouseEvent.IsMiddle() && !IsReadOnly() &&
         ( GetWindow()->GetSettings().GetMouseSettings().GetMiddleButtonAction() == MouseMiddleButtonAction::PasteSelection ) )
    {
        css::uno::Reference<css::datatransfer::clipboard::XClipboard> aSelection(GetSystemPrimarySelection());
        Paste( aSelection );
        if ( mpImpl->mpTextEngine->IsModified() )
            mpImpl->mpTextEngine->Broadcast( TextHint( SfxHintId::TextModified ) );
    }
    else if ( rMouseEvent.IsLeft() && GetSelection().HasRange() )
    {
        css::uno::Reference<css::datatransfer::clipboard::XClipboard> aSelection(GetSystemPrimarySelection());
        Copy( aSelection );
    }
}

void TextView::MouseButtonDown( const MouseEvent& rMouseEvent )
{
    mpImpl->mpTextEngine->CheckIdleFormatter();    // for fast typing and MouseButtonDown
    mpImpl->mnTravelXPos = TRAVEL_X_DONTKNOW;
    mpImpl->mbClickedInSelection = IsSelectionAtPoint( rMouseEvent.GetPosPixel() );

    mpImpl->mpTextEngine->SetActiveView( this );

    mpImpl->mpSelEngine->SelMouseButtonDown( rMouseEvent );

    // mbu 20.01.2005 - SelMouseButtonDown() possibly triggers a 'selection changed'
    // notification. The appropriate handler could change the current selection,
    // which is the case in the MailMerge address block control. To enable select'n'drag
    // we need to reevaluate the selection after the notification has been fired.
    mpImpl->mbClickedInSelection = IsSelectionAtPoint( rMouseEvent.GetPosPixel() );

    // special cases
    if ( rMouseEvent.IsShift() || ( rMouseEvent.GetClicks() < 2 ))
        return;

    if ( rMouseEvent.IsMod2() )
    {
        HideSelection();
        ImpSetSelection( mpImpl->maSelection.GetEnd() );
        SetCursorAtPoint( rMouseEvent.GetPosPixel() );  // not set by SelectionEngine for MOD2
    }

    if ( rMouseEvent.GetClicks() == 2 )
    {
        // select word
        if ( mpImpl->maSelection.GetEnd().GetIndex() < mpImpl->mpTextEngine->GetTextLen( mpImpl->maSelection.GetEnd().GetPara() ) )
        {
            HideSelection();
            // tdf#57879 - expand selection to include connector punctuations
            TextSelection aNewSel;
            mpImpl->mpTextEngine->GetWord( mpImpl->maSelection.GetEnd(), &aNewSel.GetStart(), &aNewSel.GetEnd() );
            ImpSetSelection( aNewSel );
            ShowSelection();
            ShowCursor();
        }
    }
    else if ( rMouseEvent.GetClicks() == 3 )
    {
        // select paragraph
        if ( mpImpl->maSelection.GetStart().GetIndex() || ( mpImpl->maSelection.GetEnd().GetIndex() < mpImpl->mpTextEngine->GetTextLen( mpImpl->maSelection.GetEnd().GetPara() ) ) )
        {
            HideSelection();
            TextSelection aNewSel( mpImpl->maSelection );
            aNewSel.GetStart().GetIndex() = 0;
            aNewSel.GetEnd().GetIndex() = mpImpl->mpTextEngine->mpDoc->GetNodes()[ mpImpl->maSelection.GetEnd().GetPara() ]->GetText().getLength();
            ImpSetSelection( aNewSel );
            ShowSelection();
            ShowCursor();
        }
    }
}

void TextView::MouseMove( const MouseEvent& rMouseEvent )
{
    mpImpl->mnTravelXPos = TRAVEL_X_DONTKNOW;
    mpImpl->mpSelEngine->SelMouseMove( rMouseEvent );
}

void TextView::Command( const CommandEvent& rCEvt )
{
    mpImpl->mpTextEngine->CheckIdleFormatter();    // for fast typing and MouseButtonDown
    mpImpl->mpTextEngine->SetActiveView( this );

    if ( rCEvt.GetCommand() == CommandEventId::StartExtTextInput )
    {
        DeleteSelected();
        TextNode* pNode = mpImpl->mpTextEngine->mpDoc->GetNodes()[ GetSelection().GetEnd().GetPara() ].get();
        mpImpl->mpTextEngine->mpIMEInfos = std::make_unique<TEIMEInfos>( GetSelection().GetEnd(), pNode->GetText().copy( GetSelection().GetEnd().GetIndex() ) );
        mpImpl->mpTextEngine->mpIMEInfos->bWasCursorOverwrite = !IsInsertMode();
    }
    else if ( rCEvt.GetCommand() == CommandEventId::EndExtTextInput )
    {
        SAL_WARN_IF( !mpImpl->mpTextEngine->mpIMEInfos, "vcl", "CommandEventId::EndExtTextInput => No Start ?" );
        if( mpImpl->mpTextEngine->mpIMEInfos )
        {
            TEParaPortion* pPortion = mpImpl->mpTextEngine->mpTEParaPortions->GetObject( mpImpl->mpTextEngine->mpIMEInfos->aPos.GetPara() );
            pPortion->MarkSelectionInvalid( mpImpl->mpTextEngine->mpIMEInfos->aPos.GetIndex() );

            bool bInsertMode = !mpImpl->mpTextEngine->mpIMEInfos->bWasCursorOverwrite;

            mpImpl->mpTextEngine->mpIMEInfos.reset();

            mpImpl->mpTextEngine->TextModified();
            mpImpl->mpTextEngine->FormatAndUpdate( this );

            SetInsertMode( bInsertMode );

            if ( mpImpl->mpTextEngine->IsModified() )
                mpImpl->mpTextEngine->Broadcast( TextHint( SfxHintId::TextModified ) );
        }
    }
    else if ( rCEvt.GetCommand() == CommandEventId::ExtTextInput )
    {
        SAL_WARN_IF( !mpImpl->mpTextEngine->mpIMEInfos, "vcl", "CommandEventId::ExtTextInput => No Start ?" );
        if( mpImpl->mpTextEngine->mpIMEInfos )
        {
            const CommandExtTextInputData* pData = rCEvt.GetExtTextInputData();

            if ( !pData->IsOnlyCursorChanged() )
            {
                TextSelection aSelect( mpImpl->mpTextEngine->mpIMEInfos->aPos );
                aSelect.GetEnd().GetIndex() += mpImpl->mpTextEngine->mpIMEInfos->nLen;
                aSelect = mpImpl->mpTextEngine->ImpDeleteText( aSelect );
                aSelect = mpImpl->mpTextEngine->ImpInsertText( aSelect, pData->GetText() );

                if ( mpImpl->mpTextEngine->mpIMEInfos->bWasCursorOverwrite )
                {
                    const sal_Int32 nOldIMETextLen = mpImpl->mpTextEngine->mpIMEInfos->nLen;
                    const sal_Int32 nNewIMETextLen = pData->GetText().getLength();

                    if ( ( nOldIMETextLen > nNewIMETextLen ) &&
                         ( nNewIMETextLen < mpImpl->mpTextEngine->mpIMEInfos->aOldTextAfterStartPos.getLength() ) )
                    {
                        // restore old characters
                        sal_Int32 nRestore = nOldIMETextLen - nNewIMETextLen;
                        TextPaM aPaM( mpImpl->mpTextEngine->mpIMEInfos->aPos );
                        aPaM.GetIndex() += nNewIMETextLen;
                        mpImpl->mpTextEngine->ImpInsertText( aPaM, mpImpl->mpTextEngine->mpIMEInfos->aOldTextAfterStartPos.copy( nNewIMETextLen, nRestore ) );
                    }
                    else if ( ( nOldIMETextLen < nNewIMETextLen ) &&
                              ( nOldIMETextLen < mpImpl->mpTextEngine->mpIMEInfos->aOldTextAfterStartPos.getLength() ) )
                    {
                        // overwrite
                        const sal_Int32 nOverwrite = std::min( nNewIMETextLen, mpImpl->mpTextEngine->mpIMEInfos->aOldTextAfterStartPos.getLength() ) - nOldIMETextLen;
                        SAL_WARN_IF( !nOverwrite || (nOverwrite >= 0xFF00), "vcl", "IME Overwrite?!" );
                        TextPaM aPaM( mpImpl->mpTextEngine->mpIMEInfos->aPos );
                        aPaM.GetIndex() += nNewIMETextLen;
                        TextSelection aSel( aPaM );
                        aSel.GetEnd().GetIndex() += nOverwrite;
                        mpImpl->mpTextEngine->ImpDeleteText( aSel );
                    }
                }

                if ( pData->GetTextAttr() )
                {
                    mpImpl->mpTextEngine->mpIMEInfos->CopyAttribs( pData->GetTextAttr(), pData->GetText().getLength() );
                }
                else
                {
                    mpImpl->mpTextEngine->mpIMEInfos->DestroyAttribs();
                }

                TEParaPortion* pPPortion = mpImpl->mpTextEngine->mpTEParaPortions->GetObject( mpImpl->mpTextEngine->mpIMEInfos->aPos.GetPara() );
                pPPortion->MarkSelectionInvalid( mpImpl->mpTextEngine->mpIMEInfos->aPos.GetIndex() );
                mpImpl->mpTextEngine->FormatAndUpdate( this );
            }

            TextSelection aNewSel = TextPaM( mpImpl->mpTextEngine->mpIMEInfos->aPos.GetPara(), mpImpl->mpTextEngine->mpIMEInfos->aPos.GetIndex()+pData->GetCursorPos() );
            SetSelection( aNewSel );
            SetInsertMode( !pData->IsCursorOverwrite() );

            if ( pData->IsCursorVisible() )
                ShowCursor();
            else
                HideCursor();
        }
    }
    else if ( rCEvt.GetCommand() == CommandEventId::CursorPos )
    {
        if ( mpImpl->mpTextEngine->mpIMEInfos && mpImpl->mpTextEngine->mpIMEInfos->nLen )
        {
            TextPaM aPaM( GetSelection().GetEnd() );
            tools::Rectangle aR1 = mpImpl->mpTextEngine->PaMtoEditCursor( aPaM );

            sal_Int32 nInputEnd = mpImpl->mpTextEngine->mpIMEInfos->aPos.GetIndex() + mpImpl->mpTextEngine->mpIMEInfos->nLen;

            if ( !mpImpl->mpTextEngine->IsFormatted() )
                mpImpl->mpTextEngine->FormatDoc();

            TEParaPortion* pParaPortion = mpImpl->mpTextEngine->mpTEParaPortions->GetObject( aPaM.GetPara() );
            std::vector<TextLine>::size_type nLine = pParaPortion->GetLineNumber( aPaM.GetIndex(), true );
            TextLine& rLine = pParaPortion->GetLines()[ nLine ];
            if ( nInputEnd > rLine.GetEnd() )
                nInputEnd = rLine.GetEnd();
            tools::Rectangle aR2 = mpImpl->mpTextEngine->PaMtoEditCursor( TextPaM( aPaM.GetPara(), nInputEnd ) );

            tools::Long nWidth = aR2.Left()-aR1.Right();
            aR1.Move( -GetStartDocPos().X(), -GetStartDocPos().Y() );
            GetWindow()->SetCursorRect( &aR1, nWidth );
        }
        else
        {
            GetWindow()->SetCursorRect();
        }
    }
    else
    {
        mpImpl->mpSelEngine->Command( rCEvt );
    }
}

void TextView::ShowCursor( bool bGotoCursor, bool bForceVisCursor )
{
    // this setting has more weight
    if ( !mpImpl->mbAutoScroll )
        bGotoCursor = false;
    ImpShowCursor( bGotoCursor, bForceVisCursor, false );
}

void TextView::HideCursor()
{
    mpImpl->mpCursor->Hide();
}

void TextView::Scroll( tools::Long ndX, tools::Long ndY )
{
    SAL_WARN_IF( !mpImpl->mpTextEngine->IsFormatted(), "vcl", "Scroll: Not formatted!" );

    if ( !ndX && !ndY )
        return;

    Point aNewStartPos( mpImpl->maStartDocPos );

    // Vertical:
    aNewStartPos.AdjustY( -ndY );
    if ( aNewStartPos.Y() < 0 )
        aNewStartPos.setY( 0 );

    // Horizontal:
    aNewStartPos.AdjustX( -ndX );
    if ( aNewStartPos.X() < 0 )
        aNewStartPos.setX( 0 );

    tools::Long nDiffX = mpImpl->maStartDocPos.X() - aNewStartPos.X();
    tools::Long nDiffY = mpImpl->maStartDocPos.Y() - aNewStartPos.Y();

    if ( nDiffX || nDiffY )
    {
        bool bVisCursor = mpImpl->mpCursor->IsVisible();
        mpImpl->mpCursor->Hide();
        mpImpl->mpWindow->PaintImmediately();
        mpImpl->maStartDocPos = aNewStartPos;

        if ( mpImpl->mpTextEngine->IsRightToLeft() )
            nDiffX = -nDiffX;
        mpImpl->mpWindow->Scroll( nDiffX, nDiffY );
        mpImpl->mpWindow->PaintImmediately();
        mpImpl->mpCursor->SetPos( mpImpl->mpCursor->GetPos() + Point( nDiffX, nDiffY ) );
        if ( bVisCursor && !mpImpl->mbReadOnly )
            mpImpl->mpCursor->Show();
    }

    mpImpl->mpTextEngine->Broadcast( TextHint( SfxHintId::TextViewScrolled ) );
}

void TextView::Undo()
{
    mpImpl->mpTextEngine->SetActiveView( this );
    mpImpl->mpTextEngine->GetUndoManager().Undo();
}

void TextView::Redo()
{
    mpImpl->mpTextEngine->SetActiveView( this );
    mpImpl->mpTextEngine->GetUndoManager().Redo();
}

void TextView::Cut()
{
    mpImpl->mpTextEngine->UndoActionStart();
    Copy();
    DeleteSelected();
    mpImpl->mpTextEngine->UndoActionEnd();
}

void TextView::Copy( css::uno::Reference< css::datatransfer::clipboard::XClipboard > const & rxClipboard )
{
    if ( !rxClipboard.is() )
        return;

    rtl::Reference<TETextDataObject> pDataObj = new TETextDataObject( GetSelected() );

    SolarMutexReleaser aReleaser;

    try
    {
        rxClipboard->setContents( pDataObj, nullptr );

        css::uno::Reference< css::datatransfer::clipboard::XFlushableClipboard > xFlushableClipboard( rxClipboard, css::uno::UNO_QUERY );
        if( xFlushableClipboard.is() )
            xFlushableClipboard->flushClipboard();
    }
    catch( const css::uno::Exception& )
    {
    }
}

void TextView::Copy()
{
    css::uno::Reference<css::datatransfer::clipboard::XClipboard> aClipboard(GetWindow()->GetClipboard());
    Copy( aClipboard );
}

void TextView::Paste( css::uno::Reference< css::datatransfer::clipboard::XClipboard > const & rxClipboard )
{
    if ( !rxClipboard.is() )
        return;

    css::uno::Reference< css::datatransfer::XTransferable > xDataObj;

    try
        {
            SolarMutexReleaser aReleaser;
            xDataObj = rxClipboard->getContents();
        }
    catch( const css::uno::Exception& )
        {
        }

    if ( !xDataObj.is() )
        return;

    css::datatransfer::DataFlavor aFlavor;
    SotExchange::GetFormatDataFlavor( SotClipboardFormatId::STRING, aFlavor );
    if ( !xDataObj->isDataFlavorSupported( aFlavor ) )
        return;

    try
    {
        css::uno::Any aData = xDataObj->getTransferData( aFlavor );
        OUString aText;
        aData >>= aText;
        bool bWasTruncated = false;
        if( mpImpl->mpTextEngine->GetMaxTextLen() != 0 )
            bWasTruncated = ImplTruncateNewText( aText );
        InsertText( aText );
        mpImpl->mpTextEngine->Broadcast( TextHint( SfxHintId::TextModified ) );

        if( bWasTruncated )
            Edit::ShowTruncationWarning(mpImpl->mpWindow->GetFrameWeld());
    }
    catch( const css::datatransfer::UnsupportedFlavorException& )
    {
    }
}

void TextView::Paste()
{
    css::uno::Reference<css::datatransfer::clipboard::XClipboard> aClipboard(GetWindow()->GetClipboard());
    Paste( aClipboard );
}

OUString TextView::GetSelected() const
{
    return GetSelected( GetSystemLineEnd() );
}

OUString TextView::GetSelected( LineEnd aSeparator ) const
{
    return mpImpl->mpTextEngine->GetText( mpImpl->maSelection, aSeparator );
}

void TextView::SetInsertMode( bool bInsert )
{
    if ( mpImpl->mbInsertMode != bInsert )
    {
        mpImpl->mbInsertMode = bInsert;
        ShowCursor( mpImpl->mbAutoScroll, false );
    }
}

void TextView::SetReadOnly( bool bReadOnly )
{
    if ( mpImpl->mbReadOnly != bReadOnly )
    {
        mpImpl->mbReadOnly = bReadOnly;
        if ( !mpImpl->mbReadOnly )
            ShowCursor( mpImpl->mbAutoScroll, false );
        else
            HideCursor();

        GetWindow()->SetInputContext( InputContext( mpImpl->mpTextEngine->GetFont(), bReadOnly ? InputContextFlags::Text|InputContextFlags::ExtText : InputContextFlags::NONE ) );
    }
}

TextSelection const & TextView::ImpMoveCursor( const KeyEvent& rKeyEvent )
{
    // normally only needed for Up/Down; but who cares
    mpImpl->mpTextEngine->CheckIdleFormatter();

    TextPaM aPaM( mpImpl->maSelection.GetEnd() );
    TextPaM aOldEnd( aPaM );

    TextDirectionality eTextDirection = TextDirectionality::LeftToRight_TopToBottom;
    if ( mpImpl->mpTextEngine->IsRightToLeft() )
        eTextDirection = TextDirectionality::RightToLeft_TopToBottom;

    KeyEvent aTranslatedKeyEvent = rKeyEvent.LogicalTextDirectionality( eTextDirection );

    bool bCtrl = aTranslatedKeyEvent.GetKeyCode().IsMod1();
    sal_uInt16 nCode = aTranslatedKeyEvent.GetKeyCode().GetCode();

    bool bSelect = aTranslatedKeyEvent.GetKeyCode().IsShift();
    switch ( nCode )
    {
        case KEY_UP:        aPaM = CursorUp( aPaM );
                            break;
        case KEY_DOWN:      aPaM = CursorDown( aPaM );
                            break;
        case KEY_HOME:
            // tdf#145764 - move cursor to the beginning or first character in the same line
            aPaM = bCtrl                  ? CursorStartOfDoc()
                   : aPaM.GetIndex() == 0 ? CursorFirstWord( aPaM )
                                          : CursorStartOfLine( aPaM );
                            break;
        case KEY_END:       aPaM = bCtrl ? CursorEndOfDoc() : CursorEndOfLine( aPaM );
                            break;
        case KEY_PAGEUP:    aPaM = bCtrl ? CursorStartOfDoc() : PageUp( aPaM );
                            break;
        case KEY_PAGEDOWN:  aPaM = bCtrl ? CursorEndOfDoc() : PageDown( aPaM );
                            break;
        case KEY_LEFT:      aPaM = bCtrl ? CursorWordLeft( aPaM ) : CursorLeft( aPaM, aTranslatedKeyEvent.GetKeyCode().IsMod2() ? sal_uInt16(css::i18n::CharacterIteratorMode::SKIPCHARACTER) : sal_uInt16(css::i18n::CharacterIteratorMode::SKIPCELL) );
                            break;
        case KEY_RIGHT:     aPaM = bCtrl ? CursorWordRight( aPaM ) : CursorRight( aPaM, aTranslatedKeyEvent.GetKeyCode().IsMod2() ? sal_uInt16(css::i18n::CharacterIteratorMode::SKIPCHARACTER) : sal_uInt16(css::i18n::CharacterIteratorMode::SKIPCELL) );
                            break;
        case css::awt::Key::SELECT_WORD_FORWARD:
                            bSelect = true;
                            [[fallthrough]];
        case css::awt::Key::MOVE_WORD_FORWARD:
                            aPaM = CursorWordRight( aPaM );
                            break;
        case css::awt::Key::SELECT_WORD_BACKWARD:
                            bSelect = true;
                            [[fallthrough]];
        case css::awt::Key::MOVE_WORD_BACKWARD:
                            aPaM = CursorWordLeft( aPaM );
                            break;
        case css::awt::Key::SELECT_TO_BEGIN_OF_LINE:
                            bSelect = true;
                            [[fallthrough]];
        case css::awt::Key::MOVE_TO_BEGIN_OF_LINE:
                            aPaM = CursorStartOfLine( aPaM );
                            break;
        case css::awt::Key::SELECT_TO_END_OF_LINE:
                            bSelect = true;
                            [[fallthrough]];
        case css::awt::Key::MOVE_TO_END_OF_LINE:
                            aPaM = CursorEndOfLine( aPaM );
                            break;
        case css::awt::Key::SELECT_TO_BEGIN_OF_PARAGRAPH:
                            bSelect = true;
                            [[fallthrough]];
        case css::awt::Key::MOVE_TO_BEGIN_OF_PARAGRAPH:
                            aPaM = CursorStartOfParagraph( aPaM );
                            break;
        case css::awt::Key::SELECT_TO_END_OF_PARAGRAPH:
                            bSelect = true;
                            [[fallthrough]];
        case css::awt::Key::MOVE_TO_END_OF_PARAGRAPH:
                            aPaM = CursorEndOfParagraph( aPaM );
                            break;
        case css::awt::Key::SELECT_TO_BEGIN_OF_DOCUMENT:
                            bSelect = true;
                            [[fallthrough]];
        case css::awt::Key::MOVE_TO_BEGIN_OF_DOCUMENT:
                            aPaM = CursorStartOfDoc();
                            break;
        case css::awt::Key::SELECT_TO_END_OF_DOCUMENT:
                            bSelect = true;
                            [[fallthrough]];
        case css::awt::Key::MOVE_TO_END_OF_DOCUMENT:
                            aPaM = CursorEndOfDoc();
                            break;
    }

    // might cause a CreateAnchor or Deselection all
    mpImpl->mpSelEngine->CursorPosChanging( bSelect, aTranslatedKeyEvent.GetKeyCode().IsMod1() );

    if ( aOldEnd != aPaM )
    {
        mpImpl->mpTextEngine->CursorMoved( aOldEnd.GetPara() );

        TextSelection aNewSelection( mpImpl->maSelection );
        aNewSelection.GetEnd() = aPaM;
        if ( bSelect )
        {
            // extend the selection
            ImpSetSelection( aNewSelection );
            ShowSelection( TextSelection( aOldEnd, aPaM ) );
        }
        else
        {
            aNewSelection.GetStart() = aPaM;
            ImpSetSelection( aNewSelection );
        }
    }

    return mpImpl->maSelection;
}

void TextView::InsertText( const OUString& rStr )
{
    mpImpl->mpTextEngine->UndoActionStart();

    TextSelection aNewSel = mpImpl->mpTextEngine->ImpInsertText( mpImpl->maSelection, rStr );

    ImpSetSelection( aNewSel );

    mpImpl->mpTextEngine->UndoActionEnd();

    mpImpl->mpTextEngine->FormatAndUpdate( this );
}

TextPaM TextView::CursorLeft( const TextPaM& rPaM, sal_uInt16 nCharacterIteratorMode )
{
    TextPaM aPaM( rPaM );

    if ( aPaM.GetIndex() )
    {
        TextNode* pNode = mpImpl->mpTextEngine->mpDoc->GetNodes()[ aPaM.GetPara() ].get();
        css::uno::Reference < css::i18n::XBreakIterator > xBI = mpImpl->mpTextEngine->GetBreakIterator();
        sal_Int32 nCount = 1;
        aPaM.GetIndex() = xBI->previousCharacters( pNode->GetText(), aPaM.GetIndex(), mpImpl->mpTextEngine->GetLocale(), nCharacterIteratorMode, nCount, nCount );
    }
    else if ( aPaM.GetPara() )
    {
        aPaM.GetPara()--;
        TextNode* pNode = mpImpl->mpTextEngine->mpDoc->GetNodes()[ aPaM.GetPara() ].get();
        aPaM.GetIndex() = pNode->GetText().getLength();
    }
    return aPaM;
}

TextPaM TextView::CursorRight( const TextPaM& rPaM, sal_uInt16 nCharacterIteratorMode )
{
    TextPaM aPaM( rPaM );

    TextNode* pNode = mpImpl->mpTextEngine->mpDoc->GetNodes()[ aPaM.GetPara() ].get();
    if ( aPaM.GetIndex() < pNode->GetText().getLength() )
    {
        css::uno::Reference < css::i18n::XBreakIterator > xBI = mpImpl->mpTextEngine->GetBreakIterator();
        sal_Int32 nCount = 1;
        aPaM.GetIndex() = xBI->nextCharacters( pNode->GetText(), aPaM.GetIndex(), mpImpl->mpTextEngine->GetLocale(), nCharacterIteratorMode, nCount, nCount );
    }
    else if ( aPaM.GetPara() < ( mpImpl->mpTextEngine->mpDoc->GetNodes().size()-1) )
    {
        aPaM.GetPara()++;
        aPaM.GetIndex() = 0;
    }

    return aPaM;
}

TextPaM TextView::CursorFirstWord( const TextPaM& rPaM )
{
    TextPaM aPaM(rPaM);
    TextNode* pNode = mpImpl->mpTextEngine->mpDoc->GetNodes()[aPaM.GetPara()].get();

    css::uno::Reference<css::i18n::XBreakIterator> xBI = mpImpl->mpTextEngine->GetBreakIterator();
    aPaM.GetIndex() = xBI->beginOfSentence(pNode->GetText(), 0, mpImpl->mpTextEngine->GetLocale());

    return aPaM;
}

TextPaM TextView::CursorWordLeft( const TextPaM& rPaM )
{
    TextPaM aPaM( rPaM );

    if ( aPaM.GetIndex() )
    {
        // tdf#57879 - expand selection to the left to include connector punctuations
        mpImpl->mpTextEngine->GetWord( rPaM, &aPaM );
        if ( aPaM.GetIndex() >= rPaM.GetIndex() )
        {
            TextNode* pNode = mpImpl->mpTextEngine->mpDoc->GetNodes()[ aPaM.GetPara() ].get();
            css::uno::Reference < css::i18n::XBreakIterator > xBI = mpImpl->mpTextEngine->GetBreakIterator();
            aPaM.GetIndex() = xBI->previousWord( pNode->GetText(), rPaM.GetIndex(), mpImpl->mpTextEngine->GetLocale(), css::i18n::WordType::ANYWORD_IGNOREWHITESPACES ).startPos;
            if ( aPaM.GetIndex() > 0 )
                mpImpl->mpTextEngine->GetWord( aPaM, &aPaM );
            else
                aPaM.GetIndex() = 0;
        }
    }
    else if ( aPaM.GetPara() )
    {
        aPaM.GetPara()--;
        TextNode* pNode = mpImpl->mpTextEngine->mpDoc->GetNodes()[ aPaM.GetPara() ].get();
        aPaM.GetIndex() = pNode->GetText().getLength();
    }
    return aPaM;
}

TextPaM TextView::CursorWordRight( const TextPaM& rPaM )
{
    TextPaM aPaM( rPaM );

    TextNode* pNode = mpImpl->mpTextEngine->mpDoc->GetNodes()[ aPaM.GetPara() ].get();
    if ( aPaM.GetIndex() < pNode->GetText().getLength() )
    {
        css::uno::Reference < css::i18n::XBreakIterator > xBI = mpImpl->mpTextEngine->GetBreakIterator();
        aPaM.GetIndex() = xBI->nextWord(  pNode->GetText(), aPaM.GetIndex(), mpImpl->mpTextEngine->GetLocale(), css::i18n::WordType::ANYWORD_IGNOREWHITESPACES ).endPos;
        mpImpl->mpTextEngine->GetWord( aPaM, nullptr, &aPaM );
    }
    else if ( aPaM.GetPara() < ( mpImpl->mpTextEngine->mpDoc->GetNodes().size()-1) )
    {
        aPaM.GetPara()++;
        aPaM.GetIndex() = 0;
    }

    return aPaM;
}

TextPaM TextView::ImpDelete( sal_uInt8 nMode, sal_uInt8 nDelMode )
{
    if ( mpImpl->maSelection.HasRange() )  // only delete selection
        return mpImpl->mpTextEngine->ImpDeleteText( mpImpl->maSelection );

    TextPaM aStartPaM = mpImpl->maSelection.GetStart();
    TextPaM aEndPaM = aStartPaM;
    if ( nMode == DEL_LEFT )
    {
        if ( nDelMode == DELMODE_SIMPLE )
        {
            aEndPaM = CursorLeft( aEndPaM, sal_uInt16(css::i18n::CharacterIteratorMode::SKIPCHARACTER) );
        }
        else if ( nDelMode == DELMODE_RESTOFWORD )
        {
            TextNode* pNode = mpImpl->mpTextEngine->mpDoc->GetNodes()[  aEndPaM.GetPara() ].get();
            css::uno::Reference < css::i18n::XBreakIterator > xBI = mpImpl->mpTextEngine->GetBreakIterator();
            css::i18n::Boundary aBoundary = xBI->getWordBoundary( pNode->GetText(), mpImpl->maSelection.GetEnd().GetIndex(), mpImpl->mpTextEngine->GetLocale(), css::i18n::WordType::ANYWORD_IGNOREWHITESPACES, true );
            if ( aBoundary.startPos == mpImpl->maSelection.GetEnd().GetIndex() )
                aBoundary = xBI->previousWord( pNode->GetText(), mpImpl->maSelection.GetEnd().GetIndex(), mpImpl->mpTextEngine->GetLocale(), css::i18n::WordType::ANYWORD_IGNOREWHITESPACES );
            // #i63506# startPos is -1 when the paragraph starts with a tab
            aEndPaM.GetIndex() = std::max<sal_Int32>(aBoundary.startPos, 0);
        }
        else    // DELMODE_RESTOFCONTENT
        {
            if ( aEndPaM.GetIndex() != 0 )
                aEndPaM.GetIndex() = 0;
            else if ( aEndPaM.GetPara() )
            {
                // previous paragraph
                aEndPaM.GetPara()--;
                aEndPaM.GetIndex() = 0;
            }
        }
    }
    else
    {
        if ( nDelMode == DELMODE_SIMPLE )
        {
            aEndPaM = CursorRight( aEndPaM, sal_uInt16(css::i18n::CharacterIteratorMode::SKIPCELL) );
        }
        else if ( nDelMode == DELMODE_RESTOFWORD )
        {
            TextNode* pNode = mpImpl->mpTextEngine->mpDoc->GetNodes()[  aEndPaM.GetPara() ].get();
            css::uno::Reference < css::i18n::XBreakIterator > xBI = mpImpl->mpTextEngine->GetBreakIterator();
            css::i18n::Boundary aBoundary = xBI->nextWord( pNode->GetText(), mpImpl->maSelection.GetEnd().GetIndex(), mpImpl->mpTextEngine->GetLocale(), css::i18n::WordType::ANYWORD_IGNOREWHITESPACES );
            aEndPaM.GetIndex() = aBoundary.startPos;
        }
        else    // DELMODE_RESTOFCONTENT
        {
            TextNode* pNode = mpImpl->mpTextEngine->mpDoc->GetNodes()[ aEndPaM.GetPara() ].get();
            if ( aEndPaM.GetIndex() < pNode->GetText().getLength() )
                aEndPaM.GetIndex() = pNode->GetText().getLength();
            else if ( aEndPaM.GetPara() < ( mpImpl->mpTextEngine->mpDoc->GetNodes().size() - 1 ) )
            {
                // next paragraph
                aEndPaM.GetPara()++;
                TextNode* pNextNode = mpImpl->mpTextEngine->mpDoc->GetNodes()[ aEndPaM.GetPara() ].get();
                aEndPaM.GetIndex() = pNextNode->GetText().getLength();
            }
        }
    }

    return mpImpl->mpTextEngine->ImpDeleteText( TextSelection( aStartPaM, aEndPaM ) );
}

TextPaM TextView::CursorUp( const TextPaM& rPaM )
{
    TextPaM aPaM( rPaM );

    tools::Long nX;
    if ( mpImpl->mnTravelXPos == TRAVEL_X_DONTKNOW )
    {
        nX = mpImpl->mpTextEngine->GetEditCursor( rPaM, false ).Left();
        mpImpl->mnTravelXPos = static_cast<sal_uInt16>(nX)+1;
    }
    else
        nX = mpImpl->mnTravelXPos;

    TEParaPortion* pPPortion = mpImpl->mpTextEngine->mpTEParaPortions->GetObject( rPaM.GetPara() );
    std::vector<TextLine>::size_type nLine = pPPortion->GetLineNumber( rPaM.GetIndex(), false );
    if ( nLine )    // same paragraph
    {
        aPaM.GetIndex() = mpImpl->mpTextEngine->GetCharPos( rPaM.GetPara(), nLine-1, nX );
        // If we need to go to the end of a line that was wrapped automatically,
        // the cursor ends up at the beginning of the 2nd line
        // Problem: Last character of an automatically wrapped line = Cursor
        TextLine& rLine = pPPortion->GetLines()[ nLine - 1 ];
        if ( aPaM.GetIndex() && ( aPaM.GetIndex() == rLine.GetEnd() ) )
            --aPaM.GetIndex();
    }
    else if ( rPaM.GetPara() )  // previous paragraph
    {
        aPaM.GetPara()--;
        pPPortion = mpImpl->mpTextEngine->mpTEParaPortions->GetObject( aPaM.GetPara() );
        std::vector<TextLine>::size_type nL = pPPortion->GetLines().size() - 1;
        aPaM.GetIndex() = mpImpl->mpTextEngine->GetCharPos( aPaM.GetPara(), nL, nX+1 );
    }

    return aPaM;
}

TextPaM TextView::CursorDown( const TextPaM& rPaM )
{
    TextPaM aPaM( rPaM );

    tools::Long nX;
    if ( mpImpl->mnTravelXPos == TRAVEL_X_DONTKNOW )
    {
        nX = mpImpl->mpTextEngine->GetEditCursor( rPaM, false ).Left();
        mpImpl->mnTravelXPos = static_cast<sal_uInt16>(nX)+1;
    }
    else
        nX = mpImpl->mnTravelXPos;

    TEParaPortion* pPPortion = mpImpl->mpTextEngine->mpTEParaPortions->GetObject( rPaM.GetPara() );
    std::vector<TextLine>::size_type nLine = pPPortion->GetLineNumber( rPaM.GetIndex(), false );
    if ( nLine < ( pPPortion->GetLines().size() - 1 ) )
    {
        aPaM.GetIndex() = mpImpl->mpTextEngine->GetCharPos( rPaM.GetPara(), nLine+1, nX );

        // special case CursorUp
        TextLine& rLine = pPPortion->GetLines()[ nLine + 1 ];
        if ( ( aPaM.GetIndex() == rLine.GetEnd() ) && ( aPaM.GetIndex() > rLine.GetStart() ) && aPaM.GetIndex() < pPPortion->GetNode()->GetText().getLength() )
            --aPaM.GetIndex();
    }
    else if ( rPaM.GetPara() < ( mpImpl->mpTextEngine->mpDoc->GetNodes().size() - 1 ) )   // next paragraph
    {
        aPaM.GetPara()++;
        pPPortion = mpImpl->mpTextEngine->mpTEParaPortions->GetObject( aPaM.GetPara() );
        aPaM.GetIndex() = mpImpl->mpTextEngine->GetCharPos( aPaM.GetPara(), 0, nX+1 );
        TextLine& rLine = pPPortion->GetLines().front();
        if ( ( aPaM.GetIndex() == rLine.GetEnd() ) && ( aPaM.GetIndex() > rLine.GetStart() ) && ( pPPortion->GetLines().size() > 1 ) )
            --aPaM.GetIndex();
    }

    return aPaM;
}

TextPaM TextView::CursorStartOfLine( const TextPaM& rPaM )
{
    TextPaM aPaM( rPaM );

    TEParaPortion* pPPortion = mpImpl->mpTextEngine->mpTEParaPortions->GetObject( rPaM.GetPara() );
    std::vector<TextLine>::size_type nLine = pPPortion->GetLineNumber( aPaM.GetIndex(), false );
    TextLine& rLine = pPPortion->GetLines()[ nLine ];
    aPaM.GetIndex() = rLine.GetStart();

    return aPaM;
}

TextPaM TextView::CursorEndOfLine( const TextPaM& rPaM )
{
    TextPaM aPaM( rPaM );

    TEParaPortion* pPPortion = mpImpl->mpTextEngine->mpTEParaPortions->GetObject( rPaM.GetPara() );
    std::vector<TextLine>::size_type nLine = pPPortion->GetLineNumber( aPaM.GetIndex(), false );
    TextLine& rLine = pPPortion->GetLines()[ nLine ];
    aPaM.GetIndex() = rLine.GetEnd();

    if ( rLine.GetEnd() > rLine.GetStart() )  // empty line
    {
        sal_Unicode cLastChar = pPPortion->GetNode()->GetText()[ aPaM.GetIndex()-1 ];
        if ( ( cLastChar == ' ' ) && ( aPaM.GetIndex() != pPPortion->GetNode()->GetText().getLength() ) )
        {
            // for a blank in an automatically-wrapped line it is better to stand before it,
            // as the user will intend to stand behind the prior word.
            // If there is a change, special case for Pos1 after End!
            --aPaM.GetIndex();
        }
    }
    return aPaM;
}

TextPaM TextView::CursorStartOfParagraph( const TextPaM& rPaM )
{
    TextPaM aPaM( rPaM );
    aPaM.GetIndex() = 0;
    return aPaM;
}

TextPaM TextView::CursorEndOfParagraph( const TextPaM& rPaM )
{
    TextNode* pNode = mpImpl->mpTextEngine->mpDoc->GetNodes()[ rPaM.GetPara() ].get();
    TextPaM aPaM( rPaM );
    aPaM.GetIndex() = pNode->GetText().getLength();
    return aPaM;
}

TextPaM TextView::CursorStartOfDoc()
{
    TextPaM aPaM( 0, 0 );
    return aPaM;
}

TextPaM TextView::CursorEndOfDoc()
{
    const sal_uInt32 nNode = static_cast<sal_uInt32>(mpImpl->mpTextEngine->mpDoc->GetNodes().size() - 1);
    TextNode* pNode = mpImpl->mpTextEngine->mpDoc->GetNodes()[ nNode ].get();
    TextPaM aPaM( nNode, pNode->GetText().getLength() );
    return aPaM;
}

TextPaM TextView::PageUp( const TextPaM& rPaM )
{
    tools::Rectangle aRect = mpImpl->mpTextEngine->PaMtoEditCursor( rPaM );
    Point aTopLeft = aRect.TopLeft();
    aTopLeft.AdjustY( -(mpImpl->mpWindow->GetOutputSizePixel().Height() * 9/10) );
    aTopLeft.AdjustX(1 );
    if ( aTopLeft.Y() < 0 )
        aTopLeft.setY( 0 );

    TextPaM aPaM = mpImpl->mpTextEngine->GetPaM( aTopLeft );
    return aPaM;
}

TextPaM TextView::PageDown( const TextPaM& rPaM )
{
    tools::Rectangle aRect = mpImpl->mpTextEngine->PaMtoEditCursor( rPaM );
    Point aBottomRight = aRect.BottomRight();
    aBottomRight.AdjustY(mpImpl->mpWindow->GetOutputSizePixel().Height() * 9/10 );
    aBottomRight.AdjustX(1 );
    tools::Long nHeight = mpImpl->mpTextEngine->GetTextHeight();
    if ( aBottomRight.Y() > nHeight )
        aBottomRight.setY( nHeight-1 );

    TextPaM aPaM = mpImpl->mpTextEngine->GetPaM( aBottomRight );
    return aPaM;
}

void TextView::ImpShowCursor( bool bGotoCursor, bool bForceVisCursor, bool bSpecial )
{
    if ( mpImpl->mpTextEngine->IsFormatting() )
        return;
    if ( !mpImpl->mpTextEngine->GetUpdateMode() )
        return;
    if ( mpImpl->mpTextEngine->IsInUndo() )
        return;

    mpImpl->mpTextEngine->CheckIdleFormatter();
    if ( !mpImpl->mpTextEngine->IsFormatted() )
        mpImpl->mpTextEngine->FormatAndUpdate( this );

    TextPaM aPaM( mpImpl->maSelection.GetEnd() );
    tools::Rectangle aEditCursor = mpImpl->mpTextEngine->PaMtoEditCursor( aPaM, bSpecial );

    // Remember that we placed the cursor behind the last character of a line
    mpImpl->mbCursorAtEndOfLine = false;
    if( bSpecial )
    {
        TEParaPortion* pParaPortion = mpImpl->mpTextEngine->mpTEParaPortions->GetObject( aPaM.GetPara() );
        mpImpl->mbCursorAtEndOfLine =
            pParaPortion->GetLineNumber( aPaM.GetIndex(), true ) != pParaPortion->GetLineNumber( aPaM.GetIndex(), false );
    }

    if ( !IsInsertMode() && !mpImpl->maSelection.HasRange() )
    {
        TextNode* pNode = mpImpl->mpTextEngine->mpDoc->GetNodes()[ aPaM.GetPara() ].get();
        if ( !pNode->GetText().isEmpty() && ( aPaM.GetIndex() < pNode->GetText().getLength() ) )
        {
            // If we are behind a portion, and the next portion has other direction, we must change position...
            aEditCursor.SetLeft( mpImpl->mpTextEngine->GetEditCursor( aPaM, false, true ).Left() );
            aEditCursor.SetRight( aEditCursor.Left() );

            TEParaPortion* pParaPortion = mpImpl->mpTextEngine->mpTEParaPortions->GetObject( aPaM.GetPara() );

            sal_Int32 nTextPortionStart = 0;
            std::size_t nTextPortion = pParaPortion->GetTextPortions().FindPortion( aPaM.GetIndex(), nTextPortionStart, true );
            TETextPortion& rTextPortion = pParaPortion->GetTextPortions()[ nTextPortion ];
            if ( rTextPortion.GetKind() == PORTIONKIND_TAB )
            {
                aEditCursor.AdjustRight(rTextPortion.GetWidth() );
            }
            else
            {
                TextPaM aNext = CursorRight( TextPaM( aPaM.GetPara(), aPaM.GetIndex() ), sal_uInt16(css::i18n::CharacterIteratorMode::SKIPCELL) );
                aEditCursor.SetRight( mpImpl->mpTextEngine->GetEditCursor( aNext, true ).Left() );
            }
        }
    }

    Size aOutSz = mpImpl->mpWindow->GetOutputSizePixel();
    if ( aEditCursor.GetHeight() > aOutSz.Height() )
        aEditCursor.SetBottom( aEditCursor.Top() + aOutSz.Height() - 1 );

    aEditCursor.AdjustLeft( -1 );

    if ( bGotoCursor
        // #i81283# protect maStartDocPos against initialization problems
        && aOutSz.Width() && aOutSz.Height()
    )
    {
        tools::Long nVisStartY = mpImpl->maStartDocPos.Y();
        tools::Long nVisEndY = mpImpl->maStartDocPos.Y() + aOutSz.Height();
        tools::Long nVisStartX = mpImpl->maStartDocPos.X();
        tools::Long nVisEndX = mpImpl->maStartDocPos.X() + aOutSz.Width();
        tools::Long nMoreX = aOutSz.Width() / 4;

        Point aNewStartPos( mpImpl->maStartDocPos );

        if ( aEditCursor.Bottom() > nVisEndY )
        {
            aNewStartPos.AdjustY( aEditCursor.Bottom() - nVisEndY);
        }
        else if ( aEditCursor.Top() < nVisStartY )
        {
            aNewStartPos.AdjustY( -( nVisStartY - aEditCursor.Top() ) );
        }

        if ( aEditCursor.Right() >= nVisEndX )
        {
            aNewStartPos.AdjustX( aEditCursor.Right() - nVisEndX );

            // do you want some more?
            aNewStartPos.AdjustX(nMoreX );
        }
        else if ( aEditCursor.Left() <= nVisStartX )
        {
            aNewStartPos.AdjustX( -( nVisStartX - aEditCursor.Left() ) );

            // do you want some more?
            aNewStartPos.AdjustX( -nMoreX );
        }

        // X can be wrong for the 'some more' above:
//      sal_uLong nMaxTextWidth = mpImpl->mpTextEngine->GetMaxTextWidth();
//      if ( !nMaxTextWidth || ( nMaxTextWidth > 0x7FFFFFFF ) )
//          nMaxTextWidth = 0x7FFFFFFF;
//      long nMaxX = (long)nMaxTextWidth - aOutSz.Width();
        tools::Long nMaxX = mpImpl->mpTextEngine->CalcTextWidth() - aOutSz.Width();
        if ( nMaxX < 0 )
            nMaxX = 0;

        if ( aNewStartPos.X() < 0 )
            aNewStartPos.setX( 0 );
        else if ( aNewStartPos.X() > nMaxX )
            aNewStartPos.setX( nMaxX );

        // Y should not be further down than needed
        tools::Long nYMax = mpImpl->mpTextEngine->GetTextHeight() - aOutSz.Height();
        if ( nYMax < 0 )
            nYMax = 0;
        if ( aNewStartPos.Y() > nYMax )
            aNewStartPos.setY( nYMax );

        if ( aNewStartPos != mpImpl->maStartDocPos )
            Scroll( -(aNewStartPos.X() - mpImpl->maStartDocPos.X()), -(aNewStartPos.Y() - mpImpl->maStartDocPos.Y()) );
    }

    if ( aEditCursor.Right() < aEditCursor.Left() )
    {
        tools::Long n = aEditCursor.Left();
        aEditCursor.SetLeft( aEditCursor.Right() );
        aEditCursor.SetRight( n );
    }

    Point aPoint( GetWindowPos( !mpImpl->mpTextEngine->IsRightToLeft() ? aEditCursor.TopLeft() : aEditCursor.TopRight() ) );
    mpImpl->mpCursor->SetPos( aPoint );
    mpImpl->mpCursor->SetSize( aEditCursor.GetSize() );
    if ( bForceVisCursor && mpImpl->mbCursorEnabled )
        mpImpl->mpCursor->Show();
}

void TextView::SetCursorAtPoint( const Point& rPosPixel )
{
    mpImpl->mpTextEngine->CheckIdleFormatter();

    Point aDocPos = GetDocPos( rPosPixel );

    TextPaM aPaM = mpImpl->mpTextEngine->GetPaM( aDocPos );

    // aTmpNewSel: Diff between old and new; not the new selection
    TextSelection aTmpNewSel( mpImpl->maSelection.GetEnd(), aPaM );
    TextSelection aNewSel( mpImpl->maSelection );
    aNewSel.GetEnd() = aPaM;

    if ( !mpImpl->mpSelEngine->HasAnchor() )
    {
        if ( mpImpl->maSelection.GetStart() != aPaM )
            mpImpl->mpTextEngine->CursorMoved( mpImpl->maSelection.GetStart().GetPara() );
        aNewSel.GetStart() = aPaM;
        ImpSetSelection( aNewSel );
    }
    else
    {
        ImpSetSelection( aNewSel );
        ShowSelection( aTmpNewSel );
    }

    bool bForceCursor = !mpImpl->mpDDInfo; // && !mbInSelection
    ImpShowCursor( mpImpl->mbAutoScroll, bForceCursor, false );
}

bool TextView::IsSelectionAtPoint( const Point& rPosPixel )
{
    Point aDocPos = GetDocPos( rPosPixel );
    TextPaM aPaM = mpImpl->mpTextEngine->GetPaM( aDocPos );
    // BeginDrag is only called, however, if IsSelectionAtPoint()
    // Problem: IsSelectionAtPoint is not called by Command()
    // if before MBDown returned false.
    return IsInSelection( aPaM );
}

bool TextView::IsInSelection( const TextPaM& rPaM ) const
{
    TextSelection aSel = mpImpl->maSelection;
    aSel.Justify();

    const sal_uInt32 nStartNode = aSel.GetStart().GetPara();
    const sal_uInt32 nEndNode = aSel.GetEnd().GetPara();
    const sal_uInt32 nCurNode = rPaM.GetPara();

    if ( ( nCurNode > nStartNode ) && ( nCurNode < nEndNode ) )
        return true;

    if ( nStartNode == nEndNode )
    {
        if ( nCurNode == nStartNode )
            if ( ( rPaM.GetIndex() >= aSel.GetStart().GetIndex() ) && ( rPaM.GetIndex() < aSel.GetEnd().GetIndex() ) )
                return true;
    }
    else if ( ( nCurNode == nStartNode ) && ( rPaM.GetIndex() >= aSel.GetStart().GetIndex() ) )
        return true;
    else if ( ( nCurNode == nEndNode ) && ( rPaM.GetIndex() < aSel.GetEnd().GetIndex() ) )
        return true;

    return false;
}

void TextView::ImpHideDDCursor()
{
    if ( mpImpl->mpDDInfo && mpImpl->mpDDInfo->mbVisCursor )
    {
        mpImpl->mpDDInfo->maCursor.Hide();
        mpImpl->mpDDInfo->mbVisCursor = false;
    }
}

void TextView::ImpShowDDCursor()
{
    if ( !mpImpl->mpDDInfo->mbVisCursor )
    {
        tools::Rectangle aCursor = mpImpl->mpTextEngine->PaMtoEditCursor( mpImpl->mpDDInfo->maDropPos, true );
        aCursor.AdjustRight( 1 );
        aCursor.SetPos( GetWindowPos( aCursor.TopLeft() ) );

        mpImpl->mpDDInfo->maCursor.SetWindow( mpImpl->mpWindow );
        mpImpl->mpDDInfo->maCursor.SetPos( aCursor.TopLeft() );
        mpImpl->mpDDInfo->maCursor.SetSize( aCursor.GetSize() );
        mpImpl->mpDDInfo->maCursor.Show();
        mpImpl->mpDDInfo->mbVisCursor = true;
    }
}

void TextView::SetPaintSelection( bool bPaint )
{
    if ( bPaint != mpImpl->mbPaintSelection )
    {
        mpImpl->mbPaintSelection = bPaint;
        ShowSelection( mpImpl->maSelection );
    }
}

void TextView::Read( SvStream& rInput )
{
    mpImpl->mpTextEngine->Read( rInput, &mpImpl->maSelection );
    ShowCursor();
}

bool TextView::ImplTruncateNewText( OUString& rNewText ) const
{
    bool bTruncated = false;

    const sal_Int32 nMaxLen = mpImpl->mpTextEngine->GetMaxTextLen();
    // 0 means unlimited
    if( nMaxLen != 0 )
    {
        const sal_Int32 nCurLen = mpImpl->mpTextEngine->GetTextLen();

        const sal_Int32 nNewLen = rNewText.getLength();
        if ( nCurLen + nNewLen > nMaxLen )
        {
            // see how much text will be replaced
            const sal_Int32 nSelLen = mpImpl->mpTextEngine->GetTextLen( mpImpl->maSelection );
            if ( nCurLen + nNewLen - nSelLen > nMaxLen )
            {
                const sal_Int32 nTruncatedLen = nMaxLen - (nCurLen - nSelLen);
                rNewText = rNewText.copy( 0, nTruncatedLen );
                bTruncated = true;
            }
        }
    }
    return bTruncated;
}

bool TextView::ImplCheckTextLen( const OUString& rNewText ) const
{
    bool bOK = true;
    if ( mpImpl->mpTextEngine->GetMaxTextLen() )
    {
        sal_Int32 n = mpImpl->mpTextEngine->GetTextLen() + rNewText.getLength();
        if ( n > mpImpl->mpTextEngine->GetMaxTextLen() )
        {
            // calculate how much text is being deleted
            n -= mpImpl->mpTextEngine->GetTextLen( mpImpl->maSelection );
            if ( n > mpImpl->mpTextEngine->GetMaxTextLen() )
                bOK = false;
        }
    }
    return bOK;
}

void TextView::dragGestureRecognized( const css::datatransfer::dnd::DragGestureEvent& rDGE )
{
    if ( !mpImpl->mbClickedInSelection )
        return;

    SolarMutexGuard aVclGuard;

    SAL_WARN_IF( !mpImpl->maSelection.HasRange(), "vcl", "TextView::dragGestureRecognized: mpImpl->mbClickedInSelection, but no selection?" );

    mpImpl->mpDDInfo.reset(new TextDDInfo);
    mpImpl->mpDDInfo->mbStarterOfDD = true;

    rtl::Reference<TETextDataObject> pDataObj = new TETextDataObject( GetSelected() );

    mpImpl->mpCursor->Hide();

    sal_Int8 nActions = css::datatransfer::dnd::DNDConstants::ACTION_COPY;
    if ( !IsReadOnly() )
        nActions |= css::datatransfer::dnd::DNDConstants::ACTION_MOVE;
    rDGE.DragSource->startDrag( rDGE, nActions, 0 /*cursor*/, 0 /*image*/, pDataObj, mpImpl->mxDnDListener );
}

void TextView::dragDropEnd( const css::datatransfer::dnd::DragSourceDropEvent& )
{
    ImpHideDDCursor();
    mpImpl->mpDDInfo.reset();
}

void TextView::drop( const css::datatransfer::dnd::DropTargetDropEvent& rDTDE )
{
    SolarMutexGuard aVclGuard;

    if ( !mpImpl->mbReadOnly && mpImpl->mpDDInfo )
    {
        ImpHideDDCursor();

        // Data for deleting after DROP_MOVE:
        TextSelection aPrevSel( mpImpl->maSelection );
        aPrevSel.Justify();
        const sal_uInt32 nPrevParaCount = mpImpl->mpTextEngine->GetParagraphCount();
        const sal_Int32 nPrevStartParaLen = mpImpl->mpTextEngine->GetTextLen( aPrevSel.GetStart().GetPara() );

        bool bStarterOfDD = false;
        for ( sal_uInt16 nView = mpImpl->mpTextEngine->GetViewCount(); nView && !bStarterOfDD; )
            bStarterOfDD = mpImpl->mpTextEngine->GetView( --nView )->mpImpl->mpDDInfo && mpImpl->mpTextEngine->GetView( nView )->mpImpl->mpDDInfo->mbStarterOfDD;

        HideSelection();
        ImpSetSelection( mpImpl->mpDDInfo->maDropPos );

        mpImpl->mpTextEngine->UndoActionStart();

        OUString aText;
        css::uno::Reference< css::datatransfer::XTransferable > xDataObj = rDTDE.Transferable;
        if ( xDataObj.is() )
        {
            css::datatransfer::DataFlavor aFlavor;
            SotExchange::GetFormatDataFlavor( SotClipboardFormatId::STRING, aFlavor );
            if ( xDataObj->isDataFlavorSupported( aFlavor ) )
            {
                css::uno::Any aData = xDataObj->getTransferData( aFlavor );
                OUString aOUString;
                aData >>= aOUString;
                aText = convertLineEnd(aOUString, LINEEND_LF);
            }
        }

        if ( !aText.isEmpty() && ( aText[ aText.getLength()-1 ] == LINE_SEP ) )
            aText = aText.copy(0, aText.getLength()-1);

        if ( ImplCheckTextLen( aText ) )
            ImpSetSelection( mpImpl->mpTextEngine->ImpInsertText( mpImpl->mpDDInfo->maDropPos, aText ) );

        if ( aPrevSel.HasRange() &&
                (( rDTDE.DropAction & css::datatransfer::dnd::DNDConstants::ACTION_MOVE ) || !bStarterOfDD) )
        {
            // adjust selection if necessary
            if ( ( mpImpl->mpDDInfo->maDropPos.GetPara() < aPrevSel.GetStart().GetPara() ) ||
                 ( ( mpImpl->mpDDInfo->maDropPos.GetPara() == aPrevSel.GetStart().GetPara() )
                        && ( mpImpl->mpDDInfo->maDropPos.GetIndex() < aPrevSel.GetStart().GetIndex() ) ) )
            {
                const sal_uInt32 nNewParasBeforeSelection =
                    mpImpl->mpTextEngine->GetParagraphCount() - nPrevParaCount;

                aPrevSel.GetStart().GetPara() += nNewParasBeforeSelection;
                aPrevSel.GetEnd().GetPara() += nNewParasBeforeSelection;

                if ( mpImpl->mpDDInfo->maDropPos.GetPara() == aPrevSel.GetStart().GetPara() )
                {
                    const sal_Int32 nNewChars =
                        mpImpl->mpTextEngine->GetTextLen( aPrevSel.GetStart().GetPara() ) - nPrevStartParaLen;

                    aPrevSel.GetStart().GetIndex() += nNewChars;
                    if ( aPrevSel.GetStart().GetPara() == aPrevSel.GetEnd().GetPara() )
                        aPrevSel.GetEnd().GetIndex() += nNewChars;
                }
            }
            else
            {
                // adjust current selection
                TextPaM aPaM = mpImpl->maSelection.GetStart();
                aPaM.GetPara() -= ( aPrevSel.GetEnd().GetPara() - aPrevSel.GetStart().GetPara() );
                if ( aPrevSel.GetEnd().GetPara() == mpImpl->mpDDInfo->maDropPos.GetPara() )
                {
                    aPaM.GetIndex() -= aPrevSel.GetEnd().GetIndex();
                    if ( aPrevSel.GetStart().GetPara() == mpImpl->mpDDInfo->maDropPos.GetPara() )
                        aPaM.GetIndex() += aPrevSel.GetStart().GetIndex();
                }
                ImpSetSelection( aPaM );

            }
            mpImpl->mpTextEngine->ImpDeleteText( aPrevSel );
        }

        mpImpl->mpTextEngine->UndoActionEnd();

        mpImpl->mpDDInfo.reset();

        mpImpl->mpTextEngine->FormatAndUpdate( this );

        mpImpl->mpTextEngine->Broadcast( TextHint( SfxHintId::TextModified ) );
    }
    rDTDE.Context->dropComplete( false/*bChanges*/ );
}

void TextView::dragEnter( const css::datatransfer::dnd::DropTargetDragEnterEvent& )
{
}

void TextView::dragExit( const css::datatransfer::dnd::DropTargetEvent& )
{
    SolarMutexGuard aVclGuard;
    ImpHideDDCursor();
}

void TextView::dragOver( const css::datatransfer::dnd::DropTargetDragEvent& rDTDE )
{
    SolarMutexGuard aVclGuard;

    if (!mpImpl->mpDDInfo)
        mpImpl->mpDDInfo.reset(new TextDDInfo);

    TextPaM aPrevDropPos = mpImpl->mpDDInfo->maDropPos;
    Point aMousePos( rDTDE.LocationX, rDTDE.LocationY );
    Point aDocPos = GetDocPos( aMousePos );
    mpImpl->mpDDInfo->maDropPos = mpImpl->mpTextEngine->GetPaM( aDocPos );

    // Don't drop in selection or in read only engine
    if ( IsReadOnly() || IsInSelection( mpImpl->mpDDInfo->maDropPos ))
    {
        ImpHideDDCursor();
        rDTDE.Context->rejectDrag();
    }
    else
    {
        // delete old Cursor
        if ( !mpImpl->mpDDInfo->mbVisCursor || ( aPrevDropPos != mpImpl->mpDDInfo->maDropPos ) )
        {
            ImpHideDDCursor();
            ImpShowDDCursor();
        }
        rDTDE.Context->acceptDrag( rDTDE.DropAction );
    }
}

Point TextView::ImpGetOutputStartPos( const Point& rStartDocPos ) const
{
    Point aStartPos( -rStartDocPos.X(), -rStartDocPos.Y() );
    if ( mpImpl->mpTextEngine->IsRightToLeft() )
    {
        Size aSz = mpImpl->mpWindow->GetOutputSizePixel();
        aStartPos.setX( rStartDocPos.X() + aSz.Width() - 1 ); // -1: Start is 0
    }
    return aStartPos;
}

Point TextView::GetDocPos( const Point& rWindowPos ) const
{
    // Window Position => Document Position

    Point aPoint;

    aPoint.setY( rWindowPos.Y() + mpImpl->maStartDocPos.Y() );

    if ( !mpImpl->mpTextEngine->IsRightToLeft() )
    {
        aPoint.setX( rWindowPos.X() + mpImpl->maStartDocPos.X() );
    }
    else
    {
        Size aSz = mpImpl->mpWindow->GetOutputSizePixel();
        aPoint.setX( ( aSz.Width() - 1 ) - rWindowPos.X() + mpImpl->maStartDocPos.X() );
    }

    return aPoint;
}

Point TextView::GetWindowPos( const Point& rDocPos ) const
{
    // Document Position => Window Position

    Point aPoint;

    aPoint.setY( rDocPos.Y() - mpImpl->maStartDocPos.Y() );

    if ( !mpImpl->mpTextEngine->IsRightToLeft() )
    {
        aPoint.setX( rDocPos.X() - mpImpl->maStartDocPos.X() );
    }
    else
    {
        Size aSz = mpImpl->mpWindow->GetOutputSizePixel();
        aPoint.setX( ( aSz.Width() - 1 ) - ( rDocPos.X() - mpImpl->maStartDocPos.X() ) );
    }

    return aPoint;
}

sal_Int32 TextView::GetLineNumberOfCursorInSelection() const
{
 // PROGRESS
    sal_Int32 nLineNo = -1;
    if( mpImpl->mbCursorEnabled )
    {
        TextPaM aPaM = GetSelection().GetEnd();
        TEParaPortion* pPPortion = mpImpl->mpTextEngine->mpTEParaPortions->GetObject( aPaM.GetPara() );
        nLineNo = pPPortion->GetLineNumber( aPaM.GetIndex(), false );
            //TODO: std::vector<TextLine>::size_type -> sal_Int32!
        if( mpImpl->mbCursorAtEndOfLine )
            --nLineNo;
    }
    return nLineNo;
}

// (+) class TextSelFunctionSet

TextSelFunctionSet::TextSelFunctionSet( TextView* pView )
{
    mpView = pView;
}

void TextSelFunctionSet::BeginDrag()
{
}

void TextSelFunctionSet::CreateAnchor()
{
//  TextSelection aSel( mpView->GetSelection() );
//  aSel.GetStart() = aSel.GetEnd();
//  mpView->SetSelection( aSel );

    // may not be followed by ShowCursor
    mpView->HideSelection();
    mpView->ImpSetSelection( mpView->mpImpl->maSelection.GetEnd() );
}

void TextSelFunctionSet::SetCursorAtPoint( const Point& rPointPixel, bool )
{
    mpView->SetCursorAtPoint( rPointPixel );
}

bool TextSelFunctionSet::IsSelectionAtPoint( const Point& rPointPixel )
{
    return mpView->IsSelectionAtPoint( rPointPixel );
}

void TextSelFunctionSet::DeselectAll()
{
    CreateAnchor();
}

void TextSelFunctionSet::DeselectAtPoint( const Point& )
{
    // only for multiple selection
}

void TextSelFunctionSet::DestroyAnchor()
{
    // only for multiple selection
}
TextEngine*         TextView::GetTextEngine() const
{ return mpImpl->mpTextEngine; }
vcl::Window*             TextView::GetWindow() const
{ return mpImpl->mpWindow; }
void                TextView::EnableCursor( bool bEnable )
{ mpImpl->mbCursorEnabled = bEnable; }
bool                TextView::IsCursorEnabled() const
{ return mpImpl->mbCursorEnabled; }
void                TextView::SetStartDocPos( const Point& rPos )
{ mpImpl->maStartDocPos = rPos; }
const Point&        TextView::GetStartDocPos() const
{ return mpImpl->maStartDocPos; }
void                TextView::SetAutoIndentMode( bool bAutoIndent )
{ mpImpl->mbAutoIndent = bAutoIndent; }
bool                TextView::IsReadOnly() const
{ return mpImpl->mbReadOnly; }
void                TextView::SetAutoScroll( bool bAutoScroll )
{ mpImpl->mbAutoScroll = bAutoScroll; }
bool                TextView::IsAutoScroll() const
{ return mpImpl->mbAutoScroll; }
bool                TextView::HasSelection() const
{ return mpImpl->maSelection.HasRange(); }
bool                TextView::IsInsertMode() const
{ return mpImpl->mbInsertMode; }

void TextView::MatchGroup()
{
    TextSelection aTmpSel( GetSelection() );
    aTmpSel.Justify();
    if ( ( aTmpSel.GetStart().GetPara() != aTmpSel.GetEnd().GetPara() ) ||
         ( ( aTmpSel.GetEnd().GetIndex() - aTmpSel.GetStart().GetIndex() ) > 1 ) )
    {
        return;
    }

    TextSelection aMatchSel = static_cast<ExtTextEngine*>(GetTextEngine())->MatchGroup( aTmpSel.GetStart() );
    if ( aMatchSel.HasRange() )
        SetSelection( aMatchSel );
}

void TextView::CenterPaM( const TextPaM& rPaM )
{
    // Get textview size and the corresponding y-coordinates
    Size aOutSz = mpImpl->mpWindow->GetOutputSizePixel();
    tools::Long nVisStartY = mpImpl->maStartDocPos.Y();
    tools::Long nVisEndY = mpImpl->maStartDocPos.Y() + aOutSz.Height();

    // Retrieve the coordinates of the PaM
    tools::Rectangle aRect = mpImpl->mpTextEngine->PaMtoEditCursor(rPaM);

    // Recalculate the offset of the center y-coordinates and scroll
    Scroll(0, (nVisStartY + nVisEndY) / 2 - aRect.TopLeft().getY());
}

bool TextView::Search( const i18nutil::SearchOptions& rSearchOptions, bool bForward )
{
    bool bFound = false;
    TextSelection aSel( GetSelection() );
    if ( static_cast<ExtTextEngine*>(GetTextEngine())->Search( aSel, rSearchOptions, bForward ) )
    {
        bFound = true;
        // First add the beginning of the word to the selection,
        // so that the whole word is in the visible region.
        SetSelection( aSel.GetStart() );
        ShowCursor( true, false );
    }
    else
    {
        aSel = GetSelection().GetEnd();
    }

    SetSelection( aSel );
    // tdf#49482: Move the start of the selection to the center of the textview
    if (bFound)
    {
        CenterPaM( aSel.GetStart() );
    }
    ShowCursor();

    return bFound;
}

sal_uInt16 TextView::Replace( const i18nutil::SearchOptions& rSearchOptions, bool bAll, bool bForward )
{
    sal_uInt16 nFound = 0;

    if ( !bAll )
    {
        if ( GetSelection().HasRange() )
        {
            InsertText( rSearchOptions.replaceString );
            nFound = 1;
            Search( rSearchOptions, bForward ); // right away to the next
        }
        else
        {
            if( Search( rSearchOptions, bForward ) )
                nFound = 1;
        }
    }
    else
    {
        // the writer replaces all, from beginning to end

        ExtTextEngine* pTextEngine = static_cast<ExtTextEngine*>(GetTextEngine());

        // HideSelection();
        TextSelection aSel;

        bool bSearchInSelection = (0 != (rSearchOptions.searchFlag & css::util::SearchFlags::REG_NOT_BEGINOFLINE) );
        if ( bSearchInSelection )
        {
            aSel = GetSelection();
            aSel.Justify();
        }

        TextSelection aSearchSel( aSel );

        bool bFound = pTextEngine->Search( aSel, rSearchOptions );
        if ( bFound )
            pTextEngine->UndoActionStart();
        while ( bFound )
        {
            nFound++;

            TextPaM aNewStart = pTextEngine->ImpInsertText( aSel, rSearchOptions.replaceString );
            // tdf#64690 - extend selection to include inserted text portions
            if ( aSel.GetEnd().GetPara() == aSearchSel.GetEnd().GetPara() )
            {
                aSearchSel.GetEnd().GetIndex() += rSearchOptions.replaceString.getLength() - 1;
            }
            aSel = aSearchSel;
            aSel.GetStart() = aNewStart;
            bFound = pTextEngine->Search( aSel, rSearchOptions );
        }
        if ( nFound )
        {
            SetSelection( aSel.GetStart() );
            pTextEngine->FormatAndUpdate( this );
            pTextEngine->UndoActionEnd();
        }
    }
    return nFound;
}

bool TextView::ImpIndentBlock( bool bRight )
{
    bool bDone = false;

    TextSelection aSel = GetSelection();
    aSel.Justify();

    HideSelection();
    GetTextEngine()->UndoActionStart();

    const sal_uInt32 nStartPara = aSel.GetStart().GetPara();
    sal_uInt32 nEndPara = aSel.GetEnd().GetPara();
    if ( aSel.HasRange() && !aSel.GetEnd().GetIndex() )
    {
        nEndPara--; // do not indent
    }

    for ( sal_uInt32 nPara = nStartPara; nPara <= nEndPara; ++nPara )
    {
        if ( bRight )
        {
            // add tabs
            GetTextEngine()->ImpInsertText( TextPaM( nPara, 0 ), '\t' );
            bDone = true;
        }
        else
        {
            // remove Tabs/Blanks
            OUString aText = GetTextEngine()->GetText( nPara );
            if ( !aText.isEmpty() && (
                    ( aText[ 0 ] == '\t' ) ||
                    ( aText[ 0 ] == ' ' ) ) )
            {
                GetTextEngine()->ImpDeleteText( TextSelection( TextPaM( nPara, 0 ), TextPaM( nPara, 1 ) ) );
                bDone = true;
            }
        }
    }

    GetTextEngine()->UndoActionEnd();

    bool bRange = aSel.HasRange();
    if ( bRight )
    {
        ++aSel.GetStart().GetIndex();
        if ( bRange && ( aSel.GetEnd().GetPara() == nEndPara ) )
            ++aSel.GetEnd().GetIndex();
    }
    else
    {
        if ( aSel.GetStart().GetIndex() )
            --aSel.GetStart().GetIndex();
        if ( bRange && aSel.GetEnd().GetIndex() )
            --aSel.GetEnd().GetIndex();
    }

    ImpSetSelection( aSel );
    GetTextEngine()->FormatAndUpdate( this );

    return bDone;
}

bool TextView::IndentBlock()
{
    return ImpIndentBlock( true );
}

bool TextView::UnindentBlock()
{
    return ImpIndentBlock( false );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
