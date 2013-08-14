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
// MyEDITVIEW, due to exported EditView
#ifndef _MyEDITVIEW_HXX
#define _MyEDITVIEW_HXX

#include <com/sun/star/i18n/WordType.hpp>

#include <rsc/rscsfx.hxx>
#include <i18nlangtag/lang.h>
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
class OutputDevice;

#include <editeng/editdata.hxx>
#include <com/sun/star/uno/Reference.h>
#include "editeng/editengdllapi.h"

namespace com {
namespace sun {
namespace star {
namespace datatransfer {
    class XTransferable;
}
namespace linguistic2 {
    class XSpellChecker1;
    class XLanguageGuessing;
}
}}}

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
    String          aDicNameSingle;

                    EDITENG_DLLPRIVATE EditView( const EditView& );
    EDITENG_DLLPRIVATE EditView&        operator=( const EditView& );

public:
                    EditView( EditEngine* pEng, Window* pWindow );
    virtual         ~EditView();

    void            SetEditEngine( EditEngine* pEditEngine );
    EditEngine*     GetEditEngine() const;

    void            SetWindow( Window* pWin );
    Window*         GetWindow() const;

    void            Paint( const Rectangle& rRect, OutputDevice* pTargetDevice = 0 );
    void            Invalidate();
    Pair            Scroll( long nHorzScroll, long nVertScroll, sal_uInt8 nRangeCheck = RGCHK_NEG );

    void            ShowCursor( sal_Bool bGotoCursor = sal_True, sal_Bool bForceVisCursor = sal_True );
    void            HideCursor();

    void            SetSelectionMode( EESelectionMode eMode );

    void            SetReadOnly( sal_Bool bReadOnly );
    sal_Bool            IsReadOnly() const;

    sal_Bool            HasSelection() const;
    ESelection      GetSelection() const;
    void            SetSelection( const ESelection& rNewSel );
    sal_Bool            SelectCurrentWord( sal_Int16 nWordType = ::com::sun::star::i18n::WordType::ANYWORD_IGNOREWHITESPACES );

    sal_Bool            IsInsertMode() const;
    void            SetInsertMode( sal_Bool bInsert );

    void            ReplaceSelected( const String& rStr );
    OUString        GetSelected();
    void            DeleteSelected();

    sal_uInt16          GetSelectedScriptType() const;

                        // VisArea position of the Output window.
                        // A size change also affects the VisArea
    void                SetOutputArea( const Rectangle& rRect );
    const Rectangle&    GetOutputArea() const;

                        // Document position.
                        // A size change also affects the VisArea
    void                SetVisArea( const Rectangle& rRect );
    const Rectangle&    GetVisArea() const;

    const Pointer&  GetPointer() const;

    Cursor*         GetCursor() const;

    void            InsertText( const OUString& rNew, sal_Bool bSelect = sal_False );

    sal_Bool            PostKeyEvent( const KeyEvent& rKeyEvent, Window* pFrameWin = NULL );

    sal_Bool            MouseButtonUp( const MouseEvent& rMouseEvent );
    sal_Bool            MouseButtonDown( const MouseEvent& rMouseEvent );
    sal_Bool            MouseMove( const MouseEvent& rMouseEvent );
    void            Command( const CommandEvent& rCEvt );

    void            Cut();
    void            Copy();
    void            Paste();
    void            PasteSpecial();

    void            Undo();
    void            Redo();

    // especially for Oliver Specht
    Point           GetWindowPosTopLeft( sal_Int32 nParagraph );
    void            MoveParagraphs( Range aParagraphs, sal_Int32 nNewPos );
    void            MoveParagraphs( long nDiff );

    const SfxItemSet&   GetEmptyItemSet();
    SfxItemSet          GetAttribs();
    void                SetAttribs( const SfxItemSet& rSet );
    void                RemoveAttribs( sal_Bool bRemoveParaAttribs = sal_False, sal_uInt16 nWhich = 0 );
    void                RemoveCharAttribs( sal_Int32 nPara, sal_uInt16 nWhich = 0 );
    void                RemoveAttribsKeepLanguages( sal_Bool bRemoveParaAttribs = sal_False );

    sal_uLong           Read( SvStream& rInput, const String& rBaseURL, EETextFormat eFormat, sal_Bool bSelect = sal_False, SvKeyValueIterator* pHTTPHeaderAttrs = NULL );

    void            SetBackgroundColor( const Color& rColor );
    Color           GetBackgroundColor() const;

    void            SetControlWord( sal_uInt32 nWord );
    sal_uInt32      GetControlWord() const;

    EditTextObject* CreateTextObject();
    void            InsertText( const EditTextObject& rTextObject );
    void            InsertText( ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable > xDataObj, const OUString& rBaseURL, sal_Bool bUseSpecial );

    ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable > GetTransferable();

    // An EditView, so that when TRUE the update will be free from flickering:
    void            SetEditEngineUpdateMode( sal_Bool bUpdate );
    void            ForceUpdate();

    const SfxStyleSheet* GetStyleSheet() const;
    SfxStyleSheet* GetStyleSheet();

    void            SetAnchorMode( EVAnchorMode eMode );
    EVAnchorMode    GetAnchorMode() const;

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
    void            ExecuteSpellPopup( const Point& rPosPixel, Link* pCallBack = 0 );

    void                InsertField( const SvxFieldItem& rFld );
    const SvxFieldItem* GetFieldUnderMousePointer() const;
    const SvxFieldItem* GetFieldUnderMousePointer( sal_Int32& nPara, xub_StrLen& nPos ) const;
    const SvxFieldItem* GetField( const Point& rPos, sal_Int32* pnPara = NULL, xub_StrLen* pnPos = NULL ) const;

    const SvxFieldItem* GetFieldAtSelection() const;

    void            SetInvalidateMore( sal_uInt16 nPixel );
    sal_uInt16          GetInvalidateMore() const;

    // grows or shrinks the font height for the current selection
    void            ChangeFontSize( bool bGrow, const FontList* pList );

    static bool ChangeFontSize( bool bGrow, SfxItemSet& rSet, const FontList* pFontList );

    String          GetSurroundingText() const;
    Selection       GetSurroundingTextSelection() const;

    /** Tries to determine the language of 'rText', returning a matching known
        locale if possible, or a fallback, or LANGUAGE_NONE if nothing found or
        matched.

        @param bIsParaText
                If TRUE, rText is a paragraph and the language is obtained by
                passing the text to xLangGuess.
                IF FALSE, a language match is tried for, in order,
                    1. the default document language (non-CTL, non-CJK, aka LATIN)
                    2. the UI language (Tools->Options->LanguageSettings->Languages User Interface)
                    3. the locale (Tools->Options->LanguageSettings->Languages Locale)
                    4. en-US
                    If nothing matched, LANGUAGE_NONE is returned.
    */
    static LanguageType CheckLanguage(
                            const OUString &rText,
                            com::sun::star::uno::Reference< com::sun::star::linguistic2::XSpellChecker1 > xSpell,
                            com::sun::star::uno::Reference< com::sun::star::linguistic2::XLanguageGuessing > xLangGuess,
                            bool bIsParaText );
};

#endif // _MyEDITVIEW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
