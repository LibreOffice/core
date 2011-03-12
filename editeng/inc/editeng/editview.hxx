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
// MyEDITVIEW, due to exported EditView
#ifndef _MyEDITVIEW_HXX
#define _MyEDITVIEW_HXX

#include <com/sun/star/i18n/WordType.hpp>

#include <rsc/rscsfx.hxx>
#include <i18npool/lang.h>
#include <tools/color.hxx>
#include <tools/gen.hxx>

class EditEngine;
class ImpEditEngine;
class ImpEditView;
class SvxSearchItem;
class SvxFieldItem;
class Window;
class Pointer;
class Cursor;
class KeyEvent;
class MouseEvent;
class DropEvent;
class CommandEvent;
class Rectangle;
class Link;
class Pair;
class Point;
class Range;
class SvStream;
class SvKeyValueIterator;
class SfxStyleSheet;
class Font;
class FontList;

#include <editeng/editdata.hxx>
#include <com/sun/star/uno/Reference.h>
#include "editeng/editengdllapi.h"

namespace com {
namespace sun {
namespace star {
namespace datatransfer {
    class XTransferable;
}}}}

class EDITENG_DLLPUBLIC EditView
{
    friend class EditEngine;
    friend class ImpEditEngine;
    friend class EditSelFunctionSet;

public: // Needed for Undo
    ImpEditView*    GetImpEditView() const      { return pImpEditView; }
    ImpEditEngine*  GetImpEditEngine() const;

private:
    ImpEditView*    pImpEditView;

                    EDITENG_DLLPRIVATE EditView( const EditView& );
    EDITENG_DLLPRIVATE EditView&        operator=( const EditView& );

public:
                    EditView( EditEngine* pEng, Window* pWindow );
    virtual         ~EditView();

    void            SetEditEngine( EditEngine* pEditEngine );
    EditEngine*     GetEditEngine() const;

    void            SetWindow( Window* pWin );
    Window*         GetWindow() const;

    void            Paint( const Rectangle& rRect );
    void            Invalidate();
    Pair            Scroll( long nHorzScroll, long nVertScroll, sal_uInt8 nRangeCheck = RGCHK_NEG );

    void            ShowCursor( sal_Bool bGotoCursor = sal_True, sal_Bool bForceVisCursor = sal_True );
    void            HideCursor();

    EESelectionMode GetSelectionMode() const;
    void            SetSelectionMode( EESelectionMode eMode );

    void            SetReadOnly( sal_Bool bReadOnly );
    sal_Bool            IsReadOnly() const;

    sal_Bool            HasSelection() const;
    ESelection      GetSelection() const;
    void            SetSelection( const ESelection& rNewSel );
    sal_Bool            SelectCurrentWord( sal_Int16 nWordType = ::com::sun::star::i18n::WordType::ANYWORD_IGNOREWHITESPACES );

    void            IndentBlock();
    void            UnindentBlock();

    sal_Bool            IsInsertMode() const;
    void            SetInsertMode( sal_Bool bInsert );

    void            ReplaceSelected( const String& rStr );
    String          GetSelected();
    void            DeleteSelected();

    sal_uInt16          GetSelectedScriptType() const;

                        // VisArea position of the Output window.
                        // A size change also affects the VisArea
    void                SetOutputArea( const Rectangle& rRec );
    const Rectangle&    GetOutputArea() const;

                        // Document position.
                        // A size change also affects the VisArea
    void                SetVisArea( const Rectangle& rRec );
    const Rectangle&    GetVisArea() const;

    void            SetPointer( const Pointer& rPointer );
    const Pointer&  GetPointer() const;

    void            SetCursor( const Cursor& rCursor );
    Cursor*         GetCursor() const;

    void            InsertText( const String& rNew, sal_Bool bSelect = sal_False );

    sal_Bool            PostKeyEvent( const KeyEvent& rKeyEvent, Window* pFrameWin = NULL );

    sal_Bool            MouseButtonUp( const MouseEvent& rMouseEvent );
    sal_Bool            MouseButtonDown( const MouseEvent& rMouseEvent );
    sal_Bool            MouseMove( const MouseEvent& rMouseEvent );
    void            Command( const CommandEvent& rCEvt );

    sal_Bool            Drop( const DropEvent& rEvt );
    sal_Bool            QueryDrop( DropEvent& rEvt );
    ESelection      GetDropPos();

    void            Cut();
    void            Copy();
    void            Paste();
    void            PasteSpecial();

    void            EnablePaste( sal_Bool bEnable );
    sal_Bool            IsPasteEnabled() const;

    void            Undo();
    void            Redo();

    // especially for Oliver Specht
    sal_uInt16          GetParagraph( const Point& rMousePosPixel );
    Point           GetWindowPosTopLeft( sal_uInt16 nParagraph );
    void            MoveParagraphs( Range aParagraphs, sal_uInt16 nNewPos );
    void            MoveParagraphs( long nDiff );

    const SfxItemSet&   GetEmptyItemSet();
    SfxItemSet          GetAttribs();
    void                SetAttribs( const SfxItemSet& rSet );
    void                SetParaAttribs( const SfxItemSet& rSet, sal_uInt16 nPara );
    void                RemoveAttribs( sal_Bool bRemoveParaAttribs = sal_False, sal_uInt16 nWhich = 0 );
    void                RemoveCharAttribs( sal_uInt16 nPara, sal_uInt16 nWhich = 0 );
    void                RemoveAttribsKeepLanguages( sal_Bool bRemoveParaAttribs = sal_False );

    sal_uLong           Read( SvStream& rInput, const String& rBaseURL, EETextFormat eFormat, sal_Bool bSelect = sal_False, SvKeyValueIterator* pHTTPHeaderAttrs = NULL );
    sal_uLong           Write( SvStream& rOutput, EETextFormat eFormat );

    void            SetBackgroundColor( const Color& rColor );
    Color           GetBackgroundColor() const;

    void            SetControlWord( sal_uInt32 nWord );
    sal_uInt32      GetControlWord() const;

    EditTextObject* CreateTextObject();
    void            InsertText( const EditTextObject& rTextObject );
    void            InsertText( ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable > xDataObj, const String& rBaseURL, sal_Bool bUseSpecial );

    ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable > GetTransferable();

    // An EditView, so that when TRUE the update will be free from flickering:
    void            SetEditEngineUpdateMode( sal_Bool bUpdate );
    void            ForceUpdate();

    SfxStyleSheet*  GetStyleSheet() const;
    void            SetStyleSheet( SfxStyleSheet* pStyle );

    void            SetAnchorMode( EVAnchorMode eMode );
    EVAnchorMode    GetAnchorMode() const;

    sal_Bool            MatchGroup();

    void            CompleteAutoCorrect( Window* pFrameWin = NULL );

    EESpellState    StartSpeller( sal_Bool bMultipleDoc = sal_False );
    EESpellState    StartThesaurus();
    sal_uInt16          StartSearchAndReplace( const SvxSearchItem& rSearchItem );

    // for text conversion
    void            StartTextConversion( LanguageType nSrcLang, LanguageType nDestLang, const Font *pDestFont, sal_Int32 nOptions, sal_Bool bIsInteractive, sal_Bool bMultipleDoc );
    sal_Bool        HasConvertibleTextPortion( LanguageType nLang );

    void            TransliterateText( sal_Int32 nTransliterationMode );

    sal_Bool            IsCursorAtWrongSpelledWord( sal_Bool bMarkIfWrong = sal_False );
    sal_Bool            IsWrongSpelledWordAtPos( const Point& rPosPixel, sal_Bool bMarkIfWrong = sal_False );
    void            SpellIgnoreWord();
    void            ExecuteSpellPopup( const Point& rPosPixel, Link* pCallBack = 0 );

    void                InsertField( const SvxFieldItem& rFld );
    const SvxFieldItem* GetFieldUnderMousePointer() const;
    const SvxFieldItem* GetFieldUnderMousePointer( sal_uInt16& nPara, xub_StrLen& nPos ) const;
    const SvxFieldItem* GetField( const Point& rPos, sal_uInt16* pnPara = NULL, xub_StrLen* pnPos = NULL ) const;

    const SvxFieldItem* GetFieldAtSelection() const;

    String          GetWordUnderMousePointer() const;
    String          GetWordUnderMousePointer( Rectangle& rWordRect ) const;

    void            SetInvalidateMore( sal_uInt16 nPixel );
    sal_uInt16          GetInvalidateMore() const;

    // grows or shrinks the font height for the current selection
    void            ChangeFontSize( bool bGrow, const FontList* pList );

    static bool ChangeFontSize( bool bGrow, SfxItemSet& rSet, const FontList* pFontList );

    String          GetSurroundingText() const;
    Selection       GetSurroundingTextSelection() const;
};

#endif // _MyEDITVIEW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
