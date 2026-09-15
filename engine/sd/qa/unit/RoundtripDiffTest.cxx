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

/* A self-labelling fidelity harness for the pptx filter.
 *
 * For every document of a corpus we take three model dumps:
 *
 *   D  --import-->  A   --export/import-->  B   --export/import-->  C
 *
 * A != B means the export/import pair loses or changes something.  B != C is
 * worse: the input of that second round trip is a file we wrote ourselves, so
 * our export is not even a fixed point of our own import.  Neither comparison
 * needs a reference file or a human, which is what lets this run over hundreds
 * of documents and produce a frequency ranked list of what the filter loses.
 *
 * Driven by the environment:
 *
 *   SD_RT_RUN      run at all; without it, and without SD_RT_CORPUS, this is a no-op
 *   SD_RT_CORPUS   directory of documents to process (default sd/qa/unit/data/pptx)
 *   SD_RT_REPORT   report file, appended to and resumable (default sd-roundtrip.log)
 *   SD_RT_LIMIT    stop after that many documents
 *   SD_RT_DUMPDIR  if set, write the A, B and C dumps of every document there
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

#include <test/unoapi_test.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/drawing/XMasterPagesSupplier.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XTextRange.hpp>

#include <comphelper/anytostring.hxx>
#include <osl/file.hxx>
#include <rtl/character.hxx>
#include <rtl/ustrbuf.hxx>

using namespace css;
using namespace ::cpo;

namespace
{
/** Longest value we keep; a few properties (bitmaps, geometry) are enormous. */
constexpr sal_Int32 MAX_VALUE_LEN = 400;

/** Two numbers this close count as the same value. */
bool numbersEqual(double fLeft, double fRight)
{
    const double fMagnitude = std::max(std::abs(fLeft), std::abs(fRight));
    return std::abs(fLeft - fRight) <= std::max(1.0, 0.002 * fMagnitude);
}

/** Properties that differ between two saves of the same model for reasons that
    say nothing about fidelity: names we generate ourselves, interop leftovers. */
bool isNoise(const OUString& rName)
{
    static const std::set<OUString> aNoise{
        // Names we generate ourselves when the model is filled in.
        u"FillGradientName"_ustr, u"FillHatchName"_ustr, u"FillBitmapName"_ustr,
        u"FillTransparenceGradientName"_ustr, u"LineDashName"_ustr,
        // Rendered previews, which differ in a byte or two every time they are
        // produced, and a shape identifier that is freshly drawn on every import.
        u"MetaFile"_ustr, u"Preview"_ustr, u"Guid"_ustr,
        // Display strings that only repeat the shape name.
        u"UINameSingular"_ustr, u"UINamePlural"_ustr, u"LinkDisplayName"_ustr,
        // Carried through the round trip untouched by design.
        u"InteropGrabBag"_ustr, u"UserDefinedAttributes"_ustr
    };
    return aNoise.count(rName) != 0;
}

/** Walks a drawing model and writes every reachable property into a flat map
    keyed by the path of the object it belongs to.  Deliberately generic: it asks
    XPropertySetInfo what is there instead of naming properties, so it never
    falls behind the model the way a hand written dumper does. */
class ModelDumper
{
public:
    std::map<OString, OUString> maValues;

    void dump(const uno::Reference<lang::XComponent>& xComponent)
    {
        uno::Reference<drawing::XDrawPagesSupplier> xDrawPages(xComponent, uno::UNO_QUERY);
        if (xDrawPages.is())
            dumpPages(xDrawPages->getDrawPages(), "page");

        uno::Reference<drawing::XMasterPagesSupplier> xMasterPages(xComponent, uno::UNO_QUERY);
        if (xMasterPages.is())
            dumpPages(xMasterPages->getMasterPages(), "master");
    }

private:
    static OString indexed(std::string_view rBase, std::string_view rTag, sal_Int32 nIndex)
    {
        const OString aIndex = "[" + OString::number(nIndex) + "]";
        if (rBase.empty())
            return OString::Concat(rTag) + aIndex;
        return OString::Concat(rBase) + "/" + rTag + aIndex;
    }

    void add(std::string_view rPath, std::string_view rName, const OUString& rValue)
    {
        maValues[OString::Concat(rPath) + "@" + rName]
            = rValue.getLength() > MAX_VALUE_LEN ? rValue.copy(0, MAX_VALUE_LEN) : rValue;
    }

    void dumpPages(const uno::Reference<drawing::XDrawPages>& xPages, const char* pTag)
    {
        if (!xPages.is())
            return;
        for (sal_Int32 i = 0; i < xPages->getCount(); ++i)
        {
            OString aPath = indexed({}, pTag, i);
            uno::Reference<drawing::XDrawPage> xPage(xPages->getByIndex(i), uno::UNO_QUERY);
            if (!xPage.is())
                continue;
            dumpProperties(uno::Reference<beans::XPropertySet>(xPage, uno::UNO_QUERY), aPath);
            dumpShapes(xPage, aPath);
        }
    }

    void dumpShapes(const uno::Reference<drawing::XShapes>& xShapes, std::string_view rPath)
    {
        if (!xShapes.is())
            return;
        add(rPath, "ShapeCount", OUString::number(xShapes->getCount()));
        for (sal_Int32 i = 0; i < xShapes->getCount(); ++i)
        {
            uno::Reference<drawing::XShape> xShape(xShapes->getByIndex(i), uno::UNO_QUERY);
            if (!xShape.is())
                continue;
            OString aPath = indexed(rPath, "shape", i);
            add(aPath, "ShapeType", xShape->getShapeType());
            dumpProperties(uno::Reference<beans::XPropertySet>(xShape, uno::UNO_QUERY), aPath);

            uno::Reference<drawing::XShapes> xChildren(xShape, uno::UNO_QUERY);
            if (xChildren.is())
                dumpShapes(xChildren, aPath);
            else
                dumpText(uno::Reference<text::XText>(xShape, uno::UNO_QUERY), aPath);
        }
    }

    /** The hand written XShapeDumper takes shape text as one flat string, which
        makes every loss of character formatting invisible.  Go down to runs. */
    void dumpText(const uno::Reference<text::XText>& xText, std::string_view rPath)
    {
        uno::Reference<container::XEnumerationAccess> xParagraphs(xText, uno::UNO_QUERY);
        if (!xParagraphs.is())
            return;

        uno::Reference<container::XEnumeration> xParagraphEnum;
        try
        {
            xParagraphEnum = xParagraphs->createEnumeration();
        }
        catch (...)
        {
            return;
        }

        sal_Int32 nParagraphCount = 0;
        for (sal_Int32 nParagraph = 0; xParagraphEnum.is() && xParagraphEnum->hasMoreElements();
             ++nParagraph)
        {
            nParagraphCount = nParagraph + 1;
            uno::Reference<beans::XPropertySet> xParagraph(xParagraphEnum->nextElement(),
                                                           uno::UNO_QUERY);
            if (!xParagraph.is())
                continue;
            OString aPath = indexed(rPath, "para", nParagraph);
            dumpProperties(xParagraph, aPath);


            uno::Reference<container::XEnumerationAccess> xRuns(xParagraph, uno::UNO_QUERY);
            if (!xRuns.is())
                continue;
            uno::Reference<container::XEnumeration> xRunEnum = xRuns->createEnumeration();
            sal_Int32 nRunCount = 0;
            for (sal_Int32 nRun = 0; xRunEnum.is() && xRunEnum->hasMoreElements(); ++nRun)
            {
                nRunCount = nRun + 1;
                uno::Reference<text::XTextRange> xRun(xRunEnum->nextElement(), uno::UNO_QUERY);
                if (!xRun.is())
                    continue;
                OString aRunPath = indexed(aPath, "run", nRun);
                add(aRunPath, "Text", xRun->getString());
                dumpProperties(uno::Reference<beans::XPropertySet>(xRun, uno::UNO_QUERY),
                               aRunPath);
            }
            add(aPath, "RunCount", OUString::number(nRunCount));
        }
        add(rPath, "ParagraphCount", OUString::number(nParagraphCount));
    }

    void dumpProperties(const uno::Reference<beans::XPropertySet>& xPropertySet,
                        std::string_view rPath)
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

            add(rPath, OUStringToOString(rProperty.Name, RTL_TEXTENCODING_UTF8),
                comphelper::anyToString(aValue));
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

/** Compares two values number aware, so that a position expressed in EMU and
    converted back through 1/100 mm does not show up as a difference. */
bool valuesEqual(const OUString& rLeft, const OUString& rRight)
{
    if (rLeft == rRight)
        return true;

    sal_Int32 nLeft = 0, nRight = 0;
    const sal_Int32 nLeftLen = rLeft.getLength(), nRightLen = rRight.getLength();

    auto isNumberStart = [](const OUString& rText, sal_Int32 nPos) {
        sal_Unicode c = rText[nPos];
        if (rtl::isAsciiDigit(c))
            return true;
        return (c == '-' || c == '+') && nPos + 1 < rText.getLength()
               && rtl::isAsciiDigit(rText[nPos + 1]);
    };
    auto readNumber = [](const OUString& rText, sal_Int32& rPos) {
        sal_Int32 nStart = rPos;
        if (rText[rPos] == '-' || rText[rPos] == '+')
            ++rPos;
        while (rPos < rText.getLength()
               && (rtl::isAsciiDigit(rText[rPos]) || rText[rPos] == '.'))
            ++rPos;
        return o3tl::toDouble(rText.subView(nStart, rPos - nStart));
    };

    while (nLeft < nLeftLen && nRight < nRightLen)
    {
        if (isNumberStart(rLeft, nLeft) && isNumberStart(rRight, nRight))
        {
            double fLeft = readNumber(rLeft, nLeft);
            double fRight = readNumber(rRight, nRight);
            if (!numbersEqual(fLeft, fRight))
                return false;
        }
        else if (rLeft[nLeft] == rRight[nRight])
        {
            ++nLeft;
            ++nRight;
        }
        else
            return false;
    }
    return nLeft == nLeftLen && nRight == nRightLen;
}

/** Says whether a difference at this key tells anything about the filter.

    Three kinds do not. A shape states the character and paragraph properties of the text it holds
    as a single value, which means nothing when the text carries several, and the text itself is
    compared run by run anyway. A run of no characters is formatted in a way nobody can see. And
    the text of a placeholder nobody has typed into is the prompt the application supplies, which
    is never written to a file, so the two sides hold different prompts in different sizes. */
bool tellsSomething(const std::map<OString, OUString>& rBefore,
                    const std::map<OString, OUString>& rAfter, std::string_view rKey)
{
    const size_t nAt = rKey.rfind('@');
    if (nAt == std::string_view::npos)
        return true;
    const OString aPath(rKey.substr(0, nAt));
    const OString aProperty(rKey.substr(nAt + 1));

    auto isEmptyOnEitherSide = [&rBefore, &rAfter](const OString& rValueKey, const char* pEmpty) {
        for (const auto& rSide : { std::cref(rBefore), std::cref(rAfter) })
        {
            const auto it = rSide.get().find(rValueKey);
            if (it != rSide.get().end() && it->second == OUString::createFromAscii(pEmpty))
                return true;
        }
        return false;
    };

    if (aPath.indexOf("/para[") < 0
        && (aProperty.startsWith("Char") || aProperty.startsWith("Para")))
        return false;

    if (aPath.indexOf("/run[") >= 0 && aProperty != "Text"
        && isEmptyOnEitherSide(aPath + "@Text", ""))
        return false;

    const sal_Int32 nParagraph = aPath.indexOf("/para[");
    if (nParagraph >= 0
        && isEmptyOnEitherSide(OString::Concat(aPath.subView(0, nParagraph))
                                   + "@IsEmptyPresentationObject",
                               "true"))
        return false;

    // Once the two sides hold a different number of paragraphs or runs, each index below that
    // point names a different piece of text and every property of it reads as changed.
    auto differsBetweenSides = [&rBefore, &rAfter](const OString& rCountKey) {
        const auto aFirst = rBefore.find(rCountKey);
        const auto aSecond = rAfter.find(rCountKey);
        if (aFirst == rBefore.end() || aSecond == rAfter.end())
            return false;
        return aFirst->second != aSecond->second;
    };
    if (nParagraph >= 0
        && differsBetweenSides(OString::Concat(aPath.subView(0, nParagraph))
                               + "@ParagraphCount"))
        return false;
    const sal_Int32 nRun = aPath.indexOf("/run[");
    if (nRun >= 0
        && differsBetweenSides(OString::Concat(aPath.subView(0, nRun)) + "@RunCount"))
        return false;

    return true;
}

std::vector<Diff> compare(const std::map<OString, OUString>& rBefore,
                          const std::map<OString, OUString>& rAfter)
{
    std::vector<Diff> aDiffs;
    for (const auto& rEntry : rBefore)
    {
        if (!tellsSomething(rBefore, rAfter, rEntry.first))
            continue;
        auto it = rAfter.find(rEntry.first);
        if (it == rAfter.end())
            aDiffs.push_back({ DiffKind::Lost, rEntry.first, rEntry.second, OUString() });
        else if (!valuesEqual(rEntry.second, it->second))
            aDiffs.push_back({ DiffKind::Changed, rEntry.first, rEntry.second, it->second });
    }
    for (const auto& rEntry : rAfter)
    {
        if (rBefore.find(rEntry.first) == rBefore.end()
            && tellsSomething(rBefore, rAfter, rEntry.first))
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

/** The report is tab separated, so values must not carry tabs or newlines. */
OString flatten(std::u16string_view rText)
{
    OUStringBuffer aBuffer(rText);
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

class SdRoundtripDiffTest : public UnoApiTest
{
public:
    SdRoundtripDiffTest()
        : UnoApiTest(u"/sd/qa/unit/data/"_ustr)
    {
    }

    void testCorpus();

    CPPUNIT_TEST_SUITE(SdRoundtripDiffTest);
    CPPUNIT_TEST(testCorpus);
    CPPUNIT_TEST_SUITE_END();

private:
    std::vector<OUString> collectCorpus(const OUString& rDirectoryUrl);
    void writeDump(std::string_view rDirectory, std::string_view rName, std::string_view rStage,
                   const std::map<OString, OUString>& rValues);
};

std::vector<OUString> SdRoundtripDiffTest::collectCorpus(const OUString& rDirectoryUrl)
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
        if (!aStatus.getFileName().endsWithIgnoreAsciiCase(u".pptx"))
            continue;
        aFiles.push_back(aStatus.getFileURL());
    }
    std::sort(aFiles.begin(), aFiles.end());
    return aFiles;
}

void SdRoundtripDiffTest::writeDump(std::string_view rDirectory, std::string_view rName,
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

void SdRoundtripDiffTest::testCorpus()
{
    // A corpus run takes hours, so it never happens as part of make check.
    if (!std::getenv("SD_RT_RUN") && !std::getenv("SD_RT_CORPUS"))
        return;

    skipValidation();

    OUString aCorpusUrl;
    OString aCorpus = environment("SD_RT_CORPUS", "");
    if (aCorpus.isEmpty())
        aCorpusUrl = createFileURL(u"pptx/");
    else
        osl::FileBase::getFileURLFromSystemPath(
            OStringToOUString(aCorpus, RTL_TEXTENCODING_UTF8), aCorpusUrl);

    const OString aReportPath = environment("SD_RT_REPORT", "sd-roundtrip.log");
    const OString aDumpDirectory = environment("SD_RT_DUMPDIR", "");
    const sal_Int32 nLimit = environment("SD_RT_LIMIT", "0").toInt32();

    // Resume: anything already reported on is not done again, so that the run
    // can be restarted after a document takes the process down with it.
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

        // Named before the work starts: if the process dies on this document,
        // the report still says which one it was.
        aReport << "START\t" << aName << "\n";
        aReport.flush();

        ModelDumper aFirst, aSecond, aThird;
        try
        {
            loadFromURL(rUrl);
            aFirst.dump(mxComponent);
            saveAndReload(TestFilter::PPTX);
            aSecond.dump(mxComponent);
            saveAndReload(TestFilter::PPTX);
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

        aReport << "FILE\t" << aName << "\tOK\t" << aFirst.maValues.size() << "\t"
                << aLossy.size() << "\t" << aUnstable.size() << "\n";
        for (const Diff& rDiff : aLossy)
            aReport << "D1\t" << aName << "\t" << kindName(rDiff.eKind) << "\t" << rDiff.aKey
                    << "\t" << flatten(rDiff.aBefore) << "\t" << flatten(rDiff.aAfter) << "\n";
        for (const Diff& rDiff : aUnstable)
            aReport << "D2\t" << aName << "\t" << kindName(rDiff.eKind) << "\t" << rDiff.aKey
                    << "\t" << flatten(rDiff.aBefore) << "\t" << flatten(rDiff.aAfter) << "\n";
        aReport.flush();
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION(SdRoundtripDiffTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
