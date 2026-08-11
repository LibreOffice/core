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

#include <cmath>
#include <vector>

#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/awt/FontStrikeout.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/awt/Size.hpp>
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
#include <com/sun/star/uno/Reference.hxx>
#include <cpo/uno/RuntimeException.hpp>
#include <com/sun/star/uno/XInterface.hpp>
#include <cpo/uno/Any.hxx>
#include <cpo/uno/Sequence.hxx>
#include <cppuhelper/implbase.hxx>
#include <o3tl/string_view.hxx>
#include <rtl/ustring.hxx>
#include <sal/config.h>
#include <sal/types.h>
#include <scriptinterop/XPage.hpp>
#include <scriptinterop/XPresentation.hpp>
#include <scriptinterop/XShape.hpp>
#include <scriptinterop/XSlide.hpp>
#include <scriptinterop/XSlideSelection.hpp>
#include <scriptinterop/XTextParagraph.hpp>
#include <scriptinterop/XTextRange.hpp>
#include <scriptinterop/XTextStyle.hpp>

#include "presentation.hxx"

namespace
{
// The API works in points; the UNO drawing layer works in 1/100 mm.  A value that is not a
// finite number, or that falls outside the drawing layer's integer range after the conversion,
// is an error.  The comparison is written so that a NaN input fails it too.
sal_Int32 pointsToHundredthMm(double points)
{
    auto const hundredthMm = std::round(points * 2540.0 / 72.0);
    if (!(hundredthMm >= SAL_MIN_INT32 && hundredthMm <= SAL_MAX_INT32))
    {
        throw cpo::uno::RuntimeException(
            u"expected a length in points that fits the page coordinate range, got "_ustr
            + OUString::number(points));
    }
    return static_cast<sal_Int32>(hundredthMm);
}

// A shape width or height in points.  The value must not be negative; the comparison is written
// so that a NaN input fails it too.
sal_Int32 extentToHundredthMm(double points)
{
    if (!(points >= 0))
    {
        throw cpo::uno::RuntimeException(u"expected a non-negative size in points, got "_ustr
                                         + OUString::number(points));
    }
    return pointsToHundredthMm(points);
}

double hundredthMmToPoints(sal_Int32 hundredthMm) { return hundredthMm * 72.0 / 2540.0; }

// Formatting is applied through a cursor, so it lands on the text runs themselves and survives
// saving.  With a range the cursor spans just that range; without one it spans the whole text.
css::uno::Reference<css::beans::XPropertySet>
cursorProperties(css::uno::Reference<css::text::XText> const& text,
                 css::uno::Reference<css::text::XTextRange> const& range)
{
    css::uno::Reference<css::text::XTextCursor> cursor;
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
    return css::uno::Reference<css::beans::XPropertySet>(cursor, css::uno::UNO_QUERY_THROW);
}

sal_Int32 parseHexColor(OUString const& hexColor)
{
    bool valid = hexColor.getLength() == 7 && hexColor[0] == '#';
    if (valid)
    {
        for (sal_Int32 i = 1; i != 7; ++i)
        {
            auto const c = hexColor[i];
            if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F')))
            {
                valid = false;
                break;
            }
        }
    }
    if (!valid)
    {
        throw cpo::uno::RuntimeException(u"expected a color in \"#rrggbb\" form, got "_ustr
                                         + hexColor);
    }
    return static_cast<sal_Int32>(o3tl::toUInt32(hexColor.subView(1), 16));
}

// A page counts as one of the presentation's slides when the model's slide container holds it.
// Notes, handout and master pages live in other containers, so they do not count.
bool isSlide(css::uno::Reference<css::frame::XModel> const& model,
             css::uno::Reference<css::drawing::XDrawPage> const& page)
{
    css::uno::Reference<css::drawing::XDrawPagesSupplier> const sup(model, css::uno::UNO_QUERY);
    if (!sup.is())
    {
        return false;
    }
    auto const pages = sup->getDrawPages();
    auto const n = pages->getCount();
    for (sal_Int32 i = 0; i != n; ++i)
    {
        css::uno::Reference<css::drawing::XDrawPage> candidate;
        pages->getByIndex(i) >>= candidate;
        if (candidate == page)
        {
            return true;
        }
    }
    return false;
}

class TextStyleImpl : public cppu::WeakImplHelper<scriptinterop::XTextStyle>
{
public:
    TextStyleImpl(css::uno::Reference<css::text::XText> const& text,
                  css::uno::Reference<css::text::XTextRange> const& range)
        : text_(text)
        , range_(range)
    {
    }

    css::uno::Reference<css::uno::XInterface> SAL_CALL getuno() override
    {
        if (range_.is())
        {
            return range_;
        }
        return text_;
    }

    css::uno::Reference<scriptinterop::XTextStyle> SAL_CALL setBold(bool bold) override
    {
        auto const props = cursorProperties(text_, range_);
        auto const weight = bold ? css::awt::FontWeight::BOLD : css::awt::FontWeight::NORMAL;
        props->setPropertyValue(u"CharWeight"_ustr, cpo::uno::Any(weight));
        props->setPropertyValue(u"CharWeightAsian"_ustr, cpo::uno::Any(weight));
        props->setPropertyValue(u"CharWeightComplex"_ustr, cpo::uno::Any(weight));
        return this;
    }

    css::uno::Reference<scriptinterop::XTextStyle> SAL_CALL setItalic(bool italic) override
    {
        auto const props = cursorProperties(text_, range_);
        auto const slant = italic ? css::awt::FontSlant_ITALIC : css::awt::FontSlant_NONE;
        props->setPropertyValue(u"CharPosture"_ustr, cpo::uno::Any(slant));
        props->setPropertyValue(u"CharPostureAsian"_ustr, cpo::uno::Any(slant));
        props->setPropertyValue(u"CharPostureComplex"_ustr, cpo::uno::Any(slant));
        return this;
    }

    css::uno::Reference<scriptinterop::XTextStyle>
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

    css::uno::Reference<scriptinterop::XTextStyle> SAL_CALL setFontSize(double points) override
    {
        auto const props = cursorProperties(text_, range_);
        // CharHeight is measured in points, so the value passes through unconverted:
        auto const height = static_cast<float>(points);
        props->setPropertyValue(u"CharHeight"_ustr, cpo::uno::Any(height));
        props->setPropertyValue(u"CharHeightAsian"_ustr, cpo::uno::Any(height));
        props->setPropertyValue(u"CharHeightComplex"_ustr, cpo::uno::Any(height));
        return this;
    }

    css::uno::Reference<scriptinterop::XTextStyle>
        SAL_CALL setForegroundColor(OUString const& hexColor) override
    {
        cursorProperties(text_, range_)
            ->setPropertyValue(u"CharColor"_ustr, cpo::uno::Any(parseHexColor(hexColor)));
        return this;
    }

private:
    css::uno::Reference<css::text::XText> text_;
    css::uno::Reference<css::text::XTextRange> range_;
};

class TextParagraphImpl : public cppu::WeakImplHelper<scriptinterop::XTextParagraph>
{
public:
    explicit TextParagraphImpl(css::uno::Reference<scriptinterop::XTextRange> const& range)
        : range_(range)
    {
    }

    css::uno::Reference<css::uno::XInterface> SAL_CALL getuno() override
    {
        return range_->getuno();
    }

    css::uno::Reference<scriptinterop::XTextRange> SAL_CALL getRange() override
    {
        return range_;
    }

private:
    css::uno::Reference<scriptinterop::XTextRange> range_;
};

class TextRangeImpl : public cppu::WeakImplHelper<scriptinterop::XTextRange>
{
public:
    explicit TextRangeImpl(css::uno::Reference<css::text::XText> const& text)
        : text_(text)
    {
    }

    TextRangeImpl(css::uno::Reference<css::text::XText> const& text,
                  css::uno::Reference<css::text::XTextRange> const& range)
        : text_(text)
        , range_(range)
    {
    }

    css::uno::Reference<css::uno::XInterface> SAL_CALL getuno() override
    {
        if (range_.is())
        {
            return range_;
        }
        return text_;
    }

    OUString SAL_CALL asString() override
    {
        if (range_.is())
        {
            return range_->getString();
        }
        return text_->getString();
    }

    css::uno::Reference<scriptinterop::XTextRange> SAL_CALL setText(OUString const& newText)
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

    css::uno::Reference<scriptinterop::XTextRange> SAL_CALL appendText(OUString const& text)
        override
    {
        if (range_.is())
        {
            throw cpo::uno::RuntimeException(
                u"appendText: only the shape's whole text range can append"_ustr);
        }
        css::uno::Reference<css::text::XTextPortionAppend> const append(text_,
                                                                        css::uno::UNO_QUERY);
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

    css::uno::Reference<scriptinterop::XTextParagraph> SAL_CALL
    appendParagraph(OUString const& text) override
    {
        if (range_.is())
        {
            throw cpo::uno::RuntimeException(
                u"appendParagraph: only the shape's whole text range can append"_ustr);
        }
        css::uno::Reference<css::text::XParagraphAppend> const append(text_,
                                                                      css::uno::UNO_QUERY);
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

    css::uno::Reference<scriptinterop::XTextRange> SAL_CALL setBulletLevel(sal_Int32 level)
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

    css::uno::Reference<scriptinterop::XTextStyle> SAL_CALL getTextStyle() override
    {
        // Character formatting lives on the text runs, so an empty range holds none to style.
        if (asString().isEmpty())
        {
            throw cpo::uno::RuntimeException(
                u"getTextStyle: the range is empty and has no characters to style"_ustr);
        }
        return new TextStyleImpl(text_, range_);
    }

private:
    css::uno::Reference<css::text::XText> text_;
    css::uno::Reference<css::text::XTextRange> range_;
};

class ShapeImpl : public cppu::WeakImplHelper<scriptinterop::XShape>
{
public:
    ShapeImpl(css::uno::Reference<css::drawing::XDrawPage> const& page,
              css::uno::Reference<css::drawing::XShape> const& shape)
        : page_(page)
        , shape_(shape)
    {
    }

    css::uno::Reference<css::uno::XInterface> SAL_CALL getuno() override { return shape_; }

    css::uno::Reference<scriptinterop::XTextRange> SAL_CALL getText() override
    {
        css::uno::Reference<css::text::XText> const text(shape_, css::uno::UNO_QUERY);
        if (!text.is())
        {
            throw cpo::uno::RuntimeException(u"getText: shape cannot hold text"_ustr);
        }
        return new TextRangeImpl(text);
    }

    double SAL_CALL getLeft() override { return hundredthMmToPoints(shape_->getPosition().X); }

    double SAL_CALL getTop() override { return hundredthMmToPoints(shape_->getPosition().Y); }

    double SAL_CALL getWidth() override { return hundredthMmToPoints(shape_->getSize().Width); }

    double SAL_CALL getHeight() override { return hundredthMmToPoints(shape_->getSize().Height); }

    css::uno::Reference<scriptinterop::XShape> SAL_CALL setLeft(double points) override
    {
        auto pos = shape_->getPosition();
        pos.X = pointsToHundredthMm(points);
        shape_->setPosition(pos);
        return this;
    }

    css::uno::Reference<scriptinterop::XShape> SAL_CALL setTop(double points) override
    {
        auto pos = shape_->getPosition();
        pos.Y = pointsToHundredthMm(points);
        shape_->setPosition(pos);
        return this;
    }

    css::uno::Reference<scriptinterop::XShape> SAL_CALL setWidth(double points) override
    {
        auto size = shape_->getSize();
        size.Width = extentToHundredthMm(points);
        shape_->setSize(size);
        return this;
    }

    css::uno::Reference<scriptinterop::XShape> SAL_CALL setHeight(double points) override
    {
        auto size = shape_->getSize();
        size.Height = extentToHundredthMm(points);
        shape_->setSize(size);
        return this;
    }

    void SAL_CALL remove() override
    {
        css::uno::Reference<css::drawing::XShapes> const shapes(page_, css::uno::UNO_QUERY_THROW);
        shapes->remove(shape_);
    }

private:
    css::uno::Reference<css::drawing::XDrawPage> page_;
    css::uno::Reference<css::drawing::XShape> shape_;
};

class SlideImpl : public cppu::WeakImplHelper<scriptinterop::XSlide>
{
public:
    SlideImpl(css::uno::Reference<css::frame::XModel> const& model,
              css::uno::Reference<css::drawing::XDrawPage> const& page)
        : model_(model)
        , page_(page)
    {
    }

    css::uno::Reference<css::uno::XInterface> SAL_CALL getuno() override { return page_; }

    cpo::uno::Sequence<css::uno::Reference<scriptinterop::XShape>> SAL_CALL getShapes() override
    {
        std::vector<css::uno::Reference<scriptinterop::XShape>> shapes;
        css::uno::Reference<css::container::XIndexAccess> const access = page_;
        if (access.is())
        {
            auto const n = access->getCount();
            for (sal_Int32 i = 0; i != n; ++i)
            {
                css::uno::Reference<css::drawing::XShape> shape;
                access->getByIndex(i) >>= shape;
                if (shape.is())
                {
                    shapes.emplace_back(new ShapeImpl(page_, shape));
                }
            }
        }
        return cpo::uno::Sequence<css::uno::Reference<scriptinterop::XShape>>(shapes.data(),
                                                                              shapes.size());
    }

    css::uno::Reference<scriptinterop::XShape>
        SAL_CALL insertTextBox(OUString const& text, double left, double top, double width,
                               double height) override
    {
        // The whole geometry is converted up front, so a bad value fails before the slide is
        // touched:
        css::awt::Point const position(pointsToHundredthMm(left), pointsToHundredthMm(top));
        css::awt::Size const size(extentToHundredthMm(width), extentToHundredthMm(height));
        css::uno::Reference<css::lang::XMultiServiceFactory> const factory(model_,
                                                                           css::uno::UNO_QUERY);
        if (!factory.is())
        {
            throw cpo::uno::RuntimeException(u"insertTextBox: no shape factory"_ustr);
        }
        css::uno::Reference<css::drawing::XShape> const shape(
            factory->createInstance(u"com.sun.star.drawing.TextShape"_ustr),
            css::uno::UNO_QUERY_THROW);
        css::uno::Reference<css::drawing::XShapes> const shapes(page_, css::uno::UNO_QUERY_THROW);
        // The shape only gets its edit engine when it enters the page, so the text is set after
        // the add:
        shapes->add(shape);
        // The box keeps the requested height; long text overflows instead of resizing the shape:
        css::uno::Reference<css::beans::XPropertySet> const props(shape,
                                                                  css::uno::UNO_QUERY_THROW);
        props->setPropertyValue(u"TextAutoGrowHeight"_ustr, cpo::uno::Any(false));
        css::uno::Reference<css::text::XText> const shapeText(shape, css::uno::UNO_QUERY_THROW);
        shapeText->setString(text);
        // The geometry goes in last, after the text, so no text-driven resize can override it:
        shape->setPosition(position);
        shape->setSize(size);
        return new ShapeImpl(page_, shape);
    }

    css::uno::Reference<scriptinterop::XSlide>
        SAL_CALL setBackgroundColor(OUString const& hexColor) override
    {
        auto const color = parseHexColor(hexColor);
        css::uno::Reference<css::lang::XMultiServiceFactory> const factory(model_,
                                                                           css::uno::UNO_QUERY);
        if (!factory.is())
        {
            throw cpo::uno::RuntimeException(u"setBackgroundColor: no background factory"_ustr);
        }
        // The page's Background property takes a property set created by the document model.  A
        // solid fill stored on the slide itself wins over whatever the master slide would paint.
        css::uno::Reference<css::beans::XPropertySet> const background(
            factory->createInstance(u"com.sun.star.drawing.Background"_ustr),
            css::uno::UNO_QUERY_THROW);
        background->setPropertyValue(u"FillStyle"_ustr,
                                     cpo::uno::Any(css::drawing::FillStyle_SOLID));
        background->setPropertyValue(u"FillColor"_ustr, cpo::uno::Any(color));
        css::uno::Reference<css::beans::XPropertySet> const pageProps(page_,
                                                                      css::uno::UNO_QUERY_THROW);
        pageProps->setPropertyValue(u"Background"_ustr, cpo::uno::Any(background));
        return this;
    }

    void SAL_CALL remove() override
    {
        css::uno::Reference<css::drawing::XDrawPagesSupplier> const sup(model_,
                                                                        css::uno::UNO_QUERY_THROW);
        auto const pages = sup->getDrawPages();
        // A presentation always keeps at least one slide, so removing the last one is an error
        // rather than a silent no-op:
        if (pages->getCount() <= 1)
        {
            throw cpo::uno::RuntimeException(u"remove: cannot remove the only slide"_ustr);
        }
        pages->remove(page_);
    }

private:
    css::uno::Reference<css::frame::XModel> model_;
    css::uno::Reference<css::drawing::XDrawPage> page_;
};

class PageImpl : public cppu::WeakImplHelper<scriptinterop::XPage>
{
public:
    PageImpl(css::uno::Reference<css::frame::XModel> const& model,
             css::uno::Reference<css::drawing::XDrawPage> const& page)
        : model_(model)
        , page_(page)
    {
    }

    css::uno::Reference<css::uno::XInterface> SAL_CALL getuno() override { return page_; }

    css::uno::Reference<scriptinterop::XSlide> SAL_CALL asSlide() override
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
    css::uno::Reference<css::frame::XModel> model_;
    css::uno::Reference<css::drawing::XDrawPage> page_;
};

class SlideSelectionImpl : public cppu::WeakImplHelper<scriptinterop::XSlideSelection>
{
public:
    explicit SlideSelectionImpl(css::uno::Reference<css::frame::XModel> const& model)
        : model_(model)
    {
    }

    css::uno::Reference<css::uno::XInterface> SAL_CALL getuno() override
    {
        auto const controller = model_->getCurrentController();
        if (!controller.is())
        {
            throw cpo::uno::RuntimeException(u"getuno: the presentation has no view"_ustr);
        }
        return controller;
    }

    css::uno::Reference<scriptinterop::XPage> SAL_CALL getCurrentPage() override
    {
        css::uno::Reference<css::drawing::XDrawView> const view(model_->getCurrentController(),
                                                                css::uno::UNO_QUERY);
        // Each view reports its own page kind as current: the normal drawing view a slide, the
        // notes, handout and master views their own page kind.
        auto const page = view.is() ? view->getCurrentPage() : nullptr;
        if (!page.is())
        {
            return nullptr;
        }
        return new PageImpl(model_, page);
    }

private:
    css::uno::Reference<css::frame::XModel> model_;
};

class PresentationImpl : public cppu::WeakImplHelper<scriptinterop::XPresentation>
{
public:
    explicit PresentationImpl(css::uno::Reference<css::frame::XModel> const& model)
        : model_(model)
    {
    }

    css::uno::Reference<css::uno::XInterface> SAL_CALL getuno() override { return model_; }

    cpo::uno::Sequence<css::uno::Reference<scriptinterop::XSlide>> SAL_CALL getSlides() override
    {
        std::vector<css::uno::Reference<scriptinterop::XSlide>> slides;
        auto const pages = drawPages();
        auto const n = pages->getCount();
        for (sal_Int32 i = 0; i != n; ++i)
        {
            css::uno::Reference<css::drawing::XDrawPage> page;
            pages->getByIndex(i) >>= page;
            if (page.is())
            {
                slides.emplace_back(new SlideImpl(model_, page));
            }
        }
        return cpo::uno::Sequence<css::uno::Reference<scriptinterop::XSlide>>(slides.data(),
                                                                              slides.size());
    }

    css::uno::Reference<scriptinterop::XSlide> SAL_CALL appendSlide() override
    {
        auto const pages = drawPages();
        // Inserting at getCount() appends; the new page is blank, without layout placeholders.
        auto const page = pages->insertNewByIndex(pages->getCount());
        return new SlideImpl(model_, page);
    }

    double SAL_CALL getPageWidth() override { return pageSizePoints(u"Width"_ustr); }

    double SAL_CALL getPageHeight() override { return pageSizePoints(u"Height"_ustr); }

    css::uno::Reference<scriptinterop::XSlideSelection> SAL_CALL getSelection() override
    {
        return new SlideSelectionImpl(model_);
    }

private:
    // Reads the size property of the first slide.  The drawing layer stores it as a 1/100 mm
    // integer.
    double pageSizePoints(OUString const& propertyName)
    {
        css::uno::Reference<css::drawing::XDrawPage> page;
        drawPages()->getByIndex(0) >>= page;
        css::uno::Reference<css::beans::XPropertySet> const props(page,
                                                                  css::uno::UNO_QUERY_THROW);
        sal_Int32 sizeHundredthMm = 0;
        props->getPropertyValue(propertyName) >>= sizeHundredthMm;
        return hundredthMmToPoints(sizeHundredthMm);
    }

    css::uno::Reference<css::drawing::XDrawPages> drawPages()
    {
        css::uno::Reference<css::drawing::XDrawPagesSupplier> const sup(model_,
                                                                        css::uno::UNO_QUERY_THROW);
        return sup->getDrawPages();
    }

    css::uno::Reference<css::frame::XModel> model_;
};
}

namespace scriptinterop::detail
{
css::uno::Reference<scriptinterop::XPresentation>
createPresentation(css::uno::Reference<css::frame::XModel> const& model)
{
    return new PresentationImpl(model);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
