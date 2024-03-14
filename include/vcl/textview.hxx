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
    struct SearchOptions2;
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
    explicit TETextDataObject( OUString aText );

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

    SAL_DLLPRIVATE bool ImpIndentBlock( bool bRight );
    SAL_DLLPRIVATE void ShowSelection();
    SAL_DLLPRIVATE void HideSelection();
    SAL_DLLPRIVATE void ShowSelection( const TextSelection& rSel );
    SAL_DLLPRIVATE void ImpShowHideSelection( const TextSelection* pRange = nullptr );

    SAL_DLLPRIVATE TextSelection const & ImpMoveCursor( const KeyEvent& rKeyEvent );
    SAL_DLLPRIVATE TextPaM ImpDelete( sal_uInt8 nMode, sal_uInt8 nDelMode );
    SAL_DLLPRIVATE bool IsInSelection( const TextPaM& rPaM ) const;

    SAL_DLLPRIVATE void ImpPaint(vcl::RenderContext& rRenderContext, const Point& rStartPos, tools::Rectangle const* pPaintArea, TextSelection const* pSelection);
    SAL_DLLPRIVATE void ImpPaint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect);
    SAL_DLLPRIVATE void  ImpShowCursor( bool bGotoCursor, bool bForceVisCursor, bool bEndKey );
    SAL_DLLPRIVATE void  ImpSetSelection( const TextSelection& rSelection );
    SAL_DLLPRIVATE Point ImpGetOutputStartPos( const Point& rStartDocPos ) const;

    SAL_DLLPRIVATE void ImpHideDDCursor();
    SAL_DLLPRIVATE void ImpShowDDCursor();

    SAL_DLLPRIVATE bool ImplTruncateNewText( OUString& rNewText ) const;
    SAL_DLLPRIVATE bool ImplCheckTextLen( std::u16string_view rNewText ) const;

    // DragAndDropClient
    SAL_DLLPRIVATE virtual void        dragGestureRecognized( const css::datatransfer::dnd::DragGestureEvent& dge ) override;
    SAL_DLLPRIVATE virtual void        dragDropEnd( const css::datatransfer::dnd::DragSourceDropEvent& dsde ) override;
    SAL_DLLPRIVATE virtual void        drop( const css::datatransfer::dnd::DropTargetDropEvent& dtde ) override;
    SAL_DLLPRIVATE virtual void        dragEnter( const css::datatransfer::dnd::DropTargetDragEnterEvent& dtdee ) override;
    SAL_DLLPRIVATE virtual void        dragExit( const css::datatransfer::dnd::DropTargetEvent& dte ) override;
    SAL_DLLPRIVATE virtual void        dragOver( const css::datatransfer::dnd::DropTargetDragEvent& dtde ) override;

            using       DragAndDropClient::dragEnter;
            using       DragAndDropClient::dragExit;
            using       DragAndDropClient::dragOver;

public:
                        TextView( ExtTextEngine* pEng, vcl::Window* pWindow );
    virtual            ~TextView() override;

    TextEngine*         GetTextEngine() const;
    vcl::Window*             GetWindow() const;

    SAL_DLLPRIVATE void Invalidate();
    void                Scroll( tools::Long nHorzScroll, tools::Long nVertScroll );

    void                ShowCursor( bool bGotoCursor = true, bool bForceVisCursor = true );
    void                HideCursor();

    SAL_DLLPRIVATE void EnableCursor( bool bEnable );
    SAL_DLLPRIVATE bool IsCursorEnabled() const;

    SAL_DLLPRIVATE const TextSelection& GetSelection() const;
    TextSelection&      GetSelection();
    void                SetSelection( const TextSelection& rNewSel );
    SAL_DLLPRIVATE void SetSelection( const TextSelection& rNewSel, bool bGotoCursor );
    bool                HasSelection() const;

    OUString            GetSelected() const;
    SAL_DLLPRIVATE OUString GetSelected( LineEnd aSeparator ) const;
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

    SAL_DLLPRIVATE void Copy( css::uno::Reference< css::datatransfer::clipboard::XClipboard > const & rxClipboard );
    SAL_DLLPRIVATE void Paste( css::uno::Reference< css::datatransfer::clipboard::XClipboard > const & rxClipboard );

    void                Undo();
    void                Redo();

    void                Read( SvStream& rInput );

    void                SetStartDocPos( const Point& rPos );
    const Point&        GetStartDocPos() const;

    Point               GetDocPos( const Point& rWindowPos ) const;
    Point               GetWindowPos( const Point& rDocPos ) const;

    SAL_DLLPRIVATE void SetInsertMode( bool bInsert );
    bool                IsInsertMode() const;

    void                SetAutoIndentMode( bool bAutoIndent );

    void                SetReadOnly( bool bReadOnly );
    bool                IsReadOnly() const;

    void                SetAutoScroll( bool bAutoScroll );
    SAL_DLLPRIVATE bool IsAutoScroll() const;

    SAL_DLLPRIVATE void SetCursorAtPoint( const Point& rPointPixel );
    SAL_DLLPRIVATE bool IsSelectionAtPoint( const Point& rPointPixel );

    SAL_DLLPRIVATE void SetPaintSelection( bool bPaint);

    // Moved in here from the protected part.
    // For 'SvtXECTextCursor' (TL). Possibly needs to be solved again differently.
    SAL_DLLPRIVATE TextPaM             PageUp( const TextPaM& rPaM );
    SAL_DLLPRIVATE TextPaM             PageDown( const TextPaM& rPaM );
    SAL_DLLPRIVATE TextPaM             CursorUp( const TextPaM& rPaM );
    SAL_DLLPRIVATE TextPaM             CursorDown( const TextPaM& rPaM );
    SAL_DLLPRIVATE TextPaM             CursorLeft( const TextPaM& rPaM, sal_uInt16 nCharacterIteratorMode );
    SAL_DLLPRIVATE TextPaM             CursorRight( const TextPaM& rPaM, sal_uInt16 nCharacterIteratorMode );
    SAL_DLLPRIVATE TextPaM             CursorFirstWord( const TextPaM& rPaM );
    SAL_DLLPRIVATE TextPaM             CursorWordLeft( const TextPaM& rPaM );
    SAL_DLLPRIVATE TextPaM             CursorWordRight( const TextPaM& rPaM );
    SAL_DLLPRIVATE TextPaM             CursorStartOfLine( const TextPaM& rPaM );
    TextPaM                            CursorEndOfLine( const TextPaM& rPaM );
    SAL_DLLPRIVATE static TextPaM      CursorStartOfParagraph( const TextPaM& rPaM );
    SAL_DLLPRIVATE TextPaM             CursorEndOfParagraph( const TextPaM& rPaM );
    SAL_DLLPRIVATE static TextPaM      CursorStartOfDoc();
    SAL_DLLPRIVATE TextPaM             CursorEndOfDoc();

    /**
        Returns the number in paragraph of the line in which the cursor is blinking
        if enabled, -1 otherwise.
     */
    sal_Int32           GetLineNumberOfCursorInSelection() const;

    void                MatchGroup();

    // tdf#49482: Moves the start of the PaM to the center of the textview
    SAL_DLLPRIVATE void CenterPaM( const TextPaM& rPaM );

    bool                Search( const i18nutil::SearchOptions2& rSearchOptions, bool bForward );
    sal_uInt16          Replace( const i18nutil::SearchOptions2& rSearchOptions, bool bAll, bool bForward );

    bool                IndentBlock();
    bool                UnindentBlock();

    // Used in the Basic IDE to toggle comment on a block of code
    void                ToggleComment();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
