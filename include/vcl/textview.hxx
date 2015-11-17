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

#ifndef INCLUDED_VCL_TEXTVIEW_HXX
#define INCLUDED_VCL_TEXTVIEW_HXX

#include <tools/gen.hxx>
#include <tools/lineend.hxx>
#include <vcl/dllapi.h>
#include <vcl/dndhelp.hxx>
#include <vcl/textdata.hxx>
#include <vcl/window.hxx>
#include <memory>

class TextEngine;
class OutputDevice;

class KeyEvent;
class MouseEvent;
class CommandEvent;
class TextSelFunctionSet;
class SelectionEngine;
class VirtualDevice;
struct TextDDInfo;

namespace com { namespace sun { namespace star { namespace datatransfer { namespace clipboard {
    class XClipboard;
}}}}}

struct ImpTextView;

class VCL_DLLPUBLIC TextView : public vcl::unohelper::DragAndDropClient
{
    friend class        TextEngine;
    friend class        TextUndo;
    friend class        TextUndoManager;
    friend class        TextSelFunctionSet;
    friend class        ExtTextView;

private:
    std::unique_ptr<ImpTextView>  mpImpl;

                        TextView( const TextView& ) = delete;
    TextView&           operator=( const TextView& ) = delete;

protected:
    void                ShowSelection();
    void                HideSelection();
    void                ShowSelection( const TextSelection& rSel );
    void                ImpShowHideSelection( bool bShow, const TextSelection* pRange = nullptr );

    TextSelection       ImpMoveCursor( const KeyEvent& rKeyEvent );
    TextPaM             ImpDelete( sal_uInt8 nMode, sal_uInt8 nDelMode );
    bool                IsInSelection( const TextPaM& rPaM );

    void                ImpPaint(vcl::RenderContext& rRenderContext, const Point& rStartPos, Rectangle const* pPaintArea, TextSelection const* pPaintRange = nullptr, TextSelection const* pSelection = nullptr);
    void                ImpPaint(vcl::RenderContext& rRenderContext, const Rectangle& rRect, bool bUseVirtDev);
    void                ImpShowCursor( bool bGotoCursor, bool bForceVisCursor, bool bEndKey );
    void                ImpHighlight( const TextSelection& rSel );
    void                ImpSetSelection( const TextSelection& rSelection );
    Point               ImpGetOutputStartPos( const Point& rStartDocPos ) const;

    void                ImpHideDDCursor();
    void                ImpShowDDCursor();

    bool                ImplTruncateNewText( OUString& rNewText ) const;
    bool                ImplCheckTextLen( const OUString& rNewText );

    VirtualDevice*      GetVirtualDevice();

    // DragAndDropClient
    virtual void        dragGestureRecognized( const css::datatransfer::dnd::DragGestureEvent& dge ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void        dragDropEnd( const css::datatransfer::dnd::DragSourceDropEvent& dsde ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void        drop( const css::datatransfer::dnd::DropTargetDropEvent& dtde ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void        dragEnter( const css::datatransfer::dnd::DropTargetDragEnterEvent& dtdee ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void        dragExit( const css::datatransfer::dnd::DropTargetEvent& dte ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void        dragOver( const css::datatransfer::dnd::DropTargetDragEvent& dtde ) throw (css::uno::RuntimeException, std::exception) override;

            using       DragAndDropClient::dragEnter;
            using       DragAndDropClient::dragExit;
            using       DragAndDropClient::dragOver;

public:
                        TextView( TextEngine* pEng, vcl::Window* pWindow );
    virtual            ~TextView();

    TextEngine*         GetTextEngine() const;
    vcl::Window*             GetWindow() const;

    void                Invalidate();
    void                Scroll( long nHorzScroll, long nVertScroll );

    void                ShowCursor( bool bGotoCursor = true, bool bForceVisCursor = true );
    void                HideCursor();

    void                EnableCursor( bool bEnable );
    bool                IsCursorEnabled() const;

    const TextSelection&    GetSelection() const;
    TextSelection&      GetSelection();
    void                SetSelection( const TextSelection& rNewSel );
    void                SetSelection( const TextSelection& rNewSel, bool bGotoCursor );
    bool                HasSelection() const;

    OUString            GetSelected();
    OUString            GetSelected( LineEnd aSeparator );
    void                DeleteSelected();

    void                InsertText( const OUString& rNew, bool bSelect = false );

    bool                KeyInput( const KeyEvent& rKeyEvent );
    void                Paint(vcl::RenderContext& rRenderContext, const Rectangle& rRect);
    void                MouseButtonUp( const MouseEvent& rMouseEvent );
    void                MouseButtonDown( const MouseEvent& rMouseEvent );
    void                MouseMove( const MouseEvent& rMouseEvent );
    void                Command( const CommandEvent& rCEvt );

    void                Cut();
    void                Copy();
    void                Paste();

    void                Copy( css::uno::Reference< css::datatransfer::clipboard::XClipboard >& rxClipboard );
    void                Paste( css::uno::Reference< css::datatransfer::clipboard::XClipboard >& rxClipboard );

    void                Undo();
    void                Redo();

    bool            Read( SvStream& rInput );

    void                SetStartDocPos( const Point& rPos );
    const Point&        GetStartDocPos() const;

    Point               GetDocPos( const Point& rWindowPos ) const;
    Point               GetWindowPos( const Point& rDocPos ) const;

    void                SetInsertMode( bool bInsert );
    bool            IsInsertMode() const;

    void                SetAutoIndentMode( bool bAutoIndent );

    void                SetReadOnly( bool bReadOnly );
    bool            IsReadOnly() const;

    void                SetAutoScroll( bool bAutoScroll );
    bool            IsAutoScroll() const;

    bool            SetCursorAtPoint( const Point& rPointPixel );
    bool            IsSelectionAtPoint( const Point& rPointPixel );

    void                SetPaintSelection( bool bPaint);

    void                EraseVirtualDevice();

    // aus dem protected Teil hierher verschoben
    // For 'SvtXECTextCursor' (TL). Must ggf nochmal anders gel?st werden.
    TextPaM             PageUp( const TextPaM& rPaM );
    TextPaM             PageDown( const TextPaM& rPaM );
    TextPaM             CursorUp( const TextPaM& rPaM );
    TextPaM             CursorDown( const TextPaM& rPaM );
    TextPaM             CursorLeft( const TextPaM& rPaM, sal_uInt16 nCharacterIteratorMode );
    TextPaM             CursorRight( const TextPaM& rPaM, sal_uInt16 nCharacterIteratorMode );
    TextPaM             CursorWordLeft( const TextPaM& rPaM );
    TextPaM             CursorWordRight( const TextPaM& rPaM );
    TextPaM             CursorStartOfLine( const TextPaM& rPaM );
    TextPaM             CursorEndOfLine( const TextPaM& rPaM );
    static TextPaM      CursorStartOfParagraph( const TextPaM& rPaM );
    TextPaM             CursorEndOfParagraph( const TextPaM& rPaM );
    static TextPaM      CursorStartOfDoc();
    TextPaM             CursorEndOfDoc();

    /**
        Drag and Drop, deleting and selection regards all text that has an attribute
        TEXTATTR_PROTECTED set as one entity. Drag and dropped text is automatically
        attributed as protected.
     */
    void                SupportProtectAttribute(bool bSupport);

    /**
        Returns the number in paragraph of the line in which the cursor is blinking
        if enabled, -1 otherwise.
     */
    sal_Int32           GetLineNumberOfCursorInSelection() const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
