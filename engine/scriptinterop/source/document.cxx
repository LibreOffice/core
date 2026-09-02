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
#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/graphic/XGraphicProvider.hpp>
#include <com/sun/star/io/XTempFile.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/text/WritingMode2.hpp>
#include <com/sun/star/text/XFootnote.hpp>
#include <com/sun/star/text/XFootnotesSupplier.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XTextContent.hpp>
#include <com/sun/star/text/XTextCursor.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/text/XTextRangeCompare.hpp>
#include <com/sun/star/text/XTextViewCursor.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <cpo/uno/RuntimeException.hpp>
#include <cpo/uno/XComponentContext.hpp>
#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <comphelper/processfactory.hxx>
#include <cpo/uno/Any.hxx>
#include <cpo/uno/Sequence.hxx>
#include <cppuhelper/implbase.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/ustring.hxx>
#include <sal/config.h>
#include <sal/log.hxx>
#include <sal/types.h>
#include <scriptinterop/ElementType.hpp>
#include <scriptinterop/ImageOptions.hpp>
#include <scriptinterop/XBody.hpp>
#include <scriptinterop/XCursor.hpp>
#include <scriptinterop/XDocument.hpp>
#include <scriptinterop/XFootnote.hpp>
#include <scriptinterop/XParagraph.hpp>
#include <scriptinterop/XRangeElement.hpp>
#include <scriptinterop/XSelection.hpp>
#include <scriptinterop/XTextRun.hpp>

#include "document.hxx"

namespace
{
class SelectionImpl : public cppu::WeakImplHelper<scriptinterop::XSelection>
{
public:
    explicit SelectionImpl(css::uno::Reference<css::container::XIndexAccess> const& ranges)
        : ranges_(ranges)
    {
    }

    css::uno::Reference<css::uno::XInterface> SAL_CALL getuno() override { return ranges_; }

    cpo::uno::Sequence<css::uno::Reference<scriptinterop::XRangeElement>> getRangeElements()
        override;

    OUString SAL_CALL getText() override
    {
        OUStringBuffer buf;
        if (ranges_.is())
        {
            auto const n = ranges_->getCount();
            for (sal_Int32 i = 0; i != n; ++i)
            {
                css::uno::Reference<css::text::XTextRange> range;
                ranges_->getByIndex(i) >>= range;
                if (range.is())
                {
                    if (!buf.isEmpty())
                    {
                        buf.append('\n');
                    }
                    buf.append(range->getString());
                }
            }
        }
        return buf.makeStringAndClear();
    }

    void SAL_CALL replace(OUString const& newText) override
    {
        if (!ranges_.is())
        {
            return;
        }
        auto const n = ranges_->getCount();
        for (sal_Int32 i = 0; i != n; ++i)
        {
            css::uno::Reference<css::text::XTextRange> range;
            ranges_->getByIndex(i) >>= range;
            if (range.is())
            {
                range->setString(newText);
            }
        }
    }

private:
    css::uno::Reference<css::container::XIndexAccess> ranges_;
};

class TextRunImpl: public cppu::WeakImplHelper<scriptinterop::XTextRun> {
public:
    explicit TextRunImpl(css::uno::Reference<css::text::XTextRange> const & range): range_(range) {}

    css::uno::Reference<css::uno::XInterface> getuno() override { return range_; }

    sal_Int16 getEscapement() override {
        sal_Int16 esc = 0;
        getProp(u"CharEscapement"_ustr) >>= esc;
        return esc > 0 ? 1 : esc < 0 ? -1 : 0;
    }

    OUString getFontFamily() override {
        OUString name;
        getProp(u"CharFontName"_ustr) >>= name;
        return name;
    }

    OUString getLinkUrl() override {
        OUString url;
        getProp(u"HyperLinkURL"_ustr) >>= url;
        return url;
    }

    OUString getText() override { return range_.is() ? range_->getString() : u""_ustr; }

    bool isBold() override {
        float weight = css::awt::FontWeight::NORMAL;
        getProp(u"CharWeight"_ustr) >>= weight;
        return weight >= css::awt::FontWeight::BOLD;
    }

    bool isItalic() override {
        css::awt::FontSlant slant = css::awt::FontSlant_NONE;
        getProp(u"CharPosture"_ustr) >>= slant;
        return slant == css::awt::FontSlant_ITALIC || slant == css::awt::FontSlant_OBLIQUE;
    }

    bool isStrikethrough() override {
        sal_Int16 strike = css::awt::FontStrikeout::NONE;
        getProp(u"CharStrikeout"_ustr) >>= strike;
        return strike != css::awt::FontStrikeout::NONE;
    }

    bool isUnderline() override {
        sal_Int16 underline = css::awt::FontUnderline::NONE;
        getProp(u"CharUnderline"_ustr) >>= underline;
        return underline != css::awt::FontUnderline::NONE;
    }

private:
    cpo::uno::Any getProp(OUString const & name) {
        css::uno::Reference<css::beans::XPropertySet> const props(range_, css::uno::UNO_QUERY);
        if (!props.is()) {
            return {};
        }
        auto const info(props->getPropertySetInfo());
        if (!info.is() || !info->hasPropertyByName(name)) {
            return {};
        }
        return props->getPropertyValue(name);
    }

    css::uno::Reference<css::text::XTextRange> range_;
};

class ParagraphImpl : public cppu::WeakImplHelper<scriptinterop::XParagraph>
{
public:
    explicit ParagraphImpl(css::uno::Reference<css::text::XTextContent> const& content)
        : content_(content)
    {
    }

    css::uno::Reference<css::uno::XInterface> SAL_CALL getuno() override { return content_; }

    scriptinterop::ElementType getElementType() override {
        css::uno::Reference<css::beans::XPropertySet> const props(content_, css::uno::UNO_QUERY);
        if (props.is()) {
            auto const info(props->getPropertySetInfo());
            if (info.is() && info->hasPropertyByName(u"NumberingIsNumber"_ustr)) {
                bool numbered = false;
                props->getPropertyValue(u"NumberingIsNumber"_ustr) >>= numbered;
                if (numbered) {
                    return scriptinterop::ElementType_LIST_ITEM;
                }
            }
        }
        return scriptinterop::ElementType_PARAGRAPH;
    }

    OUString SAL_CALL getText() override
    {
        css::uno::Reference<css::text::XTextRange> const range(content_, css::uno::UNO_QUERY);
        return range.is() ? range->getString() : OUString();
    }

    cpo::uno::Sequence<css::uno::Reference<scriptinterop::XTextRun>> getTextRuns() override {
        std::vector<css::uno::Reference<scriptinterop::XTextRun>> v;
        if (css::uno::Reference<css::container::XEnumerationAccess> const ea{
                content_, css::uno::UNO_QUERY})
        {
            auto const en = ea->createEnumeration();
            while (en.is() && en->hasMoreElements()) {
                css::uno::Reference<css::text::XTextRange> portion;
                en->nextElement() >>= portion;
                if (portion.is()) {
                    v.emplace_back(new TextRunImpl(portion));
                }
            }
        }
        return cpo::uno::Sequence(v.data(), v.size());
    }

    bool isLeftToRight() override {
        css::uno::Reference<css::beans::XPropertySet> const props(content_, css::uno::UNO_QUERY);
        if (!props.is()) {
            return true;
        }
        auto const info(props->getPropertySetInfo());
        if (!info.is() || !info->hasPropertyByName(u"WritingMode"_ustr)) {
            return true;
        }
        sal_Int16 mode = css::text::WritingMode2::LR_TB;
        props->getPropertyValue(u"WritingMode"_ustr) >>= mode;
        return mode != css::text::WritingMode2::RL_TB;
    }

private:
    css::uno::Reference<css::text::XTextContent> content_;
};

cpo::uno::Sequence<css::uno::Reference<scriptinterop::XParagraph>> enumerateParagraphs(
    css::uno::Reference<css::text::XText> const & text)
{
    std::vector<css::uno::Reference<scriptinterop::XParagraph>> v;
    if (css::uno::Reference<css::container::XEnumerationAccess> const ea{text, css::uno::UNO_QUERY})
    {
        auto const en = ea->createEnumeration();
        while (en.is() && en->hasMoreElements()) {
            css::uno::Reference<css::text::XTextContent> xtc;
            en->nextElement() >>= xtc;
            if (!xtc.is()) {
                continue;
            }
            css::uno::Reference<css::lang::XServiceInfo> const info(xtc, css::uno::UNO_QUERY);
            if (!info.is() || !info->supportsService(u"com.sun.star.text.Paragraph"_ustr)) {
                continue;
            }
            v.emplace_back(new ParagraphImpl(xtc));
        }
    }
    return cpo::uno::Sequence(v.data(), v.size());
}

// Walk the containing XText's paragraphs and return the one whose extent covers `marker`s start;
// null if the walk finds no paragraph or if the ranges live in different Text hosts:
css::uno::Reference<css::text::XTextContent> findContainingParagraph(
    css::uno::Reference<css::text::XTextRange> const & marker)
{
    if (!marker.is()) {
        return {};
    }
    auto const containingText = marker->getText();
    if (!containingText.is()) {
        return {};
    }
    css::uno::Reference<css::text::XTextRangeCompare> const cmp(
        containingText, css::uno::UNO_QUERY);
    if (!cmp.is()) {
        return {};
    }
    css::uno::Reference<css::container::XEnumerationAccess> const ea(
        containingText, css::uno::UNO_QUERY);
    if (!ea.is()) {
        return {};
    }
    auto const en = ea->createEnumeration();
    while (en.is() && en->hasMoreElements()) {
        css::uno::Reference<css::text::XTextContent> xtc;
        en->nextElement() >>= xtc;
        if (!xtc.is()) {
            continue;
        }
        css::uno::Reference<css::lang::XServiceInfo> const info(xtc, css::uno::UNO_QUERY);
        if (!info.is() || !info->supportsService(u"com.sun.star.text.Paragraph"_ustr)) {
            continue;
        }
        css::uno::Reference<css::text::XTextRange> const paraRange(xtc, css::uno::UNO_QUERY);
        if (!paraRange.is()) {
            continue;
        }
        try {
            if (cmp->compareRegionStarts(marker->getStart(), paraRange->getStart()) >= 0
                && cmp->compareRegionStarts(marker->getStart(), paraRange->getEnd()) <= 0)
            {
                return xtc;
            }
        } catch (css::lang::IllegalArgumentException const & e) {
            SAL_WARN(
                "scriptinterop",
                "findContainingParagraph: compareRegionStarts failed: " << e.Message);
        }
    }
    return {};
}

class RangeElementImpl: public cppu::WeakImplHelper<scriptinterop::XRangeElement> {
public:
    explicit RangeElementImpl(css::uno::Reference<css::text::XTextRange> const & range):
        range_(range), paragraph_(findContainingParagraph(range))
    {
        rangeLen_ = range.is() ? range->getString().getLength() : sal_Int32(0);
        if (!paragraph_.is()) {
            return;
        }
        css::uno::Reference<css::text::XTextRange> const paraRange(
            paragraph_, css::uno::UNO_QUERY);
        if (!paraRange.is()) {
            return;
        }
        paragraphLen_ = paraRange->getString().getLength();
        auto const host = paraRange->getText();
        if (!host.is()) {
            return;
        }
        auto const probe = host->createTextCursorByRange(paraRange->getStart());
        if (!probe.is()) {
            return;
        }
        probe->gotoRange(range->getStart(), true);
        startOffset_ = probe->getString().getLength();
    }

    css::uno::Reference<css::uno::XInterface> getuno() override { return range_; }

    css::uno::Reference<scriptinterop::XParagraph> getElement() override {
        return paragraph_.is() ? new ParagraphImpl(paragraph_) : nullptr;
    }

    sal_Int32 getEndOffsetInclusive() override {
        return rangeLen_ == 0 ? startOffset_ : startOffset_ + rangeLen_ - 1;
    }

    sal_Int32 getStartOffset() override { return startOffset_; }

    bool isPartial() override {
        if (!paragraph_.is()) {
            return true;
        }
        return startOffset_ != 0 || startOffset_ + rangeLen_ != paragraphLen_;
    }

private:
    css::uno::Reference<css::text::XTextRange> range_;
    css::uno::Reference<css::text::XTextContent> paragraph_;
    sal_Int32 startOffset_ = 0;
    sal_Int32 rangeLen_ = 0;
    sal_Int32 paragraphLen_ = 0;
};

cpo::uno::Sequence<css::uno::Reference<scriptinterop::XRangeElement>>
SelectionImpl::getRangeElements() {
    std::vector<css::uno::Reference<scriptinterop::XRangeElement>> v;
    if (ranges_.is()) {
        auto const n = ranges_->getCount();
        for (sal_Int32 i = 0; i != n; ++i) {
            css::uno::Reference<css::text::XTextRange> range;
            ranges_->getByIndex(i) >>= range;
            if (range.is()) {
                v.emplace_back(new RangeElementImpl(range));
            }
        }
    }
    return cpo::uno::Sequence(v.data(), v.size());
}

class CursorImpl: public cppu::WeakImplHelper<scriptinterop::XCursor> {
public:
    explicit CursorImpl(css::uno::Reference<css::frame::XModel> const & model): model_(model) {}

    css::uno::Reference<css::uno::XInterface> getuno() override { return viewCursor(); }

    css::uno::Reference<scriptinterop::XParagraph> getElement() override {
        auto const c = viewCursor();
        if (!c.is()) {
            return {};
        }
        auto const para = findContainingParagraph(c);
        return para.is() ? new ParagraphImpl(para) : nullptr;
    }

    sal_Int32 getOffset() override {
        auto const c = viewCursor();
        auto const para = findContainingParagraph(c);
        if (!para.is()) {
            return 0;
        }
        css::uno::Reference<css::text::XTextRange> const paraRange(para, css::uno::UNO_QUERY);
        if (!paraRange.is()) {
            return 0;
        }
        auto const host = paraRange->getText();
        if (!host.is()) {
            return 0;
        }
        auto const probe = host->createTextCursorByRange(paraRange->getStart());
        if (!probe.is()) {
            return 0;
        }
        probe->gotoRange(c->getStart(), true);
        return probe->getString().getLength();
    }

    OUString getSurroundingText() override {
        auto const c = viewCursor();
        auto const para = findContainingParagraph(c);
        if (!para.is()) {
            return u""_ustr;
        }
        css::uno::Reference<css::text::XTextRange> const range(para, css::uno::UNO_QUERY);
        return range.is() ? range->getString() : u""_ustr;
    }

    void insertText(OUString const & text) override {
        auto const c = viewCursor();
        if (!c.is()) {
            return;
        }
        auto const host = c->getText();
        if (!host.is()) {
            return;
        }
        host->insertString(c->getStart(), text, false);
    }

private:
    css::uno::Reference<css::text::XTextViewCursor> viewCursor() {
        css::uno::Reference<css::text::XTextViewCursorSupplier> const sup(
            model_->getCurrentController(), css::uno::UNO_QUERY);
        return sup.is() ? sup->getViewCursor() : css::uno::Reference<css::text::XTextViewCursor>();
    }

    css::uno::Reference<css::frame::XModel> model_;
};

class FootnoteImpl: public cppu::WeakImplHelper<scriptinterop::XFootnote> {
public:
    explicit FootnoteImpl(css::uno::Reference<css::text::XFootnote> const & footnote):
        footnote_(footnote) {}

    css::uno::Reference<css::uno::XInterface> getuno() override { return footnote_; }

    cpo::uno::Sequence<css::uno::Reference<scriptinterop::XParagraph>> getParagraphs() override {
        return enumerateParagraphs(
            css::uno::Reference<css::text::XText>(footnote_, css::uno::UNO_QUERY));
    }

private:
    css::uno::Reference<css::text::XFootnote> footnote_;
};

class BodyImpl: public cppu::WeakImplHelper<scriptinterop::XBody> {
public:
    explicit BodyImpl(css::uno::Reference<css::text::XText> const & text): text_(text) {}

    css::uno::Reference<css::uno::XInterface> getuno() override { return text_; }

    cpo::uno::Sequence<css::uno::Reference<scriptinterop::XParagraph>> getChildren() override {
        return enumerateParagraphs(text_);
    }

    OUString getText() override { return text_.is() ? text_->getString() : u""_ustr; }

private:
    css::uno::Reference<css::text::XText> text_;
};

class DocumentImpl : public cppu::WeakImplHelper<scriptinterop::XDocument>
{
public:
    explicit DocumentImpl(css::uno::Reference<css::frame::XModel> const& model)
        : model_(model)
    {
    }

    css::uno::Reference<css::uno::XInterface> SAL_CALL getuno() override { return model_; }

    css::uno::Reference<scriptinterop::XSelection> SAL_CALL getSelection() override
    {
        css::uno::Reference<css::text::XTextDocument> const doc(model_, css::uno::UNO_QUERY_THROW);
        css::uno::Reference<css::view::XSelectionSupplier> const sup(doc->getCurrentController(),
                                                                     css::uno::UNO_QUERY);
        css::uno::Reference<css::container::XIndexAccess> ranges;
        if (sup.is())
        {
            sup->getSelection() >>= ranges;
        }
        return new SelectionImpl(ranges);
    }

    css::uno::Reference<scriptinterop::XBody> getBody() override
    {
        css::uno::Reference<css::text::XTextDocument> const doc(model_, css::uno::UNO_QUERY_THROW);
        return new BodyImpl(doc->getText());
    }

    css::uno::Reference<scriptinterop::XCursor> getCursor() override
    {
        return new CursorImpl(model_);
    }

    cpo::uno::Sequence<css::uno::Reference<scriptinterop::XFootnote>> getFootnotes() override {
        std::vector<css::uno::Reference<scriptinterop::XFootnote>> v;
        if (css::uno::Reference<css::text::XFootnotesSupplier> const sup{
                model_, css::uno::UNO_QUERY})
        {
            auto const idx = sup->getFootnotes();
            if (idx.is()) {
                auto const n = idx->getCount();
                for (sal_Int32 i = 0; i != n; ++i) {
                    css::uno::Reference<css::text::XFootnote> footnote;
                    idx->getByIndex(i) >>= footnote;
                    if (footnote.is()) {
                        v.emplace_back(new FootnoteImpl(footnote));
                    }
                }
            }
        }
        return cpo::uno::Sequence(v.data(), v.size());
    }

    void SAL_CALL insertImage(cpo::uno::Sequence<sal_Int8> const& data,
                              scriptinterop::ImageOptions const& opts) override
    {
        css::uno::Reference<css::text::XTextDocument> const doc(model_, css::uno::UNO_QUERY_THROW);
        auto const componentCtx = comphelper::getProcessComponentContext();
        auto const smgr = componentCtx->getServiceManager();
        // Stage the bytes in a TempFile to give GraphicProvider a file URL:
        css::uno::Reference<css::io::XTempFile> const tmp(
            smgr->createInstanceWithContext(u"com.sun.star.io.TempFile"_ustr, componentCtx),
            css::uno::UNO_QUERY_THROW);
        tmp->getOutputStream()->writeBytes(data);
        tmp->getOutputStream()->closeOutput();
        css::uno::Reference<css::graphic::XGraphicProvider> const gp(
            smgr->createInstanceWithContext(u"com.sun.star.graphic.GraphicProvider"_ustr,
                                            componentCtx),
            css::uno::UNO_QUERY_THROW);
        cpo::uno::Sequence<css::beans::PropertyValue> loaderArgs{
            { u"URL"_ustr, 0, cpo::uno::Any(tmp->getUri()), {} }
        };
        auto const xgraphic = gp->queryGraphic(loaderArgs);
        if (!xgraphic.is())
        {
            throw cpo::uno::RuntimeException(u"insertImage: failed to load graphic"_ustr);
        }
        css::uno::Reference<css::lang::XMultiServiceFactory> const docFactory(doc,
                                                                              css::uno::UNO_QUERY);
        css::uno::Reference<css::text::XTextContent> const graphic(
            docFactory.is()
                ? docFactory->createInstance(u"com.sun.star.text.TextGraphicObject"_ustr)
                : nullptr,
            css::uno::UNO_QUERY);
        css::uno::Reference<css::beans::XPropertySet> const props(graphic, css::uno::UNO_QUERY);
        if (!props.is())
        {
            throw cpo::uno::RuntimeException(
                u"insertImage: failed to create TextGraphicObject"_ustr);
        }
        props->setPropertyValue(u"Graphic"_ustr, cpo::uno::Any(xgraphic));
        // Width and Height are in 1/100 mm:
        props->setPropertyValue(
            u"Width"_ustr, cpo::uno::Any(static_cast<sal_Int32>(std::round(opts.widthCm * 1000))));
        props->setPropertyValue(u"Height"_ustr, cpo::uno::Any(static_cast<sal_Int32>(
                                                    std::round(opts.heightCm * 1000))));
        props->setPropertyValue(u"AnchorType"_ustr,
                                cpo::uno::Any(css::text::TextContentAnchorType_AS_CHARACTER));
        css::uno::Reference<css::text::XTextViewCursorSupplier> const cursorSupplier(
            doc->getCurrentController(), css::uno::UNO_QUERY);
        css::uno::Reference<css::text::XTextRange> const cursor(
            cursorSupplier.is() ? cursorSupplier->getViewCursor()
                                : css::uno::Reference<css::text::XTextViewCursor>());
        if (!cursor.is())
        {
            throw cpo::uno::RuntimeException(u"insertImage: no view cursor"_ustr);
        }
        doc->getText()->insertTextContent(cursor, graphic, false);
    }

private:
    css::uno::Reference<css::frame::XModel> model_;
};
}

namespace scriptinterop::detail
{
css::uno::Reference<scriptinterop::XDocument>
createDocument(css::uno::Reference<css::frame::XModel> const& model)
{
    return new DocumentImpl(model);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
