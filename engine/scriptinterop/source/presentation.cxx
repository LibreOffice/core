/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <vector>

#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/awt/FontStrikeout.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/beans/Optional.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/drawing/XDrawPages.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/drawing/XDrawView.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/text/XParagraphAppend.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XTextCursor.hpp>
#include <com/sun/star/text/XTextPortionAppend.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <cpo/uno/Reference.hxx>
#include <cpo/uno/RuntimeException.hpp>
#include <cpo/uno/XInterface.hpp>
#include <cpo/uno/Any.hxx>
#include <cpo/uno/Sequence.hxx>
#include <cppuhelper/implbase.hxx>
#include <rtl/ustring.hxx>
#include <sal/config.h>
#include <sal/types.h>
#include <scriptinterop/ContentAlignment.hpp>
#include <scriptinterop/PageType.hpp>
#include <scriptinterop/PlaceholderType.hpp>
#include <scriptinterop/PredefinedLayout.hpp>
#include <scriptinterop/ShapeType.hpp>
#include <scriptinterop/SlideLinkingMode.hpp>
#include <scriptinterop/TextBaselineOffset.hpp>
#include <scriptinterop/XAutofit.hpp>
#include <scriptinterop/XAutoText.hpp>
#include <scriptinterop/XBorder.hpp>
#include <scriptinterop/XColor.hpp>
#include <scriptinterop/XColorScheme.hpp>
#include <scriptinterop/XFill.hpp>
#include <scriptinterop/XGroup.hpp>
#include <scriptinterop/XImage.hpp>
#include <scriptinterop/XLayout.hpp>
#include <scriptinterop/XLine.hpp>
#include <scriptinterop/XLink.hpp>
#include <scriptinterop/XListStyle.hpp>
#include <scriptinterop/XMaster.hpp>
#include <scriptinterop/XNotesMaster.hpp>
#include <scriptinterop/XNotesPage.hpp>
#include <scriptinterop/XPage.hpp>
#include <scriptinterop/XPageBackground.hpp>
#include <scriptinterop/XPageElement.hpp>
#include <scriptinterop/XPageElementBase.hpp>
#include <scriptinterop/XParagraphStyle.hpp>
#include <scriptinterop/XPresentation.hpp>
#include <scriptinterop/XShape.hpp>
#include <scriptinterop/XSlide.hpp>
#include <scriptinterop/XSlideSelection.hpp>
#include <scriptinterop/XSlideTable.hpp>
#include <scriptinterop/XTextParagraph.hpp>
#include <scriptinterop/XTextRange.hpp>
#include <scriptinterop/XTextStyle.hpp>
#include <scriptinterop/XVideo.hpp>
#include <scriptinterop/XWordArt.hpp>
#include <xmloff/autolayout.hxx>

#include "conversions.hxx"
#include "presentation.hxx"

using scriptinterop::detail::extentToHundredthMm;
using scriptinterop::detail::hundredthMmToPoints;
using scriptinterop::detail::parseHexColor;
using scriptinterop::detail::pointsToHundredthMm;

namespace
{
// Maps a predefined layout to the matching value of a page's Layout property.  Only the
// predefined layouts with a matching page layout are accepted; the others are still awaiting an
// implementation.
sal_Int16 predefinedAutoLayout(scriptinterop::PredefinedLayout layout)
{
    switch (layout)
    {
        case scriptinterop::PredefinedLayout_BLANK:
            return AUTOLAYOUT_NONE;
        case scriptinterop::PredefinedLayout_TITLE:
            return AUTOLAYOUT_TITLE;
        case scriptinterop::PredefinedLayout_TITLE_AND_BODY:
            return AUTOLAYOUT_TITLE_CONTENT;
        case scriptinterop::PredefinedLayout_TITLE_AND_TWO_COLUMNS:
            return AUTOLAYOUT_TITLE_2CONTENT;
        case scriptinterop::PredefinedLayout_TITLE_ONLY:
            return AUTOLAYOUT_TITLE_ONLY;
        default:
            throw cpo::uno::RuntimeException(
                u"appendSlide with this predefined layout: not implemented"_ustr);
    }
}

// Formatting is applied through a cursor, so it lands on the text runs themselves and survives
// saving.  With a range the cursor spans just that range; without one it spans the whole text.
cpo::uno::Reference<css::beans::XPropertySet>
cursorProperties(cpo::uno::Reference<css::text::XText> const& text,
                 cpo::uno::Reference<css::text::XTextRange> const& range)
{
    cpo::uno::Reference<css::text::XTextCursor> cursor;
    if (range.is())
    {
        cursor = text->createTextCursorByRange(range);
    }
    else
    {
        cursor = text->createTextCursor();
        cursor->gotoStart(false);
        cursor->gotoEnd(true);
    }
    return cpo::uno::Reference<css::beans::XPropertySet>(cursor, cpo::uno::UNO_QUERY_THROW);
}

// A page counts as one of the presentation's slides when the model's slide container holds it.
// Notes, handout and master pages live in other containers, so they do not count.
bool isSlide(cpo::uno::Reference<css::frame::XModel> const& model,
             cpo::uno::Reference<css::drawing::XDrawPage> const& page)
{
    cpo::uno::Reference<css::drawing::XDrawPagesSupplier> const sup(model, cpo::uno::UNO_QUERY);
    if (!sup.is())
    {
        return false;
    }
    auto const pages = sup->getDrawPages();
    auto const n = pages->getCount();
    for (sal_Int32 i = 0; i != n; ++i)
    {
        cpo::uno::Reference<css::drawing::XDrawPage> candidate;
        pages->getByIndex(i) >>= candidate;
        if (candidate == page)
        {
            return true;
        }
    }
    return false;
}

// Throwing defaults for the shared page element surface.  A leaf implementation class derives
// from this template with its own interface and overrides the methods it implements; the rest
// keep throwing until an implementation is added.
template <typename Iface> class PageElementStub : public cppu::WeakImplHelper<Iface>
{
public:
    cpo::uno::Reference<scriptinterop::XPageElementBase> SAL_CALL
    alignOnPage(scriptinterop::AlignmentPosition) override
    {
        throw cpo::uno::RuntimeException(u"alignOnPage: not implemented"_ustr);
    }

    cpo::uno::Reference<scriptinterop::XPageElementBase> SAL_CALL bringForward() override
    {
        throw cpo::uno::RuntimeException(u"bringForward: not implemented"_ustr);
    }

    cpo::uno::Reference<scriptinterop::XPageElementBase> SAL_CALL bringToFront() override
    {
        throw cpo::uno::RuntimeException(u"bringToFront: not implemented"_ustr);
    }

    cpo::uno::Reference<scriptinterop::XPageElement> SAL_CALL duplicate() override
    {
        throw cpo::uno::RuntimeException(u"duplicate: not implemented"_ustr);
    }

    cpo::uno::Sequence<cpo::uno::Reference<scriptinterop::XConnectionSite>> SAL_CALL
    getConnectionSites() override
    {
        throw cpo::uno::RuntimeException(u"getConnectionSites: not implemented"_ustr);
    }

    OUString SAL_CALL getDescription() override
    {
        throw cpo::uno::RuntimeException(u"getDescription: not implemented"_ustr);
    }

    css::beans::Optional<double> SAL_CALL getHeight() override
    {
        throw cpo::uno::RuntimeException(u"getHeight: not implemented"_ustr);
    }

    double SAL_CALL getInherentHeight() override
    {
        throw cpo::uno::RuntimeException(u"getInherentHeight: not implemented"_ustr);
    }

    double SAL_CALL getInherentWidth() override
    {
        throw cpo::uno::RuntimeException(u"getInherentWidth: not implemented"_ustr);
    }

    double SAL_CALL getLeft() override
    {
        throw cpo::uno::RuntimeException(u"getLeft: not implemented"_ustr);
    }

    OUString SAL_CALL getObjectId() override
    {
        throw cpo::uno::RuntimeException(u"getObjectId: not implemented"_ustr);
    }

    scriptinterop::PageElementType SAL_CALL getPageElementType() override
    {
        throw cpo::uno::RuntimeException(u"getPageElementType: not implemented"_ustr);
    }

    cpo::uno::Reference<scriptinterop::XGroup> SAL_CALL getParentGroup() override
    {
        throw cpo::uno::RuntimeException(u"getParentGroup: not implemented"_ustr);
    }

    cpo::uno::Reference<scriptinterop::XPage> SAL_CALL getParentPage() override
    {
        throw cpo::uno::RuntimeException(u"getParentPage: not implemented"_ustr);
    }

    double SAL_CALL getRotation() override
    {
        throw cpo::uno::RuntimeException(u"getRotation: not implemented"_ustr);
    }

    OUString SAL_CALL getTitle() override
    {
        throw cpo::uno::RuntimeException(u"getTitle: not implemented"_ustr);
    }

    double SAL_CALL getTop() override
    {
        throw cpo::uno::RuntimeException(u"getTop: not implemented"_ustr);
    }

    cpo::uno::Reference<scriptinterop::XAffineTransform> SAL_CALL getTransform() override
    {
        throw cpo::uno::RuntimeException(u"getTransform: not implemented"_ustr);
    }

    css::beans::Optional<double> SAL_CALL getWidth() override
    {
        throw cpo::uno::RuntimeException(u"getWidth: not implemented"_ustr);
    }

    cpo::uno::Reference<scriptinterop::XPageElementBase> SAL_CALL
    preconcatenateTransform(cpo::uno::Reference<scriptinterop::XAffineTransform> const&) override
    {
        throw cpo::uno::RuntimeException(u"preconcatenateTransform: not implemented"_ustr);
    }

    void SAL_CALL remove() override
    {
        throw cpo::uno::RuntimeException(u"remove: not implemented"_ustr);
    }

    cpo::uno::Reference<scriptinterop::XPageElementBase> SAL_CALL scaleHeight(double) override
    {
        throw cpo::uno::RuntimeException(u"scaleHeight: not implemented"_ustr);
    }

    cpo::uno::Reference<scriptinterop::XPageElementBase> SAL_CALL scaleWidth(double) override
    {
        throw cpo::uno::RuntimeException(u"scaleWidth: not implemented"_ustr);
    }

    void SAL_CALL select() override
    {
        throw cpo::uno::RuntimeException(u"select: not implemented"_ustr);
    }

    void SAL_CALL selectWithReplace(bool) override
    {
        throw cpo::uno::RuntimeException(u"selectWithReplace: not implemented"_ustr);
    }

    cpo::uno::Reference<scriptinterop::XPageElementBase> SAL_CALL sendBackward() override
    {
        throw cpo::uno::RuntimeException(u"sendBackward: not implemented"_ustr);
    }

    cpo::uno::Reference<scriptinterop::XPageElementBase> SAL_CALL sendToBack() override
    {
        throw cpo::uno::RuntimeException(u"sendToBack: not implemented"_ustr);
    }

    cpo::uno::Reference<scriptinterop::XPageElementBase> SAL_CALL
    setDescription(OUString const&) override
    {
        throw cpo::uno::RuntimeException(u"setDescription: not implemented"_ustr);
    }

    cpo::uno::Reference<scriptinterop::XPageElementBase> SAL_CALL setHeight(double) override
    {
        throw cpo::uno::RuntimeException(u"setHeight: not implemented"_ustr);
    }

    cpo::uno::Reference<scriptinterop::XPageElementBase> SAL_CALL setLeft(double) override
    {
        throw cpo::uno::RuntimeException(u"setLeft: not implemented"_ustr);
    }

    cpo::uno::Reference<scriptinterop::XPageElementBase> SAL_CALL setRotation(double) override
    {
        throw cpo::uno::RuntimeException(u"setRotation: not implemented"_ustr);
    }

    cpo::uno::Reference<scriptinterop::XPageElementBase> SAL_CALL
    setTitle(OUString const&) override
    {
        throw cpo::uno::RuntimeException(u"setTitle: not implemented"_ustr);
    }

    cpo::uno::Reference<scriptinterop::XPageElementBase> SAL_CALL setTop(double) override
    {
        throw cpo::uno::RuntimeException(u"setTop: not implemented"_ustr);
    }

    cpo::uno::Reference<scriptinterop::XPageElementBase> SAL_CALL
    setTransform(cpo::uno::Reference<scriptinterop::XAffineTransform> const&) override
    {
        throw cpo::uno::RuntimeException(u"setTransform: not implemented"_ustr);
    }

    cpo::uno::Reference<scriptinterop::XPageElementBase> SAL_CALL setWidth(double) override
    {
        throw cpo::uno::RuntimeException(u"setWidth: not implemented"_ustr);
    }
};

// Throwing defaults for the shared page surface.  A leaf implementation class derives from this
// template with its own interface and overrides the methods it implements; the rest keep
// throwing until an implementation is added.
template <typename Iface> class EditablePageStub : public cppu::WeakImplHelper<Iface>
{
public:
    cpo::uno::Sequence<cpo::uno::Reference<scriptinterop::XGroup>> SAL_CALL getGroups() override
    {
        throw cpo::uno::RuntimeException(u"getGroups: not implemented"_ustr);
    }

    cpo::uno::Sequence<cpo::uno::Reference<scriptinterop::XImage>> SAL_CALL getImages() override
    {
        throw cpo::uno::RuntimeException(u"getImages: not implemented"_ustr);
    }

    cpo::uno::Sequence<cpo::uno::Reference<scriptinterop::XLine>> SAL_CALL getLines() override
    {
        throw cpo::uno::RuntimeException(u"getLines: not implemented"_ustr);
    }

    OUString SAL_CALL getObjectId() override
    {
        throw cpo::uno::RuntimeException(u"getObjectId: not implemented"_ustr);
    }

    cpo::uno::Reference<scriptinterop::XPageElement> SAL_CALL
    getPageElementById(OUString const&) override
    {
        throw cpo::uno::RuntimeException(u"getPageElementById: not implemented"_ustr);
    }

    cpo::uno::Sequence<cpo::uno::Reference<scriptinterop::XPageElement>> SAL_CALL
    getPageElements() override
    {
        throw cpo::uno::RuntimeException(u"getPageElements: not implemented"_ustr);
    }

    cpo::uno::Reference<scriptinterop::XPageElement> SAL_CALL
    getPlaceholder(scriptinterop::PlaceholderType) override
    {
        throw cpo::uno::RuntimeException(u"getPlaceholder: not implemented"_ustr);
    }

    cpo::uno::Reference<scriptinterop::XPageElement> SAL_CALL
    getPlaceholderByIndex(scriptinterop::PlaceholderType, sal_Int32) override
    {
        throw cpo::uno::RuntimeException(u"getPlaceholderByIndex: not implemented"_ustr);
    }

    cpo::uno::Sequence<cpo::uno::Reference<scriptinterop::XPageElement>> SAL_CALL
    getPlaceholders() override
    {
        throw cpo::uno::RuntimeException(u"getPlaceholders: not implemented"_ustr);
    }

    cpo::uno::Sequence<cpo::uno::Reference<scriptinterop::XShape>> SAL_CALL getShapes() override
    {
        throw cpo::uno::RuntimeException(u"getShapes: not implemented"_ustr);
    }

    cpo::uno::Sequence<cpo::uno::Reference<scriptinterop::XSlideTable>> SAL_CALL
    getTables() override
    {
        throw cpo::uno::RuntimeException(u"getTables: not implemented"_ustr);
    }

    cpo::uno::Sequence<cpo::uno::Reference<scriptinterop::XVideo>> SAL_CALL getVideos() override
    {
        throw cpo::uno::RuntimeException(u"getVideos: not implemented"_ustr);
    }

    cpo::uno::Sequence<cpo::uno::Reference<scriptinterop::XWordArt>> SAL_CALL
    getWordArts() override
    {
        throw cpo::uno::RuntimeException(u"getWordArts: not implemented"_ustr);
    }

    cpo::uno::Reference<scriptinterop::XPageBackground> SAL_CALL getBackground() override
    {
        throw cpo::uno::RuntimeException(u"getBackground: not implemented"_ustr);
    }

    cpo::uno::Reference<scriptinterop::XColorScheme> SAL_CALL getColorScheme() override
    {
        throw cpo::uno::RuntimeException(u"getColorScheme: not implemented"_ustr);
    }

    scriptinterop::PageType SAL_CALL getPageType() override
    {
        throw cpo::uno::RuntimeException(u"getPageType: not implemented"_ustr);
    }

    cpo::uno::Reference<scriptinterop::XGroup> SAL_CALL
    group(cpo::uno::Sequence<cpo::uno::Reference<scriptinterop::XPageElementBase>> const&)
        override
    {
        throw cpo::uno::RuntimeException(u"group: not implemented"_ustr);
    }

    cpo::uno::Reference<scriptinterop::XGroup> SAL_CALL
    insertGroup(cpo::uno::Reference<scriptinterop::XGroup> const&) override
    {
        throw cpo::uno::RuntimeException(u"insertGroup: not implemented"_ustr);
    }

    cpo::uno::Reference<scriptinterop::XImage> SAL_CALL insertImage(cpo::uno::Any const&) override
    {
        throw cpo::uno::RuntimeException(u"insertImage: not implemented"_ustr);
    }

    cpo::uno::Reference<scriptinterop::XImage> SAL_CALL
    insertImageAt(cpo::uno::Any const&, double, double, double, double) override
    {
        throw cpo::uno::RuntimeException(u"insertImageAt: not implemented"_ustr);
    }

    cpo::uno::Reference<scriptinterop::XLine> SAL_CALL
    insertLine(cpo::uno::Reference<scriptinterop::XLine> const&) override
    {
        throw cpo::uno::RuntimeException(u"insertLine: not implemented"_ustr);
    }

    cpo::uno::Reference<scriptinterop::XLine> SAL_CALL
    insertLineBetween(scriptinterop::LineCategory,
                      cpo::uno::Reference<scriptinterop::XConnectionSite> const&,
                      cpo::uno::Reference<scriptinterop::XConnectionSite> const&) override
    {
        throw cpo::uno::RuntimeException(u"insertLineBetween: not implemented"_ustr);
    }

    cpo::uno::Reference<scriptinterop::XLine> SAL_CALL
    insertLineAt(scriptinterop::LineCategory, double, double, double, double) override
    {
        throw cpo::uno::RuntimeException(u"insertLineAt: not implemented"_ustr);
    }

    cpo::uno::Reference<scriptinterop::XPageElement> SAL_CALL
    insertPageElement(cpo::uno::Reference<scriptinterop::XPageElement> const&) override
    {
        throw cpo::uno::RuntimeException(u"insertPageElement: not implemented"_ustr);
    }

    cpo::uno::Reference<scriptinterop::XShape> SAL_CALL insertShape(cpo::uno::Any const&) override
    {
        throw cpo::uno::RuntimeException(u"insertShape: not implemented"_ustr);
    }

    cpo::uno::Reference<scriptinterop::XShape> SAL_CALL
    insertShapeAt(scriptinterop::ShapeType, double, double, double, double) override
    {
        throw cpo::uno::RuntimeException(u"insertShapeAt: not implemented"_ustr);
    }

    cpo::uno::Reference<scriptinterop::XSlideTable> SAL_CALL insertTable(sal_Int32,
                                                                         sal_Int32) override
    {
        throw cpo::uno::RuntimeException(u"insertTable: not implemented"_ustr);
    }

    cpo::uno::Reference<scriptinterop::XSlideTable> SAL_CALL
    insertTableAt(sal_Int32, sal_Int32, double, double, double, double) override
    {
        throw cpo::uno::RuntimeException(u"insertTableAt: not implemented"_ustr);
    }

    cpo::uno::Reference<scriptinterop::XSlideTable> SAL_CALL
    insertTableCopy(cpo::uno::Reference<scriptinterop::XSlideTable> const&) override
    {
        throw cpo::uno::RuntimeException(u"insertTableCopy: not implemented"_ustr);
    }

    cpo::uno::Reference<scriptinterop::XShape> SAL_CALL insertTextBox(OUString const&) override
    {
        throw cpo::uno::RuntimeException(u"insertTextBox: not implemented"_ustr);
    }

    cpo::uno::Reference<scriptinterop::XShape> SAL_CALL
    insertTextBoxAt(OUString const&, double, double, double, double) override
    {
        throw cpo::uno::RuntimeException(u"insertTextBoxAt: not implemented"_ustr);
    }

    cpo::uno::Reference<scriptinterop::XWordArt> SAL_CALL
    insertWordArt(cpo::uno::Reference<scriptinterop::XWordArt> const&) override
    {
        throw cpo::uno::RuntimeException(u"insertWordArt: not implemented"_ustr);
    }

    void SAL_CALL remove() override
    {
        throw cpo::uno::RuntimeException(u"remove: not implemented"_ustr);
    }

    sal_Int32 SAL_CALL replaceAllText(OUString const&, OUString const&) override
    {
        throw cpo::uno::RuntimeException(u"replaceAllText: not implemented"_ustr);
    }

    sal_Int32 SAL_CALL replaceAllTextMatchCase(OUString const&, OUString const&, bool) override
    {
        throw cpo::uno::RuntimeException(u"replaceAllTextMatchCase: not implemented"_ustr);
    }

    void SAL_CALL selectAsCurrentPage() override
    {
        throw cpo::uno::RuntimeException(u"selectAsCurrentPage: not implemented"_ustr);
    }
};

class TextStyleImpl : public cppu::WeakImplHelper<scriptinterop::XTextStyle>
{
public:
    TextStyleImpl(cpo::uno::Reference<css::text::XText> const& text,
                  cpo::uno::Reference<css::text::XTextRange> const& range)
        : text_(text)
        , range_(range)
    {
    }

    cpo::uno::Reference<cpo::uno::XInterface> SAL_CALL getuno() override
    {
        if (range_.is())
        {
            return range_;
        }
        return text_;
    }

    cpo::uno::Reference<scriptinterop::XColor> SAL_CALL getBackgroundColor() override
    {
        throw cpo::uno::RuntimeException(u"getBackgroundColor: not implemented"_ustr);
    }

    scriptinterop::TextBaselineOffset SAL_CALL getBaselineOffset() override
    {
        throw cpo::uno::RuntimeException(u"getBaselineOffset: not implemented"_ustr);
    }

    OUString SAL_CALL getFontFamily() override
    {
        throw cpo::uno::RuntimeException(u"getFontFamily: not implemented"_ustr);
    }

    double SAL_CALL getFontSize() override
    {
        throw cpo::uno::RuntimeException(u"getFontSize: not implemented"_ustr);
    }

    cpo::uno::Reference<scriptinterop::XColor> SAL_CALL getForegroundColor() override
    {
        throw cpo::uno::RuntimeException(u"getForegroundColor: not implemented"_ustr);
    }

    cpo::uno::Reference<scriptinterop::XLink> SAL_CALL getLink() override
    {
        throw cpo::uno::RuntimeException(u"getLink: not implemented"_ustr);
    }

    bool SAL_CALL hasLink() override
    {
        throw cpo::uno::RuntimeException(u"hasLink: not implemented"_ustr);
    }

    bool SAL_CALL isBackgroundTransparent() override
    {
        throw cpo::uno::RuntimeException(u"isBackgroundTransparent: not implemented"_ustr);
    }

    bool SAL_CALL isBold() override
    {
        throw cpo::uno::RuntimeException(u"isBold: not implemented"_ustr);
    }

    bool SAL_CALL isItalic() override
    {
        throw cpo::uno::RuntimeException(u"isItalic: not implemented"_ustr);
    }

    bool SAL_CALL isSmallCaps() override
    {
        throw cpo::uno::RuntimeException(u"isSmallCaps: not implemented"_ustr);
    }

    bool SAL_CALL isStrikethrough() override
    {
        throw cpo::uno::RuntimeException(u"isStrikethrough: not implemented"_ustr);
    }

    bool SAL_CALL isUnderline() override
    {
        throw cpo::uno::RuntimeException(u"isUnderline: not implemented"_ustr);
    }

    cpo::uno::Reference<scriptinterop::XTextStyle> SAL_CALL removeLink() override
    {
        throw cpo::uno::RuntimeException(u"removeLink: not implemented"_ustr);
    }

    cpo::uno::Reference<scriptinterop::XTextStyle>
        SAL_CALL setBackgroundColor(cpo::uno::Any const&) override
    {
        throw cpo::uno::RuntimeException(u"setBackgroundColor: not implemented"_ustr);
    }

    cpo::uno::Reference<scriptinterop::XTextStyle>
        SAL_CALL setBackgroundColorRgb(sal_Int32, sal_Int32, sal_Int32) override
    {
        throw cpo::uno::RuntimeException(u"setBackgroundColorRgb: not implemented"_ustr);
    }

    cpo::uno::Reference<scriptinterop::XTextStyle> SAL_CALL setBackgroundColorTransparent() override
    {
        throw cpo::uno::RuntimeException(u"setBackgroundColorTransparent: not implemented"_ustr);
    }

    cpo::uno::Reference<scriptinterop::XTextStyle>
        SAL_CALL setBaselineOffset(scriptinterop::TextBaselineOffset) override
    {
        throw cpo::uno::RuntimeException(u"setBaselineOffset: not implemented"_ustr);
    }

    cpo::uno::Reference<scriptinterop::XTextStyle> SAL_CALL setBold(bool bold) override
    {
        auto const props = cursorProperties(text_, range_);
        auto const weight = bold ? css::awt::FontWeight::BOLD : css::awt::FontWeight::NORMAL;
        props->setPropertyValue(u"CharWeight"_ustr, cpo::uno::Any(weight));
        props->setPropertyValue(u"CharWeightAsian"_ustr, cpo::uno::Any(weight));
        props->setPropertyValue(u"CharWeightComplex"_ustr, cpo::uno::Any(weight));
        return this;
    }

    cpo::uno::Reference<scriptinterop::XTextStyle> SAL_CALL setFontFamily(OUString const&) override
    {
        throw cpo::uno::RuntimeException(u"setFontFamily: not implemented"_ustr);
    }

    cpo::uno::Reference<scriptinterop::XTextStyle> SAL_CALL setFontSize(double points) override
    {
        auto const props = cursorProperties(text_, range_);
        // CharHeight is measured in points, so the value passes through unconverted:
        auto const height = static_cast<float>(points);
        props->setPropertyValue(u"CharHeight"_ustr, cpo::uno::Any(height));
        props->setPropertyValue(u"CharHeightAsian"_ustr, cpo::uno::Any(height));
        props->setPropertyValue(u"CharHeightComplex"_ustr, cpo::uno::Any(height));
        return this;
    }

    cpo::uno::Reference<scriptinterop::XTextStyle>
        SAL_CALL setForegroundColor(cpo::uno::Any const& color) override
    {
        if (!color.has<OUString>())
        {
            throw cpo::uno::RuntimeException(u"setForegroundColor: not implemented"_ustr);
        }
        cursorProperties(text_, range_)
            ->setPropertyValue(u"CharColor"_ustr,
                               cpo::uno::Any(parseHexColor(color.get<OUString>())));
        return this;
    }

    cpo::uno::Reference<scriptinterop::XTextStyle>
        SAL_CALL setForegroundColorRgb(sal_Int32, sal_Int32, sal_Int32) override
    {
        throw cpo::uno::RuntimeException(u"setForegroundColorRgb: not implemented"_ustr);
    }

    cpo::uno::Reference<scriptinterop::XTextStyle> SAL_CALL setItalic(bool italic) override
    {
        auto const props = cursorProperties(text_, range_);
        auto const slant = italic ? css::awt::FontSlant_ITALIC : css::awt::FontSlant_NONE;
        props->setPropertyValue(u"CharPosture"_ustr, cpo::uno::Any(slant));
        props->setPropertyValue(u"CharPostureAsian"_ustr, cpo::uno::Any(slant));
        props->setPropertyValue(u"CharPostureComplex"_ustr, cpo::uno::Any(slant));
        return this;
    }

    cpo::uno::Reference<scriptinterop::XTextStyle> SAL_CALL setLinkSlide(cpo::uno::Any const&)
        override
    {
        throw cpo::uno::RuntimeException(u"setLinkSlide: not implemented"_ustr);
    }

    cpo::uno::Reference<scriptinterop::XTextStyle> SAL_CALL setLinkUrl(OUString const&) override
    {
        throw cpo::uno::RuntimeException(u"setLinkUrl: not implemented"_ustr);
    }

    cpo::uno::Reference<scriptinterop::XTextStyle> SAL_CALL setSmallCaps(bool) override
    {
        throw cpo::uno::RuntimeException(u"setSmallCaps: not implemented"_ustr);
    }

    cpo::uno::Reference<scriptinterop::XTextStyle>
        SAL_CALL setStrikethrough(bool strikethrough) override
    {
        // A single CharStrikeout property covers all scripts; there is no Asian or complex
        // variant.
        auto const strikeout
            = strikethrough ? css::awt::FontStrikeout::SINGLE : css::awt::FontStrikeout::NONE;
        cursorProperties(text_, range_)
            ->setPropertyValue(u"CharStrikeout"_ustr, cpo::uno::Any(strikeout));
        return this;
    }

    cpo::uno::Reference<scriptinterop::XTextStyle> SAL_CALL setUnderline(bool) override
    {
        throw cpo::uno::RuntimeException(u"setUnderline: not implemented"_ustr);
    }

private:
    cpo::uno::Reference<css::text::XText> text_;
    cpo::uno::Reference<css::text::XTextRange> range_;
};

class TextParagraphImpl : public cppu::WeakImplHelper<scriptinterop::XTextParagraph>
{
public:
    explicit TextParagraphImpl(cpo::uno::Reference<scriptinterop::XTextRange> const& range)
        : range_(range)
    {
    }

    cpo::uno::Reference<cpo::uno::XInterface> SAL_CALL getuno() override
    {
        return range_->getuno();
    }

    sal_Int32 SAL_CALL getIndex() override
    {
        throw cpo::uno::RuntimeException(u"getIndex: not implemented"_ustr);
    }

    css::beans::Optional<cpo::uno::Reference<scriptinterop::XTextRange>> SAL_CALL getRange()
        override
    {
        return {range_.is(), range_};
    }

private:
    cpo::uno::Reference<scriptinterop::XTextRange> range_;
};

class TextRangeImpl : public cppu::WeakImplHelper<scriptinterop::XTextRange>
{
public:
    explicit TextRangeImpl(cpo::uno::Reference<css::text::XText> const& text)
        : text_(text)
    {
    }

    TextRangeImpl(cpo::uno::Reference<css::text::XText> const& text,
                  cpo::uno::Reference<css::text::XTextRange> const& range)
        : text_(text)
        , range_(range)
    {
    }

    cpo::uno::Reference<cpo::uno::XInterface> SAL_CALL getuno() override
    {
        if (range_.is())
        {
            return range_;
        }
        return text_;
    }

    cpo::uno::Reference<scriptinterop::XTextRange> SAL_CALL
    appendRange(cpo::uno::Reference<scriptinterop::XTextRange> const&) override
    {
        throw cpo::uno::RuntimeException(u"appendRange: not implemented"_ustr);
    }

    cpo::uno::Reference<scriptinterop::XTextRange> SAL_CALL
    appendRangeMatchSourceFormatting(cpo::uno::Reference<scriptinterop::XTextRange> const&,
                                     bool) override
    {
        throw cpo::uno::RuntimeException(u"appendRangeMatchSourceFormatting: not implemented"_ustr);
    }

    OUString SAL_CALL asRenderedString() override
    {
        throw cpo::uno::RuntimeException(u"asRenderedString: not implemented"_ustr);
    }

    OUString SAL_CALL asString() override
    {
        if (range_.is())
        {
            return range_->getString();
        }
        return text_->getString();
    }

    void SAL_CALL clear() override
    {
        throw cpo::uno::RuntimeException(u"clear: not implemented"_ustr);
    }

    void SAL_CALL clearRange(sal_Int32, sal_Int32) override
    {
        throw cpo::uno::RuntimeException(u"clearRange: not implemented"_ustr);
    }

    cpo::uno::Sequence<cpo::uno::Reference<scriptinterop::XTextRange>> SAL_CALL
    find(OUString const&) override
    {
        throw cpo::uno::RuntimeException(u"find: not implemented"_ustr);
    }

    cpo::uno::Sequence<cpo::uno::Reference<scriptinterop::XTextRange>> SAL_CALL
    findFrom(OUString const&, sal_Int32) override
    {
        throw cpo::uno::RuntimeException(u"findFrom: not implemented"_ustr);
    }

    cpo::uno::Sequence<cpo::uno::Reference<scriptinterop::XAutoText>> SAL_CALL
    getAutoTexts() override
    {
        throw cpo::uno::RuntimeException(u"getAutoTexts: not implemented"_ustr);
    }

    sal_Int32 SAL_CALL getEndIndex() override
    {
        throw cpo::uno::RuntimeException(u"getEndIndex: not implemented"_ustr);
    }

    sal_Int32 SAL_CALL getLength() override
    {
        throw cpo::uno::RuntimeException(u"getLength: not implemented"_ustr);
    }

    cpo::uno::Sequence<cpo::uno::Reference<scriptinterop::XTextRange>> SAL_CALL getLinks() override
    {
        throw cpo::uno::RuntimeException(u"getLinks: not implemented"_ustr);
    }

    cpo::uno::Sequence<cpo::uno::Reference<scriptinterop::XTextParagraph>> SAL_CALL
    getListParagraphs() override
    {
        throw cpo::uno::RuntimeException(u"getListParagraphs: not implemented"_ustr);
    }

    cpo::uno::Reference<scriptinterop::XListStyle> SAL_CALL getListStyle() override
    {
        throw cpo::uno::RuntimeException(u"getListStyle: not implemented"_ustr);
    }

    cpo::uno::Sequence<cpo::uno::Reference<scriptinterop::XTextParagraph>> SAL_CALL
    getParagraphs() override
    {
        throw cpo::uno::RuntimeException(u"getParagraphs: not implemented"_ustr);
    }

    cpo::uno::Reference<scriptinterop::XParagraphStyle> SAL_CALL getParagraphStyle() override
    {
        throw cpo::uno::RuntimeException(u"getParagraphStyle: not implemented"_ustr);
    }

    cpo::uno::Reference<scriptinterop::XTextRange> SAL_CALL getRange(sal_Int32, sal_Int32) override
    {
        throw cpo::uno::RuntimeException(u"getRange: not implemented"_ustr);
    }

    cpo::uno::Sequence<cpo::uno::Reference<scriptinterop::XTextRange>> SAL_CALL getRuns() override
    {
        throw cpo::uno::RuntimeException(u"getRuns: not implemented"_ustr);
    }

    sal_Int32 SAL_CALL getStartIndex() override
    {
        throw cpo::uno::RuntimeException(u"getStartIndex: not implemented"_ustr);
    }

    cpo::uno::Reference<scriptinterop::XTextParagraph> SAL_CALL
    insertParagraph(sal_Int32, OUString const&) override
    {
        throw cpo::uno::RuntimeException(u"insertParagraph: not implemented"_ustr);
    }

    cpo::uno::Reference<scriptinterop::XTextRange> SAL_CALL
    insertRange(sal_Int32, cpo::uno::Reference<scriptinterop::XTextRange> const&) override
    {
        throw cpo::uno::RuntimeException(u"insertRange: not implemented"_ustr);
    }

    cpo::uno::Reference<scriptinterop::XTextRange> SAL_CALL
    insertRangeMatchSourceFormatting(sal_Int32,
                                     cpo::uno::Reference<scriptinterop::XTextRange> const&,
                                     bool) override
    {
        throw cpo::uno::RuntimeException(u"insertRangeMatchSourceFormatting: not implemented"_ustr);
    }

    cpo::uno::Reference<scriptinterop::XTextRange> SAL_CALL
    insertText(sal_Int32, OUString const&) override
    {
        throw cpo::uno::RuntimeException(u"insertText: not implemented"_ustr);
    }

    bool SAL_CALL isEmpty() override
    {
        throw cpo::uno::RuntimeException(u"isEmpty: not implemented"_ustr);
    }

    sal_Int32 SAL_CALL replaceAllText(OUString const&, OUString const&) override
    {
        throw cpo::uno::RuntimeException(u"replaceAllText: not implemented"_ustr);
    }

    sal_Int32 SAL_CALL replaceAllTextMatchCase(OUString const&, OUString const&, bool) override
    {
        throw cpo::uno::RuntimeException(u"replaceAllTextMatchCase: not implemented"_ustr);
    }

    void SAL_CALL select() override
    {
        throw cpo::uno::RuntimeException(u"select: not implemented"_ustr);
    }

    cpo::uno::Reference<scriptinterop::XTextRange> SAL_CALL setText(OUString const& newText)
        override
    {
        if (range_.is())
        {
            range_->setString(newText);
        }
        else
        {
            text_->setString(newText);
        }
        return this;
    }

    cpo::uno::Reference<scriptinterop::XTextRange> SAL_CALL appendText(OUString const& text)
        override
    {
        if (range_.is())
        {
            throw cpo::uno::RuntimeException(
                u"appendText: only the shape's whole text range can append"_ustr);
        }
        cpo::uno::Reference<css::text::XTextPortionAppend> const append(text_,
                                                                        cpo::uno::UNO_QUERY);
        if (!append.is())
        {
            throw cpo::uno::RuntimeException(
                u"appendText: the text cannot take appended runs"_ustr);
        }
        // The engine returns a range covering exactly the appended run, with any character
        // formatting inherited from the preceding text stripped off.
        auto const run = append->appendTextPortion(text, {});
        if (!run.is())
        {
            throw cpo::uno::RuntimeException(u"appendText: appending failed"_ustr);
        }
        return new TextRangeImpl(text_, run);
    }

    cpo::uno::Reference<scriptinterop::XTextParagraph> SAL_CALL
    appendParagraph(OUString const& text) override
    {
        if (range_.is())
        {
            throw cpo::uno::RuntimeException(
                u"appendParagraph: only the shape's whole text range can append"_ustr);
        }
        cpo::uno::Reference<css::text::XParagraphAppend> const append(text_,
                                                                      cpo::uno::UNO_QUERY);
        if (!append.is())
        {
            throw cpo::uno::RuntimeException(
                u"appendParagraph: the text cannot take appended paragraphs"_ustr);
        }
        // finishParagraph reports the paragraph it closed; the interesting one is the new empty
        // last paragraph at the text end, where the given text goes.
        append->finishParagraph({});
        if (!text.isEmpty())
        {
            // The appended run lands in the new last paragraph and covers exactly its text, so
            // it doubles as the paragraph's range.
            return new TextParagraphImpl(appendText(text));
        }
        // With no text the new paragraph stays empty, so the paragraph's range is a cursor at
        // the text end.  That cursor is a live position: the edit engine keeps it inside the
        // last paragraph as text is inserted into it.
        auto const cursor = text_->createTextCursor();
        cursor->gotoEnd(false);
        return new TextParagraphImpl(new TextRangeImpl(text_, cursor));
    }

    cpo::uno::Reference<scriptinterop::XTextRange> SAL_CALL setBulletLevel(sal_Int32 level)
        override
    {
        if (level < -1 || level > 9)
        {
            throw cpo::uno::RuntimeException(
                u"setBulletLevel: the level must be between -1 (no bullet) and 9"_ustr);
        }
        // NumberingLevel drives the outliner depth of every paragraph the underlying cursor
        // touches: with a range that is each paragraph the range reaches into, and without one
        // it is every paragraph of the text.  The document's preset bullet list supplies the
        // glyph and the per-level indent.  -1 takes the paragraphs off the list.
        cursorProperties(text_, range_)
            ->setPropertyValue(u"NumberingLevel"_ustr,
                               cpo::uno::Any(static_cast<sal_Int16>(level)));
        return this;
    }

    css::beans::Optional<cpo::uno::Reference<scriptinterop::XTextStyle>> SAL_CALL getTextStyle()
        override
    {
        // Character formatting lives on the text runs, so an empty range holds none to style.
        if (asString().isEmpty())
        {
            return {false, {}};
        }
        return {true, new TextStyleImpl(text_, range_)};
    }

private:
    cpo::uno::Reference<css::text::XText> text_;
    cpo::uno::Reference<css::text::XTextRange> range_;
};

class ShapeImpl : public PageElementStub<scriptinterop::XShape>
{
public:
    ShapeImpl(cpo::uno::Reference<css::drawing::XDrawPage> const& page,
              cpo::uno::Reference<css::drawing::XShape> const& shape)
        : page_(page)
        , shape_(shape)
    {
    }

    cpo::uno::Reference<cpo::uno::XInterface> SAL_CALL getuno() override { return shape_; }

    cpo::uno::Reference<scriptinterop::XTextRange> SAL_CALL getText() override
    {
        cpo::uno::Reference<css::text::XText> const text(shape_, cpo::uno::UNO_QUERY);
        if (!text.is())
        {
            throw cpo::uno::RuntimeException(u"getText: shape cannot hold text"_ustr);
        }
        return new TextRangeImpl(text);
    }

    double SAL_CALL getLeft() override { return hundredthMmToPoints(shape_->getPosition().X); }

    double SAL_CALL getTop() override { return hundredthMmToPoints(shape_->getPosition().Y); }

    css::beans::Optional<double> SAL_CALL getWidth() override {
        return {true, hundredthMmToPoints(shape_->getSize().Width)};
    }

    css::beans::Optional<double> SAL_CALL getHeight() override {
        return {true, hundredthMmToPoints(shape_->getSize().Height)};
    }

    cpo::uno::Reference<scriptinterop::XPageElementBase> SAL_CALL setLeft(double points) override
    {
        auto pos = shape_->getPosition();
        pos.X = pointsToHundredthMm(points);
        shape_->setPosition(pos);
        return this;
    }

    cpo::uno::Reference<scriptinterop::XPageElementBase> SAL_CALL setTop(double points) override
    {
        auto pos = shape_->getPosition();
        pos.Y = pointsToHundredthMm(points);
        shape_->setPosition(pos);
        return this;
    }

    cpo::uno::Reference<scriptinterop::XPageElementBase> SAL_CALL setWidth(double points) override
    {
        auto size = shape_->getSize();
        size.Width = extentToHundredthMm(points);
        shape_->setSize(size);
        return this;
    }

    cpo::uno::Reference<scriptinterop::XPageElementBase> SAL_CALL setHeight(double points) override
    {
        auto size = shape_->getSize();
        size.Height = extentToHundredthMm(points);
        shape_->setSize(size);
        return this;
    }

    void SAL_CALL remove() override
    {
        cpo::uno::Reference<css::drawing::XShapes> const shapes(page_, cpo::uno::UNO_QUERY_THROW);
        shapes->remove(shape_);
    }

    cpo::uno::Reference<scriptinterop::XFill> SAL_CALL getFill() override
    {
        throw cpo::uno::RuntimeException(u"getFill: not implemented"_ustr);
    }

    cpo::uno::Reference<scriptinterop::XAutofit> SAL_CALL getAutofit() override
    {
        throw cpo::uno::RuntimeException(u"getAutofit: not implemented"_ustr);
    }

    cpo::uno::Reference<scriptinterop::XBorder> SAL_CALL getBorder() override
    {
        throw cpo::uno::RuntimeException(u"getBorder: not implemented"_ustr);
    }

    cpo::uno::Reference<scriptinterop::XLink> SAL_CALL getLink() override
    {
        throw cpo::uno::RuntimeException(u"getLink: not implemented"_ustr);
    }

    void SAL_CALL removeLink() override
    {
        throw cpo::uno::RuntimeException(u"removeLink: not implemented"_ustr);
    }

    cpo::uno::Reference<scriptinterop::XLink> SAL_CALL setLinkUrl(OUString const&) override
    {
        throw cpo::uno::RuntimeException(u"setLinkUrl: not implemented"_ustr);
    }

    cpo::uno::Reference<scriptinterop::XLink> SAL_CALL setLinkSlide(cpo::uno::Any const&) override
    {
        throw cpo::uno::RuntimeException(u"setLinkSlide: not implemented"_ustr);
    }

    cpo::uno::Reference<scriptinterop::XPageElement> SAL_CALL getParentPlaceholder() override
    {
        throw cpo::uno::RuntimeException(u"getParentPlaceholder: not implemented"_ustr);
    }

    sal_Int32 SAL_CALL getPlaceholderIndex() override
    {
        throw cpo::uno::RuntimeException(u"getPlaceholderIndex: not implemented"_ustr);
    }

    scriptinterop::PlaceholderType SAL_CALL getPlaceholderType() override
    {
        throw cpo::uno::RuntimeException(u"getPlaceholderType: not implemented"_ustr);
    }

    scriptinterop::ShapeType SAL_CALL getShapeType() override
    {
        throw cpo::uno::RuntimeException(u"getShapeType: not implemented"_ustr);
    }

    scriptinterop::ContentAlignment SAL_CALL getContentAlignment() override
    {
        throw cpo::uno::RuntimeException(u"getContentAlignment: not implemented"_ustr);
    }

    cpo::uno::Reference<scriptinterop::XShape> SAL_CALL
    setContentAlignment(scriptinterop::ContentAlignment) override
    {
        throw cpo::uno::RuntimeException(u"setContentAlignment: not implemented"_ustr);
    }

    cpo::uno::Reference<scriptinterop::XImage> SAL_CALL
    replaceWithImage(cpo::uno::Any const&) override
    {
        throw cpo::uno::RuntimeException(u"replaceWithImage: not implemented"_ustr);
    }

    cpo::uno::Reference<scriptinterop::XImage> SAL_CALL
    replaceWithImageCropped(cpo::uno::Any const&, bool) override
    {
        throw cpo::uno::RuntimeException(u"replaceWithImageCropped: not implemented"_ustr);
    }

private:
    cpo::uno::Reference<css::drawing::XDrawPage> page_;
    cpo::uno::Reference<css::drawing::XShape> shape_;
};

class SlideImpl : public EditablePageStub<scriptinterop::XSlide>
{
public:
    SlideImpl(cpo::uno::Reference<css::frame::XModel> const& model,
              cpo::uno::Reference<css::drawing::XDrawPage> const& page)
        : model_(model)
        , page_(page)
    {
    }

    cpo::uno::Reference<cpo::uno::XInterface> SAL_CALL getuno() override { return page_; }

    cpo::uno::Sequence<cpo::uno::Reference<scriptinterop::XShape>> SAL_CALL getShapes() override
    {
        std::vector<cpo::uno::Reference<scriptinterop::XShape>> shapes;
        cpo::uno::Reference<css::container::XIndexAccess> const access = page_;
        if (access.is())
        {
            auto const n = access->getCount();
            for (sal_Int32 i = 0; i != n; ++i)
            {
                cpo::uno::Reference<css::drawing::XShape> shape;
                access->getByIndex(i) >>= shape;
                if (shape.is())
                {
                    shapes.emplace_back(new ShapeImpl(page_, shape));
                }
            }
        }
        return cpo::uno::Sequence<cpo::uno::Reference<scriptinterop::XShape>>(shapes.data(),
                                                                              shapes.size());
    }

    cpo::uno::Reference<scriptinterop::XShape> SAL_CALL
    insertTextBox(OUString const& text) override
    {
        // A text box inserted without geometry lands at the page's top left corner with the GAS
        // API's default square size: 3000000 English Metric Units, at 12700 to the point, or
        // 236.22 points.
        double const defaultExtent = 3000000.0 / 12700.0;
        return insertTextBoxAt(text, 0.0, 0.0, defaultExtent, defaultExtent);
    }

    // The geometry is converted up front, so a bad value fails before the slide is touched.
    cpo::uno::Reference<scriptinterop::XShape> SAL_CALL insertTextBoxAt(OUString const& text,
                                                                        double left, double top,
                                                                        double width,
                                                                        double height) override
    {
        css::awt::Point const position(pointsToHundredthMm(left), pointsToHundredthMm(top));
        css::awt::Size const size(extentToHundredthMm(width), extentToHundredthMm(height));
        cpo::uno::Reference<css::lang::XMultiServiceFactory> const factory(model_,
                                                                           cpo::uno::UNO_QUERY);
        if (!factory.is())
        {
            throw cpo::uno::RuntimeException(u"insertTextBox: no shape factory"_ustr);
        }
        cpo::uno::Reference<css::drawing::XShape> const shape(
            factory->createInstance(u"com.sun.star.drawing.TextShape"_ustr),
            cpo::uno::UNO_QUERY_THROW);
        cpo::uno::Reference<css::drawing::XShapes> const shapes(page_, cpo::uno::UNO_QUERY_THROW);
        // The shape only gets its edit engine when it enters the page, so the text is set after
        // the add:
        shapes->add(shape);
        // The box keeps the requested height; long text overflows instead of resizing the shape:
        cpo::uno::Reference<css::beans::XPropertySet> const props(shape,
                                                                  cpo::uno::UNO_QUERY_THROW);
        props->setPropertyValue(u"TextAutoGrowHeight"_ustr, cpo::uno::Any(false));
        cpo::uno::Reference<css::text::XText> const shapeText(shape, cpo::uno::UNO_QUERY_THROW);
        shapeText->setString(text);
        // The geometry goes in last, after the text, so no text-driven resize can override it:
        shape->setPosition(position);
        shape->setSize(size);
        return new ShapeImpl(page_, shape);
    }

    cpo::uno::Reference<scriptinterop::XSlide>
        SAL_CALL setBackgroundColor(OUString const& hexColor) override
    {
        auto const color = parseHexColor(hexColor);
        cpo::uno::Reference<css::lang::XMultiServiceFactory> const factory(model_,
                                                                           cpo::uno::UNO_QUERY);
        if (!factory.is())
        {
            throw cpo::uno::RuntimeException(u"setBackgroundColor: no background factory"_ustr);
        }
        // The page's Background property takes a property set created by the document model.  A
        // solid fill stored on the slide itself wins over whatever the master slide would paint.
        cpo::uno::Reference<css::beans::XPropertySet> const background(
            factory->createInstance(u"com.sun.star.drawing.Background"_ustr),
            cpo::uno::UNO_QUERY_THROW);
        background->setPropertyValue(u"FillStyle"_ustr,
                                     cpo::uno::Any(css::drawing::FillStyle_SOLID));
        background->setPropertyValue(u"FillColor"_ustr, cpo::uno::Any(color));
        cpo::uno::Reference<css::beans::XPropertySet> const pageProps(page_,
                                                                      cpo::uno::UNO_QUERY_THROW);
        pageProps->setPropertyValue(u"Background"_ustr, cpo::uno::Any(background));
        return this;
    }

    void SAL_CALL remove() override
    {
        cpo::uno::Reference<css::drawing::XDrawPagesSupplier> const sup(model_,
                                                                        cpo::uno::UNO_QUERY_THROW);
        auto const pages = sup->getDrawPages();
        // A presentation always keeps at least one slide, so removing the last one is an error
        // rather than a silent no-op:
        if (pages->getCount() <= 1)
        {
            throw cpo::uno::RuntimeException(u"remove: cannot remove the only slide"_ustr);
        }
        pages->remove(page_);
    }

    cpo::uno::Reference<scriptinterop::XSlide> SAL_CALL duplicate() override
    {
        throw cpo::uno::RuntimeException(u"duplicate: not implemented"_ustr);
    }

    cpo::uno::Reference<scriptinterop::XLayout> SAL_CALL getLayout() override
    {
        throw cpo::uno::RuntimeException(u"getLayout: not implemented"_ustr);
    }

    cpo::uno::Reference<scriptinterop::XNotesPage> SAL_CALL getNotesPage() override
    {
        throw cpo::uno::RuntimeException(u"getNotesPage: not implemented"_ustr);
    }

    scriptinterop::SlideLinkingMode SAL_CALL getSlideLinkingMode() override
    {
        throw cpo::uno::RuntimeException(u"getSlideLinkingMode: not implemented"_ustr);
    }

    bool SAL_CALL isSkipped() override
    {
        throw cpo::uno::RuntimeException(u"isSkipped: not implemented"_ustr);
    }

    void SAL_CALL move(sal_Int32) override
    {
        throw cpo::uno::RuntimeException(u"move: not implemented"_ustr);
    }

    void SAL_CALL setSkipped(bool) override
    {
        throw cpo::uno::RuntimeException(u"setSkipped: not implemented"_ustr);
    }

private:
    cpo::uno::Reference<css::frame::XModel> model_;
    cpo::uno::Reference<css::drawing::XDrawPage> page_;
};

class PageImpl : public EditablePageStub<scriptinterop::XPage>
{
public:
    PageImpl(cpo::uno::Reference<css::frame::XModel> const& model,
             cpo::uno::Reference<css::drawing::XDrawPage> const& page)
        : model_(model)
        , page_(page)
    {
    }

    cpo::uno::Reference<cpo::uno::XInterface> SAL_CALL getuno() override { return page_; }

    cpo::uno::Reference<scriptinterop::XLayout> SAL_CALL asLayout() override
    {
        throw cpo::uno::RuntimeException(u"asLayout: not implemented"_ustr);
    }

    cpo::uno::Reference<scriptinterop::XMaster> SAL_CALL asMaster() override
    {
        throw cpo::uno::RuntimeException(u"asMaster: not implemented"_ustr);
    }

    cpo::uno::Reference<scriptinterop::XSlide> SAL_CALL asSlide() override
    {
        // Only a page in the presentation's slide container is a slide; a notes, handout or
        // master page is not:
        if (!isSlide(model_, page_))
        {
            throw cpo::uno::RuntimeException(u"asSlide: the page is not a slide"_ustr);
        }
        return new SlideImpl(model_, page_);
    }

private:
    cpo::uno::Reference<css::frame::XModel> model_;
    cpo::uno::Reference<css::drawing::XDrawPage> page_;
};

class SlideSelectionImpl : public cppu::WeakImplHelper<scriptinterop::XSlideSelection>
{
public:
    explicit SlideSelectionImpl(cpo::uno::Reference<css::frame::XModel> const& model)
        : model_(model)
    {
    }

    cpo::uno::Reference<cpo::uno::XInterface> SAL_CALL getuno() override
    {
        auto const controller = model_->getCurrentController();
        if (!controller.is())
        {
            throw cpo::uno::RuntimeException(u"getuno: the presentation has no view"_ustr);
        }
        return controller;
    }

    css::beans::Optional<cpo::uno::Reference<scriptinterop::XPage>> SAL_CALL getCurrentPage()
        override
    {
        cpo::uno::Reference<css::drawing::XDrawView> const view(model_->getCurrentController(),
                                                                cpo::uno::UNO_QUERY);
        // Each view reports its own page kind as current: the normal drawing view a slide, the
        // notes, handout and master views their own page kind.
        auto const page = view.is() ? view->getCurrentPage() : nullptr;
        if (!page.is())
        {
            return {false, {}};
        }
        return {true, new PageImpl(model_, page)};
    }

    cpo::uno::Reference<scriptinterop::XPageElementRange> SAL_CALL getPageElementRange() override
    {
        throw cpo::uno::RuntimeException(u"getPageElementRange: not implemented"_ustr);
    }

    cpo::uno::Reference<scriptinterop::XPageRange> SAL_CALL getPageRange() override
    {
        throw cpo::uno::RuntimeException(u"getPageRange: not implemented"_ustr);
    }

    scriptinterop::SelectionType SAL_CALL getSelectionType() override
    {
        throw cpo::uno::RuntimeException(u"getSelectionType: not implemented"_ustr);
    }

    cpo::uno::Reference<scriptinterop::XSlideTableCellRange> SAL_CALL getTableCellRange() override
    {
        throw cpo::uno::RuntimeException(u"getTableCellRange: not implemented"_ustr);
    }

    cpo::uno::Reference<scriptinterop::XTextRange> SAL_CALL getTextRange() override
    {
        throw cpo::uno::RuntimeException(u"getTextRange: not implemented"_ustr);
    }

private:
    cpo::uno::Reference<css::frame::XModel> model_;
};

class PresentationImpl : public cppu::WeakImplHelper<scriptinterop::XPresentation>
{
public:
    explicit PresentationImpl(cpo::uno::Reference<css::frame::XModel> const& model)
        : model_(model)
    {
    }

    cpo::uno::Reference<cpo::uno::XInterface> SAL_CALL getuno() override { return model_; }

    cpo::uno::Reference<scriptinterop::XSlide> SAL_CALL appendSlide() override
    {
        return appendSlideWithLayout(AUTOLAYOUT_NONE);
    }

    cpo::uno::Reference<scriptinterop::XSlide> SAL_CALL
    appendSlideFrom(cpo::uno::Any const& layoutOrSlide) override
    {
        // The argument is checked before the slide is created, so a rejected call leaves the
        // presentation unchanged.
        scriptinterop::PredefinedLayout predefined;
        if (!(layoutOrSlide >>= predefined))
        {
            throw cpo::uno::RuntimeException(
                u"appendSlide with a layout argument that is not a predefined layout: not "
                "implemented"_ustr);
        }
        return appendSlideWithLayout(predefinedAutoLayout(predefined));
    }

    cpo::uno::Reference<scriptinterop::XSlide> SAL_CALL
    appendSlideLinked(cpo::uno::Reference<scriptinterop::XSlide> const&,
                      scriptinterop::SlideLinkingMode) override
    {
        throw cpo::uno::RuntimeException(
            u"appendSlide with a slide linking mode: not implemented"_ustr);
    }

    cpo::uno::Sequence<cpo::uno::Reference<scriptinterop::XLayout>> SAL_CALL getLayouts() override
    {
        throw cpo::uno::RuntimeException(u"getLayouts: not implemented"_ustr);
    }

    cpo::uno::Sequence<cpo::uno::Reference<scriptinterop::XMaster>> SAL_CALL getMasters() override
    {
        throw cpo::uno::RuntimeException(u"getMasters: not implemented"_ustr);
    }

    OUString SAL_CALL getName() override
    {
        throw cpo::uno::RuntimeException(u"getName: not implemented"_ustr);
    }

    cpo::uno::Reference<scriptinterop::XNotesMaster> SAL_CALL getNotesMaster() override
    {
        throw cpo::uno::RuntimeException(u"getNotesMaster: not implemented"_ustr);
    }

    double SAL_CALL getNotesPageHeight() override
    {
        throw cpo::uno::RuntimeException(u"getNotesPageHeight: not implemented"_ustr);
    }

    double SAL_CALL getNotesPageWidth() override
    {
        throw cpo::uno::RuntimeException(u"getNotesPageWidth: not implemented"_ustr);
    }

    cpo::uno::Reference<scriptinterop::XPageElement> SAL_CALL
    getPageElementById(OUString const&) override
    {
        throw cpo::uno::RuntimeException(u"getPageElementById: not implemented"_ustr);
    }

    double SAL_CALL getPageHeight() override { return pageSizePoints(u"Height"_ustr); }

    double SAL_CALL getPageWidth() override { return pageSizePoints(u"Width"_ustr); }

    css::beans::Optional<cpo::uno::Reference<scriptinterop::XSlideSelection>> SAL_CALL
    getSelection() override
    {
        return {true, new SlideSelectionImpl(model_)};
    }

    cpo::uno::Reference<scriptinterop::XSlide> SAL_CALL getSlideById(OUString const&) override
    {
        throw cpo::uno::RuntimeException(u"getSlideById: not implemented"_ustr);
    }

    cpo::uno::Sequence<cpo::uno::Reference<scriptinterop::XSlide>> SAL_CALL getSlides() override
    {
        std::vector<cpo::uno::Reference<scriptinterop::XSlide>> slides;
        auto const pages = drawPages();
        auto const n = pages->getCount();
        for (sal_Int32 i = 0; i != n; ++i)
        {
            cpo::uno::Reference<css::drawing::XDrawPage> page;
            pages->getByIndex(i) >>= page;
            if (page.is())
            {
                slides.emplace_back(new SlideImpl(model_, page));
            }
        }
        return cpo::uno::Sequence<cpo::uno::Reference<scriptinterop::XSlide>>(slides.data(),
                                                                              slides.size());
    }

    cpo::uno::Reference<scriptinterop::XSlide> SAL_CALL insertSlide(sal_Int32) override
    {
        throw cpo::uno::RuntimeException(u"insertSlide: not implemented"_ustr);
    }

    cpo::uno::Reference<scriptinterop::XSlide> SAL_CALL
    insertSlideFrom(sal_Int32, cpo::uno::Any const&) override
    {
        throw cpo::uno::RuntimeException(u"insertSlideFrom: not implemented"_ustr);
    }

    cpo::uno::Reference<scriptinterop::XSlide> SAL_CALL
    insertSlideLinked(sal_Int32, cpo::uno::Reference<scriptinterop::XSlide> const&,
                      scriptinterop::SlideLinkingMode) override
    {
        throw cpo::uno::RuntimeException(u"insertSlideLinked: not implemented"_ustr);
    }

    sal_Int32 SAL_CALL replaceAllText(OUString const&, OUString const&) override
    {
        throw cpo::uno::RuntimeException(u"replaceAllText: not implemented"_ustr);
    }

    sal_Int32 SAL_CALL replaceAllTextMatchCase(OUString const&, OUString const&, bool) override
    {
        throw cpo::uno::RuntimeException(u"replaceAllTextMatchCase: not implemented"_ustr);
    }

private:
    cpo::uno::Reference<scriptinterop::XSlide> appendSlideWithLayout(sal_Int16 autoLayout)
    {
        auto const pages = drawPages();
        // Inserting at getCount() appends; the new page is blank, without layout placeholders.
        auto const page = pages->insertNewByIndex(pages->getCount());
        if (autoLayout != AUTOLAYOUT_NONE)
        {
            // Setting the page's Layout property creates the layout's placeholder shapes.
            cpo::uno::Reference<css::beans::XPropertySet> const props(page,
                                                                      cpo::uno::UNO_QUERY_THROW);
            props->setPropertyValue(u"Layout"_ustr, cpo::uno::Any(autoLayout));
        }
        return new SlideImpl(model_, page);
    }

    // Reads the size property of the first slide.  The drawing layer stores it as a 1/100 mm
    // integer.
    double pageSizePoints(OUString const& propertyName)
    {
        cpo::uno::Reference<css::drawing::XDrawPage> page;
        drawPages()->getByIndex(0) >>= page;
        cpo::uno::Reference<css::beans::XPropertySet> const props(page,
                                                                  cpo::uno::UNO_QUERY_THROW);
        sal_Int32 sizeHundredthMm = 0;
        props->getPropertyValue(propertyName) >>= sizeHundredthMm;
        return hundredthMmToPoints(sizeHundredthMm);
    }

    cpo::uno::Reference<css::drawing::XDrawPages> drawPages()
    {
        cpo::uno::Reference<css::drawing::XDrawPagesSupplier> const sup(model_,
                                                                        cpo::uno::UNO_QUERY_THROW);
        return sup->getDrawPages();
    }

    cpo::uno::Reference<css::frame::XModel> model_;
};
}

namespace scriptinterop::detail
{
cpo::uno::Reference<scriptinterop::XPresentation>
createPresentation(cpo::uno::Reference<css::frame::XModel> const& model)
{
    return new PresentationImpl(model);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
