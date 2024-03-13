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

#include <memory>
#include <com/sun/star/i18n/WordType.hpp>

#include <i18nlangtag/lang.h>
#include <sot/formats.hxx>
#include <tools/color.hxx>
#include <tools/gen.hxx>
#include <comphelper/errcode.hxx>
#include <vcl/vclptr.hxx>
#include <editeng/editstat.hxx>
#include <editeng/flditem.hxx>
#include <svl/languageoptions.hxx>
#include <editeng/editdata.hxx>
#include <com/sun/star/uno/Reference.h>
#include <editeng/editengdllapi.h>


class EditTextObject;
class EditEngine;
class ImpEditEngine;
class ImpEditView;
class OutlinerViewShell;
class SvxSearchItem;
class SvxFieldItem;
namespace vcl { class Window; }
namespace weld { class Widget; }
class KeyEvent;
class MouseEvent;
class CommandEvent;
class SvStream;
class SvKeyValueIterator;
class SfxStyleSheet;
class SfxItemSet;
namespace vcl { class Cursor; }
namespace vcl { class Font; }
class FontList;
class InputContext;
class OutputDevice;
enum class TransliterationFlags;
enum class PointerStyle;

namespace com {
namespace sun {
namespace star {
namespace datatransfer {
    class XTransferable;
    namespace clipboard {
        class XClipboard;
    }
    namespace dnd {
        class XDropTarget;
    }
}
namespace linguistic2 {
    class XSpellChecker1;
    class XLanguageGuessing;
}
}}}

template <typename Arg, typename Ret> class Link;

enum class ScrollRangeCheck
{
    NoNegative    = 1,   // No negative VisArea when scrolling
    PaperWidthTextSize = 2,   // VisArea must be within paper width, Text Size
};

enum class LOKSpecialFlags {
    NONE               = 0x0000,
    LayoutRTL          = 0x0001,
};

namespace o3tl
{
    template<> struct typed_flags<LOKSpecialFlags> : is_typed_flags<LOKSpecialFlags, 0x77> {};
}

// Helper class that allows to set a callback at the EditView. When
// set, Invalidates and repaints are suppressed at the EditView, but
// EditViewInvalidate() will be triggered to allow the consumer to
// react itself as needed.
// Also Selection visualization is suppressed and EditViewSelectionChange
// is triggered when Selection changes and needs reaction.
class EDITENG_DLLPUBLIC EditViewCallbacks
{
public:
    virtual ~EditViewCallbacks();

    // call this when text visualization changed in any way. It
    // will also update selection, so no need to call this self
    // additionally (but will also do no harm)
    virtual void EditViewInvalidate(const tools::Rectangle& rRect) = 0;

    // call this when only selection is changed. Text change will
    // then *not* be checked and not be reacted on. Still, when
    // only the selection is changed, this is useful and faster
    virtual void EditViewSelectionChange() = 0;

    // return the OutputDevice that the EditView will draw to
    virtual OutputDevice& EditViewOutputDevice() const = 0;

    virtual weld::Widget* EditViewPopupParent() const
    {
        return nullptr;
    }

    // return the Mouse Position
    virtual Point EditViewPointerPosPixel() const = 0;

    // Triggered to update InputEngine context information
    virtual void EditViewInputContext(const InputContext& rInputContext) = 0;

    // Triggered to update InputEngine cursor position
    virtual void EditViewCursorRect(const tools::Rectangle& rRect, int nExtTextInputWidth) = 0;

    // Triggered if scroll bar state should change
    virtual void EditViewScrollStateChange()
    {
    }

    // Access to clipboard
    virtual css::uno::Reference<css::datatransfer::clipboard::XClipboard> GetClipboard() const = 0;

    // implemented if drag and drop support is wanted
    virtual css::uno::Reference<css::datatransfer::dnd::XDropTarget> GetDropTarget()
    {
        return nullptr;
    }
};

class EDITENG_DLLPUBLIC EditView final
{
    friend class EditEngine;
    friend class ImpEditEngine;
    friend class EditSelFunctionSet;

public:
    typedef std::vector<VclPtr<vcl::Window>> OutWindowSet;

    ImpEditView& getImpl() const { return *mpImpEditView; }
    SAL_DLLPRIVATE ImpEditEngine& getImpEditEngine() const;

    void setEditEngine(EditEngine* pEditEngine);
    EditEngine& getEditEngine() const;

private:
    std::unique_ptr<ImpEditView> mpImpEditView;
    OUString        aDicNameSingle;

                    EditView( const EditView& ) = delete;
    EditView&       operator=( const EditView& ) = delete;

    // counts how many characters take unfolded fields
    // bCanOverflow - count field length without trim to the selected pos
    SAL_DLLPRIVATE sal_Int32       countFieldsOffsetSum(sal_Int32 nPara, sal_Int32 nPo, bool bCanOverflow) const;

public:
                    EditView( EditEngine* pEng, vcl::Window* pWindow );
                    ~EditView();

    // set EditViewCallbacks for external handling of Repaints/Visualization
    void setEditViewCallbacks(EditViewCallbacks* pEditViewCallbacks);
    EditViewCallbacks* getEditViewCallbacks() const;

    void            SetWindow( vcl::Window* pWin );
    vcl::Window*    GetWindow() const;
    OutputDevice&   GetOutputDevice() const;

    LanguageType    GetInputLanguage() const;

    SAL_DLLPRIVATE bool            HasOtherViewWindow( vcl::Window* pWin );
    bool            AddOtherViewWindow( vcl::Window* pWin );
    bool            RemoveOtherViewWindow( vcl::Window* pWin );

    void            Paint( const tools::Rectangle& rRect, OutputDevice* pTargetDevice = nullptr );
    tools::Rectangle       GetInvalidateRect() const;
    SAL_DLLPRIVATE void            InvalidateWindow(const tools::Rectangle& rClipRect);
    void            InvalidateOtherViewWindows( const tools::Rectangle& rInvRect );
    void            Invalidate();
    ::Pair            Scroll( tools::Long nHorzScroll, tools::Long nVertScroll, ScrollRangeCheck nRangeCheck = ScrollRangeCheck::NoNegative );

    void            SetBroadcastLOKViewCursor(bool bSet);
    tools::Rectangle       GetEditCursor() const;
    void            ShowCursor( bool bGotoCursor = true, bool bForceVisCursor = true, bool bActivate = false );
    void            HideCursor( bool bDeactivate = false );

    void            SetSelectionMode( EESelectionMode eMode );

    void            SetReadOnly( bool bReadOnly );
    bool            IsReadOnly() const;

    bool            HasSelection() const;
    ESelection      GetSelection() const;
    void            SetSelection( const ESelection& rNewSel );
    bool            IsSelectionAtPoint(const Point& rPointPixel);
    void            SelectCurrentWord( sal_Int16 nWordType = css::i18n::WordType::ANYWORD_IGNOREWHITESPACES );
    /// Returns the rectangles of the current selection in TWIPs.
    void GetSelectionRectangles(std::vector<tools::Rectangle>& rLogicRects) const;
    bool IsSelectionFullPara() const;
    bool IsSelectionWithinSinglePara() const;

    bool            IsInsertMode() const;
    void            SetInsertMode( bool bInsert );

    OUString        GetSelected() const;
    void            DeleteSelected();

    SvtScriptType       GetSelectedScriptType() const;

                        // VisArea position of the Output window.
                        // A size change also affects the VisArea
    void                SetOutputArea( const tools::Rectangle& rRect );
    const tools::Rectangle&    GetOutputArea() const;

                        // Document position.
                        // A size change also affects the VisArea
    void                SetVisArea( const tools::Rectangle& rRect );
    tools::Rectangle    GetVisArea() const;

    PointerStyle    GetPointer() const;

    vcl::Cursor*    GetCursor() const;

    void            InsertText( const OUString& rNew, bool bSelect = false , bool bLOKShowSelect = true);
    void            InsertParaBreak();

    bool            PostKeyEvent( const KeyEvent& rKeyEvent, vcl::Window const * pFrameWin = nullptr );

    bool            MouseButtonUp( const MouseEvent& rMouseEvent );
    bool            MouseButtonDown( const MouseEvent& rMouseEvent );
    SAL_DLLPRIVATE void            ReleaseMouse();
    bool            MouseMove( const MouseEvent& rMouseEvent );
    bool            Command(const CommandEvent& rCEvt);

    void            Cut();
    void            Copy();
    void            Paste();
    void            PasteSpecial(SotClipboardFormatId format = SotClipboardFormatId::NONE);

    void            Undo();
    void            Redo();

    // especially for Oliver Specht
    Point           GetWindowPosTopLeft( sal_Int32 nParagraph );
    SAL_DLLPRIVATE void            MoveParagraphs( Range aParagraphs, sal_Int32 nNewPos );
    SAL_DLLPRIVATE void            MoveParagraphs( tools::Long nDiff );

    const SfxItemSet& GetEmptyItemSet() const;
    SfxItemSet          GetAttribs();
    void                SetAttribs( const SfxItemSet& rSet );
    void                RemoveAttribs( bool bRemoveParaAttribs = false, sal_uInt16 nWhich = 0 );
    SAL_DLLPRIVATE void                RemoveAttribs( EERemoveParaAttribsMode eMode, sal_uInt16 nWhich );
    void                RemoveCharAttribs( sal_Int32 nPara, sal_uInt16 nWhich );
    void                RemoveAttribsKeepLanguages( bool bRemoveParaAttribs );

    SAL_DLLPRIVATE ErrCode             Read( SvStream& rInput, EETextFormat eFormat, SvKeyValueIterator* pHTTPHeaderAttrs );

    void            SetBackgroundColor( const Color& rColor );
    Color const &   GetBackgroundColor() const;

    /// Informs this edit view about which view shell contains it.
    void RegisterViewShell(OutlinerViewShell* pViewShell);
    /// Informs this edit view about which other shell listens to it.
    void RegisterOtherShell(OutlinerViewShell* pOtherShell);

    void            SetControlWord( EVControlBits nWord );
    EVControlBits   GetControlWord() const;

    std::unique_ptr<EditTextObject> CreateTextObject();
    void            InsertText( const EditTextObject& rTextObject );
    void            InsertText( css::uno::Reference< css::datatransfer::XTransferable > const & xDataObj, const OUString& rBaseURL, bool bUseSpecial );

    css::uno::Reference<css::datatransfer::clipboard::XClipboard> GetClipboard() const;
    css::uno::Reference<css::datatransfer::XTransferable> GetTransferable() const;

    // An EditView, so that when TRUE the update will be free from flickering:
    // @return the previous bUpdateLayout state
    bool            SetEditEngineUpdateLayout( bool bUpdate );
    void            ForceLayoutCalculation();

    SAL_DLLPRIVATE const SfxStyleSheet* GetStyleSheet() const;
    SAL_DLLPRIVATE SfxStyleSheet* GetStyleSheet();

    SAL_DLLPRIVATE void            SetAnchorMode( EEAnchorMode eMode );
    SAL_DLLPRIVATE EEAnchorMode    GetAnchorMode() const;

    void            CompleteAutoCorrect( vcl::Window const * pFrameWin = nullptr );

    EESpellState    StartSpeller(weld::Widget* pDialogParent, bool bMultipleDoc = false);
    EESpellState    StartThesaurus(weld::Widget* pDialogParent);
    SAL_DLLPRIVATE sal_Int32       StartSearchAndReplace( const SvxSearchItem& rSearchItem );

    // for text conversion
    void            StartTextConversion(weld::Widget* pDialogParent, LanguageType nSrcLang, LanguageType nDestLang, const vcl::Font *pDestFont, sal_Int32 nOptions, bool bIsInteractive, bool bMultipleDoc);

    void            TransliterateText( TransliterationFlags nTransliterationMode );

    SAL_DLLPRIVATE bool            IsCursorAtWrongSpelledWord();
    bool            IsWrongSpelledWordAtPos( const Point& rPosPixel, bool bMarkIfWrong = false );
    bool            ExecuteSpellPopup(const Point& rPosPixel, const Link<SpellCallbackInfo&,void>& rCallBack);
    OUString        SpellIgnoreWord();

    void                InsertField( const SvxFieldItem& rFld );
    const SvxFieldItem* GetFieldUnderMousePointer() const;
    SAL_DLLPRIVATE const SvxFieldItem* GetFieldUnderMousePointer( sal_Int32& nPara, sal_Int32& nPos ) const;
    const SvxFieldItem* GetField( const Point& rPos, sal_Int32* pnPara = nullptr, sal_Int32* pnPos = nullptr ) const;

    /// return the selected field or the field immediately after (or before) the current cursor
    const SvxFieldItem* GetFieldAtSelection(bool bAlsoCheckBeforeCursor = false) const;
    SAL_DLLPRIVATE const SvxFieldItem* GetFieldAtSelection(bool* pIsBeforeCursor) const;

    /// return field under mouse, at selection, or immediately after (or before) the current cursor
    const SvxFieldData* GetFieldUnderMouseOrInSelectionOrAtCursor(bool bAlsoCheckBeforeCursor = false) const;
    /// if no selection, select the field immediately after or before the current cursor
    void SelectFieldAtCursor();
    /// Converts position in paragraph to logical position without unfolding fields
    sal_Int32       GetPosNoField(sal_Int32 nPara, sal_Int32 nPos) const;
    /// Converts logical position in paragraph to position with unfolded fields
    sal_Int32       GetPosWithField(sal_Int32 nPara, sal_Int32 nPos) const;

    SAL_DLLPRIVATE void            SetInvalidateMore( sal_uInt16 nPixel );
    SAL_DLLPRIVATE sal_uInt16      GetInvalidateMore() const;

    // grows or shrinks the font height for the current selection
    void            ChangeFontSize( bool bGrow, const FontList* pList );

    static bool     ChangeFontSize( bool bGrow, SfxItemSet& rSet, const FontList* pFontList );

    OUString        GetSurroundingText() const;
    Selection       GetSurroundingTextSelection() const;
    bool            DeleteSurroundingText(const Selection& rRange);

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
    /// Trigger selection drawing callback in pOtherShell based on our shell's selection state.
    void DrawSelectionXOR(OutlinerViewShell* pOtherShell);

    /**
     * This is meant for Calc(LOK), but there may be other use-cases.
     * In Calc, all logical positions are computed by
     * doing independent pixel-alignment for each cell's size. The *LOKSpecial* methods
     * can be used to set/get the output-area and visible-area in real logical
     * units. This enables EditView to send cursor/selection messages in
     * real logical units like it is done for Writer.
     */
    void InitLOKSpecialPositioning(MapUnit eUnit, const tools::Rectangle& rOutputArea,
                                   const Point& rVisDocStartPos);
    void SetLOKSpecialOutputArea(const tools::Rectangle& rOutputArea);
    const tools::Rectangle & GetLOKSpecialOutputArea() const;
    void SetLOKSpecialVisArea(const tools::Rectangle& rVisArea);
    tools::Rectangle GetLOKSpecialVisArea() const;
    bool HasLOKSpecialPositioning() const;

    void SetLOKSpecialFlags(LOKSpecialFlags eFlags);

    void SuppressLOKMessages(bool bSet);
    bool IsSuppressLOKMessages() const;

    /// To inform editeng that negated x document coordinates are in use.
    void SetNegativeX(bool bSet);
    bool IsNegativeX() const;
};

#endif // INCLUDED_EDITENG_EDITVIEW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
