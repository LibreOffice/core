/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <SecLabelApply.hxx>

#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/text/ControlCharacter.hpp>
#include <com/sun/star/text/XBookmarksSupplier.hpp>
#include <com/sun/star/text/XParagraphCursor.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XTextContent.hpp>
#include <com/sun/star/text/XTextCursor.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>

#include <vector>

using namespace css;
using namespace ::cpo;

namespace sw::seclabel
{
namespace
{
// The character style tagging our header/footer marking runs, so they can be found and
// removed without touching the user's own content. A style (unlike a bookmark) survives
// the DOCX round-trip inside a header/footer, so removal works after a reopen too. The
// body markings identify themselves by bookmark instead (those anchor in the body text,
// where bookmarks round-trip cleanly).
constexpr OUString MARKING_STYLE = u"Security Label"_ustr;
constexpr OUString BOOKMARK_DOC_START = u"__CplSecLabelDocStart"_ustr;
constexpr OUString BOOKMARK_DOC_END = u"__CplSecLabelDocEnd"_ustr;

bool getBool(const uno::Reference<beans::XPropertySet>& xPageStyle, const OUString& rProp)
{
    bool bValue = true; // header/footer sharing defaults to on
    xPageStyle->getPropertyValue(rProp) >>= bValue;
    return bValue;
}

// The document's page styles (XNameAccess), or an empty reference.
uno::Reference<container::XNameAccess> getPageStyles(const uno::Reference<frame::XModel>& xModel)
{
    uno::Reference<style::XStyleFamiliesSupplier> xSupplier(xModel, uno::UNO_QUERY);
    if (!xSupplier.is())
        return {};
    uno::Reference<container::XNameAccess> xPageStyles;
    xSupplier->getStyleFamilies()->getByName(u"PageStyles"_ustr) >>= xPageStyles;
    return xPageStyles;
}

// Ensure the marking character style exists in the document (created empty: it is only a
// marker, the marking run carries its own direct formatting).
void ensureMarkingStyle(const uno::Reference<frame::XModel>& xModel)
{
    uno::Reference<style::XStyleFamiliesSupplier> xSupplier(xModel, uno::UNO_QUERY);
    if (!xSupplier.is())
        return;
    uno::Reference<container::XNameContainer> xCharStyles;
    xSupplier->getStyleFamilies()->getByName(u"CharacterStyles"_ustr) >>= xCharStyles;
    if (!xCharStyles.is() || xCharStyles->hasByName(MARKING_STYLE))
        return;
    uno::Reference<lang::XMultiServiceFactory> xFactory(xModel, uno::UNO_QUERY);
    if (!xFactory.is())
        return;
    uno::Reference<style::XStyle> xStyle(
        xFactory->createInstance(u"com.sun.star.style.CharacterStyle"_ustr), uno::UNO_QUERY);
    if (xStyle.is())
        xCharStyles->insertByName(MARKING_STYLE, cpo::uno::Any(xStyle));
}

// Format a cursor selection as a marking run: our marker char style plus direct bold and
// colour (the direct formatting overrides the empty style). Centring is paragraph-level, so
// it is applied only for whole-paragraph markings (bCenterParagraph); a portion marking
// shares its paragraph with the user's own text and must not re-align it.
void formatMarkingSelection(const uno::Reference<text::XTextCursor>& xCursor, sal_Int32 nColor,
                            bool bCenterParagraph)
{
    uno::Reference<beans::XPropertySet> xProps(xCursor, uno::UNO_QUERY);
    if (!xProps.is())
        return;
    xProps->setPropertyValue(u"CharStyleName"_ustr, cpo::uno::Any(MARKING_STYLE));
    xProps->setPropertyValue(u"CharWeight"_ustr, cpo::uno::Any(awt::FontWeight::BOLD));
    xProps->setPropertyValue(u"CharColor"_ustr, cpo::uno::Any(nColor));
    if (bCenterParagraph)
        xProps->setPropertyValue(u"ParaAdjust"_ustr, cpo::uno::Any(style::ParagraphAdjust_CENTER));
}

// Whether a header/footer text holds no content of the user's own. getString() alone is not
// enough: a footer whose only content is a page-number field reports an empty string yet is
// not empty. Treat any second paragraph, any non-empty run, or any non-text portion (field,
// frame, footnote, ...) as content.
bool isTextEffectivelyEmpty(const uno::Reference<text::XText>& xText)
{
    uno::Reference<container::XEnumerationAccess> xAccess(xText, uno::UNO_QUERY);
    if (!xAccess.is())
        return xText->getString().isEmpty();
    uno::Reference<container::XEnumeration> xParas = xAccess->createEnumeration();
    int nParas = 0;
    while (xParas->hasMoreElements())
    {
        uno::Reference<container::XEnumerationAccess> xPortions(xParas->nextElement(),
                                                               uno::UNO_QUERY);
        if (++nParas > 1)
            return false; // more than one paragraph
        if (!xPortions.is())
            return false; // e.g. a table
        uno::Reference<container::XEnumeration> xPortEnum = xPortions->createEnumeration();
        while (xPortEnum->hasMoreElements())
        {
            uno::Reference<beans::XPropertySet> xPortion(xPortEnum->nextElement(), uno::UNO_QUERY);
            uno::Reference<text::XTextRange> xRange(xPortion, uno::UNO_QUERY);
            if (xRange.is() && !xRange->getString().isEmpty())
                return false; // real text
            OUString sType;
            if (xPortion.is() && (xPortion->getPropertyValue(u"TextPortionType"_ustr) >>= sType)
                && sType != u"Text"_ustr)
                return false; // a field, frame, footnote, ...
        }
    }
    return true;
}

// Insert rMarking as its own formatted paragraph in xText -- at the start (bAtStart) or
// end -- coexisting with any existing content. When xText is empty the marking becomes
// its sole paragraph (no extra blank line). If rBookmark is non-empty the paragraph is
// also bookmarked (body markings, for precise in-session removal).
void insertMarkingParagraph(const uno::Reference<frame::XModel>& xModel,
                            const uno::Reference<text::XText>& xText, const OUString& rMarking,
                            sal_Int32 nColor, bool bAtStart, const OUString& rBookmark)
{
    if (!xText.is())
        return;

    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    if (isTextEffectivelyEmpty(xText))
    {
        xCursor->gotoStart(false);
        xText->insertString(xCursor, rMarking, false);
    }
    else if (bAtStart)
    {
        xCursor->gotoStart(false);
        xText->insertControlCharacter(xCursor, text::ControlCharacter::PARAGRAPH_BREAK, false);
        xCursor->gotoStart(false);
        xText->insertString(xCursor, rMarking, false);
    }
    else
    {
        xCursor->gotoEnd(false);
        xText->insertControlCharacter(xCursor, text::ControlCharacter::PARAGRAPH_BREAK, false);
        xText->insertString(xCursor, rMarking, false);
    }

    // The cursor sits at the end of the marking: select the paragraph to format (+ bookmark).
    uno::Reference<text::XParagraphCursor> xPara(xCursor, uno::UNO_QUERY);
    if (xPara.is())
        xPara->gotoStartOfParagraph(true);
    formatMarkingSelection(xCursor, nColor, /*bCenterParagraph*/ true);

    if (rBookmark.isEmpty())
        return;
    uno::Reference<lang::XMultiServiceFactory> xFactory(xModel, uno::UNO_QUERY);
    if (!xFactory.is())
        return;
    uno::Reference<text::XTextContent> xMark(
        xFactory->createInstance(u"com.sun.star.text.Bookmark"_ustr), uno::UNO_QUERY);
    uno::Reference<container::XNamed> xNamed(xMark, uno::UNO_QUERY);
    if (!xMark.is() || !xNamed.is())
        return;
    xNamed->setName(rBookmark);
    xText->insertTextContent(xCursor, xMark, true);
}

// Whether a paragraph is wholly one of our markings: every non-empty run carries the marker
// style, and at least one does. Empty runs are ignored -- a paragraph mark can retain the
// style after a merge (deleting a marking paragraph leaves its styled, empty mark behind), and
// counting that would wrongly flag the user's own text as a marking. A paragraph that mixes
// the user's plain text with a styled run is not a whole-paragraph marking (portion markings
// are removed separately). The empty-paragraph short-circuit also keeps the removal sweep from
// looping on the empty paragraph left when a header's sole marking is cleared.
bool isMarkingParagraph(const uno::Reference<text::XTextContent>& xPara)
{
    uno::Reference<text::XTextRange> xRange(xPara, uno::UNO_QUERY);
    if (!xRange.is() || xRange->getString().isEmpty())
        return false;
    uno::Reference<container::XEnumerationAccess> xPortions(xPara, uno::UNO_QUERY);
    if (!xPortions.is())
        return false; // e.g. a table, not a paragraph
    uno::Reference<container::XEnumeration> xEnum = xPortions->createEnumeration();
    bool bHasMarking = false;
    while (xEnum->hasMoreElements())
    {
        uno::Reference<beans::XPropertySet> xPortion(xEnum->nextElement(), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xPortionRange(xPortion, uno::UNO_QUERY);
        if (!xPortionRange.is() || xPortionRange->getString().isEmpty())
            continue; // ignore empty runs (e.g. a styled paragraph mark left by a merge)
        OUString sStyle;
        if (xPortion.is() && (xPortion->getPropertyValue(u"CharStyleName"_ustr) >>= sStyle)
            && sStyle == MARKING_STYLE)
            bHasMarking = true;
        else
            return false; // a non-empty run of the user's own text: not a whole-paragraph marking
    }
    return bHasMarking;
}

// Delete a whole marking paragraph swept from a header/footer text (xParaRange is the enumerated
// paragraph). Clear its content forward -- reliable even when it is the last paragraph, where
// selecting backward over it is not -- then absorb one break so no blank line remains. No
// bookmarks are involved here, so the transient empty paragraph is harmless.
void deleteSweptMarkingParagraph(const uno::Reference<text::XText>& xText,
                                 const uno::Reference<text::XTextRange>& xParaRange, bool bAtStart)
{
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursorByRange(xParaRange);
    uno::Reference<text::XParagraphCursor> xPara(xCursor, uno::UNO_QUERY);
    if (!xPara.is())
        return;
    xPara->gotoStartOfParagraph(false);
    xPara->gotoEndOfParagraph(true);
    xText->insertString(xCursor, OUString(), true); // the paragraph is now empty

    // Remove the empty paragraph by absorbing one break (no-op if it is the sole paragraph).
    if (bAtStart)
        xCursor->goRight(1, true); // trailing break
    else
        xCursor->goLeft(1, true); // leading break
    xText->insertString(xCursor, OUString(), true);
}

// Remove every marking paragraph from a header/footer text. Re-enumerates after each
// deletion (removal invalidates the enumeration); the texts are tiny.
void clearMarkingFromText(const uno::Reference<text::XText>& xText, bool bAtStart)
{
    uno::Reference<container::XEnumerationAccess> xAccess(xText, uno::UNO_QUERY);
    if (!xAccess.is())
        return;
    for (;;)
    {
        uno::Reference<container::XEnumeration> xEnum = xAccess->createEnumeration();
        uno::Reference<text::XTextRange> xFound;
        while (xEnum->hasMoreElements())
        {
            uno::Reference<text::XTextContent> xParaContent(xEnum->nextElement(), uno::UNO_QUERY);
            if (isMarkingParagraph(xParaContent))
            {
                xFound.set(xParaContent, uno::UNO_QUERY);
                break;
            }
        }
        if (!xFound.is())
            return;
        deleteSweptMarkingParagraph(xText, xFound, bAtStart);
    }
}

// Header and footer text-property sets (shared/right, left, first) for the variant sweep.
struct Area
{
    OUString aIsOn;
    OUString aIsShared;
    OUString aText;
    OUString aTextLeft;
    OUString aTextFirst;
    bool bAtStart; // header marks at the top, footer at the bottom
};
const Area HEADER{ u"HeaderIsOn"_ustr,     u"HeaderIsShared"_ustr, u"HeaderText"_ustr,
                   u"HeaderTextLeft"_ustr, u"HeaderTextFirst"_ustr, true };
const Area FOOTER{ u"FooterIsOn"_ustr,     u"FooterIsShared"_ustr, u"FooterText"_ustr,
                   u"FooterTextLeft"_ustr, u"FooterTextFirst"_ustr, false };

uno::Reference<text::XText> areaText(const uno::Reference<beans::XPropertySet>& xPageStyle,
                                     const OUString& rProp)
{
    return uno::Reference<text::XText>(xPageStyle->getPropertyValue(rProp), uno::UNO_QUERY);
}

// Clear our markings from one area (all active variants) of a page style.
void clearArea(const uno::Reference<beans::XPropertySet>& xPageStyle, const Area& rArea)
{
    clearMarkingFromText(areaText(xPageStyle, rArea.aText), rArea.bAtStart);
    clearMarkingFromText(areaText(xPageStyle, rArea.aTextLeft), rArea.bAtStart);
    clearMarkingFromText(areaText(xPageStyle, rArea.aTextFirst), rArea.bAtStart);
}

// Enable an area and mark every variant the page style actually shows: shared/right
// always, left when left and right pages differ, first when the first page differs.
void markArea(const uno::Reference<frame::XModel>& xModel,
              const uno::Reference<beans::XPropertySet>& xPageStyle, const Area& rArea,
              const OUString& rMarking, sal_Int32 nColor)
{
    xPageStyle->setPropertyValue(rArea.aIsOn, cpo::uno::Any(true));
    insertMarkingParagraph(xModel, areaText(xPageStyle, rArea.aText), rMarking, nColor,
                           rArea.bAtStart, OUString());
    if (!getBool(xPageStyle, rArea.aIsShared))
        insertMarkingParagraph(xModel, areaText(xPageStyle, rArea.aTextLeft), rMarking, nColor,
                               rArea.bAtStart, OUString());
    if (!getBool(xPageStyle, u"FirstIsShared"_ustr))
        insertMarkingParagraph(xModel, areaText(xPageStyle, rArea.aTextFirst), rMarking, nColor,
                               rArea.bAtStart, OUString());
}

// Clear our header/footer markings from every page style (in use or not), so a re-label
// or remove leaves none behind.
void clearAllPageStyles(const uno::Reference<frame::XModel>& xModel)
{
    uno::Reference<container::XNameAccess> xPageStyles = getPageStyles(xModel);
    if (!xPageStyles.is())
        return;
    for (const OUString& rName : xPageStyles->getElementNames())
    {
        uno::Reference<beans::XPropertySet> xPageStyle(xPageStyles->getByName(rName), uno::UNO_QUERY);
        if (!xPageStyle.is())
            continue;
        clearArea(xPageStyle, HEADER);
        clearArea(xPageStyle, FOOTER);
    }
}
}

void applyMarking(const uno::Reference<frame::XModel>& xModel, const OUString& rMarking,
                  sal_Int32 nColor, std::u16string_view rPageStyleName)
{
    uno::Reference<container::XNameAccess> xPageStyles = getPageStyles(xModel);
    if (!xPageStyles.is())
        return;

    ensureMarkingStyle(xModel);
    // Replace any prior marking everywhere first, so a re-label never stacks or strands a
    // banner; then mark every page style in use (plus the current one) so the marking is
    // on every page whichever style a page uses.
    clearAllPageStyles(xModel);

    for (const OUString& rName : xPageStyles->getElementNames())
    {
        uno::Reference<style::XStyle> xStyle(xPageStyles->getByName(rName), uno::UNO_QUERY);
        const bool bMark = (xStyle.is() && xStyle->isInUse()) || rName == rPageStyleName;
        if (!bMark)
            continue;
        uno::Reference<beans::XPropertySet> xPageStyle(xStyle, uno::UNO_QUERY);
        if (!xPageStyle.is())
            continue;
        markArea(xModel, xPageStyle, HEADER, rMarking, nColor);
        markArea(xModel, xPageStyle, FOOTER, rMarking, nColor);
    }
}

void removeLabel(const uno::Reference<frame::XModel>& xModel, std::u16string_view /*rPageStyleName*/)
{
    removeBodyMarkings(xModel);
    clearAllPageStyles(xModel);
}

// --- Body (cover/end-page) markings: bookmarked, they anchor in the body text where
// bookmarks round-trip cleanly. ---

namespace
{
// Remove a previously inserted body marking (the whole paragraph) by its bookmark.
void removeBookmarkedMarking(const uno::Reference<frame::XModel>& xModel, const OUString& rName,
                             bool bAtStart)
{
    uno::Reference<text::XBookmarksSupplier> xSupplier(xModel, uno::UNO_QUERY);
    if (!xSupplier.is())
        return;
    uno::Reference<container::XNameAccess> xMarks = xSupplier->getBookmarks();
    if (!xMarks.is() || !xMarks->hasByName(rName))
        return;
    uno::Reference<text::XTextContent> xMark(xMarks->getByName(rName), uno::UNO_QUERY);
    if (!xMark.is())
        return;
    uno::Reference<text::XTextRange> xAnchor = xMark->getAnchor();
    if (!xAnchor.is())
        return;
    uno::Reference<text::XText> xText = xAnchor->getText();
    if (!xText.is())
        return;

    // Delete just the marking's own text (the bookmarked range) and its bookmark, then drop
    // the paragraph it occupied only if that left it empty. So if the user typed alongside the
    // marking, their text -- and its paragraph -- survive; bAtStart says which break to absorb.
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursorByRange(xAnchor);
    xText->removeTextContent(xMark);
    xText->insertString(xCursor, OUString(), true);

    uno::Reference<text::XParagraphCursor> xPara(xCursor, uno::UNO_QUERY);
    if (!xPara.is())
        return;
    xPara->gotoStartOfParagraph(false);
    xPara->gotoEndOfParagraph(true);
    if (xCursor->getString().isEmpty()
        && (bAtStart ? xCursor->goRight(1, true) : xCursor->goLeft(1, true)))
        xText->insertString(xCursor, OUString(), true);
}

// Remove portion markings from the body: char-styled runs inside a paragraph that also
// holds the user's own (plain) text. A cover/end-page marking is a whole char-styled
// paragraph (no plain text) removed via its bookmark, so such paragraphs are skipped.
// Collects the runs first (deleting mutates the enumeration), then deletes them.
void removePortionMarkings(const uno::Reference<frame::XModel>& xModel)
{
    uno::Reference<text::XTextDocument> xTextDoc(xModel, uno::UNO_QUERY);
    if (!xTextDoc.is())
        return;
    uno::Reference<text::XText> xBody = xTextDoc->getText();
    uno::Reference<container::XEnumerationAccess> xParaAccess(xBody, uno::UNO_QUERY);
    if (!xParaAccess.is())
        return;

    std::vector<uno::Reference<text::XTextRange>> aToDelete;
    uno::Reference<container::XEnumeration> xParas = xParaAccess->createEnumeration();
    while (xParas->hasMoreElements())
    {
        uno::Reference<container::XEnumerationAccess> xPortAccess(xParas->nextElement(),
                                                                 uno::UNO_QUERY);
        if (!xPortAccess.is())
            continue; // e.g. a table
        std::vector<uno::Reference<text::XTextRange>> aStyled;
        bool bHasPlain = false;
        uno::Reference<container::XEnumeration> xPorts = xPortAccess->createEnumeration();
        while (xPorts->hasMoreElements())
        {
            uno::Reference<beans::XPropertySet> xPortion(xPorts->nextElement(), uno::UNO_QUERY);
            if (!xPortion.is())
                continue;
            OUString sStyle;
            xPortion->getPropertyValue(u"CharStyleName"_ustr) >>= sStyle;
            uno::Reference<text::XTextRange> xRange(xPortion, uno::UNO_QUERY);
            if (sStyle == MARKING_STYLE)
            {
                if (xRange.is())
                    aStyled.push_back(xRange);
            }
            else if (xRange.is() && !xRange->getString().isEmpty())
            {
                bHasPlain = true;
            }
        }
        if (bHasPlain)
            aToDelete.insert(aToDelete.end(), aStyled.begin(), aStyled.end());
    }

    for (const auto& xRange : aToDelete)
    {
        uno::Reference<text::XTextCursor> xCursor = xBody->createTextCursorByRange(xRange);
        xBody->insertString(xCursor, OUString(), true);
    }
}
}

void applyBodyMarkings(const uno::Reference<frame::XModel>& xModel, const OUString& rMarking,
                       sal_Int32 nColor, bool bStart, bool bEnd)
{
    uno::Reference<text::XTextDocument> xTextDoc(xModel, uno::UNO_QUERY);
    if (!xTextDoc.is())
        return;
    ensureMarkingStyle(xModel); // the marking run references it
    uno::Reference<text::XText> xBody = xTextDoc->getText();

    // Always clear first, so a re-label that drops a placement removes its stale body
    // marking; then (re)insert the ones the selection asks for. Also drop any prior
    // portion marking, whose placement may not be requested this time.
    removePortionMarkings(xModel);
    removeBookmarkedMarking(xModel, BOOKMARK_DOC_START, true);
    if (bStart)
        insertMarkingParagraph(xModel, xBody, rMarking, nColor, true, BOOKMARK_DOC_START);

    removeBookmarkedMarking(xModel, BOOKMARK_DOC_END, false);
    if (bEnd)
        insertMarkingParagraph(xModel, xBody, rMarking, nColor, false, BOOKMARK_DOC_END);
}

void removeBodyMarkings(const uno::Reference<frame::XModel>& xModel)
{
    removePortionMarkings(xModel);
    removeBookmarkedMarking(xModel, BOOKMARK_DOC_START, true);
    removeBookmarkedMarking(xModel, BOOKMARK_DOC_END, false);
}

void applyPortionMarking(const uno::Reference<frame::XModel>& xModel,
                         std::u16string_view rMarking, sal_Int32 nColor)
{
    // Portion marking annotates one portion: the paragraph holding the view cursor.
    uno::Reference<text::XTextViewCursorSupplier> xSupplier(xModel->getCurrentController(),
                                                            uno::UNO_QUERY);
    if (!xSupplier.is())
        return;
    uno::Reference<text::XTextRange> xViewCursor = xSupplier->getViewCursor();
    if (!xViewCursor.is())
        return;
    uno::Reference<text::XText> xText = xViewCursor->getText();
    if (!xText.is())
        return;
    ensureMarkingStyle(xModel); // the marking run references it

    const OUString aPrefix = u"("_ustr + rMarking + u") "_ustr;

    // Work at the start of the portion's paragraph.
    uno::Reference<text::XTextCursor> xCursor
        = xText->createTextCursorByRange(xViewCursor->getStart());
    uno::Reference<text::XParagraphCursor> xPara(xCursor, uno::UNO_QUERY);
    if (!xPara.is())
        return;
    xPara->gotoStartOfParagraph(false);

    // Idempotent: skip if this paragraph is already portion-marked with this prefix.
    uno::Reference<text::XTextCursor> xProbe = xText->createTextCursorByRange(xCursor->getStart());
    xProbe->goRight(aPrefix.getLength(), true);
    if (xProbe->getString() == aPrefix)
        return;

    xText->insertString(xCursor, aPrefix, false); // cursor ends after the prefix
    xCursor->goLeft(aPrefix.getLength(), true); // select the inserted prefix
    formatMarkingSelection(xCursor, nColor, /*bCenterParagraph*/ false);
}

} // namespace sw::seclabel

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
