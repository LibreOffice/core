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
#include <comphelper/string.hxx>

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
    xmlDocUniquePtr dumpAndParseSvg(std::u16string_view aSource);
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

xmlDocUniquePtr Test::dumpAndParseSvg(std::u16string_view aSource)
{
    Primitive2DSequence aSequence = parseSvg(aSource);

    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(aSequence);

    CPPUNIT_ASSERT (pDocument);
    return pDocument;
}

void Test::checkRectPrimitive(Primitive2DSequence const & rPrimitive)
{
    drawinglayer::Primitive2dXmlDump dumper;
    xmlDocUniquePtr pDocument = dumper.dumpAndParse(Primitive2DContainer(rPrimitive));

    CPPUNIT_ASSERT (pDocument);

    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor"_ostr, "color"_ostr, u"#00cc00"_ustr); // rect background color
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor/polypolygon"_ostr, "height"_ostr, u"100"_ustr); // rect background height
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor/polypolygon"_ostr, "width"_ostr, u"100"_ustr); // rect background width
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor/polypolygon"_ostr, "minx"_ostr, u"10"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor/polypolygon"_ostr, "miny"_ostr, u"10"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor/polypolygon"_ostr, "maxx"_ostr, u"110"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor/polypolygon"_ostr, "maxy"_ostr, u"110"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/polypolygonstroke/line"_ostr, "color"_ostr, u"#ff0000"_ustr); // rect stroke color
    assertXPath(pDocument, "/primitive2D/transform/polypolygonstroke/line"_ostr, "width"_ostr, u"3"_ustr); // rect stroke width
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
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/symbol.svg");

    // tdf#126330: Without the fix in place, this test would have failed with
    // - Expected: 1
    // - Actual  : 2
    // number of nodes is incorrect
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor"_ostr, "color"_ostr, u"#00d000"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf150124)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf150124.svg");

    assertXPathChildren(pDocument, "/primitive2D"_ostr, 1);
    assertXPath(pDocument, "/primitive2D/hiddengeometry"_ostr, 1);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf155819)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf155819.svg");

    assertXPath(pDocument, "/primitive2D/transform/polypolygonstroke/line"_ostr, 1);
    assertXPath(pDocument, "/primitive2D/transform/polypolygonstroke/polypolygon"_ostr, 1);
    // Without the fix in place, this test would have failed with
    // - Expected: 4
    // - Actual  : 0
    assertXPath(pDocument, "/primitive2D/transform/transform"_ostr, 4);
}

CPPUNIT_TEST_FIXTURE(Test, testNormalBlend)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/normalBlend.svg");

    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap"_ostr, "height"_ostr, u"170"_ustr);
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap"_ostr, "width"_ostr, u"170"_ustr);
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap/data"_ostr, 170);

    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap"_ostr, "xy11"_ostr, u"170"_ustr);
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap"_ostr, "xy12"_ostr, u"0"_ustr);
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap"_ostr, "xy13"_ostr, u"0"_ustr);
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap"_ostr, "xy21"_ostr, u"0"_ustr);
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap"_ostr, "xy22"_ostr, u"170"_ustr);
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap"_ostr, "xy23"_ostr, u"0"_ustr);
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap"_ostr, "xy31"_ostr, u"0"_ustr);
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap"_ostr, "xy32"_ostr, u"0"_ustr);
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap"_ostr, "xy33"_ostr, u"1"_ustr);

    // Check the colors in the diagonal
    OUString sDataRow = getXPath(pDocument, "/primitive2D/transform/transform/bitmap/data[40]"_ostr, "row"_ostr);
    std::vector<OUString> aPixels = comphelper::string::split(sDataRow, ',');
    CPPUNIT_ASSERT_EQUAL(u"ffd700"_ustr, aPixels[40]);

    sDataRow = getXPath(pDocument, "/primitive2D/transform/transform/bitmap/data[85]"_ostr, "row"_ostr);
    aPixels = comphelper::string::split(sDataRow, ',');
    CPPUNIT_ASSERT_EQUAL(u"ffd700"_ustr, aPixels[85]);

    sDataRow = getXPath(pDocument, "/primitive2D/transform/transform/bitmap/data[130]"_ostr, "row"_ostr);
    aPixels = comphelper::string::split(sDataRow, ',');
    CPPUNIT_ASSERT_EQUAL(u"8a2be2"_ustr, aPixels[130]);
}

CPPUNIT_TEST_FIXTURE(Test, testFeColorMatrix)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/filterFeColorMatrix.svg");

    assertXPath(pDocument, "/primitive2D/transform/mask/modifiedColor[1]"_ostr, "modifier"_ostr, u"matrix"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/mask/modifiedColor[2]"_ostr, "modifier"_ostr, u"saturate"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/mask/modifiedColor[3]"_ostr, "modifier"_ostr, u"hueRotate"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/mask/modifiedColor[4]"_ostr, "modifier"_ostr, u"luminance_to_alpha"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testFilterFeComposite)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/filterFeComposite.svg");

    assertXPath(pDocument, "/primitive2D/transform/mask"_ostr, 5);
    // over operator
    assertXPath(pDocument, "/primitive2D/transform/mask[1]/polypolygoncolor"_ostr, 3);
    assertXPath(pDocument, "/primitive2D/transform/mask[1]/polypolygon/polygon/point"_ostr, 8);
    // xor operator
    assertXPath(pDocument, "/primitive2D/transform/mask[2]/polypolygoncolor"_ostr, 3);
    assertXPath(pDocument, "/primitive2D/transform/mask[2]/polypolygon/polygon[1]/point"_ostr, 8);
    assertXPath(pDocument, "/primitive2D/transform/mask[2]/polypolygon/polygon[2]/point"_ostr, 4);
    // in operator
    assertXPath(pDocument, "/primitive2D/transform/mask[3]/polypolygoncolor"_ostr, 3);
    assertXPath(pDocument, "/primitive2D/transform/mask[3]/polypolygon/polygon/point"_ostr, 4);
    // out operator
    assertXPath(pDocument, "/primitive2D/transform/mask[4]/polypolygoncolor"_ostr, 3);
    assertXPath(pDocument, "/primitive2D/transform/mask[4]/polypolygon/polygon/point"_ostr, 6);
    // atop operator
    assertXPath(pDocument, "/primitive2D/transform/mask[5]/polypolygoncolor"_ostr, 3);
    assertXPath(pDocument, "/primitive2D/transform/mask[5]/polypolygon/polygon[1]/point"_ostr, 6);
    assertXPath(pDocument, "/primitive2D/transform/mask[5]/polypolygon/polygon[2]/point"_ostr, 4);
}

CPPUNIT_TEST_FIXTURE(Test, testFilterFeGaussianBlur)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/filterFeGaussianBlur.svg");

    assertXPath(pDocument, "/primitive2D/transform/softedge"_ostr, "radius"_ostr, u"5"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testInFilterAttribute)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/inFilterAttribute.svg");

    // Without the fix in place, the feGaussianBlur and feColorMatrix filter would have been applied
    assertXPath(pDocument, "/primitive2D/transform/transform"_ostr, "xy11"_ostr, u"1"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/transform"_ostr, "xy12"_ostr, u"0"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/transform"_ostr, "xy13"_ostr, u"40"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/transform"_ostr, "xy21"_ostr, u"0"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/transform"_ostr, "xy22"_ostr, u"1"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/transform"_ostr, "xy23"_ostr, u"40"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/transform"_ostr, "xy31"_ostr, u"0"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/transform"_ostr, "xy32"_ostr, u"0"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/transform"_ostr, "xy33"_ostr, u"1"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/transform/polypolygoncolor"_ostr, "color"_ostr, u"#ffffff"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testResultFilterAttribute)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/resultFilterAttribute.svg");

    // Without the fix in place, the feColorMatrix filter would have been applied
    assertXPath(pDocument, "/primitive2D/transform/transform"_ostr, "xy11"_ostr, u"1"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/transform"_ostr, "xy12"_ostr, u"0"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/transform"_ostr, "xy13"_ostr, u"40"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/transform"_ostr, "xy21"_ostr, u"0"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/transform"_ostr, "xy22"_ostr, u"1"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/transform"_ostr, "xy23"_ostr, u"40"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/transform"_ostr, "xy31"_ostr, u"0"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/transform"_ostr, "xy32"_ostr, u"0"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/transform"_ostr, "xy33"_ostr, u"1"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/transform/softedge"_ostr, "radius"_ostr, u"2"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/transform/softedge/polypolygoncolor"_ostr, "color"_ostr, u"#ffffff"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testFilterFeMerge)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/filterFeMerge.svg");

    assertXPath(pDocument, "/primitive2D/transform/mask/softedge"_ostr, 2);
    assertXPath(pDocument, "/primitive2D/transform/mask/polypolygon"_ostr, 1);
    assertXPath(pDocument, "/primitive2D/transform/mask/polypolygoncolor"_ostr, 1);
    assertXPath(pDocument, "/primitive2D/transform/mask/polypolygonstroke"_ostr, 1);
}

CPPUNIT_TEST_FIXTURE(Test, testFilterFeOffset)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/filterFeOffset.svg");

    assertXPath(pDocument, "/primitive2D/transform/mask/transform"_ostr, "xy11"_ostr, u"1"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/mask/transform"_ostr, "xy12"_ostr, u"0"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/mask/transform"_ostr, "xy13"_ostr, u"44"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/mask/transform"_ostr, "xy21"_ostr, u"0"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/mask/transform"_ostr, "xy22"_ostr, u"1"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/mask/transform"_ostr, "xy23"_ostr, u"66"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/mask/transform"_ostr, "xy31"_ostr, u"0"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/mask/transform"_ostr, "xy32"_ostr, u"0"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/mask/transform"_ostr, "xy33"_ostr, u"1"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testFilterFeFlood)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/filterFeFlood.svg");

    assertXPath(pDocument, "/primitive2D/transform/unifiedtransparence"_ostr, "transparence"_ostr, u"50"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/unifiedtransparence/polypolygoncolor"_ostr, "color"_ostr, u"#008000"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/unifiedtransparence/polypolygoncolor/polypolygon"_ostr, "height"_ostr, u"100"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/unifiedtransparence/polypolygoncolor/polypolygon"_ostr, "width"_ostr, u"100"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/unifiedtransparence/polypolygoncolor/polypolygon"_ostr, "minx"_ostr, u"50"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/unifiedtransparence/polypolygoncolor/polypolygon"_ostr, "miny"_ostr, u"50"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testFilterFeDropShadow)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/filterFeDropShadow.svg");

    assertXPath(pDocument, "/primitive2D/transform/unifiedtransparence"_ostr, "transparence"_ostr, u"50"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/unifiedtransparence/shadow"_ostr, "color"_ostr, u"#0000ff"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/unifiedtransparence/shadow"_ostr, "blur"_ostr, u"0.2"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/unifiedtransparence/shadow"_ostr, "blur"_ostr, u"0.2"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor"_ostr, "color"_ostr, u"#ffc0cb"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testFilterFeImage)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/filterFeImage.svg");

    assertXPath(pDocument, "/primitive2D/transform/transform/bitmap"_ostr);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf87309)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf87309.svg");

    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor"_ostr, "color"_ostr, u"#000000"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor/polypolygon"_ostr, "height"_ostr, u"100"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor/polypolygon"_ostr, "width"_ostr, u"100"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor/polypolygon"_ostr, "minx"_ostr, u"10"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor/polypolygon"_ostr, "miny"_ostr, u"10"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor/polypolygon"_ostr, "maxx"_ostr, u"110"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor/polypolygon"_ostr, "maxy"_ostr, u"110"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testFontsizeKeywords)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/FontsizeKeywords.svg");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "fontcolor"_ostr, u"#000000"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "text"_ostr, u"Sample"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "height"_ostr, u"9"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "familyname"_ostr, u"Times New Roman"_ustr);

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "fontcolor"_ostr, u"#ffffff"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "text"_ostr, u"Sample"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "height"_ostr, u"11"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "familyname"_ostr, u"Times New Roman"_ustr);

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[3]"_ostr, "fontcolor"_ostr, u"#ffd700"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[3]"_ostr, "text"_ostr, u"Sample"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[3]"_ostr, "height"_ostr, u"13"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[3]"_ostr, "familyname"_ostr, u"Times New Roman"_ustr);

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[4]"_ostr, "fontcolor"_ostr, u"#ff0000"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[4]"_ostr, "text"_ostr, u"Sample"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[4]"_ostr, "height"_ostr, u"16"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[4]"_ostr, "familyname"_ostr, u"Times New Roman"_ustr);

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[5]"_ostr, "fontcolor"_ostr, u"#ffff00"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[5]"_ostr, "text"_ostr, u"Sample"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[5]"_ostr, "height"_ostr, u"19"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[5]"_ostr, "familyname"_ostr, u"Times New Roman"_ustr);

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[6]"_ostr, "fontcolor"_ostr, u"#0000ff"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[6]"_ostr, "text"_ostr, u"Sample"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[6]"_ostr, "height"_ostr, u"23"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[6]"_ostr, "familyname"_ostr, u"Times New Roman"_ustr);

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[7]"_ostr, "fontcolor"_ostr, u"#008000"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[7]"_ostr, "text"_ostr, u"Sample"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[7]"_ostr, "height"_ostr, u"28"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[7]"_ostr, "familyname"_ostr, u"Times New Roman"_ustr);

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[8]"_ostr, "fontcolor"_ostr, u"#ff7f50"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[8]"_ostr, "text"_ostr, u"Sample"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[8]"_ostr, "height"_ostr, u"13"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[8]"_ostr, "familyname"_ostr, u"Times New Roman"_ustr);

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[9]"_ostr, "fontcolor"_ostr, u"#ffc0cb"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[9]"_ostr, "text"_ostr, u"Sample"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[9]"_ostr, "height"_ostr, u"19"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[9]"_ostr, "familyname"_ostr, u"Times New Roman"_ustr);

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[10]"_ostr, "fontcolor"_ostr, u"#fffff0"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[10]"_ostr, "text"_ostr, u"Sample"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[10]"_ostr, "height"_ostr, u"16"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[9]"_ostr, "familyname"_ostr, u"Times New Roman"_ustr);
}


CPPUNIT_TEST_FIXTURE(Test, testFontsizePercentage)
{
    //Check when font-size uses percentage and defined globally
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/FontsizePercentage.svg");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "fontcolor"_ostr, u"#000000"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "text"_ostr, u"Sample"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "height"_ostr, u"16"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "familyname"_ostr, u"Times New Roman"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testFontsizeRelative)
{
    //Check when font-size uses relative units (em,ex) and it's based on its parent's font-size
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/FontsizeRelative.svg");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "fontcolor"_ostr, u"#000000"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "text"_ostr, u"Sample"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "height"_ostr, u"50"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "familyname"_ostr, u"DejaVu Serif"_ustr);

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "fontcolor"_ostr, u"#000000"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "text"_ostr, u"Sample"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "height"_ostr, u"50"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "familyname"_ostr, u"DejaVu Serif"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf160386)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf160386.svg");

    // Without the fix in place, this test would have failed with
    // - Expected: 1
    // - Actual  : 11
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion"_ostr, 1);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion"_ostr, "text"_ostr, u"Hello!"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf145896)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf145896.svg");

    // Without the fix in place, this test would have failed with
    // - Expected: #ffff00
    // - Actual  : #000000
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[1]"_ostr, "color"_ostr, u"#ffff00"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[2]"_ostr, "color"_ostr, u"#008000"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[3]"_ostr, "color"_ostr, u"#0000ff"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf156579)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf156579.svg");

    // Without the fix in place, nothing would be displayed
    assertXPath(pDocument, "/primitive2D/transform/mask/transform/polypolygoncolor[1]"_ostr, "color"_ostr, u"#0000ff"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf156168)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf156168.svg");

    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor"_ostr, 8);
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[1]"_ostr, "color"_ostr, u"#0000ff"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[2]"_ostr, "color"_ostr, u"#0000ff"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[3]"_ostr, "color"_ostr, u"#ff0000"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[4]"_ostr, "color"_ostr, u"#ff0000"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[5]"_ostr, "color"_ostr, u"#000000"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[6]"_ostr, "color"_ostr, u"#000000"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[7]"_ostr, "color"_ostr, u"#ff0000"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[8]"_ostr, "color"_ostr, u"#ff0000"_ustr);

    // Without the fix in place, this test would have failed with
    // - Expected: 4
    // - Actual  : 3
    assertXPath(pDocument, "/primitive2D/transform/polypolygonstroke"_ostr, 4);
    assertXPath(pDocument, "/primitive2D/transform/polypolygonstroke[1]/line"_ostr, "width"_ostr, u"5"_ustr);

    assertXPath(pDocument, "/primitive2D/transform/polypolygonstroke[1]/line"_ostr, "color"_ostr, u"#00ff00"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/polypolygonstroke[2]/line"_ostr, "width"_ostr, u"5"_ustr);

    assertXPath(pDocument, "/primitive2D/transform/polypolygonstroke[2]/line"_ostr, "color"_ostr, u"#00ff00"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/polypolygonstroke[3]/line"_ostr, "width"_ostr, u"5"_ustr);

    assertXPath(pDocument, "/primitive2D/transform/polypolygonstroke[3]/line"_ostr, "color"_ostr, u"#00ff00"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/polypolygonstroke[4]/line"_ostr, "width"_ostr, u"5"_ustr);

    assertXPath(pDocument, "/primitive2D/transform/polypolygonstroke[4]/line"_ostr, "color"_ostr, u"#00ff00"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf160373)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf160373.svg");

    // Without the fix in place, nothing would be displayed
    assertXPath(pDocument, "/primitive2D/transform/transform/polypolygoncolor"_ostr, "color"_ostr, u"#0000ff"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf129356)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf129356.svg");

    // Without the fix in place, this test would have failed with
    // - Expected: #008000
    // - Actual  : #0000ff
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[1]"_ostr, "color"_ostr, u"#008000"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[2]"_ostr, "color"_ostr, u"#008000"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[3]"_ostr, "color"_ostr, u"#008000"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[4]"_ostr, "color"_ostr, u"#008000"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[5]"_ostr, "color"_ostr, u"#008000"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[6]"_ostr, "color"_ostr, u"#008000"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[7]"_ostr, "color"_ostr, u"#008000"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[8]"_ostr, "color"_ostr, u"#008000"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf156034)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf156034.svg");

    // Without the fix in place, this test would have failed with
    // - Expected: #008000
    // - Actual  : #0000ff
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[1]"_ostr, "color"_ostr, u"#008000"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[2]"_ostr, "color"_ostr, u"#008000"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[3]"_ostr, "color"_ostr, u"#008000"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[4]"_ostr, "color"_ostr, u"#008000"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[5]"_ostr, "color"_ostr, u"#008000"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[6]"_ostr, "color"_ostr, u"#008000"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[7]"_ostr, "color"_ostr, u"#008000"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[8]"_ostr, "color"_ostr, u"#008000"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf156038)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf156038.svg");

    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[1]"_ostr, "color"_ostr, u"#0000ff"_ustr);

    // Without the fix in place, this test would have failed with
    // - Expected: #008000
    // - Actual  : #0000ff
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[2]"_ostr, "color"_ostr, u"#008000"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[3]"_ostr, "color"_ostr, u"#0000ff"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[4]"_ostr, "color"_ostr, u"#008000"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[5]"_ostr, "color"_ostr, u"#0000ff"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[6]"_ostr, "color"_ostr, u"#008000"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[7]"_ostr, "color"_ostr, u"#0000ff"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[8]"_ostr, "color"_ostr, u"#008000"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf156018)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf156018.svg");

    // Without the fix in place, this test would have failed with
    // - Expected: #008000
    // - Actual  : #0000ff
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[1]"_ostr, "color"_ostr, u"#008000"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[2]"_ostr, "color"_ostr, u"#0000ff"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf156201)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf156201.svg");

    assertXPath(pDocument, "/primitive2D/transform/transform/transform/polypolygoncolor"_ostr, "color"_ostr, u"#2f3ba1"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf156167)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf156167.svg");

    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[1]"_ostr, "color"_ostr, u"#ffa500"_ustr);

    // Without the fix in place, this test would have failed with
    // - Expected: #ffa500
    // - Actual  : #ff0000
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[2]"_ostr, "color"_ostr, u"#ffa500"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[3]"_ostr, "color"_ostr, u"#ffa500"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf155932)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf155932.svg");

    assertXPath(pDocument, "/primitive2D/transform/mask/mask/unifiedtransparence"_ostr, "transparence"_ostr, u"50"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/mask/mask/unifiedtransparence[1]/polypolygoncolor"_ostr, "color"_ostr, u"#0000ff"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf97717)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf97717.svg");

    assertXPath(pDocument, "/primitive2D/transform/unifiedtransparence[1]"_ostr, "transparence"_ostr, u"50"_ustr);
    // Without the fix in place, this test would have failed here since the patch
    // would have contained two unifiedtransparence
    assertXPath(pDocument, "/primitive2D/transform/unifiedtransparence[1]/polypolygoncolor"_ostr, "color"_ostr, u"#ccccff"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/unifiedtransparence[2]"_ostr, "transparence"_ostr, u"50"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/unifiedtransparence[2]/polypolygoncolor"_ostr, "color"_ostr, u"#ccccff"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testMarkerOrient)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/MarkerOrient.svg");

    assertXPath(pDocument, "/primitive2D/transform/transform[1]"_ostr, "xy11"_ostr, u"0"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/transform[1]"_ostr, "xy12"_ostr, u"1"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/transform[1]"_ostr, "xy13"_ostr, u"7"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/transform[1]"_ostr, "xy21"_ostr, u"-1"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/transform[1]"_ostr, "xy22"_ostr, u"0"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/transform[1]"_ostr, "xy23"_ostr, u"13"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/transform[1]"_ostr, "xy31"_ostr, u"0"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/transform[1]"_ostr, "xy32"_ostr, u"0"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/transform[1]"_ostr, "xy33"_ostr, u"1"_ustr);

    assertXPath(pDocument, "/primitive2D/transform/transform[2]"_ostr, "xy11"_ostr, u"1"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/transform[2]"_ostr, "xy12"_ostr, u"0"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/transform[2]"_ostr, "xy13"_ostr, u"87"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/transform[2]"_ostr, "xy21"_ostr, u"0"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/transform[2]"_ostr, "xy22"_ostr, u"1"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/transform[2]"_ostr, "xy23"_ostr, u"87"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/transform[2]"_ostr, "xy31"_ostr, u"0"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/transform[2]"_ostr, "xy32"_ostr, u"0"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/transform[2]"_ostr, "xy33"_ostr, u"1"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testContextFill)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/contextFill.svg");

    assertXPath(pDocument, "/primitive2D/transform/polypolygonstroke/line"_ostr, "color"_ostr, u"#ff0000"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/transform/polypolygoncolor"_ostr, "color"_ostr, u"#ff0000"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testFillContextStroke)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/fillContextStroke.svg");

    assertXPath(pDocument, "/primitive2D/transform/polypolygonstroke/line"_ostr, "color"_ostr, u"#ff0000"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/transform/polypolygoncolor"_ostr, "color"_ostr, u"#ff0000"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testContextStroke)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/contextStroke.svg");

    assertXPath(pDocument, "/primitive2D/transform/polypolygonstroke/line"_ostr, "color"_ostr, u"#ff0000"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/transform[1]/polypolygonstroke/line"_ostr, "color"_ostr, u"#ff0000"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/transform[2]/polypolygonstroke/line"_ostr, "color"_ostr, u"#ff0000"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/transform[3]/polypolygonstroke/line"_ostr, "color"_ostr, u"#ff0000"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/transform[4]/polypolygonstroke/line"_ostr, "color"_ostr, u"#ff0000"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testMarkerInPresentation)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/markerInPresentation.svg");

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
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/markerInCssStyle.svg");

    // Without the fix in place, this test would have failed with
    // - Expected: 20
    // - Actual  : 0
    assertXPath(pDocument, "/primitive2D/transform/transform/polypolygonstroke/line"_ostr, 20);

    assertXPath(pDocument, "/primitive2D/transform/transform[1]/polypolygonstroke/line"_ostr, "color"_ostr, u"#008000"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/transform[1]/polypolygonstroke/line"_ostr, "width"_ostr, u"1"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/transform[1]/polypolygonstroke/line"_ostr, "linejoin"_ostr, u"Miter"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/transform[1]/polypolygonstroke/line"_ostr, "miterangle"_ostr, u"29"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/transform[1]/polypolygonstroke/line"_ostr, "linecap"_ostr, u"BUTT"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testTextXmlSpace)
{
    //Check tspan fontsize when using relative units
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/textXmlSpace.svg");

    assertXPath(pDocument, "/primitive2D/transform/mask/textsimpleportion[1]"_ostr, "text"_ostr, u"a b"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/mask/textsimpleportion[2]"_ostr, "text"_ostr, u"a b"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/mask/textsimpleportion[3]"_ostr, "text"_ostr, u"a b"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/mask/textsimpleportion[4]"_ostr, "text"_ostr, u"ab"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/mask/textsimpleportion[5]"_ostr, "text"_ostr, u" a  b "_ustr);
    assertXPath(pDocument, "/primitive2D/transform/mask/textsimpleportion[6]"_ostr, "text"_ostr, u"a b"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/mask/textsimpleportion[7]"_ostr, "text"_ostr, u"a   b"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/mask/textsimpleportion[8]"_ostr, "text"_ostr, u"a b"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf45771)
{
    //Check text fontsize when using relative units
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf45771.svg");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "fontcolor"_ostr, u"#000000"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "text"_ostr, u"Sample"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "height"_ostr, u"32"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "familyname"_ostr, u"Times New Roman"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf155833)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf155833.svg");

    assertXPath(pDocument, "/primitive2D/transform/mask/transform/transform/transform/transform/transform/bitmap"_ostr, 1);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf97941)
{
    //Check tspan fontsize when using relative units
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf97941.svg");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "fontcolor"_ostr, u"#000000"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "text"_ostr, u"Sample"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "height"_ostr, u"48"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "familyname"_ostr, u"Times New Roman"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf156777)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf156777.svg");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion"_ostr, 23);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "text"_ostr, u"Quick brown fox jumps over the lazy dog."_ustr);

    // Without the fix in place, this test would have failed with
    // - Expected: #008000
    // - Actual  : #000000
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "fontcolor"_ostr, u"#008000"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "x"_ostr, u"85"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "y"_ostr, u"23"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf156834)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf156834.svg");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion"_ostr, 4);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "text"_ostr, u"Auto"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "x"_ostr, u"30"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "y"_ostr, u"20"_ustr);

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "text"_ostr, u"Middle"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "x"_ostr, u"30"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "y"_ostr, u"57"_ustr);

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[3]"_ostr, "text"_ostr, u"Hanging"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[3]"_ostr, "x"_ostr, u"30"_ustr);
    assertXPathDoubleValue(pDocument, "/primitive2D/transform/textsimpleportion[3]"_ostr, "y"_ostr, 93.5, 0.5);

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[4]"_ostr, "text"_ostr, u"Central"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[4]"_ostr, "x"_ostr, u"30"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[4]"_ostr, "y"_ostr, u"117"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf104339)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf104339.svg");

    assertXPath(pDocument, "/primitive2D/transform/transform/transform/transform/transform/polypolygoncolor"_ostr, "color"_ostr, u"#000000"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf85770)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf85770.svg");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "fontcolor"_ostr, u"#000000"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "text"_ostr, u"Start Middle End"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "height"_ostr, u"11"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "familyname"_ostr, u"Times New Roman"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "fontcolor"_ostr, u"#000000"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "text"_ostr, u"Start "_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "height"_ostr, u"11"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "familyname"_ostr, u"Times New Roman"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[3]"_ostr, "fontcolor"_ostr, u"#000000"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[3]"_ostr, "text"_ostr, u" End"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[3]"_ostr, "height"_ostr, u"11"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[3]"_ostr, "familyname"_ostr, u"Times New Roman"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf86938)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf86938.svg");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "text"_ostr, u"line"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "x"_ostr, u"290"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "y"_ostr, u"183"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "text"_ostr, u"above"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "x"_ostr, u"290"_ustr);

    // Without the fix in place, this test would have failed with
    // - Expected: 159
    // - Actual  : 207
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "y"_ostr, u"159"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[3]"_ostr, "text"_ostr, u"below"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[3]"_ostr, "x"_ostr, u"290"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[3]"_ostr, "y"_ostr, u"207"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf93583)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf93583.svg");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "text"_ostr, u"This is the"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "x"_ostr, u"58"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "y"_ostr, u"303"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "width"_ostr, u"16"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "height"_ostr, u"16"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "text"_ostr, u" first "_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "x"_ostr, u"125"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "y"_ostr, u"303"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "width"_ostr, u"32"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "height"_ostr, u"32"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[3]"_ostr, "text"_ostr, u"line"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[3]"_ostr, "x"_ostr, u"192"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[3]"_ostr, "y"_ostr, u"303"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[3]"_ostr, "width"_ostr, u"16"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[3]"_ostr, "height"_ostr, u"16"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf156616)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf156616.svg");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "text"_ostr, u"First "_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "x"_ostr, u"114"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "y"_ostr, u"103"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "text"_ostr, u"line "_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "x"_ostr, u"147"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "y"_ostr, u"103"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[3]"_ostr, "text"_ostr, u"Second line"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[3]"_ostr, "x"_ostr, u"114"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[3]"_ostr, "y"_ostr, u"122"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[4]"_ostr, "text"_ostr, u"First "_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[4]"_ostr, "x"_ostr, u"85"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[4]"_ostr, "y"_ostr, u"153"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[5]"_ostr, "text"_ostr, u"line "_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[5]"_ostr, "x"_ostr, u"118"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[5]"_ostr, "y"_ostr, u"153"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[6]"_ostr, "text"_ostr, u"Second line"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[6]"_ostr, "x"_ostr, u"77"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[6]"_ostr, "y"_ostr, u"172"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[7]"_ostr, "text"_ostr, u"First "_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[7]"_ostr, "x"_ostr, u"55"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[7]"_ostr, "y"_ostr, u"203"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[8]"_ostr, "text"_ostr, u"line "_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[8]"_ostr, "x"_ostr, u"88"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[8]"_ostr, "y"_ostr, u"203"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[9]"_ostr, "text"_ostr, u"Second line"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[9]"_ostr, "x"_ostr, u"40"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[9]"_ostr, "y"_ostr, u"222"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf79163)
{
    //Check Opacity
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf79163.svg");

    assertXPath(pDocument, "/primitive2D/transform/unifiedtransparence"_ostr, "transparence"_ostr, u"50"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf97542_1)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf97542_1.svg");

    assertXPath(pDocument, "/primitive2D/transform/objectinfo/textsimpleportion"_ostr, "fontcolor"_ostr, u"#ffff00"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/objectinfo/textsimpleportion"_ostr, "text"_ostr, u"Text"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/objectinfo/textsimpleportion"_ostr, "height"_ostr, u"48"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/objectinfo/textsimpleportion"_ostr, "familyname"_ostr, u"DejaVu Serif"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf97542_2)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf97542_2.svg");

    assertXPath(pDocument, "/primitive2D/transform/objectinfo/svgradialgradient"_ostr, "startx"_ostr, u"1"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/objectinfo/svgradialgradient"_ostr, "starty"_ostr, u"1"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/objectinfo/svgradialgradient/focalx"_ostr, 0);
    assertXPath(pDocument, "/primitive2D/transform/objectinfo/svgradialgradient/focaly"_ostr, 0);
    assertXPath(pDocument, "/primitive2D/transform/objectinfo/svgradialgradient"_ostr, "radius"_ostr, u"3"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/objectinfo/svgradialgradient"_ostr, "spreadmethod"_ostr, u"pad"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/objectinfo/svgradialgradient"_ostr, "opacity"_ostr, u"1"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf97543)
{
    // check visibility="inherit"
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf97543.svg");

    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor"_ostr, "color"_ostr, u"#00cc00"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor/polypolygon"_ostr, "height"_ostr, u"100"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor/polypolygon"_ostr, "width"_ostr, u"100"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor/polypolygon"_ostr, "minx"_ostr, u"10"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor/polypolygon"_ostr, "miny"_ostr, u"10"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor/polypolygon"_ostr, "maxx"_ostr, u"110"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor/polypolygon"_ostr, "maxy"_ostr, u"110"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testRGBColor)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/RGBColor.svg");

    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor"_ostr, "color"_ostr, u"#646464"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor/polypolygon"_ostr, "height"_ostr, u"100"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor/polypolygon"_ostr, "width"_ostr, u"100"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor/polypolygon"_ostr, "minx"_ostr, u"10"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor/polypolygon"_ostr, "miny"_ostr, u"10"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor/polypolygon"_ostr, "maxx"_ostr, u"110"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor/polypolygon"_ostr, "maxy"_ostr, u"110"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf149673)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf149673.svg");

    assertXPath(pDocument, "/primitive2D/transform/unifiedtransparence"_ostr, "transparence"_ostr, u"90"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/unifiedtransparence/polypolygoncolor[1]"_ostr, "color"_ostr, u"#ff0000"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/unifiedtransparence/polypolygoncolor[2]"_ostr, "color"_ostr, u"#00ff00"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/unifiedtransparence/polypolygoncolor[3]"_ostr, "color"_ostr, u"#0000ff"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testRGBAColor)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/RGBAColor.svg");

    assertXPath(pDocument, "/primitive2D/transform/unifiedtransparence"_ostr, "transparence"_ostr, u"50"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testNoneColor)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/noneColor.svg");

    //No polypolygoncolor exists
    assertXPath(pDocument, "/primitive2D/transform/mask/polypolygoncolor"_ostr, 0);
    assertXPath(pDocument, "/primitive2D/transform/mask/polypolygonstroke/line"_ostr, "color"_ostr, u"#000000"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/mask/polypolygonstroke/line"_ostr, "width"_ostr, u"3"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf117920)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf117920.svg");

    assertXPath(pDocument, "/primitive2D/transform/transform"_ostr, "xy11"_ostr, u"1"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/transform"_ostr, "xy12"_ostr, u"0"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/transform"_ostr, "xy13"_ostr, u"-18"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/transform"_ostr, "xy21"_ostr, u"0"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/transform"_ostr, "xy22"_ostr, u"1"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/transform"_ostr, "xy23"_ostr, u"-6"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/transform"_ostr, "xy31"_ostr, u"0"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/transform"_ostr, "xy32"_ostr, u"0"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/transform"_ostr, "xy33"_ostr, u"1"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf97936)
{
    // check that both rectangles are rendered in the viewBox
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf97936.svg");

    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[1]"_ostr);
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[1]/polypolygon"_ostr, "height"_ostr, u"50"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[1]/polypolygon"_ostr, "width"_ostr, u"50"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[1]/polypolygon"_ostr, "minx"_ostr, u"70"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[1]/polypolygon"_ostr, "miny"_ostr, u"50"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[1]/polypolygon"_ostr, "maxx"_ostr, u"120"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[1]/polypolygon"_ostr, "maxy"_ostr, u"100"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[2]"_ostr);
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[2]/polypolygon"_ostr, "height"_ostr, u"50"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[2]/polypolygon"_ostr, "width"_ostr, u"50"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[2]/polypolygon"_ostr, "minx"_ostr, u"10"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[2]/polypolygon"_ostr, "miny"_ostr, u"50"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[2]/polypolygon"_ostr, "maxx"_ostr, u"60"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[2]/polypolygon"_ostr, "maxy"_ostr, u"100"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf149893)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf149893.svg");

    // Without the fix in place, this test would have failed with
    // - Expected: #008000
    // - Actual  : #000000
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor"_ostr, "color"_ostr, u"#008000"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testShapeWithClipPathAndCssStyle)
{
    // tdf#97539: Check there is a mask and 3 polygons
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/ShapeWithClipPathAndCssStyle.svg");

    assertXPath(pDocument, "/primitive2D/transform/mask/polypolygon/polygon"_ostr, 2);
    assertXPath(pDocument, "/primitive2D/transform/mask/polypolygoncolor/polypolygon/polygon"_ostr, 1);
}

CPPUNIT_TEST_FIXTURE(Test, testClipPathAndParentStyle)
{
    //Check that fill color, stroke color and stroke-width are inherited from use element
    //when the element is within a clipPath element
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/ClipPathAndParentStyle.svg");

    assertXPath(pDocument, "/primitive2D/transform/transform/polypolygoncolor"_ostr, "color"_ostr, u"#ff0000"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/transform/polypolygonstroke/line"_ostr, "color"_ostr, u"#000000"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/transform/polypolygonstroke/line"_ostr, "width"_ostr, u"5"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf155814)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf155814.svg");

    assertXPath(pDocument, "/primitive2D/transform/mask/mask/transform/unifiedtransparence"_ostr, "transparence"_ostr, u"50"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/mask/mask/transform/unifiedtransparence/polypolygoncolor"_ostr, "color"_ostr, u"#0000ff"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testClipPathAndStyle)
{
    //Check that fill color, stroke color and stroke-width are inherited from use element
    //when the element is within a clipPath element
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/ClipPathAndStyle.svg");

    assertXPath(pDocument, "/primitive2D/transform/transform/polypolygoncolor"_ostr, "color"_ostr, u"#ccccff"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/transform/polypolygonstroke/line"_ostr, "color"_ostr, u"#0000cc"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/transform/polypolygonstroke/line"_ostr, "width"_ostr, u"2"_ustr);

}

CPPUNIT_TEST_FIXTURE(Test, testShapeWithClipPath)
{
    // Check there is a mask and 3 polygons
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/ShapeWithClipPath.svg");

    assertXPath(pDocument, "/primitive2D/transform/mask/polypolygon/polygon"_ostr, 2);
    assertXPath(pDocument, "/primitive2D/transform/mask/polypolygoncolor/polypolygon/polygon"_ostr, 1);
}

CPPUNIT_TEST_FIXTURE(Test, testClipPathUsingClipPath)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/ClipPathUsingClipPath.svg");

    assertXPath(pDocument, "/primitive2D/transform/mask/polypolygon/polygon/point"_ostr, 20);
    assertXPath(pDocument, "/primitive2D/transform/mask/mask/polypolygon/polygon/point"_ostr, 13);
}

CPPUNIT_TEST_FIXTURE(Test, testFillRule)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/FillRule.svg");

    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor"_ostr, "color"_ostr, u"#000000"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor/polypolygon/polygon"_ostr, 2);
    assertXPath(pDocument, "/primitive2D/transform/polypolygonstroke/line"_ostr, "color"_ostr, u"#ff0000"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/polypolygonstroke/polypolygon/polygon"_ostr, 2);
}

CPPUNIT_TEST_FIXTURE(Test, testClipRule)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/ClipRule.svg");

    // Without the place in place, this test would have failed with
    // - Expected: 5
    // - Actual  : 10
    assertXPath(pDocument, "/primitive2D/transform/mask[1]/polypolygon/polygon/point"_ostr, 5);
    assertXPath(pDocument, "/primitive2D/transform/mask[1]/polypolygoncolor"_ostr, "color"_ostr, u"#0000ff"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/mask[1]/polypolygoncolor/polypolygon/polygon/point"_ostr, 4);

    assertXPath(pDocument, "/primitive2D/transform/mask[2]/polypolygon/polygon/point"_ostr, 5);
    assertXPath(pDocument, "/primitive2D/transform/mask[2]/polypolygoncolor"_ostr, "color"_ostr, u"#ff0000"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/mask[2]/polypolygoncolor/polypolygon/polygon/point"_ostr, 4);
}

CPPUNIT_TEST_FIXTURE(Test, testi125329)
{
    //Check style inherit from * css element
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/i125329.svg");

    assertXPath(pDocument, "/primitive2D/transform/transform/objectinfo/polypolygoncolor"_ostr, "color"_ostr, u"#c0c0c0"_ustr); // rect background color
    assertXPath(pDocument, "/primitive2D/transform/transform/objectinfo/polypolygoncolor/polypolygon"_ostr, "height"_ostr, u"30"_ustr); // rect background height
    assertXPath(pDocument, "/primitive2D/transform/transform/objectinfo/polypolygoncolor/polypolygon"_ostr, "width"_ostr, u"50"_ustr); // rect background width
    assertXPath(pDocument, "/primitive2D/transform/transform/objectinfo/polypolygoncolor/polypolygon"_ostr, "minx"_ostr, u"15"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/transform/objectinfo/polypolygoncolor/polypolygon"_ostr, "miny"_ostr, u"15"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/transform/objectinfo/polypolygoncolor/polypolygon"_ostr, "maxx"_ostr, u"65"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/transform/objectinfo/polypolygoncolor/polypolygon"_ostr, "maxy"_ostr, u"45"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/transform/objectinfo/polypolygonstroke/line"_ostr, "color"_ostr, u"#008000"_ustr); // rect stroke color
    assertXPath(pDocument, "/primitive2D/transform/transform/objectinfo/polypolygonstroke/line"_ostr, "width"_ostr, u"1"_ustr); // rect stroke width
}

CPPUNIT_TEST_FIXTURE(Test, testMaskingPath07b)
{
    //For the time being, check that masking-path-07-b.svg can be imported and it doesn't hang on loading
    //it used to hang after d5649ae7b76278cb3155f951d6327157c7c92b65
    dumpAndParseSvg(u"/svgio/qa/cppunit/data/masking-path-07-b.svg");
}

CPPUNIT_TEST_FIXTURE(Test, test123926)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf123926.svg");

    assertXPath(pDocument, "/primitive2D/transform/transform/transform/unifiedtransparence/polypolygoncolor"_ostr, "color"_ostr, u"#7cb5ec"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, test47446)
{
    //Check that marker's fill attribute is black is not set
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/47446.svg");

    assertXPath(pDocument, "/primitive2D/transform/transform/transform/polypolygoncolor"_ostr, "color"_ostr, u"#000000"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, test47446b)
{
    //Check that marker's fill attribute is inherit from def
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/47446b.svg");

    assertXPath(pDocument, "/primitive2D/transform/transform/transform/polypolygoncolor"_ostr, "color"_ostr, u"#ffff00"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf103888)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf103888.svg");

    // Without the fix in place, this test would have failed here with number of nodes is incorrect
    assertXPath(pDocument, "/primitive2D/transform/transform/textsimpleportion[1]"_ostr, "text"_ostr, u"Her"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/transform/textsimpleportion[2]"_ostr, "text"_ostr, u"vor"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/transform/textsimpleportion[3]"_ostr, "text"_ostr, u"hebung"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf156251)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf156251.svg");

    // Without the fix in place, this test would have failed with
    // - Expected: 'You are '
    // - Actual  : 'You are'
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "text"_ostr, u"You are "_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "text"_ostr, u"not"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[3]"_ostr, "text"_ostr, u" a banana!"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[4]"_ostr, "text"_ostr, u"You are"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[5]"_ostr, "text"_ostr, u" not "_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[6]"_ostr, "text"_ostr, u"a banana!"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testMaskText)
{
    //Check that mask is applied on text
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/maskText.svg");

    assertXPath(pDocument, "/primitive2D/transform/transform/polypolygoncolor"_ostr, "color"_ostr, u"#000000"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/transform/transform/textsimpleportion"_ostr, "fontcolor"_ostr, u"#ffffff"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/transform/transform/textsimpleportion"_ostr, "text"_ostr, u"Black White"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/transform/transform/textsimpleportion"_ostr, "height"_ostr, u"26"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/transform/transform/textsimpleportion"_ostr, "familyname"_ostr, u"Times New Roman"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf99994)
{
    //Check text fontsize when using relative units
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf99994.svg");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "fontcolor"_ostr, u"#0000ff"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "height"_ostr, u"16"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "text"_ostr, u"test"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "familyname"_ostr, u"DejaVu Sans"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf99115)
{
    //Check that styles are resolved correctly where there is a * css selector
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf99115.svg");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "text"_ostr, u"red 1"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "fontcolor"_ostr, u"#ff0000"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "height"_ostr, u"18"_ustr);

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "text"_ostr, u"red 2"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "fontcolor"_ostr, u"#ff0000"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "height"_ostr, u"18"_ustr);

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[3]"_ostr, "text"_ostr, u"red 3"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[3]"_ostr, "fontcolor"_ostr, u"#ff0000"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[3]"_ostr, "height"_ostr, u"18"_ustr);

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[4]"_ostr, "text"_ostr, u"blue 4"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[4]"_ostr, "fontcolor"_ostr, u"#0000ff"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[4]"_ostr, "height"_ostr, u"18"_ustr);

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[5]"_ostr, "text"_ostr, u"blue 5"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[5]"_ostr, "fontcolor"_ostr, u"#0000ff"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[5]"_ostr, "height"_ostr, u"18"_ustr);

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[6]"_ostr, "text"_ostr, u"blue 6"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[6]"_ostr, "fontcolor"_ostr, u"#0000ff"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[6]"_ostr, "height"_ostr, u"18"_ustr);

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[7]"_ostr, "text"_ostr, u"green 7"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[7]"_ostr, "fontcolor"_ostr, u"#008000"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[7]"_ostr, "height"_ostr, u"18"_ustr);

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[8]"_ostr, "text"_ostr, u"green 8"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[8]"_ostr, "fontcolor"_ostr, u"#008000"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[8]"_ostr, "height"_ostr, u"18"_ustr);

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[9]"_ostr, "text"_ostr, u"green 9"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[9]"_ostr, "fontcolor"_ostr, u"#008000"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[9]"_ostr, "height"_ostr, u"18"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf101237)
{
    //Check that fill color, stroke color and stroke-width are inherited from use element
    //when the element is within a clipPath element
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf101237.svg");

    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor"_ostr, "color"_ostr, u"#ff0000"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/polypolygonstroke/line"_ostr, "color"_ostr, u"#000000"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/polypolygonstroke/line"_ostr, "width"_ostr, u"5"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf97710)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf97710.svg");

    assertXPath(pDocument, "/primitive2D/transform/mask/polypolygoncolor[1]"_ostr, "color"_ostr, u"#000000"_ustr);

    // Without the fix in place, this test would have failed with
    // - Expected: 100
    // - Actual  : 0
    assertXPath(pDocument, "/primitive2D/transform/mask/polypolygoncolor[1]/polypolygon"_ostr, "width"_ostr, u"100"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/mask/polypolygoncolor[1]/polypolygon"_ostr, "height"_ostr, u"100"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/mask/polypolygoncolor[2]"_ostr, "color"_ostr, u"#008000"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/mask/polypolygoncolor[2]/polypolygon"_ostr, "width"_ostr, u"100"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/mask/polypolygoncolor[2]/polypolygon"_ostr, "height"_ostr, u"100"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/mask/polypolygonstroke/line"_ostr, "color"_ostr, u"#000000"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/mask/polypolygonstroke/line"_ostr, "width"_ostr, u"1"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf94765)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf94765.svg");

    //Check that both rectangles use the gradient as fill
    assertXPath(pDocument, "/primitive2D/transform/transform/svglineargradient[1]"_ostr, "startx"_ostr, u"1"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/transform/svglineargradient[1]"_ostr, "starty"_ostr, u"1"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/transform/svglineargradient[1]"_ostr, "endx"_ostr, u"2"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/transform/svglineargradient[1]"_ostr, "endy"_ostr, u"1"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/transform/svglineargradient[2]"_ostr, "startx"_ostr, u"0"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/transform/svglineargradient[2]"_ostr, "starty"_ostr, u"0"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/transform/svglineargradient[2]"_ostr, "endx"_ostr, u"1"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/transform/svglineargradient[2]"_ostr, "endy"_ostr, u"0"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf156236)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf156236.svg");

    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[1]/polypolygon"_ostr, "path"_ostr, u"m50 180h-30v-60h60v60z"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[2]/polypolygon"_ostr, "path"_ostr, u"m150 180h15c8.2842712474619 0 15-6.7157287525381 15-15v-30c0-8.2842712474619-6.7157287525381-15-15-15h-30c-8.2842712474619 0-15 6.7157287525381-15 15v30c0 8.2842712474619 6.7157287525381 15 15 15z"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[3]/polypolygon"_ostr, "path"_ostr, u"m250 180h15c8.2842712474619 0 15-6.7157287525381 15-15v-30c0-8.2842712474619-6.7157287525381-15-15-15h-30c-8.2842712474619 0-15 6.7157287525381-15 15v30c0 8.2842712474619 6.7157287525381 15 15 15z"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[4]/polypolygon"_ostr, "path"_ostr, u"m350 180c16.5685424949238 0 30-6.7157287525381 30-15v-30c0-8.2842712474619-13.4314575050762-15-30-15s-30 6.7157287525381-30 15v30c0 8.2842712474619 13.4314575050762 15 30 15z"_ustr);
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
        Primitive2DSequence aSequence = parseSvg(u"svgio/qa/cppunit/data/Drawing_WithWidthHeight.svg");
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
        Primitive2DSequence aSequence = parseSvg(u"svgio/qa/cppunit/data/Drawing_NoWidthHeight.svg");
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
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf155733.svg");

    assertXPath(pDocument, "/primitive2D/transform/softedge"_ostr, "radius"_ostr, u"5"_ustr);

    // Without the fix in place, the softedge would have been applied to the second element
    // - Expected: 1
    // - Actual  : 0
    assertXPath(pDocument, "/primitive2D/transform/transform/unifiedtransparence"_ostr, "transparence"_ostr, u"50"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf158445)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf158445.svg");

    assertXPath(pDocument, "/primitive2D/transform/mask/transform/transform/transform/polypolygoncolor"_ostr, "color"_ostr, u"#000000"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/mask/transform/transform/transform/polypolygoncolor/polypolygon"_ostr, "height"_ostr, u"8.052"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/mask/transform/transform/transform/polypolygoncolor/polypolygon"_ostr, "width"_ostr, u"5.328"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf159594)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf159594.svg");

    assertXPath(pDocument, "/primitive2D/transform/transform/polypolygoncolor"_ostr, "color"_ostr, u"#000000"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/transform/polypolygoncolor/polypolygon"_ostr, "height"_ostr, u"11.671875"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/transform/polypolygoncolor/polypolygon"_ostr, "width"_ostr, u"7.5"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf97663)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/em_units.svg");

    // tdf#97663: Without the fix in place, this test would have failed with
    // - Expected: 236
    // - Actual  : 204
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "y"_ostr, u"237"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf156269)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf156269.svg");

    assertXPath(pDocument, "//textsimpleportion[@text='one']"_ostr, "width"_ostr, u"16"_ustr);
    assertXPath(pDocument, "//textsimpleportion[@text='one']"_ostr, "height"_ostr, u"16"_ustr);
    assertXPath(pDocument, "//textsimpleportion[@text='one']"_ostr, "x"_ostr, u"10"_ustr);
    assertXPath(pDocument, "//textsimpleportion[@text='one']"_ostr, "y"_ostr, u"50"_ustr);
    assertXPath(pDocument, "//textsimpleportion[@text='one']"_ostr, "fontcolor"_ostr, u"#808080"_ustr);

    assertXPath(pDocument, "//textsimpleportion[@text='two']"_ostr, "width"_ostr, u"16"_ustr);
    assertXPath(pDocument, "//textsimpleportion[@text='two']"_ostr, "height"_ostr, u"16"_ustr);

    // Without the fix in place, this test would have failed with
    // - Expected: 60
    // - Actual  : 10
    assertXPath(pDocument, "//textsimpleportion[@text='two']"_ostr, "x"_ostr, u"60"_ustr);
    assertXPath(pDocument, "//textsimpleportion[@text='two']"_ostr, "y"_ostr, u"100"_ustr);
    assertXPath(pDocument, "//textsimpleportion[@text='two']"_ostr, "fontcolor"_ostr, u"#000000"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf95400)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf95400.svg");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "width"_ostr, u"16"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "height"_ostr, u"16"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "x"_ostr, u"30"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "y"_ostr, u"20"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "text"_ostr, u"ABC"_ustr);
    assertXPathDoubleValue(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "dx0"_ostr, 36, 0.5);
    assertXPathDoubleValue(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "dx1"_ostr, 69, 0.5);
    assertXPathDoubleValue(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "dx2"_ostr, 102, 0.5);

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "width"_ostr, u"50"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "height"_ostr, u"16"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "x"_ostr, u"30"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "y"_ostr, u"30"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "text"_ostr, u"ABC"_ustr);
    assertXPathNoAttribute(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "dx0"_ostr);
}

CPPUNIT_TEST_FIXTURE(Test, testTextAnchor)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf151103.svg");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "x"_ostr, u"60"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "y"_ostr, u"40"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "text"_ostr, u"ABC"_ustr);

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "x"_ostr, u"43"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "y"_ostr, u"50"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "text"_ostr, u"ABC"_ustr);

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[3]"_ostr, "x"_ostr, u"26"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[3]"_ostr, "y"_ostr, u"60"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[3]"_ostr, "text"_ostr, u"ABC"_ustr);

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[4]"_ostr, "x"_ostr, u"60"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[4]"_ostr, "y"_ostr, u"40"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[4]"_ostr, "text"_ostr, u"ABC"_ustr);

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[5]"_ostr, "x"_ostr, u"43"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[5]"_ostr, "y"_ostr, u"50"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[5]"_ostr, "text"_ostr, u"ABC"_ustr);

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[6]"_ostr, "x"_ostr, u"26"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[6]"_ostr, "y"_ostr, u"60"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[6]"_ostr, "text"_ostr, u"ABC"_ustr);

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[7]"_ostr, "x"_ostr, u"60"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[7]"_ostr, "y"_ostr, u"40"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[7]"_ostr, "text"_ostr, u"ABC"_ustr);

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[8]"_ostr, "x"_ostr, u"43"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[8]"_ostr, "y"_ostr, u"50"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[8]"_ostr, "text"_ostr, u"ABC"_ustr);

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[9]"_ostr, "x"_ostr, u"26"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[9]"_ostr, "y"_ostr, u"60"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[9]"_ostr, "text"_ostr, u"ABC"_ustr);

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[10]"_ostr, "x"_ostr, u"60"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[10]"_ostr, "y"_ostr, u"40"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[10]"_ostr, "text"_ostr, u"A"_ustr);

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[11]"_ostr, "x"_ostr, u"72"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[11]"_ostr, "y"_ostr, u"40"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[11]"_ostr, "text"_ostr, u"B"_ustr);

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[12]"_ostr, "x"_ostr, u"82"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[12]"_ostr, "y"_ostr, u"40"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[12]"_ostr, "text"_ostr, u"C"_ustr);

    // Without the fix in place, this test would have failed with
    // - Expected: 43
    // - Actual  : 54
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[13]"_ostr, "x"_ostr, u"43"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[13]"_ostr, "y"_ostr, u"50"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[13]"_ostr, "text"_ostr, u"A"_ustr);

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[14]"_ostr, "x"_ostr, u"55"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[14]"_ostr, "y"_ostr, u"50"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[14]"_ostr, "text"_ostr, u"B"_ustr);

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[15]"_ostr, "x"_ostr, u"65"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[15]"_ostr, "y"_ostr, u"50"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[15]"_ostr, "text"_ostr, u"C"_ustr);

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[16]"_ostr, "x"_ostr, u"26"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[16]"_ostr, "y"_ostr, u"60"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[16]"_ostr, "text"_ostr, u"A"_ustr);

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[17]"_ostr, "x"_ostr, u"38"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[17]"_ostr, "y"_ostr, u"60"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[17]"_ostr, "text"_ostr, u"B"_ustr);

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[18]"_ostr, "x"_ostr, u"48"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[18]"_ostr, "y"_ostr, u"60"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[18]"_ostr, "text"_ostr, u"C"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf156577)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf156577.svg");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "width"_ostr, u"16"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "height"_ostr, u"16"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "x"_ostr, u"30"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "y"_ostr, u"20"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "text"_ostr, u"ABC"_ustr);
    assertXPathNoAttribute(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "dx0"_ostr);

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "width"_ostr, u"16"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "height"_ostr, u"16"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "x"_ostr, u"30"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "y"_ostr, u"30"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "text"_ostr, u"ABC"_ustr);

    // Without the fix in place, this test would have failed with
    // - Expected: 22
    // - Actual  : 52
    assertXPathDoubleValue(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "dx0"_ostr, 22, 0.5);
    assertXPathDoubleValue(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "dx1"_ostr, 52, 0.5);
    assertXPathDoubleValue(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "dx2"_ostr, 93, 0.5);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf156283)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf156283.svg");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "width"_ostr, u"16"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "height"_ostr, u"16"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "x"_ostr, u"30"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "y"_ostr, u"20"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "text"_ostr, u"ABC"_ustr);
    assertXPathDoubleValue(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "dx0"_ostr, 41, 0.5);
    assertXPathDoubleValue(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "dx1"_ostr, 52, 0.5);
    assertXPathDoubleValue(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "dx2"_ostr, 62, 0.5);

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "width"_ostr, u"16"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "height"_ostr, u"16"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "x"_ostr, u"30"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "y"_ostr, u"30"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "text"_ostr, u"ABC"_ustr);

    // Without the fix in place, this test would have failed with
    // - Expected: 41
    // - Actual  : 12
    assertXPathDoubleValue(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "dx0"_ostr, 41, 0.5);
    assertXPathDoubleValue(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "dx1"_ostr, 51, 0.5);
    assertXPathDoubleValue(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "dx2"_ostr, 62, 0.5);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf156569)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf156569.svg");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "width"_ostr, u"16"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "height"_ostr, u"16"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "x"_ostr, u"0"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "y"_ostr, u"20"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "text"_ostr, u"ABC"_ustr);
    assertXPathDoubleValue(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "dx0"_ostr, 40, 0.5);
    assertXPathDoubleValue(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "dx1"_ostr, 80, 0.5);
    assertXPathDoubleValue(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "dx2"_ostr, 91, 0.5);

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "width"_ostr, u"16"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "height"_ostr, u"16"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "x"_ostr, u"0"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "y"_ostr, u"30"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "text"_ostr, u"ABC"_ustr);
    assertXPathDoubleValue(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "dx0"_ostr, 40, 0.5);

    // Without the fix in place, this test would have failed with
    // - Expected: 80
    // - Actual  : 51
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "dx1"_ostr, u"80"_ustr);
    assertXPathDoubleValue(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "dx2"_ostr, 91, 0.5);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf156837)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf156837.svg");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion"_ostr, 2);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "x"_ostr, u"114"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "y"_ostr, u"103"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "height"_ostr, u"16"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "text"_ostr, u"x "_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "x"_ostr, u"126"_ustr);

    // Without the fix in place, this test would have failed with
    // - Expected: 94
    // - Actual  : 103
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "y"_ostr, u"94"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "height"_ostr, u"10"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]"_ostr, "text"_ostr, u"3"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf160773)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf160773.svg");

    // tdf#160773 Check there is a rectangle
    assertXPath(pDocument, "/primitive2D/transform/mask/transform/polypolygoncolor"_ostr, "color"_ostr, u"#ff0000"_ustr);

    // tdf#159661 Check there is text in the right position
    assertXPath(pDocument, "/primitive2D/transform/mask/transform/textsimpleportion"_ostr, 1);
    assertXPath(pDocument, "/primitive2D/transform/mask/transform/textsimpleportion"_ostr, "x"_ostr, u"0"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/mask/transform/textsimpleportion"_ostr, "y"_ostr, u"1"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/mask/transform/textsimpleportion"_ostr, "height"_ostr, u"0"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/mask/transform/textsimpleportion"_ostr, "width"_ostr, u"0"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/mask/transform/textsimpleportion"_ostr, "text"_ostr, u"Red"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf156271)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf156271.svg");

    assertXPath(pDocument, "/primitive2D/transform/mask/textsimpleportion[1]"_ostr, "width"_ostr, u"16"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/mask/textsimpleportion[1]"_ostr, "height"_ostr, u"16"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/mask/textsimpleportion[1]"_ostr, "x"_ostr, u"40"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/mask/textsimpleportion[1]"_ostr, "y"_ostr, u"10"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/mask/textsimpleportion[1]"_ostr, "text"_ostr, u"AB"_ustr);
    assertXPathDoubleValue(pDocument, "/primitive2D/transform/mask/textsimpleportion[1]"_ostr, "dx0"_ostr, -30, 0.5);
    assertXPathDoubleValue(pDocument, "/primitive2D/transform/mask/textsimpleportion[1]"_ostr, "dx1"_ostr, -19, 0.5);

    assertXPath(pDocument, "/primitive2D/transform/mask/textsimpleportion[2]"_ostr, "width"_ostr, u"16"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/mask/textsimpleportion[2]"_ostr, "height"_ostr, u"16"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/mask/textsimpleportion[2]"_ostr, "x"_ostr, u"40"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/mask/textsimpleportion[2]"_ostr, "y"_ostr, u"20"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/mask/textsimpleportion[2]"_ostr, "text"_ostr, u"AB"_ustr);

    // Without the fix in place, this test would have failed with
    // - Expected: -30
    // - Actual  : 0
    assertXPathDoubleValue(pDocument, "/primitive2D/transform/mask/textsimpleportion[2]"_ostr, "dx0"_ostr, -30, 0.5);
    assertXPathDoubleValue(pDocument, "/primitive2D/transform/mask/textsimpleportion[2]"_ostr, "dx1"_ostr, -19, 0.5);

    assertXPath(pDocument, "/primitive2D/transform/mask/textsimpleportion[3]"_ostr, "width"_ostr, u"16"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/mask/textsimpleportion[3]"_ostr, "height"_ostr, u"16"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/mask/textsimpleportion[3]"_ostr, "x"_ostr, u"40"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/mask/textsimpleportion[3]"_ostr, "y"_ostr, u"30"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/mask/textsimpleportion[3]"_ostr, "text"_ostr, u"AB"_ustr);
    assertXPathDoubleValue(pDocument, "/primitive2D/transform/mask/textsimpleportion[3]"_ostr, "dx0"_ostr, -30, 0.5);
    assertXPathDoubleValue(pDocument, "/primitive2D/transform/mask/textsimpleportion[3]"_ostr, "dx1"_ostr, -19, 0.5);

    assertXPath(pDocument, "/primitive2D/transform/mask/textsimpleportion[4]"_ostr, "width"_ostr, u"16"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/mask/textsimpleportion[4]"_ostr, "height"_ostr, u"16"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/mask/textsimpleportion[4]"_ostr, "x"_ostr, u"40"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/mask/textsimpleportion[4]"_ostr, "y"_ostr, u"40"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/mask/textsimpleportion[4]"_ostr, "text"_ostr, u"AB"_ustr);
    assertXPathDoubleValue(pDocument, "/primitive2D/transform/mask/textsimpleportion[4]"_ostr, "dx0"_ostr, 12, 0.5);
    assertXPathDoubleValue(pDocument, "/primitive2D/transform/mask/textsimpleportion[4]"_ostr, "dx1"_ostr, 22, 0.5);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf159968)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf159968.svg");

    // Check no mask is applied to the marker
    assertXPath(pDocument,
            "/primitive2D/transform/transform/transform/transform/polypolygoncolor"_ostr, "color"_ostr, u"#000000"_ustr);
    assertXPath(pDocument,
            "/primitive2D/transform/transform/transform/transform/polypolygoncolor/polypolygon/polygon/point"_ostr, 5);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf160517)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf160517.svg");

    assertXPath(pDocument,
            "/primitive2D/transform/bitmap"_ostr, "height"_ostr, u"110"_ustr);
    assertXPath(pDocument,
            "/primitive2D/transform/bitmap"_ostr, "width"_ostr, u"110"_ustr);
    assertXPath(pDocument,
            "/primitive2D/transform/bitmap/data"_ostr, 110);

    assertXPath(pDocument,
            "/primitive2D/transform/bitmap"_ostr, "xy11"_ostr, u"110"_ustr);
    assertXPath(pDocument,
            "/primitive2D/transform/bitmap"_ostr, "xy12"_ostr, u"0"_ustr);
    assertXPath(pDocument,
            "/primitive2D//transform/bitmap"_ostr, "xy13"_ostr, u"0"_ustr);
    assertXPath(pDocument,
            "/primitive2D//transform/bitmap"_ostr, "xy21"_ostr, u"0"_ustr);
    assertXPath(pDocument,
            "/primitive2D//transform/bitmap"_ostr, "xy22"_ostr, u"110"_ustr);
    assertXPath(pDocument,
            "/primitive2D//transform/bitmap"_ostr, "xy23"_ostr, u"0"_ustr);
    assertXPath(pDocument,
            "/primitive2D//transform/bitmap"_ostr, "xy31"_ostr, u"0"_ustr);
    assertXPath(pDocument,
            "/primitive2D//transform/bitmap"_ostr, "xy32"_ostr, u"0"_ustr);
    assertXPath(pDocument,
            "/primitive2D//transform/bitmap"_ostr, "xy33"_ostr, u"1"_ustr);

    // Check the color of a pixel in the middle
    const OUString sDataRow = getXPath(pDocument, "/primitive2D/transform/bitmap/data[55]"_ostr, "row"_ostr);
    std::vector<OUString> aPixels = comphelper::string::split(sDataRow, ',');
    CPPUNIT_ASSERT_EQUAL(u"008100"_ustr, aPixels[55]);
}

CPPUNIT_TEST_FIXTURE(Test, testArithmeticComposite)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/arithmetic.svg");

    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap"_ostr, "height"_ostr, u"150"_ustr);
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap"_ostr, "width"_ostr, u"150"_ustr);
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap/data"_ostr, 150);

    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap"_ostr, "xy11"_ostr, u"150"_ustr);
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap"_ostr, "xy12"_ostr, u"0"_ustr);
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap"_ostr, "xy13"_ostr, u"0"_ustr);
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap"_ostr, "xy21"_ostr, u"0"_ustr);
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap"_ostr, "xy22"_ostr, u"150"_ustr);
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap"_ostr, "xy23"_ostr, u"0"_ustr);
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap"_ostr, "xy31"_ostr, u"0"_ustr);
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap"_ostr, "xy32"_ostr, u"0"_ustr);
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap"_ostr, "xy33"_ostr, u"1"_ustr);

    // Check the colors in the diagonal
    OUString sDataRow = getXPath(pDocument, "/primitive2D/transform/transform/bitmap/data[25]"_ostr, "row"_ostr);
    std::vector<OUString> aPixels = comphelper::string::split(sDataRow, ',');
    CPPUNIT_ASSERT_EQUAL(u"000000"_ustr, aPixels[25]);

    sDataRow = getXPath(pDocument, "/primitive2D/transform/transform/bitmap/data[75]"_ostr, "row"_ostr);
    aPixels = comphelper::string::split(sDataRow, ',');
    CPPUNIT_ASSERT_EQUAL(u"ff8000"_ustr, aPixels[75]);

    sDataRow = getXPath(pDocument, "/primitive2D/transform/transform/bitmap/data[125]"_ostr, "row"_ostr);
    aPixels = comphelper::string::split(sDataRow, ',');
    CPPUNIT_ASSERT_EQUAL(u"ff8000"_ustr, aPixels[125]);
}

CPPUNIT_TEST_FIXTURE(Test, testArithmeticComposite2)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/arithmetic2.svg");

    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap"_ostr, "height"_ostr, u"150"_ustr);
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap"_ostr, "width"_ostr, u"150"_ustr);
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap/data"_ostr, 150);

    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap"_ostr, "xy11"_ostr, u"150"_ustr);
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap"_ostr, "xy12"_ostr, u"0"_ustr);
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap"_ostr, "xy13"_ostr, u"0"_ustr);
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap"_ostr, "xy21"_ostr, u"0"_ustr);
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap"_ostr, "xy22"_ostr, u"150"_ustr);
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap"_ostr, "xy23"_ostr, u"0"_ustr);
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap"_ostr, "xy31"_ostr, u"0"_ustr);
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap"_ostr, "xy32"_ostr, u"0"_ustr);
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap"_ostr, "xy33"_ostr, u"1"_ustr);

    // Check the colors in the diagonal
    OUString sDataRow = getXPath(pDocument, "/primitive2D/transform/transform/bitmap/data[25]"_ostr, "row"_ostr);
    std::vector<OUString> aPixels = comphelper::string::split(sDataRow, ',');
    CPPUNIT_ASSERT_EQUAL(u"ff0000"_ustr, aPixels[25]);

    sDataRow = getXPath(pDocument, "/primitive2D/transform/transform/bitmap/data[75]"_ostr, "row"_ostr);
    aPixels = comphelper::string::split(sDataRow, ',');
    CPPUNIT_ASSERT_EQUAL(u"ff8000"_ustr, aPixels[75]);

    sDataRow = getXPath(pDocument, "/primitive2D/transform/transform/bitmap/data[125]"_ostr, "row"_ostr);
    aPixels = comphelper::string::split(sDataRow, ',');
    CPPUNIT_ASSERT_EQUAL(u"008000"_ustr, aPixels[125]);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf160726)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf160726.svg");

    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap"_ostr, "height"_ostr, u"250"_ustr);
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap"_ostr, "width"_ostr, u"250"_ustr);
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap/data"_ostr, 250);

    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap"_ostr, "xy11"_ostr, u"250"_ustr);
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap"_ostr, "xy12"_ostr, u"0"_ustr);
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap"_ostr, "xy13"_ostr, u"0"_ustr);
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap"_ostr, "xy21"_ostr, u"0"_ustr);
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap"_ostr, "xy22"_ostr, u"250"_ustr);
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap"_ostr, "xy23"_ostr, u"0"_ustr);
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap"_ostr, "xy31"_ostr, u"0"_ustr);
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap"_ostr, "xy32"_ostr, u"0"_ustr);
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap"_ostr, "xy33"_ostr, u"1"_ustr);

    // Check the colors in the diagonal
    OUString sDataRow = getXPath(pDocument, "/primitive2D/transform/transform/bitmap/data[50]"_ostr, "row"_ostr);
    std::vector<OUString> aPixels = comphelper::string::split(sDataRow, ',');
    CPPUNIT_ASSERT_EQUAL(u"ff0000"_ustr, aPixels[50]);

    sDataRow = getXPath(pDocument, "/primitive2D/transform/transform/bitmap/data[125]"_ostr, "row"_ostr);
    aPixels = comphelper::string::split(sDataRow, ',');
    CPPUNIT_ASSERT_EQUAL(u"ffff00"_ustr, aPixels[125]);

    sDataRow = getXPath(pDocument, "/primitive2D/transform/transform/bitmap/data[200]"_ostr, "row"_ostr);
    aPixels = comphelper::string::split(sDataRow, ',');
    CPPUNIT_ASSERT_EQUAL(u"ffff00"_ustr, aPixels[200]);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf160782)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf160782.svg");

    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap"_ostr, "height"_ostr, u"255"_ustr);
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap"_ostr, "width"_ostr, u"255"_ustr);
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap/data"_ostr, 255);

    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap"_ostr, "xy11"_ostr, u"255"_ustr);
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap"_ostr, "xy12"_ostr, u"0"_ustr);
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap"_ostr, "xy13"_ostr, u"0"_ustr);
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap"_ostr, "xy21"_ostr, u"0"_ustr);
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap"_ostr, "xy22"_ostr, u"255"_ustr);
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap"_ostr, "xy23"_ostr, u"0"_ustr);
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap"_ostr, "xy31"_ostr, u"0"_ustr);
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap"_ostr, "xy32"_ostr, u"0"_ustr);
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap"_ostr, "xy33"_ostr, u"1"_ustr);

    // Check the colors in the diagonal
    OUString sDataRow = getXPath(pDocument, "/primitive2D/transform/transform/bitmap/data[50]"_ostr, "row"_ostr);
    std::vector<OUString> aPixels = comphelper::string::split(sDataRow, ',');
    CPPUNIT_ASSERT_EQUAL(u"ff0000"_ustr, aPixels[50]);

    sDataRow = getXPath(pDocument, "/primitive2D/transform/transform/bitmap/data[125]"_ostr, "row"_ostr);
    aPixels = comphelper::string::split(sDataRow, ',');
    CPPUNIT_ASSERT_EQUAL(u"ffff00"_ustr, aPixels[125]);

    sDataRow = getXPath(pDocument, "/primitive2D/transform/transform/bitmap/data[200]"_ostr, "row"_ostr);
    aPixels = comphelper::string::split(sDataRow, ',');
    CPPUNIT_ASSERT_EQUAL(u"ffff00"_ustr, aPixels[200]);
}

CPPUNIT_TEST_FIXTURE(Test, testScreenBlend)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/screenBlend.svg");

    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap"_ostr, "height"_ostr, u"150"_ustr);
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap"_ostr, "width"_ostr, u"150"_ustr);
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap/data"_ostr, 150);

    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap"_ostr, "xy11"_ostr, u"150"_ustr);
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap"_ostr, "xy12"_ostr, u"0"_ustr);
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap"_ostr, "xy13"_ostr, u"0"_ustr);
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap"_ostr, "xy21"_ostr, u"0"_ustr);
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap"_ostr, "xy22"_ostr, u"150"_ustr);
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap"_ostr, "xy23"_ostr, u"0"_ustr);
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap"_ostr, "xy31"_ostr, u"0"_ustr);
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap"_ostr, "xy32"_ostr, u"0"_ustr);
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap"_ostr, "xy33"_ostr, u"1"_ustr);

    // Check the colors in the diagonal
    OUString sDataRow = getXPath(pDocument, "/primitive2D/transform/transform/bitmap/data[25]"_ostr, "row"_ostr);
    std::vector<OUString> aPixels = comphelper::string::split(sDataRow, ',');
    CPPUNIT_ASSERT_EQUAL(u"ff0000"_ustr, aPixels[25]);

    sDataRow = getXPath(pDocument, "/primitive2D/transform/transform/bitmap/data[75]"_ostr, "row"_ostr);
    aPixels = comphelper::string::split(sDataRow, ',');
    CPPUNIT_ASSERT_EQUAL(u"ff8000"_ustr, aPixels[75]);

    sDataRow = getXPath(pDocument, "/primitive2D/transform/transform/bitmap/data[125]"_ostr, "row"_ostr);
    aPixels = comphelper::string::split(sDataRow, ',');
    CPPUNIT_ASSERT_EQUAL(u"008000"_ustr, aPixels[125]);
}

CPPUNIT_TEST_FIXTURE(Test, testMultiplyBlend)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/multiplyBlend.svg");

    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap"_ostr, "height"_ostr, u"150"_ustr);
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap"_ostr, "width"_ostr, u"150"_ustr);
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap/data"_ostr, 150);

    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap"_ostr, "xy11"_ostr, u"150"_ustr);
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap"_ostr, "xy12"_ostr, u"0"_ustr);
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap"_ostr, "xy13"_ostr, u"0"_ustr);
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap"_ostr, "xy21"_ostr, u"0"_ustr);
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap"_ostr, "xy22"_ostr, u"150"_ustr);
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap"_ostr, "xy23"_ostr, u"0"_ustr);
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap"_ostr, "xy31"_ostr, u"0"_ustr);
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap"_ostr, "xy32"_ostr, u"0"_ustr);
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap"_ostr, "xy33"_ostr, u"1"_ustr);

    // Check the colors in the diagonal
    OUString sDataRow = getXPath(pDocument, "/primitive2D/transform/transform/bitmap/data[25]"_ostr, "row"_ostr);
    std::vector<OUString> aPixels = comphelper::string::split(sDataRow, ',');
    CPPUNIT_ASSERT_EQUAL(u"ff0000"_ustr, aPixels[25]);

    sDataRow = getXPath(pDocument, "/primitive2D/transform/transform/bitmap/data[75]"_ostr, "row"_ostr);
    aPixels = comphelper::string::split(sDataRow, ',');
    CPPUNIT_ASSERT_EQUAL(u"000000"_ustr, aPixels[75]);

    sDataRow = getXPath(pDocument, "/primitive2D/transform/transform/bitmap/data[125]"_ostr, "row"_ostr);
    aPixels = comphelper::string::split(sDataRow, ',');
    CPPUNIT_ASSERT_EQUAL(u"008000"_ustr, aPixels[125]);
}

CPPUNIT_TEST_FIXTURE(Test, testDarkenBlend)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/darkenBlend.svg");

    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap"_ostr, "height"_ostr, u"150"_ustr);
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap"_ostr, "width"_ostr, u"150"_ustr);
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap/data"_ostr, 150);

    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap"_ostr, "xy11"_ostr, u"150"_ustr);
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap"_ostr, "xy12"_ostr, u"0"_ustr);
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap"_ostr, "xy13"_ostr, u"0"_ustr);
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap"_ostr, "xy21"_ostr, u"0"_ustr);
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap"_ostr, "xy22"_ostr, u"150"_ustr);
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap"_ostr, "xy23"_ostr, u"0"_ustr);
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap"_ostr, "xy31"_ostr, u"0"_ustr);
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap"_ostr, "xy32"_ostr, u"0"_ustr);
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap"_ostr, "xy33"_ostr, u"1"_ustr);

    // Check the colors in the diagonal
    OUString sDataRow = getXPath(pDocument, "/primitive2D/transform/transform/bitmap/data[25]"_ostr, "row"_ostr);
    std::vector<OUString> aPixels = comphelper::string::split(sDataRow, ',');
    CPPUNIT_ASSERT_EQUAL(u"ff0000"_ustr, aPixels[25]);

    sDataRow = getXPath(pDocument, "/primitive2D/transform/transform/bitmap/data[75]"_ostr, "row"_ostr);
    aPixels = comphelper::string::split(sDataRow, ',');
    CPPUNIT_ASSERT_EQUAL(u"000000"_ustr, aPixels[75]);

    sDataRow = getXPath(pDocument, "/primitive2D/transform/transform/bitmap/data[125]"_ostr, "row"_ostr);
    aPixels = comphelper::string::split(sDataRow, ',');
    CPPUNIT_ASSERT_EQUAL(u"008000"_ustr, aPixels[125]);
}

CPPUNIT_TEST_FIXTURE(Test, testLightenBlend)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/lightenBlend.svg");

    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap"_ostr, "height"_ostr, u"150"_ustr);
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap"_ostr, "width"_ostr, u"150"_ustr);
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap/data"_ostr, 150);

    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap"_ostr, "xy11"_ostr, u"150"_ustr);
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap"_ostr, "xy12"_ostr, u"0"_ustr);
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap"_ostr, "xy13"_ostr, u"0"_ustr);
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap"_ostr, "xy21"_ostr, u"0"_ustr);
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap"_ostr, "xy22"_ostr, u"150"_ustr);
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap"_ostr, "xy23"_ostr, u"0"_ustr);
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap"_ostr, "xy31"_ostr, u"0"_ustr);
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap"_ostr, "xy32"_ostr, u"0"_ustr);
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap"_ostr, "xy33"_ostr, u"1"_ustr);

    // Check the colors in the diagonal
    OUString sDataRow = getXPath(pDocument, "/primitive2D/transform/transform/bitmap/data[25]"_ostr, "row"_ostr);
    std::vector<OUString> aPixels = comphelper::string::split(sDataRow, ',');
    CPPUNIT_ASSERT_EQUAL(u"ff0000"_ustr, aPixels[25]);

    sDataRow = getXPath(pDocument, "/primitive2D/transform/transform/bitmap/data[75]"_ostr, "row"_ostr);
    aPixels = comphelper::string::split(sDataRow, ',');
    CPPUNIT_ASSERT_EQUAL(u"ff8000"_ustr, aPixels[75]);

    sDataRow = getXPath(pDocument, "/primitive2D/transform/transform/bitmap/data[125]"_ostr, "row"_ostr);
    aPixels = comphelper::string::split(sDataRow, ',');
    CPPUNIT_ASSERT_EQUAL(u"008000"_ustr, aPixels[125]);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf149880)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf149880.svg");

    // Without the fix in place, this test would have failed with
    // - Expected: 1
    // - Actual  : 0
    // - In <>, XPath '/primitive2D/transform/mask/unhandled' number of nodes is incorrect
    assertXPath(pDocument,
            "/primitive2D/transform/mask/unhandled"_ostr, "id"_ostr, u"PATTERNFILL"_ustr);
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
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/CssClassRedefinition.svg");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "text"_ostr, u"012"_ustr);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "fontcolor"_ostr, u"#ff0000"_ustr);
    assertXPath(
        pDocument, "/primitive2D/transform/textsimpleportion[1]"_ostr, "familyname"_ostr, u"Open Symbol"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testTspanFillOpacity)
{
    // Given an SVG file with <tspan fill-opacity="0.30">:
    // When rendering that SVG:
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tspan-fill-opacity.svg");

    // Then make sure that the text portion is wrapped in a transparency primitive with the correct
    // transparency value:
    sal_Int32 nTransparence = getXPath(pDocument, "//textsimpleportion[@text='hello']/parent::unifiedtransparence"_ostr, "transparence"_ostr).toInt32();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // - XPath '//textsimpleportion[@text='hello']/parent::unifiedtransparence' number of nodes is incorrect
    // i.e. the relevant <textsimpleportion> had no <unifiedtransparence> parent, the text was not
    // semi-transparent.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(70), nTransparence);
}

CPPUNIT_TEST_FIXTURE(Test, testDyInEms)
{
    // tdf#160593 given an SVG file with <tspan dy="1.5em" style="font-size:0.5em">:
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/dy_in_ems.svg");

    assertXPath(pDocument, "//textsimpleportion"_ostr, 2);
    assertXPath(pDocument, "//textsimpleportion[1]"_ostr, "y"_ostr, u"20"_ustr);
    // Then make sure that the vertical offset is based on font-size of tspan, not of its parent.
    // Given the parent's font-size is 16 px, the expected vertical offset is 1.5 * (16 * 0.5) = 12,
    // which means that the resulting y is expected to be 20 + 12 = 32.
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 32
    // - Actual  : 44
    // i.e. the offset was calculated as 1.5 multiplied by the parent's font-size of 16 px,
    // not by the current tspan's half font-size.
    assertXPath(pDocument, "//textsimpleportion[2]"_ostr, "y"_ostr, u"32"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testExs)
{
    // tdf#160594, tdf#160717 given an SVG file with <tspan dy="3ex" style="font-size:1ex">:
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/dy_in_exs.svg");

    assertXPath(pDocument, "//textsimpleportion"_ostr, 2);
    assertXPath(pDocument, "//textsimpleportion[1]"_ostr, "height"_ostr, u"16"_ustr);
    assertXPath(pDocument, "//textsimpleportion[1]"_ostr, "y"_ostr, u"20"_ustr);

    sal_Int32 nSize = getXPath(pDocument, "//textsimpleportion[2]"_ostr, "height"_ostr).toInt32();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected less than: 16
    // - Actual  : 16
    // i.e. the parent font-size was used, instead of its x-size.
    CPPUNIT_ASSERT_LESS(sal_Int32(16), nSize);

    sal_Int32 nYPos = getXPath(pDocument, "//textsimpleportion[2]"_ostr, "y"_ostr).toInt32();
    // Then make sure that the vertical offset is based on x-size of tspan, not of its parent.
    // Given the tspan's font-size is nSize, its x-size is less than nSize, and the expected
    // vertical offset is less than 3 * nSize, which means that the resulting y is expected
    // to be strictly less than 20 + 3 * nSize.
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected less than: 44
    // - Actual  : 44
    // i.e. the parent x-size (or current font-size) was used, instead of current x-size.
    CPPUNIT_ASSERT_LESS(sal_Int32(20 + 3 * nSize), nYPos);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
