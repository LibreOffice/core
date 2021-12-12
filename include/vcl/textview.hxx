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

#include <config_options.h>
#include <tools/gen.hxx>
#include <tools/lineend.hxx>
#include <tools/stream.hxx>
#include <vcl/dllapi.h>
#include <vcl/dndhelp.hxx>
#include <vcl/textdata.hxx>
#include <vcl/outdev.hxx>
#include <memory>

class TextEngine;

class KeyEvent;
class MouseEvent;
class CommandEvent;
namespace vcl { class Window; }

namespace com::sun::star::datatransfer::clipboard {
    class XClipboard;
}
namespace i18nutil {
    struct SearchOptions;
}


struct ImpTextView;
class ExtTextEngine;

class UNLESS_MERGELIBS(VCL_DLLPUBLIC) TETextDataObject final : public css::datatransfer::XTransferable,
                        public ::cppu::OWeakObject

{
private:
    OUString        maText;
    SvMemoryStream  maHTMLStream;

public:
    explicit TETextDataObject( const OUString& rText );

    SvMemoryStream& GetHTMLStream() { return maHTMLStream; }

    // css::uno::XInterface
    css::uno::Any                               SAL_CALL queryInterface( const css::uno::Type & rType ) override;
    void                                        SAL_CALL acquire() noexcept override  { OWeakObject::acquire(); }
    void                                        SAL_CALL release() noexcept override  { OWeakObject::release(); }

    // css::datatransfer::XTransferable
    css::uno::Any SAL_CALL getTransferData( const css::datatransfer::DataFlavor& aFlavor ) override;
    css::uno::Sequence< css::datatransfer::DataFlavor > SAL_CALL getTransferDataFlavors(  ) override;
    sal_Bool SAL_CALL isDataFlavorSupported( const css::datatransfer::DataFlavor& aFlavor ) override;
};


class VCL_DLLPUBLIC TextView final : public vcl::unohelper::DragAndDropClient
{
    friend class        TextEngine;
    friend class        TextUndo;
    friend class        TextUndoManager;
    friend class        TextSelFunctionSet;

    std::unique_ptr<ImpTextView>  mpImpl;

                        TextView( const TextView& ) = delete;
    TextView&           operator=( const TextView& ) = delete;

    bool                ImpIndentBlock( bool bRight );
    void                ShowSelection();
    void                HideSelection();
    void                ShowSelection( const TextSelection& rSel );
    void                ImpShowHideSelection( const TextSelection* pRange = nullptr );

    TextSelection const & ImpMoveCursor( const KeyEvent& rKeyEvent );
    TextPaM             ImpDelete( sal_uInt8 nMode, sal_uInt8 nDelMode );
    bool                IsInSelection( const TextPaM& rPaM ) const;

    void                ImpPaint(vcl::RenderContext& rRenderContext, const Point& rStartPos, tools::Rectangle const* pPaintArea, TextSelection const* pSelection);
    void                ImpPaint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect);
    void                ImpShowCursor( bool bGotoCursor, bool bForceVisCursor, bool bEndKey );
    void                ImpSetSelection( const TextSelection& rSelection );
    Point               ImpGetOutputStartPos( const Point& rStartDocPos ) const;

    void                ImpHideDDCursor();
    void                ImpShowDDCursor();

    bool                ImplTruncateNewText( OUString& rNewText ) const;
    bool                ImplCheckTextLen( const OUString& rNewText ) const;

    // DragAndDropClient
    virtual void        dragGestureRecognized( const css::datatransfer::dnd::DragGestureEvent& dge ) override;
    virtual void        dragDropEnd( const css::datatransfer::dnd::DragSourceDropEvent& dsde ) override;
    virtual void        drop( const css::datatransfer::dnd::DropTargetDropEvent& dtde ) override;
    virtual void        dragEnter( const css::datatransfer::dnd::DropTargetDragEnterEvent& dtdee ) override;
    virtual void        dragExit( const css::datatransfer::dnd::DropTargetEvent& dte ) override;
    virtual void        dragOver( const css::datatransfer::dnd::DropTargetDragEvent& dtde ) override;

            using       DragAndDropClient::dragEnter;
            using       DragAndDropClient::dragExit;
            using       DragAndDropClient::dragOver;

public:
                        TextView( ExtTextEngine* pEng, vcl::Window* pWindow );
    virtual            ~TextView() override;

    TextEngine*         GetTextEngine() const;
    vcl::Window*             GetWindow() const;

    void                Invalidate();
    void                Scroll( tools::Long nHorzScroll, tools::Long nVertScroll );

    void                ShowCursor( bool bGotoCursor = true, bool bForceVisCursor = true );
    void                HideCursor();

    void                EnableCursor( bool bEnable );
    bool                IsCursorEnabled() const;

    const TextSelection&    GetSelection() const;
    TextSelection&      GetSelection();
    void                SetSelection( const TextSelection& rNewSel );
    void                SetSelection( const TextSelection& rNewSel, bool bGotoCursor );
    bool                HasSelection() const;

    OUString            GetSelected() const;
    OUString            GetSelected( LineEnd aSeparator ) const;
    void                DeleteSelected();

    void                InsertText( const OUString& rNew );

    bool                KeyInput( const KeyEvent& rKeyEvent );
    void                Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect);
    void                MouseButtonUp( const MouseEvent& rMouseEvent );
    void                MouseButtonDown( const MouseEvent& rMouseEvent );
    void                MouseMove( const MouseEvent& rMouseEvent );
    void                Command( const CommandEvent& rCEvt );

    void                Cut();
    void                Copy();
    void                Paste();

    void                Copy( css::uno::Reference< css::datatransfer::clipboard::XClipboard > const & rxClipboard );
    void                Paste( css::uno::Reference< css::datatransfer::clipboard::XClipboard > const & rxClipboard );

    void                Undo();
    void                Redo();

    void                Read( SvStream& rInput );

    void                SetStartDocPos( const Point& rPos );
    const Point&        GetStartDocPos() const;

    Point               GetDocPos( const Point& rWindowPos ) const;
    Point               GetWindowPos( const Point& rDocPos ) const;

    void                SetInsertMode( bool bInsert );
    bool                IsInsertMode() const;

    void                SetAutoIndentMode( bool bAutoIndent );

    void                SetReadOnly( bool bReadOnly );
    bool                IsReadOnly() const;

    void                SetAutoScroll( bool bAutoScroll );
    bool                IsAutoScroll() const;

    void                SetCursorAtPoint( const Point& rPointPixel );
    bool                IsSelectionAtPoint( const Point& rPointPixel );

    void                SetPaintSelection( bool bPaint);

    // Moved in here from the protected part.
    // For 'SvtXECTextCursor' (TL). Possibly needs to be solved again differently.
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
        Returns the number in paragraph of the line in which the cursor is blinking
        if enabled, -1 otherwise.
     */
    sal_Int32           GetLineNumberOfCursorInSelection() const;

    void                MatchGroup();

    // tdf#49482: Moves the start of the PaM to the center of the textview
    void                CenterPaM( const TextPaM& rPaM );

    bool                Search( const i18nutil::SearchOptions& rSearchOptions, bool bForward );
    sal_uInt16          Replace( const i18nutil::SearchOptions& rSearchOptions, bool bAll, bool bForward );

    bool                IndentBlock();
    bool                UnindentBlock();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
