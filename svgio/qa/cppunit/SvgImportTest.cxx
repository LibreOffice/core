/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>
#include <sal/log.hxx>

#include <test/bootstrapfixture.hxx>
#include <test/xmltesttools.hxx>

#include <comphelper/seqstream.hxx>

#include <com/sun/star/graphic/SvgTools.hpp>
#include <com/sun/star/graphic/XPrimitive2D.hpp>

#include <drawinglayer/primitive2d/Tools.hxx>
#include <drawinglayer/tools/primitive2dxmldump.hxx>
#include <drawinglayer/primitive2d/Primitive2DContainer.hxx>

#include <memory>
#include <string_view>

using namespace css;
using namespace css::uno;
using namespace css::io;
using namespace css::graphic;
using drawinglayer::primitive2d::Primitive2DSequence;
using drawinglayer::primitive2d::Primitive2DContainer;
using drawinglayer::primitive2d::Primitive2DReference;

class Test : public test::BootstrapFixture, public XmlTestTools
{
protected:
    void checkRectPrimitive(Primitive2DSequence const & rPrimitive);

    Primitive2DSequence parseSvg(std::u16string_view aSource);
};

Primitive2DSequence Test::parseSvg(std::u16string_view aSource)
{
    const Reference<XSvgParser> xSvgParser = SvgTools::create(m_xContext);

    OUString aUrl  = m_directories.getURLFromSrc(aSource);
    OUString aPath = m_directories.getPathFromSrc(aSource);

    SvFileStream aFileStream(aUrl, StreamMode::READ);
    std::size_t nSize = aFileStream.remainingSize();
    std::unique_ptr<sal_Int8[]> pBuffer(new sal_Int8[nSize + 1]);
    aFileStream.ReadBytes(pBuffer.get(), nSize);
    pBuffer[nSize] = 0;

    Sequence<sal_Int8> aData(pBuffer.get(), nSize + 1);
    Reference<XInputStream> aInputStream(new comphelper::SequenceInputStream(aData));

    return xSvgParser->getDecomposition(aInputStream, aPath);
}

void Test::checkRectPrimitive(Primitive2DSequence const & rPrimitive)
{
    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(rPrimitive));

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor"_ostr, "color"_ostr, "#00cc00"); // rect background color
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor/polypolygon"_ostr, "height"_ostr, "100"); // rect background height
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor/polypolygon"_ostr, "width"_ostr, "100"); // rect background width
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor/polypolygon"_ostr, "minx"_ostr, "10");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor/polypolygon"_ostr, "miny"_ostr, "10");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor/polypolygon"_ostr, "maxx"_ostr, "110");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor/polypolygon"_ostr, "maxy"_ostr, "110");
    assertXPath(pDocument, "/primitive2D/transform/polypolygonstroke/line"_ostr, "color"_ostr, "#ff0000"); // rect stroke color
    assertXPath(pDocument, "/primitive2D/transform/polypolygonstroke/line"_ostr, "width"_ostr, "3"); // rect stroke width


}

namespace
{
bool arePrimitive2DSequencesEqual(const Primitive2DSequence& rA, const Primitive2DSequence& rB)
{
    auto rv = std::mismatch(rA.begin(), rA.end(), rB.begin(), rB.end(),
        [](const css::uno::Reference<css::graphic::XPrimitive2D>& a,
           const css::uno::Reference<css::graphic::XPrimitive2D>& b)
        {
            return drawinglayer::primitive2d::arePrimitive2DReferencesEqual(a, b);
        });
    if (rv.first == rA.end() && rv.second == rB.end())
        return true;
    if (rv.first == rA.end() || rv.second == rB.end())
    {
        SAL_WARN("svgio",
                "first seq length == " << rA.size() <<
                "second seq length == " << rB.size());
        return false;
    }
    auto idx = std::distance(rA.begin(), rv.first);
    SAL_WARN("svgio",
            "first difference at index " << idx <<
            " expected element " << typeid(*rA[idx]).name() <<
            " but got element " << typeid(*rB[idx]).name());
    return false;
}
}

// Attributes for an object (like rect as in this case) can be defined
// in different ways (directly with xml attributes, or with CSS styles),
// however the end result should be the same.
CPPUNIT_TEST_FIXTURE(Test, testStyles)
{
    Primitive2DSequence aSequenceRect = parseSvg(u"/svgio/qa/cppunit/data/Rect.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequenceRect.getLength()));
    checkRectPrimitive(aSequenceRect);

    Primitive2DSequence aSequenceRectWithStyle = parseSvg(u"/svgio/qa/cppunit/data/RectWithStyles.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequenceRectWithStyle.getLength()));
    checkRectPrimitive(aSequenceRectWithStyle);

    Primitive2DSequence aSequenceRectWithParentStyle = parseSvg(u"/svgio/qa/cppunit/data/RectWithParentStyles.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequenceRectWithParentStyle.getLength()));
    checkRectPrimitive(aSequenceRectWithParentStyle);

    Primitive2DSequence aSequenceRectWithStylesByGroup = parseSvg(u"/svgio/qa/cppunit/data/RectWithStylesByGroup.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequenceRectWithStylesByGroup.getLength()));
    checkRectPrimitive(aSequenceRectWithStylesByGroup);

    CPPUNIT_ASSERT(arePrimitive2DSequencesEqual(aSequenceRect, aSequenceRectWithStyle));
    CPPUNIT_ASSERT(arePrimitive2DSequencesEqual(aSequenceRect, aSequenceRectWithParentStyle));
    CPPUNIT_ASSERT(arePrimitive2DSequencesEqual(aSequenceRect, aSequenceRectWithStylesByGroup));
}

CPPUNIT_TEST_FIXTURE(Test, testSymbol)
{
    Primitive2DSequence aSequenceTdf87309 = parseSvg(u"/svgio/qa/cppunit/data/symbol.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequenceTdf87309.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(aSequenceTdf87309);

    CPPUNIT_ASSERT (pDocument);

    // tdf#126330: Without the fix in place, this test would have failed with
    // - Expected: 1
    // - Actual  : 2
    // number of nodes is incorrect
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor"_ostr, "color"_ostr, "#00d000");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf150124)
{
    Primitive2DSequence aSequence = parseSvg(u"/svgio/qa/cppunit/data/tdf150124.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(aSequence);

    CPPUNIT_ASSERT (pDocument);

    assertXPathChildren(pDocument, "/primitive2D"_ostr, 1);
    assertXPath(pDocument, "/primitive2D/hiddengeometry"_ostr, 1);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf155819)
{
    Primitive2DSequence aSequence = parseSvg(u"/svgio/qa/cppunit/data/tdf155819.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(aSequence);

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/polypolygonstroke/line"_ostr, 1);
    assertXPath(pDocument, "/primitive2D/transform/polypolygonstroke/polypolygon"_ostr, 1);
    // Without the fix in place, this test would have failed with
    // - Expected: 4
    // - Actual  : 0
    assertXPath(pDocument, "/primitive2D/transform/transform"_ostr, 4);
}

CPPUNIT_TEST_FIXTURE(Test, testFilterFeBlend)
{
    Primitive2DSequence aSequence = parseSvg(u"/svgio/qa/cppunit/data/filterFeBlend.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(aSequence);

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/transform/polypolygoncolor[1]"_ostr, "color"_ostr, "#8a2be2");
    assertXPath(pDocument, "/primitive2D/transform/transform/polypolygoncolor[1]/polypolygon"_ostr, "height"_ostr, "100");
    assertXPath(pDocument, "/primitive2D/transform/transform/polypolygoncolor[1]/polypolygon"_ostr, "width"_ostr, "100");
    assertXPath(pDocument, "/primitive2D/transform/transform/polypolygoncolor[1]/polypolygon"_ostr, "minx"_ostr, "70");
    assertXPath(pDocument, "/primitive2D/transform/transform/polypolygoncolor[1]/polypolygon"_ostr, "miny"_ostr, "70");
    assertXPath(pDocument, "/primitive2D/transform/transform/polypolygoncolor[1]/polypolygon"_ostr, "maxx"_ostr, "170");
    assertXPath(pDocument, "/primitive2D/transform/transform/polypolygoncolor[1]/polypolygon"_ostr, "maxy"_ostr, "170");
    assertXPath(pDocument, "/primitive2D/transform/transform/polypolygoncolor[2]"_ostr, "color"_ostr, "#ffd700");
    assertXPath(pDocument, "/primitive2D/transform/transform/polypolygoncolor[2]/polypolygon"_ostr, "height"_ostr, "100");
    assertXPath(pDocument, "/primitive2D/transform/transform/polypolygoncolor[2]/polypolygon"_ostr, "width"_ostr, "100");
    assertXPath(pDocument, "/primitive2D/transform/transform/polypolygoncolor[2]/polypolygon"_ostr, "minx"_ostr, "30");
    assertXPath(pDocument, "/primitive2D/transform/transform/polypolygoncolor[2]/polypolygon"_ostr, "miny"_ostr, "30");
    assertXPath(pDocument, "/primitive2D/transform/transform/polypolygoncolor[2]/polypolygon"_ostr, "maxx"_ostr, "130");
    assertXPath(pDocument, "/primitive2D/transform/transform/polypolygoncolor[2]/polypolygon"_ostr, "maxy"_ostr, "130");
}

CPPUNIT_TEST_FIXTURE(Test, testFeColorMatrix)
{
    Primitive2DSequence aSequence = parseSvg(u"/svgio/qa/cppunit/data/filterFeColorMatrix.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(aSequence);

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/mask/modifiedColor[1]"_ostr, "modifier"_ostr, "matrix");
    assertXPath(pDocument, "/primitive2D/transform/mask/modifiedColor[2]"_ostr, "modifier"_ostr, "saturate");
    assertXPath(pDocument, "/primitive2D/transform/mask/modifiedColor[3]"_ostr, "modifier"_ostr, "hueRotate");
    assertXPath(pDocument, "/primitive2D/transform/mask/modifiedColor[4]"_ostr, "modifier"_ostr, "luminance_to_alpha");
}

CPPUNIT_TEST_FIXTURE(Test, testFilterFeGaussianBlur)
{
    Primitive2DSequence aSequenceTdf132246 = parseSvg(u"/svgio/qa/cppunit/data/filterFeGaussianBlur.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequenceTdf132246.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(aSequenceTdf132246);

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/softedge"_ostr, "radius"_ostr, "5");
}

CPPUNIT_TEST_FIXTURE(Test, testInFilterAttribute)
{
    Primitive2DSequence aSequence = parseSvg(u"/svgio/qa/cppunit/data/inFilterAttribute.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(aSequence);

    CPPUNIT_ASSERT (pDocument);

    // Without the fix in place, the feGaussianBlur and feColorMatrix filter would have been applied
    assertXPath(pDocument, "/primitive2D/transform/transform"_ostr, "xy11"_ostr, "1");
    assertXPath(pDocument, "/primitive2D/transform/transform"_ostr, "xy12"_ostr, "0");
    assertXPath(pDocument, "/primitive2D/transform/transform"_ostr, "xy13"_ostr, "40");
    assertXPath(pDocument, "/primitive2D/transform/transform"_ostr, "xy21"_ostr, "0");
    assertXPath(pDocument, "/primitive2D/transform/transform"_ostr, "xy22"_ostr, "1");
    assertXPath(pDocument, "/primitive2D/transform/transform"_ostr, "xy23"_ostr, "40");
    assertXPath(pDocument, "/primitive2D/transform/transform"_ostr, "xy31"_ostr, "0");
    assertXPath(pDocument, "/primitive2D/transform/transform"_ostr, "xy32"_ostr, "0");
    assertXPath(pDocument, "/primitive2D/transform/transform"_ostr, "xy33"_ostr, "1");
    assertXPath(pDocument, "/primitive2D/transform/transform/polypolygoncolor"_ostr, "color"_ostr, "#ffffff");
}

CPPUNIT_TEST_FIXTURE(Test, testResultFilterAttribute)
{
    Primitive2DSequence aSequence = parseSvg(u"/svgio/qa/cppunit/data/resultFilterAttribute.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(aSequence);

    CPPUNIT_ASSERT (pDocument);

    // Without the fix in place, the feColorMatrix filter would have been applied
    assertXPath(pDocument, "/primitive2D/transform/transform"_ostr, "xy11"_ostr, "1");
    assertXPath(pDocument, "/primitive2D/transform/transform"_ostr, "xy12"_ostr, "0");
    assertXPath(pDocument, "/primitive2D/transform/transform"_ostr, "xy13"_ostr, "40");
    assertXPath(pDocument, "/primitive2D/transform/transform"_ostr, "xy21"_ostr, "0");
    assertXPath(pDocument, "/primitive2D/transform/transform"_ostr, "xy22"_ostr, "1");
    assertXPath(pDocument, "/primitive2D/transform/transform"_ostr, "xy23"_ostr, "40");
    assertXPath(pDocument, "/primitive2D/transform/transform"_ostr, "xy31"_ostr, "0");
    assertXPath(pDocument, "/primitive2D/transform/transform"_ostr, "xy32"_ostr, "0");
    assertXPath(pDocument, "/primitive2D/transform/transform"_ostr, "xy33"_ostr, "1");
    assertXPath(pDocument, "/primitive2D/transform/transform/softedge"_ostr, "radius"_ostr, "2");
    assertXPath(pDocument, "/primitive2D/transform/transform/softedge/polypolygoncolor"_ostr, "color"_ostr, "#ffffff");
}

CPPUNIT_TEST_FIXTURE(Test, testFilterFeMerge)
{
    Primitive2DSequence aSequence = parseSvg(u"/svgio/qa/cppunit/data/filterFeMerge.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(aSequence);

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/mask/softedge"_ostr, 2);
    assertXPath(pDocument, "/primitive2D/transform/mask/polypolygon"_ostr, 1);
    assertXPath(pDocument, "/primitive2D/transform/mask/polypolygoncolor"_ostr, 1);
    assertXPath(pDocument, "/primitive2D/transform/mask/polypolygonstroke"_ostr, 1);
}

CPPUNIT_TEST_FIXTURE(Test, testFilterFeOffset)
{
    Primitive2DSequence aSequenceTdf132246 = parseSvg(u"/svgio/qa/cppunit/data/filterFeOffset.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequenceTdf132246.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(aSequenceTdf132246);

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/mask/transform"_ostr, "xy11"_ostr, "1");
    assertXPath(pDocument, "/primitive2D/transform/mask/transform"_ostr, "xy12"_ostr, "0");
    assertXPath(pDocument, "/primitive2D/transform/mask/transform"_ostr, "xy13"_ostr, "44");
    assertXPath(pDocument, "/primitive2D/transform/mask/transform"_ostr, "xy21"_ostr, "0");
    assertXPath(pDocument, "/primitive2D/transform/mask/transform"_ostr, "xy22"_ostr, "1");
    assertXPath(pDocument, "/primitive2D/transform/mask/transform"_ostr, "xy23"_ostr, "66");
    assertXPath(pDocument, "/primitive2D/transform/mask/transform"_ostr, "xy31"_ostr, "0");
    assertXPath(pDocument, "/primitive2D/transform/mask/transform"_ostr, "xy32"_ostr, "0");
    assertXPath(pDocument, "/primitive2D/transform/mask/transform"_ostr, "xy33"_ostr, "1");
}

CPPUNIT_TEST_FIXTURE(Test, testFilterFeFlood)
{
    Primitive2DSequence aSequenceTdf132246 = parseSvg(u"/svgio/qa/cppunit/data/filterFeFlood.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequenceTdf132246.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(aSequenceTdf132246);

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/unifiedtransparence"_ostr, "transparence"_ostr, "50");
    assertXPath(pDocument, "/primitive2D/transform/unifiedtransparence/polypolygoncolor"_ostr, "color"_ostr, "#008000");
    assertXPath(pDocument, "/primitive2D/transform/unifiedtransparence/polypolygoncolor/polypolygon"_ostr, "height"_ostr, "100");
    assertXPath(pDocument, "/primitive2D/transform/unifiedtransparence/polypolygoncolor/polypolygon"_ostr, "width"_ostr, "100");
    assertXPath(pDocument, "/primitive2D/transform/unifiedtransparence/polypolygoncolor/polypolygon"_ostr, "minx"_ostr, "50");
    assertXPath(pDocument, "/primitive2D/transform/unifiedtransparence/polypolygoncolor/polypolygon"_ostr, "miny"_ostr, "50");
}

CPPUNIT_TEST_FIXTURE(Test, testFilterFeDropShadow)
{
    Primitive2DSequence aSequenceTdf132246 = parseSvg(u"/svgio/qa/cppunit/data/filterFeDropShadow.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequenceTdf132246.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(aSequenceTdf132246);

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/unifiedtransparence"_ostr, "transparence"_ostr, "50");
    assertXPath(pDocument, "/primitive2D/transform/unifiedtransparence/shadow"_ostr, "color"_ostr, "#0000ff");
    assertXPath(pDocument, "/primitive2D/transform/unifiedtransparence/shadow"_ostr, "blur"_ostr, "0.2");
    assertXPath(pDocument, "/primitive2D/transform/unifiedtransparence/shadow"_ostr, "blur"_ostr, "0.2");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor"_ostr, "color"_ostr, "#ffc0cb");
}

CPPUNIT_TEST_FIXTURE(Test, testFilterFeImage)
{
    Primitive2DSequence aSequenceTdf132246 = parseSvg(u"/svgio/qa/cppunit/data/filterFeImage.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequenceTdf132246.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(aSequenceTdf132246);

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/transform/bitmap"_ostr);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf87309)
{
    Primitive2DSequence aSequenceTdf87309 = parseSvg(u"/svgio/qa/cppunit/data/tdf87309.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequenceTdf87309.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(aSequenceTdf87309);

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor"_ostr, "color"_ostr, "#000000");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor/polypolygon"_ostr, "height"_ostr, "100");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor/polypolygon"_ostr, "width"_ostr, "100");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor/polypolygon"_ostr, "minx"_ostr, "10");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor/polypolygon"_ostr, "miny"_ostr, "10");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor/polypolygon"_ostr, "maxx"_ostr, "110");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor/polypolygon"_ostr, "maxy"_ostr, "110");
}

CPPUNIT_TEST_FIXTURE(Test, testFontsizeKeywords)
{
    Primitive2DSequence aSequenceFontsizeKeywords = parseSvg(u"/svgio/qa/cppunit/data/FontsizeKeywords.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequenceFontsizeKeywords.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(aSequenceFontsizeKeywords);

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "fontcolor"_ostr, "#000000");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "text"_ostr, "Sample");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "height"_ostr, "9");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "familyname"_ostr, "Times New Roman");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "fontcolor"_ostr, "#ffffff");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "text"_ostr, "Sample");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "height"_ostr, "11");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "familyname"_ostr, "Times New Roman");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[3]"_ostr, "fontcolor"_ostr, "#ffd700");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[3]"_ostr, "text"_ostr, "Sample");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[3]"_ostr, "height"_ostr, "13");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[3]"_ostr, "familyname"_ostr, "Times New Roman");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[4]"_ostr, "fontcolor"_ostr, "#ff0000");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[4]"_ostr, "text"_ostr, "Sample");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[4]"_ostr, "height"_ostr, "16");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[4]"_ostr, "familyname"_ostr, "Times New Roman");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[5]"_ostr, "fontcolor"_ostr, "#ffff00");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[5]"_ostr, "text"_ostr, "Sample");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[5]"_ostr, "height"_ostr, "19");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[5]"_ostr, "familyname"_ostr, "Times New Roman");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[6]"_ostr, "fontcolor"_ostr, "#0000ff");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[6]"_ostr, "text"_ostr, "Sample");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[6]"_ostr, "height"_ostr, "23");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[6]"_ostr, "familyname"_ostr, "Times New Roman");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[7]"_ostr, "fontcolor"_ostr, "#008000");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[7]"_ostr, "text"_ostr, "Sample");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[7]"_ostr, "height"_ostr, "27");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[7]"_ostr, "familyname"_ostr, "Times New Roman");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[8]"_ostr, "fontcolor"_ostr, "#ff7f50");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[8]"_ostr, "text"_ostr, "Sample");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[8]"_ostr, "height"_ostr, "13");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[8]"_ostr, "familyname"_ostr, "Times New Roman");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[9]"_ostr, "fontcolor"_ostr, "#ffc0cb");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[9]"_ostr, "text"_ostr, "Sample");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[9]"_ostr, "height"_ostr, "19");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[9]"_ostr, "familyname"_ostr, "Times New Roman");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[10]"_ostr, "fontcolor"_ostr, "#fffff0");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[10]"_ostr, "text"_ostr, "Sample");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[10]"_ostr, "height"_ostr, "16");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[9]"_ostr, "familyname"_ostr, "Times New Roman");
}


CPPUNIT_TEST_FIXTURE(Test, testFontsizePercentage)
{
    //Check when font-size uses percentage and defined globally
    Primitive2DSequence aSequenceFontsizePercentage = parseSvg(u"/svgio/qa/cppunit/data/FontsizePercentage.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequenceFontsizePercentage.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(aSequenceFontsizePercentage);

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "fontcolor"_ostr, "#000000");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "text"_ostr, "Sample");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "height"_ostr, "16");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "familyname"_ostr, "Times New Roman");
}

CPPUNIT_TEST_FIXTURE(Test, testFontsizeRelative)
{
    //Check when font-size uses relative units (em,ex) and it's based on its parent's font-size
    Primitive2DSequence aSequenceFontsizeRelative = parseSvg(u"/svgio/qa/cppunit/data/FontsizeRelative.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequenceFontsizeRelative.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(aSequenceFontsizeRelative);

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "fontcolor"_ostr, "#000000");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "text"_ostr, "Sample");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "height"_ostr, "50");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "familyname"_ostr, "DejaVu Serif");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "fontcolor"_ostr, "#000000");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "text"_ostr, "Sample");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "height"_ostr, "50");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "familyname"_ostr, "DejaVu Serif");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf145896)
{
    Primitive2DSequence aSequence = parseSvg(u"/svgio/qa/cppunit/data/tdf145896.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(aSequence);

    CPPUNIT_ASSERT (pDocument);

    // Without the fix in place, this test would have failed with
    // - Expected: #ffff00
    // - Actual  : #000000
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[1]"_ostr, "color"_ostr, "#ffff00");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[2]"_ostr, "color"_ostr, "#008000");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[3]"_ostr, "color"_ostr, "#0000ff");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf156168)
{
    Primitive2DSequence aSequence = parseSvg(u"/svgio/qa/cppunit/data/tdf156168.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(aSequence);

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor"_ostr, 8);
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[1]"_ostr, "color"_ostr, "#0000ff");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[2]"_ostr, "color"_ostr, "#0000ff");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[3]"_ostr, "color"_ostr, "#ff0000");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[4]"_ostr, "color"_ostr, "#ff0000");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[5]"_ostr, "color"_ostr, "#000000");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[6]"_ostr, "color"_ostr, "#000000");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[7]"_ostr, "color"_ostr, "#ff0000");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[8]"_ostr, "color"_ostr, "#ff0000");

    // Without the fix in place, this test would have failed with
    // - Expected: 4
    // - Actual  : 3
    assertXPath(pDocument, "/primitive2D/transform/polypolygonstroke"_ostr, 4);
    assertXPath(pDocument, "/primitive2D/transform/polypolygonstroke[1]/line"_ostr, "width"_ostr, "5");

    assertXPath(pDocument, "/primitive2D/transform/polypolygonstroke[1]/line"_ostr, "color"_ostr, "#00ff00");
    assertXPath(pDocument, "/primitive2D/transform/polypolygonstroke[2]/line"_ostr, "width"_ostr, "5");

    assertXPath(pDocument, "/primitive2D/transform/polypolygonstroke[2]/line"_ostr, "color"_ostr, "#00ff00");
    assertXPath(pDocument, "/primitive2D/transform/polypolygonstroke[3]/line"_ostr, "width"_ostr, "5");

    assertXPath(pDocument, "/primitive2D/transform/polypolygonstroke[3]/line"_ostr, "color"_ostr, "#00ff00");
    assertXPath(pDocument, "/primitive2D/transform/polypolygonstroke[4]/line"_ostr, "width"_ostr, "5");

    assertXPath(pDocument, "/primitive2D/transform/polypolygonstroke[4]/line"_ostr, "color"_ostr, "#00ff00");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf129356)
{
    Primitive2DSequence aSequence = parseSvg(u"/svgio/qa/cppunit/data/tdf129356.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(aSequence);

    CPPUNIT_ASSERT (pDocument);

    // Without the fix in place, this test would have failed with
    // - Expected: #008000
    // - Actual  : #0000ff
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[1]"_ostr, "color"_ostr, "#008000");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[2]"_ostr, "color"_ostr, "#008000");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[3]"_ostr, "color"_ostr, "#008000");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[4]"_ostr, "color"_ostr, "#008000");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[5]"_ostr, "color"_ostr, "#008000");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[6]"_ostr, "color"_ostr, "#008000");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[7]"_ostr, "color"_ostr, "#008000");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[8]"_ostr, "color"_ostr, "#008000");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf156034)
{
    Primitive2DSequence aSequence = parseSvg(u"/svgio/qa/cppunit/data/tdf156034.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(aSequence);

    CPPUNIT_ASSERT (pDocument);

    // Without the fix in place, this test would have failed with
    // - Expected: #008000
    // - Actual  : #0000ff
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[1]"_ostr, "color"_ostr, "#008000");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[2]"_ostr, "color"_ostr, "#008000");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[3]"_ostr, "color"_ostr, "#008000");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[4]"_ostr, "color"_ostr, "#008000");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[5]"_ostr, "color"_ostr, "#008000");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[6]"_ostr, "color"_ostr, "#008000");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[7]"_ostr, "color"_ostr, "#008000");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[8]"_ostr, "color"_ostr, "#008000");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf156038)
{
    Primitive2DSequence aSequence = parseSvg(u"/svgio/qa/cppunit/data/tdf156038.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(aSequence);

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[1]"_ostr, "color"_ostr, "#0000ff");

    // Without the fix in place, this test would have failed with
    // - Expected: #008000
    // - Actual  : #0000ff
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[2]"_ostr, "color"_ostr, "#008000");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[3]"_ostr, "color"_ostr, "#0000ff");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[4]"_ostr, "color"_ostr, "#008000");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[5]"_ostr, "color"_ostr, "#0000ff");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[6]"_ostr, "color"_ostr, "#008000");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[7]"_ostr, "color"_ostr, "#0000ff");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[8]"_ostr, "color"_ostr, "#008000");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf156018)
{
    Primitive2DSequence aSequence = parseSvg(u"/svgio/qa/cppunit/data/tdf156018.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(aSequence);

    CPPUNIT_ASSERT (pDocument);

    // Without the fix in place, this test would have failed with
    // - Expected: #008000
    // - Actual  : #0000ff
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[1]"_ostr, "color"_ostr, "#008000");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[2]"_ostr, "color"_ostr, "#0000ff");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf156201)
{
    Primitive2DSequence aSequence = parseSvg(u"/svgio/qa/cppunit/data/tdf156201.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(aSequence);

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/transform/transform/polypolygoncolor"_ostr, "color"_ostr, "#2f3ba1");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf156167)
{
    Primitive2DSequence aSequence = parseSvg(u"/svgio/qa/cppunit/data/tdf156167.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(aSequence);

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[1]"_ostr, "color"_ostr, "#ffa500");

    // Without the fix in place, this test would have failed with
    // - Expected: #ffa500
    // - Actual  : #ff0000
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[2]"_ostr, "color"_ostr, "#ffa500");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[3]"_ostr, "color"_ostr, "#ffa500");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf155932)
{
    Primitive2DSequence aSequence = parseSvg(u"/svgio/qa/cppunit/data/tdf155932.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(aSequence);

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/mask/mask/unifiedtransparence"_ostr, "transparence"_ostr, "50");
    assertXPath(pDocument, "/primitive2D/transform/mask/mask/unifiedtransparence[1]/polypolygoncolor"_ostr, "color"_ostr, "#0000ff");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf97717)
{
    Primitive2DSequence aSequence = parseSvg(u"/svgio/qa/cppunit/data/tdf97717.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(aSequence);

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/unifiedtransparence[1]"_ostr, "transparence"_ostr, "50");
    // Without the fix in place, this test would have failed here since the patch
    // would have contained two unifiedtransparence
    assertXPath(pDocument, "/primitive2D/transform/unifiedtransparence[1]/polypolygoncolor"_ostr, "color"_ostr, "#ccccff");
    assertXPath(pDocument, "/primitive2D/transform/unifiedtransparence[2]"_ostr, "transparence"_ostr, "50");
    assertXPath(pDocument, "/primitive2D/transform/unifiedtransparence[2]/polypolygoncolor"_ostr, "color"_ostr, "#ccccff");
}

CPPUNIT_TEST_FIXTURE(Test, testMarkerOrient)
{
    Primitive2DSequence aSequence = parseSvg(u"/svgio/qa/cppunit/data/MarkerOrient.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(aSequence);

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/transform[1]"_ostr, "xy11"_ostr, "0");
    assertXPath(pDocument, "/primitive2D/transform/transform[1]"_ostr, "xy12"_ostr, "0");
    assertXPath(pDocument, "/primitive2D/transform/transform[1]"_ostr, "xy13"_ostr, "7");
    assertXPath(pDocument, "/primitive2D/transform/transform[1]"_ostr, "xy21"_ostr, "0");
    assertXPath(pDocument, "/primitive2D/transform/transform[1]"_ostr, "xy22"_ostr, "0");
    assertXPath(pDocument, "/primitive2D/transform/transform[1]"_ostr, "xy23"_ostr, "13");
    assertXPath(pDocument, "/primitive2D/transform/transform[1]"_ostr, "xy31"_ostr, "0");
    assertXPath(pDocument, "/primitive2D/transform/transform[1]"_ostr, "xy32"_ostr, "0");
    assertXPath(pDocument, "/primitive2D/transform/transform[1]"_ostr, "xy33"_ostr, "1");

    assertXPath(pDocument, "/primitive2D/transform/transform[2]"_ostr, "xy11"_ostr, "0");
    assertXPath(pDocument, "/primitive2D/transform/transform[2]"_ostr, "xy12"_ostr, "0");
    assertXPath(pDocument, "/primitive2D/transform/transform[2]"_ostr, "xy13"_ostr, "87");
    assertXPath(pDocument, "/primitive2D/transform/transform[2]"_ostr, "xy21"_ostr, "0");
    assertXPath(pDocument, "/primitive2D/transform/transform[2]"_ostr, "xy22"_ostr, "0");
    assertXPath(pDocument, "/primitive2D/transform/transform[2]"_ostr, "xy23"_ostr, "87");
    assertXPath(pDocument, "/primitive2D/transform/transform[2]"_ostr, "xy31"_ostr, "0");
    assertXPath(pDocument, "/primitive2D/transform/transform[2]"_ostr, "xy32"_ostr, "0");
    assertXPath(pDocument, "/primitive2D/transform/transform[2]"_ostr, "xy33"_ostr, "1");
}

CPPUNIT_TEST_FIXTURE(Test, testMarkerInPresentation)
{
    Primitive2DSequence aSequence = parseSvg(u"/svgio/qa/cppunit/data/markerInPresentation.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(aSequence);

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/transform/polypolygonstroke/line"_ostr, 1);
    assertXPath(pDocument, "/primitive2D/transform/transform/polypolygonstroke/polypolygon/polygon"_ostr, 1);
    assertXPath(pDocument, "/primitive2D/transform/transform/polypolygonstroke/polypolygon/polygon"_ostr, 1);

    // Without the fix in place, this test would have failed with
    // - Expected: 0
    // - Actual  : 2
    assertXPath(pDocument, "/primitive2D/transform/transform/transform"_ostr, 0);
}

CPPUNIT_TEST_FIXTURE(Test, testMarkerInCssStyle)
{
    Primitive2DSequence aSequence = parseSvg(u"/svgio/qa/cppunit/data/markerInCssStyle.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(aSequence);

    CPPUNIT_ASSERT (pDocument);

    // Without the fix in place, this test would have failed with
    // - Expected: 20
    // - Actual  : 0
    assertXPath(pDocument, "/primitive2D/transform/transform/polypolygonstroke/line"_ostr, 20);

    assertXPath(pDocument, "/primitive2D/transform/transform[1]/polypolygonstroke/line"_ostr, "color"_ostr, "#008000");
    assertXPath(pDocument, "/primitive2D/transform/transform[1]/polypolygonstroke/line"_ostr, "width"_ostr, "1");
    assertXPath(pDocument, "/primitive2D/transform/transform[1]/polypolygonstroke/line"_ostr, "linejoin"_ostr, "Miter");
    assertXPath(pDocument, "/primitive2D/transform/transform[1]/polypolygonstroke/line"_ostr, "miterangle"_ostr, "28");
    assertXPath(pDocument, "/primitive2D/transform/transform[1]/polypolygonstroke/line"_ostr, "linecap"_ostr, "BUTT");
}

CPPUNIT_TEST_FIXTURE(Test, testTextXmlSpace)
{
    //Check tspan fontsize when using relative units
    Primitive2DSequence aSequenceTdf97941 = parseSvg(u"/svgio/qa/cppunit/data/textXmlSpace.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequenceTdf97941.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(aSequenceTdf97941);

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/mask/textsimpleportion[1]"_ostr, "text"_ostr, "a b");
    assertXPath(pDocument, "/primitive2D/transform/mask/textsimpleportion[2]"_ostr, "text"_ostr, "a b");
    assertXPath(pDocument, "/primitive2D/transform/mask/textsimpleportion[3]"_ostr, "text"_ostr, "a b");
    assertXPath(pDocument, "/primitive2D/transform/mask/textsimpleportion[4]"_ostr, "text"_ostr, "ab");
    assertXPath(pDocument, "/primitive2D/transform/mask/textsimpleportion[5]"_ostr, "text"_ostr, " a  b ");
    assertXPath(pDocument, "/primitive2D/transform/mask/textsimpleportion[6]"_ostr, "text"_ostr, "a b");
    assertXPath(pDocument, "/primitive2D/transform/mask/textsimpleportion[7]"_ostr, "text"_ostr, "a   b");
    assertXPath(pDocument, "/primitive2D/transform/mask/textsimpleportion[8]"_ostr, "text"_ostr, "a b");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf45771)
{
    //Check text fontsize when using relative units
    Primitive2DSequence aSequenceTdf45771 = parseSvg(u"/svgio/qa/cppunit/data/tdf45771.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequenceTdf45771.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(aSequenceTdf45771);

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "fontcolor"_ostr, "#000000");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "text"_ostr, "Sample");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "height"_ostr, "32");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "familyname"_ostr, "Times New Roman");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf155833)
{
    Primitive2DSequence aSequence = parseSvg(u"/svgio/qa/cppunit/data/tdf155833.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(aSequence);

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/mask/transform/transform/transform/transform/transform/bitmap"_ostr, 1);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf97941)
{
    //Check tspan fontsize when using relative units
    Primitive2DSequence aSequenceTdf97941 = parseSvg(u"/svgio/qa/cppunit/data/tdf97941.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequenceTdf97941.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(aSequenceTdf97941);

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "fontcolor"_ostr, "#000000");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "text"_ostr, "Sample");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "height"_ostr, "48");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "familyname"_ostr, "Times New Roman");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf156777)
{
    Primitive2DSequence aSequence = parseSvg(u"/svgio/qa/cppunit/data/tdf156777.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequence));

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion"_ostr, 23);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "text"_ostr, "Quick brown fox jumps over the lazy dog.");

    // Without the fix in place, this test would have failed with
    // - Expected: #008000
    // - Actual  : #000000
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "fontcolor"_ostr, "#008000");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "x"_ostr, "84");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "y"_ostr, "23");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf156834)
{
    Primitive2DSequence aSequence = parseSvg(u"/svgio/qa/cppunit/data/tdf156834.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequence));

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion"_ostr, 4);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "text"_ostr, "Auto");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "x"_ostr, "30");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "y"_ostr, "20");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "text"_ostr, "Middle");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "x"_ostr, "30");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "y"_ostr, "56");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[3]"_ostr, "text"_ostr, "Hanging");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[3]"_ostr, "x"_ostr, "30");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[3]"_ostr, "y"_ostr, "94");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[4]"_ostr, "text"_ostr, "Central");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[4]"_ostr, "x"_ostr, "30");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[4]"_ostr, "y"_ostr, "116");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf104339)
{
    Primitive2DSequence aSequenceTdf104339 = parseSvg(u"/svgio/qa/cppunit/data/tdf104339.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequenceTdf104339.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(aSequenceTdf104339);

    CPPUNIT_ASSERT (pDocument);
    assertXPath(pDocument, "/primitive2D/transform/transform/transform/transform/transform/polypolygoncolor"_ostr, "color"_ostr, "#000000");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf85770)
{
    Primitive2DSequence aSequenceTdf85770 = parseSvg(u"/svgio/qa/cppunit/data/tdf85770.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequenceTdf85770.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequenceTdf85770));

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "fontcolor"_ostr, "#000000");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "text"_ostr, "Start Middle End");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "height"_ostr, "11");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "familyname"_ostr, "Times New Roman");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "fontcolor"_ostr, "#000000");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "text"_ostr, "Start");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "height"_ostr, "11");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "familyname"_ostr, "Times New Roman");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[3]"_ostr, "fontcolor"_ostr, "#000000");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[3]"_ostr, "text"_ostr, " End");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[3]"_ostr, "height"_ostr, "11");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[3]"_ostr, "familyname"_ostr, "Times New Roman");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf86938)
{
    Primitive2DSequence aSequence = parseSvg(u"/svgio/qa/cppunit/data/tdf86938.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequence));

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "text"_ostr, "line");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "x"_ostr, "290");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "y"_ostr, "183");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "text"_ostr, "above");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "x"_ostr, "290");

    // Without the fix in place, this test would have failed with
    // - Expected: 159
    // - Actual  : 207
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "y"_ostr, "159");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[3]"_ostr, "text"_ostr, "below");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[3]"_ostr, "x"_ostr, "290");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[3]"_ostr, "y"_ostr, "207");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf93583)
{
    Primitive2DSequence aSequence = parseSvg(u"/svgio/qa/cppunit/data/tdf93583.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequence));

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "text"_ostr, "This is the");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "x"_ostr, "62");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "y"_ostr, "303");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "width"_ostr, "16");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "height"_ostr, "16");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "text"_ostr, " first");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "x"_ostr, "127");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "y"_ostr, "303");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "width"_ostr, "32");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "height"_ostr, "32");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[3]"_ostr, "text"_ostr, " line");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[3]"_ostr, "x"_ostr, "187");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[3]"_ostr, "y"_ostr, "303");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[3]"_ostr, "width"_ostr, "16");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[3]"_ostr, "height"_ostr, "16");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf156616)
{
    Primitive2DSequence aSequence = parseSvg(u"/svgio/qa/cppunit/data/tdf156616.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequence));

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "text"_ostr, "First");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "x"_ostr, "114");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "y"_ostr, "103");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "text"_ostr, " line");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "x"_ostr, "142");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "y"_ostr, "103");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[3]"_ostr, "text"_ostr, "Second line");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[3]"_ostr, "x"_ostr, "114");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[3]"_ostr, "y"_ostr, "122");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[4]"_ostr, "text"_ostr, "First");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[4]"_ostr, "x"_ostr, "86");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[4]"_ostr, "y"_ostr, "153");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[5]"_ostr, "text"_ostr, " line");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[5]"_ostr, "x"_ostr, "114");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[5]"_ostr, "y"_ostr, "153");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[6]"_ostr, "text"_ostr, "Second line");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[6]"_ostr, "x"_ostr, "77");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[6]"_ostr, "y"_ostr, "172");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[7]"_ostr, "text"_ostr, "First");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[7]"_ostr, "x"_ostr, "59");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[7]"_ostr, "y"_ostr, "203");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[8]"_ostr, "text"_ostr, " line");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[8]"_ostr, "x"_ostr, "87");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[8]"_ostr, "y"_ostr, "203");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[9]"_ostr, "text"_ostr, "Second line");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[9]"_ostr, "x"_ostr, "40");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[9]"_ostr, "y"_ostr, "222");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf79163)
{
    //Check Opacity
    Primitive2DSequence aSequenceTdf79163 = parseSvg(u"/svgio/qa/cppunit/data/tdf79163.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequenceTdf79163.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequenceTdf79163));

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/unifiedtransparence"_ostr, "transparence"_ostr, "50");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf97542_1)
{
    Primitive2DSequence aSequenceTdf97542_1 = parseSvg(u"/svgio/qa/cppunit/data/tdf97542_1.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequenceTdf97542_1.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequenceTdf97542_1));

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/objectinfo/textsimpleportion"_ostr, "fontcolor"_ostr, "#ffff00");
    assertXPath(pDocument, "/primitive2D/transform/objectinfo/textsimpleportion"_ostr, "text"_ostr, "Text");
    assertXPath(pDocument, "/primitive2D/transform/objectinfo/textsimpleportion"_ostr, "height"_ostr, "48");
    assertXPath(pDocument, "/primitive2D/transform/objectinfo/textsimpleportion"_ostr, "familyname"_ostr, "DejaVu Serif");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf97542_2)
{
    Primitive2DSequence aSequenceTdf97542_2 = parseSvg(u"/svgio/qa/cppunit/data/tdf97542_2.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequenceTdf97542_2.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequenceTdf97542_2));

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/objectinfo/svgradialgradient"_ostr, "startx"_ostr, "1");
    assertXPath(pDocument, "/primitive2D/transform/objectinfo/svgradialgradient"_ostr, "starty"_ostr, "1");
    assertXPath(pDocument, "/primitive2D/transform/objectinfo/svgradialgradient/focalx"_ostr, 0);
    assertXPath(pDocument, "/primitive2D/transform/objectinfo/svgradialgradient/focaly"_ostr, 0);
    assertXPath(pDocument, "/primitive2D/transform/objectinfo/svgradialgradient"_ostr, "radius"_ostr, "3");
    assertXPath(pDocument, "/primitive2D/transform/objectinfo/svgradialgradient"_ostr, "spreadmethod"_ostr, "pad");
    assertXPath(pDocument, "/primitive2D/transform/objectinfo/svgradialgradient"_ostr, "opacity"_ostr, "1");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf97543)
{
    // check visibility="inherit"
    Primitive2DSequence aSequenceTdf97543 = parseSvg(u"/svgio/qa/cppunit/data/tdf97543.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequenceTdf97543.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequenceTdf97543));

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor"_ostr, "color"_ostr, "#00cc00");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor/polypolygon"_ostr, "height"_ostr, "100");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor/polypolygon"_ostr, "width"_ostr, "100");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor/polypolygon"_ostr, "minx"_ostr, "10");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor/polypolygon"_ostr, "miny"_ostr, "10");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor/polypolygon"_ostr, "maxx"_ostr, "110");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor/polypolygon"_ostr, "maxy"_ostr, "110");
}

CPPUNIT_TEST_FIXTURE(Test, testRGBColor)
{
    Primitive2DSequence aSequenceRGBColor = parseSvg(u"/svgio/qa/cppunit/data/RGBColor.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequenceRGBColor.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequenceRGBColor));

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor"_ostr, "color"_ostr, "#646464");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor/polypolygon"_ostr, "height"_ostr, "100");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor/polypolygon"_ostr, "width"_ostr, "100");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor/polypolygon"_ostr, "minx"_ostr, "10");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor/polypolygon"_ostr, "miny"_ostr, "10");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor/polypolygon"_ostr, "maxx"_ostr, "110");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor/polypolygon"_ostr, "maxy"_ostr, "110");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf149673)
{
    Primitive2DSequence aSequence = parseSvg(u"/svgio/qa/cppunit/data/tdf149673.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequence));

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/unifiedtransparence"_ostr, "transparence"_ostr, "90");
    assertXPath(pDocument, "/primitive2D/transform/unifiedtransparence/polypolygoncolor[1]"_ostr, "color"_ostr, "#ff0000");
    assertXPath(pDocument, "/primitive2D/transform/unifiedtransparence/polypolygoncolor[2]"_ostr, "color"_ostr, "#00ff00");
    assertXPath(pDocument, "/primitive2D/transform/unifiedtransparence/polypolygoncolor[3]"_ostr, "color"_ostr, "#0000ff");
}

CPPUNIT_TEST_FIXTURE(Test, testRGBAColor)
{
    Primitive2DSequence aSequenceRGBAColor = parseSvg(u"/svgio/qa/cppunit/data/RGBAColor.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequenceRGBAColor.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequenceRGBAColor));

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/unifiedtransparence"_ostr, "transparence"_ostr, "50");
}

CPPUNIT_TEST_FIXTURE(Test, testNoneColor)
{
    Primitive2DSequence aSequenceRGBAColor = parseSvg(u"/svgio/qa/cppunit/data/noneColor.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequenceRGBAColor.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequenceRGBAColor));

    CPPUNIT_ASSERT (pDocument);

    //No polypolygoncolor exists
    assertXPath(pDocument, "/primitive2D/transform/mask/polypolygoncolor"_ostr, 0);
    assertXPath(pDocument, "/primitive2D/transform/mask/polypolygonstroke/line"_ostr, "color"_ostr, "#000000");
    assertXPath(pDocument, "/primitive2D/transform/mask/polypolygonstroke/line"_ostr, "width"_ostr, "3");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf117920)
{
    Primitive2DSequence aSequence = parseSvg(u"/svgio/qa/cppunit/data/tdf117920.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequence));

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/transform"_ostr, "xy11"_ostr, "1");
    assertXPath(pDocument, "/primitive2D/transform/transform"_ostr, "xy12"_ostr, "0");
    assertXPath(pDocument, "/primitive2D/transform/transform"_ostr, "xy13"_ostr, "-18");
    assertXPath(pDocument, "/primitive2D/transform/transform"_ostr, "xy21"_ostr, "0");
    assertXPath(pDocument, "/primitive2D/transform/transform"_ostr, "xy22"_ostr, "1");
    assertXPath(pDocument, "/primitive2D/transform/transform"_ostr, "xy23"_ostr, "-6");
    assertXPath(pDocument, "/primitive2D/transform/transform"_ostr, "xy31"_ostr, "0");
    assertXPath(pDocument, "/primitive2D/transform/transform"_ostr, "xy32"_ostr, "0");
    assertXPath(pDocument, "/primitive2D/transform/transform"_ostr, "xy33"_ostr, "1");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf97936)
{
    // check that both rectangles are rendered in the viewBox
    Primitive2DSequence aSequenceTdf97936 = parseSvg(u"/svgio/qa/cppunit/data/tdf97936.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequenceTdf97936.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequenceTdf97936));

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[1]"_ostr);
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[1]/polypolygon"_ostr, "height"_ostr, "50");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[1]/polypolygon"_ostr, "width"_ostr, "50");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[1]/polypolygon"_ostr, "minx"_ostr, "70");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[1]/polypolygon"_ostr, "miny"_ostr, "50");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[1]/polypolygon"_ostr, "maxx"_ostr, "120");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[1]/polypolygon"_ostr, "maxy"_ostr, "100");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[2]"_ostr);
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[2]/polypolygon"_ostr, "height"_ostr, "50");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[2]/polypolygon"_ostr, "width"_ostr, "50");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[2]/polypolygon"_ostr, "minx"_ostr, "10");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[2]/polypolygon"_ostr, "miny"_ostr, "50");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[2]/polypolygon"_ostr, "maxx"_ostr, "60");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[2]/polypolygon"_ostr, "maxy"_ostr, "100");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf149893)
{
    Primitive2DSequence aSequence = parseSvg(u"/svgio/qa/cppunit/data/tdf149893.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequence));

    CPPUNIT_ASSERT (pDocument);

    // Without the fix in place, this test would have failed with
    // - Expected: #008000
    // - Actual  : #000000
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor"_ostr, "color"_ostr, "#008000");
}

CPPUNIT_TEST_FIXTURE(Test, testShapeWithClipPathAndCssStyle)
{
    // tdf#97539: Check there is a mask and 3 polygons
    Primitive2DSequence aSequenceClipPathAndStyle = parseSvg(u"/svgio/qa/cppunit/data/ShapeWithClipPathAndCssStyle.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequenceClipPathAndStyle.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequenceClipPathAndStyle));

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/mask/polypolygon/polygon"_ostr, 2);
    assertXPath(pDocument, "/primitive2D/transform/mask/polypolygoncolor/polypolygon/polygon"_ostr, 1);
}

CPPUNIT_TEST_FIXTURE(Test, testClipPathAndParentStyle)
{
    //Check that fill color, stroke color and stroke-width are inherited from use element
    //when the element is within a clipPath element
    Primitive2DSequence aSequenceClipPathAndParentStyle = parseSvg(u"/svgio/qa/cppunit/data/ClipPathAndParentStyle.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequenceClipPathAndParentStyle.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequenceClipPathAndParentStyle));

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/transform/polypolygoncolor"_ostr, "color"_ostr, "#ff0000");
    assertXPath(pDocument, "/primitive2D/transform/transform/polypolygonstroke/line"_ostr, "color"_ostr, "#000000");
    assertXPath(pDocument, "/primitive2D/transform/transform/polypolygonstroke/line"_ostr, "width"_ostr, "5");

}

CPPUNIT_TEST_FIXTURE(Test, testTdf155814)
{
    Primitive2DSequence aSequence = parseSvg(u"/svgio/qa/cppunit/data/tdf155814.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequence));

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/mask/mask/transform/unifiedtransparence"_ostr, "transparence"_ostr, "50");
    assertXPath(pDocument, "/primitive2D/transform/mask/mask/transform/unifiedtransparence/polypolygoncolor"_ostr, "color"_ostr, "#0000ff");
}

CPPUNIT_TEST_FIXTURE(Test, testClipPathAndStyle)
{
    //Check that fill color, stroke color and stroke-width are inherited from use element
    //when the element is within a clipPath element
    Primitive2DSequence aSequenceClipPathAndStyle = parseSvg(u"/svgio/qa/cppunit/data/ClipPathAndStyle.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequenceClipPathAndStyle.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequenceClipPathAndStyle));

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/transform/polypolygoncolor"_ostr, "color"_ostr, "#ccccff");
    assertXPath(pDocument, "/primitive2D/transform/transform/polypolygonstroke/line"_ostr, "color"_ostr, "#0000cc");
    assertXPath(pDocument, "/primitive2D/transform/transform/polypolygonstroke/line"_ostr, "width"_ostr, "2");

}

CPPUNIT_TEST_FIXTURE(Test, testShapeWithClipPath)
{
    // Check there is a mask and 3 polygons
    Primitive2DSequence aSequenceClipPathAndStyle = parseSvg(u"/svgio/qa/cppunit/data/ShapeWithClipPath.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequenceClipPathAndStyle.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequenceClipPathAndStyle));

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/mask/polypolygon/polygon"_ostr, 2);
    assertXPath(pDocument, "/primitive2D/transform/mask/polypolygoncolor/polypolygon/polygon"_ostr, 1);
}

CPPUNIT_TEST_FIXTURE(Test, testClipPathUsingClipPath)
{
    Primitive2DSequence aSequenceClipPathAndStyle = parseSvg(u"/svgio/qa/cppunit/data/ClipPathUsingClipPath.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequenceClipPathAndStyle.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequenceClipPathAndStyle));

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/mask/polypolygon/polygon/point"_ostr, 20);
    assertXPath(pDocument, "/primitive2D/transform/mask/mask/polypolygon/polygon/point"_ostr, 13);
}

CPPUNIT_TEST_FIXTURE(Test, testFillRule)
{
    Primitive2DSequence aSequence = parseSvg(u"/svgio/qa/cppunit/data/FillRule.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequence));

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor"_ostr, "color"_ostr, "#000000");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor/polypolygon/polygon"_ostr, 2);
    assertXPath(pDocument, "/primitive2D/transform/polypolygonstroke/line"_ostr, "color"_ostr, "#ff0000");
    assertXPath(pDocument, "/primitive2D/transform/polypolygonstroke/polypolygon/polygon"_ostr, 2);
}

CPPUNIT_TEST_FIXTURE(Test, testClipRule)
{
    Primitive2DSequence aSequence = parseSvg(u"/svgio/qa/cppunit/data/ClipRule.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequence));

    CPPUNIT_ASSERT (pDocument);

    // Without the place in place, this test would have failed with
    // - Expected: 5
    // - Actual  : 10
    assertXPath(pDocument, "/primitive2D/transform/mask[1]/polypolygon/polygon/point"_ostr, 5);
    assertXPath(pDocument, "/primitive2D/transform/mask[1]/polypolygoncolor"_ostr, "color"_ostr, "#0000ff");
    assertXPath(pDocument, "/primitive2D/transform/mask[1]/polypolygoncolor/polypolygon/polygon/point"_ostr, 4);

    assertXPath(pDocument, "/primitive2D/transform/mask[2]/polypolygon/polygon/point"_ostr, 5);
    assertXPath(pDocument, "/primitive2D/transform/mask[2]/polypolygoncolor"_ostr, "color"_ostr, "#ff0000");
    assertXPath(pDocument, "/primitive2D/transform/mask[2]/polypolygoncolor/polypolygon/polygon/point"_ostr, 4);
}

CPPUNIT_TEST_FIXTURE(Test, testi125329)
{
    //Check style inherit from * css element
    Primitive2DSequence aSequencei125329 = parseSvg(u"/svgio/qa/cppunit/data/i125329.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequencei125329.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequencei125329));

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/transform/objectinfo/polypolygoncolor"_ostr, "color"_ostr, "#c0c0c0"); // rect background color
    assertXPath(pDocument, "/primitive2D/transform/transform/objectinfo/polypolygoncolor/polypolygon"_ostr, "height"_ostr, "30"); // rect background height
    assertXPath(pDocument, "/primitive2D/transform/transform/objectinfo/polypolygoncolor/polypolygon"_ostr, "width"_ostr, "50"); // rect background width
    assertXPath(pDocument, "/primitive2D/transform/transform/objectinfo/polypolygoncolor/polypolygon"_ostr, "minx"_ostr, "15");
    assertXPath(pDocument, "/primitive2D/transform/transform/objectinfo/polypolygoncolor/polypolygon"_ostr, "miny"_ostr, "15");
    assertXPath(pDocument, "/primitive2D/transform/transform/objectinfo/polypolygoncolor/polypolygon"_ostr, "maxx"_ostr, "65");
    assertXPath(pDocument, "/primitive2D/transform/transform/objectinfo/polypolygoncolor/polypolygon"_ostr, "maxy"_ostr, "45");
    assertXPath(pDocument, "/primitive2D/transform/transform/objectinfo/polypolygonstroke/line"_ostr, "color"_ostr, "#008000"); // rect stroke color
    assertXPath(pDocument, "/primitive2D/transform/transform/objectinfo/polypolygonstroke/line"_ostr, "width"_ostr, "1"); // rect stroke width
}

CPPUNIT_TEST_FIXTURE(Test, testMaskingPath07b)
{
    //For the time being, check that masking-path-07-b.svg can be imported and it doesn't hang on loading
    //it used to hang after d5649ae7b76278cb3155f951d6327157c7c92b65
    Primitive2DSequence aSequenceMaskingPath07b = parseSvg(u"/svgio/qa/cppunit/data/masking-path-07-b.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequenceMaskingPath07b.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequenceMaskingPath07b));

    CPPUNIT_ASSERT (pDocument);

}

CPPUNIT_TEST_FIXTURE(Test, test123926)
{
    Primitive2DSequence aSequence123926 = parseSvg(u"/svgio/qa/cppunit/data/tdf123926.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence123926.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequence123926));

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/transform/transform/unifiedtransparence/polypolygoncolor"_ostr, "color"_ostr, "#7cb5ec");
}

CPPUNIT_TEST_FIXTURE(Test, test47446)
{
    //Check that marker's fill attribute is black is not set
    Primitive2DSequence aSequence47446 = parseSvg(u"/svgio/qa/cppunit/data/47446.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence47446.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequence47446));

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/transform/transform/polypolygoncolor"_ostr, "color"_ostr, "#000000");

}

CPPUNIT_TEST_FIXTURE(Test, test47446b)
{
    //Check that marker's fill attribute is inherit from def
    Primitive2DSequence aSequence47446b = parseSvg(u"/svgio/qa/cppunit/data/47446b.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence47446b.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequence47446b));

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/transform/transform/polypolygoncolor"_ostr, "color"_ostr, "#ffff00");

}

CPPUNIT_TEST_FIXTURE(Test, testTdf103888)
{
    Primitive2DSequence aSequenceMaskText = parseSvg(u"/svgio/qa/cppunit/data/tdf103888.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequenceMaskText.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequenceMaskText));

    CPPUNIT_ASSERT (pDocument);

    // Without the fix in place, this test would have failed here with number of nodes is incorrect
    assertXPath(pDocument, "/primitive2D/transform/transform/textsimpleportion[1]"_ostr, "text"_ostr, "Her");
    assertXPath(pDocument, "/primitive2D/transform/transform/textsimpleportion[2]"_ostr, "text"_ostr, "vor");
    assertXPath(pDocument, "/primitive2D/transform/transform/textsimpleportion[3]"_ostr, "text"_ostr, "hebung");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf156251)
{
    Primitive2DSequence aSequence = parseSvg(u"/svgio/qa/cppunit/data/tdf156251.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequence));

    CPPUNIT_ASSERT (pDocument);

    // Without the fix in place, this test would have failed with
    // - Expected: 'You are '
    // - Actual  : 'You are'
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "text"_ostr, "You are");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "text"_ostr, " not");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[3]"_ostr, "text"_ostr, " a banana!");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[4]"_ostr, "text"_ostr, "You are");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[5]"_ostr, "text"_ostr, " not");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[6]"_ostr, "text"_ostr, " a banana!");
}

CPPUNIT_TEST_FIXTURE(Test, testMaskText)
{
    //Check that mask is applied on text
    Primitive2DSequence aSequenceMaskText = parseSvg(u"/svgio/qa/cppunit/data/maskText.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequenceMaskText.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequenceMaskText));

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/transform/polypolygoncolor"_ostr, "color"_ostr, "#000000");
    assertXPath(pDocument, "/primitive2D/transform/transform/transform/textsimpleportion"_ostr, "fontcolor"_ostr, "#ffffff");
    assertXPath(pDocument, "/primitive2D/transform/transform/transform/textsimpleportion"_ostr, "text"_ostr, "Black White");
    assertXPath(pDocument, "/primitive2D/transform/transform/transform/textsimpleportion"_ostr, "height"_ostr, "26");
    assertXPath(pDocument, "/primitive2D/transform/transform/transform/textsimpleportion"_ostr, "familyname"_ostr, "Times New Roman");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf99994)
{
    //Check text fontsize when using relative units
    Primitive2DSequence aSequenceTdf99994 = parseSvg(u"/svgio/qa/cppunit/data/tdf99994.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequenceTdf99994.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequenceTdf99994));

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "fontcolor"_ostr, "#0000ff");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "height"_ostr, "16");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "text"_ostr, "test");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "familyname"_ostr, "DejaVu Sans");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf99115)
{
    //Check that styles are resolved correctly where there is a * css selector
    Primitive2DSequence aSequenceTdf99115 = parseSvg(u"/svgio/qa/cppunit/data/tdf99115.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequenceTdf99115.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequenceTdf99115) );

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "text"_ostr, "red 1");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "fontcolor"_ostr, "#ff0000");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "height"_ostr, "18");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "text"_ostr, "red 2");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "fontcolor"_ostr, "#ff0000");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "height"_ostr, "18");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[3]"_ostr, "text"_ostr, "red 3");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[3]"_ostr, "fontcolor"_ostr, "#ff0000");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[3]"_ostr, "height"_ostr, "18");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[4]"_ostr, "text"_ostr, "blue 4");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[4]"_ostr, "fontcolor"_ostr, "#0000ff");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[4]"_ostr, "height"_ostr, "18");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[5]"_ostr, "text"_ostr, "blue 5");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[5]"_ostr, "fontcolor"_ostr, "#0000ff");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[5]"_ostr, "height"_ostr, "18");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[6]"_ostr, "text"_ostr, "blue 6");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[6]"_ostr, "fontcolor"_ostr, "#0000ff");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[6]"_ostr, "height"_ostr, "18");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[7]"_ostr, "text"_ostr, "green 7");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[7]"_ostr, "fontcolor"_ostr, "#008000");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[7]"_ostr, "height"_ostr, "18");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[8]"_ostr, "text"_ostr, "green 8");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[8]"_ostr, "fontcolor"_ostr, "#008000");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[8]"_ostr, "height"_ostr, "18");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[9]"_ostr, "text"_ostr, "green 9");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[9]"_ostr, "fontcolor"_ostr, "#008000");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[9]"_ostr, "height"_ostr, "18");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf101237)
{
    //Check that fill color, stroke color and stroke-width are inherited from use element
    //when the element is within a clipPath element
    Primitive2DSequence aSequenceTdf101237 = parseSvg(u"/svgio/qa/cppunit/data/tdf101237.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequenceTdf101237.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequenceTdf101237));

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor"_ostr, "color"_ostr, "#ff0000");
    assertXPath(pDocument, "/primitive2D/transform/polypolygonstroke/line"_ostr, "color"_ostr, "#000000");
    assertXPath(pDocument, "/primitive2D/transform/polypolygonstroke/line"_ostr, "width"_ostr, "5");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf97710)
{
    Primitive2DSequence aSequence = parseSvg(u"/svgio/qa/cppunit/data/tdf97710.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequence));

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/mask/polypolygoncolor[1]"_ostr, "color"_ostr, "#000000");

    // Without the fix in place, this test would have failed with
    // - Expected: 100
    // - Actual  : 0
    assertXPath(pDocument, "/primitive2D/transform/mask/polypolygoncolor[1]/polypolygon"_ostr, "width"_ostr, "100");
    assertXPath(pDocument, "/primitive2D/transform/mask/polypolygoncolor[1]/polypolygon"_ostr, "height"_ostr, "100");
    assertXPath(pDocument, "/primitive2D/transform/mask/polypolygoncolor[2]"_ostr, "color"_ostr, "#008000");
    assertXPath(pDocument, "/primitive2D/transform/mask/polypolygoncolor[2]/polypolygon"_ostr, "width"_ostr, "100");
    assertXPath(pDocument, "/primitive2D/transform/mask/polypolygoncolor[2]/polypolygon"_ostr, "height"_ostr, "100");
    assertXPath(pDocument, "/primitive2D/transform/mask/polypolygonstroke/line"_ostr, "color"_ostr, "#000000");
    assertXPath(pDocument, "/primitive2D/transform/mask/polypolygonstroke/line"_ostr, "width"_ostr, "1");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf94765)
{
    Primitive2DSequence aSequenceTdf94765 = parseSvg(u"/svgio/qa/cppunit/data/tdf94765.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequenceTdf94765.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequenceTdf94765));

    CPPUNIT_ASSERT (pDocument);

    //Check that both rectangles use the gradient as fill
    assertXPath(pDocument, "/primitive2D/transform/transform/svglineargradient[1]"_ostr, "startx"_ostr, "1");
    assertXPath(pDocument, "/primitive2D/transform/transform/svglineargradient[1]"_ostr, "starty"_ostr, "1");
    assertXPath(pDocument, "/primitive2D/transform/transform/svglineargradient[1]"_ostr, "endx"_ostr, "2");
    assertXPath(pDocument, "/primitive2D/transform/transform/svglineargradient[1]"_ostr, "endy"_ostr, "1");
    assertXPath(pDocument, "/primitive2D/transform/transform/svglineargradient[2]"_ostr, "startx"_ostr, "0");
    assertXPath(pDocument, "/primitive2D/transform/transform/svglineargradient[2]"_ostr, "starty"_ostr, "0");
    assertXPath(pDocument, "/primitive2D/transform/transform/svglineargradient[2]"_ostr, "endx"_ostr, "0");
    assertXPath(pDocument, "/primitive2D/transform/transform/svglineargradient[2]"_ostr, "endy"_ostr, "0");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf156236)
{
    Primitive2DSequence aSequenceTdf94765 = parseSvg(u"/svgio/qa/cppunit/data/tdf156236.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequenceTdf94765.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequenceTdf94765));

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[1]/polypolygon"_ostr, "path"_ostr, "m50 180h-30v-60h60v60z");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[2]/polypolygon"_ostr, "path"_ostr, "m150 180h15c8.2842712474619 0 15-6.7157287525381 15-15v-30c0-8.2842712474619-6.7157287525381-15-15-15h-30c-8.2842712474619 0-15 6.7157287525381-15 15v30c0 8.2842712474619 6.7157287525381 15 15 15z");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[3]/polypolygon"_ostr, "path"_ostr, "m250 180h15c8.2842712474619 0 15-6.7157287525381 15-15v-30c0-8.2842712474619-6.7157287525381-15-15-15h-30c-8.2842712474619 0-15 6.7157287525381-15 15v30c0 8.2842712474619 6.7157287525381 15 15 15z");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[4]/polypolygon"_ostr, "path"_ostr, "m350 180c16.5685424949238 0 30-6.7157287525381 30-15v-30c0-8.2842712474619-13.4314575050762-15-30-15s-30 6.7157287525381-30 15v30c0 8.2842712474619 13.4314575050762 15 30 15z");
}

CPPUNIT_TEST_FIXTURE(Test, testBehaviourWhenWidthAndHeightIsOrIsNotSet)
{
    // This test checks the behaviour when width and height attributes
    // are and are not set. In both cases the result must be the same,
    // however if the width / height are set, then the size of the image
    // is enforced, but this isn't really possible in LibreOffice (or
    // maybe we could lock the size in this case).
    // The behaviour in browsers is that when a SVG image has width / height
    // attributes set, then the image is shown with that size, but if it
    // isn't set then it is shown as scalable image which is the size of
    // the container.

    {
        const Primitive2DSequence aSequence = parseSvg(u"svgio/qa/cppunit/data/Drawing_WithWidthHeight.svg");
        CPPUNIT_ASSERT(aSequence.hasElements());

        geometry::RealRectangle2D aRealRect;
        basegfx::B2DRange aRange;
        uno::Sequence<beans::PropertyValue> aViewParameters;

        for (css::uno::Reference<css::graphic::XPrimitive2D> const & xReference : aSequence)
        {
            if (xReference.is())
            {
                aRealRect = xReference->getRange(aViewParameters);
                aRange.expand(basegfx::B2DRange(aRealRect.X1, aRealRect.Y1, aRealRect.X2, aRealRect.Y2));
            }
        }

        double fWidth = (aRange.getWidth() / 2540.0) * 96.0;
        double fHeight = (aRange.getHeight() / 2540.0) * 96.0;

        CPPUNIT_ASSERT_DOUBLES_EQUAL(11.0, fWidth, 1E-12);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(11.0, fHeight, 1E-12);
    }

    {
        const Primitive2DSequence aSequence = parseSvg(u"svgio/qa/cppunit/data/Drawing_NoWidthHeight.svg");
        CPPUNIT_ASSERT(aSequence.hasElements());


        geometry::RealRectangle2D aRealRect;
        basegfx::B2DRange aRange;
        uno::Sequence<beans::PropertyValue> aViewParameters;

        for (css::uno::Reference<css::graphic::XPrimitive2D> const & xReference : aSequence)
        {
            if (xReference.is())
            {
                aRealRect = xReference->getRange(aViewParameters);
                aRange.expand(basegfx::B2DRange(aRealRect.X1, aRealRect.Y1, aRealRect.X2, aRealRect.Y2));
            }
        }

        double fWidth = (aRange.getWidth() / 2540.0) * 96.0;
        double fHeight = (aRange.getHeight() / 2540.0) * 96.0;

        CPPUNIT_ASSERT_DOUBLES_EQUAL(11.0, fWidth, 1E-12);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(11.0, fHeight, 1E-12);
    }
}

CPPUNIT_TEST_FIXTURE(Test, testTdf155733)
{
    Primitive2DSequence aSequence = parseSvg(u"/svgio/qa/cppunit/data/tdf155733.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequence));

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/softedge"_ostr, "radius"_ostr, "5");

    // Without the fix in place, the softedge would have been applied to the second element
    // - Expected: 1
    // - Actual  : 0
    assertXPath(pDocument, "/primitive2D/transform/transform/unifiedtransparence"_ostr, "transparence"_ostr, "50");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf158445)
{
    Primitive2DSequence aSequence = parseSvg(u"/svgio/qa/cppunit/data/tdf158445.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequence));

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/mask/transform/transform/transform/polypolygoncolor"_ostr, "color"_ostr, "#000000");
    assertXPath(pDocument, "/primitive2D/transform/mask/transform/transform/transform/polypolygoncolor/polypolygon"_ostr, "height"_ostr, "8.052");
    assertXPath(pDocument, "/primitive2D/transform/mask/transform/transform/transform/polypolygoncolor/polypolygon"_ostr, "width"_ostr, "5.328");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf159594)
{
    Primitive2DSequence aSequence = parseSvg(u"/svgio/qa/cppunit/data/tdf159594.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequence));

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/transform/polypolygoncolor"_ostr, "color"_ostr, "#000000");
    assertXPath(pDocument, "/primitive2D/transform/transform/polypolygoncolor/polypolygon"_ostr, "height"_ostr, "11.671875");
    assertXPath(pDocument, "/primitive2D/transform/transform/polypolygoncolor/polypolygon"_ostr, "width"_ostr, "7.5");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf97663)
{
    Primitive2DSequence aSequence = parseSvg(u"/svgio/qa/cppunit/data/em_units.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequence));

    CPPUNIT_ASSERT (pDocument);

    // tdf#97663: Without the fix in place, this test would have failed with
    // - Expected: 236
    // - Actual  : 204
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "y"_ostr, "236");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf156269)
{
    Primitive2DSequence aSequence = parseSvg(u"/svgio/qa/cppunit/data/tdf156269.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequence));

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "//textsimpleportion[@text='one']"_ostr, "width"_ostr, "16");
    assertXPath(pDocument, "//textsimpleportion[@text='one']"_ostr, "height"_ostr, "16");
    assertXPath(pDocument, "//textsimpleportion[@text='one']"_ostr, "x"_ostr, "10");
    assertXPath(pDocument, "//textsimpleportion[@text='one']"_ostr, "y"_ostr, "50");
    assertXPath(pDocument, "//textsimpleportion[@text='one']"_ostr, "fontcolor"_ostr, "#808080");

    assertXPath(pDocument, "//textsimpleportion[@text='two']"_ostr, "width"_ostr, "16");
    assertXPath(pDocument, "//textsimpleportion[@text='two']"_ostr, "height"_ostr, "16");

    // Without the fix in place, this test would have failed with
    // - Expected: 60
    // - Actual  : 10
    assertXPath(pDocument, "//textsimpleportion[@text='two']"_ostr, "x"_ostr, "60");
    assertXPath(pDocument, "//textsimpleportion[@text='two']"_ostr, "y"_ostr, "100");
    assertXPath(pDocument, "//textsimpleportion[@text='two']"_ostr, "fontcolor"_ostr, "#000000");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf95400)
{
    Primitive2DSequence aSequence = parseSvg(u"/svgio/qa/cppunit/data/tdf95400.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequence));

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "width"_ostr, "16");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "height"_ostr, "16");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "x"_ostr, "30");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "y"_ostr, "20");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "text"_ostr, "ABC");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "dx0"_ostr, "36");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "dx1"_ostr, "69");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "dx2"_ostr, "102");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "width"_ostr, "48");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "height"_ostr, "16");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "x"_ostr, "30");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "y"_ostr, "30");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "text"_ostr, "ABC");
    assertXPathNoAttribute(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "dx0"_ostr);
}

CPPUNIT_TEST_FIXTURE(Test, testTextAnchor)
{
    Primitive2DSequence aSequence = parseSvg(u"/svgio/qa/cppunit/data/tdf151103.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequence));

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "x"_ostr, "60");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "y"_ostr, "40");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "text"_ostr, "ABC");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "x"_ostr, "43");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "y"_ostr, "50");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "text"_ostr, "ABC");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[3]"_ostr, "x"_ostr, "26");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[3]"_ostr, "y"_ostr, "60");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[3]"_ostr, "text"_ostr, "ABC");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[4]"_ostr, "x"_ostr, "60");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[4]"_ostr, "y"_ostr, "40");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[4]"_ostr, "text"_ostr, "ABC");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[5]"_ostr, "x"_ostr, "43");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[5]"_ostr, "y"_ostr, "50");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[5]"_ostr, "text"_ostr, "ABC");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[6]"_ostr, "x"_ostr, "26");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[6]"_ostr, "y"_ostr, "60");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[6]"_ostr, "text"_ostr, "ABC");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[7]"_ostr, "x"_ostr, "60");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[7]"_ostr, "y"_ostr, "40");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[7]"_ostr, "text"_ostr, "ABC");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[8]"_ostr, "x"_ostr, "43");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[8]"_ostr, "y"_ostr, "50");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[8]"_ostr, "text"_ostr, "ABC");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[9]"_ostr, "x"_ostr, "26");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[9]"_ostr, "y"_ostr, "60");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[9]"_ostr, "text"_ostr, "ABC");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[10]"_ostr, "x"_ostr, "60");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[10]"_ostr, "y"_ostr, "40");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[10]"_ostr, "text"_ostr, "A");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[11]"_ostr, "x"_ostr, "72");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[11]"_ostr, "y"_ostr, "40");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[11]"_ostr, "text"_ostr, "B");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[12]"_ostr, "x"_ostr, "83");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[12]"_ostr, "y"_ostr, "40");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[12]"_ostr, "text"_ostr, "C");

    // Without the fix in place, this test would have failed with
    // - Expected: 43
    // - Actual  : 54
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[13]"_ostr, "x"_ostr, "43");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[13]"_ostr, "y"_ostr, "50");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[13]"_ostr, "text"_ostr, "A");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[14]"_ostr, "x"_ostr, "55");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[14]"_ostr, "y"_ostr, "50");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[14]"_ostr, "text"_ostr, "B");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[15]"_ostr, "x"_ostr, "66");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[15]"_ostr, "y"_ostr, "50");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[15]"_ostr, "text"_ostr, "C");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[16]"_ostr, "x"_ostr, "26");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[16]"_ostr, "y"_ostr, "60");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[16]"_ostr, "text"_ostr, "A");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[17]"_ostr, "x"_ostr, "38");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[17]"_ostr, "y"_ostr, "60");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[17]"_ostr, "text"_ostr, "B");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[18]"_ostr, "x"_ostr, "49");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[18]"_ostr, "y"_ostr, "60");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[18]"_ostr, "text"_ostr, "C");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf156577)
{
    Primitive2DSequence aSequence = parseSvg(u"/svgio/qa/cppunit/data/tdf156577.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequence));

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "width"_ostr, "16");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "height"_ostr, "16");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "x"_ostr, "30");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "y"_ostr, "20");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "text"_ostr, "ABC");
    assertXPathNoAttribute(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "dx0"_ostr);

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "width"_ostr, "16");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "height"_ostr, "16");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "x"_ostr, "30");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "y"_ostr, "30");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "text"_ostr, "ABC");

    // Without the fix in place, this test would have failed with
    // - Expected: 22
    // - Actual  : 52
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "dx0"_ostr, "22");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "dx1"_ostr, "53");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "dx2"_ostr, "94");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf156283)
{
    Primitive2DSequence aSequence = parseSvg(u"/svgio/qa/cppunit/data/tdf156283.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequence));

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "width"_ostr, "16");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "height"_ostr, "16");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "x"_ostr, "30");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "y"_ostr, "20");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "text"_ostr, "ABC");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "dx0"_ostr, "41");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "dx1"_ostr, "52");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "dx2"_ostr, "63");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "width"_ostr, "16");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "height"_ostr, "16");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "x"_ostr, "30");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "y"_ostr, "30");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "text"_ostr, "ABC");

    // Without the fix in place, this test would have failed with
    // - Expected: 41
    // - Actual  : 12
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "dx0"_ostr, "41");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "dx1"_ostr, "52");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "dx2"_ostr, "63");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf156569)
{
    Primitive2DSequence aSequence = parseSvg(u"/svgio/qa/cppunit/data/tdf156569.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequence));

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "width"_ostr, "16");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "height"_ostr, "16");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "x"_ostr, "0");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "y"_ostr, "20");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "text"_ostr, "ABC");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "dx0"_ostr, "40");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "dx1"_ostr, "80");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "dx2"_ostr, "91");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "width"_ostr, "16");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "height"_ostr, "16");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "x"_ostr, "0");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "y"_ostr, "30");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "text"_ostr, "ABC");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "dx0"_ostr, "40");

    // Without the fix in place, this test would have failed with
    // - Expected: 80
    // - Actual  : 51
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "dx1"_ostr, "80");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "dx2"_ostr, "91");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf156837)
{
    Primitive2DSequence aSequence = parseSvg(u"/svgio/qa/cppunit/data/tdf156837.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequence));

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion"_ostr, 2);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "x"_ostr, "114");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "y"_ostr, "103");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "height"_ostr, "16");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "text"_ostr, "x");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "x"_ostr, "122");

    // Without the fix in place, this test would have failed with
    // - Expected: 94
    // - Actual  : 103
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "y"_ostr, "94");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "height"_ostr, "10");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "text"_ostr, " 3");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf156271)
{
    Primitive2DSequence aSequence = parseSvg(u"/svgio/qa/cppunit/data/tdf156271.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequence));

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/mask/textsimpleportion[1]"_ostr, "width"_ostr, "16");
    assertXPath(pDocument, "/primitive2D/transform/mask/textsimpleportion[1]"_ostr, "height"_ostr, "16");
    assertXPath(pDocument, "/primitive2D/transform/mask/textsimpleportion[1]"_ostr, "x"_ostr, "40");
    assertXPath(pDocument, "/primitive2D/transform/mask/textsimpleportion[1]"_ostr, "y"_ostr, "10");
    assertXPath(pDocument, "/primitive2D/transform/mask/textsimpleportion[1]"_ostr, "text"_ostr, "AB");
    assertXPath(pDocument, "/primitive2D/transform/mask/textsimpleportion[1]"_ostr, "dx0"_ostr, "-30");
    assertXPath(pDocument, "/primitive2D/transform/mask/textsimpleportion[1]"_ostr, "dx1"_ostr, "-19");

    assertXPath(pDocument, "/primitive2D/transform/mask/textsimpleportion[2]"_ostr, "width"_ostr, "16");
    assertXPath(pDocument, "/primitive2D/transform/mask/textsimpleportion[2]"_ostr, "height"_ostr, "16");
    assertXPath(pDocument, "/primitive2D/transform/mask/textsimpleportion[2]"_ostr, "x"_ostr, "40");
    assertXPath(pDocument, "/primitive2D/transform/mask/textsimpleportion[2]"_ostr, "y"_ostr, "20");
    assertXPath(pDocument, "/primitive2D/transform/mask/textsimpleportion[2]"_ostr, "text"_ostr, "AB");

    // Without the fix in place, this test would have failed with
    // - Expected: -30
    // - Actual  : 0
    assertXPath(pDocument, "/primitive2D/transform/mask/textsimpleportion[2]"_ostr, "dx0"_ostr, "-30");
    assertXPath(pDocument, "/primitive2D/transform/mask/textsimpleportion[2]"_ostr, "dx1"_ostr, "-19");

    assertXPath(pDocument, "/primitive2D/transform/mask/textsimpleportion[3]"_ostr, "width"_ostr, "16");
    assertXPath(pDocument, "/primitive2D/transform/mask/textsimpleportion[3]"_ostr, "height"_ostr, "16");
    assertXPath(pDocument, "/primitive2D/transform/mask/textsimpleportion[3]"_ostr, "x"_ostr, "40");
    assertXPath(pDocument, "/primitive2D/transform/mask/textsimpleportion[3]"_ostr, "y"_ostr, "30");
    assertXPath(pDocument, "/primitive2D/transform/mask/textsimpleportion[3]"_ostr, "text"_ostr, "AB");
    assertXPath(pDocument, "/primitive2D/transform/mask/textsimpleportion[3]"_ostr, "dx0"_ostr, "-30");
    assertXPath(pDocument, "/primitive2D/transform/mask/textsimpleportion[3]"_ostr, "dx1"_ostr, "-19");

    assertXPath(pDocument, "/primitive2D/transform/mask/textsimpleportion[4]"_ostr, "width"_ostr, "16");
    assertXPath(pDocument, "/primitive2D/transform/mask/textsimpleportion[4]"_ostr, "height"_ostr, "16");
    assertXPath(pDocument, "/primitive2D/transform/mask/textsimpleportion[4]"_ostr, "x"_ostr, "40");
    assertXPath(pDocument, "/primitive2D/transform/mask/textsimpleportion[4]"_ostr, "y"_ostr, "40");
    assertXPath(pDocument, "/primitive2D/transform/mask/textsimpleportion[4]"_ostr, "text"_ostr, "AB");
    assertXPath(pDocument, "/primitive2D/transform/mask/textsimpleportion[4]"_ostr, "dx0"_ostr, "12");
    assertXPath(pDocument, "/primitive2D/transform/mask/textsimpleportion[4]"_ostr, "dx1"_ostr, "23");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf159968)
{
    Primitive2DSequence aSequence = parseSvg(u"/svgio/qa/cppunit/data/tdf159968.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequence));

    CPPUNIT_ASSERT (pDocument);

    // Check no mask is applied to the marker
    assertXPath(pDocument,
            "/primitive2D/transform/transform/transform/transform/polypolygoncolor"_ostr, "color"_ostr, "#000000");
    assertXPath(pDocument,
            "/primitive2D/transform/transform/transform/transform/polypolygoncolor/polypolygon/polygon/point"_ostr, 5);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf149880)
{
    Primitive2DSequence aSequence = parseSvg(u"/svgio/qa/cppunit/data/tdf149880.svg");
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(aSequence.getLength()));

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequence));

    CPPUNIT_ASSERT (pDocument);

    // Without the fix in place, this test would have failed with
    // - Expected: 1
    // - Actual  : 0
    // - In <>, XPath '/primitive2D/transform/mask/unhandled' number of nodes is incorrect
    assertXPath(pDocument,
            "/primitive2D/transform/mask/unhandled"_ostr, "id"_ostr, "PATTERNFILL");
    assertXPath(pDocument,
            "/primitive2D/transform/mask/unhandled/mask/transform/transform/bitmap"_ostr, 28);
}

CPPUNIT_TEST_FIXTURE(Test, testCssClassRedefinition)
{
    // Tests for svg css class redefinition behavior
    // Example:
    // .c1 {fill:#00ff00}
    // .c1 {font-family:Sans}
    // .c1 {fill:#ff0000}
    // Expected result is .c1 {font-family:Sans; fill:#ff0000} because
    // the second redefinition appends attributes to the class and the
    // third redefinition replaces the already existing
    // attribute in the original definition
    Primitive2DSequence aSequence = parseSvg(u"/svgio/qa/cppunit/data/CssClassRedefinition.svg");
    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(aSequence));
    CPPUNIT_ASSERT (pDocument);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "text"_ostr, "012");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "fontcolor"_ostr, "#ff0000");
    assertXPath(
        pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "familyname"_ostr, "Open Symbol");
}

CPPUNIT_TEST_FIXTURE(Test, testTspanFillOpacity)
{
    // Given an SVG file with <tspan fill-opacity="0.30">:
    std::u16string_view aPath = u"/svgio/qa/cppunit/data/tspan-fill-opacity.svg";

    // When rendering that SVG:
    Primitive2DSequence aSequence = parseSvg(aPath);

    // Then make sure that the text portion is wrapped in a transparency primitive with the correct
    // transparency value:
    drawinglayer::Primitive2dXmlDump aDumper;
    xmlDocUniquePtr pDocument = aDumper.dumpAndParse(Primitive2DContainer(aSequence));
    sal_Int32 nTransparence = getXPath(pDocument, "//textsimpleportion[@text='hello']/parent::unifiedtransparence"_ostr, "transparence"_ostr).toInt32();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // - XPath '//textsimpleportion[@text='hello']/parent::unifiedtransparence' number of nodes is incorrect
    // i.e. the relevant <textsimpleportion> had no <unifiedtransparence> parent, the text was not
    // semi-transparent.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(70), nTransparence);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
