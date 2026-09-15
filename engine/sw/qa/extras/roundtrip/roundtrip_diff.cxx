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

/* A self-labelling fidelity harness for the docx filter.
 *
 * For every document of a corpus we take three dumps of the model:
 *
 *   D  --import-->  A   --export/import-->  B   --export/import-->  C
 *
 * A difference between A and B is something the export and the import lost between them. A
 * difference between B and C is worse, because the file read for it is one we wrote ourselves.
 * Neither comparison needs a reference file or a person, which is what lets it run over the whole
 * corpus and rank what the filter loses by the number of documents it happens in.
 *
 * A table, a frame and a style are read under the name they carry, so the two sides line up
 * wherever the text around them moved. A paragraph has no name and is read under its number, so a
 * paragraph gained or lost renumbers the ones after it; where the two sides disagree on how many
 * there are, the report states that alone and leaves the rest out.
 *
 * Driven by the environment:
 *
 *   SW_RT_RUN      run at all; without it, and without SW_RT_CORPUS, this is a no-op
 *   SW_RT_CORPUS   directory of documents to read (default sw/qa/extras/ooxmlexport/data)
 *   SW_RT_REPORT   report file, appended to and resumable (default sw-roundtrip.log)
 *   SW_RT_LIMIT    stop after that many documents
 *   SW_RT_PARAS    how many paragraphs of a document to read (default 500)
 *   SW_RT_DUMPDIR  if set, write the A, B and C dumps of every document there
 */

#include <sal/config.h>

#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <map>
#include <set>
#include <string_view>
#include <vector>

#include <o3tl/string_view.hxx>

#include <test/unoapixml_test.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/text/WrapTextMode.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XTextFramesSupplier.hpp>
#include <com/sun/star/text/XTextGraphicObjectsSupplier.hpp>
#include <com/sun/star/text/XEndnotesSupplier.hpp>
#include <com/sun/star/text/XFootnotesSupplier.hpp>
#include <com/sun/star/text/XTextSectionsSupplier.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/text/XTextTablesSupplier.hpp>

#include <comphelper/anytostring.hxx>
#include <osl/file.hxx>
#include <rtl/character.hxx>
#include <rtl/strbuf.hxx>
#include <rtl/ustrbuf.hxx>

using namespace css;
using namespace ::cpo;

namespace
{
/** Longest value the report prints. The comparison reads the whole value, because a border or a
    geometry states more than this and a difference beyond the cut would otherwise be invisible. */
constexpr sal_Int32 MAX_VALUE_LEN = 400;

/** How much a number of a given type is allowed to move before it counts as another number. */
enum class Slack
{
    /** Anything not stated as one of the number types: a constant group stands for a state, and
        one step of it is another state. A constant group counts in tens at most, so a number
        larger than that is a measure written as a whole number and moves like one. */
    None,
    /** A length, an integer of twips or hundredths of a millimetre, which a conversion through
        another unit leaves one off. */
    Unit,
    /** A measure carried as a fraction, which a conversion moves by a fraction. */
    Fraction
};

bool numbersEqual(double fLeft, double fRight, Slack eSlack)
{
    const double fMagnitude = std::max(std::abs(fLeft), std::abs(fRight));
    if (eSlack == Slack::None)
        return fLeft == fRight || (fMagnitude >= 100 && std::abs(fLeft - fRight) <= 1.0);
    const double fAllowed
        = eSlack == Slack::Unit ? std::max(1.0, 0.002 * fMagnitude) : 0.002 * fMagnitude;
    return std::abs(fLeft - fRight) <= fAllowed;
}

/** A value states the type it is of before it states itself, and a value built out of others
    states the type of each of them, so the type nearest in front of a number is the one it is
    of. */
Slack slackOf(std::u16string_view rText, size_t nTypeStart)
{
    const size_t nEnd = rText.find(u')', nTypeStart);
    if (nEnd == std::u16string_view::npos)
        return Slack::None;
    const std::u16string_view aType = rText.substr(nTypeStart + 1, nEnd - nTypeStart - 1);
    if (aType == u"long" || aType == u"hyper" || aType == u"unsigned long")
        return Slack::Unit;
    if (aType == u"float" || aType == u"double")
        return Slack::Fraction;
    return Slack::None;
}

/** Properties that differ between two saves of the same model for reasons that say nothing about
    fidelity: names the application makes up, and what it keeps for itself. */
bool isNoise(const OUString& rName)
{
    static const std::set<OUString> aNoise{
        // The picture a shape is drawn into, which differs in a byte or two every time it is
        // produced, and the handle a live object is held under.
        u"MetaFile"_ustr, u"RuntimeUID"_ustr, u"Bitmap"_ustr, u"GraphicStreamURL"_ustr,
        u"GraphicURL"_ustr, u"ReplacementGraphicURL"_ustr,
        // Names the application makes up, and what it keeps for itself.
        u"AbsoluteName"_ustr, u"PrivateTempFileURL"_ustr, u"PreviewMetafile"_ustr,
        u"PreviewBitmap"_ustr, u"Guid"_ustr, u"UserDefinedAttributes"_ustr,
        u"ParaUserDefinedAttributes"_ustr, u"TextUserDefinedAttributes"_ustr,
        // The handle an automatic style is held under, which is an address in memory, and the
        // number a piece of text is filed under, which is counted afresh for every document.
        u"CharAutoStyleName"_ustr, u"ParaAutoStyleName"_ustr, u"SortedTextId"_ustr,
        u"RedlineIdentifier"_ustr,
        // The name an embedded object is filed under in the storage, counted afresh for every
        // document that is written.
        u"StreamName"_ustr,
        // What the editing session recorded about who typed what, which the file need not carry.
        u"Rsid"_ustr, u"ParRsid"_ustr, u"ParaRsid"_ustr,
        // What the layout counted, which is a result of laying the text out and not a part of
        // the document.
        u"ParagraphCount"_ustr, u"WordCount"_ustr, u"CharacterCount"_ustr, u"PageCount"_ustr,
        /*  The name a list and the style behind it are held under, both made up while reading
            and counted afresh, so two reads of one document disagree on them. What the list
            looks like is held in rules that this reads nothing of yet, so a list that comes back
            numbered differently is not seen here. */
        u"ListId"_ustr, u"NumberingStyleName"_ustr,
        // The name a part is offered to a person under, which is made of the name the application
        // gave it and the word for what kind of part it is, in the language of the user.
        u"UINameSingular"_ustr, u"LinkDisplayName"_ustr
    };
    // Whatever the filters could not express and put aside for each other, which is written and
    // read back in a shape of its own and says nothing about what the document holds.
    return aNoise.count(rName) != 0 || rName.endsWith("InteropGrabBag");
}

/** Walks a text document and writes every reachable property into a flat map keyed by the part of
    the document and the property name. Deliberately generic: it asks XPropertySetInfo what is
    there instead of naming properties, so it never falls behind the model. */
class ModelDumper
{
public:
    std::map<OString, OUString> maValues;

    explicit ModelDumper(sal_Int32 nParagraphLimit)
        : mnParagraphLimit(nParagraphLimit)
    {
    }

    void dump(const uno::Reference<lang::XComponent>& xComponent)
    {
        uno::Reference<text::XTextDocument> xDocument(xComponent, uno::UNO_QUERY);
        if (!xDocument.is())
            return;
        dumpProperties(uno::Reference<beans::XPropertySet>(xComponent, uno::UNO_QUERY),
                       "document"_ostr);

        dumpText(xDocument->getText(), "body"_ostr);
        uno::Reference<text::XTextTablesSupplier> xTables(xComponent, uno::UNO_QUERY);
        dumpNamed(xTables.is() ? xTables->getTextTables() : uno::Reference<container::XNameAccess>(),
                  "table");
        uno::Reference<text::XTextFramesSupplier> xFrames(xComponent, uno::UNO_QUERY);
        dumpNamed(xFrames.is() ? xFrames->getTextFrames() : uno::Reference<container::XNameAccess>(),
                  "frame");
        uno::Reference<text::XTextGraphicObjectsSupplier> xGraphics(xComponent, uno::UNO_QUERY);
        dumpNamed(xGraphics.is() ? xGraphics->getGraphicObjects()
                                 : uno::Reference<container::XNameAccess>(),
                  "graphic");
        uno::Reference<text::XTextSectionsSupplier> xSections(xComponent, uno::UNO_QUERY);
        dumpNamed(xSections.is() ? xSections->getTextSections()
                                 : uno::Reference<container::XNameAccess>(),
                  "section");
        dumpStyles(xComponent);
        dumpShapes(xComponent);
        dumpNotes(xComponent);
    }

private:
    sal_Int32 mnParagraphLimit;
    sal_Int32 mnParagraphsRead = 0;

    static OString indexed(std::string_view rTag, sal_Int32 nIndex)
    {
        return OString::Concat(rTag) + "[" + OString::number(nIndex) + "]";
    }

    static OString named(std::string_view rTag, std::u16string_view rName)
    {
        return OString::Concat(rTag) + "[" + OUStringToOString(rName, RTL_TEXTENCODING_UTF8) + "]";
    }

    void add(const OString& rPath, std::string_view rName, const OUString& rValue)
    {
        maValues[OString::Concat(rPath) + "@" + rName] = rValue;
    }

    /** Walks the paragraphs of a text. A table met on the way is stated by name alone, because the
        tables of a document are read as a whole further down. */
    void dumpText(const uno::Reference<text::XText>& xText, const OString& rPath)
    {
        uno::Reference<container::XEnumerationAccess> xAccess(xText, uno::UNO_QUERY);
        if (!xAccess.is())
            return;

        uno::Reference<container::XEnumeration> xParagraphs;
        try
        {
            xParagraphs = xAccess->createEnumeration();
        }
        catch (...)
        {
            return;
        }
        if (!xParagraphs.is())
            return;

        sal_Int32 nIndex = 0;
        while (true)
        {
            uno::Reference<lang::XServiceInfo> xElement;
            try
            {
                if (!xParagraphs->hasMoreElements())
                    break;
                xElement.set(xParagraphs->nextElement(), uno::UNO_QUERY);
            }
            catch (...)
            {
                break;
            }
            if (!xElement.is())
                continue;

            if (xElement->supportsService(u"com.sun.star.text.TextTable"_ustr))
            {
                // Where a table sits among the paragraphs. The name it carries is made up while
                // reading and counted afresh, so only the place it sits at is stated.
                add(rPath, "TableAt", OUString::number(nIndex));
            }
            else if (mnParagraphsRead < mnParagraphLimit)
            {
                ++mnParagraphsRead;
                dumpParagraph(xElement, rPath + OString::Concat("/") + indexed("para", nIndex));
            }
            ++nIndex;
        }
        add(rPath, "ParagraphCount", OUString::number(nIndex));
    }

    void dumpParagraph(const uno::Reference<lang::XServiceInfo>& xParagraph, const OString& rPath)
    {
        // The text of a paragraph is what says which paragraph it is, the way the address says
        // which cell a cell is. A comparison leans on it, so it is read first.
        if (uno::Reference<text::XTextRange> xRange{ xParagraph, uno::UNO_QUERY }; xRange.is())
        {
            try
            {
                add(rPath, "Text", xRange->getString());
            }
            catch (...)
            {
            }
        }

        dumpProperties(uno::Reference<beans::XPropertySet>(xParagraph, uno::UNO_QUERY), rPath);

        uno::Reference<container::XEnumerationAccess> xAccess(xParagraph, uno::UNO_QUERY);
        if (!xAccess.is())
            return;
        uno::Reference<container::XEnumeration> xPortions;
        try
        {
            xPortions = xAccess->createEnumeration();
        }
        catch (...)
        {
            return;
        }
        if (!xPortions.is())
            return;

        sal_Int32 nIndex = 0;
        while (true)
        {
            uno::Reference<text::XTextRange> xPortion;
            try
            {
                if (!xPortions->hasMoreElements())
                    break;
                xPortion.set(xPortions->nextElement(), uno::UNO_QUERY);
            }
            catch (...)
            {
                break;
            }
            if (!xPortion.is())
                continue;

            const OString aPath = rPath + OString::Concat("/") + indexed("run", nIndex);
            try
            {
                add(aPath, "Text", xPortion->getString());
            }
            catch (...)
            {
            }
            dumpProperties(uno::Reference<beans::XPropertySet>(xPortion, uno::UNO_QUERY), aPath);
            ++nIndex;
        }
        add(rPath, "RunCount", OUString::number(nIndex));
    }

    /** Reads a family of parts under their number, in the order the document lists them. The name
        is stated as a value rather than used as the key, because a frame and a table that the
        document does not name are given made up names that are counted afresh on every read. */
    void dumpNamed(const uno::Reference<container::XNameAccess>& xNames, std::string_view rTag)
    {
        if (!xNames.is())
            return;
        cpo::uno::Sequence<OUString> aNames = xNames->getElementNames();
        add(OString(rTag), "Count", OUString::number(aNames.getLength()));

        std::vector<OUString> aSorted(aNames.begin(), aNames.end());
        std::sort(aSorted.begin(), aSorted.end());
        sal_Int32 nIndex = 0;
        for (const OUString& rName : aSorted)
        {
            uno::Reference<cpo::uno::XInterface> xElement;
            try
            {
                xNames->getByName(rName) >>= xElement;
            }
            catch (...)
            {
                ++nIndex;
                continue;
            }
            const OString aPath = indexed(rTag, nIndex++);
            add(aPath, "Name", rName);
            dumpProperties(uno::Reference<beans::XPropertySet>(xElement, uno::UNO_QUERY), aPath);

            if (uno::Reference<text::XTextTable> xTable{ xElement, uno::UNO_QUERY }; xTable.is())
                dumpTable(xTable, aPath);
            else if (uno::Reference<text::XText> xText{ xElement, uno::UNO_QUERY }; xText.is())
                dumpText(xText, aPath);
        }
    }

    void dumpTable(const uno::Reference<text::XTextTable>& xTable, const OString& rPath)
    {
        const cpo::uno::Sequence<OUString> aCells = xTable->getCellNames();
        add(rPath, "CellCount", OUString::number(aCells.getLength()));
        for (const OUString& rCell : aCells)
        {
            uno::Reference<text::XText> xText;
            try
            {
                xText.set(xTable->getCellByName(rCell), uno::UNO_QUERY);
            }
            catch (...)
            {
                continue;
            }
            if (!xText.is())
                continue;
            const OString aPath = rPath + OString::Concat("/") + named("cell", rCell);
            dumpProperties(uno::Reference<beans::XPropertySet>(xText, uno::UNO_QUERY), aPath);
            dumpText(xText, aPath);
        }
    }

    /** Reads the page styles, which carry the margins and the headers and footers, and the
        paragraph styles the document defines. */
    void dumpStyles(const uno::Reference<lang::XComponent>& xComponent)
    {
        uno::Reference<style::XStyleFamiliesSupplier> xSupplier(xComponent, uno::UNO_QUERY);
        if (!xSupplier.is())
            return;
        uno::Reference<container::XNameAccess> xFamilies(xSupplier->getStyleFamilies());
        if (!xFamilies.is())
            return;
        for (const char* pFamily : { "PageStyles", "ParagraphStyles" })
        {
            uno::Reference<container::XNameAccess> xStyles;
            try
            {
                xFamilies->getByName(OUString::createFromAscii(pFamily)) >>= xStyles;
            }
            catch (...)
            {
                continue;
            }
            if (!xStyles.is())
                continue;
            for (const OUString& rName : xStyles->getElementNames())
            {
                uno::Reference<beans::XPropertySet> xStyle;
                try
                {
                    xStyles->getByName(rName) >>= xStyle;
                }
                catch (...)
                {
                    continue;
                }
                // A style nothing uses is written by neither side of a comparison and says
                // nothing about what a document holds.
                uno::Reference<style::XStyle> xInUse(xStyle, uno::UNO_QUERY);
                if (!xInUse.is() || !xInUse->isInUse())
                    continue;
                const OString aPath
                    = OString::Concat(pFamily) + "/" + named("style", rName);
                dumpProperties(xStyle, aPath);
                dumpPageText(xStyle, aPath);
            }
        }
    }

    /** Reads the text of the headers and the footers a page style carries. Each of them is a text
        of its own that the document reaches only through the style, so nothing else reads it. */
    void dumpPageText(const uno::Reference<beans::XPropertySet>& xStyle, const OString& rPath)
    {
        for (const char* pName : { "HeaderText", "HeaderTextLeft", "HeaderTextRight",
                                   "HeaderTextFirst", "FooterText", "FooterTextLeft",
                                   "FooterTextRight", "FooterTextFirst" })
        {
            uno::Reference<text::XText> xText;
            try
            {
                xStyle->getPropertyValue(OUString::createFromAscii(pName)) >>= xText;
            }
            catch (...)
            {
                continue;
            }
            if (xText.is())
                dumpText(xText, OString::Concat(rPath) + "/" + pName);
        }
    }

    /** Reads the footnotes and the endnotes, each of which is a text of its own. */
    void dumpNotes(const uno::Reference<lang::XComponent>& xComponent)
    {
        uno::Reference<container::XIndexAccess> xFootnotes;
        if (uno::Reference<text::XFootnotesSupplier> xSupplier{ xComponent, uno::UNO_QUERY };
            xSupplier.is())
            xFootnotes = xSupplier->getFootnotes();
        dumpNoteList(xFootnotes, "footnote");

        uno::Reference<container::XIndexAccess> xEndnotes;
        if (uno::Reference<text::XEndnotesSupplier> xSupplier{ xComponent, uno::UNO_QUERY };
            xSupplier.is())
            xEndnotes = xSupplier->getEndnotes();
        dumpNoteList(xEndnotes, "endnote");
    }

    void dumpNoteList(const uno::Reference<container::XIndexAccess>& xNotes,
                      std::string_view rTag)
    {
        if (!xNotes.is())
            return;
        add(OString(rTag), "Count", OUString::number(xNotes->getCount()));
        for (sal_Int32 i = 0; i < xNotes->getCount(); ++i)
        {
            uno::Reference<text::XText> xText;
            try
            {
                xNotes->getByIndex(i) >>= xText;
            }
            catch (...)
            {
                continue;
            }
            if (!xText.is())
                continue;
            const OString aPath = indexed(rTag, i);
            dumpProperties(uno::Reference<beans::XPropertySet>(xText, uno::UNO_QUERY), aPath);
            dumpText(xText, aPath);
        }
    }

    /** Says what a shape is, in the terms that stay the same however the shapes are ordered: the
        kind of shape it is, and the text it holds. A shape carries a name as well, but the ones
        the application makes up are counted afresh on every read and say nothing. */
    static OUString shapeIdentity(const uno::Reference<drawing::XShape>& xShape)
    {
        OUString aIdentity = xShape->getShapeType();
        uno::Reference<text::XText> xText(xShape, uno::UNO_QUERY);
        if (xText.is())
        {
            try
            {
                // The report states one finding per line, so the text arrives on one line.
                const OUString aString = xText->getString()
                                             .replaceAll(u"\r", u" ")
                                             .replaceAll(u"\n", u" ")
                                             .replaceAll(u"\t", u" ");
                if (!aString.isEmpty())
                    aIdentity += OUString::Concat(u";")
                                 + aString.subView(0, std::min<sal_Int32>(aString.getLength(), 40));
            }
            catch (...)
            {
            }
        }
        return aIdentity;
    }

    void dumpShapes(const uno::Reference<lang::XComponent>& xComponent)
    {
        uno::Reference<drawing::XDrawPageSupplier> xSupplier(xComponent, uno::UNO_QUERY);
        if (!xSupplier.is())
            return;
        uno::Reference<drawing::XShapes> xShapes = xSupplier->getDrawPage();
        if (!xShapes.is())
            return;
        add("shape"_ostr, "Count", OUString::number(xShapes->getCount()));
        std::map<OUString, sal_Int32> aSeen;
        for (sal_Int32 i = 0; i < xShapes->getCount(); ++i)
        {
            uno::Reference<drawing::XShape> xShape;
            try
            {
                xShapes->getByIndex(i) >>= xShape;
            }
            catch (...)
            {
                continue;
            }
            if (!xShape.is())
                continue;
            /*  A shape is read under what it is rather than under its number, so that the two
                sides line up even where the shapes come back in a different order. Shapes that
                are alike in both respects are told apart by their order among themselves, which
                is all that is left to tell them apart by. */
            OUString aIdentity = shapeIdentity(xShape);
            const sal_Int32 nSame = aSeen[aIdentity]++;
            if (nSame > 0)
                aIdentity += "#" + OUString::number(nSame);
            const OString aPath = named("shape", aIdentity);
            // States that the shape is there at all, so a shape that only one side holds is one
            // finding rather than one for every property it carries.
            add(aPath, "Present", u"yes"_ustr);
            add(aPath, "ShapeType", xShape->getShapeType());
            dumpProperties(uno::Reference<beans::XPropertySet>(xShape, uno::UNO_QUERY), aPath);
            dropAnswersToQuestionsNotAsked(xShape, aPath);
        }
    }

    /** Reads one whole number a shape states, or leaves it alone where the shape has no answer. */
    static bool readNumber(const uno::Reference<beans::XPropertySet>& xProperties,
                           const OUString& rName, sal_Int16& rValue)
    {
        try
        {
            return xProperties->getPropertyValue(rName) >>= rValue;
        }
        catch (...)
        {
            return false;
        }
    }

    /** Drops what a shape answers where nothing asked.

        The distance a shape asks the text to keep from it says nothing where the text flows
        through the shape, because text that flows through a shape keeps no distance from it.
        What a relative width or height is measured against says nothing where the shape has no
        relative width or height. The document reads the same whatever these say, and the two
        filters arrive at them by different routes, so they would report a document as changing
        where nothing about it had. */
    void dropAnswersToQuestionsNotAsked(const uno::Reference<drawing::XShape>& xShape,
                                        std::string_view rPath)
    {
        uno::Reference<beans::XPropertySet> xProperties(xShape, uno::UNO_QUERY);
        if (!xProperties.is())
            return;

        text::WrapTextMode eSurround = text::WrapTextMode_PARALLEL;
        try
        {
            if ((xProperties->getPropertyValue(u"Surround"_ustr) >>= eSurround)
                && eSurround == text::WrapTextMode_THROUGH)
            {
                for (const char* pName :
                     { "LeftMargin", "RightMargin", "TopMargin", "BottomMargin" })
                    maValues.erase(OString::Concat(rPath) + "@" + pName);
            }
        }
        catch (...)
        {
        }

        sal_Int16 nRelative = 0;
        if (readNumber(xProperties, u"RelativeWidth"_ustr, nRelative) && nRelative == 0)
            maValues.erase(OString::Concat(rPath) + "@RelativeWidthRelation");
        nRelative = 0;
        if (readNumber(xProperties, u"RelativeHeight"_ustr, nRelative) && nRelative == 0)
            maValues.erase(OString::Concat(rPath) + "@RelativeHeightRelation");
    }

    void dumpProperties(const uno::Reference<beans::XPropertySet>& xPropertySet,
                        const OString& rPath)
    {
        if (!xPropertySet.is())
            return;
        uno::Reference<beans::XPropertySetInfo> xInfo;
        try
        {
            xInfo = xPropertySet->getPropertySetInfo();
        }
        catch (...)
        {
            return;
        }
        if (!xInfo.is())
            return;

        for (const beans::Property& rProperty : xInfo->getProperties())
        {
            if (rProperty.Type.getTypeClass() == cpo::uno::TypeClass_INTERFACE
                || isNoise(rProperty.Name))
                continue;

            cpo::uno::Any aValue;
            try
            {
                aValue = xPropertySet->getPropertyValue(rProperty.Name);
            }
            catch (...)
            {
                continue;
            }
            if (!aValue.hasValue() || aValue.getValueTypeClass() == cpo::uno::TypeClass_INTERFACE)
                continue;

            const OUString aText = comphelper::anyToString(aValue);
            // A property that holds an empty list says the same as one that holds nothing at all,
            // and the two sides of a comparison do not always answer the same way.
            if (aText.endsWith(") {}"))
                continue;

            maValues[OString::Concat(rPath) + "@"
                     + OUStringToOString(rProperty.Name, RTL_TEXTENCODING_UTF8)]
                = aText;
        }
    }
};

enum class DiffKind
{
    Lost,
    Added,
    Changed
};

struct Diff
{
    DiffKind eKind;
    OString aKey;
    OUString aBefore;
    OUString aAfter;
};

/** Compares two values number aware, so that a width converted through another unit and back does
    not read as a difference. */
bool valuesEqual(std::u16string_view rLeft, std::u16string_view rRight)
{
    if (rLeft == rRight)
        return true;

    size_t nLeft = 0, nRight = 0;
    const size_t nLeftLen = rLeft.size(), nRightLen = rRight.size();

    auto isNumberStart = [](std::u16string_view rText, size_t nPos) {
        sal_Unicode c = rText[nPos];
        if (rtl::isAsciiDigit(c))
            return true;
        return (c == '-' || c == '+') && nPos + 1 < rText.size()
               && rtl::isAsciiDigit(rText[nPos + 1]);
    };
    auto readNumber = [](std::u16string_view rText, size_t& rPos) {
        size_t nStart = rPos;
        if (rText[rPos] == '-' || rText[rPos] == '+')
            ++rPos;
        while (rPos < rText.size() && (rtl::isAsciiDigit(rText[rPos]) || rText[rPos] == '.'))
            ++rPos;
        return o3tl::toDouble(rText.substr(nStart, rPos - nStart));
    };

    Slack eSlack = Slack::None;

    while (nLeft < nLeftLen && nRight < nRightLen)
    {
        if (isNumberStart(rLeft, nLeft) && isNumberStart(rRight, nRight))
        {
            if (!numbersEqual(readNumber(rLeft, nLeft), readNumber(rRight, nRight), eSlack))
                return false;
        }
        else if (rLeft[nLeft] == rRight[nRight])
        {
            if (rLeft[nLeft] == '(')
                eSlack = slackOf(rLeft, nLeft);
            ++nLeft;
            ++nRight;
        }
        else
            return false;
    }
    return nLeft == nLeftLen && nRight == nRightLen;
}

/** Says whether a paragraph or a run that only one side holds tells anything.

    A paragraph is read under its number, so one gained or lost renumbers every paragraph after it
    and the two sides stop lining up. Where the sides disagree on how many there are, the count
    itself is the finding and the rest is left out. */
bool sidesCountTheSame(const std::map<OString, OUString>& rBefore,
                       const std::map<OString, OUString>& rAfter, std::string_view rKey)
{
    // A family read under numbers lines up only while both sides hold as many parts. Shapes are
    // read under what they are instead, so how many there are does not throw them out of step.
    for (const char* pTag : { "table", "frame", "graphic", "section", "footnote", "endnote" })
    {
        if (!o3tl::starts_with(rKey, pTag))
            continue;
        const OString aCountKey = OString::Concat(pTag) + "@Count";
        // The count is the finding itself, so it is not what leaves it out.
        if (std::string_view(aCountKey) == rKey)
            continue;
        const auto aFirst = rBefore.find(aCountKey);
        const auto aSecond = rAfter.find(aCountKey);
        if (aFirst != rBefore.end() && aSecond != rAfter.end() && aFirst->second != aSecond->second)
            return false;
    }

    /*  A shape is read under what it is, so a shape that only one side holds does not throw the
        others out of step. It is stated once, by the mark that says it is there, and everything
        it carries is left out. */
    if (o3tl::starts_with(rKey, "shape[") && !o3tl::ends_with(rKey, "@Present"))
    {
        const size_t nShapeEnd = rKey.rfind(']');
        if (nShapeEnd != std::string_view::npos && nShapeEnd > 0)
        {
            const OString aPresentKey
                = OString::Concat(rKey.substr(0, nShapeEnd + 1)) + "@Present";
            if (rBefore.find(aPresentKey) == rBefore.end()
                || rAfter.find(aPresentKey) == rAfter.end())
                return false;
        }
    }

    const size_t nBounded = rKey.find("/para[");
    if (nBounded == std::string_view::npos)
        return true;

    /*  A paragraph is read under its number, and the two sides line up only while the paragraph
        of that number is the same paragraph. Its text says whether it is: where the two sides
        hold different text under one number, the text itself is the finding and everything the
        paragraph carries is left out, because it describes two different paragraphs. */
    const size_t nParaEnd = rKey.find(']', nBounded);
    if (nParaEnd != std::string_view::npos)
    {
        const OString aTextKey = OString::Concat(rKey.substr(0, nParaEnd + 1)) + "@Text";
        if (std::string_view(aTextKey) != rKey)
        {
            const auto aFirstText = rBefore.find(aTextKey);
            const auto aSecondText = rAfter.find(aTextKey);
            if (aFirstText != rBefore.end() && aSecondText != rAfter.end()
                && aFirstText->second != aSecondText->second)
                return false;
        }
    }

    const OString aCountKey = OString::Concat(rKey.substr(0, nBounded)) + "@ParagraphCount";
    const auto aFirst = rBefore.find(aCountKey);
    const auto aSecond = rAfter.find(aCountKey);
    // A frame or a table that only one side holds is a finding of its own, stated once by the
    // count that only one side carries, so what it holds is left out.
    if (aFirst == rBefore.end() || aSecond == rAfter.end())
        return false;
    if (aFirst->second != aSecond->second)
        return false;

    // A run is read under its number as well, and a paragraph split into a different number of
    // them lines up no better than a body with a different number of paragraphs.
    const size_t nRun = rKey.find("/run[");
    if (nRun == std::string_view::npos)
        return true;
    const OString aRunKey = OString::Concat(rKey.substr(0, nRun)) + "@RunCount";
    const auto aFirstRun = rBefore.find(aRunKey);
    const auto aSecondRun = rAfter.find(aRunKey);
    return aFirstRun == rBefore.end() || aSecondRun == rAfter.end()
           || aFirstRun->second == aSecondRun->second;
}

/** Says whether a run only repeats what the paragraph around it already states.

    A run answers the properties of its paragraph as well as its own, so a paragraph that reads
    differently after a round trip makes every run in it read differently too. Where the run says
    what the paragraph says on both sides, the paragraph states the difference and the run adds
    nothing to it. Where the run says something else, it holds a formatting of its own and what
    becomes of that is the finding. */
bool paragraphSaysTheSame(const std::map<OString, OUString>& rBefore,
                          const std::map<OString, OUString>& rAfter, const OString& rKey)
{
    const sal_Int32 nRun = rKey.indexOf("/run[");
    const sal_Int32 nProperty = rKey.lastIndexOf('@');
    if (nRun < 0 || nProperty < 0)
        return false;

    const OString aParagraphKey = OString::Concat(rKey.subView(0, nRun)) + rKey.subView(nProperty);
    const auto aFirst = rBefore.find(aParagraphKey);
    const auto aSecond = rAfter.find(aParagraphKey);
    if (aFirst == rBefore.end() || aSecond == rAfter.end())
        return false;

    const auto aRunFirst = rBefore.find(rKey);
    const auto aRunSecond = rAfter.find(rKey);
    return aRunFirst != rBefore.end() && aRunSecond != rAfter.end()
           && valuesEqual(aRunFirst->second, aFirst->second)
           && valuesEqual(aRunSecond->second, aSecond->second);
}

std::vector<Diff> compare(const std::map<OString, OUString>& rBefore,
                          const std::map<OString, OUString>& rAfter)
{
    std::vector<Diff> aDiffs;
    for (const auto& rEntry : rBefore)
    {
        auto it = rAfter.find(rEntry.first);
        if (it == rAfter.end())
        {
            if (sidesCountTheSame(rBefore, rAfter, rEntry.first))
                aDiffs.push_back({ DiffKind::Lost, rEntry.first, rEntry.second, OUString() });
        }
        else if (!valuesEqual(rEntry.second, it->second)
                 && sidesCountTheSame(rBefore, rAfter, rEntry.first)
                 && !paragraphSaysTheSame(rBefore, rAfter, rEntry.first))
            aDiffs.push_back({ DiffKind::Changed, rEntry.first, rEntry.second, it->second });
    }
    for (const auto& rEntry : rAfter)
    {
        if (rBefore.find(rEntry.first) == rBefore.end()
            && sidesCountTheSame(rBefore, rAfter, rEntry.first))
            aDiffs.push_back({ DiffKind::Added, rEntry.first, OUString(), rEntry.second });
    }
    return aDiffs;
}

const char* kindName(DiffKind eKind)
{
    switch (eKind)
    {
        case DiffKind::Lost:
            return "LOST";
        case DiffKind::Added:
            return "ADDED";
        case DiffKind::Changed:
            return "CHANGED";
    }
    return "?";
}

/** The report is tab separated, so a value must not carry tabs or newlines. */
OString flatten(const OUString& rText)
{
    OUStringBuffer aBuffer(rText.getLength() > MAX_VALUE_LEN ? rText.copy(0, MAX_VALUE_LEN)
                                                            : rText);
    for (sal_Int32 i = 0; i < aBuffer.getLength(); ++i)
    {
        if (aBuffer[i] == '\t' || aBuffer[i] == '\n' || aBuffer[i] == '\r')
            aBuffer[i] = ' ';
    }
    return OUStringToOString(aBuffer.makeStringAndClear(), RTL_TEXTENCODING_UTF8);
}

OString environment(const char* pName, const char* pDefault)
{
    const char* pValue = std::getenv(pName);
    return OString(pValue && *pValue ? pValue : pDefault);
}
}

class SwRoundtripDiffTest : public UnoApiXmlTest
{
public:
    SwRoundtripDiffTest()
        : UnoApiXmlTest(u"/sw/qa/extras/ooxmlexport/data/"_ustr)
    {
    }

    void testCorpus();

    CPPUNIT_TEST_SUITE(SwRoundtripDiffTest);
    CPPUNIT_TEST(testCorpus);
    CPPUNIT_TEST_SUITE_END();

private:
    std::vector<OUString> collectCorpus(const OUString& rDirectoryUrl);
    void writeDump(std::string_view rDirectory, std::string_view rName, std::string_view rStage,
                   const std::map<OString, OUString>& rValues);
};

std::vector<OUString> SwRoundtripDiffTest::collectCorpus(const OUString& rDirectoryUrl)
{
    std::vector<OUString> aFiles;
    osl::Directory aDirectory(rDirectoryUrl);
    if (aDirectory.open() != osl::FileBase::E_None)
        return aFiles;

    osl::DirectoryItem aItem;
    while (aDirectory.getNextItem(aItem) == osl::FileBase::E_None)
    {
        osl::FileStatus aStatus(osl_FileStatus_Mask_Type | osl_FileStatus_Mask_FileName
                                | osl_FileStatus_Mask_FileURL);
        if (aItem.getFileStatus(aStatus) != osl::FileBase::E_None)
            continue;
        if (aStatus.getFileType() != osl::FileStatus::Regular)
            continue;
        if (!aStatus.getFileName().endsWithIgnoreAsciiCase(u".docx"))
            continue;
        aFiles.push_back(aStatus.getFileURL());
    }
    std::sort(aFiles.begin(), aFiles.end());
    return aFiles;
}

void SwRoundtripDiffTest::writeDump(std::string_view rDirectory, std::string_view rName,
                                    std::string_view rStage,
                                    const std::map<OString, OUString>& rValues)
{
    if (rDirectory.empty())
        return;
    const OString aPath = OString::Concat(rDirectory) + "/" + rName + "." + rStage + ".dump";
    std::ofstream aStream(aPath.getStr());
    for (const auto& rEntry : rValues)
        aStream << rEntry.first << "\t" << flatten(rEntry.second) << "\n";
}

void SwRoundtripDiffTest::testCorpus()
{
    // A corpus run takes hours, so it never happens as part of make check.
    if (!std::getenv("SW_RT_RUN") && !std::getenv("SW_RT_CORPUS"))
        return;

    skipValidation();

    OUString aCorpusUrl;
    const OString aCorpus = environment("SW_RT_CORPUS", "");
    if (aCorpus.isEmpty())
        aCorpusUrl = createFileURL(u""_ustr);
    else
        osl::FileBase::getFileURLFromSystemPath(OStringToOUString(aCorpus, RTL_TEXTENCODING_UTF8),
                                                aCorpusUrl);

    const OString aReportPath = environment("SW_RT_REPORT", "sw-roundtrip.log");
    const OString aDumpDirectory = environment("SW_RT_DUMPDIR", "");
    const sal_Int32 nLimit = environment("SW_RT_LIMIT", "0").toInt32();
    const sal_Int32 nParagraphLimit = environment("SW_RT_PARAS", "500").toInt32();

    // Resume: a document already reported on is not read again, so that the run can be restarted
    // after one of them takes the process down.
    std::set<OString> aDone;
    {
        std::ifstream aStream(aReportPath.getStr());
        std::string aLine;
        while (std::getline(aStream, aLine))
        {
            if (aLine.compare(0, 5, "FILE\t") == 0)
            {
                std::string aRest = aLine.substr(5);
                aDone.insert(OString(aRest.substr(0, aRest.find('\t')).c_str()));
            }
        }
    }

    std::vector<OUString> aFiles = collectCorpus(aCorpusUrl);
    CPPUNIT_ASSERT_MESSAGE("no documents in the corpus", !aFiles.empty());

    std::ofstream aReport(aReportPath.getStr(), std::ios::app);
    sal_Int32 nProcessed = 0;

    for (const OUString& rUrl : aFiles)
    {
        const OString aName
            = OUStringToOString(rUrl.subView(rUrl.lastIndexOf('/') + 1), RTL_TEXTENCODING_UTF8);
        if (aDone.count(aName))
            continue;
        if (nLimit && nProcessed >= nLimit)
            break;
        ++nProcessed;

        // Named before the work starts: if the process dies on this document, the report still
        // says which one it was.
        aReport << "START\t" << aName << "\n";
        aReport.flush();

        ModelDumper aFirst(nParagraphLimit), aSecond(nParagraphLimit), aThird(nParagraphLimit);
        try
        {
            loadFromURL(rUrl);
            aFirst.dump(mxComponent);
            saveAndReload(TestFilter::DOCX);
            aSecond.dump(mxComponent);
            saveAndReload(TestFilter::DOCX);
            aThird.dump(mxComponent);
        }
        catch (const std::exception& rException)
        {
            aReport << "FILE\t" << aName << "\tERROR\t" << rException.what() << "\n";
            aReport.flush();
            continue;
        }
        catch (...)
        {
            aReport << "FILE\t" << aName << "\tERROR\tunknown exception\n";
            aReport.flush();
            continue;
        }

        writeDump(aDumpDirectory, aName, "a", aFirst.maValues);
        writeDump(aDumpDirectory, aName, "b", aSecond.maValues);
        writeDump(aDumpDirectory, aName, "c", aThird.maValues);

        const std::vector<Diff> aLossy = compare(aFirst.maValues, aSecond.maValues);
        const std::vector<Diff> aUnstable = compare(aSecond.maValues, aThird.maValues);

        aReport << "FILE\t" << aName << "\tOK\t" << aFirst.maValues.size() << "\t" << aLossy.size()
                << "\t" << aUnstable.size() << "\n";
        for (const Diff& rDiff : aLossy)
            aReport << "D1\t" << aName << "\t" << kindName(rDiff.eKind) << "\t" << rDiff.aKey
                    << "\t" << flatten(rDiff.aBefore) << "\t" << flatten(rDiff.aAfter) << "\n";
        for (const Diff& rDiff : aUnstable)
            aReport << "D2\t" << aName << "\t" << kindName(rDiff.eKind) << "\t" << rDiff.aKey
                    << "\t" << flatten(rDiff.aBefore) << "\t" << flatten(rDiff.aAfter) << "\n";
        aReport.flush();
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION(SwRoundtripDiffTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
