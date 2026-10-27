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

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <iterator>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>

#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/awt/FontStrikeout.hpp>
#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/awt/Size.hpp>
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
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/graphic/XGraphicProvider.hpp>
#include <com/sun/star/io/XTempFile.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/style/NumberingType.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/table/XCell.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/text/ControlCharacter.hpp>
#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/RelOrientation.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/text/WrapTextMode.hpp>
#include <com/sun/star/text/WritingMode2.hpp>
#include <com/sun/star/text/XFootnote.hpp>
#include <com/sun/star/text/XFootnotesSupplier.hpp>
#include <com/sun/star/text/XParagraphAppend.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XTextContent.hpp>
#include <com/sun/star/text/XTextCursor.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XTextGraphicObjectsSupplier.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/text/XTextRangeCompare.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/text/XTextTableCursor.hpp>
#include <com/sun/star/text/XTextTablesSupplier.hpp>
#include <com/sun/star/text/XTextViewCursor.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/util/SearchAlgorithms2.hpp>
#include <com/sun/star/util/SearchOptions2.hpp>
#include <com/sun/star/util/TextSearch2.hpp>
#include <com/sun/star/util/XModifyBroadcaster.hpp>
#include <com/sun/star/util/XModifyListener.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <cpo/uno/Reference.hxx>
#include <cpo/uno/RuntimeException.hpp>
#include <cpo/uno/XComponentContext.hpp>
#include <cpo/uno/XInterface.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <comphelper/diagnose_ex.hxx>
#include <comphelper/processfactory.hxx>
#include <cpo/uno/Any.hxx>
#include <cpo/uno/Exception.hdl>
#include <cpo/uno/Sequence.hxx>
#include <cppu/unotype.hxx>
#include <cppuhelper/implbase.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <o3tl/safeint.hxx>
#include <o3tl/string_view.hxx>
#include <o3tl/unit_conversion.hxx>
#include <rtl/character.hxx>
#include <rtl/ref.hxx>
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
#include <scriptinterop/XPositionedImage.hpp>
#include <scriptinterop/XRangeBuilder.hpp>
#include <scriptinterop/XRangeElement.hpp>
#include <scriptinterop/XSelection.hpp>
#include <scriptinterop/XTable.hpp>
#include <scriptinterop/XTableCell.hpp>
#include <scriptinterop/XTableRow.hpp>
#include <scriptinterop/XText.hpp>
#include <tools/color.hxx>
#include <tools/debug.hxx>
#include <unicode/regex.h>

#include "document.hxx"

namespace
{
template<typename T>
css::beans::Optional<cpo::uno::Reference<T>> maybe(cpo::uno::Reference<T> const & ref)
{ return {ref.is(), ref}; }

class ModificationsListener: public cppu::WeakImplHelper<css::util::XModifyListener> {
public:
    std::uintmax_t generation() const { return generation_; }

    void modified(css::lang::EventObject const &) override { incrementGeneration(); }

    void disposing(css::lang::EventObject const &) override { incrementGeneration(); }

private:
    void incrementGeneration() {
        DBG_TESTSOLARMUTEX();
        // Overflow is arguably harmless for practical purposes:
        ++generation_;
    }

    std::uintmax_t generation_ = 0;
};

template<typename T> class Cached {
public:
    template<typename F> T const & get(std::uintmax_t generation, F compute) {
        DBG_TESTSOLARMUTEX();
        if (!value_ || generation_ != generation) {
            value_ = compute();
            generation_ = generation;
        }
        return *value_;
    }

private:
    std::optional<T> value_;
    std::uintmax_t generation_;
};

class Model {
public:
    explicit Model(cpo::uno::Reference<css::frame::XModel> const & model):
        model_(model), listener_(new ModificationsListener)
    {
        cpo::uno::Reference<css::util::XModifyBroadcaster>(model_, cpo::uno::UNO_QUERY_THROW)
            ->addModifyListener(listener_);
    }

    ~Model() {
        try {
            cpo::uno::Reference<css::util::XModifyBroadcaster>(model_, cpo::uno::UNO_QUERY_THROW)
                ->removeModifyListener(listener_);
        } catch (css::lang::DisposedException const &) {
        } catch (cpo::uno::Exception const &) {
            TOOLS_WARN_EXCEPTION("scriptinterop", "removing the modify listener");
        }
    }

    Model(Model const &) = delete;
    Model & operator =(Model const &) = delete;

    cpo::uno::Reference<css::frame::XModel> const & uno() const { return model_; }

    std::uintmax_t generation() const { return listener_->generation(); }

    // The paragraphs and tables of the document's body:
    Cached<std::vector<cpo::uno::Reference<css::text::XTextContent>>> & bodyContents() {
        return bodyContents_;
    }

private:
    cpo::uno::Reference<css::frame::XModel> model_;
    rtl::Reference<ModificationsListener> listener_;
    Cached<std::vector<cpo::uno::Reference<css::text::XTextContent>>> bodyContents_;
};

// The Model of a document model, which all users of that document model share for as long as any of
// them exists:
std::shared_ptr<Model> sharedModel(cpo::uno::Reference<css::frame::XModel> const & model) {
    DBG_TESTSOLARMUTEX();
    // Keys are XInterface rather than XModel pointers for guaranteed equality comparison, and the
    // Model entries' model_ members keep them alive:
    static std::map<cpo::uno::XInterface *, std::weak_ptr<Model>> models;
    std::erase_if(models, [](auto const & entry) { return entry.second.expired(); });
    auto & entry = models[
        cpo::uno::Reference<cpo::uno::XInterface>(model, cpo::uno::UNO_QUERY).get()];
    auto shared = entry.lock();
    if (!shared) {
        shared = std::make_shared<Model>(model);
        entry = shared;
    }
    return shared;
}

// The length of the line end at offset in string, which is zero at the end of string:
sal_Int32 lineEndAt(OUString const & string, sal_Int32 offset) {
    if (offset == string.getLength()) {
        return 0;
    }
    if (string.match(u"\r\n", offset)) {
        return 2;
    }
    if (string.match(u"\n", offset)) {
        return 1;
    }
    throw cpo::uno::RuntimeException(
        "textOf: expected a line end at offset " + OUString::number(offset));
}

// The offset and the length of the number of each footnote reference in string, which is the
// string of range, in increasing order:
std::vector<std::pair<sal_Int32, sal_Int32>> footnoteNumbers(
    cpo::uno::Reference<css::text::XTextRange> const & range, OUString const & string)
{
    std::vector<std::pair<sal_Int32, sal_Int32>> numbers;
    auto const host = range->getText();
    // A table's text starts after the line end that follows the paragraph or table before it, and
    // a table's text is the strings of its cells, each followed by a line end except the last one:
    cpo::uno::Reference<css::text::XTextRange> previousParagraph;
    sal_Int32 afterPreviousTable = 0;
    // A text cursor cannot start in a table, as at the start of a table cell that begins with a
    // nested table, so the whole of a text enumerates its elements itself:
    cpo::uno::Reference<css::container::XEnumerationAccess> elements;
    if (cpo::uno::Reference<css::text::XText>(range, cpo::uno::UNO_QUERY).is()) {
        elements.set(range, cpo::uno::UNO_QUERY_THROW);
    } else {
        elements.set(host->createTextCursorByRange(range), cpo::uno::UNO_QUERY_THROW);
    }
    for (auto const en = elements->createEnumeration(); en->hasMoreElements();) {
        auto const element = en->nextElement();
        cpo::uno::Reference<css::text::XTextTable> const table(element, cpo::uno::UNO_QUERY);
        if (table.is()) {
            sal_Int32 offset = afterPreviousTable;
            if (previousParagraph.is()) {
                auto const before = host->createTextCursorByRange(range->getStart());
                before->gotoRange(previousParagraph->getEnd(), true);
                offset = before->getString().getLength();
                offset += lineEndAt(string, offset);
            }
            cpo::uno::Reference<css::table::XCellRange> const cells(
                table, cpo::uno::UNO_QUERY_THROW);
            auto const rowCount = table->getRows()->getCount();
            bool first = true;
            for (sal_Int32 row = 0; row != rowCount; ++row) {
                for (sal_Int32 column = 0;; ++column) {
                    cpo::uno::Reference<css::text::XText> cell;
                    try {
                        cell.set(cells->getCellByPosition(column, row), cpo::uno::UNO_QUERY_THROW);
                    } catch (css::lang::IndexOutOfBoundsException const &) {
                        break;
                    }
                    if (!first) {
                        offset += lineEndAt(string, offset);
                    }
                    first = false;
                    auto const cellString = cell->getString();
                    if (!string.match(cellString, offset)) {
                        throw cpo::uno::RuntimeException(
                            "textOf: expected the text of a table cell at offset "
                            + OUString::number(offset));
                    }
                    for (auto const & number: footnoteNumbers(cell, cellString)) {
                        numbers.emplace_back(offset + number.first, number.second);
                    }
                    offset += cellString.getLength();
                }
            }
            afterPreviousTable = offset + lineEndAt(string, offset);
            previousParagraph.clear();
            continue;
        }
        cpo::uno::Reference<css::container::XEnumerationAccess> const portions(
            element, cpo::uno::UNO_QUERY_THROW);
        for (auto const pen = portions->createEnumeration(); pen->hasMoreElements();) {
            cpo::uno::Reference<css::beans::XPropertySet> const portion(
                pen->nextElement(), cpo::uno::UNO_QUERY_THROW);
            OUString type;
            portion->getPropertyValue(u"TextPortionType"_ustr) >>= type;
            if (type == u"Footnote") {
                cpo::uno::Reference<css::text::XTextRange> const footnote(
                    portion, cpo::uno::UNO_QUERY_THROW);
                auto const before = host->createTextCursorByRange(range->getStart());
                before->gotoRange(footnote->getStart(), true);
                // The string of an empty selection right at a footnote reference is the number of
                // that footnote:
                numbers.emplace_back(
                    before->isCollapsed() ? 0 : before->getString().getLength(),
                    footnote->getString().getLength());
            }
        }
        previousParagraph.set(element, cpo::uno::UNO_QUERY_THROW);
    }
    return numbers;
}

// The text of a range, which is its string without the numbers of the footnote references in it,
// as GAS leaves those out:
OUString textOf(cpo::uno::Reference<css::text::XTextRange> const & range)
{
    // Writer's string of a whole text, as of a table cell, lacks most of a table that the text
    // starts with, so a whole text joins the texts of its paragraphs and table cells with line
    // ends:
    if (cpo::uno::Reference<css::text::XText> const whole{range, cpo::uno::UNO_QUERY}) {
        OUStringBuffer buf;
        bool first = true;
        auto const append = [&buf, &first](OUString const & part) {
            if (!first) {
                buf.append('\n');
            }
            first = false;
            buf.append(part);
        };
        cpo::uno::Reference<css::container::XEnumerationAccess> const elements(
            whole, cpo::uno::UNO_QUERY_THROW);
        for (auto const en = elements->createEnumeration(); en->hasMoreElements();) {
            auto const element = en->nextElement();
            if (cpo::uno::Reference<css::text::XTextTable> const table{
                    element, cpo::uno::UNO_QUERY})
            {
                cpo::uno::Reference<css::table::XCellRange> const cells(
                    table, cpo::uno::UNO_QUERY_THROW);
                auto const rowCount = table->getRows()->getCount();
                for (sal_Int32 row = 0; row != rowCount; ++row) {
                    for (sal_Int32 column = 0;; ++column) {
                        cpo::uno::Reference<css::text::XTextRange> cell;
                        try {
                            cell.set(
                                cells->getCellByPosition(column, row), cpo::uno::UNO_QUERY_THROW);
                        } catch (css::lang::IndexOutOfBoundsException const &) {
                            break;
                        }
                        append(textOf(cell));
                    }
                }
            } else {
                append(textOf(cpo::uno::Reference<css::text::XTextRange>(
                    element, cpo::uno::UNO_QUERY_THROW)));
            }
        }
        return buf.makeStringAndClear();
    }
    // A text portion contains a footnote reference only when it is one:
    if (cpo::uno::Reference<css::lang::XServiceInfo> const info{range, cpo::uno::UNO_QUERY};
        info.is() && info->supportsService(u"com.sun.star.text.TextPortion"_ustr))
    {
        OUString type;
        cpo::uno::Reference<css::beans::XPropertySet>(range, cpo::uno::UNO_QUERY_THROW)
            ->getPropertyValue(u"TextPortionType"_ustr) >>= type;
        return type == u"Footnote" ? OUString() : range->getString();
    }
    OUString text = range->getString();
    auto const numbers = footnoteNumbers(range, text);
    for (auto i = numbers.rbegin(); i != numbers.rend(); ++i) {
        text = text.replaceAt(i->first, i->second, u"");
    }
    return text;
}

// Moves the cursor right past the steps without text (the anchors of inline images, but also those
// of footnotes and other objects, and empty fields) before the next character of text, which there
// must be in the paragraph (as GAS starts the text at an offset after the inline images before the
// character there):
void skipAnchors(cpo::uno::Reference<css::text::XTextCursor> const & cursor) {
    for (auto const probe = cursor->getText()->createTextCursorByRange(cursor);
         probe->goRight(1, true) && textOf(probe).isEmpty(); probe->collapseToEnd())
    {
        cursor->gotoRange(probe->getEnd(), false);
    }
}

// Moves the cursor right past count characters of text, in steps that fit the short that goRight
// takes, plus one step for each anchor character, as of an inline image, which the text lacks:
void goRight(
    cpo::uno::Reference<css::text::XTextCursor> const & cursor, sal_Int32 count, bool expand)
{
    assert(count >= 0);
    auto const passed = cursor->getText()->createTextCursorByRange(cursor->getEnd());
    sal_Int32 remaining = count;
    do {
        auto const step = std::min<sal_Int32>(remaining, SAL_MAX_INT16);
        if (!cursor->goRight(step, expand)) {
            break;
        }
        passed->gotoRange(cursor->getEnd(), true);
        // A text field's text can be longer than the one step it takes, so this can go negative:
        remaining = count - textOf(passed).getLength();
    } while (remaining > 0);
}

// Calls f with each image anchored as given to a character of the paragraph, which its text
// portions have, and with the text portion of that character:
template<typename F> void forEachFramedImage(
    cpo::uno::Reference<css::text::XTextContent> const & paragraph,
    css::text::TextContentAnchorType anchorType, F f)
{
    cpo::uno::Reference<css::container::XEnumerationAccess> const ea(
        paragraph, cpo::uno::UNO_QUERY);
    if (!ea.is()) {
        return;
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
                    && anchor == anchorType)
                {
                    f(portion, frame);
                }
            }
        }
    }
}

// A paragraph in reading order, as the runs of its text before, between and after its inline images
// (which can be empty), and those images:
struct ParagraphPart {
    // The image, or null for a run of text:
    cpo::uno::Reference<css::text::XTextContent> image;
    cpo::uno::Reference<css::text::XTextRange> range;
    // For a run of text, the name of the image before it (which is unique in the document), or
    // empty for the first run:
    OUString after;
};

std::vector<ParagraphPart> paragraphParts(
    cpo::uno::Reference<css::text::XTextContent> const & paragraph)
{
    cpo::uno::Reference<css::text::XTextRange> const whole(paragraph, cpo::uno::UNO_QUERY_THROW);
    auto const host = whole->getText();
    std::vector<ParagraphPart> parts;
    auto run = host->createTextCursorByRange(whole->getStart());
    OUString after;
    forEachFramedImage(
        paragraph, css::text::TextContentAnchorType_AS_CHARACTER,
        [&host, &parts, &run, &after](
            cpo::uno::Reference<css::beans::XPropertySet> const & portion,
            cpo::uno::Reference<css::text::XTextContent> const & image)
        {
            cpo::uno::Reference<css::text::XTextRange> const range(
                portion, cpo::uno::UNO_QUERY_THROW);
            run->gotoRange(range->getStart(), true);
            parts.push_back({{}, run, after});
            parts.push_back({image, range, {}});
            run = host->createTextCursorByRange(range->getEnd());
            after = cpo::uno::Reference<css::container::XNamed>(image, cpo::uno::UNO_QUERY_THROW)
                ->getName();
        });
    run->gotoRange(whole->getEnd(), true);
    parts.push_back({{}, run, after});
    return parts;
}

cpo::uno::Sequence<cpo::uno::Reference<scriptinterop::XElement>> enumerateElements(
    std::shared_ptr<Model> const & model,
    cpo::uno::Reference<css::text::XText> const & text,
    cpo::uno::Reference<scriptinterop::XElement> const & parent);

// The paragraphs and tables of a text, in document order:
std::vector<cpo::uno::Reference<css::text::XTextContent>> enumerateContents(
    cpo::uno::Reference<css::text::XText> const & text);

// A paragraph or table of a text, as an element:
cpo::uno::Reference<scriptinterop::XElement> elementOf(
    std::shared_ptr<Model> const & model,
    cpo::uno::Reference<scriptinterop::XElement> const & parent,
    cpo::uno::Reference<css::text::XTextContent> const & content);

cpo::uno::Sequence<cpo::uno::Reference<scriptinterop::XElement>> elementsOf(
    std::shared_ptr<Model> const & model,
    cpo::uno::Reference<scriptinterop::XElement> const & parent,
    std::vector<cpo::uno::Reference<css::text::XTextContent>> const & contents);

// A container whose children are the paragraphs and tables of a text:
class SAL_LOPLUGIN_ANNOTATE("crosscast") ContentsContainer {
public:
    virtual std::vector<cpo::uno::Reference<css::text::XTextContent>> const & contents() = 0;

protected:
    ~ContentsContainer() = default;
};

cpo::uno::Reference<scriptinterop::XElement> siblingContent(
    std::shared_ptr<Model> const & model,
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

// Removes a paragraph, unless it is the last one in its text, which GAS refuses to remove:
void removeParagraph(cpo::uno::Reference<css::text::XTextContent> const & paragraph)
{
    if (paragraph.is()) {
        auto const anchor = paragraph->getAnchor();
        cpo::uno::Reference<css::container::XEnumerationAccess> const access(
            anchor->getText(), cpo::uno::UNO_QUERY_THROW);
        cpo::uno::Reference<css::text::XTextContent> last;
        for (auto const en = access->createEnumeration(); en->hasMoreElements();) {
            en->nextElement() >>= last;
        }
        cpo::uno::Reference<css::text::XTextRangeCompare> const compare(
            anchor->getText(), cpo::uno::UNO_QUERY_THROW);
        if (last.is() && compare->compareRegionStarts(last->getAnchor(), anchor) == 0) {
            throw cpo::uno::RuntimeException(
                u"removeFromParent: the last paragraph of a section cannot be removed"_ustr);
        }
    }
    removeContent(paragraph);
}

sal_Int32 hundredthMmToPixels(sal_Int32 hundredthMm) {
    return o3tl::convert(hundredthMm, o3tl::Length::mm100, o3tl::Length::px);
}

sal_Int32 pixelsToHundredthMm(sal_Int32 pixels) {
    return o3tl::convert(pixels, o3tl::Length::px, o3tl::Length::mm100);
}

// A color, which GAS gives as "#rrggbb" (in either case on input):
sal_Int32 parseColor(OUString const & hex) {
    if (hex.getLength() != 7 || hex[0] != '#'
        || !std::all_of(
            hex.getStr() + 1, hex.getStr() + 7,
            [](sal_Unicode c) { return rtl::isAsciiHexDigit(c); }))
    {
        // Same exception message as GAS:
        throw cpo::uno::RuntimeException(u"Invalid color value."_ustr);
    }
    return o3tl::toInt32(hex.subView(1), 16);
}

std::optional<sal_Int32> parseColor(css::beans::Optional<OUString> const & hex) {
    if (!hex.IsPresent) {
        return {};
    }
    return parseColor(hex.Value);
}

scriptinterop::TextAlignment textAlignmentArgument(
    css::beans::Optional<scriptinterop::TextAlignment> const & textAlignment)
{
    if (!textAlignment.IsPresent) {
        // Same exception message as GAS:
        throw cpo::uno::RuntimeException(u"Argument cannot be null: textAlignment"_ustr);
    }
    return textAlignment.Value;
}

// One of the codes that GAS's Document.getSupportedLanguageCodes gives, or empty for a language
// that GAS does not support:
OUString gasLanguageCode(css::lang::Locale const & locale) {
    static constexpr std::u16string_view regional[] = {
        u"de_CH", u"en_CA", u"en_GB", u"es_419", u"fr_CA", u"pt_BR", u"pt_PT", u"zh_CN", u"zh_TW",
        u"zh_HK"};
    static constexpr std::u16string_view plain[] = {
        u"af", u"az", u"id", u"ms", u"ca", u"cs", u"cy", u"da", u"de", u"et", u"en", u"es", u"eu",
        u"fil", u"fr", u"gl", u"hr", u"zu", u"is", u"it", u"sw", u"lv", u"lt", u"hu", u"mt", u"nl",
        u"nn", u"no", u"uz", u"pl", u"ro", u"sq", u"sk", u"sl", u"fi", u"sv", u"vi", u"tr", u"my",
        u"be", u"bg", u"kk", u"mk", u"mn", u"ru", u"sr", u"uk", u"el", u"hy", u"iw", u"ur", u"ar",
        u"fa", u"ne", u"mr", u"hi", u"bn", u"pa", u"as", u"gu", u"ta", u"te", u"kn", u"ml", u"si",
        u"th", u"lo", u"ka", u"am", u"km", u"or", u"ja", u"ko"};
    if (locale.Language.isEmpty()) {
        return {};
    }
    LanguageTag const tag(locale);
    // GAS has the old code iw for Hebrew, and no for Norwegian Bokmal:
    auto const language = tag.getLanguage() == "he" ? u"iw"_ustr
        : tag.getLanguage() == "nb" ? u"no"_ustr : tag.getLanguage();
    OUString const withRegion = language + "_" + tag.getCountry();
    if (std::find(std::begin(regional), std::end(regional), withRegion) != std::end(regional)) {
        return withRegion;
    }
    if (std::find(std::begin(plain), std::end(plain), language) != std::end(plain)) {
        return language;
    }
    return {};
}

OUString formatColor(sal_Int32 color) {
    // Just take the RGB part, ignoring any transparency value (and the leading 1 makes number give
    // seven digits, of which the subView drops that 1 again):
    OUString const digits(OUString::number(0x1000000 | (color & 0xFFFFFF), 16));
    return OUString::Concat("#") + digits.subView(1);
}

cpo::uno::Reference<css::text::XTextContent> createGraphicFromBlob(
    std::shared_ptr<Model> const & model,
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
        model->uno(), cpo::uno::UNO_QUERY_THROW);
    cpo::uno::Reference<css::text::XTextContent> const graphic(
        docFactory->createInstance(u"com.sun.star.text.TextGraphicObject"_ustr),
        cpo::uno::UNO_QUERY_THROW);
    cpo::uno::Reference<css::beans::XPropertySet> const props(graphic, cpo::uno::UNO_QUERY_THROW);
    props->setPropertyValue(u"Graphic"_ustr, cpo::uno::Any(xgraphic));
    // Same as in GAS, the image has its natural size in pixels, or for a vector graphic without
    // one its natural size in 1/100 mm:
    cpo::uno::Reference<css::beans::XPropertySet> const descriptor(
        xgraphic, cpo::uno::UNO_QUERY_THROW);
    css::awt::Size size;
    descriptor->getPropertyValue(u"SizePixel"_ustr) >>= size;
    if (size.Width != 0 && size.Height != 0) {
        size = css::awt::Size(pixelsToHundredthMm(size.Width), pixelsToHundredthMm(size.Height));
    } else {
        descriptor->getPropertyValue(u"Size100thMM"_ustr) >>= size;
    }
    if (size.Width != 0 && size.Height != 0) {
        props->setPropertyValue(u"Width"_ustr, cpo::uno::Any(size.Width));
        props->setPropertyValue(u"Height"_ustr, cpo::uno::Any(size.Height));
    }
    props->setPropertyValue(
        u"AnchorType"_ustr, cpo::uno::Any(css::text::TextContentAnchorType_AS_CHARACTER));
    return graphic;
}

// The text of the text ranges of a selection, with a line end between two ranges:
OUString textOfRanges(cpo::uno::Reference<css::container::XIndexAccess> const & ranges) {
    OUStringBuffer buf;
    auto const n = ranges->getCount();
    for (sal_Int32 i = 0; i != n; ++i)
    {
        cpo::uno::Reference<css::text::XTextRange> range;
        if (!(ranges->getByIndex(i) >>= range) || !range.is())
        {
            throw cpo::uno::RuntimeException(
                u"getText: the selection contains something that is not text"_ustr);
        }
        if (!buf.isEmpty())
        {
            buf.append('\n');
        }
        buf.append(textOf(range));
    }
    return buf.makeStringAndClear();
}

void replaceRanges(
    cpo::uno::Reference<css::container::XIndexAccess> const & ranges, OUString const & newText)
{
    auto const n = ranges->getCount();
    for (sal_Int32 i = 0; i != n; ++i)
    {
        cpo::uno::Reference<css::text::XTextRange> range;
        if (!(ranges->getByIndex(i) >>= range) || !range.is())
        {
            throw cpo::uno::RuntimeException(
                u"replace: the selection contains something that is not text"_ustr);
        }
        range->setString(newText);
    }
}

class SelectionImpl : public cppu::WeakImplHelper<scriptinterop::XSelection>
{
public:
    SelectionImpl(
        std::shared_ptr<Model> const & model,
        cpo::uno::Reference<css::container::XIndexAccess> const& ranges)
        : model_(model), ranges_(ranges)
    {
    }

    cpo::uno::Reference<cpo::uno::XInterface> getuno() override { return ranges_; }

    cpo::uno::Sequence<cpo::uno::Reference<scriptinterop::XRangeElement>> getRangeElements()
        override;

    cpo::uno::Sequence<cpo::uno::Reference<scriptinterop::XRangeElement>> getSelectedElements()
        override
    { return getRangeElements(); }

    OUString getText() override { return textOfRanges(ranges_); }

    void replace(OUString const& newText) override { replaceRanges(ranges_, newText); }

private:
    std::shared_ptr<Model> model_;
    cpo::uno::Reference<css::container::XIndexAccess> ranges_;
};

// The text ranges that a range built by a script stands for, in document order:
class TextRangesIndex: public cppu::WeakImplHelper<css::container::XIndexAccess> {
public:
    explicit TextRangesIndex(std::vector<cpo::uno::Reference<css::text::XTextRange>> && ranges):
        ranges_(std::move(ranges)) {}

    sal_Int32 getCount() override { return ranges_.size(); }

    cpo::uno::Any getByIndex(sal_Int32 index) override {
        if (index < 0 || o3tl::make_unsigned(index) >= ranges_.size()) {
            throw css::lang::IndexOutOfBoundsException();
        }
        return cpo::uno::Any(ranges_[index]);
    }

    cpo::uno::Type getElementType() override {
        return cppu::UnoType<css::text::XTextRange>::get();
    }

    bool hasElements() override { return !ranges_.empty(); }

private:
    std::vector<cpo::uno::Reference<css::text::XTextRange>> ranges_;
};

// The document text that a text element covers:
cpo::uno::Reference<css::text::XTextRange> textRangeOf(
    cpo::uno::Reference<scriptinterop::XText> const & text);

class RangeBuilderImpl: public cppu::WeakImplHelper<scriptinterop::XRangeBuilder> {
public:
    explicit RangeBuilderImpl(std::shared_ptr<Model> const & model):
        model_(model) {}

    cpo::uno::Reference<scriptinterop::XRangeBuilder> addElement(
        cpo::uno::Reference<scriptinterop::XElement> const & added) override;

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
        auto const para = textRangeOf(text);
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
        goRight(cursor, startOffset, false);
        if (!text->getText().isEmpty()) {
            skipAnchors(cursor);
        }
        goRight(cursor, endOffsetInclusive - startOffset + 1, true);
        added_.push_back({text, cursor, true});
        return this;
    }

    cpo::uno::Reference<scriptinterop::XRangeBuilder> addElementsBetween(
        cpo::uno::Reference<scriptinterop::XElement> const & startElement,
        cpo::uno::Reference<scriptinterop::XElement> const & endElementInclusive) override;

    cpo::uno::Reference<scriptinterop::XRangeBuilder> addElementsBetweenOffsets(
        cpo::uno::Reference<scriptinterop::XText> const & startTextElement, sal_Int32 startOffset,
        cpo::uno::Reference<scriptinterop::XText> const & endTextElementInclusive,
        sal_Int32 endOffsetInclusive) override;

    cpo::uno::Reference<scriptinterop::XSelection> build() override;

private:
    void addBetween(
        cpo::uno::Reference<scriptinterop::XElement> const & startElement,
        std::vector<sal_Int32> const & start, bool startOffset,
        cpo::uno::Reference<scriptinterop::XElement> const & endElement,
        std::vector<sal_Int32> const & end, bool endText);

    bool cover(
        cpo::uno::Reference<scriptinterop::XElement> const & element,
        std::vector<sal_Int32> const & path, std::vector<sal_Int32> const & start, bool startOffset,
        std::vector<sal_Int32> const & end, bool endText);

    // What was added, in that order, with the text it covers (null for a table, row, cell or inline
    // image, which are added as a whole):
    struct Added {
        cpo::uno::Reference<scriptinterop::XElement> element;
        cpo::uno::Reference<css::text::XTextRange> range;
        bool offsets = false;
    };

    std::shared_ptr<Model> model_;
    std::vector<Added> added_;
};

OUString elementTypeName(scriptinterop::ElementType type) {
    switch (type) {
    case scriptinterop::ElementType_PARAGRAPH:
        return u"PARAGRAPH"_ustr;
    case scriptinterop::ElementType_LIST_ITEM:
        return u"LIST_ITEM"_ustr;
    case scriptinterop::ElementType_TABLE:
        return u"TABLE"_ustr;
    case scriptinterop::ElementType_TABLE_ROW:
        return u"TABLE_ROW"_ustr;
    case scriptinterop::ElementType_TABLE_CELL:
        return u"TABLE_CELL"_ustr;
    case scriptinterop::ElementType_INLINE_IMAGE:
        return u"INLINE_IMAGE"_ustr;
    case scriptinterop::ElementType_PAGE_BREAK:
        return u"PAGE_BREAK"_ustr;
    case scriptinterop::ElementType_HORIZONTAL_RULE:
        return u"HORIZONTAL_RULE"_ustr;
    case scriptinterop::ElementType_BODY_SECTION:
        return u"BODY_SECTION"_ustr;
    case scriptinterop::ElementType_FOOTNOTE:
        return u"FOOTNOTE"_ustr;
    case scriptinterop::ElementType_FOOTNOTE_SECTION:
        return u"FOOTNOTE_SECTION"_ustr;
    case scriptinterop::ElementType_TEXT:
        return u"TEXT"_ustr;
    default:
        throw cpo::uno::RuntimeException(
            "unknown element type " + OUString::number(static_cast<sal_Int32>(type)));
    }
}

// As in GAS, an element can be cast to the interface of its own type only:
template<typename T> class ElementImpl: public cppu::WeakImplHelper<T> {
public:
    cpo::uno::Reference<scriptinterop::XBody> asBody() override {
        return cast<scriptinterop::XBody>(scriptinterop::ElementType_BODY_SECTION);
    }

    cpo::uno::Reference<scriptinterop::XFootnote> asFootnote() override {
        return cast<scriptinterop::XFootnote>(scriptinterop::ElementType_FOOTNOTE);
    }

    cpo::uno::Reference<scriptinterop::XContainerElement> asFootnoteSection() override {
        return cast<scriptinterop::XContainerElement>(scriptinterop::ElementType_FOOTNOTE_SECTION);
    }

    cpo::uno::Reference<scriptinterop::XInlineImage> asInlineImage() override {
        return cast<scriptinterop::XInlineImage>(scriptinterop::ElementType_INLINE_IMAGE);
    }

    cpo::uno::Reference<scriptinterop::XParagraph> asListItem() override {
        return cast<scriptinterop::XParagraph>(scriptinterop::ElementType_LIST_ITEM);
    }

    cpo::uno::Reference<scriptinterop::XParagraph> asParagraph() override {
        return cast<scriptinterop::XParagraph>(scriptinterop::ElementType_PARAGRAPH);
    }

    cpo::uno::Reference<scriptinterop::XTable> asTable() override {
        return cast<scriptinterop::XTable>(scriptinterop::ElementType_TABLE);
    }

    cpo::uno::Reference<scriptinterop::XTableCell> asTableCell() override {
        return cast<scriptinterop::XTableCell>(scriptinterop::ElementType_TABLE_CELL);
    }

    cpo::uno::Reference<scriptinterop::XTableRow> asTableRow() override {
        return cast<scriptinterop::XTableRow>(scriptinterop::ElementType_TABLE_ROW);
    }

    cpo::uno::Reference<scriptinterop::XText> asText() override;

private:
    template<typename U> cpo::uno::Reference<U> cast(scriptinterop::ElementType type) {
        auto const own = this->getType();
        cpo::uno::Reference<U> const element(static_cast<T *>(this), cpo::uno::UNO_QUERY);
        if (own != type || !element.is()) {
            throw cpo::uno::RuntimeException(
                elementTypeName(own) + " can't be cast to " + elementTypeName(type) + ".");
        }
        return element;
    }
};

class TextImpl: public ElementImpl<scriptinterop::XText> {
public:
    TextImpl(
        std::shared_ptr<Model> const & model,
        cpo::uno::Reference<scriptinterop::XElement> const & parent,
        cpo::uno::Reference<css::text::XTextContent> const & content,
        scriptinterop::ElementType reportedType, std::optional<OUString> run = {}):
        model_(model), parent_(parent), content_(content), reportedType_(reportedType), run_(run)
    {}

    cpo::uno::Reference<cpo::uno::XInterface> getuno() override { return content_; }

    cpo::uno::Reference<scriptinterop::XParagraph> asListItem() override;

    cpo::uno::Reference<scriptinterop::XParagraph> asParagraph() override;

    cpo::uno::Reference<scriptinterop::XText> asText() override { return this; }

    cpo::uno::Reference<scriptinterop::XText> appendText(OUString const & text) override {
        auto const whole = wholeRange();
        auto const host = whole->getText();
        host->insertString(host->createTextCursorByRange(whole->getEnd()), text, false);
        return this;
    }

    // TODO: return a detached deep copy, not this; mutations on the "copy" write back to the live
    // element:
    cpo::uno::Reference<scriptinterop::XElement> copy() override { return this; }

    cpo::uno::Reference<scriptinterop::XText> deleteText(
        sal_Int32 startOffset, sal_Int32 endOffsetInclusive) override
    {
        subRange(u"deleteText", startOffset, endOffsetInclusive)->setString(OUString());
        return this;
    }

    cpo::uno::Reference<scriptinterop::XText> editAsText() override { return this; }

    css::beans::Optional<cpo::uno::Reference<scriptinterop::XRangeElement>> findText(
        OUString const & searchPattern) override
    { return findTextFrom(searchPattern, {}); }

    css::beans::Optional<cpo::uno::Reference<scriptinterop::XRangeElement>> findTextFrom(
        OUString const & searchPattern,
        cpo::uno::Reference<scriptinterop::XRangeElement> const & from) override;

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

    css::beans::Optional<OUString> getForegroundColor() override {
        // Same as in GAS, the color of the whole text is null when its runs differ:
        std::optional<sal_Int32> color;
        for (auto const & r: runs()) {
            if (textOf(r).isEmpty()) {
                continue;
            }
            auto const runColor = colorOn(r);
            // Colors count as the same if their RGB parts are, as a GAS color has no transparency:
            if (!runColor || (color && (*color & 0xFFFFFF) != (*runColor & 0xFFFFFF))) {
                return {false, {}};
            }
            color = runColor;
        }
        if (!color) {
            return {false, {}};
        }
        return {true, formatColor(*color)};
    }

    css::beans::Optional<OUString> getForegroundColorAt(sal_Int32 offset) override {
        checkOffset(offset);
        auto const color = colorOn(runAt(offset));
        if (!color) {
            return {false, {}};
        }
        return {true, formatColor(*color)};
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

    css::beans::Optional<cpo::uno::Reference<scriptinterop::XElement>> getNextSibling() override {
        return sibling(true);
    }

    css::beans::Optional<cpo::uno::Reference<scriptinterop::XElement>> getParent() override {
        return maybe(parent_);
    }

    css::beans::Optional<cpo::uno::Reference<scriptinterop::XElement>> getPreviousSibling() override
    { return sibling(false); }

    OUString getText() override { return textOf(wholeRange()); }

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
        auto const runs = this->runs();
        std::vector<sal_Int32> v;
        v.reserve(runs.empty() ? 1 : runs.size());
        sal_Int32 off = 0;
        for (auto const & r: runs) {
            // A footnote reference is a run without text:
            auto const length = textOf(r).getLength();
            if (length != 0) {
                v.push_back(off);
                off += length;
            }
        }
        if (v.empty()) {
            v.push_back(0);
        }
        return cpo::uno::Sequence(v.data(), v.size());
    }

    cpo::uno::Reference<scriptinterop::XText> insertText(sal_Int32 offset, OUString const & text)
        override
    {
        if (offset < 0) {
            throw cpo::uno::RuntimeException(
                "insertText: expected a non-negative offset, got " + OUString::number(offset));
        }
        auto const whole = wholeRange();
        auto const host = whole->getText();
        auto const cursor = host->createTextCursorByRange(whole->getStart());
        goRight(cursor, offset, false);
        if (offset < textOf(whole).getLength()) {
            skipAnchors(cursor);
        }
        host->insertString(cursor, text, false);
        return this;
    }

    scriptinterop::ElementType getType() override { return reportedType_; }

    // As in GAS, a text merges with the element of the same type before it, which a child text
    // never has (as only an inline image separates two runs of a paragraph), and which for the text
    // of a paragraph (as made with editAsText) is the paragraph before it:
    cpo::uno::Reference<scriptinterop::XText> merge() override;

    // For a child text, the name of the inline image before its run of the paragraph's text, which
    // is empty for the first run:
    std::optional<OUString> const & run() const { return run_; }

    // The child texts that this text is made of, which is just itself for a child text:
    std::vector<cpo::uno::Reference<scriptinterop::XText>> childTexts();

    // The characters of this text from start to end (exclusive), as a partial range element:
    cpo::uno::Reference<scriptinterop::XRangeElement> part(sal_Int32 start, sal_Int32 end);

    cpo::uno::Reference<css::text::XTextRange> wholeRange() {
        if (!run_) {
            return cpo::uno::Reference<css::text::XTextRange>(content_, cpo::uno::UNO_QUERY_THROW);
        }
        for (auto const & part: paragraphParts(content_)) {
            if (!part.image.is() && part.after == *run_) {
                return part.range;
            }
        }
        throw cpo::uno::RuntimeException(u"the text is no longer part of its paragraph"_ustr);
    }

    cpo::uno::Reference<scriptinterop::XParagraph> paragraph();

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

    // As in GAS, a child text removes its run of the paragraph's text:
    cpo::uno::Reference<scriptinterop::XElement> removeFromParent() override {
        if (run_) {
            wholeRange()->setString(OUString());
        } else {
            removeParagraph(content_);
        }
        return this;
    }

    cpo::uno::Reference<scriptinterop::XText> setBold(
        css::beans::Optional<bool> const & value) override
    {
        setBoldOn(wholeRange(), value);
        return this;
    }

    cpo::uno::Reference<scriptinterop::XText> setBoldRange(
        sal_Int32 startOffset, sal_Int32 endOffsetInclusive,
        css::beans::Optional<bool> const & value) override
    {
        setBoldOn(subRange(u"setBold", startOffset, endOffsetInclusive), value);
        return this;
    }

    cpo::uno::Reference<scriptinterop::XText> setFontFamily(
        css::beans::Optional<OUString> const & fontFamilyName) override
    {
        setFontFamilyOn(wholeRange(), fontFamilyName);
        return this;
    }

    cpo::uno::Reference<scriptinterop::XText> setFontFamilyRange(
        sal_Int32 startOffset, sal_Int32 endOffsetInclusive,
        css::beans::Optional<OUString> const & fontFamilyName) override
    {
        setFontFamilyOn(
            subRange(u"setFontFamily", startOffset, endOffsetInclusive), fontFamilyName);
        return this;
    }

    cpo::uno::Reference<scriptinterop::XText> setForegroundColor(
        css::beans::Optional<OUString> const & color) override
    {
        auto const value = parseColor(color);
        auto const whole = wholeRange();
        // The color goes on all the text of the paragraph, over the colors that parts of it
        // already have:
        setForegroundColorOn(whole->getText()->createTextCursorByRange(whole), value);
        return this;
    }

    cpo::uno::Reference<scriptinterop::XText> setForegroundColorRange(
        sal_Int32 startOffset, sal_Int32 endOffsetInclusive,
        css::beans::Optional<OUString> const & color) override
    {
        auto const value = parseColor(color);
        checkOffset(startOffset);
        checkOffset(endOffsetInclusive);
        setForegroundColorOn(
            subRange(u"setForegroundColor", startOffset, endOffsetInclusive), value);
        return this;
    }

    cpo::uno::Reference<scriptinterop::XText> setItalic(
        css::beans::Optional<bool> const & value) override
    {
        setItalicOn(wholeRange(), value);
        return this;
    }

    cpo::uno::Reference<scriptinterop::XText> setItalicRange(
        sal_Int32 startOffset, sal_Int32 endOffsetInclusive,
        css::beans::Optional<bool> const & value) override
    {
        setItalicOn(subRange(u"setItalic", startOffset, endOffsetInclusive), value);
        return this;
    }

    cpo::uno::Reference<scriptinterop::XText> setLinkUrl(
        css::beans::Optional<OUString> const & url) override
    {
        auto const whole = wholeRange();
        setLinkUrlOn(whole->getText()->createTextCursorByRange(whole), url);
        return this;
    }

    cpo::uno::Reference<scriptinterop::XText> setLinkUrlRange(
        sal_Int32 startOffset, sal_Int32 endOffsetInclusive,
        css::beans::Optional<OUString> const & url) override
    {
        setLinkUrlOn(subRange(u"setLinkUrl", startOffset, endOffsetInclusive), url);
        return this;
    }

    cpo::uno::Reference<scriptinterop::XText> setStrikethrough(
        css::beans::Optional<bool> const & value) override
    {
        setStrikethroughOn(wholeRange(), value);
        return this;
    }

    cpo::uno::Reference<scriptinterop::XText> setStrikethroughRange(
        sal_Int32 startOffset, sal_Int32 endOffsetInclusive,
        css::beans::Optional<bool> const & value) override
    {
        setStrikethroughOn(subRange(u"setStrikethrough", startOffset, endOffsetInclusive), value);
        return this;
    }

    cpo::uno::Reference<scriptinterop::XText> setText(OUString const & text) override {
        wholeRange()->setString(text);
        return this;
    }

    cpo::uno::Reference<scriptinterop::XText> setTextAlignment(
        css::beans::Optional<scriptinterop::TextAlignment> const & textAlignment) override
    {
        setTextAlignmentOn(wholeRange(), textAlignmentArgument(textAlignment));
        return this;
    }

    cpo::uno::Reference<scriptinterop::XText> setTextAlignmentRange(
        sal_Int32 startOffset, sal_Int32 endOffsetInclusive,
        css::beans::Optional<scriptinterop::TextAlignment> const & textAlignment) override
    {
        auto const value = textAlignmentArgument(textAlignment);
        setTextAlignmentOn(subRange(u"setTextAlignment", startOffset, endOffsetInclusive), value);
        return this;
    }

    cpo::uno::Reference<scriptinterop::XText> setUnderline(
        css::beans::Optional<bool> const & value) override
    {
        setUnderlineOn(wholeRange(), value);
        return this;
    }

    cpo::uno::Reference<scriptinterop::XText> setUnderlineRange(
        sal_Int32 startOffset, sal_Int32 endOffsetInclusive,
        css::beans::Optional<bool> const & value) override
    {
        setUnderlineOn(subRange(u"setUnderline", startOffset, endOffsetInclusive), value);
        return this;
    }

private:
    friend class CursorImpl;

    // The text portions of the paragraph as the document has them now:
    std::vector<cpo::uno::Reference<css::text::XTextRange>> runs() {
        std::vector<cpo::uno::Reference<css::text::XTextRange>> runs;
        if (cpo::uno::Reference<css::container::XEnumerationAccess> const ea{
                content_, cpo::uno::UNO_QUERY})
        {
            auto const en = ea->createEnumeration();
            while (en.is() && en->hasMoreElements()) {
                cpo::uno::Reference<css::text::XTextRange> portion;
                en->nextElement() >>= portion;
                if (portion.is()) {
                    runs.push_back(portion);
                }
            }
        }
        if (run_) {
            auto const whole = wholeRange();
            cpo::uno::Reference<css::text::XTextRangeCompare> const cmp(
                whole->getText(), cpo::uno::UNO_QUERY_THROW);
            std::erase_if(runs, [&cmp, &whole](auto const & r) {
                return cmp->compareRegionStarts(whole, r) < 0
                    || cmp->compareRegionEnds(r, whole) < 0;
            });
        }
        return runs;
    }

    cpo::uno::Reference<css::text::XTextRange> runAt(sal_Int32 offset) {
        auto const runs = this->runs();
        if (runs.empty()) {
            throw cpo::uno::RuntimeException(u"the text has no content at this offset"_ustr);
        }
        sal_Int32 start = 0;
        for (auto const & r: runs) {
            auto const len = textOf(r).getLength();
            if (offset < start + len) {
                return r;
            }
            start += len;
        }
        return runs.back();
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

    cpo::uno::Reference<css::text::XTextRange> subRange(
        std::u16string_view method, sal_Int32 startOffset, sal_Int32 endOffsetInclusive)
    {
        if (startOffset < 0 || endOffsetInclusive < startOffset) {
            throw cpo::uno::RuntimeException(
                OUString::Concat(method)
                + ": expected 0 <= startOffset <= endOffsetInclusive, got startOffset="
                + OUString::number(startOffset) + ", endOffsetInclusive="
                + OUString::number(endOffsetInclusive));
        }
        auto const whole = wholeRange();
        auto const cursor = whole->getText()->createTextCursorByRange(whole->getStart());
        goRight(cursor, startOffset, false);
        if (startOffset < textOf(whole).getLength()) {
            skipAnchors(cursor);
        }
        goRight(cursor, endOffsetInclusive - startOffset + 1, true);
        return cursor;
    }

    static void setProp(
        cpo::uno::Reference<css::text::XTextRange> const & range, OUString const & name,
        cpo::uno::Any const & value)
    {
        cpo::uno::Reference<css::beans::XPropertySet>(range, cpo::uno::UNO_QUERY_THROW)
            ->setPropertyValue(name, value);
    }

    static void resetProp(
        cpo::uno::Reference<css::text::XTextRange> const & range, OUString const & name)
    {
        cpo::uno::Reference<css::beans::XPropertyState>(
            range->getText()->createTextCursorByRange(range), cpo::uno::UNO_QUERY_THROW)
            ->setPropertyToDefault(name);
    }

    static void setBoldOn(
        cpo::uno::Reference<css::text::XTextRange> const & range,
        css::beans::Optional<bool> const & value)
    {
        if (!value.IsPresent) {
            resetProp(range, u"CharWeight"_ustr);
            return;
        }
        setProp(range, u"CharWeight"_ustr, cpo::uno::Any(static_cast<float>(
            value.Value ? css::awt::FontWeight::BOLD : css::awt::FontWeight::NORMAL)));
    }

    static void setFontFamilyOn(
        cpo::uno::Reference<css::text::XTextRange> const & range,
        css::beans::Optional<OUString> const & fontFamilyName)
    {
        if (!fontFamilyName.IsPresent) {
            resetProp(range, u"CharFontName"_ustr);
            return;
        }
        setProp(range, u"CharFontName"_ustr, cpo::uno::Any(fontFamilyName.Value));
    }

    static void setForegroundColorOn(
        cpo::uno::Reference<css::text::XTextRange> const & range, std::optional<sal_Int32> color)
    {
        if (!color) {
            resetProp(range, u"CharColor"_ustr);
            return;
        }
        setProp(range, u"CharColor"_ustr, cpo::uno::Any(*color));
    }

    // A run set to Writer's automatic color, COL_AUTO, reads as having no color, the same as a run
    // without one:
    std::optional<sal_Int32> colorOn(cpo::uno::Reference<css::text::XTextRange> const & range) {
        auto const any = getProp(
            range, u"getForegroundColor", u"foreground color", u"CharColor"_ustr);
        sal_Int32 color;
        if (!any || !(*any >>= color) || Color(ColorTransparency, color) == COL_AUTO) {
            return {};
        }
        return color;
    }

    void checkOffset(sal_Int32 offset) {
        // Same exception messages as GAS:
        if (offset < 0) {
            throw cpo::uno::RuntimeException(
                "Index (" + OUString::number(offset) + ") value must be greater or equal to zero.");
        }
        auto const length = getText().getLength();
        if (offset >= length) {
            throw cpo::uno::RuntimeException(
                "Index (" + OUString::number(offset) + ") must be less than the content length ("
                + OUString::number(length) + ").");
        }
    }

    static void setItalicOn(
        cpo::uno::Reference<css::text::XTextRange> const & range,
        css::beans::Optional<bool> const & value)
    {
        if (!value.IsPresent) {
            resetProp(range, u"CharPosture"_ustr);
            return;
        }
        setProp(range, u"CharPosture"_ustr, cpo::uno::Any(
            value.Value ? css::awt::FontSlant_ITALIC : css::awt::FontSlant_NONE));
    }

    static void setLinkUrlOn(
        cpo::uno::Reference<css::text::XTextRange> const & range,
        css::beans::Optional<OUString> const & url)
    {
        if (!url.IsPresent) {
            resetProp(range, u"HyperLinkURL"_ustr);
            return;
        }
        setProp(range, u"HyperLinkURL"_ustr, cpo::uno::Any(url.Value));
    }

    static void setStrikethroughOn(
        cpo::uno::Reference<css::text::XTextRange> const & range,
        css::beans::Optional<bool> const & value)
    {
        if (!value.IsPresent) {
            resetProp(range, u"CharStrikeout"_ustr);
            return;
        }
        setProp(range, u"CharStrikeout"_ustr, cpo::uno::Any(static_cast<sal_Int16>(
            value.Value ? css::awt::FontStrikeout::SINGLE : css::awt::FontStrikeout::NONE)));
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

    static void setUnderlineOn(
        cpo::uno::Reference<css::text::XTextRange> const & range,
        css::beans::Optional<bool> const & value)
    {
        if (!value.IsPresent) {
            resetProp(range, u"CharUnderline"_ustr);
            return;
        }
        setProp(range, u"CharUnderline"_ustr, cpo::uno::Any(static_cast<sal_Int16>(
            value.Value ? css::awt::FontUnderline::SINGLE : css::awt::FontUnderline::NONE)));
    }

    css::beans::Optional<cpo::uno::Reference<scriptinterop::XElement>> sibling(bool next) {
        // A text made with editAsText, which is its paragraph, has the paragraph's siblings:
        if (!run_) {
            if (reportedType_ == scriptinterop::ElementType_TEXT) {
                return {false, {}};
            }
            return maybe(siblingContent(model_, content_, parent_, next));
        }
        cpo::uno::Reference<scriptinterop::XContainerElement> const paragraph(
            parent_, cpo::uno::UNO_QUERY_THROW);
        auto const index = paragraph->getChildIndex(this) + (next ? 1 : -1);
        if (index < 0 || index >= paragraph->getNumChildren()) {
            return {false, {}};
        }
        return {true, paragraph->getChild(index)};
    }

    std::shared_ptr<Model> model_;
    cpo::uno::Reference<scriptinterop::XElement> parent_;
    cpo::uno::Reference<css::text::XTextContent> content_;
    scriptinterop::ElementType reportedType_;
    std::optional<OUString> run_;
};

cpo::uno::Reference<css::text::XTextRange> textRangeOf(
    cpo::uno::Reference<scriptinterop::XText> const & text)
{
    if (auto const impl = dynamic_cast<TextImpl *>(text.get())) {
        return impl->wholeRange();
    }
    return cpo::uno::Reference<css::text::XTextRange>(text->getuno(), cpo::uno::UNO_QUERY_THROW);
}

sal_Int32 childIndexOf(
    scriptinterop::XContainerElement & container,
    cpo::uno::Reference<scriptinterop::XElement> const & child)
{
    if (!child.is()) {
        throw cpo::uno::RuntimeException(u"getChildIndex: the child must not be null"_ustr);
    }
    auto const target = child->getuno();
    assert(target.is());
    // The text children of a paragraph are the runs of its text:
    auto const text = child->getType() == scriptinterop::ElementType_TEXT
        ? dynamic_cast<TextImpl *>(child.get()) : nullptr;
    // Writer can have two UNO objects for one inline image, which then match by its unique name:
    cpo::uno::Reference<css::container::XNamed> const named(
        child->getType() == scriptinterop::ElementType_INLINE_IMAGE ? target : nullptr,
        cpo::uno::UNO_QUERY);
    auto const n = container.getNumChildren();
    for (sal_Int32 i = 0; i != n; ++i) {
        auto const candidate = container.getChild(i);
        if (candidate->getType() != child->getType()) {
            continue;
        }
        if (candidate->getuno() == target
            && (text == nullptr
                || dynamic_cast<TextImpl &>(*candidate).run() == text->run()))
        {
            return i;
        }
        if (named.is()) {
            cpo::uno::Reference<css::container::XNamed> const other(
                candidate->getuno(), cpo::uno::UNO_QUERY);
            if (other.is() && other->getName() == named->getName()) {
                return i;
            }
        }
    }
    throw cpo::uno::RuntimeException(
        u"getChildIndex: the element does not contain the given child"_ustr);
}

// As in GAS, the text of a container joins the texts of all the paragraphs in it, also those in
// nested tables, and each paragraph but the last is followed by a one-character separator:
class ContainerTextImpl: public ElementImpl<scriptinterop::XText> {
public:
    explicit ContainerTextImpl(
        cpo::uno::Reference<scriptinterop::XContainerElement> const & container):
        container_(container)
    {}

    cpo::uno::Reference<cpo::uno::XInterface> getuno() override { return container_->getuno(); }

    cpo::uno::Reference<scriptinterop::XContainerElement> const & container() const {
        return container_;
    }

    // As in GAS, the separator after a paragraph has the attributes of the paragraph's last
    // character:
    std::pair<cpo::uno::Reference<scriptinterop::XText>, sal_Int32> at(sal_Int32 offset) const {
        auto const contents = this->contents();
        checkIndex(contents, offset);
        for (auto const & paragraph: contents.paragraphs) {
            if (offset <= paragraph.start + paragraph.length) {
                auto const inner = std::min(offset - paragraph.start, paragraph.length - 1);
                return {paragraph.text, std::max<sal_Int32>(inner, 0)};
            }
        }
        throw cpo::uno::RuntimeException(u"the offset is past the last paragraph"_ustr);
    }

    cpo::uno::Reference<scriptinterop::XBody> asBody() override { return container_->asBody(); }

    cpo::uno::Reference<scriptinterop::XFootnote> asFootnote() override {
        return container_->asFootnote();
    }

    cpo::uno::Reference<scriptinterop::XContainerElement> asFootnoteSection() override {
        return container_->asFootnoteSection();
    }

    cpo::uno::Reference<scriptinterop::XInlineImage> asInlineImage() override {
        return container_->asInlineImage();
    }

    cpo::uno::Reference<scriptinterop::XParagraph> asListItem() override {
        return container_->asListItem();
    }

    cpo::uno::Reference<scriptinterop::XParagraph> asParagraph() override {
        return container_->asParagraph();
    }

    cpo::uno::Reference<scriptinterop::XTable> asTable() override { return container_->asTable(); }

    cpo::uno::Reference<scriptinterop::XTableCell> asTableCell() override {
        return container_->asTableCell();
    }

    cpo::uno::Reference<scriptinterop::XTableRow> asTableRow() override {
        return container_->asTableRow();
    }

    cpo::uno::Reference<scriptinterop::XText> asText() override { return this; }

    cpo::uno::Reference<scriptinterop::XText> appendText(OUString const &) override {
        throw notYetImplemented(u"appendText");
    }

    // TODO: return a detached deep copy, not this, as for the text of a paragraph:
    cpo::uno::Reference<scriptinterop::XElement> copy() override { return this; }

    cpo::uno::Reference<scriptinterop::XText> deleteText(sal_Int32, sal_Int32) override {
        throw notYetImplemented(u"deleteText");
    }

    cpo::uno::Reference<scriptinterop::XText> editAsText() override { return this; }

    css::beans::Optional<cpo::uno::Reference<scriptinterop::XRangeElement>> findText(
        OUString const & searchPattern) override
    { return findTextFrom(searchPattern, {}); }

    css::beans::Optional<cpo::uno::Reference<scriptinterop::XRangeElement>> findTextFrom(
        OUString const & searchPattern,
        cpo::uno::Reference<scriptinterop::XRangeElement> const & from) override;

    css::beans::Optional<OUString> getFontFamily(sal_Int32 offset) override {
        auto const [text, inner] = at(offset);
        return text->getFontFamily(inner);
    }

    css::beans::Optional<OUString> getForegroundColor() override {
        std::optional<css::beans::Optional<OUString>> color;
        for (auto const & paragraph: contents().paragraphs) {
            if (paragraph.length == 0) {
                continue;
            }
            auto const paragraphColor = paragraph.text->getForegroundColor();
            if (!color) {
                color = paragraphColor;
            } else if (color->IsPresent != paragraphColor.IsPresent
                       || color->Value != paragraphColor.Value)
            {
                return {false, {}};
            }
        }
        return color ? *color : css::beans::Optional<OUString>();
    }

    css::beans::Optional<OUString> getForegroundColorAt(sal_Int32 offset) override {
        auto const [text, inner] = at(offset);
        return text->getForegroundColorAt(inner);
    }

    css::beans::Optional<OUString> getLinkUrl(sal_Int32 offset) override {
        auto const [text, inner] = at(offset);
        return text->getLinkUrl(inner);
    }

    css::beans::Optional<cpo::uno::Reference<scriptinterop::XElement>> getNextSibling() override {
        return container_->getNextSibling();
    }

    css::beans::Optional<cpo::uno::Reference<scriptinterop::XElement>> getParent() override {
        return container_->getParent();
    }

    css::beans::Optional<cpo::uno::Reference<scriptinterop::XElement>> getPreviousSibling() override
    { return container_->getPreviousSibling(); }

    OUString getText() override { return contents().text; }

    css::beans::Optional<scriptinterop::TextAlignment> getTextAlignment(sal_Int32 offset) override {
        auto const [text, inner] = at(offset);
        return text->getTextAlignment(inner);
    }

    cpo::uno::Sequence<sal_Int32> getTextAttributeIndices() override {
        std::vector<sal_Int32> v;
        for (auto const & paragraph: contents().paragraphs) {
            for (auto const i: paragraph.text->getTextAttributeIndices()) {
                v.push_back(paragraph.start + i);
            }
        }
        if (v.empty()) {
            v.push_back(0);
        }
        return cpo::uno::Sequence(v.data(), v.size());
    }

    cpo::uno::Reference<scriptinterop::XText> insertText(sal_Int32, OUString const &) override {
        throw notYetImplemented(u"insertText");
    }

    scriptinterop::ElementType getType() override { return container_->getType(); }

    css::beans::Optional<bool> isBold(sal_Int32 offset) override {
        auto const [text, inner] = at(offset);
        return text->isBold(inner);
    }

    css::beans::Optional<bool> isItalic(sal_Int32 offset) override {
        auto const [text, inner] = at(offset);
        return text->isItalic(inner);
    }

    css::beans::Optional<bool> isStrikethrough(sal_Int32 offset) override {
        auto const [text, inner] = at(offset);
        return text->isStrikethrough(inner);
    }

    css::beans::Optional<bool> isUnderline(sal_Int32 offset) override {
        auto const [text, inner] = at(offset);
        return text->isUnderline(inner);
    }

    cpo::uno::Reference<scriptinterop::XText> merge() override {
        throw notYetImplemented(u"merge");
    }

    cpo::uno::Reference<scriptinterop::XElement> removeFromParent() override {
        container_->removeFromParent();
        return this;
    }

    cpo::uno::Reference<scriptinterop::XText> setBold(
        css::beans::Optional<bool> const & value) override
    {
        for (auto const & paragraph: contents().paragraphs) {
            paragraph.text->setBold(value);
        }
        return this;
    }

    cpo::uno::Reference<scriptinterop::XText> setBoldRange(
        sal_Int32 startOffset, sal_Int32 endOffsetInclusive,
        css::beans::Optional<bool> const & value) override
    {
        forRange(
            u"setBold", startOffset, endOffsetInclusive,
            [value](auto const & text, sal_Int32 start, sal_Int32 end) {
                text->setBoldRange(start, end, value);
            });
        return this;
    }

    cpo::uno::Reference<scriptinterop::XText> setFontFamily(
        css::beans::Optional<OUString> const & fontFamilyName) override
    {
        for (auto const & paragraph: contents().paragraphs) {
            paragraph.text->setFontFamily(fontFamilyName);
        }
        return this;
    }

    cpo::uno::Reference<scriptinterop::XText> setFontFamilyRange(
        sal_Int32 startOffset, sal_Int32 endOffsetInclusive,
        css::beans::Optional<OUString> const & fontFamilyName) override
    {
        forRange(
            u"setFontFamily", startOffset, endOffsetInclusive,
            [&fontFamilyName](auto const & text, sal_Int32 start, sal_Int32 end) {
                text->setFontFamilyRange(start, end, fontFamilyName);
            });
        return this;
    }

    cpo::uno::Reference<scriptinterop::XText> setForegroundColor(
        css::beans::Optional<OUString> const & color) override
    {
        for (auto const & paragraph: contents().paragraphs) {
            paragraph.text->setForegroundColor(color);
        }
        return this;
    }

    cpo::uno::Reference<scriptinterop::XText> setForegroundColorRange(
        sal_Int32 startOffset, sal_Int32 endOffsetInclusive,
        css::beans::Optional<OUString> const & color) override
    {
        forRange(
            u"setForegroundColor", startOffset, endOffsetInclusive,
            [&color](auto const & text, sal_Int32 start, sal_Int32 end) {
                text->setForegroundColorRange(start, end, color);
            });
        return this;
    }

    cpo::uno::Reference<scriptinterop::XText> setItalic(
        css::beans::Optional<bool> const & value) override
    {
        for (auto const & paragraph: contents().paragraphs) {
            paragraph.text->setItalic(value);
        }
        return this;
    }

    cpo::uno::Reference<scriptinterop::XText> setItalicRange(
        sal_Int32 startOffset, sal_Int32 endOffsetInclusive,
        css::beans::Optional<bool> const & value) override
    {
        forRange(
            u"setItalic", startOffset, endOffsetInclusive,
            [value](auto const & text, sal_Int32 start, sal_Int32 end) {
                text->setItalicRange(start, end, value);
            });
        return this;
    }

    cpo::uno::Reference<scriptinterop::XText> setLinkUrl(
        css::beans::Optional<OUString> const & url) override
    {
        for (auto const & paragraph: contents().paragraphs) {
            paragraph.text->setLinkUrl(url);
        }
        return this;
    }

    cpo::uno::Reference<scriptinterop::XText> setLinkUrlRange(
        sal_Int32 startOffset, sal_Int32 endOffsetInclusive,
        css::beans::Optional<OUString> const & url) override
    {
        forRange(
            u"setLinkUrl", startOffset, endOffsetInclusive,
            [&url](auto const & text, sal_Int32 start, sal_Int32 end) {
                text->setLinkUrlRange(start, end, url);
            });
        return this;
    }

    cpo::uno::Reference<scriptinterop::XText> setStrikethrough(
        css::beans::Optional<bool> const & value) override
    {
        for (auto const & paragraph: contents().paragraphs) {
            paragraph.text->setStrikethrough(value);
        }
        return this;
    }

    cpo::uno::Reference<scriptinterop::XText> setStrikethroughRange(
        sal_Int32 startOffset, sal_Int32 endOffsetInclusive,
        css::beans::Optional<bool> const & value) override
    {
        forRange(
            u"setStrikethrough", startOffset, endOffsetInclusive,
            [value](auto const & text, sal_Int32 start, sal_Int32 end) {
                text->setStrikethroughRange(start, end, value);
            });
        return this;
    }

    cpo::uno::Reference<scriptinterop::XText> setText(OUString const &) override {
        throw notYetImplemented(u"setText");
    }

    cpo::uno::Reference<scriptinterop::XText> setTextAlignment(
        css::beans::Optional<scriptinterop::TextAlignment> const & textAlignment) override
    {
        textAlignmentArgument(textAlignment);
        for (auto const & paragraph: contents().paragraphs) {
            paragraph.text->setTextAlignment(textAlignment);
        }
        return this;
    }

    cpo::uno::Reference<scriptinterop::XText> setTextAlignmentRange(
        sal_Int32 startOffset, sal_Int32 endOffsetInclusive,
        css::beans::Optional<scriptinterop::TextAlignment> const & textAlignment) override
    {
        textAlignmentArgument(textAlignment);
        forRange(
            u"setTextAlignment", startOffset, endOffsetInclusive,
            [textAlignment](auto const & text, sal_Int32 start, sal_Int32 end) {
                text->setTextAlignmentRange(start, end, textAlignment);
            });
        return this;
    }

    cpo::uno::Reference<scriptinterop::XText> setUnderline(
        css::beans::Optional<bool> const & value) override
    {
        for (auto const & paragraph: contents().paragraphs) {
            paragraph.text->setUnderline(value);
        }
        return this;
    }

    cpo::uno::Reference<scriptinterop::XText> setUnderlineRange(
        sal_Int32 startOffset, sal_Int32 endOffsetInclusive,
        css::beans::Optional<bool> const & value) override
    {
        forRange(
            u"setUnderline", startOffset, endOffsetInclusive,
            [value](auto const & text, sal_Int32 start, sal_Int32 end) {
                text->setUnderlineRange(start, end, value);
            });
        return this;
    }

private:
    // The text of one paragraph, and where it starts in the text of the container:
    struct Paragraph {
        cpo::uno::Reference<scriptinterop::XText> text;
        sal_Int32 start;
        sal_Int32 length;
    };

    // The paragraphs of the container as the document has them now, and their texts joined with
    // line ends:
    struct Contents {
        std::vector<Paragraph> paragraphs;
        OUString text;
    };

    Contents contents() const {
        Contents result;
        collect(container_, result);
        return result;
    }

    // The children of a container, which a body and a table cell hand out in one go, so that a
    // walk over the container reads each of their children once:
    static std::vector<cpo::uno::Reference<scriptinterop::XElement>> childrenOf(
        cpo::uno::Reference<scriptinterop::XContainerElement> const & container)
    {
        cpo::uno::Sequence<cpo::uno::Reference<scriptinterop::XElement>> all;
        if (cpo::uno::Reference<scriptinterop::XBody> const body{container, cpo::uno::UNO_QUERY}) {
            all = body->getChildren();
        } else if (cpo::uno::Reference<scriptinterop::XTableCell> const cell{
                       container, cpo::uno::UNO_QUERY})
        {
            all = cell->getChildren();
        } else {
            std::vector<cpo::uno::Reference<scriptinterop::XElement>> children;
            auto const n = container->getNumChildren();
            for (sal_Int32 i = 0; i != n; ++i) {
                children.push_back(container->getChild(i));
            }
            return children;
        }
        return {all.begin(), all.end()};
    }

    static void collect(
        cpo::uno::Reference<scriptinterop::XContainerElement> const & container,
        Contents & contents)
    {
        for (auto const & child: childrenOf(container)) {
            if (cpo::uno::Reference<scriptinterop::XParagraph> const paragraph{
                    child, cpo::uno::UNO_QUERY})
            {
                if (!contents.paragraphs.empty()) {
                    contents.text += "\n";
                }
                auto const text = paragraph->editAsText();
                auto const string = text->getText();
                contents.paragraphs.push_back(
                    {text, contents.text.getLength(), string.getLength()});
                contents.text += string;
            } else if (cpo::uno::Reference<scriptinterop::XContainerElement> const nested{
                           child, cpo::uno::UNO_QUERY})
            {
                collect(nested, contents);
            } else {
                throw cpo::uno::RuntimeException(
                    "the text of a " + elementTypeName(container->getType())
                    + " cannot hold a " + elementTypeName(child->getType()));
            }
        }
    }

    static void checkIndex(Contents const & contents, sal_Int32 offset) {
        if (offset < 0 || offset >= contents.text.getLength()) {
            throw cpo::uno::RuntimeException(
                "Index (" + OUString::number(offset) + ") must be less than the content length ("
                + OUString::number(contents.text.getLength()) + ").");
        }
    }

    // Calls f with the text of each paragraph that the range reaches into, and the part of the
    // range inside it, leaving out the separators:
    template<typename F> void forRange(
        std::u16string_view method, sal_Int32 startOffset, sal_Int32 endOffsetInclusive, F f) const
    {
        if (startOffset < 0 || endOffsetInclusive < startOffset) {
            throw cpo::uno::RuntimeException(
                OUString::Concat(method)
                + ": expected 0 <= startOffset <= endOffsetInclusive, got startOffset="
                + OUString::number(startOffset) + ", endOffsetInclusive="
                + OUString::number(endOffsetInclusive));
        }
        auto const contents = this->contents();
        checkIndex(contents, endOffsetInclusive);
        for (auto const & paragraph: contents.paragraphs) {
            auto const start = std::max(startOffset, paragraph.start);
            auto const end = std::min(endOffsetInclusive, paragraph.start + paragraph.length - 1);
            if (start <= end) {
                f(paragraph.text, start - paragraph.start, end - paragraph.start);
            }
        }
    }

    cpo::uno::RuntimeException notYetImplemented(std::u16string_view method) {
        return cpo::uno::RuntimeException(
            OUString::Concat(method) + " is not yet implemented for the text of a "
            + elementTypeName(container_->getType())); // TODO
    }

    cpo::uno::Reference<scriptinterop::XContainerElement> container_;
};

template<typename T> cpo::uno::Reference<scriptinterop::XText> ElementImpl<T>::asText() {
    switch (this->getType()) {
    case scriptinterop::ElementType_BODY_SECTION:
    case scriptinterop::ElementType_FOOTNOTE_SECTION:
    case scriptinterop::ElementType_TABLE:
    case scriptinterop::ElementType_TABLE_CELL:
    case scriptinterop::ElementType_TABLE_ROW:
        return new ContainerTextImpl(cpo::uno::Reference<scriptinterop::XContainerElement>(
            static_cast<T *>(this), cpo::uno::UNO_QUERY_THROW));
    default:
        return cast<scriptinterop::XText>(scriptinterop::ElementType_TEXT);
    }
}

class InlineImageImpl: public ElementImpl<scriptinterop::XInlineImage> {
public:
    explicit InlineImageImpl(
        cpo::uno::Reference<scriptinterop::XElement> const & parent,
        cpo::uno::Reference<css::text::XTextContent> const & content):
        parent_(parent), content_(content) {}

    cpo::uno::Reference<cpo::uno::XInterface> getuno() override { return content_; }

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

    css::beans::Optional<cpo::uno::Reference<scriptinterop::XElement>> getNextSibling() override {
        return sibling(1);
    }

    css::beans::Optional<cpo::uno::Reference<scriptinterop::XElement>> getParent() override {
        return maybe(parent_);
    }

    css::beans::Optional<cpo::uno::Reference<scriptinterop::XElement>> getPreviousSibling() override
    { return sibling(-1); }

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

    cpo::uno::Reference<scriptinterop::XElement> removeFromParent() override {
        removeContent(content_);
        return this;
    }

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
    css::beans::Optional<cpo::uno::Reference<scriptinterop::XElement>> sibling(sal_Int32 step) {
        cpo::uno::Reference<scriptinterop::XContainerElement> const container(
            parent_, cpo::uno::UNO_QUERY);
        if (!container.is()) {
            return {false, {}};
        }
        auto const index = container->getChildIndex(this) + step;
        if (index < 0 || index >= container->getNumChildren()) {
            return {false, {}};
        }
        return {true, container->getChild(index)};
    }

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

class PositionedImageImpl: public cppu::WeakImplHelper<scriptinterop::XPositionedImage> {
public:
    PositionedImageImpl(
        std::shared_ptr<Model> const & model,
        cpo::uno::Reference<css::text::XTextContent> const & paragraph,
        cpo::uno::Reference<css::text::XTextContent> const & image):
        model_(model), paragraph_(paragraph), image_(image) {}

    cpo::uno::Reference<cpo::uno::XInterface> getuno() override { return image_; }

    sal_Int32 getHeight() override {
        sal_Int32 hundredthMm = 0;
        props()->getPropertyValue(u"Height"_ustr) >>= hundredthMm;
        return hundredthMmToPixels(hundredthMm);
    }

    OUString getId() override {
        return cpo::uno::Reference<css::container::XNamed>(image_, cpo::uno::UNO_QUERY_THROW)
            ->getName();
    }

    scriptinterop::PositionedLayout getLayout() override {
        css::text::WrapTextMode wrap = css::text::WrapTextMode_PARALLEL;
        props()->getPropertyValue(u"TextWrap"_ustr) >>= wrap;
        switch (wrap) {
        case css::text::WrapTextMode_THROUGH:
            return scriptinterop::PositionedLayout_ABOVE_TEXT;
        case css::text::WrapTextMode_NONE:
            return scriptinterop::PositionedLayout_BREAK_BOTH;
        case css::text::WrapTextMode_RIGHT:
            return scriptinterop::PositionedLayout_BREAK_LEFT;
        case css::text::WrapTextMode_LEFT:
            return scriptinterop::PositionedLayout_BREAK_RIGHT;
        default:
            return scriptinterop::PositionedLayout_WRAP_TEXT;
        }
    }

    double getLeftOffset() override { return points(u"HoriOrientPosition"_ustr); }

    cpo::uno::Reference<scriptinterop::XParagraph> getParagraph() override;

    double getTopOffset() override { return points(u"VertOrientPosition"_ustr); }

    sal_Int32 getWidth() override {
        sal_Int32 hundredthMm = 0;
        props()->getPropertyValue(u"Width"_ustr) >>= hundredthMm;
        return hundredthMmToPixels(hundredthMm);
    }

    cpo::uno::Reference<scriptinterop::XPositionedImage> setHeight(sal_Int32 height) override {
        props()->setPropertyValue(u"Height"_ustr, cpo::uno::Any(pixelsToHundredthMm(height)));
        return this;
    }

    cpo::uno::Reference<scriptinterop::XPositionedImage> setLayout(
        scriptinterop::PositionedLayout layout) override
    {
        auto wrap = css::text::WrapTextMode_PARALLEL;
        switch (layout) {
        case scriptinterop::PositionedLayout_ABOVE_TEXT:
            wrap = css::text::WrapTextMode_THROUGH;
            props()->setPropertyValue(u"Opaque"_ustr, cpo::uno::Any(true));
            break;
        case scriptinterop::PositionedLayout_BREAK_BOTH:
            wrap = css::text::WrapTextMode_NONE;
            break;
        case scriptinterop::PositionedLayout_BREAK_LEFT:
            wrap = css::text::WrapTextMode_RIGHT;
            break;
        case scriptinterop::PositionedLayout_BREAK_RIGHT:
            wrap = css::text::WrapTextMode_LEFT;
            break;
        default:
            break;
        }
        props()->setPropertyValue(u"TextWrap"_ustr, cpo::uno::Any(wrap));
        return this;
    }

    cpo::uno::Reference<scriptinterop::XPositionedImage> setLeftOffset(double offset) override {
        props()->setPropertyValue(
            u"HoriOrient"_ustr, cpo::uno::Any(css::text::HoriOrientation::NONE));
        setPoints(u"HoriOrientPosition"_ustr, offset);
        return this;
    }

    cpo::uno::Reference<scriptinterop::XPositionedImage> setTopOffset(double offset) override {
        props()->setPropertyValue(
            u"VertOrient"_ustr, cpo::uno::Any(css::text::VertOrientation::NONE));
        setPoints(u"VertOrientPosition"_ustr, offset);
        return this;
    }

    cpo::uno::Reference<scriptinterop::XPositionedImage> setWidth(sal_Int32 width) override {
        props()->setPropertyValue(u"Width"_ustr, cpo::uno::Any(pixelsToHundredthMm(width)));
        return this;
    }

    cpo::uno::Reference<scriptinterop::XPositionedImage> setId(OUString const & id) override {
        cpo::uno::Reference<css::container::XNamed>(image_, cpo::uno::UNO_QUERY_THROW)
            ->setName(id);
        return this;
    }

    css::beans::Optional<sal_Int32> getPageAnchor() override {
        auto anchor = css::text::TextContentAnchorType_AT_PARAGRAPH;
        props()->getPropertyValue(u"AnchorType"_ustr) >>= anchor;
        if (anchor != css::text::TextContentAnchorType_AT_PAGE) {
            return {false, {}};
        }
        sal_Int16 page = 0;
        props()->getPropertyValue(u"AnchorPageNo"_ustr) >>= page;
        return {true, page};
    }

    cpo::uno::Reference<scriptinterop::XPositionedImage> setPageAnchor(sal_Int32 page) override {
        if (page < 1 || page > SAL_MAX_INT16) {
            throw cpo::uno::RuntimeException(
                "setPageAnchor: invalid page number " + OUString::number(page));
        }
        props()->setPropertyValue(
            u"AnchorType"_ustr, cpo::uno::Any(css::text::TextContentAnchorType_AT_PAGE));
        props()->setPropertyValue(u"AnchorPageNo"_ustr, cpo::uno::Any(sal_Int16(page)));
        props()->setPropertyValue(
            u"HoriOrientRelation"_ustr, cpo::uno::Any(css::text::RelOrientation::PAGE_FRAME));
        props()->setPropertyValue(
            u"VertOrientRelation"_ustr, cpo::uno::Any(css::text::RelOrientation::PAGE_FRAME));
        return this;
    }

private:
    cpo::uno::Reference<css::beans::XPropertySet> props() {
        return cpo::uno::Reference<css::beans::XPropertySet>(image_, cpo::uno::UNO_QUERY_THROW);
    }

    double points(OUString const & name) {
        sal_Int32 hundredthMm = 0;
        props()->getPropertyValue(name) >>= hundredthMm;
        return o3tl::convert(double(hundredthMm), o3tl::Length::mm100, o3tl::Length::pt);
    }

    void setPoints(OUString const & name, double value) {
        props()->setPropertyValue(name, cpo::uno::Any(static_cast<sal_Int32>(std::round(
            o3tl::convert(value, o3tl::Length::pt, o3tl::Length::mm100)))));
    }

    std::shared_ptr<Model> model_;
    cpo::uno::Reference<css::text::XTextContent> paragraph_;
    cpo::uno::Reference<css::text::XTextContent> image_;
};

class ParagraphImpl : public ElementImpl<scriptinterop::XParagraph>
{
public:
    ParagraphImpl(
        std::shared_ptr<Model> const & model,
        cpo::uno::Reference<scriptinterop::XElement> const & parent,
        cpo::uno::Reference<css::text::XTextContent> const& content)
        : model_(model), parent_(parent), content_(content)
    {
    }

    cpo::uno::Reference<cpo::uno::XInterface> getuno() override { return content_; }

    cpo::uno::Reference<scriptinterop::XPositionedImage> addPositionedImage(
        cpo::uno::Reference<scriptinterop::XBlob> const & image) override
    {
        auto const graphic = createGraphicFromBlob(model_, image);
        cpo::uno::Reference<css::beans::XPropertySet> const props(
            graphic, cpo::uno::UNO_QUERY_THROW);
        // As in GAS, the image is positioned relative to the paragraph and has the text wrap
        // around it:
        props->setPropertyValue(
            u"AnchorType"_ustr, cpo::uno::Any(css::text::TextContentAnchorType_AT_PARAGRAPH));
        props->setPropertyValue(
            u"HoriOrient"_ustr, cpo::uno::Any(css::text::HoriOrientation::NONE));
        props->setPropertyValue(
            u"HoriOrientRelation"_ustr, cpo::uno::Any(css::text::RelOrientation::FRAME));
        props->setPropertyValue(u"HoriOrientPosition"_ustr, cpo::uno::Any(sal_Int32(0)));
        props->setPropertyValue(
            u"VertOrient"_ustr, cpo::uno::Any(css::text::VertOrientation::NONE));
        props->setPropertyValue(
            u"VertOrientRelation"_ustr, cpo::uno::Any(css::text::RelOrientation::FRAME));
        props->setPropertyValue(u"VertOrientPosition"_ustr, cpo::uno::Any(sal_Int32(0)));
        props->setPropertyValue(
            u"TextWrap"_ustr, cpo::uno::Any(css::text::WrapTextMode_PARALLEL));
        cpo::uno::Reference<css::text::XTextRange> const range(content_, cpo::uno::UNO_QUERY_THROW);
        range->getText()->insertTextContent(range->getStart(), graphic, false);
        return new PositionedImageImpl(model_, content_, graphic);
    }

    cpo::uno::Reference<scriptinterop::XText> asText() override {
        return new TextImpl(model_, parent_, content_, getType());
    }

    cpo::uno::Reference<scriptinterop::XContainerElement> clear() override {
        cpo::uno::Reference<css::text::XTextRange>(content_, cpo::uno::UNO_QUERY_THROW)
            ->setString(OUString());
        return this;
    }

    // TODO: return a detached deep copy, not this; mutations on the "copy" write back to the live
    // element:
    cpo::uno::Reference<scriptinterop::XElement> copy() override { return this; }

    cpo::uno::Reference<scriptinterop::XText> editAsText() override {
        return asText();
    }

    css::beans::Optional<cpo::uno::Reference<scriptinterop::XRangeElement>> findText(
        OUString const & searchPattern) override
    { return asText()->findText(searchPattern); }

    css::beans::Optional<cpo::uno::Reference<scriptinterop::XRangeElement>> findTextFrom(
        OUString const & searchPattern,
        cpo::uno::Reference<scriptinterop::XRangeElement> const & from) override
    { return asText()->findTextFrom(searchPattern, from); }

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

    cpo::uno::Reference<scriptinterop::XElement> getChild(sal_Int32 index) override {
        auto const & children = this->children();
        if (index < 0 || o3tl::make_unsigned(index) >= children.size()) {
            return {};
        }
        auto const & child = children[index];
        if (child.image.is()) {
            return new InlineImageImpl(this, child.image);
        }
        return new TextImpl(model_, this, content_, scriptinterop::ElementType_TEXT, child.run);
    }

    sal_Int32 getChildIndex(cpo::uno::Reference<scriptinterop::XElement> const & child) override
    { return childIndexOf(*this, child); }

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

    cpo::uno::Reference<scriptinterop::XParagraph> setHeading(
        scriptinterop::ParagraphHeading heading) override
    {
        OUString style;
        switch (heading) {
        case scriptinterop::ParagraphHeading_NORMAL:
            style = u"Text body"_ustr;
            break;
        case scriptinterop::ParagraphHeading_HEADING1:
            style = u"Heading 1"_ustr;
            break;
        case scriptinterop::ParagraphHeading_HEADING2:
            style = u"Heading 2"_ustr;
            break;
        case scriptinterop::ParagraphHeading_HEADING3:
            style = u"Heading 3"_ustr;
            break;
        case scriptinterop::ParagraphHeading_HEADING4:
            style = u"Heading 4"_ustr;
            break;
        case scriptinterop::ParagraphHeading_HEADING5:
            style = u"Heading 5"_ustr;
            break;
        case scriptinterop::ParagraphHeading_HEADING6:
            style = u"Heading 6"_ustr;
            break;
        case scriptinterop::ParagraphHeading_TITLE:
            style = u"Title"_ustr;
            break;
        case scriptinterop::ParagraphHeading_SUBTITLE:
            style = u"Subtitle"_ustr;
            break;
        default:
            throw cpo::uno::RuntimeException(u"setHeading: unknown heading"_ustr);
        }
        cpo::uno::Reference<css::beans::XPropertySet> const props(
            content_, cpo::uno::UNO_QUERY_THROW);
        props->setPropertyValue(u"ParaStyleName"_ustr, cpo::uno::Any(style));
        return this;
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

    cpo::uno::Reference<scriptinterop::XPositionedImage> getPositionedImage(OUString const & id)
        override
    {
        for (auto const & image: enumeratePositionedImages()) {
            if (image->getId() == id) {
                return image;
            }
        }
        throw cpo::uno::RuntimeException(
            "getPositionedImage: the paragraph has no positioned image with ID " + id);
    }

    cpo::uno::Sequence<cpo::uno::Reference<scriptinterop::XPositionedImage>> getPositionedImages()
        override
    {
        auto const images = enumeratePositionedImages();
        return cpo::uno::Sequence<cpo::uno::Reference<scriptinterop::XPositionedImage>>(
            images.data(), images.size());
    }

    css::beans::Optional<cpo::uno::Reference<scriptinterop::XElement>> getNextSibling() override {
        return maybe(siblingContent(model_, content_, parent_, true));
    }

    sal_Int32 getNumChildren() override { return children().size(); }

    css::beans::Optional<cpo::uno::Reference<scriptinterop::XElement>> getParent() override {
        return maybe(parent_);
    }

    css::beans::Optional<cpo::uno::Reference<scriptinterop::XElement>> getPreviousSibling() override
    { return maybe(siblingContent(model_, content_, parent_, false)); }

    OUString getText() override
    {
        return textOf(
            cpo::uno::Reference<css::text::XTextRange>(content_, cpo::uno::UNO_QUERY_THROW));
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

    cpo::uno::Reference<scriptinterop::XElement> removeFromParent() override {
        removeParagraph(content_);
        return this;
    }

    bool removePositionedImage(OUString const & id) override {
        auto const image = getPositionedImage(id);
        cpo::uno::Reference<css::text::XTextContent> const content(
            image->getuno(), cpo::uno::UNO_QUERY_THROW);
        cpo::uno::Reference<css::text::XTextRange>(content_, cpo::uno::UNO_QUERY_THROW)->getText()
            ->removeTextContent(content);
        return true;
    }

private:
    // As in GAS, the children are the inline images and the runs of text that are not empty, each
    // run with the name of the image before it:
    struct Child {
        cpo::uno::Reference<css::text::XTextContent> image;
        OUString run;
    };

    std::vector<Child> const & children() {
        return children_.get(model_->generation(), [this] {
            std::vector<Child> children;
            for (auto const & part: paragraphParts(content_)) {
                if (part.image.is()) {
                    children.push_back({part.image, {}});
                } else if (!textOf(part.range).isEmpty()) {
                    children.push_back({{}, part.after});
                }
            }
            return children;
        });
    }

    // The images anchored to the paragraph, which its content enumeration has, and those anchored
    // to one of its characters, which its text portions have:
    std::vector<cpo::uno::Reference<scriptinterop::XPositionedImage>> enumeratePositionedImages()
    {
        std::vector<cpo::uno::Reference<scriptinterop::XPositionedImage>> images;
        cpo::uno::Reference<css::container::XContentEnumerationAccess> const access(
            content_, cpo::uno::UNO_QUERY_THROW);
        auto const contents
            = access->createContentEnumeration(u"com.sun.star.text.TextContent"_ustr);
        while (contents.is() && contents->hasMoreElements()) {
            cpo::uno::Reference<css::text::XTextContent> frame;
            contents->nextElement() >>= frame;
            cpo::uno::Reference<css::lang::XServiceInfo> const info(frame, cpo::uno::UNO_QUERY);
            if (info.is() && info->supportsService(u"com.sun.star.text.TextGraphicObject"_ustr)) {
                images.push_back(new PositionedImageImpl(model_, content_, frame));
            }
        }
        for (auto const & frame:
                 enumerateFramedImages(css::text::TextContentAnchorType_AT_CHARACTER))
        {
            images.push_back(new PositionedImageImpl(model_, content_, frame));
        }
        return images;
    }

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

    std::vector<cpo::uno::Reference<css::text::XTextContent>> enumerateFramedImages(
        css::text::TextContentAnchorType anchorType)
    {
        std::vector<cpo::uno::Reference<css::text::XTextContent>> images;
        forEachFramedImage(
            content_, anchorType,
            [&images](auto const &, cpo::uno::Reference<css::text::XTextContent> const & frame) {
                images.push_back(frame);
            });
        return images;
    }

    std::shared_ptr<Model> model_;
    cpo::uno::Reference<scriptinterop::XElement> parent_;
    cpo::uno::Reference<css::text::XTextContent> content_;
    Cached<std::vector<Child>> children_;
};

// A paragraph's text view casts back to the paragraph, as it reports the paragraph's type:
cpo::uno::Reference<scriptinterop::XParagraph> TextImpl::asListItem() {
    if (reportedType_ != scriptinterop::ElementType_LIST_ITEM) {
        return ElementImpl::asListItem();
    }
    return new ParagraphImpl(model_, parent_, content_);
}

cpo::uno::Reference<scriptinterop::XParagraph> TextImpl::asParagraph() {
    if (reportedType_ != scriptinterop::ElementType_PARAGRAPH) {
        return ElementImpl::asParagraph();
    }
    return new ParagraphImpl(model_, parent_, content_);
}

// The child text of a paragraph or list item that holds the character at an offset into all of the
// paragraph's text, and the offset there (an offset past the last character being in the last child
// text, and null for a paragraph without text):
std::pair<cpo::uno::Reference<scriptinterop::XText>, sal_Int32> textChildAt(
    cpo::uno::Reference<scriptinterop::XContainerElement> const & paragraph, sal_Int32 offset)
{
    std::pair<cpo::uno::Reference<scriptinterop::XText>, sal_Int32> last;
    sal_Int32 start = 0;
    auto const n = paragraph->getNumChildren();
    for (sal_Int32 i = 0; i != n; ++i) {
        auto const child = paragraph->getChild(i);
        if (child->getType() != scriptinterop::ElementType_TEXT) {
            continue;
        }
        cpo::uno::Reference<scriptinterop::XText> const text(child, cpo::uno::UNO_QUERY_THROW);
        auto const length = text->getText().getLength();
        if (offset < start + length) {
            return {text, offset - start};
        }
        last = {text, offset - start};
        start += length;
    }
    return last;
}

// The element that holds the paragraphs of a text, which is the body for the body text and the
// table cell for the text of a table cell, and null for any other text (like a header or a frame):
cpo::uno::Reference<scriptinterop::XElement> containerOf(
    std::shared_ptr<Model> const & model, cpo::uno::Reference<css::text::XText> const & text);

cpo::uno::Reference<css::text::XTextTable> tableOf(
    std::shared_ptr<Model> const & model, cpo::uno::Reference<css::text::XText> const & cell);

// The element that holds a text table, which is the body for a table in the body text and the table
// cell for a table nested in a cell, and null for a table elsewhere (like in a header):
cpo::uno::Reference<scriptinterop::XElement> tableParentOf(
    std::shared_ptr<Model> const & model, cpo::uno::Reference<css::text::XTextTable> const & table);

// A paragraph, with the element that holds it as its parent:
cpo::uno::Reference<scriptinterop::XParagraph> paragraphOf(
    std::shared_ptr<Model> const & model,
    cpo::uno::Reference<css::text::XTextContent> const & paragraph);

// A text made with editAsText has the parent of its paragraph, and a child text or the text from
// getSurroundingText has its paragraph as its parent:
cpo::uno::Reference<scriptinterop::XParagraph> TextImpl::paragraph() {
    if (reportedType_ != scriptinterop::ElementType_TEXT) {
        return new ParagraphImpl(model_, parent_, content_);
    }
    return cpo::uno::Reference<scriptinterop::XParagraph>(parent_, cpo::uno::UNO_QUERY_THROW);
}

cpo::uno::Reference<scriptinterop::XParagraph> PositionedImageImpl::getParagraph() {
    if (getPageAnchor().IsPresent) {
        throw cpo::uno::RuntimeException(u"getParagraph: the image is anchored to a page"_ustr);
    }
    return paragraphOf(model_, paragraph_);
}

class TableCellImpl: public ElementImpl<scriptinterop::XTableCell>, public ContentsContainer {
public:
    TableCellImpl(
        std::shared_ptr<Model> const & model,
        cpo::uno::Reference<scriptinterop::XElement> const & parent,
        cpo::uno::Reference<css::text::XText> const & text):
        model_(model), parent_(parent), text_(text) {}

    cpo::uno::Reference<cpo::uno::XInterface> getuno() override { return text_; }

    cpo::uno::Reference<scriptinterop::XContainerElement> clear() override {
        if (!text_.is()) {
            throw cpo::uno::RuntimeException(u"clear: the table cell has no text"_ustr);
        }
        text_->setString(OUString());
        return this;
    }

    cpo::uno::Reference<scriptinterop::XText> editAsText() override { return asText(); }

    css::beans::Optional<cpo::uno::Reference<scriptinterop::XRangeElement>> findText(
        OUString const & searchPattern) override
    { return asText()->findText(searchPattern); }

    css::beans::Optional<cpo::uno::Reference<scriptinterop::XRangeElement>> findTextFrom(
        OUString const & searchPattern,
        cpo::uno::Reference<scriptinterop::XRangeElement> const & from) override
    { return asText()->findTextFrom(searchPattern, from); }

    // TODO: return a detached deep copy, not this; mutations on the "copy" write back to the live
    // element:
    cpo::uno::Reference<scriptinterop::XElement> copy() override { return this; }

    cpo::uno::Reference<scriptinterop::XElement> getChild(sal_Int32 index) override {
        auto const & list = contents();
        return index >= 0 && o3tl::make_unsigned(index) < list.size()
            ? elementOf(model_, this, list[index]) : nullptr;
    }

    sal_Int32 getChildIndex(cpo::uno::Reference<scriptinterop::XElement> const & child) override
    { return childIndexOf(*this, child); }

    cpo::uno::Sequence<cpo::uno::Reference<scriptinterop::XElement>> getChildren() override {
        return elementsOf(model_, this, contents());
    }

    // TODO: Writer text tables encode a horizontal merge in the anchor cell's name (a merged cell
    // is named for its range like "A1.B2"); parsing that would let us return the true span instead
    // of 1:
    sal_Int32 getColSpan() override { return 1; }

    // TODO: real sibling walk that steps through the containing row's cells:
    css::beans::Optional<cpo::uno::Reference<scriptinterop::XElement>> getNextSibling() override {
        return {false, {}};
    }

    sal_Int32 getNumChildren() override { return contents().size(); }

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
        return textOf(text_);
    }

    cpo::uno::Reference<scriptinterop::XElement> removeFromParent() override {
        throw cpo::uno::RuntimeException(
            u"a table cell cannot be removed on its own; remove its row instead"_ustr);
    }

    std::vector<cpo::uno::Reference<css::text::XTextContent>> const & contents() override {
        return contents_.get(model_->generation(), [this] { return enumerateContents(text_); });
    }

private:
    std::shared_ptr<Model> model_;
    cpo::uno::Reference<scriptinterop::XElement> parent_;
    cpo::uno::Reference<css::text::XText> text_;
    Cached<std::vector<cpo::uno::Reference<css::text::XTextContent>>> contents_;
};

class TableRowImpl: public ElementImpl<scriptinterop::XTableRow> {
public:
    TableRowImpl(
        std::shared_ptr<Model> const & model,
        cpo::uno::Reference<scriptinterop::XElement> const & parent,
        cpo::uno::Reference<css::text::XTextTable> const & table, sal_Int32 rowIndex):
        model_(model), parent_(parent), table_(table), rowIndex_(rowIndex) {}

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

    cpo::uno::Reference<scriptinterop::XContainerElement> clear() override {
        throw cpo::uno::RuntimeException(u"TableRow.clear is not yet implemented"_ustr); // TODO
    }

    cpo::uno::Reference<scriptinterop::XText> editAsText() override { return asText(); }

    css::beans::Optional<cpo::uno::Reference<scriptinterop::XRangeElement>> findText(
        OUString const & searchPattern) override
    { return asText()->findText(searchPattern); }

    css::beans::Optional<cpo::uno::Reference<scriptinterop::XRangeElement>> findTextFrom(
        OUString const & searchPattern,
        cpo::uno::Reference<scriptinterop::XRangeElement> const & from) override
    { return asText()->findTextFrom(searchPattern, from); }

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
            cpo::uno::Reference<scriptinterop::XTableCell>(new TableCellImpl(model_, this, text))};
    }

    cpo::uno::Reference<scriptinterop::XElement> getChild(sal_Int32 index) override {
        auto const cell = getCell(index);
        return cell.IsPresent ? cell.Value : nullptr;
    }

    sal_Int32 getChildIndex(cpo::uno::Reference<scriptinterop::XElement> const & child) override
    { return childIndexOf(*this, child); }

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
            if (i != 0) {
                buf.append('\n');
            }
            buf.append(cell.Value->getText());
        }
        return buf.makeStringAndClear();
    }

    cpo::uno::Reference<scriptinterop::XElement> removeFromParent() override {
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
        return this;
    }

private:
    std::shared_ptr<Model> model_;
    cpo::uno::Reference<scriptinterop::XElement> parent_;
    cpo::uno::Reference<css::text::XTextTable> table_;
    sal_Int32 rowIndex_;
};

class TableImpl: public ElementImpl<scriptinterop::XTable> {
public:
    TableImpl(
        std::shared_ptr<Model> const & model,
        cpo::uno::Reference<scriptinterop::XElement> const & parent,
        cpo::uno::Reference<css::text::XTextTable> const & table):
        model_(model), parent_(parent), table_(table) {}

    cpo::uno::Reference<cpo::uno::XInterface> getuno() override { return table_; }

    cpo::uno::Reference<scriptinterop::XContainerElement> clear() override {
        throw cpo::uno::RuntimeException(u"Table.clear is not yet implemented"_ustr); // TODO
    }

    cpo::uno::Reference<scriptinterop::XText> editAsText() override { return asText(); }

    css::beans::Optional<cpo::uno::Reference<scriptinterop::XRangeElement>> findText(
        OUString const & searchPattern) override
    { return asText()->findText(searchPattern); }

    css::beans::Optional<cpo::uno::Reference<scriptinterop::XRangeElement>> findTextFrom(
        OUString const & searchPattern,
        cpo::uno::Reference<scriptinterop::XRangeElement> const & from) override
    { return asText()->findTextFrom(searchPattern, from); }

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
            cpo::uno::Reference<scriptinterop::XTableRow>(
                new TableRowImpl(model_, this, table_, index))};
    }

    cpo::uno::Reference<scriptinterop::XElement> getChild(sal_Int32 index) override {
        auto const row = getRow(index);
        return row.IsPresent ? row.Value : nullptr;
    }

    sal_Int32 getChildIndex(cpo::uno::Reference<scriptinterop::XElement> const & child) override
    { return childIndexOf(*this, child); }

    css::beans::Optional<cpo::uno::Reference<scriptinterop::XElement>> getNextSibling() override {
        return maybe(siblingContent(model_, table_, parent_, true));
    }

    sal_Int32 getNumChildren() override { return getNumRows(); }

    css::beans::Optional<cpo::uno::Reference<scriptinterop::XElement>> getParent() override {
        return maybe(parent_);
    }

    css::beans::Optional<cpo::uno::Reference<scriptinterop::XElement>> getPreviousSibling() override
    { return maybe(siblingContent(model_, table_, parent_, false)); }

    OUString getText() override {
        OUStringBuffer buf;
        auto const n = getNumRows();
        for (sal_Int32 i = 0; i != n; ++i) {
            auto const row = getRow(i);
            if (!row.IsPresent) {
                throw cpo::uno::RuntimeException(
                    "getText: the table has no row " + OUString::number(i));
            }
            if (i != 0) {
                buf.append('\n');
            }
            buf.append(row.Value->getText());
        }
        return buf.makeStringAndClear();
    }

    cpo::uno::Reference<scriptinterop::XElement> removeFromParent() override {
        removeContent(cpo::uno::Reference<css::text::XTextContent>(table_, cpo::uno::UNO_QUERY_THROW));
        return this;
    }

private:
    std::shared_ptr<Model> model_;
    cpo::uno::Reference<scriptinterop::XElement> parent_;
    cpo::uno::Reference<css::text::XTextTable> table_;
};

std::vector<cpo::uno::Reference<css::text::XTextContent>> enumerateContents(
    cpo::uno::Reference<css::text::XText> const & text)
{
    std::vector<cpo::uno::Reference<css::text::XTextContent>> v;
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
                v.push_back(xtc);
            } else if (info->supportsService(u"com.sun.star.text.TextTable"_ustr)) {
                cpo::uno::Reference<css::text::XTextTable> const table(xtc, cpo::uno::UNO_QUERY);
                if (table.is()) {
                    v.push_back(xtc);
                }
            }
        }
    }
    return v;
}

cpo::uno::Reference<scriptinterop::XElement> elementOf(
    std::shared_ptr<Model> const & model,
    cpo::uno::Reference<scriptinterop::XElement> const & parent,
    cpo::uno::Reference<css::text::XTextContent> const & content)
{
    if (cpo::uno::Reference<css::text::XTextTable> const table{content, cpo::uno::UNO_QUERY}) {
        return new TableImpl(model, parent, table);
    }
    return new ParagraphImpl(model, parent, content);
}

cpo::uno::Sequence<cpo::uno::Reference<scriptinterop::XElement>> elementsOf(
    std::shared_ptr<Model> const & model,
    cpo::uno::Reference<scriptinterop::XElement> const & parent,
    std::vector<cpo::uno::Reference<css::text::XTextContent>> const & contents)
{
    std::vector<cpo::uno::Reference<scriptinterop::XElement>> v;
    for (auto const & content: contents) {
        v.push_back(elementOf(model, parent, content));
    }
    return cpo::uno::Sequence(v.data(), v.size());
}

cpo::uno::Sequence<cpo::uno::Reference<scriptinterop::XElement>> enumerateElements(
    std::shared_ptr<Model> const & model,
    cpo::uno::Reference<css::text::XText> const & text,
    cpo::uno::Reference<scriptinterop::XElement> const & parent)
{
    return elementsOf(model, parent, enumerateContents(text));
}

cpo::uno::Reference<scriptinterop::XElement> siblingContent(
    std::shared_ptr<Model> const & model,
    cpo::uno::Reference<css::text::XTextContent> const & content,
    cpo::uno::Reference<scriptinterop::XElement> const & parent, bool forward)
{
    if (!content.is()) {
        return nullptr;
    }
    // A parent that caches its children has the content among them, unless the content has moved to
    // another text since; the raw pointers match for a paragraph, while a table is held as its
    // XTextTable, so that only a comparison of the objects is guaranteed to find it:
    if (auto const container = dynamic_cast<ContentsContainer *>(parent.get())) {
        auto const & contents = container->contents();
        auto i = std::find_if(contents.begin(), contents.end(), [&content](auto const & other) {
            return other.get() == content.get();
        });
        if (i == contents.end()) {
            i = std::find(contents.begin(), contents.end(), content);
        }
        if (i != contents.end()) {
            if (forward) {
                ++i;
                return i == contents.end() ? nullptr : elementOf(model, parent, *i);
            } else {
                return i == contents.begin() ? nullptr : elementOf(model, parent, *(i - 1));
            }
        }
    }
    cpo::uno::Reference<css::text::XTextRange> const range(content, cpo::uno::UNO_QUERY);
    if (!range.is()) {
        return nullptr;
    }
    auto const host = range->getText();
    if (!host.is()) {
        return nullptr;
    }
    auto const list = enumerateElements(model, host, parent);
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

cpo::uno::Reference<scriptinterop::XText> TextImpl::merge() {
    auto const previous = getPreviousSibling();
    if (!previous.IsPresent || previous.Value->getType() != getType()) {
        throw cpo::uno::RuntimeException(
            u"Element must be preceded by an element of the same type."_ustr);
    }
    cpo::uno::Reference<css::text::XTextRange> const before(
        previous.Value->getuno(), cpo::uno::UNO_QUERY_THROW);
    auto const cursor = before->getText()->createTextCursorByRange(before->getEnd());
    cursor->gotoRange(wholeRange()->getStart(), true);
    cursor->setString(OUString());
    // Writer keeps the second paragraph when the first one is empty, so the merged text is that of
    // the paragraph that holds the place where the two were joined:
    return paragraphOf(model_, findContainingParagraph(cursor))->asText();
}

class RangeElementImpl: public cppu::WeakImplHelper<scriptinterop::XRangeElement> {
public:
    RangeElementImpl(
        std::shared_ptr<Model> const & model,
        cpo::uno::Reference<css::text::XTextRange> const & range, bool partial = false):
        model_(model), range_(range), paragraph_(findContainingParagraph(range)), partial_(partial)
    {
        rangeLen_ = textOf(range).getLength();
        if (!paragraph_.is()) {
            return;
        }
        cpo::uno::Reference<css::text::XTextRange> const paraRange(
            paragraph_, cpo::uno::UNO_QUERY_THROW);
        auto const host = paraRange->getText();
        if (!host.is()) {
            throw cpo::uno::RuntimeException(u"the paragraph is not part of the document"_ustr);
        }
        auto const probe = host->createTextCursorByRange(paraRange->getStart());
        if (!probe.is()) {
            throw cpo::uno::RuntimeException(u"cannot place a cursor in the paragraph"_ustr);
        }
        probe->gotoRange(range->getStart(), true);
        startOffset_ = textOf(probe).getLength();
        // An inline image takes up no character of the text, so only the range's start and end
        // tell whether it covers all of its paragraph (and a paragraph without text has no text
        // that a range could be a part of):
        cpo::uno::Reference<css::text::XTextRangeCompare> const cmp(
            host, cpo::uno::UNO_QUERY_THROW);
        whole_ = textOf(paraRange).isEmpty()
            || (cmp->compareRegionStarts(range, paraRange) == 0
                && cmp->compareRegionEnds(range, paraRange) == 0);
    }

    cpo::uno::Reference<cpo::uno::XInterface> getuno() override { return range_; }

    cpo::uno::Reference<scriptinterop::XElement> getElement() override {
        if (!paragraph_.is()) {
            throw cpo::uno::RuntimeException(
                u"getElement: this range element is not inside a paragraph"_ustr);
        }
        if (!isPartial()) {
            return paragraphOf(model_, paragraph_);
        }
        // Same as in GAS, the element of a partial range element is the Text that the offsets
        // count in:
        auto const text = textChild().first;
        if (!text.is()) {
            throw cpo::uno::RuntimeException(
                u"getElement: this range element is not inside a text"_ustr);
        }
        return text;
    }

    sal_Int32 getEndOffsetInclusive() override {
        if (!isPartial()) {
            return -1;
        }
        auto const start = textChild().second;
        return rangeLen_ == 0 ? start : start + rangeLen_ - 1;
    }

    sal_Int32 getStartOffset() override {
        if (!isPartial()) {
            return -1;
        }
        return textChild().second;
    }

    bool isPartial() override {
        if (partial_ || !paragraph_.is()) {
            return true;
        }
        return !whole_;
    }

private:
    std::pair<cpo::uno::Reference<scriptinterop::XText>, sal_Int32> const & textChild() {
        if (!textChild_) {
            textChild_ = textChildAt(paragraphOf(model_, paragraph_), startOffset_);
        }
        return *textChild_;
    }

    std::shared_ptr<Model> model_;
    cpo::uno::Reference<css::text::XTextRange> range_;
    cpo::uno::Reference<css::text::XTextContent> paragraph_;
    bool partial_;
    bool whole_ = false;
    std::optional<std::pair<cpo::uno::Reference<scriptinterop::XText>, sal_Int32>> textChild_;
    sal_Int32 startOffset_ = 0;
    sal_Int32 rangeLen_ = 0;
};

std::vector<cpo::uno::Reference<scriptinterop::XText>> TextImpl::childTexts() {
    if (run_) {
        return {this};
    }
    std::vector<cpo::uno::Reference<scriptinterop::XText>> texts;
    auto const paragraph = this->paragraph();
    auto const n = paragraph->getNumChildren();
    for (sal_Int32 i = 0; i != n; ++i) {
        auto const child = paragraph->getChild(i);
        if (child->getType() == scriptinterop::ElementType_TEXT) {
            texts.emplace_back(child, cpo::uno::UNO_QUERY_THROW);
        }
    }
    return texts;
}

cpo::uno::Reference<scriptinterop::XRangeElement> TextImpl::part(sal_Int32 start, sal_Int32 end) {
    auto const whole = wholeRange();
    auto const cursor = whole->getText()->createTextCursorByRange(whole->getStart());
    goRight(cursor, start, false);
    goRight(cursor, end - start, true);
    return new RangeElementImpl(model_, cursor, true);
}

// As in GAS, the first match of a regular expression in the texts, after the given range element of
// an earlier match when there is one, where each text is searched on its own, so that no match
// spans the inline image between two child texts of a paragraph:
css::beans::Optional<cpo::uno::Reference<scriptinterop::XRangeElement>> findInTexts(
    std::vector<cpo::uno::Reference<scriptinterop::XText>> const & texts,
    OUString const & searchPattern, cpo::uno::Reference<scriptinterop::XRangeElement> const & from)
{
    std::size_t first = 0;
    sal_Int32 offset = 0;
    if (from.is()) {
        rtl::Reference<TextImpl> const element(dynamic_cast<TextImpl *>(from->getElement().get()));
        auto const i = !element.is()
            ? texts.end()
            : std::find_if(texts.begin(), texts.end(), [&element](auto const & text) {
                  auto & impl = dynamic_cast<TextImpl &>(*text);
                  return impl.getuno() == element->getuno() && impl.run() == element->run();
              });
        if (i == texts.end()) {
            throw cpo::uno::RuntimeException(
                u"findText: the range element to search from is not in this text"_ustr);
        }
        first = i - texts.begin();
        offset = from->getEndOffsetInclusive() + 1;
    }
    // As in GAS, an invalid pattern is an error, which TextSearch2 (using the same regular
    // expressions of ICU) does not report:
    UParseError parseError;
    UErrorCode status = U_ZERO_ERROR;
    std::unique_ptr<icu::RegexPattern> const pattern(icu::RegexPattern::compile(
        icu::UnicodeString(searchPattern.getStr(), searchPattern.getLength()), 0, parseError,
        status));
    if (U_FAILURE(status)) {
        throw cpo::uno::RuntimeException("findText: invalid regular expression " + searchPattern);
    }
    css::util::SearchOptions2 options;
    options.AlgorithmType2 = css::util::SearchAlgorithms2::REGEXP;
    options.searchString = searchPattern;
    auto const search = css::util::TextSearch2::create(comphelper::getProcessComponentContext());
    search->setOptions2(options);
    for (auto i = first; i != texts.size(); ++i, offset = 0) {
        auto const text = texts[i]->getText();
        for (auto start = offset; start < text.getLength();) {
            auto const result = search->searchForward(text, start, text.getLength());
            if (result.subRegExpressions == 0) {
                break;
            }
            // An empty match, as a pattern like "x*" has everywhere, is skipped:
            if (result.endOffset[0] > result.startOffset[0]) {
                return {
                    true,
                    dynamic_cast<TextImpl &>(*texts[i]).part(
                        result.startOffset[0], result.endOffset[0])};
            }
            start = result.startOffset[0] + 1;
        }
    }
    return {false, {}};
}

css::beans::Optional<cpo::uno::Reference<scriptinterop::XRangeElement>> TextImpl::findTextFrom(
    OUString const & searchPattern, cpo::uno::Reference<scriptinterop::XRangeElement> const & from)
{
    return findInTexts(childTexts(), searchPattern, from);
}

css::beans::Optional<cpo::uno::Reference<scriptinterop::XRangeElement>>
ContainerTextImpl::findTextFrom(
    OUString const & searchPattern, cpo::uno::Reference<scriptinterop::XRangeElement> const & from)
{
    std::vector<cpo::uno::Reference<scriptinterop::XText>> texts;
    for (auto const & paragraph: contents().paragraphs) {
        auto const children = dynamic_cast<TextImpl &>(*paragraph.text).childTexts();
        texts.insert(texts.end(), children.begin(), children.end());
    }
    return findInTexts(texts, searchPattern, from);
}

// An element that is selected as a whole, such as a table cell:
class WholeElementRangeElementImpl: public cppu::WeakImplHelper<scriptinterop::XRangeElement> {
public:
    explicit WholeElementRangeElementImpl(
        cpo::uno::Reference<scriptinterop::XElement> const & element):
        element_(element) {}

    cpo::uno::Reference<cpo::uno::XInterface> getuno() override { return element_->getuno(); }

    cpo::uno::Reference<scriptinterop::XElement> getElement() override { return element_; }

    sal_Int32 getEndOffsetInclusive() override { return -1; }

    sal_Int32 getStartOffset() override { return -1; }

    bool isPartial() override { return false; }

private:
    cpo::uno::Reference<scriptinterop::XElement> element_;
};

class ImageRangeElementImpl: public cppu::WeakImplHelper<scriptinterop::XRangeElement> {
public:
    ImageRangeElementImpl(
        std::shared_ptr<Model> const & model,
        cpo::uno::Reference<css::text::XTextContent> const & image):
        model_(model), image_(image) {}

    cpo::uno::Reference<cpo::uno::XInterface> getuno() override { return image_; }

    cpo::uno::Reference<scriptinterop::XElement> getElement() override {
        auto const anchor = image_->getAnchor();
        if (!anchor.is()) {
            throw cpo::uno::RuntimeException(u"getElement: the image has no anchor"_ustr);
        }
        auto const para = findContainingParagraph(anchor);
        if (!para.is()) {
            throw cpo::uno::RuntimeException(
                u"getElement: the image is not anchored in a paragraph"_ustr);
        }
        return new InlineImageImpl(paragraphOf(model_, para), image_);
    }

    sal_Int32 getEndOffsetInclusive() override { return -1; }

    sal_Int32 getStartOffset() override { return -1; }

    bool isPartial() override { return false; }

private:
    std::shared_ptr<Model> model_;
    cpo::uno::Reference<css::text::XTextContent> image_;
};

// Walk the host XText and push one RangeElementImpl per paragraph that the raw range overlaps.
// The sub-range for each paragraph is the intersection of the raw range with the paragraph's
// extent, so a paragraph that lies fully inside the raw range becomes a non-partial element and
// the first and last paragraphs become partial ones.  A table that lies fully inside the raw
// range becomes a whole element, and an empty piece at either end, as when the raw range ends at
// the start of the paragraph after a table, is dropped unless it is the only one (and with partial,
// as for a text given with offsets, a whole paragraph is split at its inline images like a partial
// one):
void splitAtParagraphBoundaries(
    std::shared_ptr<Model> const & model,
    cpo::uno::Reference<css::text::XTextRange> const & range,
    std::vector<cpo::uno::Reference<scriptinterop::XRangeElement>> & out, bool partial = false)
{
    assert(range.is());
    auto host = range->getText();
    if (!host.is()) {
        throw cpo::uno::RuntimeException(u"the selected range is not part of the document"_ustr);
    }
    auto rStart = range->getStart();
    auto const rEnd = range->getEnd();
    // For a range from a table cell to after the table (as Select All gives in a body that starts
    // with a table), Writer gives the cell as the text (also when it is in a table nested in the
    // cell), so the range starts with the table that lies in the text that holds the end (which
    // only a comparison tells, as Writer creates cursors in a cell also at positions outside it):
    auto const holdsEnd = [&rEnd](cpo::uno::Reference<css::text::XText> const & text) {
        try {
            cpo::uno::Reference<css::text::XTextRangeCompare>(text, cpo::uno::UNO_QUERY_THROW)
                ->compareRegionStarts(text->getStart(), rEnd);
            return true;
        } catch (css::lang::IllegalArgumentException const &) {
            return false;
        }
    };
    for (auto text = host; !holdsEnd(text);) {
        cpo::uno::Reference<css::lang::XServiceInfo> const cell(text, cpo::uno::UNO_QUERY);
        if (!cell.is() || !cell->supportsService(u"com.sun.star.text.CellProperties"_ustr)) {
            break;
        }
        auto const table = tableOf(model, text);
        auto const parent = table.is() ? tableParentOf(model, table) : nullptr;
        cpo::uno::Reference<css::text::XText> const around(
            parent.is() ? parent->getuno() : nullptr, cpo::uno::UNO_QUERY);
        if (!around.is()) {
            break;
        }
        if (holdsEnd(around)) {
            host = around;
            rStart = table->getAnchor();
            break;
        }
        text = around;
    }
    cpo::uno::Reference<css::text::XTextRangeCompare> const cmp(host, cpo::uno::UNO_QUERY_THROW);
    cpo::uno::Reference<css::container::XEnumerationAccess> const ea(
        host, cpo::uno::UNO_QUERY_THROW);
    std::vector<cpo::uno::Reference<scriptinterop::XRangeElement>> pieces;
    std::vector<bool> empty;
    auto const en = ea->createEnumeration();
    while (en.is() && en->hasMoreElements()) {
        cpo::uno::Reference<css::text::XTextContent> xtc;
        en->nextElement() >>= xtc;
        if (!xtc.is()) {
            continue;
        }
        if (cpo::uno::Reference<css::text::XTextTable> const table{xtc, cpo::uno::UNO_QUERY}) {
            auto const anchor = table->getAnchor();
            try {
                if (anchor.is() && cmp->compareRegionStarts(rStart, anchor) >= 0
                    && cmp->compareRegionEnds(anchor, rEnd) >= 0)
                {
                    pieces.emplace_back(new WholeElementRangeElementImpl(
                        elementOf(model, containerOf(model, host), xtc)));
                    empty.push_back(false);
                }
            } catch (css::lang::IllegalArgumentException const & e) {
                SAL_WARN(
                    "scriptinterop",
                    "splitAtParagraphBoundaries: comparing a table failed: " << e.Message);
            }
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
            rtl::Reference<RangeElementImpl> const piece(new RangeElementImpl(model, cursor));
            if ((!partial && !piece->isPartial()) || cursor->isCollapsed()) {
                pieces.emplace_back(piece);
                empty.push_back(cursor->isCollapsed());
                continue;
            }
            auto const parts = paragraphParts(xtc);
            if (parts.size() == 1) {
                pieces.emplace_back(piece);
                empty.push_back(false);
                continue;
            }
            // As in GAS, a part of a paragraph is split at the paragraph's inline images, into the
            // images and the pieces of the runs of text that it holds:
            for (auto const & part: parts) {
                if (part.image.is()) {
                    if (cmp->compareRegionStarts(cursor, part.range) >= 0
                        && cmp->compareRegionEnds(part.range, cursor) >= 0)
                    {
                        pieces.emplace_back(new ImageRangeElementImpl(model, part.image));
                        empty.push_back(false);
                    }
                    continue;
                }
                auto const from = cmp->compareRegionStarts(part.range, cursor) > 0
                    ? cursor->getStart() : part.range->getStart();
                auto const to = cmp->compareRegionEnds(part.range, cursor) > 0
                    ? part.range->getEnd() : cursor->getEnd();
                if (cmp->compareRegionStarts(from, to) > 0) {
                    auto const run = host->createTextCursorByRange(from);
                    run->gotoRange(to, true);
                    pieces.emplace_back(new RangeElementImpl(model, run));
                    empty.push_back(false);
                }
            }
        } catch (css::lang::IllegalArgumentException const & e) {
            SAL_WARN(
                "scriptinterop",
                "splitAtParagraphBoundaries: compareRegionStarts failed: " << e.Message);
        }
    }
    std::size_t first = 0;
    std::size_t last = pieces.size();
    if (last - first > 1 && empty.front()) {
        ++first;
    }
    if (last - first > 1 && empty.back()) {
        --last;
    }
    out.insert(out.end(), pieces.begin() + first, pieces.begin() + last);
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
        splitAtParagraphBoundaries(model_, range, v);
    }
    return cpo::uno::Sequence(v.data(), v.size());
}

// An inline image that is selected on its own, as one whole range element:
class ImageSelectionImpl: public cppu::WeakImplHelper<scriptinterop::XSelection> {
public:
    ImageSelectionImpl(
        std::shared_ptr<Model> const & model,
        cpo::uno::Reference<css::text::XTextContent> const & image):
        model_(model), image_(image) {}

    cpo::uno::Reference<cpo::uno::XInterface> getuno() override { return image_; }

    cpo::uno::Sequence<cpo::uno::Reference<scriptinterop::XRangeElement>> getRangeElements()
        override
    { return {new ImageRangeElementImpl(model_, image_)}; }

    cpo::uno::Sequence<cpo::uno::Reference<scriptinterop::XRangeElement>> getSelectedElements()
        override
    { return getRangeElements(); }

    OUString getText() override { return {}; }

    void replace(OUString const &) override {
        throw cpo::uno::RuntimeException(
            u"replace: the selection is an image, which has no text to replace"_ustr);
    }

private:
    std::shared_ptr<Model> model_;
    cpo::uno::Reference<css::text::XTextContent> image_;
};

cpo::uno::Reference<css::text::XTextViewCursor> viewCursorOf(
    std::shared_ptr<Model> const & model)
{
    cpo::uno::Reference<css::text::XTextViewCursorSupplier> const sup(
        model->uno()->getCurrentController(), cpo::uno::UNO_QUERY_THROW);
    auto const c = sup->getViewCursor();
    if (!c.is()) {
        throw cpo::uno::RuntimeException(u"the document view has no cursor"_ustr);
    }
    return c;
}

class CursorImpl: public cppu::WeakImplHelper<scriptinterop::XCursor> {
public:
    explicit CursorImpl(std::shared_ptr<Model> const & model): model_(model) {}

    // A position at one place in the document, rather than wherever the view cursor is, which is
    // at offset into element:
    CursorImpl(
        std::shared_ptr<Model> const & model,
        cpo::uno::Reference<css::text::XTextRange> const & position,
        cpo::uno::Reference<scriptinterop::XElement> const & element, sal_Int32 offset):
        model_(model), position_(position), element_(element), offset_(offset) {}

    cpo::uno::Reference<cpo::uno::XInterface> getuno() override { return where(); }

    cpo::uno::Reference<scriptinterop::XElement> getElement() override {
        if (element_.is()) {
            return element_;
        }
        auto const para = findContainingParagraph(where());
        if (!para.is()) {
            throw cpo::uno::RuntimeException(
                u"getElement: the cursor is not inside a paragraph"_ustr);
        }
        return paragraphOf(model_, para);
    }

    OUString getLocale() override { return bcp47(u"CharLocale"_ustr); }

    OUString getLocaleAsian() override { return bcp47(u"CharLocaleAsian"_ustr); }

    OUString getLocaleComplex() override { return bcp47(u"CharLocaleComplex"_ustr); }

    sal_Int32 getOffset() override { return element_.is() ? offset_ : paragraphOffset(); }

    cpo::uno::Reference<scriptinterop::XText> getSurroundingText() override {
        auto const para = findContainingParagraph(where());
        if (!para.is()) {
            throw cpo::uno::RuntimeException(
                u"getSurroundingText: the cursor is not inside a paragraph"_ustr);
        }
        return new TextImpl(
            model_, paragraphOf(model_, para), para, scriptinterop::ElementType_TEXT);
    }

    sal_Int32 getSurroundingTextOffset() override { return paragraphOffset(); }

    css::beans::Optional<cpo::uno::Reference<scriptinterop::XInlineImage>> insertInlineImage(
        cpo::uno::Reference<scriptinterop::XBlob> const & blob) override
    {
        auto const c = where();
        auto const host = c->getText();
        if (!host.is()) {
            throw cpo::uno::RuntimeException(
                u"insertInlineImage: the cursor is not somewhere that accepts inline images"_ustr);
        }
        auto const graphic = createGraphicFromBlob(model_, blob);
        host->insertTextContent(c->getStart(), graphic, false);
        auto const para = findContainingParagraph(graphic->getAnchor());
        return {
            true,
            new InlineImageImpl(
                para.is()
                    ? paragraphOf(model_, para) : cpo::uno::Reference<scriptinterop::XParagraph>(),
                graphic)};
    }

    void insertText(OUString const & text) override {
        auto const c = where();
        auto const host = c->getText();
        if (!host.is()) {
            throw cpo::uno::RuntimeException(
                u"insertText: the cursor is not somewhere that accepts text"_ustr);
        }
        host->insertString(c->getStart(), text, false);
    }

    bool isBold() override {
        float weight = css::awt::FontWeight::NORMAL;
        property(u"CharWeight"_ustr) >>= weight;
        return weight >= css::awt::FontWeight::BOLD;
    }

    bool isItalic() override {
        css::awt::FontSlant slant = css::awt::FontSlant_NONE;
        property(u"CharPosture"_ustr) >>= slant;
        return slant == css::awt::FontSlant_ITALIC || slant == css::awt::FontSlant_OBLIQUE;
    }

    bool isUnderline() override {
        sal_Int16 underline = css::awt::FontUnderline::NONE;
        property(u"CharUnderline"_ustr) >>= underline;
        return underline != css::awt::FontUnderline::NONE;
    }

    cpo::uno::Reference<scriptinterop::XCursor> setBold(
        css::beans::Optional<bool> const & value) override
    {
        if (value.IsPresent) {
            TextImpl::setBoldOn(where(), value);
        } else {
            reset(u"CharWeight"_ustr);
        }
        return this;
    }

    cpo::uno::Reference<scriptinterop::XCursor> setItalic(
        css::beans::Optional<bool> const & value) override
    {
        if (value.IsPresent) {
            TextImpl::setItalicOn(where(), value);
        } else {
            reset(u"CharPosture"_ustr);
        }
        return this;
    }

    cpo::uno::Reference<scriptinterop::XCursor> setUnderline(
        css::beans::Optional<bool> const & value) override
    {
        if (value.IsPresent) {
            TextImpl::setUnderlineOn(where(), value);
        } else {
            reset(u"CharUnderline"_ustr);
        }
        return this;
    }

private:
    // An empty Locale is an empty string, rather than the system locale that LanguageTag would make
    // of it:
    OUString bcp47(OUString const & name) {
        auto const locale = property(name).get<css::lang::Locale>();
        if (locale.Language.isEmpty()) {
            return {};
        }
        return LanguageTag(locale).getBcp47();
    }

    cpo::uno::Any property(OUString const & name) {
        return cpo::uno::Reference<css::beans::XPropertySet>(where(), cpo::uno::UNO_QUERY_THROW)
            ->getPropertyValue(name);
    }

    // The start of the cursor is a position without a mark, which a reset inside a word extends to
    // the whole word, as setting an attribute does:
    void reset(OUString const & name) {
        cpo::uno::Reference<css::beans::XPropertyState>(
            where()->getStart(), cpo::uno::UNO_QUERY_THROW)->setPropertyToDefault(name);
    }

    cpo::uno::Reference<css::text::XTextRange> where() {
        return position_.is() ? position_ : viewCursorOf(model_);
    }

    sal_Int32 paragraphOffset() {
        auto const c = where();
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
        return textOf(probe).getLength();
    }

    std::shared_ptr<Model> model_;
    cpo::uno::Reference<css::text::XTextRange> position_;
    cpo::uno::Reference<scriptinterop::XElement> element_;
    sal_Int32 offset_ = 0;
};

class FootnoteSectionImpl:
    public ElementImpl<scriptinterop::XContainerElement>, public ContentsContainer {
public:
    FootnoteSectionImpl(
        std::shared_ptr<Model> const & model,
        cpo::uno::Reference<css::text::XText> const & text):
        model_(model), text_(text) {}

    cpo::uno::Reference<cpo::uno::XInterface> getuno() override { return text_; }

    cpo::uno::Reference<scriptinterop::XContainerElement> clear() override {
        if (!text_.is()) {
            throw cpo::uno::RuntimeException(u"clear: the footnote section has no text"_ustr);
        }
        text_->setString(OUString());
        return this;
    }

    cpo::uno::Reference<scriptinterop::XText> editAsText() override { return asText(); }

    css::beans::Optional<cpo::uno::Reference<scriptinterop::XRangeElement>> findText(
        OUString const & searchPattern) override
    { return asText()->findText(searchPattern); }

    css::beans::Optional<cpo::uno::Reference<scriptinterop::XRangeElement>> findTextFrom(
        OUString const & searchPattern,
        cpo::uno::Reference<scriptinterop::XRangeElement> const & from) override
    { return asText()->findTextFrom(searchPattern, from); }

    // TODO: return a detached deep copy, not this; mutations on the "copy" write back to the live
    // element:
    cpo::uno::Reference<scriptinterop::XElement> copy() override { return this; }

    cpo::uno::Reference<scriptinterop::XElement> getChild(sal_Int32 index) override {
        auto const & list = contents();
        return index >= 0 && o3tl::make_unsigned(index) < list.size()
            ? elementOf(model_, this, list[index]) : nullptr;
    }

    sal_Int32 getChildIndex(cpo::uno::Reference<scriptinterop::XElement> const & child) override
    { return childIndexOf(*this, child); }

    // TODO: a footnote section is not part of a sibling list in our model:
    css::beans::Optional<cpo::uno::Reference<scriptinterop::XElement>> getNextSibling() override {
        return {false, {}};
    }

    sal_Int32 getNumChildren() override { return contents().size(); }

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
        return textOf(text_);
    }

    scriptinterop::ElementType getType() override {
        return scriptinterop::ElementType_FOOTNOTE_SECTION;
    }

    cpo::uno::Reference<scriptinterop::XElement> removeFromParent() override {
        throw cpo::uno::RuntimeException(
            u"a footnote section cannot be removed on its own; remove its footnote instead"_ustr);
    }

    std::vector<cpo::uno::Reference<css::text::XTextContent>> const & contents() override {
        return contents_.get(model_->generation(), [this] { return enumerateContents(text_); });
    }

private:
    std::shared_ptr<Model> model_;
    cpo::uno::Reference<css::text::XText> text_;
    Cached<std::vector<cpo::uno::Reference<css::text::XTextContent>>> contents_;
};

class FootnoteImpl: public ElementImpl<scriptinterop::XFootnote> {
public:
    FootnoteImpl(
        std::shared_ptr<Model> const & model,
        cpo::uno::Reference<css::text::XFootnote> const & footnote):
        model_(model), footnote_(footnote) {}

    cpo::uno::Reference<cpo::uno::XInterface> getuno() override { return footnote_; }

    // TODO: return a detached deep copy, not this; mutations on the "copy" write back to the live
    // element:
    cpo::uno::Reference<scriptinterop::XElement> copy() override { return this; }

    css::beans::Optional<cpo::uno::Reference<scriptinterop::XContainerElement>>
        getFootnoteContents() override
    {
        cpo::uno::Reference<css::text::XText> const text(footnote_, cpo::uno::UNO_QUERY);
        if (!text.is()) return {false, {}};
        return {true, new FootnoteSectionImpl(model_, text)};
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
        return textOf(cpo::uno::Reference<css::text::XText>(footnote_, cpo::uno::UNO_QUERY_THROW));
    }

    scriptinterop::ElementType getType() override { return scriptinterop::ElementType_FOOTNOTE; }

    cpo::uno::Reference<scriptinterop::XElement> removeFromParent() override {
        removeContent(
            cpo::uno::Reference<css::text::XTextContent>(footnote_, cpo::uno::UNO_QUERY_THROW));
        return this;
    }

private:
    std::shared_ptr<Model> model_;
    cpo::uno::Reference<css::text::XFootnote> footnote_;
};

class BodyImpl: public ElementImpl<scriptinterop::XBody>, public ContentsContainer {
public:
    explicit BodyImpl(
        std::shared_ptr<Model> const & model,
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
        return new InlineImageImpl(
            new ParagraphImpl(model_, this, findContainingParagraph(cursor)), graphic);
    }

    cpo::uno::Reference<scriptinterop::XParagraph> appendListItem(OUString const & text) override {
        return appendImpl(text, u"List Number"_ustr);
    }

    cpo::uno::Reference<scriptinterop::XParagraph> appendParagraph(OUString const & text) override {
        return appendImpl(text, u""_ustr);
    }

    cpo::uno::Reference<scriptinterop::XContainerElement> clear() override {
        text_->setString(OUString());
        cpo::uno::Reference<css::container::XEnumerationAccess> const paragraphs(
            text_, cpo::uno::UNO_QUERY_THROW);
        cpo::uno::Reference<css::text::XTextContent> const kept(
            paragraphs->createEnumeration()->nextElement(), cpo::uno::UNO_QUERY_THROW);
        makeNormal(
            new ParagraphImpl(model_, this, kept),
            cpo::uno::Reference<css::beans::XPropertySet>(kept, cpo::uno::UNO_QUERY_THROW));
        return this;
    }

    cpo::uno::Reference<scriptinterop::XText> editAsText() override { return asText(); }

    css::beans::Optional<cpo::uno::Reference<scriptinterop::XRangeElement>> findText(
        OUString const & searchPattern) override
    { return asText()->findText(searchPattern); }

    css::beans::Optional<cpo::uno::Reference<scriptinterop::XRangeElement>> findTextFrom(
        OUString const & searchPattern,
        cpo::uno::Reference<scriptinterop::XRangeElement> const & from) override
    { return asText()->findTextFrom(searchPattern, from); }

    // TODO: return a detached deep copy, not this; mutations on the "copy" write back to the live
    // element:
    cpo::uno::Reference<scriptinterop::XElement> copy() override { return this; }

    cpo::uno::Reference<cpo::uno::XInterface> getuno() override { return text_; }

    cpo::uno::Reference<scriptinterop::XElement> getChild(sal_Int32 index) override {
        auto const & list = contents();
        return index >= 0 && o3tl::make_unsigned(index) < list.size()
            ? elementOf(model_, this, list[index]) : nullptr;
    }

    sal_Int32 getChildIndex(cpo::uno::Reference<scriptinterop::XElement> const & child) override
    { return childIndexOf(*this, child); }

    cpo::uno::Sequence<cpo::uno::Reference<scriptinterop::XElement>> getChildren() override {
        return elementsOf(model_, this, contents());
    }

    css::beans::Optional<cpo::uno::Reference<scriptinterop::XElement>> getNextSibling() override {
        return {false, {}};
    }

    sal_Int32 getNumChildren() override { return contents().size(); }

    double getMarginLeft() override { return pageStyleLength(u"LeftMargin"_ustr); }

    double getMarginTop() override { return pageStyleLength(u"TopMargin"_ustr); }

    double getPageHeight() override { return pageStyleLength(u"Height"_ustr); }

    double getPageWidth() override { return pageStyleLength(u"Width"_ustr); }

    cpo::uno::Sequence<cpo::uno::Reference<scriptinterop::XPositionedImage>> getPositionedImages()
        override
    {
        auto const images = pageAnchoredImages();
        return cpo::uno::Sequence<cpo::uno::Reference<scriptinterop::XPositionedImage>>(
            images.data(), images.size());
    }

    bool removePositionedImage(OUString const & id) override {
        for (auto const & image: pageAnchoredImages()) {
            if (image->getId() == id) {
                text_->removeTextContent(cpo::uno::Reference<css::text::XTextContent>(
                    image->getuno(), cpo::uno::UNO_QUERY_THROW));
                return true;
            }
        }
        throw cpo::uno::RuntimeException(
            "removePositionedImage: no image with ID " + id + " is anchored to a page");
    }

    css::beans::Optional<cpo::uno::Reference<scriptinterop::XElement>> getParent() override {
        return {false, {}};
    }

    css::beans::Optional<cpo::uno::Reference<scriptinterop::XElement>> getPreviousSibling() override
    { return {false, {}}; }

    OUString getText() override {
        if (!text_.is()) {
            throw cpo::uno::RuntimeException(u"getText: the document body has no text"_ustr);
        }
        return textOf(text_);
    }

    scriptinterop::ElementType getType() override {
        return scriptinterop::ElementType_BODY_SECTION;
    }

    cpo::uno::Reference<scriptinterop::XElement> removeFromParent() override {
        throw cpo::uno::RuntimeException(u"the body has no parent to remove it from"_ustr);
    }

    std::vector<cpo::uno::Reference<css::text::XTextContent>> const & contents() override {
        return model_->bodyContents().get(
            model_->generation(), [this] { return enumerateContents(text_); });
    }

private:
    std::vector<cpo::uno::Reference<scriptinterop::XPositionedImage>> pageAnchoredImages() {
        std::vector<cpo::uno::Reference<scriptinterop::XPositionedImage>> images;
        auto const objects = cpo::uno::Reference<css::text::XTextGraphicObjectsSupplier>(
            model_->uno(), cpo::uno::UNO_QUERY_THROW)->getGraphicObjects();
        for (auto const & name: objects->getElementNames()) {
            cpo::uno::Reference<css::text::XTextContent> const frame(
                objects->getByName(name), cpo::uno::UNO_QUERY_THROW);
            auto anchor = css::text::TextContentAnchorType_AT_PARAGRAPH;
            cpo::uno::Reference<css::beans::XPropertySet>(frame, cpo::uno::UNO_QUERY_THROW)
                ->getPropertyValue(u"AnchorType"_ustr) >>= anchor;
            if (anchor == css::text::TextContentAnchorType_AT_PAGE) {
                images.push_back(new PositionedImageImpl(model_, nullptr, frame));
            }
        }
        return images;
    }

    // In points, of the page style that the body starts with, as GAS has one page size and one set
    // of margins for the whole document:
    double pageStyleLength(OUString const & dimension) {
        OUString name;
        cpo::uno::Reference<css::beans::XPropertySet>(
            text_->createTextCursorByRange(text_->getStart()), cpo::uno::UNO_QUERY_THROW)
            ->getPropertyValue(u"PageStyleName"_ustr) >>= name;
        // The start of the body has no page style name when it has no layout, as when it is hidden,
        // and then the default page style, Standard, stands in:
        if (name.isEmpty()) {
            name = u"Standard"_ustr;
        }
        cpo::uno::Reference<css::container::XNameAccess> const styles(
            cpo::uno::Reference<css::style::XStyleFamiliesSupplier>(
                model_->uno(), cpo::uno::UNO_QUERY_THROW)->getStyleFamilies()->getByName(
                    u"PageStyles"_ustr),
            cpo::uno::UNO_QUERY_THROW);
        sal_Int32 size = 0;
        cpo::uno::Reference<css::beans::XPropertySet>(
            styles->getByName(name), cpo::uno::UNO_QUERY_THROW)->getPropertyValue(dimension)
            >>= size;
        return o3tl::convert(double(size), o3tl::Length::mm100, o3tl::Length::pt);
    }

    cpo::uno::Reference<scriptinterop::XParagraph> appendImpl(
        OUString const & text, OUString const & paraStyle)
    {
        if (!text_.is()) {
            throw cpo::uno::RuntimeException(u"XBody has no underlying text"_ustr);
        }
        cpo::uno::Reference<css::text::XParagraphAppend> const append(
            text_, cpo::uno::UNO_QUERY_THROW);
        append->finishParagraph({});
        text_->insertString(text_->getEnd(), text, false);
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
        cpo::uno::Reference<scriptinterop::XParagraph> const paragraph(
            new ParagraphImpl(model_, this, lastParagraph));
        cpo::uno::Reference<css::beans::XPropertySet> const props(
            lastParagraph, cpo::uno::UNO_QUERY_THROW);
        if (paraStyle.isEmpty()) {
            makeNormal(paragraph, props);
        } else {
            try {
                props->setPropertyValue(u"ParaStyleName"_ustr, cpo::uno::Any(paraStyle));
            } catch (css::lang::IllegalArgumentException const &) {
                // Document lacks the requested style; build a bullet NumberingRules from
                // scratch and apply it, so the paragraph is a real list item regardless of
                // which paragraph styles the document has registered:
                applyBulletNumbering(props);
            }
        }
        return paragraph;
    }

    // In GAS, an appended paragraph and the paragraph that a cleared body keeps are plain
    // paragraphs, while Writer can carry over a list or a heading style:
    static void makeNormal(
        cpo::uno::Reference<scriptinterop::XParagraph> const & paragraph,
        cpo::uno::Reference<css::beans::XPropertySet> const & props)
    {
        if (paragraph->getType() == scriptinterop::ElementType_LIST_ITEM) {
            paragraph->setHeading(scriptinterop::ParagraphHeading_NORMAL);
            props->setPropertyValue(u"NumberingRules"_ustr, cpo::uno::Any());
        } else if (auto const heading = paragraph->getHeading();
            heading.IsPresent && heading.Value != scriptinterop::ParagraphHeading_NORMAL)
        {
            paragraph->setHeading(scriptinterop::ParagraphHeading_NORMAL);
        }
    }

    void applyBulletNumbering(cpo::uno::Reference<css::beans::XPropertySet> const & props) {
        cpo::uno::Reference<css::lang::XMultiServiceFactory> const factory(
            model_->uno(), cpo::uno::UNO_QUERY_THROW);
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

    std::shared_ptr<Model> model_;
    cpo::uno::Reference<css::text::XText> text_;
};

// Selected table cells, each a whole range element of its own, in the order of the rows and of the
// cells in each row (as in GAS, which also brings a range of table cells into that order).  The UNO
// object is the cell range that the cells make up, and null when they do not make up a whole cell
// range:
class CellSelectionImpl: public cppu::WeakImplHelper<scriptinterop::XSelection> {
public:
    CellSelectionImpl(
        cpo::uno::Reference<cpo::uno::XInterface> const & uno,
        std::vector<cpo::uno::Reference<scriptinterop::XTableCell>> && cells):
        uno_(uno), cells_(std::move(cells)) {}

    cpo::uno::Reference<cpo::uno::XInterface> getuno() override { return uno_; }

    cpo::uno::Sequence<cpo::uno::Reference<scriptinterop::XRangeElement>> getRangeElements()
        override
    {
        cpo::uno::Sequence<cpo::uno::Reference<scriptinterop::XRangeElement>> elements(
            cells_.size());
        std::transform(
            cells_.begin(), cells_.end(), elements.getArray(),
            [](auto const & cell) -> cpo::uno::Reference<scriptinterop::XRangeElement> {
                return new WholeElementRangeElementImpl(cell);
            });
        return elements;
    }

    cpo::uno::Sequence<cpo::uno::Reference<scriptinterop::XRangeElement>> getSelectedElements()
        override
    { return getRangeElements(); }

    OUString getText() override {
        OUStringBuffer buf;
        for (auto const & cell: cells_) {
            if (!buf.isEmpty()) {
                buf.append('\n');
            }
            buf.append(cell->getText());
        }
        return buf.makeStringAndClear();
    }

    void replace(OUString const & newText) override {
        for (auto const & cell: cells_) {
            cpo::uno::Reference<css::text::XText>(cell->getuno(), cpo::uno::UNO_QUERY_THROW)
                ->setString(newText);
        }
    }

private:
    cpo::uno::Reference<cpo::uno::XInterface> uno_;
    std::vector<cpo::uno::Reference<scriptinterop::XTableCell>> cells_;
};

// Writer names a table cell after its position, like "B2":
OUString cellNameOf(cpo::uno::Reference<cpo::uno::XInterface> const & cell) {
    OUString name;
    cpo::uno::Reference<css::beans::XPropertySet>(cell, cpo::uno::UNO_QUERY_THROW)
        ->getPropertyValue(u"CellName"_ustr) >>= name;
    return name;
}

// The text table that a table cell is in, which is the one of the document's text tables that has
// that very cell under its name (as a text cursor in the cell can be in a table nested in the cell):
cpo::uno::Reference<css::text::XTextTable> tableOf(
    std::shared_ptr<Model> const & model, cpo::uno::Reference<css::text::XText> const & cell)
{
    auto const name = cellNameOf(cell);
    cpo::uno::Reference<css::container::XIndexAccess> const tables(
        cpo::uno::Reference<css::text::XTextTablesSupplier>(
            model->uno(), cpo::uno::UNO_QUERY_THROW)->getTextTables(),
        cpo::uno::UNO_QUERY_THROW);
    auto const n = tables->getCount();
    for (sal_Int32 i = 0; i != n; ++i) {
        cpo::uno::Reference<css::text::XTextTable> const table(
            tables->getByIndex(i), cpo::uno::UNO_QUERY_THROW);
        if (table->getCellByName(name) == cell) {
            return table;
        }
    }
    return {};
}

// The names of the cells of a cell range:
std::set<OUString> cellNamesOf(cpo::uno::Reference<css::table::XCellRange> const & range) {
    std::set<OUString> names;
    for (sal_Int32 row = 0;; ++row) {
        sal_Int32 column = 0;
        for (;; ++column) {
            cpo::uno::Reference<css::table::XCell> cell;
            try {
                cell = range->getCellByPosition(column, row);
            } catch (css::lang::IndexOutOfBoundsException const &) {
                break;
            }
            if (!cell.is()) {
                break;
            }
            names.insert(cellNameOf(cell));
        }
        if (column == 0) {
            break;
        }
    }
    return names;
}

// The cells of a text table that have the given names, in the order of the rows and of the cells in
// each row:
std::vector<cpo::uno::Reference<scriptinterop::XTableCell>> cellsNamed(
    std::shared_ptr<Model> const & model, cpo::uno::Reference<css::text::XTextTable> const & table,
    std::set<OUString> const & names)
{
    cpo::uno::Reference<scriptinterop::XTable> const tableElement(
        new TableImpl(model, tableParentOf(model, table), table));
    std::vector<cpo::uno::Reference<scriptinterop::XTableCell>> cells;
    auto const rows = tableElement->getNumRows();
    for (sal_Int32 i = 0; i != rows; ++i) {
        auto const row = tableElement->getRow(i);
        if (!row.IsPresent) {
            continue;
        }
        auto const n = row.Value->getNumCells();
        for (sal_Int32 j = 0; j != n; ++j) {
            auto const cell = row.Value->getCell(j);
            if (cell.IsPresent && names.contains(cellNameOf(cell.Value->getuno()))) {
                cells.push_back(cell.Value);
            }
        }
    }
    return cells;
}

cpo::uno::Reference<scriptinterop::XElement> tableParentOf(
    std::shared_ptr<Model> const & model, cpo::uno::Reference<css::text::XTextTable> const & table)
{
    rtl::Reference<BodyImpl> const body(new BodyImpl(
        model,
        cpo::uno::Reference<css::text::XTextDocument>(model->uno(), cpo::uno::UNO_QUERY_THROW)
            ->getText()));
    auto const & bodyContents = body->contents();
    if (std::find(bodyContents.begin(), bodyContents.end(), table) != bodyContents.end()) {
        return body;
    }
    cpo::uno::Reference<css::container::XIndexAccess> const tables(
        cpo::uno::Reference<css::text::XTextTablesSupplier>(
            model->uno(), cpo::uno::UNO_QUERY_THROW)->getTextTables(),
        cpo::uno::UNO_QUERY_THROW);
    auto const n = tables->getCount();
    for (sal_Int32 i = 0; i != n; ++i) {
        cpo::uno::Reference<css::text::XTextTable> const outer(
            tables->getByIndex(i), cpo::uno::UNO_QUERY_THROW);
        for (auto const & name: outer->getCellNames()) {
            auto const contents = cpo::uno::Reference<css::container::XEnumerationAccess>(
                outer->getCellByName(name), cpo::uno::UNO_QUERY_THROW)->createEnumeration();
            while (contents->hasMoreElements()) {
                if (cpo::uno::Reference<cpo::uno::XInterface>(
                        contents->nextElement(), cpo::uno::UNO_QUERY) == table)
                {
                    auto const cells = cellsNamed(model, outer, {name});
                    return cells.empty() ? nullptr : cells.front();
                }
            }
        }
    }
    return {};
}

cpo::uno::Reference<scriptinterop::XElement> containerOf(
    std::shared_ptr<Model> const & model, cpo::uno::Reference<css::text::XText> const & text)
{
    cpo::uno::Reference<css::text::XText> const bodyText(
        cpo::uno::Reference<css::text::XTextDocument>(model->uno(), cpo::uno::UNO_QUERY_THROW)
            ->getText());
    if (text == bodyText) {
        return new BodyImpl(model, bodyText);
    }
    cpo::uno::Reference<css::lang::XServiceInfo> const info(text, cpo::uno::UNO_QUERY);
    if (!(info.is() && info->supportsService(u"com.sun.star.text.CellProperties"_ustr))) {
        return {};
    }
    auto const table = tableOf(model, text);
    if (!table.is()) {
        return {};
    }
    auto const cells = cellsNamed(model, table, {cellNameOf(text)});
    return cells.empty() ? nullptr : cells.front();
}

cpo::uno::Reference<scriptinterop::XParagraph> paragraphOf(
    std::shared_ptr<Model> const & model,
    cpo::uno::Reference<css::text::XTextContent> const & paragraph)
{
    return new ParagraphImpl(
        model,
        containerOf(
            model,
            cpo::uno::Reference<css::text::XTextRange>(paragraph, cpo::uno::UNO_QUERY_THROW)
                ->getText()),
        paragraph);
}

// The position of an element in the document, as the indices of the element and of its ancestors
// among their siblings, from the outermost ancestor down, so that comparing two of these
// lexicographically compares the elements in document order:
std::vector<sal_Int32> pathOf(cpo::uno::Reference<scriptinterop::XElement> element) {
    // The text of a whole paragraph, as made with editAsText or getSurroundingText, stands for the
    // paragraph:
    if (auto const text = dynamic_cast<TextImpl *>(element.get()); text && !text->run()) {
        element = text->paragraph();
    }
    std::vector<sal_Int32> path;
    for (;;) {
        auto const parent = element->getParent();
        if (!parent.IsPresent) {
            break;
        }
        path.push_back(
            cpo::uno::Reference<scriptinterop::XContainerElement>(
                parent.Value, cpo::uno::UNO_QUERY_THROW)->getChildIndex(element));
        element = parent.Value;
    }
    std::reverse(path.begin(), path.end());
    return path;
}

// The cell range that the given table cells make up, if they are all the cells of the range from
// the first to the last of them (in the order of the rows and of the cells in each row), which is
// what Writer can select:
cpo::uno::Reference<css::table::XCellRange> selectableCells(
    std::shared_ptr<Model> const & model,
    std::vector<cpo::uno::Reference<scriptinterop::XElement>> const & cells)
{
    auto const table = tableOf(model, cpo::uno::Reference<css::text::XText>(
        cells.front()->getuno(), cpo::uno::UNO_QUERY_THROW));
    if (!table.is()) {
        throw cpo::uno::RuntimeException(u"build: the table cell is not part of a table"_ustr);
    }
    // A cell is in the same table when the table has that very cell under the cell's name:
    std::set<OUString> names;
    for (auto const & cell: cells) {
        auto const name = cellNameOf(cell->getuno());
        if (name.isEmpty()) {
            throw cpo::uno::RuntimeException(u"build: the table cell is not part of a table"_ustr);
        }
        if (table->getCellByName(name) != cell->getuno()) {
            return {};
        }
        names.insert(name);
    }
    auto const ordered = cellsNamed(model, table, names);
    if (ordered.empty()) {
        throw cpo::uno::RuntimeException(
            u"build: the table cells are not in the rows of their table"_ustr);
    }
    // Writer has no cell ranges of a table with split cells, whose cells it cannot select then:
    cpo::uno::Reference<css::table::XCellRange> range;
    try {
        range = cpo::uno::Reference<css::table::XCellRange>(
            table, cpo::uno::UNO_QUERY_THROW)->getCellRangeByName(
                cellNameOf(ordered.front()->getuno()) + ":"
                + cellNameOf(ordered.back()->getuno()));
    } catch (cpo::uno::RuntimeException const &) {}
    return range.is() && cellNamesOf(range) == names ? range : nullptr;
}

cpo::uno::Reference<scriptinterop::XRangeBuilder> RangeBuilderImpl::addElement(
    cpo::uno::Reference<scriptinterop::XElement> const & added)
{
    if (!added.is()) {
        throw cpo::uno::RuntimeException(u"addElement: the element must not be null"_ustr);
    }
    // The text of a table, row, cell or body stands for that element:
    cpo::uno::Reference<scriptinterop::XElement> element(added);
    if (auto const containerText = dynamic_cast<ContainerTextImpl *>(added.get())) {
        element = containerText->container();
    }
    switch (element->getType()) {
    case scriptinterop::ElementType_TABLE:
    case scriptinterop::ElementType_TABLE_ROW:
    case scriptinterop::ElementType_TABLE_CELL:
    case scriptinterop::ElementType_INLINE_IMAGE:
        added_.push_back({element, {}});
        return this;
    default:
        break;
    }
    cpo::uno::Reference<scriptinterop::XText> const text(element, cpo::uno::UNO_QUERY);
    auto const range = text.is()
        ? textRangeOf(text)
        : cpo::uno::Reference<css::text::XTextRange>(element->getuno(), cpo::uno::UNO_QUERY);
    if (!range.is()) {
        throw cpo::uno::RuntimeException(
            u"addElement: only text elements can be added to a range"_ustr);
    }
    // As in GAS, a child text stays a part of its paragraph also when it is all of it:
    added_.push_back({
        element, range,
        text.is() && dynamic_cast<TextImpl &>(*text).run().has_value()});
    return this;
}

// A range that a script built, as its range elements in document order, with what Writer selects
// for it (a text range, an inline image or a cell range, or null if none) as its UNO object:
class ListSelectionImpl: public cppu::WeakImplHelper<scriptinterop::XSelection> {
public:
    ListSelectionImpl(
        cpo::uno::Reference<cpo::uno::XInterface> const & uno,
        std::vector<cpo::uno::Reference<scriptinterop::XRangeElement>> && elements):
        uno_(uno), elements_(std::move(elements)) {}

    cpo::uno::Reference<cpo::uno::XInterface> getuno() override { return uno_; }

    cpo::uno::Sequence<cpo::uno::Reference<scriptinterop::XRangeElement>> getRangeElements()
        override
    { return cpo::uno::Sequence(elements_.data(), elements_.size()); }

    cpo::uno::Sequence<cpo::uno::Reference<scriptinterop::XRangeElement>> getSelectedElements()
        override
    { return getRangeElements(); }

    // The text that Writer selects for the range, if any, is the same as for a selection that the
    // user makes:
    OUString getText() override {
        if (cpo::uno::Reference<css::container::XIndexAccess> const ranges{
                uno_, cpo::uno::UNO_QUERY})
        {
            return textOfRanges(ranges);
        }
        OUStringBuffer buf;
        for (auto const & element: elements_) {
            if (element->getElement()->getType() == scriptinterop::ElementType_INLINE_IMAGE) {
                continue;
            }
            if (!buf.isEmpty()) {
                buf.append('\n');
            }
            cpo::uno::Reference<css::text::XTextRange> const range(
                element->getuno(), cpo::uno::UNO_QUERY);
            buf.append(
                range.is() && !isWhole(element) ? textOf(range) : element->getElement()->getText());
        }
        return buf.makeStringAndClear();
    }

    void replace(OUString const & newText) override {
        for (auto const & element: elements_) {
            switch (element->getElement()->getType()) {
            case scriptinterop::ElementType_TABLE:
            case scriptinterop::ElementType_TABLE_ROW:
            case scriptinterop::ElementType_INLINE_IMAGE:
                throw cpo::uno::RuntimeException(
                    "replace: the range contains a "
                    + elementTypeName(element->getElement()->getType())
                    + ", which has no text to replace");
            default:
                break;
            }
        }
        if (cpo::uno::Reference<css::container::XIndexAccess> const ranges{
                uno_, cpo::uno::UNO_QUERY})
        {
            replaceRanges(ranges, newText);
            return;
        }
        for (auto const & element: elements_) {
            cpo::uno::Reference<css::text::XTextRange>(element->getuno(), cpo::uno::UNO_QUERY_THROW)
                ->setString(newText);
        }
    }

private:
    static bool isWhole(cpo::uno::Reference<scriptinterop::XRangeElement> const & element) {
        return dynamic_cast<WholeElementRangeElementImpl *>(element.get()) != nullptr;
    }

    cpo::uno::Reference<cpo::uno::XInterface> uno_;
    std::vector<cpo::uno::Reference<scriptinterop::XRangeElement>> elements_;
};

cpo::uno::Reference<scriptinterop::XRangeBuilder> RangeBuilderImpl::addElementsBetween(
    cpo::uno::Reference<scriptinterop::XElement> const & startElement,
    cpo::uno::Reference<scriptinterop::XElement> const & endElementInclusive)
{
    if (!startElement.is() || !endElementInclusive.is()) {
        throw cpo::uno::RuntimeException(
            u"addElementsBetween: the elements must not be null"_ustr);
    }
    addBetween(
        startElement, pathOf(startElement), false, endElementInclusive,
        pathOf(endElementInclusive), false);
    return this;
}

cpo::uno::Reference<scriptinterop::XRangeBuilder> RangeBuilderImpl::addElementsBetweenOffsets(
    cpo::uno::Reference<scriptinterop::XText> const & startText, sal_Int32 startTextOffset,
    cpo::uno::Reference<scriptinterop::XText> const & endText, sal_Int32 endTextOffsetInclusive)
{
    if (!startText.is() || !endText.is()) {
        throw cpo::uno::RuntimeException(
            u"addElementsBetween: the text elements must not be null"_ustr);
    }
    if (startTextOffset < 0
        || startTextOffset >= std::max<sal_Int32>(startText->getText().getLength(), 1)
        || endTextOffsetInclusive < 0
        || endTextOffsetInclusive >= std::max<sal_Int32>(endText->getText().getLength(), 1))
    {
        throw cpo::uno::RuntimeException(
            "addElementsBetween: offset out of range, got startOffset="
            + OUString::number(startTextOffset) + ", endOffsetInclusive="
            + OUString::number(endTextOffsetInclusive));
    }
    // As in GAS, an offset into the text of a table, row, cell or body, or into a text made with
    // editAsText, is one into the child text of the paragraph there:
    struct Located {
        cpo::uno::Reference<scriptinterop::XElement> element;
        std::vector<sal_Int32> path;
        sal_Int32 offset;
        sal_Int32 length;
    };
    auto const locate = [this](cpo::uno::Reference<scriptinterop::XText> text, sal_Int32 offset)
        -> Located
    {
        if (auto const containerText = dynamic_cast<ContainerTextImpl *>(text.get())) {
            std::tie(text, offset) = containerText->at(offset);
        }
        if (auto const impl = dynamic_cast<TextImpl *>(text.get()); impl && !impl->run()) {
            auto const paragraph = paragraphOf(
                model_,
                cpo::uno::Reference<css::text::XTextContent>(
                    text->getuno(), cpo::uno::UNO_QUERY_THROW));
            auto const [child, childOffset] = textChildAt(paragraph, offset);
            if (!child.is()) {
                return {paragraph, pathOf(paragraph), 0, 0};
            }
            text = child;
            offset = childOffset;
        }
        return {text, pathOf(text), offset, text->getText().getLength()};
    };
    auto [startElement, start, startOffset, startLength] = locate(startText, startTextOffset);
    auto [endElement, end, endOffset, endLength] = locate(endText, endTextOffsetInclusive);
    // An offset at the start or at the end of its text is the start or the end of the whole text:
    if (startOffset != 0) {
        start.push_back(startOffset);
    }
    if (endOffset < endLength - 1) {
        end.push_back(endOffset);
    }
    addBetween(startElement, start, startOffset != 0, endElement, end, true);
    return this;
}

void RangeBuilderImpl::addBetween(
    cpo::uno::Reference<scriptinterop::XElement> const & startElement,
    std::vector<sal_Int32> const & start, bool startOffset,
    cpo::uno::Reference<scriptinterop::XElement> const & endElement,
    std::vector<sal_Int32> const & end, bool endText)
{
    auto const rootOf = [](cpo::uno::Reference<scriptinterop::XElement> element) {
        for (;;) {
            auto const parent = element->getParent();
            if (!parent.IsPresent) {
                return element;
            }
            element = parent.Value;
        }
    };
    auto const root = rootOf(startElement);
    if (rootOf(endElement)->getuno() != root->getuno()) {
        throw cpo::uno::RuntimeException(
            u"addElementsBetween: the elements are not in the same part of the document"_ustr);
    }
    // An end that contains the start (as a whole text contains an offset into it) is not before it:
    if (end < start
        && !(end.size() <= start.size() && std::equal(end.begin(), end.end(), start.begin())))
    {
        throw cpo::uno::RuntimeException(
            u"addElementsBetween: the end lies before the start"_ustr);
    }
    if (cover(root, {}, start, startOffset, end, endText)) {
        cpo::uno::Reference<scriptinterop::XContainerElement> const container(
            root, cpo::uno::UNO_QUERY_THROW);
        auto const n = container->getNumChildren();
        for (sal_Int32 i = 0; i != n; ++i) {
            addElement(container->getChild(i));
        }
    }
}

// Whether the element at the given path lies wholly between the start and the end (as GAS then adds
// it as a whole), else adding the parts of it that do:
bool RangeBuilderImpl::cover(
    cpo::uno::Reference<scriptinterop::XElement> const & element,
    std::vector<sal_Int32> const & path, std::vector<sal_Int32> const & start, bool startOffset,
    std::vector<sal_Int32> const & end, bool endText)
{
    auto const isPrefix = [](std::vector<sal_Int32> const & a, std::vector<sal_Int32> const & b) {
        return a.size() <= b.size() && std::equal(a.begin(), a.end(), b.begin());
    };
    bool startsInside;
    if (isPrefix(start, path)) {
        startsInside = true;
    } else if (isPrefix(path, start)) {
        // As in GAS, an element that holds the start is not whole:
        startsInside = false;
    } else if (path < start) {
        return false;
    } else {
        startsInside = true;
    }
    bool endsInside;
    if (isPrefix(end, path)) {
        endsInside = true;
    } else if (isPrefix(path, end)) {
        // As in GAS, an element other than a table that holds an end element as its last part is
        // whole, unlike one that holds an end text:
        endsInside = !endText && element->getType() != scriptinterop::ElementType_TABLE;
        auto node = element;
        for (std::size_t i = path.size(); endsInside && i != end.size(); ++i) {
            cpo::uno::Reference<scriptinterop::XContainerElement> const container(
                node, cpo::uno::UNO_QUERY_THROW);
            endsInside = end[i] == container->getNumChildren() - 1;
            node = container->getChild(end[i]);
        }
    } else if (end < path) {
        return false;
    } else {
        endsInside = true;
    }
    if (startsInside && endsInside) {
        return true;
    }
    switch (element->getType()) {
    case scriptinterop::ElementType_PARAGRAPH:
    case scriptinterop::ElementType_LIST_ITEM:
        {
            cpo::uno::Reference<scriptinterop::XContainerElement> const paragraph(
                element, cpo::uno::UNO_QUERY_THROW);
            auto const n = paragraph->getNumChildren();
            auto const holds = [&path, &isPrefix](std::vector<sal_Int32> const & p) {
                return p.size() > path.size() && isPrefix(path, p);
            };
            auto first = path;
            first.push_back(0);
            auto last = path;
            last.push_back(n - 1);
            // As in GAS, a paragraph that a start covers from its first child is whole, unless it
            // also holds an end text or an end before its last child:
            if ((!holds(start) || start == first) && (!holds(end) || (!endText && end == last))) {
                return true;
            }
            for (sal_Int32 i = 0; i != n; ++i) {
                auto childPath = path;
                childPath.push_back(i);
                auto const child = paragraph->getChild(i);
                auto const startsBefore = !(childPath < start);
                auto const endsAfter = isPrefix(end, childPath) || !(end < childPath);
                if (child->getType() != scriptinterop::ElementType_TEXT) {
                    if (startsBefore && endsAfter) {
                        addElement(child);
                    }
                    continue;
                }
                cpo::uno::Reference<scriptinterop::XText> const text(
                    child, cpo::uno::UNO_QUERY_THROW);
                sal_Int32 from = 0;
                if (startOffset && start.size() == childPath.size() + 1
                    && isPrefix(childPath, start))
                {
                    from = start.back();
                } else if (!startsBefore) {
                    continue;
                }
                sal_Int32 to = text->getText().getLength() - 1;
                if (endText && end.size() == childPath.size() + 1 && isPrefix(childPath, end)) {
                    to = end.back();
                } else if (!endsAfter) {
                    continue;
                }
                if (from <= to) {
                    addElementRange(text, from, to);
                }
            }
            return false;
        }
    default:
        break;
    }
    cpo::uno::Reference<scriptinterop::XContainerElement> const container(
        element, cpo::uno::UNO_QUERY);
    if (!container.is()) {
        return false;
    }
    auto const n = container->getNumChildren();
    for (sal_Int32 i = 0; i != n; ++i) {
        auto childPath = path;
        childPath.push_back(i);
        auto const child = container->getChild(i);
        if (cover(child, childPath, start, startOffset, end, endText)) {
            addElement(child);
        }
    }
    return false;
}

cpo::uno::Reference<scriptinterop::XSelection> RangeBuilderImpl::build() {
    if (added_.empty()) {
        throw cpo::uno::RuntimeException(
            u"build: add at least one element or element range first"_ustr);
    }
    // The range elements with their positions in the document:
    struct Piece {
        cpo::uno::Reference<scriptinterop::XRangeElement> element;
        std::vector<sal_Int32> position;
        bool text;
        // For a text, its paragraph, the child text (or -1 for all of the paragraph), and the
        // offsets of its first and last character there:
        cpo::uno::Reference<cpo::uno::XInterface> paragraph = {};
        sal_Int32 child = -1;
        sal_Int32 from = 0;
        sal_Int32 to = SAL_MAX_INT32;
        bool offsets = false;
    };
    std::vector<Piece> pieces;
    for (auto const & added: added_) {
        if (added.range.is()) {
            std::vector<cpo::uno::Reference<scriptinterop::XRangeElement>> elements;
            splitAtParagraphBoundaries(model_, added.range, elements, added.offsets);
            for (auto const & element: elements) {
                auto position = pathOf(element->getElement());
                if (element->getElement()->getType() == scriptinterop::ElementType_INLINE_IMAGE) {
                    pieces.push_back({element, std::move(position), false});
                } else if (element->isPartial()) {
                    auto const child = position.back();
                    position.push_back(element->getStartOffset());
                    pieces.push_back({
                        element, std::move(position), true,
                        element->getElement()->getParent().Value->getuno(), child,
                        element->getStartOffset(), element->getEndOffsetInclusive(),
                        added.offsets});
                } else {
                    pieces.push_back({
                        element, std::move(position), true, element->getElement()->getuno(), -1,
                        0, SAL_MAX_INT32, added.offsets});
                }
            }
        } else if (added.element->getType() == scriptinterop::ElementType_INLINE_IMAGE) {
            pieces.push_back({
                new ImageRangeElementImpl(
                    model_,
                    cpo::uno::Reference<css::text::XTextContent>(
                        added.element->getuno(), cpo::uno::UNO_QUERY_THROW)),
                pathOf(added.element), false});
        } else {
            pieces.push_back({
                new WholeElementRangeElementImpl(added.element), pathOf(added.element), false});
        }
    }
    // As in GAS, the range elements are in document order, each element only once, and an element
    // that lies in another element of the range that is a whole table, row or cell, a text that
    // lies in another text of the range, or an inline image in a whole paragraph of the range, is
    // left out:
    std::stable_sort(pieces.begin(), pieces.end(), [](Piece const & a, Piece const & b) {
        return std::tie(a.position, a.offsets) < std::tie(b.position, b.offsets);
    });
    std::vector<Piece> kept;
    for (auto & piece: pieces) {
        auto const contained = std::any_of(kept.begin(), kept.end(), [&piece](Piece const & k) {
            if (k.text && !piece.text) {
                return k.child == -1 && piece.position.size() > k.position.size()
                    && std::equal(k.position.begin(), k.position.end(), piece.position.begin());
            }
            if (k.text) {
                return k.paragraph == piece.paragraph && (k.child == -1 || k.child == piece.child)
                    && k.from <= piece.from && piece.to <= k.to;
            }
            return piece.position.size() >= k.position.size()
                && std::equal(k.position.begin(), k.position.end(), piece.position.begin())
                && (piece.position.size() > k.position.size() || !piece.text);
        });
        if (!contained) {
            kept.push_back(std::move(piece));
        }
    }
    std::vector<cpo::uno::Reference<scriptinterop::XRangeElement>> elements;
    for (auto const & piece: kept) {
        elements.push_back(piece.element);
    }
    // Writer can select text with whole tables and inline images that run on without gaps, a single
    // inline image, and the cells of a cell range (an added row or table being its cells):
    auto const isTable = [](Piece const & p) {
        return p.element->getElement()->getType() == scriptinterop::ElementType_TABLE;
    };
    auto const isImage = [](Piece const & p) {
        return p.element->getElement()->getType() == scriptinterop::ElementType_INLINE_IMAGE;
    };
    auto const isTablePart = [](Piece const & p) {
        auto const type = p.element->getElement()->getType();
        return type == scriptinterop::ElementType_TABLE
            || type == scriptinterop::ElementType_TABLE_ROW
            || type == scriptinterop::ElementType_TABLE_CELL;
    };
    cpo::uno::Reference<cpo::uno::XInterface> uno;
    if (std::any_of(kept.begin(), kept.end(), [](Piece const & p) { return p.text; })
        && std::all_of(kept.begin(), kept.end(), [&isTable, &isImage](Piece const & p) {
               return p.text || isTable(p) || isImage(p);
           }))
    {
        auto const boundary = [&isImage](Piece const & piece, bool start)
            -> cpo::uno::Reference<css::text::XTextRange>
        {
            if (piece.text) {
                cpo::uno::Reference<css::text::XTextRange> const range(
                    piece.element->getuno(), cpo::uno::UNO_QUERY_THROW);
                return start ? range->getStart() : range->getEnd();
            }
            // An inline image takes up the one step of its anchor:
            if (isImage(piece)) {
                auto const anchor = cpo::uno::Reference<css::text::XTextContent>(
                    piece.element->getuno(), cpo::uno::UNO_QUERY_THROW)->getAnchor();
                auto const cursor = anchor->getText()->createTextCursorByRange(anchor->getStart());
                if (!start) {
                    cursor->goRight(1, false);
                }
                return cursor->getStart();
            }
            auto const parent = piece.element->getElement()->getParent();
            if (!parent.IsPresent) {
                return {};
            }
            auto const contents = enumerateContents(cpo::uno::Reference<css::text::XText>(
                parent.Value->getuno(), cpo::uno::UNO_QUERY_THROW));
            auto const i = std::find(
                contents.begin(), contents.end(), piece.element->getElement()->getuno());
            if (i == contents.end() || (start ? i == contents.begin() : i + 1 == contents.end())) {
                return {};
            }
            cpo::uno::Reference<css::text::XTextRange> const neighbour(
                *(start ? i - 1 : i + 1), cpo::uno::UNO_QUERY);
            if (!neighbour.is()) {
                return {};
            }
            return start ? neighbour->getEnd() : neighbour->getStart();
        };
        auto const first = boundary(kept.front(), true);
        auto const last = boundary(kept.back(), false);
        auto const host = first.is() ? first->getText() : nullptr;
        if (host.is() && last.is() && last->getText() == host) {
            auto const cursor = host->createTextCursorByRange(first);
            cursor->gotoRange(last, true);
            std::vector<cpo::uno::Reference<scriptinterop::XRangeElement>> whole;
            splitAtParagraphBoundaries(
                model_, cursor, whole,
                std::any_of(kept.begin(), kept.end(), [](Piece const & p) { return p.offsets; }));
            // An empty paragraph at either end of the range, which Writer cannot tell from the
            // boundary of the text next to it, is left out as when splitting (unlike one that holds
            // only inline images):
            auto const isEmpty = [](Piece const & p) {
                if (!p.text) {
                    return false;
                }
                cpo::uno::Reference<css::text::XTextRange> const range(
                    p.element->getuno(), cpo::uno::UNO_QUERY_THROW);
                return cpo::uno::Reference<css::text::XTextRangeCompare>(
                    range->getText(), cpo::uno::UNO_QUERY_THROW)
                    ->compareRegionStarts(range->getStart(), range->getEnd()) == 0;
            };
            auto begin = elements.begin();
            auto end = elements.end();
            if (end - begin > 1 && isEmpty(kept.front())) {
                ++begin;
            }
            if (end - begin > 1 && isEmpty(kept.back())) {
                --end;
            }
            if (whole.size() == std::size_t(end - begin)
                && std::equal(
                    whole.begin(), whole.end(), begin,
                    [](auto const & a, auto const & b) {
                        return a->getElement()->getuno() == b->getElement()->getuno()
                            && a->isPartial() == b->isPartial()
                            && a->getStartOffset() == b->getStartOffset()
                            && a->getEndOffsetInclusive() == b->getEndOffsetInclusive();
                    }))
            {
                uno = static_cast<cppu::OWeakObject *>(new TextRangesIndex({cursor}));
            }
        }
        // Texts that Writer cannot select as one range are selected as one range for each text:
        if (!uno.is()
            && std::all_of(kept.begin(), kept.end(), [](Piece const & p) { return p.text; }))
        {
            std::vector<cpo::uno::Reference<css::text::XTextRange>> ranges;
            for (auto const & piece: kept) {
                ranges.emplace_back(piece.element->getuno(), cpo::uno::UNO_QUERY_THROW);
            }
            uno = static_cast<cppu::OWeakObject *>(new TextRangesIndex(std::move(ranges)));
        }
    } else if (kept.size() == 1
               && kept.front().element->getElement()->getType()
                   == scriptinterop::ElementType_INLINE_IMAGE)
    {
        uno = kept.front().element->getuno();
    } else if (std::all_of(kept.begin(), kept.end(), isTablePart)) {
        std::vector<cpo::uno::Reference<scriptinterop::XElement>> cells;
        for (auto const & piece: kept) {
            auto const element = piece.element->getElement();
            switch (element->getType()) {
            case scriptinterop::ElementType_TABLE:
                {
                    cpo::uno::Reference<scriptinterop::XTable> const table(
                        element, cpo::uno::UNO_QUERY_THROW);
                    auto const rows = table->getNumRows();
                    for (sal_Int32 i = 0; i != rows; ++i) {
                        auto const row = table->getRow(i);
                        auto const n = row.IsPresent ? row.Value->getNumCells() : 0;
                        for (sal_Int32 j = 0; j != n; ++j) {
                            cells.push_back(row.Value->getCell(j).Value);
                        }
                    }
                    break;
                }
            case scriptinterop::ElementType_TABLE_ROW:
                {
                    cpo::uno::Reference<scriptinterop::XTableRow> const row(
                        element, cpo::uno::UNO_QUERY_THROW);
                    auto const n = row->getNumCells();
                    for (sal_Int32 j = 0; j != n; ++j) {
                        cells.push_back(row->getCell(j).Value);
                    }
                    break;
                }
            default:
                cells.push_back(element);
                break;
            }
        }
        uno = selectableCells(model_, cells);
    }
    // As in GAS, a text added with offsets stays a part of its paragraph also when it covers all of
    // it:
    for (std::size_t i = 0; i != kept.size(); ++i) {
        if (kept[i].offsets && !elements[i]->isPartial()) {
            elements[i] = new RangeElementImpl(
                model_,
                cpo::uno::Reference<css::text::XTextRange>(
                    elements[i]->getuno(), cpo::uno::UNO_QUERY_THROW),
                true);
        }
    }
    return new ListSelectionImpl(uno, std::move(elements));
}

class DocumentImpl : public cppu::WeakImplHelper<scriptinterop::XDocument>
{
public:
    explicit DocumentImpl(cpo::uno::Reference<css::frame::XModel> const& model)
        : model_(sharedModel(model))
    {
    }

    cpo::uno::Reference<cpo::uno::XInterface> getuno() override { return model_->uno(); }

    // The range given to setSelection stays the selection, with its own range elements, for as long
    // as Writer still has what it selected for it and the document has not changed:
    css::beans::Optional<cpo::uno::Reference<scriptinterop::XSelection>> getSelection()
        override
    {
        if (selection_.is()) {
            if (selectionGeneration_ == model_->generation() && stillSelected()) {
                return {true, selection_};
            }
            selection_.clear();
            selected_.clear();
        }
        cpo::uno::Reference<css::text::XTextDocument> const doc(
            model_->uno(), cpo::uno::UNO_QUERY_THROW);
        cpo::uno::Reference<css::view::XSelectionSupplier> const sup(
            doc->getCurrentController(), cpo::uno::UNO_QUERY_THROW);
        cpo::uno::Reference<css::lang::XServiceInfo> const info{
            sup->getSelection(), cpo::uno::UNO_QUERY};
        // A positioned image is not in the text, so selecting one selects no text of the document:
        if (info.is() && info->supportsService(u"com.sun.star.text.TextGraphicObject"_ustr)
            && cpo::uno::Reference<css::beans::XPropertySet>(info, cpo::uno::UNO_QUERY_THROW)
                ->getPropertyValue(u"AnchorType"_ustr)
                != cpo::uno::Any(css::text::TextContentAnchorType_AS_CHARACTER)) {
            return {false, {}};
        }
        // A selected graphic object is the object itself rather than a collection of ranges:
        if (info.is() && info->supportsService(u"com.sun.star.text.TextGraphicObject"_ustr)) {
            return {
                true,
                cpo::uno::Reference<scriptinterop::XSelection>(new ImageSelectionImpl(model_,
                    cpo::uno::Reference<css::text::XTextContent>(info, cpo::uno::UNO_QUERY_THROW)))};
        }
        // Selected table cells are a collection of whole cells of the table that the table cursor
        // is in:
        if (info.is() && info->supportsService(u"com.sun.star.text.TextTableCursor"_ustr)) {
            cpo::uno::Reference<css::text::XTextTable> table;
            cpo::uno::Reference<css::beans::XPropertySet>(info, cpo::uno::UNO_QUERY_THROW)
                ->getPropertyValue(u"TextTable"_ustr) >>= table;
            if (!table.is()) {
                return {false, {}};
            }
            // The range name of a single selected cell is just the name of that cell:
            auto rangeName = cpo::uno::Reference<css::text::XTextTableCursor>(
                info, cpo::uno::UNO_QUERY_THROW)->getRangeName();
            if (rangeName.isEmpty()) {
                return {false, {}};
            }
            if (rangeName.indexOf(':') == -1) {
                rangeName += ":" + rangeName;
            }
            // Writer has no cell ranges of a table with split cells, whose selected cells give no
            // selection then:
            cpo::uno::Reference<css::table::XCellRange> range;
            try {
                range = cpo::uno::Reference<css::table::XCellRange>(
                    table, cpo::uno::UNO_QUERY_THROW)->getCellRangeByName(rangeName);
            } catch (cpo::uno::RuntimeException const &) {}
            if (!range.is()) {
                return {false, {}};
            }
            auto cells = cellsNamed(model_, table, cellNamesOf(range));
            if (cells.empty()) {
                return {false, {}};
            }
            return {
                true,
                cpo::uno::Reference<scriptinterop::XSelection>(
                    new CellSelectionImpl(range, std::move(cells)))};
        }
        // Selected shapes are a collection of shapes, and the text in them is not selected text of
        // the document:
        if (!(info.is() && info->supportsService(u"com.sun.star.text.TextRanges"_ustr))) {
            return {false, {}};
        }
        cpo::uno::Reference<css::container::XIndexAccess> const ranges(
            info, cpo::uno::UNO_QUERY_THROW);
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
        return {
            true,
            cpo::uno::Reference<scriptinterop::XSelection>(new SelectionImpl(model_, ranges))};
    }

    cpo::uno::Reference<scriptinterop::XBody> getBody() override
    {
        cpo::uno::Reference<css::text::XTextDocument> const doc(
            model_->uno(), cpo::uno::UNO_QUERY_THROW);
        return new BodyImpl(model_, doc->getText());
    }

    // There is no cursor while there is a selection, or while the view has no text cursor.  The
    // position given to setCursor stays the cursor, with its own element and offset, for as long as
    // the view cursor is still at that position:
    css::beans::Optional<cpo::uno::Reference<scriptinterop::XCursor>> getCursor() override
    {
        if (getSelection().IsPresent || !hasTextCursor()) {
            return {false, {}};
        }
        if (cursorPosition_.is()) {
            auto const view = viewCursorOf(model_);
            cpo::uno::Reference<css::text::XTextRange> const position(
                cursorPosition_->getuno(), cpo::uno::UNO_QUERY_THROW);
            cpo::uno::Reference<css::text::XTextRangeCompare> const cmp(
                view->getText(), cpo::uno::UNO_QUERY_THROW);
            try {
                if (cmp->compareRegionStarts(view->getStart(), position->getStart()) == 0) {
                    return {true, cursorPosition_};
                }
            } catch (css::lang::IllegalArgumentException const &) {
                // The position is in another text than the view cursor.
            }
            cursorPosition_.clear();
        }
        return {true, cpo::uno::Reference<scriptinterop::XCursor>(new CursorImpl(model_))};
    }

    cpo::uno::Reference<scriptinterop::XCursor> newPosition(
        cpo::uno::Reference<scriptinterop::XElement> const & element, sal_Int32 offset) override
    {
        if (!element.is()) {
            throw cpo::uno::RuntimeException(u"newPosition: the element must not be null"_ustr);
        }
        if (offset < 0) {
            throw cpo::uno::RuntimeException(
                "newPosition: expected a non-negative offset, got " + OUString::number(offset));
        }
        // As in GAS, an offset into the text of a paragraph (as made with editAsText) is one into
        // the child text there:
        if (auto const impl = dynamic_cast<TextImpl *>(element.get()); impl && !impl->run()) {
            auto const paragraph = paragraphOf(
                model_,
                cpo::uno::Reference<css::text::XTextContent>(
                    element->getuno(), cpo::uno::UNO_QUERY_THROW));
            auto const [child, childOffset] = textChildAt(paragraph, offset);
            return child.is() ? newPosition(child, childOffset) : newPosition(paragraph, offset);
        }
        if (element->getType() == scriptinterop::ElementType_TEXT) {
            auto const text = textRangeOf(
                cpo::uno::Reference<scriptinterop::XText>(element, cpo::uno::UNO_QUERY_THROW));
            auto const length = textOf(text).getLength();
            if (offset > length) {
                throw cpo::uno::RuntimeException(
                    "newPosition: offset " + OUString::number(offset)
                    + " is past the end of the text");
            }
            auto const cursor = text->getText()->createTextCursorByRange(text->getStart());
            goRight(cursor, offset, false);
            // As in GAS, the end of a child text is the position after it in its paragraph:
            if (auto const parent = element->getParent(); offset == length && parent.IsPresent) {
                return new CursorImpl(
                    model_, cursor->getStart(), parent.Value,
                    cpo::uno::Reference<scriptinterop::XContainerElement>(
                        parent.Value, cpo::uno::UNO_QUERY_THROW)->getChildIndex(element) + 1);
            }
            return new CursorImpl(model_, cursor->getStart(), element, offset);
        }
        cpo::uno::Reference<scriptinterop::XContainerElement> const container(
            element, cpo::uno::UNO_QUERY);
        if (!container.is()) {
            throw cpo::uno::RuntimeException(
                u"newPosition: the element is neither text nor has children"_ustr);
        }
        auto const n = container->getNumChildren();
        if (offset > n) {
            throw cpo::uno::RuntimeException(
                "newPosition: child index " + OUString::number(offset) + " is past the "
                + OUString::number(n) + " children");
        }
        if (offset == n) {
            cpo::uno::Reference<css::text::XTextRange> const whole(
                element->getuno(), cpo::uno::UNO_QUERY_THROW);
            return new CursorImpl(model_, whole->getEnd(), element, offset);
        }
        auto const childElement = container->getChild(offset);
        if (childElement->getType() == scriptinterop::ElementType_TEXT) {
            return new CursorImpl(
                model_,
                textRangeOf(cpo::uno::Reference<scriptinterop::XText>(
                    childElement, cpo::uno::UNO_QUERY_THROW))->getStart(),
                element, offset);
        }
        auto const child = childElement->getuno();
        // An inline image or a paragraph starts where its anchor does:
        if (cpo::uno::Reference<css::text::XTextContent> const content{child, cpo::uno::UNO_QUERY})
        {
            return new CursorImpl(model_, content->getAnchor()->getStart(), element, offset);
        }
        cpo::uno::Reference<css::text::XTextRange> const range(child, cpo::uno::UNO_QUERY_THROW);
        return new CursorImpl(model_, range->getStart(), element, offset);
    }

    cpo::uno::Reference<scriptinterop::XRangeBuilder> newRange() override {
        return new RangeBuilderImpl(model_);
    }

    cpo::uno::Reference<scriptinterop::XDocument> setCursor(
        cpo::uno::Reference<scriptinterop::XCursor> const & position) override
    {
        if (!position.is()) {
            throw cpo::uno::RuntimeException(u"setCursor: the position must not be null"_ustr);
        }
        cpo::uno::Reference<css::text::XTextRange> const range(
            position->getuno(), cpo::uno::UNO_QUERY_THROW);
        // Selecting the collapsed range also ends a selection of a graphic or other object, which
        // the view cursor cannot move out of on its own:
        cpo::uno::Reference<css::view::XSelectionSupplier> const sup(
            model_->uno()->getCurrentController(), cpo::uno::UNO_QUERY_THROW);
        if (!sup->select(cpo::uno::Any(range->getStart()))) {
            throw cpo::uno::RuntimeException(
                u"setCursor: the position is not somewhere the cursor can be"_ustr);
        }
        cursorPosition_ = position;
        selection_.clear();
        selected_.clear();
        return this;
    }

    cpo::uno::Reference<scriptinterop::XDocument> setSelection(
        cpo::uno::Reference<scriptinterop::XSelection> const & selection) override
    {
        if (!selection.is()) {
            throw cpo::uno::RuntimeException(u"setSelection: the selection must not be null"_ustr);
        }
        if (!selection->getuno().is()) {
            throw cpo::uno::RuntimeException(
                u"setSelection: Writer cannot select exactly the elements of this range"_ustr);
        }
        cpo::uno::Reference<css::view::XSelectionSupplier> const sup(
            model_->uno()->getCurrentController(), cpo::uno::UNO_QUERY_THROW);
        cpo::uno::Any selected(selection->getuno());
        cpo::uno::Reference<css::container::XIndexAccess> const idx(
            selection->getuno(), cpo::uno::UNO_QUERY);
        if (idx.is() && idx->getCount() != 0) {
            cpo::uno::Sequence<cpo::uno::Reference<css::text::XTextRange>> ranges(idx->getCount());
            auto const p = ranges.getArray();
            for (sal_Int32 i = 0; i != ranges.getLength(); ++i) {
                idx->getByIndex(i) >>= p[i];
            }
            if (std::all_of(ranges.begin(), ranges.end(), [](auto const & r) { return r.is(); })) {
                selected <<= ranges;
            }
        }
        if (!sup->select(selected)) {
            throw cpo::uno::RuntimeException(u"setSelection: Writer did not select the range"_ustr);
        }
        selection_ = selection;
        selected_.set(sup->getSelection(), cpo::uno::UNO_QUERY);
        selectionGeneration_ = model_->generation();
        return this;
    }

    cpo::uno::Sequence<cpo::uno::Reference<scriptinterop::XFootnote>> getFootnotes() override {
        std::vector<cpo::uno::Reference<scriptinterop::XFootnote>> v;
        cpo::uno::Reference<css::text::XFootnotesSupplier> const sup(
            model_->uno(), cpo::uno::UNO_QUERY_THROW);
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
            v.emplace_back(new FootnoteImpl(model_, footnote));
        }
        return cpo::uno::Sequence(v.data(), v.size());
    }

    css::beans::Optional<OUString> getLanguage() override {
        css::lang::Locale locale;
        cpo::uno::Reference<css::beans::XPropertySet>(
            cpo::uno::Reference<css::lang::XMultiServiceFactory>(
                model_->uno(), cpo::uno::UNO_QUERY_THROW)
                ->createInstance(u"com.sun.star.text.Defaults"_ustr),
            cpo::uno::UNO_QUERY_THROW)->getPropertyValue(u"CharLocale"_ustr) >>= locale;
        auto const code = gasLanguageCode(locale);
        if (code.isEmpty()) {
            return {false, {}};
        }
        return {true, code};
    }

    void insertImage(cpo::uno::Sequence<sal_Int8> const& data,
                              scriptinterop::ImageOptions const& opts) override
    {
        cpo::uno::Reference<css::text::XTextDocument> const doc(
            model_->uno(), cpo::uno::UNO_QUERY_THROW);
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
    // The view has a text cursor while text is selected, and none while table cells, a shape, a
    // frame or some other object are selected:
    bool hasTextCursor() {
        cpo::uno::Reference<css::view::XSelectionSupplier> const sup(
            model_->uno()->getCurrentController(), cpo::uno::UNO_QUERY_THROW);
        cpo::uno::Reference<css::lang::XServiceInfo> const info{
            sup->getSelection(), cpo::uno::UNO_QUERY};
        return info.is() && info->supportsService(u"com.sun.star.text.TextRanges"_ustr);
    }

    // Whether Writer's selection is still the one that it made for the range given to setSelection,
    // the same text ranges or the same cells of the same table:
    bool stillSelected() {
        cpo::uno::Reference<css::view::XSelectionSupplier> const sup(
            model_->uno()->getCurrentController(), cpo::uno::UNO_QUERY_THROW);
        cpo::uno::Reference<cpo::uno::XInterface> const now(
            sup->getSelection(), cpo::uno::UNO_QUERY);
        if (now == selected_) {
            return now.is();
        }
        if (cpo::uno::Reference<css::container::XIndexAccess> const
                ranges{now, cpo::uno::UNO_QUERY}, before{selected_, cpo::uno::UNO_QUERY};
            ranges.is() && before.is())
        {
            auto const n = ranges->getCount();
            if (n != before->getCount()) {
                return false;
            }
            for (sal_Int32 i = 0; i != n; ++i) {
                cpo::uno::Reference<css::text::XTextRange> const a(
                    ranges->getByIndex(i), cpo::uno::UNO_QUERY);
                cpo::uno::Reference<css::text::XTextRange> const b(
                    before->getByIndex(i), cpo::uno::UNO_QUERY);
                if (!a.is() || !b.is()) {
                    return false;
                }
                try {
                    cpo::uno::Reference<css::text::XTextRangeCompare> const cmp(
                        a->getText(), cpo::uno::UNO_QUERY_THROW);
                    if (cmp->compareRegionStarts(a, b) != 0 || cmp->compareRegionEnds(a, b) != 0) {
                        return false;
                    }
                } catch (css::lang::IllegalArgumentException const &) {
                    return false;
                }
            }
            return true;
        }
        if (cpo::uno::Reference<css::text::XTextTableCursor> const cells{now, cpo::uno::UNO_QUERY},
            before{selected_, cpo::uno::UNO_QUERY};
            cells.is() && before.is())
        {
            return cells->getRangeName() == before->getRangeName()
                && cpo::uno::Reference<css::beans::XPropertySet>(cells, cpo::uno::UNO_QUERY_THROW)
                        ->getPropertyValue(u"TextTable"_ustr)
                    == cpo::uno::Reference<css::beans::XPropertySet>(
                        before, cpo::uno::UNO_QUERY_THROW)->getPropertyValue(u"TextTable"_ustr);
        }
        return false;
    }

    std::shared_ptr<Model> model_;
    cpo::uno::Reference<scriptinterop::XCursor> cursorPosition_;
    // The range given to setSelection, what Writer selected for it, and the generation of the
    // document then:
    cpo::uno::Reference<scriptinterop::XSelection> selection_;
    cpo::uno::Reference<cpo::uno::XInterface> selected_;
    std::uintmax_t selectionGeneration_ = 0;
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
