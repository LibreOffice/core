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
#ifndef INCLUDED_EDITENG_EDITVIEW_HXX
#define INCLUDED_EDITENG_EDITVIEW_HXX

#include <com/sun/star/i18n/WordType.hpp>

#include <rsc/rscsfx.hxx>
#include <i18nlangtag/lang.h>
#include <tools/color.hxx>
#include <tools/gen.hxx>
#include <tools/link.hxx>
#include <vcl/cursor.hxx>
#include <editeng/editstat.hxx>
#include <svl/languageoptions.hxx>
#include <LibreOfficeKit/LibreOfficeKitTypes.h>

class EditEngine;
class ImpEditEngine;
class ImpEditView;
class OutlinerSearchable;
class OutlinerViewCallable;
class SvxSearchItem;
class SvxFieldItem;
namespace vcl { class Window; }
class Pointer;
class KeyEvent;
class MouseEvent;
class DropEvent;
class CommandEvent;
class Rectangle;
class Pair;
class Point;
class Range;
class SvStream;
class SvKeyValueIterator;
class SfxStyleSheet;
namespace vcl { class Font; }
class FontList;
class OutputDevice;

#include <editeng/editdata.hxx>
#include <com/sun/star/uno/Reference.h>
#include <editeng/editengdllapi.h>

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

enum class ScrollRangeCheck
{
    NONE          = 0,   // No correction of VisArea when scrolling
    NoNegative    = 1,   // No negative VisArea when scrolling
    PaperWidthTextSize = 2,   // VisArea must be within paper width, Text Size
};


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
    OUString        aDicNameSingle;

                    EditView( const EditView& ) = delete;
    EditView&       operator=( const EditView& ) = delete;

public:
                    EditView( EditEngine* pEng, vcl::Window* pWindow );
    virtual         ~EditView();

    void            SetEditEngine( EditEngine* pEditEngine );
    EditEngine*     GetEditEngine() const;

    void            SetWindow( vcl::Window* pWin );
    vcl::Window*         GetWindow() const;

    void            Paint( const Rectangle& rRect, OutputDevice* pTargetDevice = nullptr );
    void            Invalidate();
    Pair            Scroll( long nHorzScroll, long nVertScroll, ScrollRangeCheck nRangeCheck = ScrollRangeCheck::NoNegative );

    void            ShowCursor( bool bGotoCursor = true, bool bForceVisCursor = true, bool bActivate = false );
    void            HideCursor( bool bDeactivate = false );

    void            SetSelectionMode( EESelectionMode eMode );

    void            SetReadOnly( bool bReadOnly );
    bool            IsReadOnly() const;

    bool            HasSelection() const;
    ESelection      GetSelection() const;
    void            SetSelection( const ESelection& rNewSel );
    void            SelectCurrentWord( sal_Int16 nWordType = css::i18n::WordType::ANYWORD_IGNOREWHITESPACES );
    /// Returns the rectangles of the current selection in TWIPs.
    void GetSelectionRectangles(std::vector<Rectangle>& rLogicRects) const;

    bool            IsInsertMode() const;
    void            SetInsertMode( bool bInsert );

    OUString        GetSelected();
    void            DeleteSelected();

    SvtScriptType       GetSelectedScriptType() const;

                        // VisArea position of the Output window.
                        // A size change also affects the VisArea
    void                SetOutputArea( const Rectangle& rRect );
    const Rectangle&    GetOutputArea() const;

                        // Document position.
                        // A size change also affects the VisArea
    void                SetVisArea( const Rectangle& rRect );
    const Rectangle&    GetVisArea() const;

    const Pointer&  GetPointer() const;

    vcl::Cursor*    GetCursor() const;

    void            InsertText( const OUString& rNew, bool bSelect = false );

    bool            PostKeyEvent( const KeyEvent& rKeyEvent, vcl::Window* pFrameWin = nullptr );

    bool            MouseButtonUp( const MouseEvent& rMouseEvent );
    bool            MouseButtonDown( const MouseEvent& rMouseEvent );
    void            ReleaseMouse();
    bool            MouseMove( const MouseEvent& rMouseEvent );
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
    void                RemoveAttribs( bool bRemoveParaAttribs = false, sal_uInt16 nWhich = 0 );
    void                RemoveCharAttribs( sal_Int32 nPara, sal_uInt16 nWhich = 0 );
    void                RemoveAttribsKeepLanguages( bool bRemoveParaAttribs = false );

    sal_uInt32          Read( SvStream& rInput, const OUString& rBaseURL, EETextFormat eFormat, bool bSelect = false, SvKeyValueIterator* pHTTPHeaderAttrs = nullptr );

    void            SetBackgroundColor( const Color& rColor );
    Color           GetBackgroundColor() const;

    /// Register a LOK view callback.
    void registerLibreOfficeKitViewCallback(OutlinerViewCallable *pCallable);

    void            SetControlWord( EVControlBits nWord );
    EVControlBits   GetControlWord() const;

    EditTextObject* CreateTextObject();
    void            InsertText( const EditTextObject& rTextObject );
    void            InsertText( css::uno::Reference< css::datatransfer::XTransferable > xDataObj, const OUString& rBaseURL, bool bUseSpecial );

    css::uno::Reference< css::datatransfer::XTransferable > GetTransferable();

    // An EditView, so that when TRUE the update will be free from flickering:
    void            SetEditEngineUpdateMode( bool bUpdate );
    void            ForceUpdate();

    const SfxStyleSheet* GetStyleSheet() const;
    SfxStyleSheet* GetStyleSheet();

    void            SetAnchorMode( EVAnchorMode eMode );
    EVAnchorMode    GetAnchorMode() const;

    void            CompleteAutoCorrect( vcl::Window* pFrameWin = nullptr );

    EESpellState    StartSpeller( bool bMultipleDoc = false );
    EESpellState    StartThesaurus();
    sal_Int32       StartSearchAndReplace( const SvxSearchItem& rSearchItem );

    // for text conversion
    void            StartTextConversion( LanguageType nSrcLang, LanguageType nDestLang, const vcl::Font *pDestFont, sal_Int32 nOptions, bool bIsInteractive, bool bMultipleDoc );

    void            TransliterateText( sal_Int32 nTransliterationMode );

    bool            IsCursorAtWrongSpelledWord( bool bMarkIfWrong = false );
    bool            IsWrongSpelledWordAtPos( const Point& rPosPixel, bool bMarkIfWrong = false );
    void            ExecuteSpellPopup( const Point& rPosPixel, Link<SpellCallbackInfo&,void>* pCallBack = nullptr );

    void                InsertField( const SvxFieldItem& rFld );
    const SvxFieldItem* GetFieldUnderMousePointer() const;
    const SvxFieldItem* GetFieldUnderMousePointer( sal_Int32& nPara, sal_Int32& nPos ) const;
    const SvxFieldItem* GetField( const Point& rPos, sal_Int32* pnPara = nullptr, sal_Int32* pnPos = nullptr ) const;

    const SvxFieldItem* GetFieldAtSelection() const;

    void            SetInvalidateMore( sal_uInt16 nPixel );
    sal_uInt16      GetInvalidateMore() const;

    // grows or shrinks the font height for the current selection
    void            ChangeFontSize( bool bGrow, const FontList* pList );

    static bool     ChangeFontSize( bool bGrow, SfxItemSet& rSet, const FontList* pFontList );

    OUString        GetSurroundingText() const;
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
                            const css::uno::Reference< css::linguistic2::XSpellChecker1 >& xSpell,
                            const css::uno::Reference< css::linguistic2::XLanguageGuessing >& xLangGuess,
                            bool bIsParaText );
    /// Allows adjusting the point or mark of the selection to a document coordinate.
    void SetCursorLogicPosition(const Point& rPosition, bool bPoint, bool bClearMark);
};

#endif // INCLUDED_EDITENG_EDITVIEW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
