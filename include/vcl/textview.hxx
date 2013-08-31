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

#ifndef _TEXTVIEW_HXX
#define _TEXTVIEW_HXX

#include <vcl/dllapi.h>
#include <vcl/textdata.hxx>
#include <tools/gen.hxx>
#include <vcl/dndhelp.hxx>

class TextEngine;
class OutputDevice;
class Window;
class Cursor;
class KeyEvent;
class MouseEvent;
class CommandEvent;
class TextSelFunctionSet;
class SelectionEngine;
class VirtualDevice;
struct TextDDInfo;

namespace com {
namespace sun {
namespace star {
namespace datatransfer {
namespace clipboard {
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
    ImpTextView*        mpImpl;

                        TextView( const TextView& ) : vcl::unohelper::DragAndDropClient()       {}
    TextView&           operator=( const TextView& )        { return *this; }

protected:
    void                ShowSelection();
    void                HideSelection();
    void                ShowSelection( const TextSelection& rSel );
    void                ImpShowHideSelection( sal_Bool bShow, const TextSelection* pRange = NULL );

    TextSelection       ImpMoveCursor( const KeyEvent& rKeyEvent );
    TextPaM             ImpDelete( sal_uInt8 nMode, sal_uInt8 nDelMode );
    void                ImpSetSelection( const TextSelection& rNewSel, sal_Bool bUI );
    sal_Bool                IsInSelection( const TextPaM& rPaM );

    void                ImpPaint( OutputDevice* pOut, const Point& rStartPos, Rectangle const* pPaintArea, TextSelection const* pPaintRange = 0, TextSelection const* pSelection = 0 );
    void                ImpPaint( const Rectangle& rRect, sal_Bool bUseVirtDev );
    void                ImpShowCursor( sal_Bool bGotoCursor, sal_Bool bForceVisCursor, sal_Bool bEndKey );
    void                ImpHighlight( const TextSelection& rSel );
    void                ImpSetSelection( const TextSelection& rSelection );
    Point               ImpGetOutputStartPos( const Point& rStartDocPos ) const;

    void                ImpHideDDCursor();
    void                ImpShowDDCursor();

    bool                ImplTruncateNewText( OUString& rNewText ) const;
    sal_Bool            ImplCheckTextLen( const OUString& rNewText );

    VirtualDevice*      GetVirtualDevice();

    // DragAndDropClient
    virtual void        dragGestureRecognized( const ::com::sun::star::datatransfer::dnd::DragGestureEvent& dge ) throw (::com::sun::star::uno::RuntimeException);
    virtual void        dragDropEnd( const ::com::sun::star::datatransfer::dnd::DragSourceDropEvent& dsde ) throw (::com::sun::star::uno::RuntimeException);
    virtual void        drop( const ::com::sun::star::datatransfer::dnd::DropTargetDropEvent& dtde ) throw (::com::sun::star::uno::RuntimeException);
    virtual void        dragEnter( const ::com::sun::star::datatransfer::dnd::DropTargetDragEnterEvent& dtdee ) throw (::com::sun::star::uno::RuntimeException);
    virtual void        dragExit( const ::com::sun::star::datatransfer::dnd::DropTargetEvent& dte ) throw (::com::sun::star::uno::RuntimeException);
    virtual void        dragOver( const ::com::sun::star::datatransfer::dnd::DropTargetDragEvent& dtde ) throw (::com::sun::star::uno::RuntimeException);

            using       DragAndDropClient::dragEnter;
            using       DragAndDropClient::dragExit;
            using       DragAndDropClient::dragOver;

public:
                        TextView( TextEngine* pEng, Window* pWindow );
    virtual            ~TextView();

    TextEngine*         GetTextEngine() const;
    Window*             GetWindow() const;

    void                Invalidate();
    void                Scroll( long nHorzScroll, long nVertScroll );

    void                ShowCursor( sal_Bool bGotoCursor = sal_True, sal_Bool bForceVisCursor = sal_True );
    void                HideCursor();

    void                EnableCursor( sal_Bool bEnable );
    sal_Bool                IsCursorEnabled() const;

    const TextSelection&    GetSelection() const;
    TextSelection&      GetSelection();
    void                SetSelection( const TextSelection& rNewSel );
    void                SetSelection( const TextSelection& rNewSel, sal_Bool bGotoCursor );
    sal_Bool                HasSelection() const;

    OUString            GetSelected();
    OUString            GetSelected( LineEnd aSeparator );
    void                DeleteSelected();

    void                InsertText( const OUString& rNew, sal_Bool bSelect = sal_False );

    sal_Bool                KeyInput( const KeyEvent& rKeyEvent );
    void                Paint( const Rectangle& rRect );
    void                MouseButtonUp( const MouseEvent& rMouseEvent );
    void                MouseButtonDown( const MouseEvent& rMouseEvent );
    void                MouseMove( const MouseEvent& rMouseEvent );
    void                Command( const CommandEvent& rCEvt );

    void                Cut();
    void                Copy();
    void                Paste();

    void                Copy( ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboard >& rxClipboard );
    void                Paste( ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboard >& rxClipboard );

    void                Undo();
    void                Redo();

    sal_Bool                Read( SvStream& rInput );

    void                SetStartDocPos( const Point& rPos );
    const Point&        GetStartDocPos() const;

    Point               GetDocPos( const Point& rWindowPos ) const;
    Point               GetWindowPos( const Point& rDocPos ) const;

    void                SetInsertMode( sal_Bool bInsert );
    sal_Bool                IsInsertMode() const;

    void                SetAutoIndentMode( sal_Bool bAutoIndent );

    void                SetReadOnly( sal_Bool bReadOnly );
    sal_Bool                IsReadOnly() const;

    void                SetAutoScroll( sal_Bool bAutoScroll );
    sal_Bool                IsAutoScroll() const;

    sal_Bool                SetCursorAtPoint( const Point& rPointPixel );
    sal_Bool                IsSelectionAtPoint( const Point& rPointPixel );

    void                SetPaintSelection( sal_Bool bPaint);

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
    TextPaM             CursorStartOfParagraph( const TextPaM& rPaM );
    TextPaM             CursorEndOfParagraph( const TextPaM& rPaM );
    TextPaM             CursorStartOfDoc();
    TextPaM             CursorEndOfDoc();

    /**
        Drag and Drop, deleting and selection regards all text that has an attribute
        TEXTATTR_PROTECTED set as one entitity. Drag and dropped text is automatically
        attibuted as protected.
     */
    void                SupportProtectAttribute(sal_Bool bSupport);

    /**
        Returns the number in paragraph of the line in which the cursor is blinking
        if enabled, -1 otherwise.
     */
    sal_Int32           GetLineNumberOfCursorInSelection() const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
