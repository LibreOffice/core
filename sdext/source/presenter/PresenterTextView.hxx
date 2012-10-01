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

#ifndef SDEXT_PRESENTER_TEXT_VIEW_HXX
#define SDEXT_PRESENTER_TEXT_VIEW_HXX

#include "PresenterTheme.hxx"
#include <com/sun/star/accessibility/XAccessibleText.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/geometry/RealPoint2D.hpp>
#include <com/sun/star/geometry/RealSize2D.hpp>
#include <com/sun/star/i18n/XBreakIterator.hpp>
#include <com/sun/star/i18n/XScriptTypeDetector.hpp>
#include <com/sun/star/rendering/XCanvas.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/compbase1.hxx>
#include <cppuhelper/basemutex.hxx>

namespace css = ::com::sun::star;
namespace cssu = ::com::sun::star::uno;
namespace cssa = ::com::sun::star::accessibility;

namespace sdext { namespace presenter {

class PresenterTextCaret
{
public:
    PresenterTextCaret (
        const ::boost::function<css::awt::Rectangle(const sal_Int32,const sal_Int32)>&
            rCharacterBoundsAccess,
        const ::boost::function<void(const css::awt::Rectangle&)>&
            rInvalidator);
    ~PresenterTextCaret (void);

    void ShowCaret (void);
    void HideCaret (void);

    sal_Int32 GetParagraphIndex (void) const;
    sal_Int32 GetCharacterIndex (void) const;
    void SetPosition (
        const sal_Int32 nParagraphIndex,
        const sal_Int32 nCharacterIndex);

    bool IsVisible (void) const;

    /** Set a (possibly empty) functor that broadcasts changes of the caret
        position.  This is used when a PresenterTextView object is set at
        the accessibility object so that accessibility events can be sent
        when the caret changes position.
    */
    void SetCaretMotionBroadcaster (
        const ::boost::function<void(sal_Int32,sal_Int32,sal_Int32,sal_Int32)>& rBroadcaster);

    css::awt::Rectangle GetBounds (void) const;

private:
    sal_Int32 mnParagraphIndex;
    sal_Int32 mnCharacterIndex;
    sal_Int32 mnCaretBlinkTaskId;
    bool mbIsCaretVisible;
    const ::boost::function<css::awt::Rectangle(const sal_Int32,const sal_Int32)> maCharacterBoundsAccess;
    const ::boost::function<void(const css::awt::Rectangle&)> maInvalidator;
    ::boost::function<void(sal_Int32,sal_Int32,sal_Int32,sal_Int32)> maBroadcaster;
    css::awt::Rectangle maCaretBounds;

    void InvertCaret (void);
};
typedef ::boost::shared_ptr<PresenterTextCaret> SharedPresenterTextCaret;

//===== PresenterTextParagraph ================================================

class PresenterTextParagraph
{
public:
    PresenterTextParagraph (
        const sal_Int32 nParagraphIndex,
        const cssu::Reference<css::i18n::XBreakIterator>& rxBreakIterator,
        const cssu::Reference<css::i18n::XScriptTypeDetector>& rxScriptTypeDetector,
        const cssu::Reference<css::text::XTextRange>& rxTextRange,
        const SharedPresenterTextCaret& rpCaret);

    void Paint (
        const cssu::Reference<css::rendering::XCanvas>& rxCanvas,
        const css::geometry::RealSize2D& rSize,
        const PresenterTheme::SharedFontDescriptor& rpFont,
        const css::rendering::ViewState& rViewState,
        css::rendering::RenderState& rRenderState,
        const double nTopOffset,
        const double nClipTop,
        const double nClipBottom);

    double GetTotalTextHeight (void);

    void SetCharacterOffset (const sal_Int32 nCharacterOffset);
    sal_Int32 GetCharacterCount (void) const;
    sal_Unicode GetCharacter (const sal_Int32 nGlobalCharacterIndex) const;
    ::rtl::OUString GetText (void) const;
    cssa::TextSegment GetTextSegment (
        const sal_Int32 nOffset,
        const sal_Int32 nGlobalCharacterIndex,
        const sal_Int16 nTextType) const;
    cssa::TextSegment GetWordTextSegment (
        const sal_Int32 nOffset,
        const sal_Int32 nIndex) const;
    cssa::TextSegment CreateTextSegment (
        sal_Int32 nStartIndex,
        sal_Int32 nEndIndex) const;
    css::awt::Rectangle GetCharacterBounds (
        sal_Int32 nGlobalCharacterIndex,
        const bool bCaretBox);
    sal_Int32 GetIndexAtPoint (const css::awt::Point& rPoint) const;
    void SetupCellArray (
        const PresenterTheme::SharedFontDescriptor& rpFont);
    void Format (
        const double nY,
        const double nWidth,
        const PresenterTheme::SharedFontDescriptor& rpFont);
    sal_Int32 GetWordBoundary(
        const sal_Int32 nLocalCharacterIndex,
        const sal_Int32 nDistance);
    sal_Int32 GetCaretPosition (void) const;
    void SetCaretPosition (const sal_Int32 nPosition) const;
    void SetOrigin (const double nXOrigin, const double nYOrigin);
    css::awt::Point GetRelativeLocation (void) const;
    css::awt::Size GetSize (void);

private:
    ::rtl::OUString msParagraphText;
    const sal_Int32 mnParagraphIndex;
    SharedPresenterTextCaret mpCaret;

    /** A portion of a string that encodes one unicode cell.  It describes
        number of characters in the unicode string that make up the cell and its
        width in pixel (with respect to some configuration that is stored
        externally or implicitly).
    */
    class Cell
    {
    public:
        Cell (const sal_Int32 nCharacterIndex, const sal_Int32 nCharacterCount, const double nCellWidth);
        sal_Int32 mnCharacterIndex;
        sal_Int32 mnCharacterCount;
        double mnCellWidth;
    };

    class Line
    {
    public:
        Line (const sal_Int32 nLineStartCharacterIndex, const sal_Int32 nLineEndCharacterIndex);
        sal_Int32 mnLineStartCharacterIndex;
        sal_Int32 mnLineEndCharacterIndex;
        sal_Int32 mnLineStartCellIndex;
        sal_Int32 mnLineEndCellIndex;
        cssu::Reference<css::rendering::XTextLayout> mxLayoutedLine;
        double mnBaseLine;
        double mnWidth;
        cssu::Sequence<css::geometry::RealRectangle2D> maCellBoxes;

        void ProvideLayoutedLine (
            const ::rtl::OUString& rsParagraphText,
            const PresenterTheme::SharedFontDescriptor& rpFont,
            const sal_Int8 nTextDirection);
        void ProvideCellBoxes (void);
        bool IsEmpty (void) const;
    };

    cssu::Reference<css::i18n::XBreakIterator> mxBreakIterator;
    cssu::Reference<css::i18n::XScriptTypeDetector> mxScriptTypeDetector;
    ::std::vector<Line> maLines;
    ::std::vector<sal_Int32> maWordBoundaries;
    // Offset of the top of the paragraph with respect to the origin of the
    // whole text (specified by mnXOrigin and mnYOrigin).
    double mnVerticalOffset;
    double mnXOrigin;
    double mnYOrigin;
    double mnWidth;
    double mnAscent;
    double mnDescent;
    double mnLineHeight;
    css::style::ParagraphAdjust meAdjust;
    sal_Int8 mnWritingMode;
    /// The index of the first character in this paragraph with respect to
    /// the whole text.
    sal_Int32 mnCharacterOffset;
    ::std::vector<Cell> maCells;

    void AddWord (
        const double nWidth,
        css::i18n::Boundary& rCurrentLine,
        const sal_Int32 nWordBoundary,
        const PresenterTheme::SharedFontDescriptor& rpFont);
    void AddLine (
        css::i18n::Boundary& rCurrentLine);
    sal_Int8 GetTextDirection (void) const;
    bool IsTextReferencePointLeft (void) const;
};
typedef ::boost::shared_ptr<PresenterTextParagraph> SharedPresenterTextParagraph;

/** A simple text view that paints text onto a given canvas.
*/
class PresenterTextView
{
public:

    PresenterTextView (
        const css::uno::Reference<css::uno::XComponentContext>& rxContext,
        const css::uno::Reference<css::rendering::XCanvas>& rxCanvas,
        const ::boost::function<void(const ::css::awt::Rectangle&)>& rInvalidator);
    void SetText (const css::uno::Reference<css::text::XText>& rxText);
    void SetTextChangeBroadcaster (const ::boost::function<void(void)>& rBroadcaster);

    void SetLocation (const css::geometry::RealPoint2D& rLocation);
    void SetSize (const css::geometry::RealSize2D& rSize);
    double GetTotalTextHeight (void);

    void SetFont (const PresenterTheme::SharedFontDescriptor& rpFont);

    void SetOffset (
        const double nLeft,
        const double nTop);

    /** Move the caret forward or backward by character or by word.
        @param nDistance
            Should be either -1 or +1 to move caret backwards or forwards,
            respectively.
        @param nTextType
            Valid values are the
            com::sun::star::accessibility::AccessibleTextType constants.
    */
    void MoveCaret (
        const sal_Int32 nDistance,
        const sal_Int16 nTextType);

    void Paint (const css::awt::Rectangle& rUpdateBox);

    SharedPresenterTextCaret GetCaret (void) const;

    sal_Int32 GetParagraphCount (void) const;
    SharedPresenterTextParagraph GetParagraph (const sal_Int32 nParagraphIndex) const;

private:
    css::uno::Reference<css::rendering::XCanvas> mxCanvas;
    bool mbDoOuput;
    css::uno::Reference<css::i18n::XBreakIterator> mxBreakIterator;
    css::uno::Reference<css::i18n::XScriptTypeDetector> mxScriptTypeDetector;
    css::geometry::RealPoint2D maLocation;
    css::geometry::RealSize2D maSize;
    PresenterTheme::SharedFontDescriptor mpFont;
    ::std::vector<SharedPresenterTextParagraph> maParagraphs;
    SharedPresenterTextCaret mpCaret;
    double mnLeftOffset;
    double mnTopOffset;
    const ::boost::function<void(const ::css::awt::Rectangle&)> maInvalidator;
    bool mbIsFormatPending;
    sal_Int32 mnCharacterCount;
    ::boost::function<void(void)> maTextChangeBroadcaster;

    void RequestFormat (void);
    void Format (void);
    SharedPresenterTextParagraph GetParagraphForCharacterIndex (const sal_Int32 nCharacterIndex) const;
    css::awt::Rectangle GetCaretBounds (
        const sal_Int32 nParagraphIndex,
        const sal_Int32 nCharacterIndex) const;
};

} } // end of namespace ::sdext::presenter

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
