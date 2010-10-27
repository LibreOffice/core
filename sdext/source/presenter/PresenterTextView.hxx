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
    PresenterTextParagraph (
        const sal_Int32 nParagraphIndex,
        const cssu::Reference<css::i18n::XBreakIterator>& rxBreakIterator,
        const cssu::Reference<css::i18n::XScriptTypeDetector>& rxScriptTypeDetector,
        const ::rtl::OUString& rsText,
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

    sal_Int32 GetParagraphIndex (void) const;
    double GetTotalTextHeight (void);

    sal_Int32 GetCharacterOffset (void) const;
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

        sal_Int32 GetLength (void) const;
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
    /** Create a new instance that does no output but only provides
        geometric information to an accessibility object.
    */
    PresenterTextView (
        const css::uno::Reference<css::uno::XComponentContext>& rxContext,
        const css::uno::Reference<css::rendering::XCanvas>& rxCanvas);

    void SetText (const css::uno::Reference<css::text::XText>& rxText);
    void SetText (const ::rtl::OUString& rsText);
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
    sal_Int32 GetCharacterOffset (const sal_Int32 nParagraphIndex) const;
    css::awt::Rectangle GetCaretBounds (
        const sal_Int32 nParagraphIndex,
        const sal_Int32 nCharacterIndex) const;
};

} } // end of namespace ::sdext::presenter

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
