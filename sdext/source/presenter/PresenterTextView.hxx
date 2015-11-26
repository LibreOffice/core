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

#ifndef INCLUDED_SDEXT_SOURCE_PRESENTER_PRESENTERTEXTVIEW_HXX
#define INCLUDED_SDEXT_SOURCE_PRESENTER_PRESENTERTEXTVIEW_HXX

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
#include <cppuhelper/basemutex.hxx>

#include <functional>

namespace sdext { namespace presenter {

class PresenterTextCaret
{
public:
    PresenterTextCaret (
        const ::std::function<css::awt::Rectangle (const sal_Int32,const sal_Int32)>&
            rCharacterBoundsAccess,
        const ::std::function<void (const css::awt::Rectangle&)>&
            rInvalidator);
    ~PresenterTextCaret();

    void ShowCaret();
    void HideCaret();

    sal_Int32 GetParagraphIndex() const { return mnParagraphIndex;}
    sal_Int32 GetCharacterIndex() const { return mnCharacterIndex;}
    void SetPosition (
        const sal_Int32 nParagraphIndex,
        const sal_Int32 nCharacterIndex);

    bool IsVisible() const { return mbIsCaretVisible;}

    /** Set a (possibly empty) functor that broadcasts changes of the caret
        position.  This is used when a PresenterTextView object is set at
        the accessibility object so that accessibility events can be sent
        when the caret changes position.
    */
    void SetCaretMotionBroadcaster (
        const ::std::function<void (sal_Int32,sal_Int32,sal_Int32,sal_Int32)>& rBroadcaster);

    css::awt::Rectangle GetBounds() const;

private:
    sal_Int32 mnParagraphIndex;
    sal_Int32 mnCharacterIndex;
    sal_Int32 mnCaretBlinkTaskId;
    bool mbIsCaretVisible;
    const ::std::function<css::awt::Rectangle (const sal_Int32,const sal_Int32)> maCharacterBoundsAccess;
    const ::std::function<void (const css::awt::Rectangle&)> maInvalidator;
    ::std::function<void (sal_Int32,sal_Int32,sal_Int32,sal_Int32)> maBroadcaster;
    css::awt::Rectangle maCaretBounds;

    void InvertCaret();
};
typedef std::shared_ptr<PresenterTextCaret> SharedPresenterTextCaret;

//===== PresenterTextParagraph ================================================

class PresenterTextParagraph
{
public:
    PresenterTextParagraph (
        const sal_Int32 nParagraphIndex,
        const css::uno::Reference<css::i18n::XBreakIterator>& rxBreakIterator,
        const css::uno::Reference<css::i18n::XScriptTypeDetector>& rxScriptTypeDetector,
        const css::uno::Reference<css::text::XTextRange>& rxTextRange,
        const SharedPresenterTextCaret& rpCaret);

    void Paint (
        const css::uno::Reference<css::rendering::XCanvas>& rxCanvas,
        const css::geometry::RealSize2D& rSize,
        const PresenterTheme::SharedFontDescriptor& rpFont,
        const css::rendering::ViewState& rViewState,
        css::rendering::RenderState& rRenderState,
        const double nTopOffset,
        const double nClipTop,
        const double nClipBottom);

    double GetTotalTextHeight();

    void SetCharacterOffset (const sal_Int32 nCharacterOffset);
    sal_Int32 GetCharacterCount() const;
    sal_Unicode GetCharacter (const sal_Int32 nGlobalCharacterIndex) const;
    OUString GetText() const;
    css::accessibility::TextSegment GetTextSegment (
        const sal_Int32 nOffset,
        const sal_Int32 nGlobalCharacterIndex,
        const sal_Int16 nTextType) const;
    css::accessibility::TextSegment GetWordTextSegment (
        const sal_Int32 nOffset,
        const sal_Int32 nIndex) const;
    css::accessibility::TextSegment CreateTextSegment (
        sal_Int32 nStartIndex,
        sal_Int32 nEndIndex) const;
    css::awt::Rectangle GetCharacterBounds (
        sal_Int32 nGlobalCharacterIndex,
        const bool bCaretBox);
    void SetupCellArray (
        const PresenterTheme::SharedFontDescriptor& rpFont);
    void Format (
        const double nY,
        const double nWidth,
        const PresenterTheme::SharedFontDescriptor& rpFont);
    sal_Int32 GetWordBoundary(
        const sal_Int32 nLocalCharacterIndex,
        const sal_Int32 nDistance);
    sal_Int32 GetCaretPosition() const;
    void SetCaretPosition (const sal_Int32 nPosition) const;
    void SetOrigin (const double nXOrigin, const double nYOrigin);
    css::awt::Point GetRelativeLocation() const;
    css::awt::Size GetSize();

private:
    OUString msParagraphText;
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
        css::uno::Reference<css::rendering::XTextLayout> mxLayoutedLine;
        double mnBaseLine;
        double mnWidth;
        css::uno::Sequence<css::geometry::RealRectangle2D> maCellBoxes;

        void ProvideLayoutedLine (
            const OUString& rsParagraphText,
            const PresenterTheme::SharedFontDescriptor& rpFont,
            const sal_Int8 nTextDirection);
        void ProvideCellBoxes();
        bool IsEmpty() const;
    };

    css::uno::Reference<css::i18n::XBreakIterator> mxBreakIterator;
    css::uno::Reference<css::i18n::XScriptTypeDetector> mxScriptTypeDetector;
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
    sal_Int8 GetTextDirection() const;
    bool IsTextReferencePointLeft() const;
};
typedef std::shared_ptr<PresenterTextParagraph> SharedPresenterTextParagraph;

/** A simple text view that paints text onto a given canvas.
*/
class PresenterTextView
{
public:

    PresenterTextView (
        const css::uno::Reference<css::uno::XComponentContext>& rxContext,
        const css::uno::Reference<css::rendering::XCanvas>& rxCanvas,
        const ::std::function<void (const css::awt::Rectangle&)>& rInvalidator);
    void SetText (const css::uno::Reference<css::text::XText>& rxText);
    void SetTextChangeBroadcaster(const ::std::function<void ()>& rBroadcaster);

    void SetLocation (const css::geometry::RealPoint2D& rLocation);
    void SetSize (const css::geometry::RealSize2D& rSize);
    double GetTotalTextHeight();

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
            css::accessibility::AccessibleTextType constants.
    */
    void MoveCaret (
        const sal_Int32 nDistance,
        const sal_Int16 nTextType);

    void Paint (const css::awt::Rectangle& rUpdateBox);

    SharedPresenterTextCaret GetCaret() const;

    sal_Int32 GetParagraphCount() const;
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
    bool mbIsFormatPending;
    sal_Int32 mnCharacterCount;
    ::std::function<void ()> maTextChangeBroadcaster;

    void RequestFormat();
    void Format();
    css::awt::Rectangle GetCaretBounds (
        const sal_Int32 nParagraphIndex,
        const sal_Int32 nCharacterIndex) const;
};

} } // end of namespace ::sdext::presenter

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
