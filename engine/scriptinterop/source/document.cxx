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

#include <cassert>
#include <cmath>
#include <optional>
#include <string_view>
#include <vector>

#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/awt/FontStrikeout.hpp>
#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/beans/Optional.hpp>
#include <com/sun/star/beans/PropertyState.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/container/XContentEnumerationAccess.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XIndexReplace.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/graphic/XGraphicProvider.hpp>
#include <com/sun/star/io/XTempFile.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/style/NumberingType.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/table/XCell.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/text/ControlCharacter.hpp>
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
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/text/XTextViewCursor.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <cpo/uno/Reference.hxx>
#include <cpo/uno/RuntimeException.hpp>
#include <cpo/uno/XComponentContext.hpp>
#include <cpo/uno/XInterface.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <comphelper/processfactory.hxx>
#include <cpo/uno/Any.hxx>
#include <cpo/uno/Exception.hdl>
#include <cpo/uno/Sequence.hxx>
#include <cppu/unotype.hxx>
#include <cppuhelper/implbase.hxx>
#include <o3tl/unit_conversion.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/ustring.hxx>
#include <sal/config.h>
#include <sal/log.hxx>
#include <sal/types.h>
#include <scriptinterop/ElementType.hpp>
#include <scriptinterop/GlyphType.hpp>
#include <scriptinterop/HorizontalAlignment.hpp>
#include <scriptinterop/ImageOptions.hpp>
#include <scriptinterop/ParagraphHeading.hpp>
#include <scriptinterop/TextAlignment.hpp>
#include <scriptinterop/XBlob.hpp>
#include <scriptinterop/XBody.hpp>
#include <scriptinterop/XContainerElement.hpp>
#include <scriptinterop/XCursor.hpp>
#include <scriptinterop/XDocument.hpp>
#include <scriptinterop/XElement.hpp>
#include <scriptinterop/XFootnote.hpp>
#include <scriptinterop/XInlineImage.hpp>
#include <scriptinterop/XParagraph.hpp>
#include <scriptinterop/XRangeBuilder.hpp>
#include <scriptinterop/XRangeElement.hpp>
#include <scriptinterop/XSelection.hpp>
#include <scriptinterop/XTable.hpp>
#include <scriptinterop/XTableCell.hpp>
#include <scriptinterop/XTableRow.hpp>
#include <scriptinterop/XText.hpp>

#include "document.hxx"

namespace
{
template<typename T>
css::beans::Optional<cpo::uno::Reference<T>> maybe(cpo::uno::Reference<T> const & ref)
{ return {ref.is(), ref}; }

cpo::uno::Sequence<cpo::uno::Reference<scriptinterop::XElement>> enumerateElements(
    cpo::uno::Reference<css::text::XText> const & text,
    cpo::uno::Reference<scriptinterop::XElement> const & parent);

cpo::uno::Reference<scriptinterop::XElement> siblingContent(
    cpo::uno::Reference<css::text::XTextContent> const & content,
    cpo::uno::Reference<scriptinterop::XElement> const & parent, bool forward);

void removeContent(cpo::uno::Reference<css::text::XTextContent> const & content)
{
    if (!content.is()) {
        throw cpo::uno::RuntimeException(u"element is not attached to a document"_ustr);
    }
    auto const anchor = content->getAnchor();
    if (!anchor.is()) {
        throw cpo::uno::RuntimeException(u"element has no anchor"_ustr);
    }
    auto const host = anchor->getText();
    if (!host.is()) {
        throw cpo::uno::RuntimeException(u"element has no containing text"_ustr);
    }
    host->removeTextContent(content);
}

sal_Int32 hundredthMmToPixels(sal_Int32 hundredthMm) {
    return o3tl::convert(hundredthMm, o3tl::Length::mm100, o3tl::Length::px);
}

sal_Int32 pixelsToHundredthMm(sal_Int32 pixels) {
    return o3tl::convert(pixels, o3tl::Length::px, o3tl::Length::mm100);
}

cpo::uno::Reference<css::text::XTextContent> createGraphicFromBlob(
    cpo::uno::Reference<css::frame::XModel> const & model,
    cpo::uno::Reference<scriptinterop::XBlob> const & blob)
{
    if (!blob.is()) {
        throw cpo::uno::RuntimeException(u"createGraphicFromBlob: null blob"_ustr);
    }
    auto const componentCtx = comphelper::getProcessComponentContext();
    auto const smgr = componentCtx->getServiceManager();
    cpo::uno::Reference<css::io::XTempFile> const tmp(
        smgr->createInstanceWithContext(u"com.sun.star.io.TempFile"_ustr, componentCtx),
        cpo::uno::UNO_QUERY_THROW);
    tmp->getOutputStream()->writeBytes(blob->getBytes());
    tmp->getOutputStream()->closeOutput();
    cpo::uno::Reference<css::graphic::XGraphicProvider> const gp(
        smgr->createInstanceWithContext(u"com.sun.star.graphic.GraphicProvider"_ustr, componentCtx),
        cpo::uno::UNO_QUERY_THROW);
    cpo::uno::Sequence<css::beans::PropertyValue> const loaderArgs{
        {u"URL"_ustr, 0, cpo::uno::Any(tmp->getUri()), {}}
    };
    auto const xgraphic = gp->queryGraphic(loaderArgs);
    if (!xgraphic.is()) {
        throw cpo::uno::RuntimeException(
            u"createGraphicFromBlob: failed to load graphic"_ustr);
    }
    cpo::uno::Reference<css::lang::XMultiServiceFactory> const docFactory(
        model, cpo::uno::UNO_QUERY_THROW);
    cpo::uno::Reference<css::text::XTextContent> const graphic(
        docFactory->createInstance(u"com.sun.star.text.TextGraphicObject"_ustr),
        cpo::uno::UNO_QUERY_THROW);
    cpo::uno::Reference<css::beans::XPropertySet> const props(graphic, cpo::uno::UNO_QUERY_THROW);
    props->setPropertyValue(u"Graphic"_ustr, cpo::uno::Any(xgraphic));
    props->setPropertyValue(
        u"AnchorType"_ustr, cpo::uno::Any(css::text::TextContentAnchorType_AS_CHARACTER));
    return graphic;
}

class SelectionImpl : public cppu::WeakImplHelper<scriptinterop::XSelection>
{
public:
    explicit SelectionImpl(cpo::uno::Reference<css::container::XIndexAccess> const& ranges)
        : ranges_(ranges)
    {
    }

    cpo::uno::Reference<cpo::uno::XInterface> SAL_CALL getuno() override { return ranges_; }

    cpo::uno::Sequence<cpo::uno::Reference<scriptinterop::XRangeElement>> getRangeElements()
        override;

    OUString SAL_CALL getText() override
    {
        OUStringBuffer buf;
        auto const n = ranges_->getCount();
        for (sal_Int32 i = 0; i != n; ++i)
        {
            cpo::uno::Reference<css::text::XTextRange> range;
            if (!(ranges_->getByIndex(i) >>= range) || !range.is())
            {
                throw cpo::uno::RuntimeException(
                    u"getText: the selection contains something that is not text"_ustr);
            }
            if (!buf.isEmpty())
            {
                buf.append('\n');
            }
            buf.append(range->getString());
        }
        return buf.makeStringAndClear();
    }

    void SAL_CALL replace(OUString const& newText) override
    {
        auto const n = ranges_->getCount();
        for (sal_Int32 i = 0; i != n; ++i)
        {
            cpo::uno::Reference<css::text::XTextRange> range;
            if (!(ranges_->getByIndex(i) >>= range) || !range.is())
            {
                throw cpo::uno::RuntimeException(
                    u"replace: the selection contains something that is not text"_ustr);
            }
            range->setString(newText);
        }
    }

private:
    cpo::uno::Reference<css::container::XIndexAccess> ranges_;
};

// Trivial single-element XIndexAccess wrapper so the XRangeBuilder can hand a single XTextRange
// to SelectionImpl without depending on Writer's own SwXTextRanges service:
class SingleRangeIndex: public cppu::WeakImplHelper<css::container::XIndexAccess> {
public:
    explicit SingleRangeIndex(cpo::uno::Reference<css::text::XTextRange> const & range):
        range_(range) {}

    sal_Int32 getCount() override { return 1; }

    cpo::uno::Any getByIndex(sal_Int32 index) override {
        if (index != 0) {
            throw css::lang::IndexOutOfBoundsException();
        }
        return cpo::uno::Any(range_);
    }

    cpo::uno::Type getElementType() override {
        return cppu::UnoType<css::text::XTextRange>::get();
    }

    bool hasElements() override { return true; }

private:
    cpo::uno::Reference<css::text::XTextRange> range_;
};

class RangeBuilderImpl: public cppu::WeakImplHelper<scriptinterop::XRangeBuilder> {
public:
    cpo::uno::Reference<scriptinterop::XRangeBuilder> addElement(
        cpo::uno::Reference<scriptinterop::XElement> const & element) override
    {
        if (!element.is()) {
            throw cpo::uno::RuntimeException(u"addElement: the element must not be null"_ustr);
        }
        cpo::uno::Reference<css::text::XTextRange> const range(
            element->getuno(), cpo::uno::UNO_QUERY);
        if (!range.is()) {
            throw cpo::uno::RuntimeException(
                u"addElement: only text elements can be added to a range"_ustr);
        }
        extend(range->getStart(), range->getEnd());
        return this;
    }

    cpo::uno::Reference<scriptinterop::XRangeBuilder> addElementRange(
        cpo::uno::Reference<scriptinterop::XText> const & text, sal_Int32 startOffset,
        sal_Int32 endOffsetInclusive) override
    {
        if (!text.is()) {
            throw cpo::uno::RuntimeException(
                u"addElementRange: the text must not be null"_ustr);
        }
        if (startOffset < 0 || endOffsetInclusive < startOffset) {
            throw cpo::uno::RuntimeException(
                "addElementRange: expected 0 <= startOffset <= endOffsetInclusive, got startOffset="
                + OUString::number(startOffset) + ", endOffsetInclusive="
                + OUString::number(endOffsetInclusive));
        }
        cpo::uno::Reference<css::text::XTextRange> const para(
            text->getuno(), cpo::uno::UNO_QUERY_THROW);
        auto const host = para->getText();
        if (!host.is()) {
            throw cpo::uno::RuntimeException(
                u"addElementRange: the text is not part of the document"_ustr);
        }
        auto const cursor = host->createTextCursorByRange(para->getStart());
        if (!cursor.is()) {
            throw cpo::uno::RuntimeException(
                u"addElementRange: cannot place a cursor at the element"_ustr);
        }
        cursor->goRight(startOffset, false);
        cursor->goRight(endOffsetInclusive - startOffset + 1, true);
        extend(cursor->getStart(), cursor->getEnd());
        return this;
    }

    cpo::uno::Reference<scriptinterop::XSelection> build() override {
        if (!cursor_.is()) {
            throw cpo::uno::RuntimeException(
                u"build: add at least one element or element range first"_ustr);
        }
        return new SelectionImpl(new SingleRangeIndex(cursor_));
    }

private:
    void extend(
        cpo::uno::Reference<css::text::XTextRange> const & start,
        cpo::uno::Reference<css::text::XTextRange> const & end)
    {
        if (!start.is() || !end.is()) {
            throw cpo::uno::RuntimeException(u"the element covers no text"_ustr);
        }
        if (!cursor_.is()) {
            auto const host = start->getText();
            if (!host.is()) {
                throw cpo::uno::RuntimeException(u"the element is not part of the document"_ustr);
            }
            cursor_ = host->createTextCursorByRange(start);
            if (!cursor_.is()) {
                throw cpo::uno::RuntimeException(u"cannot place a cursor at the range start"_ustr);
            }
        }
        cursor_->gotoRange(end, true);
    }

    cpo::uno::Reference<css::text::XTextCursor> cursor_;
};

class TextImpl: public cppu::WeakImplHelper<scriptinterop::XText> {
public:
    explicit TextImpl(
        cpo::uno::Reference<scriptinterop::XElement> const & parent,
        cpo::uno::Reference<css::text::XTextContent> const & content,
        scriptinterop::ElementType reportedType):
        parent_(parent), content_(content), reportedType_(reportedType)
    {
        if (cpo::uno::Reference<css::container::XEnumerationAccess> const ea{
                content_, cpo::uno::UNO_QUERY})
        {
            auto const en = ea->createEnumeration();
            while (en.is() && en->hasMoreElements()) {
                cpo::uno::Reference<css::text::XTextRange> portion;
                en->nextElement() >>= portion;
                if (portion.is()) {
                    runs_.push_back(portion);
                }
            }
        }
    }

    cpo::uno::Reference<cpo::uno::XInterface> getuno() override { return content_; }

    cpo::uno::Reference<scriptinterop::XText> appendText(OUString const & text) override {
        auto const whole = wholeRange();
        auto const host = whole->getText();
        host->insertString(host->createTextCursorByRange(whole->getEnd()), text, false);
        return this;
    }

    cpo::uno::Reference<scriptinterop::XInlineImage> asInlineImage() override { return nullptr; }

    // TODO: return a detached deep copy, not this; mutations on the "copy" write back to the live
    // element:
    cpo::uno::Reference<scriptinterop::XElement> copy() override { return this; }

    cpo::uno::Reference<scriptinterop::XText> deleteText(
        sal_Int32 startOffset, sal_Int32 endOffsetInclusive) override
    {
        subRange(startOffset, endOffsetInclusive)->setString(OUString());
        return this;
    }

    cpo::uno::Reference<scriptinterop::XText> editAsText() override { return this; }

    css::beans::Optional<OUString> getFontFamily(sal_Int32 offset) override {
        auto const any = getProp(
            runAt(offset), u"getFontFamily", u"font family", u"CharFontName"_ustr);
        if (!any) {
            return {false, {}};
        }
        OUString name;
        *any >>= name;
        return {true, name};
    }

    css::beans::Optional<OUString> getLinkUrl(sal_Int32 offset) override {
        auto const any = getProp(runAt(offset), u"getLinkUrl", u"link URL", u"HyperLinkURL"_ustr);
        if (!any) {
            return {false, {}};
        }
        OUString url;
        *any >>= url;
        return {true, url};
    }

    // A text portion has no true siblings in our model (paragraph.getChild only exposes one Text
    // child), so both sides are null; TODO: real sibling walk once inline images and breaks join
    // the paragraph's child list:
    css::beans::Optional<cpo::uno::Reference<scriptinterop::XElement>> getNextSibling() override {
        return {false, {}};
    }

    css::beans::Optional<cpo::uno::Reference<scriptinterop::XElement>> getParent() override {
        return maybe(parent_);
    }

    css::beans::Optional<cpo::uno::Reference<scriptinterop::XElement>> getPreviousSibling() override
    { return {false, {}}; }

    OUString getText() override {
        return cpo::uno::Reference<css::text::XTextRange>(content_, cpo::uno::UNO_QUERY_THROW)
            ->getString();
    }

    css::beans::Optional<scriptinterop::TextAlignment> getTextAlignment(sal_Int32 offset) override {
        auto const any = getProp(
            runAt(offset), u"getTextAlignment", u"alignment", u"CharEscapement"_ustr);
        if (!any) {
            return {false, {}};
        }
        sal_Int16 esc = 0;
        *any >>= esc;
        return {true,
            esc > 0 ? scriptinterop::TextAlignment_SUPERSCRIPT
                : esc < 0 ? scriptinterop::TextAlignment_SUBSCRIPT
                : scriptinterop::TextAlignment_NORMAL};
    }

    // GAS guarantees at least one attribute index for any text (uniform text has one index at 0):
    cpo::uno::Sequence<sal_Int32> getTextAttributeIndices() override {
        std::vector<sal_Int32> v;
        v.reserve(runs_.empty() ? 1 : runs_.size());
        sal_Int32 off = 0;
        for (auto const & r: runs_) {
            v.push_back(off);
            off += r->getString().getLength();
        }
        if (v.empty()) {
            v.push_back(0);
        }
        return cpo::uno::Sequence(v.data(), v.size());
    }

    cpo::uno::Reference<scriptinterop::XText> insertText(sal_Int32 offset, OUString const & text)
        override
    {
        auto const whole = wholeRange();
        auto const host = whole->getText();
        auto const cursor = host->createTextCursorByRange(whole->getStart());
        cursor->goRight(offset, false);
        host->insertString(cursor, text, false);
        return this;
    }

    scriptinterop::ElementType getType() override { return reportedType_; }

    css::beans::Optional<bool> isBold(sal_Int32 offset) override {
        auto const any = getProp(runAt(offset), u"isBold", u"bold attribute", u"CharWeight"_ustr);
        if (!any) {
            return {false, {}};
        }
        float weight = css::awt::FontWeight::NORMAL;
        *any >>= weight;
        return {true, weight >= css::awt::FontWeight::BOLD};
    }

    css::beans::Optional<bool> isItalic(sal_Int32 offset) override {
        auto const any = getProp(
            runAt(offset), u"isItalic", u"italic attribute", u"CharPosture"_ustr);
        if (!any) {
            return {false, {}};
        }
        css::awt::FontSlant slant = css::awt::FontSlant_NONE;
        *any >>= slant;
        return {true, slant == css::awt::FontSlant_ITALIC || slant == css::awt::FontSlant_OBLIQUE};
    }

    css::beans::Optional<bool> isStrikethrough(sal_Int32 offset) override {
        auto const any = getProp(
            runAt(offset), u"isStrikethrough", u"strikethrough attribute", u"CharStrikeout"_ustr);
        if (!any) {
            return {false, {}};
        }
        sal_Int16 strike = css::awt::FontStrikeout::NONE;
        *any >>= strike;
        return {true, strike != css::awt::FontStrikeout::NONE};
    }

    css::beans::Optional<bool> isUnderline(sal_Int32 offset) override {
        auto const any = getProp(
            runAt(offset), u"isUnderline", u"underline attribute", u"CharUnderline"_ustr);
        if (!any) {
            return {false, {}};
        }
        sal_Int16 underline = css::awt::FontUnderline::NONE;
        *any >>= underline;
        return {true, underline != css::awt::FontUnderline::NONE};
    }

    void removeFromParent() override { removeContent(content_); }

    cpo::uno::Reference<scriptinterop::XText> setBold(bool value) override {
        setBoldOn(wholeRange(), value);
        return this;
    }

    cpo::uno::Reference<scriptinterop::XText> setBoldRange(
        sal_Int32 startOffset, sal_Int32 endOffsetInclusive, bool value) override
    {
        setBoldOn(subRange(startOffset, endOffsetInclusive), value);
        return this;
    }

    cpo::uno::Reference<scriptinterop::XText> setFontFamily(OUString const & fontFamilyName)
        override
    {
        setFontFamilyOn(wholeRange(), fontFamilyName);
        return this;
    }

    cpo::uno::Reference<scriptinterop::XText> setFontFamilyRange(
        sal_Int32 startOffset, sal_Int32 endOffsetInclusive, OUString const & fontFamilyName)
        override
    {
        setFontFamilyOn(subRange(startOffset, endOffsetInclusive), fontFamilyName);
        return this;
    }

    cpo::uno::Reference<scriptinterop::XText> setItalic(bool value) override {
        setItalicOn(wholeRange(), value);
        return this;
    }

    cpo::uno::Reference<scriptinterop::XText> setItalicRange(
        sal_Int32 startOffset, sal_Int32 endOffsetInclusive, bool value) override
    {
        setItalicOn(subRange(startOffset, endOffsetInclusive), value);
        return this;
    }

    cpo::uno::Reference<scriptinterop::XText> setLinkUrl(OUString const & url) override {
        setLinkUrlOn(wholeRange(), url);
        return this;
    }

    cpo::uno::Reference<scriptinterop::XText> setLinkUrlRange(
        sal_Int32 startOffset, sal_Int32 endOffsetInclusive, OUString const & url) override
    {
        setLinkUrlOn(subRange(startOffset, endOffsetInclusive), url);
        return this;
    }

    cpo::uno::Reference<scriptinterop::XText> setStrikethrough(bool value) override {
        setStrikethroughOn(wholeRange(), value);
        return this;
    }

    cpo::uno::Reference<scriptinterop::XText> setStrikethroughRange(
        sal_Int32 startOffset, sal_Int32 endOffsetInclusive, bool value) override
    {
        setStrikethroughOn(subRange(startOffset, endOffsetInclusive), value);
        return this;
    }

    cpo::uno::Reference<scriptinterop::XText> setText(OUString const & text) override {
        wholeRange()->setString(text);
        return this;
    }

    cpo::uno::Reference<scriptinterop::XText> setTextAlignment(
        scriptinterop::TextAlignment textAlignment) override
    {
        setTextAlignmentOn(wholeRange(), textAlignment);
        return this;
    }

    cpo::uno::Reference<scriptinterop::XText> setTextAlignmentRange(
        sal_Int32 startOffset, sal_Int32 endOffsetInclusive,
        scriptinterop::TextAlignment textAlignment) override
    {
        setTextAlignmentOn(subRange(startOffset, endOffsetInclusive), textAlignment);
        return this;
    }

    cpo::uno::Reference<scriptinterop::XText> setUnderline(bool value) override {
        setUnderlineOn(wholeRange(), value);
        return this;
    }

    cpo::uno::Reference<scriptinterop::XText> setUnderlineRange(
        sal_Int32 startOffset, sal_Int32 endOffsetInclusive, bool value) override
    {
        setUnderlineOn(subRange(startOffset, endOffsetInclusive), value);
        return this;
    }

private:
    cpo::uno::Reference<css::text::XTextRange> runAt(sal_Int32 offset) {
        if (runs_.empty()) {
            throw cpo::uno::RuntimeException(u"the text has no content at this offset"_ustr);
        }
        sal_Int32 start = 0;
        for (auto const & r: runs_) {
            auto const len = r->getString().getLength();
            if (offset < start + len) {
                return r;
            }
            start += len;
        }
        return runs_.back();
    }

    std::optional<cpo::uno::Any> getProp(
        cpo::uno::Reference<css::text::XTextRange> const & range, std::u16string_view apiMethod,
        std::u16string_view apiAttribute, OUString const & name)
    {
        cpo::uno::Reference<css::beans::XPropertySet> const props(
            range, cpo::uno::UNO_QUERY_THROW);
        auto const info(props->getPropertySetInfo());
        if (!info.is() || !info->hasPropertyByName(name)) {
            throw cpo::uno::RuntimeException(
                OUString::Concat(apiMethod) + ": the text has no " + apiAttribute);
        }
        cpo::uno::Reference<css::beans::XPropertyState> const runState(range, cpo::uno::UNO_QUERY);
        if (runState.is()
            && runState->getPropertyState(name) == css::beans::PropertyState_DIRECT_VALUE)
        {
            return props->getPropertyValue(name);
        }
        // A property set on the whole paragraph shows up on the paragraph itself as DIRECT while
        // the paragraph's runs still report DEFAULT, so the paragraph is the fallback place to
        // look; not every run-level property is exposed at paragraph level, so check whether it is
        // before querying its state:
        cpo::uno::Reference<css::beans::XPropertySet> const paraProps(
            content_, cpo::uno::UNO_QUERY);
        cpo::uno::Reference<css::beans::XPropertyState> const paraState(
            content_, cpo::uno::UNO_QUERY);
        if (paraProps.is() && paraState.is() && paraProps->getPropertySetInfo().is()
            && paraProps->getPropertySetInfo()->hasPropertyByName(name)
            && paraState->getPropertyState(name) == css::beans::PropertyState_DIRECT_VALUE)
        {
            return props->getPropertyValue(name);
        }
        return std::nullopt;
    }

    cpo::uno::Reference<css::text::XTextRange> wholeRange() {
        return cpo::uno::Reference<css::text::XTextRange>(content_, cpo::uno::UNO_QUERY_THROW);
    }

    cpo::uno::Reference<css::text::XTextRange> subRange(
        sal_Int32 startOffset, sal_Int32 endOffsetInclusive)
    {
        auto const whole = wholeRange();
        auto const cursor = whole->getText()->createTextCursorByRange(whole->getStart());
        cursor->goRight(startOffset, false);
        cursor->goRight(endOffsetInclusive - startOffset + 1, true);
        return cursor;
    }

    static void setProp(
        cpo::uno::Reference<css::text::XTextRange> const & range, OUString const & name,
        cpo::uno::Any const & value)
    {
        cpo::uno::Reference<css::beans::XPropertySet>(range, cpo::uno::UNO_QUERY_THROW)
            ->setPropertyValue(name, value);
    }

    static void setBoldOn(cpo::uno::Reference<css::text::XTextRange> const & range, bool value) {
        setProp(range, u"CharWeight"_ustr, cpo::uno::Any(
            static_cast<float>(value ? css::awt::FontWeight::BOLD : css::awt::FontWeight::NORMAL)));
    }

    static void setFontFamilyOn(
        cpo::uno::Reference<css::text::XTextRange> const & range, OUString const & fontFamilyName)
    {
        setProp(range, u"CharFontName"_ustr, cpo::uno::Any(fontFamilyName));
    }

    static void setItalicOn(cpo::uno::Reference<css::text::XTextRange> const & range, bool value) {
        setProp(range, u"CharPosture"_ustr, cpo::uno::Any(
            value ? css::awt::FontSlant_ITALIC : css::awt::FontSlant_NONE));
    }

    static void setLinkUrlOn(
        cpo::uno::Reference<css::text::XTextRange> const & range, OUString const & url)
    {
        setProp(range, u"HyperLinkURL"_ustr, cpo::uno::Any(url));
    }

    static void setStrikethroughOn(
        cpo::uno::Reference<css::text::XTextRange> const & range, bool value)
    {
        setProp(range, u"CharStrikeout"_ustr, cpo::uno::Any(static_cast<sal_Int16>(
            value ? css::awt::FontStrikeout::SINGLE : css::awt::FontStrikeout::NONE)));
    }

    static void setTextAlignmentOn(
        cpo::uno::Reference<css::text::XTextRange> const & range,
        scriptinterop::TextAlignment textAlignment)
    {
        sal_Int16 escape = 0;
        sal_Int8 height = 100;
        switch (textAlignment) {
        case scriptinterop::TextAlignment_SUPERSCRIPT:
            escape = 33;
            height = 58;
            break;
        case scriptinterop::TextAlignment_SUBSCRIPT:
            escape = -33;
            height = 58;
            break;
        case scriptinterop::TextAlignment_NORMAL:
            break;
        default:
            break;
        }
        setProp(range, u"CharEscapement"_ustr, cpo::uno::Any(escape));
        setProp(range, u"CharEscapementHeight"_ustr, cpo::uno::Any(height));
    }

    static void setUnderlineOn(cpo::uno::Reference<css::text::XTextRange> const & range, bool value)
    {
        setProp(range, u"CharUnderline"_ustr, cpo::uno::Any(static_cast<sal_Int16>(
            value ? css::awt::FontUnderline::SINGLE : css::awt::FontUnderline::NONE)));
    }

    cpo::uno::Reference<scriptinterop::XElement> parent_;
    cpo::uno::Reference<css::text::XTextContent> content_;
    scriptinterop::ElementType reportedType_;
    std::vector<cpo::uno::Reference<css::text::XTextRange>> runs_;
};

class InlineImageImpl: public cppu::WeakImplHelper<scriptinterop::XInlineImage> {
public:
    explicit InlineImageImpl(
        cpo::uno::Reference<scriptinterop::XElement> const & parent,
        cpo::uno::Reference<css::text::XTextContent> const & content):
        parent_(parent), content_(content) {}

    cpo::uno::Reference<cpo::uno::XInterface> getuno() override { return content_; }

    cpo::uno::Reference<scriptinterop::XInlineImage> asInlineImage() override { return this; }

    // TODO: return a detached deep copy, not this; mutations on the "copy" write back to the live
    // element:
    cpo::uno::Reference<scriptinterop::XElement> copy() override { return this; }

    css::beans::Optional<OUString> getAltDescription() override {
        auto const any = getInlineProp(u"Description"_ustr);
        if (!any) {
            return {false, {}};
        }
        OUString description;
        *any >>= description;
        return {true, description};
    }

    css::beans::Optional<OUString> getAltTitle() override {
        auto const any = getInlineProp(u"Title"_ustr);
        if (!any) {
            return {false, {}};
        }
        OUString title;
        *any >>= title;
        return {true, title};
    }

    sal_Int32 getHeight() override {
        sal_Int32 hundredthMm = 0;
        props()->getPropertyValue(u"Height"_ustr) >>= hundredthMm;
        return hundredthMmToPixels(hundredthMm);
    }

    // TODO: siblings within a paragraph would join the paragraph's own child walk once inline
    // images are placed in reading order alongside their text runs.
    css::beans::Optional<cpo::uno::Reference<scriptinterop::XElement>> getNextSibling() override {
        return {false, {}};
    }

    css::beans::Optional<cpo::uno::Reference<scriptinterop::XElement>> getParent() override {
        return maybe(parent_);
    }

    css::beans::Optional<cpo::uno::Reference<scriptinterop::XElement>> getPreviousSibling() override
    { return {false, {}}; }

    OUString getText() override {
        auto const title = getAltTitle();
        return title.IsPresent ? title.Value : u""_ustr;
    }

    scriptinterop::ElementType getType() override {
        return scriptinterop::ElementType_INLINE_IMAGE;
    }

    sal_Int32 getWidth() override {
        sal_Int32 hundredthMm = 0;
        props()->getPropertyValue(u"Width"_ustr) >>= hundredthMm;
        return hundredthMmToPixels(hundredthMm);
    }

    void removeFromParent() override { removeContent(content_); }

    cpo::uno::Reference<scriptinterop::XInlineImage> setAltDescription(OUString const & description)
        override
    {
        props()->setPropertyValue(u"Description"_ustr, cpo::uno::Any(description));
        return this;
    }

    cpo::uno::Reference<scriptinterop::XInlineImage> setAltTitle(OUString const & title) override {
        props()->setPropertyValue(u"Title"_ustr, cpo::uno::Any(title));
        return this;
    }

    cpo::uno::Reference<scriptinterop::XInlineImage> setHeight(sal_Int32 height) override {
        props()->setPropertyValue(u"Height"_ustr, cpo::uno::Any(pixelsToHundredthMm(height)));
        return this;
    }

    cpo::uno::Reference<scriptinterop::XInlineImage> setWidth(sal_Int32 width) override {
        props()->setPropertyValue(u"Width"_ustr, cpo::uno::Any(pixelsToHundredthMm(width)));
        return this;
    }

private:
    cpo::uno::Reference<css::beans::XPropertySet> props() {
        if (!content_.is()) {
            throw cpo::uno::RuntimeException(u"InlineImageImpl has no content"_ustr);
        }
        return cpo::uno::Reference<css::beans::XPropertySet>(content_, cpo::uno::UNO_QUERY_THROW);
    }

    std::optional<cpo::uno::Any> getInlineProp(OUString const & name) {
        auto const p = props();
        auto const any = p->getPropertyValue(name);
        OUString value;
        if ((any >>= value) && value.isEmpty()) {
            return std::nullopt;
        }
        return any;
    }

    cpo::uno::Reference<scriptinterop::XElement> parent_;
    cpo::uno::Reference<css::text::XTextContent> content_;
};

class ParagraphImpl : public cppu::WeakImplHelper<scriptinterop::XParagraph>
{
public:
    explicit ParagraphImpl(
        cpo::uno::Reference<scriptinterop::XElement> const & parent,
        cpo::uno::Reference<css::text::XTextContent> const& content)
        : parent_(parent), content_(content)
    {
    }

    cpo::uno::Reference<cpo::uno::XInterface> SAL_CALL getuno() override { return content_; }

    cpo::uno::Reference<scriptinterop::XInlineImage> asInlineImage() override { return {}; }

    cpo::uno::Reference<scriptinterop::XText> asText() override {
        return new TextImpl(parent_, content_, getType());
    }

    void clear() override {
        cpo::uno::Reference<css::text::XTextRange>(content_, cpo::uno::UNO_QUERY_THROW)
            ->setString(OUString());
    }

    // TODO: return a detached deep copy, not this; mutations on the "copy" write back to the live
    // element:
    cpo::uno::Reference<scriptinterop::XElement> copy() override { return this; }

    cpo::uno::Reference<scriptinterop::XText> editAsText() override {
        return asText();
    }

    css::beans::Optional<scriptinterop::HorizontalAlignment> getAlignment() override {
        auto const any = getParaProp(u"ParaAdjust"_ustr);
        if (!any) {
            return {false, {}};
        }
        sal_Int16 adjust = static_cast<sal_Int16>(css::style::ParagraphAdjust_LEFT);
        *any >>= adjust;
        switch (adjust) {
        case static_cast<sal_Int16>(css::style::ParagraphAdjust_CENTER):
            return {true, scriptinterop::HorizontalAlignment_CENTER};
        case static_cast<sal_Int16>(css::style::ParagraphAdjust_RIGHT):
            return {true, scriptinterop::HorizontalAlignment_RIGHT};
        case static_cast<sal_Int16>(css::style::ParagraphAdjust_BLOCK):
            return {true, scriptinterop::HorizontalAlignment_JUSTIFY};
        default:
            return {true, scriptinterop::HorizontalAlignment_LEFT};
        }
    }

    // TODO: match GAS's per-portion reading order, where an image splits the surrounding text into
    // a Text before it and a Text after it:
    cpo::uno::Reference<scriptinterop::XElement> getChild(sal_Int32 index) override {
        if (index == 0) {
            return new TextImpl(this, content_, scriptinterop::ElementType_TEXT);
        }
        auto const images = enumerateInlineImages();
        auto const imgIndex = index - 1;
        if (imgIndex < 0 || imgIndex >= static_cast<sal_Int32>(images.size())) {
            return {};
        }
        return new InlineImageImpl(this, images[imgIndex]);
    }

    css::beans::Optional<scriptinterop::GlyphType> getGlyphType() override {
        auto const rulesAny = getParaProp(u"NumberingRules"_ustr);
        if (!rulesAny) {
            return {false, {}};
        }
        cpo::uno::Reference<css::beans::XPropertySet> const props(
            content_, cpo::uno::UNO_QUERY_THROW);
        cpo::uno::Reference<css::container::XIndexAccess> rules;
        *rulesAny >>= rules;
        sal_Int16 level = 0;
        auto const info(props->getPropertySetInfo());
        if (info.is() && info->hasPropertyByName(u"NumberingLevel"_ustr)) {
            props->getPropertyValue(u"NumberingLevel"_ustr) >>= level;
        }
        if (!rules.is() || level < 0 || level >= rules->getCount()) {
            return {true, scriptinterop::GlyphType_BULLET};
        }
        cpo::uno::Sequence<css::beans::PropertyValue> entry;
        rules->getByIndex(level) >>= entry;
        sal_Int16 numberingType = css::style::NumberingType::CHAR_SPECIAL;
        OUString bulletChar;
        for (auto const & pv: entry) {
            if (pv.Name == u"NumberingType") {
                pv.Value >>= numberingType;
            } else if (pv.Name == u"BulletChar") {
                pv.Value >>= bulletChar;
            }
        }
        switch (numberingType) {
        case css::style::NumberingType::ARABIC:
            return {true, scriptinterop::GlyphType_NUMBER};
        case css::style::NumberingType::CHARS_UPPER_LETTER:
        case css::style::NumberingType::CHARS_UPPER_LETTER_N:
            return {true, scriptinterop::GlyphType_LATIN_UPPER};
        case css::style::NumberingType::CHARS_LOWER_LETTER:
        case css::style::NumberingType::CHARS_LOWER_LETTER_N:
            return {true, scriptinterop::GlyphType_LATIN_LOWER};
        case css::style::NumberingType::ROMAN_UPPER:
            return {true, scriptinterop::GlyphType_ROMAN_UPPER};
        case css::style::NumberingType::ROMAN_LOWER:
            return {true, scriptinterop::GlyphType_ROMAN_LOWER};
        default:
            break;
        }
        if (!bulletChar.isEmpty()) {
            auto const ch = bulletChar[0];
            if (ch == u'◦' || ch == u'○') {
                return {true, scriptinterop::GlyphType_HOLLOW_BULLET};
            }
            if (ch == u'▪' || ch == u'■') {
                return {true, scriptinterop::GlyphType_SQUARE_BULLET};
            }
        }
        return {true, scriptinterop::GlyphType_BULLET};
    }

    css::beans::Optional<scriptinterop::ParagraphHeading> getHeading() override {
        auto const any = getParaProp(u"ParaStyleName"_ustr);
        if (!any) {
            return {false, {}};
        }
        OUString style;
        *any >>= style;
        if (style == u"Heading 1") {
            return {true, scriptinterop::ParagraphHeading_HEADING1};
        }
        if (style == u"Heading 2") {
            return {true, scriptinterop::ParagraphHeading_HEADING2};
        }
        if (style == u"Heading 3") {
            return {true, scriptinterop::ParagraphHeading_HEADING3};
        }
        if (style == u"Heading 4") {
            return {true, scriptinterop::ParagraphHeading_HEADING4};
        }
        if (style == u"Heading 5") {
            return {true, scriptinterop::ParagraphHeading_HEADING5};
        }
        if (style == u"Heading 6") {
            return {true, scriptinterop::ParagraphHeading_HEADING6};
        }
        if (style == u"Title") {
            return {true, scriptinterop::ParagraphHeading_TITLE};
        }
        if (style == u"Subtitle") {
            return {true, scriptinterop::ParagraphHeading_SUBTITLE};
        }
        return {true, scriptinterop::ParagraphHeading_NORMAL};
    }

    css::beans::Optional<double> getIndentStart() override {
        auto const any = getParaProp(u"ParaLeftMargin"_ustr);
        if (!any) {
            return {false, {}};
        }
        sal_Int32 hundredthMm = 0;
        *any >>= hundredthMm;
        return {true, hundredthMm * 72.0 / 2540.0};
    }

    css::beans::Optional<OUString> getListId() override {
        auto const rulesAny = getParaProp(u"NumberingRules"_ustr);
        if (!rulesAny) {
            return {false, {}};
        }
        cpo::uno::Reference<css::beans::XPropertySet> const props(
            content_, cpo::uno::UNO_QUERY_THROW);
        auto const info(props->getPropertySetInfo());
        OUString id;
        if (info.is() && info->hasPropertyByName(u"ListId"_ustr)) {
            props->getPropertyValue(u"ListId"_ustr) >>= id;
        }
        return {true, id};
    }

    sal_Int32 getNestingLevel() override {
        cpo::uno::Reference<css::beans::XPropertySet> const props(
            content_, cpo::uno::UNO_QUERY_THROW);
        auto const info(props->getPropertySetInfo());
        if (!info.is() || !info->hasPropertyByName(u"NumberingLevel"_ustr)) {
            return 0;
        }
        sal_Int16 level = 0;
        props->getPropertyValue(u"NumberingLevel"_ustr) >>= level;
        return level;
    }

    css::beans::Optional<cpo::uno::Reference<scriptinterop::XElement>> getNextSibling() override {
        return maybe(siblingContent(content_, parent_, true));
    }

    sal_Int32 getNumChildren() override {
        return 1 + static_cast<sal_Int32>(enumerateInlineImages().size());
    }

    css::beans::Optional<cpo::uno::Reference<scriptinterop::XElement>> getParent() override {
        return maybe(parent_);
    }

    css::beans::Optional<cpo::uno::Reference<scriptinterop::XElement>> getPreviousSibling() override
    { return maybe(siblingContent(content_, parent_, false)); }

    OUString SAL_CALL getText() override
    {
        return cpo::uno::Reference<css::text::XTextRange>(content_, cpo::uno::UNO_QUERY_THROW)
            ->getString();
    }

    scriptinterop::ElementType getType() override {
        cpo::uno::Reference<css::beans::XPropertySet> const props(
            content_, cpo::uno::UNO_QUERY_THROW);
        auto const info(props->getPropertySetInfo());
        if (info.is() && info->hasPropertyByName(u"NumberingIsNumber"_ustr)) {
            bool numbered = false;
            props->getPropertyValue(u"NumberingIsNumber"_ustr) >>= numbered;
            if (numbered) {
                return scriptinterop::ElementType_LIST_ITEM;
            }
        }
        return scriptinterop::ElementType_PARAGRAPH;
    }

    css::beans::Optional<bool> isLeftToRight() override {
        cpo::uno::Reference<css::beans::XPropertySet> const props(
            content_, cpo::uno::UNO_QUERY_THROW);
        sal_Int16 mode = css::text::WritingMode2::LR_TB;
        props->getPropertyValue(u"WritingMode"_ustr) >>= mode;
        return {true, mode != css::text::WritingMode2::RL_TB};
    }

    void removeFromParent() override { removeContent(content_); }

private:
    std::optional<cpo::uno::Any> getParaProp(OUString const & name) {
        cpo::uno::Reference<css::beans::XPropertySet> const props(
            content_, cpo::uno::UNO_QUERY_THROW);
        auto const info(props->getPropertySetInfo());
        if (!info.is() || !info->hasPropertyByName(name)) {
            return std::nullopt;
        }
        cpo::uno::Reference<css::beans::XPropertyState> const state(content_, cpo::uno::UNO_QUERY);
        if (state.is() && state->getPropertyState(name) != css::beans::PropertyState_DIRECT_VALUE) {
            return std::nullopt;
        }
        return props->getPropertyValue(name);
    }

    std::vector<cpo::uno::Reference<css::text::XTextContent>> enumerateInlineImages() {
        std::vector<cpo::uno::Reference<css::text::XTextContent>> images;
        cpo::uno::Reference<css::container::XEnumerationAccess> const ea(
            content_, cpo::uno::UNO_QUERY);
        if (!ea.is()) {
            return images;
        }
        auto const en = ea->createEnumeration();
        while (en.is() && en->hasMoreElements()) {
            cpo::uno::Reference<css::beans::XPropertySet> portion;
            en->nextElement() >>= portion;
            if (!portion.is()) {
                continue;
            }
            OUString portionType;
            portion->getPropertyValue(u"TextPortionType"_ustr) >>= portionType;
            if (portionType != u"Frame") {
                continue;
            }
            cpo::uno::Reference<css::container::XContentEnumerationAccess> const cea(
                portion, cpo::uno::UNO_QUERY);
            if (!cea.is()) {
                continue;
            }
            auto const contents
                = cea->createContentEnumeration(u"com.sun.star.text.TextContent"_ustr);
            while (contents.is() && contents->hasMoreElements()) {
                cpo::uno::Reference<css::text::XTextContent> frame;
                contents->nextElement() >>= frame;
                if (!frame.is()) {
                    continue;
                }
                cpo::uno::Reference<css::lang::XServiceInfo> const info(
                    frame, cpo::uno::UNO_QUERY);
                if (!info.is()
                    || !info->supportsService(u"com.sun.star.text.TextGraphicObject"_ustr))
                {
                    continue;
                }
                if (auto const frameProps = cpo::uno::Reference<css::beans::XPropertySet>(
                        frame, cpo::uno::UNO_QUERY))
                {
                    css::text::TextContentAnchorType anchor;
                    if ((frameProps->getPropertyValue(u"AnchorType"_ustr) >>= anchor)
                        && anchor == css::text::TextContentAnchorType_AS_CHARACTER)
                    {
                        images.push_back(frame);
                    }
                }
            }
        }
        return images;
    }

    cpo::uno::Reference<scriptinterop::XElement> parent_;
    cpo::uno::Reference<css::text::XTextContent> content_;
};

class TableCellImpl: public cppu::WeakImplHelper<scriptinterop::XTableCell> {
public:
    explicit TableCellImpl(
        cpo::uno::Reference<scriptinterop::XElement> const & parent,
        cpo::uno::Reference<css::text::XText> const & text):
        parent_(parent), text_(text) {}

    cpo::uno::Reference<cpo::uno::XInterface> getuno() override { return text_; }

    cpo::uno::Reference<scriptinterop::XInlineImage> asInlineImage() override { return {}; }

    void clear() override {
        if (!text_.is()) {
            throw cpo::uno::RuntimeException(u"clear: the table cell has no text"_ustr);
        }
        text_->setString(OUString());
    }

    // TODO: return a detached deep copy, not this; mutations on the "copy" write back to the live
    // element:
    cpo::uno::Reference<scriptinterop::XElement> copy() override { return this; }

    cpo::uno::Reference<scriptinterop::XElement> getChild(sal_Int32 index) override {
        auto const list = getChildren();
        return index >= 0 && index < list.getLength() ? list[index] : nullptr;
    }

    cpo::uno::Sequence<cpo::uno::Reference<scriptinterop::XElement>> getChildren() override {
        return enumerateElements(text_, this);
    }

    // TODO: Writer text tables encode a horizontal merge in the anchor cell's name (a merged cell
    // is named for its range like "A1.B2"); parsing that would let us return the true span instead
    // of 1:
    sal_Int32 getColSpan() override { return 1; }

    // TODO: real sibling walk that steps through the containing row's cells:
    css::beans::Optional<cpo::uno::Reference<scriptinterop::XElement>> getNextSibling() override {
        return {false, {}};
    }

    sal_Int32 getNumChildren() override { return getChildren().getLength(); }

    css::beans::Optional<cpo::uno::Reference<scriptinterop::XElement>> getParent() override {
        return maybe(parent_);
    }

    css::beans::Optional<cpo::uno::Reference<scriptinterop::XElement>> getPreviousSibling() override
    { return {false, {}}; }

    sal_Int32 getRowSpan() override {
        cpo::uno::Reference<css::beans::XPropertySet> const props(text_, cpo::uno::UNO_QUERY);
        if (!props.is()) {
            return 1;
        }
        auto const info(props->getPropertySetInfo());
        if (!info.is() || !info->hasPropertyByName(u"RowSpan"_ustr)) {
            return 1;
        }
        sal_Int32 span = 1;
        props->getPropertyValue(u"RowSpan"_ustr) >>= span;
        return span < 1 ? 1 : span;
    }

    scriptinterop::ElementType getType() override { return scriptinterop::ElementType_TABLE_CELL; }

    OUString getText() override {
        if (!text_.is()) {
            throw cpo::uno::RuntimeException(u"getText: the table cell has no text"_ustr);
        }
        return text_->getString();
    }

    void removeFromParent() override {
        throw cpo::uno::RuntimeException(
            u"a table cell cannot be removed on its own; remove its row instead"_ustr);
    }

private:
    cpo::uno::Reference<scriptinterop::XElement> parent_;
    cpo::uno::Reference<css::text::XText> text_;
};

class TableRowImpl: public cppu::WeakImplHelper<scriptinterop::XTableRow> {
public:
    TableRowImpl(
        cpo::uno::Reference<scriptinterop::XElement> const & parent,
        cpo::uno::Reference<css::text::XTextTable> const & table, sal_Int32 rowIndex):
        parent_(parent), table_(table), rowIndex_(rowIndex) {}

    cpo::uno::Reference<cpo::uno::XInterface> getuno() override {
        if (!table_.is()) {
            throw cpo::uno::RuntimeException(u"getuno: the row is not part of a table"_ustr);
        }
        auto const rows = table_->getRows();
        cpo::uno::Reference<cpo::uno::XInterface> row;
        if (rows.is() && rowIndex_ >= 0 && rowIndex_ < rows->getCount()) {
            rows->getByIndex(rowIndex_) >>= row;
        }
        return row;
    }

    cpo::uno::Reference<scriptinterop::XInlineImage> asInlineImage() override { return {}; }

    void clear() override {
        throw cpo::uno::RuntimeException(u"TableRow.clear is not yet implemented"_ustr); // TODO
    }

    // TODO: return a detached deep copy, not this; mutations on the "copy" write back to the live
    // element:
    cpo::uno::Reference<scriptinterop::XElement> copy() override { return this; }

    css::beans::Optional<cpo::uno::Reference<scriptinterop::XTableCell>> getCell(sal_Int32 index)
        override
    {
        cpo::uno::Reference<css::table::XCellRange> const range(
            table_, cpo::uno::UNO_QUERY_THROW);
        cpo::uno::Reference<css::text::XText> text;
        try {
            text.set(range->getCellByPosition(index, rowIndex_), cpo::uno::UNO_QUERY);
        } catch (css::lang::IndexOutOfBoundsException const &) {
            return {false, {}};
        }
        if (!text.is()) {
            throw cpo::uno::RuntimeException(
                "getCell: cell " + OUString::number(index) + " cannot hold text");
        }
        return {true,
            cpo::uno::Reference<scriptinterop::XTableCell>(new TableCellImpl(this, text))};
    }

    cpo::uno::Reference<scriptinterop::XElement> getChild(sal_Int32 index) override {
        auto const cell = getCell(index);
        return cell.IsPresent ? cell.Value : nullptr;
    }

    // TODO: real sibling walk that steps through the containing table's rows:
    css::beans::Optional<cpo::uno::Reference<scriptinterop::XElement>> getNextSibling() override {
        return {false, {}};
    }

    css::beans::Optional<cpo::uno::Reference<scriptinterop::XElement>> getParent() override {
        return maybe(parent_);
    }

    css::beans::Optional<cpo::uno::Reference<scriptinterop::XElement>> getPreviousSibling() override
    { return {false, {}}; }

    scriptinterop::ElementType getType() override { return scriptinterop::ElementType_TABLE_ROW; }

    sal_Int32 getNumChildren() override { return getNumCells(); }

    sal_Int32 getNumCells() override {
        if (!table_.is()) {
            throw cpo::uno::RuntimeException(u"getNumCells: the row is not part of a table"_ustr);
        }
        // Probe cell positions to find how many cells this row actually addresses; the table's
        // column count over-reports for rows with horizontally merged cells, because it always
        // reflects the first row:
        cpo::uno::Reference<css::table::XCellRange> const range(table_, cpo::uno::UNO_QUERY_THROW);
        sal_Int32 count = 0;
        for (;;) {
            cpo::uno::Reference<css::table::XCell> cell;
            try {
                cell = range->getCellByPosition(count, rowIndex_);
            } catch (css::lang::IndexOutOfBoundsException const &) {
                break;
            }
            if (!cell.is()) {
                break;
            }
            ++count;
        }
        return count;
    }

    OUString getText() override {
        OUStringBuffer buf;
        auto const n = getNumCells();
        for (sal_Int32 i = 0; i != n; ++i) {
            auto const cell = getCell(i);
            if (!cell.IsPresent) {
                throw cpo::uno::RuntimeException(
                    "getText: the row has no cell at position " + OUString::number(i));
            }
            if (!buf.isEmpty()) {
                buf.append('\t');
            }
            buf.append(cell.Value->getText());
        }
        return buf.makeStringAndClear();
    }

    void removeFromParent() override {
        if (!table_.is()) {
            throw cpo::uno::RuntimeException(
                u"removeFromParent: the row is not part of a table"_ustr);
        }
        auto const rows = table_->getRows();
        if (!rows.is() || rowIndex_ < 0 || rowIndex_ >= rows->getCount()) {
            throw cpo::uno::RuntimeException(
                u"removeFromParent: the row is not attached to its table"_ustr);
        }
        rows->removeByIndex(rowIndex_, 1);
    }

private:
    cpo::uno::Reference<scriptinterop::XElement> parent_;
    cpo::uno::Reference<css::text::XTextTable> table_;
    sal_Int32 rowIndex_;
};

class TableImpl: public cppu::WeakImplHelper<scriptinterop::XTable> {
public:
    explicit TableImpl(
        cpo::uno::Reference<scriptinterop::XElement> const & parent,
        cpo::uno::Reference<css::text::XTextTable> const & table):
        parent_(parent), table_(table) {}

    cpo::uno::Reference<cpo::uno::XInterface> getuno() override { return table_; }

    cpo::uno::Reference<scriptinterop::XInlineImage> asInlineImage() override { return {}; }

    void clear() override {
        throw cpo::uno::RuntimeException(u"Table.clear is not yet implemented"_ustr); // TODO
    }

    // TODO: return a detached deep copy, not this; mutations on the "copy" write back to the live
    // element:
    cpo::uno::Reference<scriptinterop::XElement> copy() override { return this; }

    scriptinterop::ElementType getType() override { return scriptinterop::ElementType_TABLE; }

    sal_Int32 getNumRows() override {
        if (!table_.is()) {
            throw cpo::uno::RuntimeException(u"getNumRows: the element is not a table"_ustr);
        }
        auto const rows = table_->getRows();
        if (!rows.is()) {
            throw cpo::uno::RuntimeException(u"getNumRows: the table has no rows"_ustr);
        }
        return rows->getCount();
    }

    css::beans::Optional<cpo::uno::Reference<scriptinterop::XTableRow>> getRow(sal_Int32 index)
        override
    {
        if (index < 0 || index >= getNumRows()) {
            return {false, {}};
        }
        return {true,
            cpo::uno::Reference<scriptinterop::XTableRow>(new TableRowImpl(this, table_, index))};
    }

    cpo::uno::Reference<scriptinterop::XElement> getChild(sal_Int32 index) override {
        auto const row = getRow(index);
        return row.IsPresent ? row.Value : nullptr;
    }

    css::beans::Optional<cpo::uno::Reference<scriptinterop::XElement>> getNextSibling() override {
        return maybe(siblingContent(table_, parent_, true));
    }

    sal_Int32 getNumChildren() override { return getNumRows(); }

    css::beans::Optional<cpo::uno::Reference<scriptinterop::XElement>> getParent() override {
        return maybe(parent_);
    }

    css::beans::Optional<cpo::uno::Reference<scriptinterop::XElement>> getPreviousSibling() override
    { return maybe(siblingContent(table_, parent_, false)); }

    OUString getText() override {
        OUStringBuffer buf;
        auto const n = getNumRows();
        for (sal_Int32 i = 0; i != n; ++i) {
            auto const row = getRow(i);
            if (!row.IsPresent) {
                throw cpo::uno::RuntimeException(
                    "getText: the table has no row " + OUString::number(i));
            }
            if (!buf.isEmpty()) {
                buf.append('\n');
            }
            buf.append(row.Value->getText());
        }
        return buf.makeStringAndClear();
    }

    void removeFromParent() override {
        removeContent(cpo::uno::Reference<css::text::XTextContent>(table_, cpo::uno::UNO_QUERY_THROW));
    }

private:
    cpo::uno::Reference<scriptinterop::XElement> parent_;
    cpo::uno::Reference<css::text::XTextTable> table_;
};

cpo::uno::Sequence<cpo::uno::Reference<scriptinterop::XElement>> enumerateElements(
    cpo::uno::Reference<css::text::XText> const & text,
    cpo::uno::Reference<scriptinterop::XElement> const & parent)
{
    std::vector<cpo::uno::Reference<scriptinterop::XElement>> v;
    if (cpo::uno::Reference<css::container::XEnumerationAccess> const ea{text, cpo::uno::UNO_QUERY})
    {
        auto const en = ea->createEnumeration();
        while (en.is() && en->hasMoreElements()) {
            cpo::uno::Reference<css::text::XTextContent> xtc;
            en->nextElement() >>= xtc;
            if (!xtc.is()) {
                continue;
            }
            cpo::uno::Reference<css::lang::XServiceInfo> const info(xtc, cpo::uno::UNO_QUERY);
            if (!info.is()) {
                continue;
            }
            if (info->supportsService(u"com.sun.star.text.Paragraph"_ustr)) {
                v.emplace_back(new ParagraphImpl(parent, xtc));
            } else if (info->supportsService(u"com.sun.star.text.TextTable"_ustr)) {
                cpo::uno::Reference<css::text::XTextTable> const table(xtc, cpo::uno::UNO_QUERY);
                if (table.is()) {
                    v.emplace_back(new TableImpl(parent, table));
                }
            }
        }
    }
    return cpo::uno::Sequence(v.data(), v.size());
}

cpo::uno::Reference<scriptinterop::XElement> siblingContent(
    cpo::uno::Reference<css::text::XTextContent> const & content,
    cpo::uno::Reference<scriptinterop::XElement> const & parent, bool forward)
{
    if (!content.is()) {
        return nullptr;
    }
    cpo::uno::Reference<css::text::XTextRange> const range(content, cpo::uno::UNO_QUERY);
    if (!range.is()) {
        return nullptr;
    }
    auto const host = range->getText();
    if (!host.is()) {
        return nullptr;
    }
    auto const list = enumerateElements(host, parent);
    auto const n = list.getLength();
    cpo::uno::Reference<cpo::uno::XInterface> const self(content, cpo::uno::UNO_QUERY);
    for (sal_Int32 i = 0; i != n; ++i) {
        auto const & elem = list[i];
        if (!elem.is()) {
            continue;
        }
        cpo::uno::Reference<cpo::uno::XInterface> const other(elem->getuno(), cpo::uno::UNO_QUERY);
        if (self.get() == other.get()) {
            auto const j = forward ? i + 1 : i - 1;
            return j >= 0 && j < n ? list[j] : nullptr;
        }
    }
    return nullptr;
}

// Walk the containing XText's paragraphs and return the one whose extent covers `marker`s start;
// null if the walk finds no paragraph or if the ranges live in different Text hosts:
cpo::uno::Reference<css::text::XTextContent> findContainingParagraph(
    cpo::uno::Reference<css::text::XTextRange> const & marker)
{
    assert(marker.is());
    auto const containingText = marker->getText();
    if (!containingText.is()) {
        throw cpo::uno::RuntimeException(u"the text position is not part of the document"_ustr);
    }
    cpo::uno::Reference<css::text::XTextRangeCompare> const cmp(
        containingText, cpo::uno::UNO_QUERY_THROW);
    cpo::uno::Reference<css::container::XEnumerationAccess> const ea(
        containingText, cpo::uno::UNO_QUERY_THROW);
    auto const en = ea->createEnumeration();
    while (en.is() && en->hasMoreElements()) {
        cpo::uno::Reference<css::text::XTextContent> xtc;
        en->nextElement() >>= xtc;
        if (!xtc.is()) {
            continue;
        }
        cpo::uno::Reference<css::lang::XServiceInfo> const info(xtc, cpo::uno::UNO_QUERY);
        if (!info.is() || !info->supportsService(u"com.sun.star.text.Paragraph"_ustr)) {
            continue;
        }
        cpo::uno::Reference<css::text::XTextRange> const paraRange(xtc, cpo::uno::UNO_QUERY);
        if (!paraRange.is()) {
            continue;
        }
        try {
            if (cmp->compareRegionStarts(marker->getStart(), paraRange->getStart()) <= 0
                && cmp->compareRegionStarts(marker->getStart(), paraRange->getEnd()) >= 0)
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
    explicit RangeElementImpl(cpo::uno::Reference<css::text::XTextRange> const & range):
        range_(range), paragraph_(findContainingParagraph(range))
    {
        rangeLen_ = range->getString().getLength();
        if (!paragraph_.is()) {
            return;
        }
        cpo::uno::Reference<css::text::XTextRange> const paraRange(
            paragraph_, cpo::uno::UNO_QUERY_THROW);
        paragraphLen_ = paraRange->getString().getLength();
        auto const host = paraRange->getText();
        if (!host.is()) {
            throw cpo::uno::RuntimeException(u"the paragraph is not part of the document"_ustr);
        }
        auto const probe = host->createTextCursorByRange(paraRange->getStart());
        if (!probe.is()) {
            throw cpo::uno::RuntimeException(u"cannot place a cursor in the paragraph"_ustr);
        }
        probe->gotoRange(range->getStart(), true);
        startOffset_ = probe->getString().getLength();
    }

    cpo::uno::Reference<cpo::uno::XInterface> getuno() override { return range_; }

    cpo::uno::Reference<scriptinterop::XParagraph> getElement() override {
        if (!paragraph_.is()) {
            throw cpo::uno::RuntimeException(
                u"getElement: this range element is not inside a paragraph"_ustr);
        }
        return new ParagraphImpl(nullptr, paragraph_);
    }

    sal_Int32 getEndOffsetInclusive() override {
        if (!isPartial()) {
            return -1;
        }
        return rangeLen_ == 0 ? startOffset_ : startOffset_ + rangeLen_ - 1;
    }

    sal_Int32 getStartOffset() override {
        if (!isPartial()) {
            return -1;
        }
        return startOffset_;
    }

    bool isPartial() override {
        if (!paragraph_.is()) {
            return true;
        }
        return startOffset_ != 0 || startOffset_ + rangeLen_ != paragraphLen_;
    }

private:
    cpo::uno::Reference<css::text::XTextRange> range_;
    cpo::uno::Reference<css::text::XTextContent> paragraph_;
    sal_Int32 startOffset_ = 0;
    sal_Int32 rangeLen_ = 0;
    sal_Int32 paragraphLen_ = 0;
};

// Walk the host XText and push one RangeElementImpl per paragraph that the raw range overlaps.
// The sub-range for each paragraph is the intersection of the raw range with the paragraph's
// extent, so a paragraph that lies fully inside the raw range becomes a non-partial element and
// the first and last paragraphs become partial ones:
void splitAtParagraphBoundaries(
    cpo::uno::Reference<css::text::XTextRange> const & range,
    std::vector<cpo::uno::Reference<scriptinterop::XRangeElement>> & out)
{
    assert(range.is());
    auto const host = range->getText();
    if (!host.is()) {
        throw cpo::uno::RuntimeException(u"the selected range is not part of the document"_ustr);
    }
    cpo::uno::Reference<css::text::XTextRangeCompare> const cmp(host, cpo::uno::UNO_QUERY_THROW);
    cpo::uno::Reference<css::container::XEnumerationAccess> const ea(
        host, cpo::uno::UNO_QUERY_THROW);
    auto const rStart = range->getStart();
    auto const rEnd = range->getEnd();
    auto const en = ea->createEnumeration();
    while (en.is() && en->hasMoreElements()) {
        cpo::uno::Reference<css::text::XTextContent> xtc;
        en->nextElement() >>= xtc;
        if (!xtc.is()) {
            continue;
        }
        cpo::uno::Reference<css::lang::XServiceInfo> const info(xtc, cpo::uno::UNO_QUERY);
        if (!info.is() || !info->supportsService(u"com.sun.star.text.Paragraph"_ustr)) {
            continue;
        }
        cpo::uno::Reference<css::text::XTextRange> const pRange(xtc, cpo::uno::UNO_QUERY);
        if (!pRange.is()) {
            continue;
        }
        try {
            if (cmp->compareRegionStarts(pRange->getEnd(), rStart) > 0) {
                continue;
            }
            if (cmp->compareRegionStarts(pRange->getStart(), rEnd) < 0) {
                break;
            }
            auto const subStart = cmp->compareRegionStarts(rStart, pRange->getStart()) <= 0
                ? rStart : pRange->getStart();
            auto const subEnd = cmp->compareRegionStarts(rEnd, pRange->getEnd()) >= 0
                ? rEnd : pRange->getEnd();
            auto const cursor = host->createTextCursorByRange(subStart);
            if (!cursor.is()) {
                continue;
            }
            cursor->gotoRange(subEnd, true);
            out.emplace_back(new RangeElementImpl(cursor));
        } catch (css::lang::IllegalArgumentException const & e) {
            SAL_WARN(
                "scriptinterop",
                "splitAtParagraphBoundaries: compareRegionStarts failed: " << e.Message);
        }
    }
}

cpo::uno::Sequence<cpo::uno::Reference<scriptinterop::XRangeElement>>
SelectionImpl::getRangeElements() {
    std::vector<cpo::uno::Reference<scriptinterop::XRangeElement>> v;
    auto const n = ranges_->getCount();
    for (sal_Int32 i = 0; i != n; ++i) {
        cpo::uno::Reference<css::text::XTextRange> range;
        if (!(ranges_->getByIndex(i) >>= range) || !range.is()) {
            throw cpo::uno::RuntimeException(
                u"getRangeElements: the selection contains something other than text"_ustr);
        }
        splitAtParagraphBoundaries(range, v);
    }
    return cpo::uno::Sequence(v.data(), v.size());
}

class CursorImpl: public cppu::WeakImplHelper<scriptinterop::XCursor> {
public:
    explicit CursorImpl(cpo::uno::Reference<css::frame::XModel> const & model): model_(model) {}

    cpo::uno::Reference<cpo::uno::XInterface> getuno() override { return viewCursor(); }

    cpo::uno::Reference<scriptinterop::XParagraph> getElement() override {
        auto const para = findContainingParagraph(viewCursor());
        if (!para.is()) {
            throw cpo::uno::RuntimeException(
                u"getElement: the cursor is not inside a paragraph"_ustr);
        }
        return new ParagraphImpl(nullptr, para);
    }

    sal_Int32 getOffset() override {
        auto const c = viewCursor();
        auto const para = findContainingParagraph(c);
        if (!para.is()) {
            throw cpo::uno::RuntimeException(
                u"getOffset: the cursor is not inside a paragraph"_ustr);
        }
        cpo::uno::Reference<css::text::XTextRange> const paraRange(para, cpo::uno::UNO_QUERY_THROW);
        auto const host = paraRange->getText();
        if (!host.is()) {
            throw cpo::uno::RuntimeException(
                u"getOffset: the paragraph is not part of the document"_ustr);
        }
        auto const probe = host->createTextCursorByRange(paraRange->getStart());
        if (!probe.is()) {
            throw cpo::uno::RuntimeException(
                u"getOffset: cannot place a cursor in the paragraph"_ustr);
        }
        probe->gotoRange(c->getStart(), true);
        return probe->getString().getLength();
    }

    cpo::uno::Reference<scriptinterop::XText> getSurroundingText() override {
        auto const para = findContainingParagraph(viewCursor());
        if (!para.is()) {
            throw cpo::uno::RuntimeException(
                u"getSurroundingText: the cursor is not inside a paragraph"_ustr);
        }
        return new TextImpl(nullptr, para, scriptinterop::ElementType_TEXT);
    }

    sal_Int32 getSurroundingTextOffset() override { return getOffset(); }

    css::beans::Optional<cpo::uno::Reference<scriptinterop::XInlineImage>> insertInlineImage(
        cpo::uno::Reference<scriptinterop::XBlob> const & blob) override
    {
        auto const c = viewCursor();
        auto const host = c->getText();
        if (!host.is()) {
            throw cpo::uno::RuntimeException(
                u"insertInlineImage: the cursor is not somewhere that accepts inline images"_ustr);
        }
        auto const graphic = createGraphicFromBlob(model_, blob);
        host->insertTextContent(c->getStart(), graphic, false);
        return {true, new InlineImageImpl(nullptr, graphic)};
    }

    void insertText(OUString const & text) override {
        auto const c = viewCursor();
        auto const host = c->getText();
        if (!host.is()) {
            throw cpo::uno::RuntimeException(
                u"insertText: the cursor is not somewhere that accepts text"_ustr);
        }
        host->insertString(c->getStart(), text, false);
    }

private:
    cpo::uno::Reference<css::text::XTextViewCursor> viewCursor() {
        cpo::uno::Reference<css::text::XTextViewCursorSupplier> const sup(
            model_->getCurrentController(), cpo::uno::UNO_QUERY_THROW);
        auto const c = sup->getViewCursor();
        if (!c.is()) {
            throw cpo::uno::RuntimeException(u"the document view has no cursor"_ustr);
        }
        return c;
    }

    cpo::uno::Reference<css::frame::XModel> model_;
};

class FootnoteSectionImpl: public cppu::WeakImplHelper<scriptinterop::XContainerElement> {
public:
    explicit FootnoteSectionImpl(cpo::uno::Reference<css::text::XText> const & text):
        text_(text) {}

    cpo::uno::Reference<cpo::uno::XInterface> getuno() override { return text_; }

    cpo::uno::Reference<scriptinterop::XInlineImage> asInlineImage() override { return {}; }

    void clear() override {
        if (!text_.is()) {
            throw cpo::uno::RuntimeException(u"clear: the footnote section has no text"_ustr);
        }
        text_->setString(OUString());
    }

    // TODO: return a detached deep copy, not this; mutations on the "copy" write back to the live
    // element:
    cpo::uno::Reference<scriptinterop::XElement> copy() override { return this; }

    cpo::uno::Reference<scriptinterop::XElement> getChild(sal_Int32 index) override {
        auto const list = enumerateElements(text_, this);
        return index >= 0 && index < list.getLength() ? list[index] : nullptr;
    }

    // TODO: a footnote section is not part of a sibling list in our model:
    css::beans::Optional<cpo::uno::Reference<scriptinterop::XElement>> getNextSibling() override {
        return {false, {}};
    }

    sal_Int32 getNumChildren() override { return enumerateElements(text_, this).getLength(); }

    // TODO: no natural container for a footnote section in scriptinterop (GAS's Document analogue
    // would be that parent, but we do not model it):
    css::beans::Optional<cpo::uno::Reference<scriptinterop::XElement>> getParent() override {
        return {false, {}};
    }

    css::beans::Optional<cpo::uno::Reference<scriptinterop::XElement>> getPreviousSibling() override
    { return {false, {}}; }

    OUString getText() override {
        if (!text_.is()) {
            throw cpo::uno::RuntimeException(u"getText: the footnote section has no text"_ustr);
        }
        return text_->getString();
    }

    scriptinterop::ElementType getType() override {
        return scriptinterop::ElementType_FOOTNOTE_SECTION;
    }

    void removeFromParent() override {
        throw cpo::uno::RuntimeException(
            u"a footnote section cannot be removed on its own; remove its footnote instead"_ustr);
    }

private:
    cpo::uno::Reference<css::text::XText> text_;
};

class FootnoteImpl: public cppu::WeakImplHelper<scriptinterop::XFootnote> {
public:
    explicit FootnoteImpl(cpo::uno::Reference<css::text::XFootnote> const & footnote):
        footnote_(footnote) {}

    cpo::uno::Reference<cpo::uno::XInterface> getuno() override { return footnote_; }

    cpo::uno::Reference<scriptinterop::XInlineImage> asInlineImage() override { return {}; }

    // TODO: return a detached deep copy, not this; mutations on the "copy" write back to the live
    // element:
    cpo::uno::Reference<scriptinterop::XElement> copy() override { return this; }

    css::beans::Optional<cpo::uno::Reference<scriptinterop::XContainerElement>>
        getFootnoteContents() override
    {
        cpo::uno::Reference<css::text::XText> const text(footnote_, cpo::uno::UNO_QUERY);
        if (!text.is()) return {false, {}};
        return {true, new FootnoteSectionImpl(text)};
    }

    // TODO: footnotes are surfaced via Document.getFootnotes rather than a sibling walk in our
    // model:
    css::beans::Optional<cpo::uno::Reference<scriptinterop::XElement>> getNextSibling() override {
        return {false, {}};
    }

    // TODO: GAS's Footnote.getParent is the paragraph anchoring the footnote; scriptinterop does
    // not currently track that link:
    css::beans::Optional<cpo::uno::Reference<scriptinterop::XElement>> getParent() override {
        return {false, {}};
    }

    css::beans::Optional<cpo::uno::Reference<scriptinterop::XElement>> getPreviousSibling() override
    { return {false, {}}; }

    OUString getText() override {
        return cpo::uno::Reference<css::text::XText>(footnote_, cpo::uno::UNO_QUERY_THROW)
            ->getString();
    }

    scriptinterop::ElementType getType() override { return scriptinterop::ElementType_FOOTNOTE; }

    void removeFromParent() override {
        removeContent(
            cpo::uno::Reference<css::text::XTextContent>(footnote_, cpo::uno::UNO_QUERY_THROW));
    }

private:
    cpo::uno::Reference<css::text::XFootnote> footnote_;
};

class BodyImpl: public cppu::WeakImplHelper<scriptinterop::XBody> {
public:
    explicit BodyImpl(
        cpo::uno::Reference<css::frame::XModel> const & model,
        cpo::uno::Reference<css::text::XText> const & text):
        model_(model), text_(text) {}

    cpo::uno::Reference<scriptinterop::XInlineImage> appendImage(
        cpo::uno::Reference<scriptinterop::XBlob> const & blob) override
    {
        if (!text_.is()) {
            throw cpo::uno::RuntimeException(u"XBody has no underlying text"_ustr);
        }
        auto const graphic = createGraphicFromBlob(model_, blob);
        auto const cursor = text_->createTextCursorByRange(text_->getEnd());
        text_->insertControlCharacter(cursor, css::text::ControlCharacter::PARAGRAPH_BREAK, false);
        text_->insertTextContent(cursor, graphic, false);
        return new InlineImageImpl(this, graphic);
    }

    cpo::uno::Reference<scriptinterop::XParagraph> appendListItem(OUString const & text) override {
        return appendImpl(text, u"List Number"_ustr);
    }

    cpo::uno::Reference<scriptinterop::XParagraph> appendParagraph(OUString const & text) override {
        return appendImpl(text, u""_ustr);
    }

    cpo::uno::Reference<scriptinterop::XInlineImage> asInlineImage() override { return {}; }

    void clear() override {
        throw cpo::uno::RuntimeException(u"Body.clear is not yet implemented"_ustr); // TODO
    }

    // TODO: return a detached deep copy, not this; mutations on the "copy" write back to the live
    // element:
    cpo::uno::Reference<scriptinterop::XElement> copy() override { return this; }

    cpo::uno::Reference<cpo::uno::XInterface> getuno() override { return text_; }

    cpo::uno::Reference<scriptinterop::XElement> getChild(sal_Int32 index) override {
        auto const list = getChildren();
        return index >= 0 && index < list.getLength() ? list[index] : nullptr;
    }

    cpo::uno::Sequence<cpo::uno::Reference<scriptinterop::XElement>> getChildren() override {
        return enumerateElements(text_, this);
    }

    css::beans::Optional<cpo::uno::Reference<scriptinterop::XElement>> getNextSibling() override {
        return {false, {}};
    }

    sal_Int32 getNumChildren() override { return getChildren().getLength(); }

    css::beans::Optional<cpo::uno::Reference<scriptinterop::XElement>> getParent() override {
        return {false, {}};
    }

    css::beans::Optional<cpo::uno::Reference<scriptinterop::XElement>> getPreviousSibling() override
    { return {false, {}}; }

    OUString getText() override {
        if (!text_.is()) {
            throw cpo::uno::RuntimeException(u"getText: the document body has no text"_ustr);
        }
        return text_->getString();
    }

    scriptinterop::ElementType getType() override {
        return scriptinterop::ElementType_BODY_SECTION;
    }

    void removeFromParent() override {
        throw cpo::uno::RuntimeException(u"the body has no parent to remove it from"_ustr);
    }

private:
    cpo::uno::Reference<scriptinterop::XParagraph> appendImpl(
        OUString const & text, OUString const & paraStyle)
    {
        if (!text_.is()) {
            throw cpo::uno::RuntimeException(u"XBody has no underlying text"_ustr);
        }
        auto const cursor = text_->createTextCursorByRange(text_->getEnd());
        text_->insertControlCharacter(
            cursor, css::text::ControlCharacter::PARAGRAPH_BREAK, false);
        text_->insertString(cursor, text, false);
        cpo::uno::Reference<css::text::XTextContent> lastParagraph;
        if (cpo::uno::Reference<css::container::XEnumerationAccess> const ea{
                text_, cpo::uno::UNO_QUERY})
        {
            auto const en = ea->createEnumeration();
            while (en.is() && en->hasMoreElements()) {
                cpo::uno::Reference<css::text::XTextContent> xtc;
                en->nextElement() >>= xtc;
                if (!xtc.is()) {
                    continue;
                }
                cpo::uno::Reference<css::lang::XServiceInfo> const info(
                    xtc, cpo::uno::UNO_QUERY);
                if (!info.is() || !info->supportsService(u"com.sun.star.text.Paragraph"_ustr)) {
                    continue;
                }
                lastParagraph = xtc;
            }
        }
        if (!lastParagraph.is()) {
            throw cpo::uno::RuntimeException(u"appending the paragraph failed"_ustr);
        }
        if (!paraStyle.isEmpty()) {
            cpo::uno::Reference<css::beans::XPropertySet> const props(
                lastParagraph, cpo::uno::UNO_QUERY_THROW);
            try {
                props->setPropertyValue(u"ParaStyleName"_ustr, cpo::uno::Any(paraStyle));
            } catch (css::lang::IllegalArgumentException const &) {
                // Document lacks the requested style; build a bullet NumberingRules from
                // scratch and apply it, so the paragraph is a real list item regardless of
                // which paragraph styles the document has registered:
                applyBulletNumbering(props);
            }
        }
        return new ParagraphImpl(this, lastParagraph);
    }

    void applyBulletNumbering(cpo::uno::Reference<css::beans::XPropertySet> const & props) {
        cpo::uno::Reference<css::lang::XMultiServiceFactory> const factory(
            model_, cpo::uno::UNO_QUERY_THROW);
        cpo::uno::Reference<css::container::XIndexReplace> const rules(
            factory->createInstance(u"com.sun.star.text.NumberingRules"_ustr),
            cpo::uno::UNO_QUERY_THROW);
        if (rules->getCount() == 0) {
            throw cpo::uno::RuntimeException(
                u"appendListItem: the document provides no bullet list formatting"_ustr);
        }
        rules->replaceByIndex(
            0,
            cpo::uno::Any(
                cpo::uno::Sequence<css::beans::PropertyValue>{
                    {u"NumberingType"_ustr, 0,
                     cpo::uno::Any(sal_Int16(css::style::NumberingType::ARABIC)),
                     css::beans::PropertyState_DIRECT_VALUE}}));
        props->setPropertyValue(u"NumberingRules"_ustr, cpo::uno::Any(rules));
        props->setPropertyValue(u"NumberingIsNumber"_ustr, cpo::uno::Any(true));
    }

    cpo::uno::Reference<css::frame::XModel> model_;
    cpo::uno::Reference<css::text::XText> text_;
};

class DocumentImpl : public cppu::WeakImplHelper<scriptinterop::XDocument>
{
public:
    explicit DocumentImpl(cpo::uno::Reference<css::frame::XModel> const& model)
        : model_(model)
    {
    }

    cpo::uno::Reference<cpo::uno::XInterface> SAL_CALL getuno() override { return model_; }

    css::beans::Optional<cpo::uno::Reference<scriptinterop::XSelection>> SAL_CALL getSelection()
        override
    {
        cpo::uno::Reference<css::text::XTextDocument> const doc(model_, cpo::uno::UNO_QUERY_THROW);
        cpo::uno::Reference<css::view::XSelectionSupplier> const sup(
            doc->getCurrentController(), cpo::uno::UNO_QUERY_THROW);
        cpo::uno::Reference<css::container::XIndexAccess> ranges;
        sup->getSelection() >>= ranges;
        if (!ranges.is()) {
            return {false, {}};
        }
        bool anyContent = false;
        auto const n = ranges->getCount();
        for (sal_Int32 i = 0; i != n; ++i) {
            cpo::uno::Reference<css::text::XTextRange> range;
            if (!(ranges->getByIndex(i) >>= range) || !range.is()) {
                return {false, {}};
            }
            if (!range->getString().isEmpty()) {
                anyContent = true;
                break;
            }
        }
        if (!anyContent) {
            return {false, {}};
        }
        return {true, cpo::uno::Reference<scriptinterop::XSelection>(new SelectionImpl(ranges))};
    }

    cpo::uno::Reference<scriptinterop::XBody> getBody() override
    {
        cpo::uno::Reference<css::text::XTextDocument> const doc(model_, cpo::uno::UNO_QUERY_THROW);
        return new BodyImpl(model_, doc->getText());
    }

    css::beans::Optional<cpo::uno::Reference<scriptinterop::XCursor>> getCursor() override
    {
        return {true, cpo::uno::Reference<scriptinterop::XCursor>(new CursorImpl(model_))};
    }

    cpo::uno::Reference<scriptinterop::XRangeBuilder> newRange() override {
        return new RangeBuilderImpl;
    }

    void setSelection(cpo::uno::Reference<scriptinterop::XSelection> const & selection) override {
        if (!selection.is()) {
            throw cpo::uno::RuntimeException(u"setSelection: the selection must not be null"_ustr);
        }
        cpo::uno::Reference<css::view::XSelectionSupplier> const sup(
            model_->getCurrentController(), cpo::uno::UNO_QUERY_THROW);
        // Writer's select typically only recognises its own SwXTextRanges; when our XSelection
        // wraps a single XTextRange, unwrap and pass that directly so Writer accepts it:
        cpo::uno::Reference<css::container::XIndexAccess> const idx(
            selection->getuno(), cpo::uno::UNO_QUERY);
        if (idx.is() && idx->getCount() == 1) {
            cpo::uno::Reference<css::text::XTextRange> range;
            idx->getByIndex(0) >>= range;
            if (range.is()) {
                sup->select(cpo::uno::Any(range));
                return;
            }
        }
        sup->select(cpo::uno::Any(selection->getuno()));
    }

    cpo::uno::Sequence<cpo::uno::Reference<scriptinterop::XFootnote>> getFootnotes() override {
        std::vector<cpo::uno::Reference<scriptinterop::XFootnote>> v;
        cpo::uno::Reference<css::text::XFootnotesSupplier> const sup(
            model_, cpo::uno::UNO_QUERY_THROW);
        auto const idx = sup->getFootnotes();
        if (!idx.is()) {
            throw cpo::uno::RuntimeException(
                u"getFootnotes: the document has no footnote list"_ustr);
        }
        auto const n = idx->getCount();
        for (sal_Int32 i = 0; i != n; ++i) {
            cpo::uno::Reference<css::text::XFootnote> footnote;
            if (!(idx->getByIndex(i) >>= footnote) || !footnote.is()) {
                throw cpo::uno::RuntimeException(
                    u"getFootnotes: the document's footnote list contains something that is not a"
                    " footnote"_ustr);
            }
            v.emplace_back(new FootnoteImpl(footnote));
        }
        return cpo::uno::Sequence(v.data(), v.size());
    }

    void SAL_CALL insertImage(cpo::uno::Sequence<sal_Int8> const& data,
                              scriptinterop::ImageOptions const& opts) override
    {
        cpo::uno::Reference<css::text::XTextDocument> const doc(model_, cpo::uno::UNO_QUERY_THROW);
        auto const componentCtx = comphelper::getProcessComponentContext();
        auto const smgr = componentCtx->getServiceManager();
        // Stage the bytes in a TempFile to give GraphicProvider a file URL:
        cpo::uno::Reference<css::io::XTempFile> const tmp(
            smgr->createInstanceWithContext(u"com.sun.star.io.TempFile"_ustr, componentCtx),
            cpo::uno::UNO_QUERY_THROW);
        tmp->getOutputStream()->writeBytes(data);
        tmp->getOutputStream()->closeOutput();
        cpo::uno::Reference<css::graphic::XGraphicProvider> const gp(
            smgr->createInstanceWithContext(u"com.sun.star.graphic.GraphicProvider"_ustr,
                                            componentCtx),
            cpo::uno::UNO_QUERY_THROW);
        cpo::uno::Sequence<css::beans::PropertyValue> loaderArgs{
            { u"URL"_ustr, 0, cpo::uno::Any(tmp->getUri()), {} }
        };
        auto const xgraphic = gp->queryGraphic(loaderArgs);
        if (!xgraphic.is())
        {
            throw cpo::uno::RuntimeException(u"insertImage: failed to load graphic"_ustr);
        }
        cpo::uno::Reference<css::lang::XMultiServiceFactory> const docFactory(
            doc, cpo::uno::UNO_QUERY_THROW);
        cpo::uno::Reference<css::text::XTextContent> const graphic(
            docFactory->createInstance(u"com.sun.star.text.TextGraphicObject"_ustr),
            cpo::uno::UNO_QUERY_THROW);
        cpo::uno::Reference<css::beans::XPropertySet> const props(
            graphic, cpo::uno::UNO_QUERY_THROW);
        props->setPropertyValue(u"Graphic"_ustr, cpo::uno::Any(xgraphic));
        // Width and Height are in 1/100 mm:
        props->setPropertyValue(
            u"Width"_ustr, cpo::uno::Any(static_cast<sal_Int32>(std::round(opts.widthCm * 1000))));
        props->setPropertyValue(u"Height"_ustr, cpo::uno::Any(static_cast<sal_Int32>(
                                                    std::round(opts.heightCm * 1000))));
        props->setPropertyValue(u"AnchorType"_ustr,
                                cpo::uno::Any(css::text::TextContentAnchorType_AS_CHARACTER));
        cpo::uno::Reference<css::text::XTextViewCursorSupplier> const cursorSupplier(
            doc->getCurrentController(), cpo::uno::UNO_QUERY_THROW);
        auto const cursor = cursorSupplier->getViewCursor();
        if (!cursor.is())
        {
            throw cpo::uno::RuntimeException(u"insertImage: no view cursor"_ustr);
        }
        doc->getText()->insertTextContent(cursor, graphic, false);
    }

private:
    cpo::uno::Reference<css::frame::XModel> model_;
};
}

namespace scriptinterop::detail
{
cpo::uno::Reference<scriptinterop::XDocument>
createDocument(cpo::uno::Reference<css::frame::XModel> const& model)
{
    return new DocumentImpl(model);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
