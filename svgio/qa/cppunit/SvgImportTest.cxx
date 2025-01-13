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

    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor", "color", u"#00cc00"); // rect background color
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor/polypolygon", "height", u"100"); // rect background height
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor/polypolygon", "width", u"100"); // rect background width
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor/polypolygon", "minx", u"10");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor/polypolygon", "miny", u"10");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor/polypolygon", "maxx", u"110");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor/polypolygon", "maxy", u"110");
    assertXPath(pDocument, "/primitive2D/transform/polypolygonstroke/line", "color", u"#ff0000"); // rect stroke color
    assertXPath(pDocument, "/primitive2D/transform/polypolygonstroke/line", "width", u"3"); // rect stroke width
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
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor", "color", u"#00d000");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf150124)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf150124.svg");

    assertXPathChildren(pDocument, "/primitive2D", 1);
    assertXPath(pDocument, "/primitive2D/hiddengeometry", 1);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf155819)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf155819.svg");

    assertXPath(pDocument, "/primitive2D/transform/polypolygonstroke/line", 1);
    assertXPath(pDocument, "/primitive2D/transform/polypolygonstroke/polypolygon", 1);
    // Without the fix in place, this test would have failed with
    // - Expected: 4
    // - Actual  : 0
    assertXPath(pDocument, "/primitive2D/transform/transform", 4);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf164434)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf164434.svg");

    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor", 1);
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor", "color", u"#000000");
    assertXPath(pDocument, "/primitive2D/transform/transform/polypolygoncolor", 0);
    assertXPath(pDocument, "/primitive2D/transform/transform/transform/polypolygoncolor", 1);
    assertXPath(pDocument, "/primitive2D/transform/transform/transform/polypolygoncolor", "color", u"#000000");
}

CPPUNIT_TEST_FIXTURE(Test, testNormalBlend)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/normalBlend.svg");

    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap", "height", u"170");
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap", "width", u"170");
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap/data", 170);

    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap", "xy11", u"170");
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap", "xy12", u"0");
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap", "xy13", u"0");
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap", "xy21", u"0");
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap", "xy22", u"170");
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap", "xy23", u"0");
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap", "xy31", u"0");
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap", "xy32", u"0");
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap", "xy33", u"1");

    // Check the colors in the diagonal
    OUString sDataRow = getXPath(pDocument, "/primitive2D/transform/transform/bitmap/data[40]", "row");
    std::vector<OUString> aPixels = comphelper::string::split(sDataRow, ',');
    CPPUNIT_ASSERT_EQUAL(u"ffd700"_ustr, aPixels[40]);

    sDataRow = getXPath(pDocument, "/primitive2D/transform/transform/bitmap/data[85]", "row");
    aPixels = comphelper::string::split(sDataRow, ',');
    CPPUNIT_ASSERT_EQUAL(u"ffd700"_ustr, aPixels[85]);

#if defined _WIN32 && defined _ARM64_
    // skip for windows arm64 build
#else
    sDataRow = getXPath(pDocument, "/primitive2D/transform/transform/bitmap/data[130]", "row");
    aPixels = comphelper::string::split(sDataRow, ',');
    CPPUNIT_ASSERT_EQUAL(u"8a2be2"_ustr, aPixels[130]);
#endif
}

CPPUNIT_TEST_FIXTURE(Test, testFeColorMatrix)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/filterFeColorMatrix.svg");

    assertXPath(pDocument, "/primitive2D/transform/mask/modifiedColor[1]", "modifier", u"matrix");
    assertXPath(pDocument, "/primitive2D/transform/mask/modifiedColor[2]", "modifier", u"saturate");
    assertXPath(pDocument, "/primitive2D/transform/mask/modifiedColor[3]", "modifier", u"hueRotate");
    assertXPath(pDocument, "/primitive2D/transform/mask/modifiedColor[4]", "modifier", u"luminance_to_alpha");
}

CPPUNIT_TEST_FIXTURE(Test, testFilterFeComposite)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/filterFeComposite.svg");

    assertXPath(pDocument, "/primitive2D/transform/mask", 5);
    // over operator
    assertXPath(pDocument, "/primitive2D/transform/mask[1]/polypolygoncolor", 3);
    assertXPath(pDocument, "/primitive2D/transform/mask[1]/polypolygon/polygon/point", 8);
    // xor operator
    assertXPath(pDocument, "/primitive2D/transform/mask[2]/polypolygoncolor", 3);
    assertXPath(pDocument, "/primitive2D/transform/mask[2]/polypolygon/polygon[1]/point", 8);
    assertXPath(pDocument, "/primitive2D/transform/mask[2]/polypolygon/polygon[2]/point", 4);
    // in operator
    assertXPath(pDocument, "/primitive2D/transform/mask[3]/polypolygoncolor", 3);
    assertXPath(pDocument, "/primitive2D/transform/mask[3]/polypolygon/polygon/point", 4);
    // out operator
    assertXPath(pDocument, "/primitive2D/transform/mask[4]/polypolygoncolor", 3);
    assertXPath(pDocument, "/primitive2D/transform/mask[4]/polypolygon/polygon/point", 6);
    // atop operator
    assertXPath(pDocument, "/primitive2D/transform/mask[5]/polypolygoncolor", 3);
    assertXPath(pDocument, "/primitive2D/transform/mask[5]/polypolygon/polygon[1]/point", 6);
    assertXPath(pDocument, "/primitive2D/transform/mask[5]/polypolygon/polygon[2]/point", 4);
}

CPPUNIT_TEST_FIXTURE(Test, testFilterFeGaussianBlur)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/filterFeGaussianBlur.svg");

    assertXPath(pDocument, "/primitive2D/transform/softedge", "radius", u"5");
}

CPPUNIT_TEST_FIXTURE(Test, testInFilterAttribute)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/inFilterAttribute.svg");

    // Without the fix in place, the feGaussianBlur and feColorMatrix filter would have been applied
    assertXPath(pDocument, "/primitive2D/transform/transform", "xy11", u"1");
    assertXPath(pDocument, "/primitive2D/transform/transform", "xy12", u"0");
    assertXPath(pDocument, "/primitive2D/transform/transform", "xy13", u"40");
    assertXPath(pDocument, "/primitive2D/transform/transform", "xy21", u"0");
    assertXPath(pDocument, "/primitive2D/transform/transform", "xy22", u"1");
    assertXPath(pDocument, "/primitive2D/transform/transform", "xy23", u"40");
    assertXPath(pDocument, "/primitive2D/transform/transform", "xy31", u"0");
    assertXPath(pDocument, "/primitive2D/transform/transform", "xy32", u"0");
    assertXPath(pDocument, "/primitive2D/transform/transform", "xy33", u"1");
    assertXPath(pDocument, "/primitive2D/transform/transform/polypolygoncolor", "color", u"#ffffff");
}

CPPUNIT_TEST_FIXTURE(Test, testResultFilterAttribute)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/resultFilterAttribute.svg");

    // Without the fix in place, the feColorMatrix filter would have been applied
    assertXPath(pDocument, "/primitive2D/transform/transform", "xy11", u"1");
    assertXPath(pDocument, "/primitive2D/transform/transform", "xy12", u"0");
    assertXPath(pDocument, "/primitive2D/transform/transform", "xy13", u"40");
    assertXPath(pDocument, "/primitive2D/transform/transform", "xy21", u"0");
    assertXPath(pDocument, "/primitive2D/transform/transform", "xy22", u"1");
    assertXPath(pDocument, "/primitive2D/transform/transform", "xy23", u"40");
    assertXPath(pDocument, "/primitive2D/transform/transform", "xy31", u"0");
    assertXPath(pDocument, "/primitive2D/transform/transform", "xy32", u"0");
    assertXPath(pDocument, "/primitive2D/transform/transform", "xy33", u"1");
    assertXPath(pDocument, "/primitive2D/transform/transform/softedge", "radius", u"2");
    assertXPath(pDocument, "/primitive2D/transform/transform/softedge/polypolygoncolor", "color", u"#ffffff");
}

CPPUNIT_TEST_FIXTURE(Test, testFilterFeMerge)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/filterFeMerge.svg");

    assertXPath(pDocument, "/primitive2D/transform/mask/softedge", 2);
    assertXPath(pDocument, "/primitive2D/transform/mask/polypolygon", 1);
    assertXPath(pDocument, "/primitive2D/transform/mask/polypolygoncolor", 1);
    assertXPath(pDocument, "/primitive2D/transform/mask/polypolygonstroke", 1);
}

CPPUNIT_TEST_FIXTURE(Test, testFilterFeOffset)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/filterFeOffset.svg");

    assertXPath(pDocument, "/primitive2D/transform/mask/transform", "xy11", u"1");
    assertXPath(pDocument, "/primitive2D/transform/mask/transform", "xy12", u"0");
    assertXPath(pDocument, "/primitive2D/transform/mask/transform", "xy13", u"44");
    assertXPath(pDocument, "/primitive2D/transform/mask/transform", "xy21", u"0");
    assertXPath(pDocument, "/primitive2D/transform/mask/transform", "xy22", u"1");
    assertXPath(pDocument, "/primitive2D/transform/mask/transform", "xy23", u"66");
    assertXPath(pDocument, "/primitive2D/transform/mask/transform", "xy31", u"0");
    assertXPath(pDocument, "/primitive2D/transform/mask/transform", "xy32", u"0");
    assertXPath(pDocument, "/primitive2D/transform/mask/transform", "xy33", u"1");
}

CPPUNIT_TEST_FIXTURE(Test, testFilterFeFlood)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/filterFeFlood.svg");

    assertXPath(pDocument, "/primitive2D/transform/polypolygonrgba", "transparence", u"50");
    assertXPath(pDocument, "/primitive2D/transform/polypolygonrgba", "color", u"#008000");
    assertXPath(pDocument, "/primitive2D/transform/polypolygonrgba/polypolygon", "height", u"100");
    assertXPath(pDocument, "/primitive2D/transform/polypolygonrgba/polypolygon", "width", u"100");
    assertXPath(pDocument, "/primitive2D/transform/polypolygonrgba/polypolygon", "minx", u"50");
    assertXPath(pDocument, "/primitive2D/transform/polypolygonrgba/polypolygon", "miny", u"50");
}

CPPUNIT_TEST_FIXTURE(Test, testFilterFeDropShadow)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/filterFeDropShadow.svg");

    assertXPath(pDocument, "/primitive2D/transform/unifiedtransparence", "transparence", u"50");
    assertXPath(pDocument, "/primitive2D/transform/unifiedtransparence/shadow", "color", u"#0000ff");
    assertXPath(pDocument, "/primitive2D/transform/unifiedtransparence/shadow", "blur", u"0.2");
    assertXPath(pDocument, "/primitive2D/transform/unifiedtransparence/shadow", "blur", u"0.2");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor", "color", u"#ffc0cb");
}

CPPUNIT_TEST_FIXTURE(Test, testFilterFeImage)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/filterFeImage.svg");

    assertXPath(pDocument, "/primitive2D/transform/transform/bitmap");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf87309)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf87309.svg");

    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor", "color", u"#000000");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor/polypolygon", "height", u"100");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor/polypolygon", "width", u"100");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor/polypolygon", "minx", u"10");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor/polypolygon", "miny", u"10");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor/polypolygon", "maxx", u"110");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor/polypolygon", "maxy", u"110");
}

CPPUNIT_TEST_FIXTURE(Test, testFontsizeKeywords)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/FontsizeKeywords.svg");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "fontcolor", u"#000000");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "text", u"Sample");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "height", u"9");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "familyname", u"Times New Roman");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]", "fontcolor", u"#ffffff");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]", "text", u"Sample");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]", "height", u"11");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]", "familyname", u"Times New Roman");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[3]", "fontcolor", u"#ffd700");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[3]", "text", u"Sample");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[3]", "height", u"13");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[3]", "familyname", u"Times New Roman");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[4]", "fontcolor", u"#ff0000");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[4]", "text", u"Sample");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[4]", "height", u"16");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[4]", "familyname", u"Times New Roman");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[5]", "fontcolor", u"#ffff00");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[5]", "text", u"Sample");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[5]", "height", u"19");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[5]", "familyname", u"Times New Roman");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[6]", "fontcolor", u"#0000ff");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[6]", "text", u"Sample");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[6]", "height", u"23");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[6]", "familyname", u"Times New Roman");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[7]", "fontcolor", u"#008000");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[7]", "text", u"Sample");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[7]", "height", u"28");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[7]", "familyname", u"Times New Roman");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[8]", "fontcolor", u"#ff7f50");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[8]", "text", u"Sample");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[8]", "height", u"13");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[8]", "familyname", u"Times New Roman");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[9]", "fontcolor", u"#ffc0cb");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[9]", "text", u"Sample");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[9]", "height", u"19");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[9]", "familyname", u"Times New Roman");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[10]", "fontcolor", u"#fffff0");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[10]", "text", u"Sample");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[10]", "height", u"16");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[9]", "familyname", u"Times New Roman");
}


CPPUNIT_TEST_FIXTURE(Test, testFontsizePercentage)
{
    //Check when font-size uses percentage and defined globally
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/FontsizePercentage.svg");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "fontcolor", u"#000000");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "text", u"Sample");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "height", u"16");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "familyname", u"Times New Roman");
}

CPPUNIT_TEST_FIXTURE(Test, testFontsizeRelative)
{
    //Check when font-size uses relative units (em,ex) and it's based on its parent's font-size
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/FontsizeRelative.svg");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "fontcolor", u"#000000");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "text", u"Sample");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "height", u"50");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "familyname", u"DejaVu Serif");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]", "fontcolor", u"#000000");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]", "text", u"Sample");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]", "height", u"50");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]", "familyname", u"DejaVu Serif");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf161985)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf161985.svg");

    // Without the fix in place, this test would have failed with
    // - Expected: 0
    // - Actual  : 1
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor", 0);

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion", 1);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion", "text", u"This is a test file.");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf160386)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf160386.svg");

    // Without the fix in place, this test would have failed with
    // - Expected: 1
    // - Actual  : 11
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion", 1);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion", "text", u"Hello!");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf145896)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf145896.svg");

    // Without the fix in place, this test would have failed with
    // - Expected: #ffff00
    // - Actual  : #000000
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[1]", "color", u"#ffff00");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[2]", "color", u"#008000");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[3]", "color", u"#0000ff");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf156579)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf156579.svg");

    // Without the fix in place, nothing would be displayed
    assertXPath(pDocument, "/primitive2D/transform/mask/transform/polypolygoncolor[1]", "color", u"#0000ff");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf156168)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf156168.svg");

    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor", 8);
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[1]", "color", u"#0000ff");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[2]", "color", u"#0000ff");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[3]", "color", u"#ff0000");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[4]", "color", u"#ff0000");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[5]", "color", u"#000000");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[6]", "color", u"#000000");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[7]", "color", u"#ff0000");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[8]", "color", u"#ff0000");

    // Without the fix in place, this test would have failed with
    // - Expected: 4
    // - Actual  : 3
    assertXPath(pDocument, "/primitive2D/transform/polypolygonstroke", 4);
    assertXPath(pDocument, "/primitive2D/transform/polypolygonstroke[1]/line", "width", u"5");

    assertXPath(pDocument, "/primitive2D/transform/polypolygonstroke[1]/line", "color", u"#00ff00");
    assertXPath(pDocument, "/primitive2D/transform/polypolygonstroke[2]/line", "width", u"5");

    assertXPath(pDocument, "/primitive2D/transform/polypolygonstroke[2]/line", "color", u"#00ff00");
    assertXPath(pDocument, "/primitive2D/transform/polypolygonstroke[3]/line", "width", u"5");

    assertXPath(pDocument, "/primitive2D/transform/polypolygonstroke[3]/line", "color", u"#00ff00");
    assertXPath(pDocument, "/primitive2D/transform/polypolygonstroke[4]/line", "width", u"5");

    assertXPath(pDocument, "/primitive2D/transform/polypolygonstroke[4]/line", "color", u"#00ff00");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf160373)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf160373.svg");

    // Without the fix in place, nothing would be displayed
    assertXPath(pDocument, "/primitive2D/transform/transform/polypolygoncolor", "color", u"#0000ff");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf129356)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf129356.svg");

    // Without the fix in place, this test would have failed with
    // - Expected: #008000
    // - Actual  : #0000ff
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[1]", "color", u"#008000");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[2]", "color", u"#008000");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[3]", "color", u"#008000");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[4]", "color", u"#008000");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[5]", "color", u"#008000");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[6]", "color", u"#008000");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[7]", "color", u"#008000");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[8]", "color", u"#008000");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf156034)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf156034.svg");

    // Without the fix in place, this test would have failed with
    // - Expected: #008000
    // - Actual  : #0000ff
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[1]", "color", u"#008000");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[2]", "color", u"#008000");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[3]", "color", u"#008000");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[4]", "color", u"#008000");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[5]", "color", u"#008000");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[6]", "color", u"#008000");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[7]", "color", u"#008000");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[8]", "color", u"#008000");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf156038)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf156038.svg");

    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[1]", "color", u"#0000ff");

    // Without the fix in place, this test would have failed with
    // - Expected: #008000
    // - Actual  : #0000ff
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[2]", "color", u"#008000");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[3]", "color", u"#0000ff");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[4]", "color", u"#008000");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[5]", "color", u"#0000ff");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[6]", "color", u"#008000");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[7]", "color", u"#0000ff");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[8]", "color", u"#008000");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf156018)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf156018.svg");

    // Without the fix in place, this test would have failed with
    // - Expected: #008000
    // - Actual  : #0000ff
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[1]", "color", u"#008000");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[2]", "color", u"#0000ff");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf156201)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf156201.svg");

    assertXPath(pDocument, "/primitive2D/transform/transform/transform/polypolygoncolor", "color", u"#2f3ba1");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf156167)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf156167.svg");

    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[1]", "color", u"#ffa500");

    // Without the fix in place, this test would have failed with
    // - Expected: #ffa500
    // - Actual  : #ff0000
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[2]", "color", u"#ffa500");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[3]", "color", u"#ffa500");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf155932)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf155932.svg");

    assertXPath(pDocument, "/primitive2D/transform/mask/mask/polypolygonrgba", "transparence", u"50");
    assertXPath(pDocument, "/primitive2D/transform/mask/mask/polypolygonrgba[1]", "color", u"#0000ff");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf97717)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf97717.svg");

    assertXPath(pDocument, "/primitive2D/transform/unifiedtransparence[1]", "transparence", u"50");
    // Without the fix in place, this test would have failed here since the patch
    // would have contained two unifiedtransparence
    assertXPath(pDocument, "/primitive2D/transform/unifiedtransparence[1]/polypolygoncolor", "color", u"#ccccff");
    assertXPath(pDocument, "/primitive2D/transform/unifiedtransparence[2]", "transparence", u"50");
    assertXPath(pDocument, "/primitive2D/transform/unifiedtransparence[2]/polypolygoncolor", "color", u"#ccccff");
}

CPPUNIT_TEST_FIXTURE(Test, testMarkerOrient)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/MarkerOrient.svg");

    assertXPath(pDocument, "/primitive2D/transform/transform[1]", "xy11", u"0");
    assertXPath(pDocument, "/primitive2D/transform/transform[1]", "xy12", u"1");
    assertXPath(pDocument, "/primitive2D/transform/transform[1]", "xy13", u"7");
    assertXPath(pDocument, "/primitive2D/transform/transform[1]", "xy21", u"-1");
    assertXPath(pDocument, "/primitive2D/transform/transform[1]", "xy22", u"0");
    assertXPath(pDocument, "/primitive2D/transform/transform[1]", "xy23", u"13");
    assertXPath(pDocument, "/primitive2D/transform/transform[1]", "xy31", u"0");
    assertXPath(pDocument, "/primitive2D/transform/transform[1]", "xy32", u"0");
    assertXPath(pDocument, "/primitive2D/transform/transform[1]", "xy33", u"1");

    assertXPath(pDocument, "/primitive2D/transform/transform[2]", "xy11", u"1");
    assertXPath(pDocument, "/primitive2D/transform/transform[2]", "xy12", u"0");
    assertXPath(pDocument, "/primitive2D/transform/transform[2]", "xy13", u"87");
    assertXPath(pDocument, "/primitive2D/transform/transform[2]", "xy21", u"0");
    assertXPath(pDocument, "/primitive2D/transform/transform[2]", "xy22", u"1");
    assertXPath(pDocument, "/primitive2D/transform/transform[2]", "xy23", u"87");
    assertXPath(pDocument, "/primitive2D/transform/transform[2]", "xy31", u"0");
    assertXPath(pDocument, "/primitive2D/transform/transform[2]", "xy32", u"0");
    assertXPath(pDocument, "/primitive2D/transform/transform[2]", "xy33", u"1");
}

CPPUNIT_TEST_FIXTURE(Test, testContextFill)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/contextFill.svg");

    assertXPath(pDocument, "/primitive2D/transform/polypolygonstroke/line", "color", u"#ff0000");
    assertXPath(pDocument, "/primitive2D/transform/transform/polypolygoncolor", "color", u"#ff0000");
}

CPPUNIT_TEST_FIXTURE(Test, testFillContextStroke)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/fillContextStroke.svg");

    assertXPath(pDocument, "/primitive2D/transform/polypolygonstroke/line", "color", u"#ff0000");
    assertXPath(pDocument, "/primitive2D/transform/transform/polypolygoncolor", "color", u"#ff0000");
}

CPPUNIT_TEST_FIXTURE(Test, testContextStroke)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/contextStroke.svg");

    assertXPath(pDocument, "/primitive2D/transform/polypolygonstroke/line", "color", u"#ff0000");
    assertXPath(pDocument, "/primitive2D/transform/transform[1]/polypolygonstroke/line", "color", u"#ff0000");
    assertXPath(pDocument, "/primitive2D/transform/transform[2]/polypolygonstroke/line", "color", u"#ff0000");
    assertXPath(pDocument, "/primitive2D/transform/transform[3]/polypolygonstroke/line", "color", u"#ff0000");
    assertXPath(pDocument, "/primitive2D/transform/transform[4]/polypolygonstroke/line", "color", u"#ff0000");
}

CPPUNIT_TEST_FIXTURE(Test, testContextStrokeGradient)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/contextStrokeGradient.svg");

    assertXPath(pDocument, "/primitive2D/transform/svglineargradient"_ostr);
    assertXPath(pDocument, "/primitive2D/transform/transform[1]/svglineargradient"_ostr);
    assertXPath(pDocument, "/primitive2D/transform/transform[2]/svglineargradient"_ostr);
    assertXPath(pDocument, "/primitive2D/transform/transform[3]/svglineargradient"_ostr);
    assertXPath(pDocument, "/primitive2D/transform/transform[4]/svglineargradient"_ostr);
}

CPPUNIT_TEST_FIXTURE(Test, testMarkerInPresentation)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/markerInPresentation.svg");

    assertXPath(pDocument, "/primitive2D/transform/transform/polypolygonstroke/line", 1);
    assertXPath(pDocument, "/primitive2D/transform/transform/polypolygonstroke/polypolygon/polygon", 1);
    assertXPath(pDocument, "/primitive2D/transform/transform/polypolygonstroke/polypolygon/polygon", 1);

    // Without the fix in place, this test would have failed with
    // - Expected: 0
    // - Actual  : 2
    assertXPath(pDocument, "/primitive2D/transform/transform/transform", 0);
}

CPPUNIT_TEST_FIXTURE(Test, testMarkerInCssStyle)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/markerInCssStyle.svg");

    // Without the fix in place, this test would have failed with
    // - Expected: 20
    // - Actual  : 0
    assertXPath(pDocument, "/primitive2D/transform/transform/polypolygonstroke/line", 20);

    assertXPath(pDocument, "/primitive2D/transform/transform[1]/polypolygonstroke/line", "color", u"#008000");
    assertXPath(pDocument, "/primitive2D/transform/transform[1]/polypolygonstroke/line", "width", u"1");
    assertXPath(pDocument, "/primitive2D/transform/transform[1]/polypolygonstroke/line", "linejoin", u"Miter");
    assertXPath(pDocument, "/primitive2D/transform/transform[1]/polypolygonstroke/line", "miterangle", u"29");
    assertXPath(pDocument, "/primitive2D/transform/transform[1]/polypolygonstroke/line", "linecap", u"BUTT");
}

CPPUNIT_TEST_FIXTURE(Test, testTextXmlSpace)
{
    //Check tspan fontsize when using relative units
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/textXmlSpace.svg");

    assertXPath(pDocument, "/primitive2D/transform/mask/textsimpleportion[1]", "text", u"a b");
    assertXPath(pDocument, "/primitive2D/transform/mask/textsimpleportion[2]", "text", u"a b");
    assertXPath(pDocument, "/primitive2D/transform/mask/textsimpleportion[3]", "text", u"a b");
    assertXPath(pDocument, "/primitive2D/transform/mask/textsimpleportion[4]", "text", u"ab");
    assertXPath(pDocument, "/primitive2D/transform/mask/textsimpleportion[5]", "text", u" a  b ");
    assertXPath(pDocument, "/primitive2D/transform/mask/textsimpleportion[6]", "text", u"a b");
    assertXPath(pDocument, "/primitive2D/transform/mask/textsimpleportion[7]", "text", u"a   b");
    assertXPath(pDocument, "/primitive2D/transform/mask/textsimpleportion[8]", "text", u"a b");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf45771)
{
    //Check text fontsize when using relative units
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf45771.svg");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "fontcolor", u"#000000");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "text", u"Sample");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "height", u"32");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "familyname", u"Times New Roman");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf155833)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf155833.svg");

    assertXPath(pDocument, "/primitive2D/transform/mask/transform/transform/transform/transform/transform/bitmap", 1);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf97941)
{
    //Check tspan fontsize when using relative units
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf97941.svg");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "fontcolor", u"#000000");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "text", u"Sample");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "height", u"48");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "familyname", u"Times New Roman");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf156777)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf156777.svg");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion", 23);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "text", u"Quick brown fox jumps over the lazy dog.");

    // Without the fix in place, this test would have failed with
    // - Expected: #008000
    // - Actual  : #000000
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "fontcolor", u"#008000");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "x", u"85");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "y", u"23");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf156834)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf156834.svg");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion", 4);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "text", u"Auto");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "x", u"30");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "y", u"20");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]", "text", u"Middle");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]", "x", u"30");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]", "y", u"57");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[3]", "text", u"Hanging");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[3]", "x", u"30");
    assertXPathDoubleValue(pDocument, "/primitive2D/transform/textsimpleportion[3]", "y", 93.5, 0.5);

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[4]", "text", u"Central");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[4]", "x", u"30");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[4]", "y", u"117");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf104339)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf104339.svg");

    assertXPath(pDocument, "/primitive2D/transform/transform/transform/transform/transform/polypolygoncolor", "color", u"#000000");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf85770)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf85770.svg");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "fontcolor", u"#000000");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "text", u"Start Middle End");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "height", u"11");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "familyname", u"Times New Roman");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]", "fontcolor", u"#000000");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]", "text", u"Start ");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]", "height", u"11");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]", "familyname", u"Times New Roman");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[3]", "fontcolor", u"#000000");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[3]", "text", u" End");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[3]", "height", u"11");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[3]", "familyname", u"Times New Roman");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf86938)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf86938.svg");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "text", u"line");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "x", u"290");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "y", u"183");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]", "text", u"above");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]", "x", u"290");

    // Without the fix in place, this test would have failed with
    // - Expected: 159
    // - Actual  : 207
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]", "y", u"159");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[3]", "text", u"below");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[3]", "x", u"290");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[3]", "y", u"207");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf93583)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf93583.svg");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "text", u"This is the");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "x", u"58");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "y", u"303");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "width", u"16");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "height", u"16");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]", "text", u" first ");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]", "x", u"125");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]", "y", u"303");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]", "width", u"32");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]", "height", u"32");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[3]", "text", u"line");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[3]", "x", u"192");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[3]", "y", u"303");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[3]", "width", u"16");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[3]", "height", u"16");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf156616)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf156616.svg");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "text", u"First ");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "x", u"114");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "y", u"103");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]", "text", u"line ");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]", "x", u"147");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]", "y", u"103");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[3]", "text", u"Second line");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[3]", "x", u"114");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[3]", "y", u"122");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[4]", "text", u"First ");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[4]", "x", u"85");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[4]", "y", u"153");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[5]", "text", u"line ");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[5]", "x", u"118");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[5]", "y", u"153");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[6]", "text", u"Second line");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[6]", "x", u"77");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[6]", "y", u"172");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[7]", "text", u"First ");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[7]", "x", u"55");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[7]", "y", u"203");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[8]", "text", u"line ");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[8]", "x", u"88");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[8]", "y", u"203");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[9]", "text", u"Second line");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[9]", "x", u"40");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[9]", "y", u"222");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf79163)
{
    //Check Opacity
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf79163.svg");

    assertXPath(pDocument, "/primitive2D/transform/unifiedtransparence", "transparence", u"50");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf97542_1)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf97542_1.svg");

    assertXPath(pDocument, "/primitive2D/transform/objectinfo/textsimpleportion", "fontcolor", u"#ffff00");
    assertXPath(pDocument, "/primitive2D/transform/objectinfo/textsimpleportion", "text", u"Text");
    assertXPath(pDocument, "/primitive2D/transform/objectinfo/textsimpleportion", "height", u"48");
    assertXPath(pDocument, "/primitive2D/transform/objectinfo/textsimpleportion", "familyname", u"DejaVu Serif");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf97542_2)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf97542_2.svg");

    assertXPath(pDocument, "/primitive2D/transform/objectinfo/svgradialgradient", "startx", u"1");
    assertXPath(pDocument, "/primitive2D/transform/objectinfo/svgradialgradient", "starty", u"1");
    assertXPath(pDocument, "/primitive2D/transform/objectinfo/svgradialgradient/focalx", 0);
    assertXPath(pDocument, "/primitive2D/transform/objectinfo/svgradialgradient/focaly", 0);
    assertXPath(pDocument, "/primitive2D/transform/objectinfo/svgradialgradient", "radius", u"3");
    assertXPath(pDocument, "/primitive2D/transform/objectinfo/svgradialgradient", "spreadmethod", u"pad");
    assertXPath(pDocument, "/primitive2D/transform/objectinfo/svgradialgradient", "opacity", u"1");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf97543)
{
    // check visibility="inherit"
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf97543.svg");

    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor", "color", u"#00cc00");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor/polypolygon", "height", u"100");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor/polypolygon", "width", u"100");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor/polypolygon", "minx", u"10");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor/polypolygon", "miny", u"10");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor/polypolygon", "maxx", u"110");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor/polypolygon", "maxy", u"110");
}

CPPUNIT_TEST_FIXTURE(Test, testRGBColor)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/RGBColor.svg");

    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor", "color", u"#646464");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor/polypolygon", "height", u"100");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor/polypolygon", "width", u"100");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor/polypolygon", "minx", u"10");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor/polypolygon", "miny", u"10");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor/polypolygon", "maxx", u"110");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor/polypolygon", "maxy", u"110");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf149673)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf149673.svg");

    assertXPath(pDocument, "/primitive2D/transform/unifiedtransparence", "transparence", u"90");
    assertXPath(pDocument, "/primitive2D/transform/unifiedtransparence/polypolygoncolor[1]", "color", u"#ff0000");
    assertXPath(pDocument, "/primitive2D/transform/unifiedtransparence/polypolygoncolor[2]", "color", u"#00ff00");
    assertXPath(pDocument, "/primitive2D/transform/unifiedtransparence/polypolygoncolor[3]", "color", u"#0000ff");
}

CPPUNIT_TEST_FIXTURE(Test, testRGBAColor)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/RGBAColor.svg");

    assertXPath(pDocument, "/primitive2D/transform/unifiedtransparence", "transparence", u"50");
}

CPPUNIT_TEST_FIXTURE(Test, testNoneColor)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/noneColor.svg");

    //No polypolygoncolor exists
    assertXPath(pDocument, "/primitive2D/transform/mask/polypolygoncolor", 0);
    assertXPath(pDocument, "/primitive2D/transform/mask/polypolygonstroke/line", "color", u"#000000");
    assertXPath(pDocument, "/primitive2D/transform/mask/polypolygonstroke/line", "width", u"3");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf117920)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf117920.svg");

    assertXPath(pDocument, "/primitive2D/transform/transform", "xy11", u"1");
    assertXPath(pDocument, "/primitive2D/transform/transform", "xy12", u"0");
    assertXPath(pDocument, "/primitive2D/transform/transform", "xy13", u"-18");
    assertXPath(pDocument, "/primitive2D/transform/transform", "xy21", u"0");
    assertXPath(pDocument, "/primitive2D/transform/transform", "xy22", u"1");
    assertXPath(pDocument, "/primitive2D/transform/transform", "xy23", u"-6");
    assertXPath(pDocument, "/primitive2D/transform/transform", "xy31", u"0");
    assertXPath(pDocument, "/primitive2D/transform/transform", "xy32", u"0");
    assertXPath(pDocument, "/primitive2D/transform/transform", "xy33", u"1");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf97936)
{
    // check that both rectangles are rendered in the viewBox
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf97936.svg");

    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[1]");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[1]/polypolygon", "height", u"50");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[1]/polypolygon", "width", u"50");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[1]/polypolygon", "minx", u"70");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[1]/polypolygon", "miny", u"50");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[1]/polypolygon", "maxx", u"120");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[1]/polypolygon", "maxy", u"100");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[2]");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[2]/polypolygon", "height", u"50");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[2]/polypolygon", "width", u"50");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[2]/polypolygon", "minx", u"10");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[2]/polypolygon", "miny", u"50");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[2]/polypolygon", "maxx", u"60");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[2]/polypolygon", "maxy", u"100");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf149893)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf149893.svg");

    // Without the fix in place, this test would have failed with
    // - Expected: #008000
    // - Actual  : #000000
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor", "color", u"#008000");
}

CPPUNIT_TEST_FIXTURE(Test, testShapeWithClipPathAndCssStyle)
{
    // tdf#97539: Check there is a mask and 3 polygons
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/ShapeWithClipPathAndCssStyle.svg");

    assertXPath(pDocument, "/primitive2D/transform/mask/polypolygon/polygon", 2);
    assertXPath(pDocument, "/primitive2D/transform/mask/polypolygoncolor/polypolygon/polygon", 1);
}

CPPUNIT_TEST_FIXTURE(Test, testClipPathAndParentStyle)
{
    //Check that fill color, stroke color and stroke-width are inherited from use element
    //when the element is within a clipPath element
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/ClipPathAndParentStyle.svg");

    assertXPath(pDocument, "/primitive2D/transform/transform/polypolygoncolor", "color", u"#ff0000");
    assertXPath(pDocument, "/primitive2D/transform/transform/polypolygonstroke/line", "color", u"#000000");
    assertXPath(pDocument, "/primitive2D/transform/transform/polypolygonstroke/line", "width", u"5");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf155814)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf155814.svg");

    assertXPath(pDocument, "/primitive2D/transform/mask/mask/transform/polypolygonrgba", "transparence", u"50");
    assertXPath(pDocument, "/primitive2D/transform/mask/mask/transform/polypolygonrgba", "color", u"#0000ff");
}

CPPUNIT_TEST_FIXTURE(Test, testClipPathAndStyle)
{
    //Check that fill color, stroke color and stroke-width are inherited from use element
    //when the element is within a clipPath element
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/ClipPathAndStyle.svg");

    assertXPath(pDocument, "/primitive2D/transform/transform/polypolygoncolor", "color", u"#ccccff");
    assertXPath(pDocument, "/primitive2D/transform/transform/polypolygonstroke/line", "color", u"#0000cc");
    assertXPath(pDocument, "/primitive2D/transform/transform/polypolygonstroke/line", "width", u"2");

}

CPPUNIT_TEST_FIXTURE(Test, testShapeWithClipPath)
{
    // Check there is a mask and 3 polygons
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/ShapeWithClipPath.svg");

    assertXPath(pDocument, "/primitive2D/transform/mask/polypolygon/polygon", 2);
    assertXPath(pDocument, "/primitive2D/transform/mask/polypolygoncolor/polypolygon/polygon", 1);
}

CPPUNIT_TEST_FIXTURE(Test, testClipPathUsingClipPath)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/ClipPathUsingClipPath.svg");

    assertXPath(pDocument, "/primitive2D/transform/mask/polypolygon/polygon/point", 20);
    assertXPath(pDocument, "/primitive2D/transform/mask/mask/polypolygon/polygon/point", 13);
}

CPPUNIT_TEST_FIXTURE(Test, testFillRule)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/FillRule.svg");

    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor", "color", u"#000000");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor/polypolygon/polygon", 2);
    assertXPath(pDocument, "/primitive2D/transform/polypolygonstroke/line", "color", u"#ff0000");
    assertXPath(pDocument, "/primitive2D/transform/polypolygonstroke/polypolygon/polygon", 2);
}

CPPUNIT_TEST_FIXTURE(Test, testClipRule)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/ClipRule.svg");

    // Without the place in place, this test would have failed with
    // - Expected: 5
    // - Actual  : 10
    assertXPath(pDocument, "/primitive2D/transform/mask[1]/polypolygon/polygon/point", 5);
    assertXPath(pDocument, "/primitive2D/transform/mask[1]/polypolygoncolor", "color", u"#0000ff");
    assertXPath(pDocument, "/primitive2D/transform/mask[1]/polypolygoncolor/polypolygon/polygon/point", 4);

    assertXPath(pDocument, "/primitive2D/transform/mask[2]/polypolygon/polygon/point", 5);
    assertXPath(pDocument, "/primitive2D/transform/mask[2]/polypolygoncolor", "color", u"#ff0000");
    assertXPath(pDocument, "/primitive2D/transform/mask[2]/polypolygoncolor/polypolygon/polygon/point", 4);
}

CPPUNIT_TEST_FIXTURE(Test, testi125329)
{
    //Check style inherit from * css element
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/i125329.svg");

    assertXPath(pDocument, "/primitive2D/transform/transform/objectinfo/polypolygoncolor", "color", u"#c0c0c0"); // rect background color
    assertXPath(pDocument, "/primitive2D/transform/transform/objectinfo/polypolygoncolor/polypolygon", "height", u"30"); // rect background height
    assertXPath(pDocument, "/primitive2D/transform/transform/objectinfo/polypolygoncolor/polypolygon", "width", u"50"); // rect background width
    assertXPath(pDocument, "/primitive2D/transform/transform/objectinfo/polypolygoncolor/polypolygon", "minx", u"15");
    assertXPath(pDocument, "/primitive2D/transform/transform/objectinfo/polypolygoncolor/polypolygon", "miny", u"15");
    assertXPath(pDocument, "/primitive2D/transform/transform/objectinfo/polypolygoncolor/polypolygon", "maxx", u"65");
    assertXPath(pDocument, "/primitive2D/transform/transform/objectinfo/polypolygoncolor/polypolygon", "maxy", u"45");
    assertXPath(pDocument, "/primitive2D/transform/transform/objectinfo/polypolygonstroke/line", "color", u"#008000"); // rect stroke color
    assertXPath(pDocument, "/primitive2D/transform/transform/objectinfo/polypolygonstroke/line", "width", u"1"); // rect stroke width
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

    assertXPath(pDocument, "/primitive2D/transform/transform/transform/polypolygonrgba", "color", u"#7cb5ec");
}

CPPUNIT_TEST_FIXTURE(Test, test47446)
{
    //Check that marker's fill attribute is black is not set
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/47446.svg");

    assertXPath(pDocument, "/primitive2D/transform/transform/transform/polypolygoncolor", "color", u"#000000");
}

CPPUNIT_TEST_FIXTURE(Test, test47446b)
{
    //Check that marker's fill attribute is inherit from def
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/47446b.svg");

    assertXPath(pDocument, "/primitive2D/transform/transform/transform/polypolygoncolor", "color", u"#ffff00");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf103888)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf103888.svg");

    // Without the fix in place, this test would have failed here with number of nodes is incorrect
    assertXPath(pDocument, "/primitive2D/transform/transform/textsimpleportion[1]", "text", u"Her");
    assertXPath(pDocument, "/primitive2D/transform/transform/textsimpleportion[2]", "text", u"vor");
    assertXPath(pDocument, "/primitive2D/transform/transform/textsimpleportion[3]", "text", u"hebung");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf156251)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf156251.svg");

    // Without the fix in place, this test would have failed with
    // - Expected: 'You are '
    // - Actual  : 'You are'
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "text", u"You are ");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]", "text", u"not");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[3]", "text", u" a banana!");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[4]", "text", u"You are");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[5]", "text", u" not ");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[6]", "text", u"a banana!");
}

CPPUNIT_TEST_FIXTURE(Test, testMaskText)
{
    //Check that mask is applied on text
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/maskText.svg");

    assertXPath(pDocument, "/primitive2D/transform/transform/polypolygoncolor", "color", u"#000000");
    assertXPath(pDocument, "/primitive2D/transform/transform/transform/textsimpleportion", "fontcolor", u"#ffffff");
    assertXPath(pDocument, "/primitive2D/transform/transform/transform/textsimpleportion", "text", u"Black White");
    assertXPath(pDocument, "/primitive2D/transform/transform/transform/textsimpleportion", "height", u"26");
    assertXPath(pDocument, "/primitive2D/transform/transform/transform/textsimpleportion", "familyname", u"Times New Roman");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf99994)
{
    //Check text fontsize when using relative units
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf99994.svg");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "fontcolor", u"#0000ff");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "height", u"16");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "text", u"test");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "familyname", u"DejaVu Sans");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf99115)
{
    //Check that styles are resolved correctly where there is a * css selector
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf99115.svg");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "text", u"red 1");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "fontcolor", u"#ff0000");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "height", u"18");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]", "text", u"red 2");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]", "fontcolor", u"#ff0000");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]", "height", u"18");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[3]", "text", u"red 3");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[3]", "fontcolor", u"#ff0000");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[3]", "height", u"18");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[4]", "text", u"blue 4");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[4]", "fontcolor", u"#0000ff");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[4]", "height", u"18");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[5]", "text", u"blue 5");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[5]", "fontcolor", u"#0000ff");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[5]", "height", u"18");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[6]", "text", u"blue 6");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[6]", "fontcolor", u"#0000ff");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[6]", "height", u"18");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[7]", "text", u"green 7");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[7]", "fontcolor", u"#008000");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[7]", "height", u"18");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[8]", "text", u"green 8");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[8]", "fontcolor", u"#008000");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[8]", "height", u"18");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[9]", "text", u"green 9");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[9]", "fontcolor", u"#008000");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[9]", "height", u"18");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf101237)
{
    //Check that fill color, stroke color and stroke-width are inherited from use element
    //when the element is within a clipPath element
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf101237.svg");

    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor", "color", u"#ff0000");
    assertXPath(pDocument, "/primitive2D/transform/polypolygonstroke/line", "color", u"#000000");
    assertXPath(pDocument, "/primitive2D/transform/polypolygonstroke/line", "width", u"5");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf97710)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf97710.svg");

    assertXPath(pDocument, "/primitive2D/transform/mask/polypolygoncolor[1]", "color", u"#000000");

    // Without the fix in place, this test would have failed with
    // - Expected: 100
    // - Actual  : 0
    assertXPath(pDocument, "/primitive2D/transform/mask/polypolygoncolor[1]/polypolygon", "width", u"100");
    assertXPath(pDocument, "/primitive2D/transform/mask/polypolygoncolor[1]/polypolygon", "height", u"100");
    assertXPath(pDocument, "/primitive2D/transform/mask/polypolygoncolor[2]", "color", u"#008000");
    assertXPath(pDocument, "/primitive2D/transform/mask/polypolygoncolor[2]/polypolygon", "width", u"100");
    assertXPath(pDocument, "/primitive2D/transform/mask/polypolygoncolor[2]/polypolygon", "height", u"100");
    assertXPath(pDocument, "/primitive2D/transform/mask/polypolygonstroke/line", "color", u"#000000");
    assertXPath(pDocument, "/primitive2D/transform/mask/polypolygonstroke/line", "width", u"1");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf94765)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf94765.svg");

    //Check that both rectangles use the gradient as fill
    assertXPath(pDocument, "/primitive2D/transform/transform/svglineargradient[1]", "startx", u"1");
    assertXPath(pDocument, "/primitive2D/transform/transform/svglineargradient[1]", "starty", u"1");
    assertXPath(pDocument, "/primitive2D/transform/transform/svglineargradient[1]", "endx", u"2");
    assertXPath(pDocument, "/primitive2D/transform/transform/svglineargradient[1]", "endy", u"1");
    assertXPath(pDocument, "/primitive2D/transform/transform/svglineargradient[2]", "startx", u"0");
    assertXPath(pDocument, "/primitive2D/transform/transform/svglineargradient[2]", "starty", u"0");
    assertXPath(pDocument, "/primitive2D/transform/transform/svglineargradient[2]", "endx", u"1");
    assertXPath(pDocument, "/primitive2D/transform/transform/svglineargradient[2]", "endy", u"0");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf156236)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf156236.svg");

    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[1]/polypolygon", "path", u"m50 180h-30v-60h60v60z");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[2]/polypolygon", "path", u"m150 180h15c8.2842712474619 0 15-6.7157287525381 15-15v-30c0-8.2842712474619-6.7157287525381-15-15-15h-30c-8.2842712474619 0-15 6.7157287525381-15 15v30c0 8.2842712474619 6.7157287525381 15 15 15z");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[3]/polypolygon", "path", u"m250 180h15c8.2842712474619 0 15-6.7157287525381 15-15v-30c0-8.2842712474619-6.7157287525381-15-15-15h-30c-8.2842712474619 0-15 6.7157287525381-15 15v30c0 8.2842712474619 6.7157287525381 15 15 15z");
    assertXPath(pDocument, "/primitive2D/transform/polypolygoncolor[4]/polypolygon", "path", u"m350 180c16.5685424949238 0 30-6.7157287525381 30-15v-30c0-8.2842712474619-13.4314575050762-15-30-15s-30 6.7157287525381-30 15v30c0 8.2842712474619 13.4314575050762 15 30 15z");
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

    assertXPath(pDocument, "/primitive2D/transform/softedge", "radius", u"5");

    // Without the fix in place, the softedge would have been applied to the second element
    // - Expected: 1
    // - Actual  : 0
    assertXPath(pDocument, "/primitive2D/transform/transform/polypolygonrgba", "transparence", u"50");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf158445)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf158445.svg");

    assertXPath(pDocument, "/primitive2D/transform/mask/transform/transform/transform/polypolygoncolor", "color", u"#000000");
    assertXPath(pDocument, "/primitive2D/transform/mask/transform/transform/transform/polypolygoncolor/polypolygon", "height", u"8.052");
    assertXPath(pDocument, "/primitive2D/transform/mask/transform/transform/transform/polypolygoncolor/polypolygon", "width", u"5.328");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf159594)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf159594.svg");

    assertXPath(pDocument, "/primitive2D/transform/transform/polypolygoncolor", "color", u"#000000");
    assertXPath(pDocument, "/primitive2D/transform/transform/polypolygoncolor/polypolygon", "height", u"11.671875");
    assertXPath(pDocument, "/primitive2D/transform/transform/polypolygoncolor/polypolygon", "width", u"7.5");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf97663)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/em_units.svg");

    // tdf#97663: Without the fix in place, this test would have failed with
    // - Expected: 236
    // - Actual  : 204
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]", "y", u"237");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf156269)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf156269.svg");

    assertXPath(pDocument, "//textsimpleportion[@text='one']", "width", u"16");
    assertXPath(pDocument, "//textsimpleportion[@text='one']", "height", u"16");
    assertXPath(pDocument, "//textsimpleportion[@text='one']", "x", u"10");
    assertXPath(pDocument, "//textsimpleportion[@text='one']", "y", u"50");
    assertXPath(pDocument, "//textsimpleportion[@text='one']", "fontcolor", u"#808080");

    assertXPath(pDocument, "//textsimpleportion[@text='two']", "width", u"16");
    assertXPath(pDocument, "//textsimpleportion[@text='two']", "height", u"16");

    // Without the fix in place, this test would have failed with
    // - Expected: 60
    // - Actual  : 10
    assertXPath(pDocument, "//textsimpleportion[@text='two']", "x", u"60");
    assertXPath(pDocument, "//textsimpleportion[@text='two']", "y", u"100");
    assertXPath(pDocument, "//textsimpleportion[@text='two']", "fontcolor", u"#000000");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf95400)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf95400.svg");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "width", u"16");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "height", u"16");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "x", u"30");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "y", u"20");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "text", u"ABC");
    assertXPathDoubleValue(pDocument, "/primitive2D/transform/textsimpleportion[1]", "dx0", 36, 0.5);
    assertXPathDoubleValue(pDocument, "/primitive2D/transform/textsimpleportion[1]", "dx1", 69, 0.5);
    assertXPathDoubleValue(pDocument, "/primitive2D/transform/textsimpleportion[1]", "dx2", 102, 0.5);

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]", "width", u"50");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]", "height", u"16");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]", "x", u"30");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]", "y", u"30");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]", "text", u"ABC");
    assertXPathNoAttribute(pDocument, "/primitive2D/transform/textsimpleportion[2]", "dx0");
}

CPPUNIT_TEST_FIXTURE(Test, testTextAnchor)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf151103.svg");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "x", u"60");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "y", u"40");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "text", u"ABC");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]", "x", u"43");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]", "y", u"50");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]", "text", u"ABC");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[3]", "x", u"26");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[3]", "y", u"60");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[3]", "text", u"ABC");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[4]", "x", u"60");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[4]", "y", u"40");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[4]", "text", u"ABC");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[5]", "x", u"43");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[5]", "y", u"50");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[5]", "text", u"ABC");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[6]", "x", u"26");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[6]", "y", u"60");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[6]", "text", u"ABC");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[7]", "x", u"60");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[7]", "y", u"40");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[7]", "text", u"ABC");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[8]", "x", u"43");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[8]", "y", u"50");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[8]", "text", u"ABC");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[9]", "x", u"26");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[9]", "y", u"60");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[9]", "text", u"ABC");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[10]", "x", u"60");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[10]", "y", u"40");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[10]", "text", u"A");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[11]", "x", u"72");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[11]", "y", u"40");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[11]", "text", u"B");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[12]", "x", u"82");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[12]", "y", u"40");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[12]", "text", u"C");

    // Without the fix in place, this test would have failed with
    // - Expected: 43
    // - Actual  : 54
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[13]", "x", u"43");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[13]", "y", u"50");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[13]", "text", u"A");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[14]", "x", u"55");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[14]", "y", u"50");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[14]", "text", u"B");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[15]", "x", u"65");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[15]", "y", u"50");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[15]", "text", u"C");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[16]", "x", u"26");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[16]", "y", u"60");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[16]", "text", u"A");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[17]", "x", u"38");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[17]", "y", u"60");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[17]", "text", u"B");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[18]", "x", u"48");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[18]", "y", u"60");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[18]", "text", u"C");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf156577)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf156577.svg");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "width", u"16");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "height", u"16");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "x", u"30");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "y", u"20");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "text", u"ABC");
    assertXPathNoAttribute(pDocument, "/primitive2D/transform/textsimpleportion[1]", "dx0");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]", "width", u"16");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]", "height", u"16");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]", "x", u"30");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]", "y", u"30");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]", "text", u"ABC");

    // Without the fix in place, this test would have failed with
    // - Expected: 22
    // - Actual  : 52
    assertXPathDoubleValue(pDocument, "/primitive2D/transform/textsimpleportion[2]", "dx0", 22, 0.5);
    assertXPathDoubleValue(pDocument, "/primitive2D/transform/textsimpleportion[2]", "dx1", 52, 0.5);
    assertXPathDoubleValue(pDocument, "/primitive2D/transform/textsimpleportion[2]", "dx2", 93, 0.5);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf156283)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf156283.svg");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "width", u"16");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "height", u"16");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "x", u"30");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "y", u"20");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "text", u"ABC");
    assertXPathDoubleValue(pDocument, "/primitive2D/transform/textsimpleportion[1]", "dx0", 41, 0.5);
    assertXPathDoubleValue(pDocument, "/primitive2D/transform/textsimpleportion[1]", "dx1", 52, 0.5);
    assertXPathDoubleValue(pDocument, "/primitive2D/transform/textsimpleportion[1]", "dx2", 62, 0.5);

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]", "width", u"16");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]", "height", u"16");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]", "x", u"30");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]", "y", u"30");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]", "text", u"ABC");

    // Without the fix in place, this test would have failed with
    // - Expected: 41
    // - Actual  : 12
    assertXPathDoubleValue(pDocument, "/primitive2D/transform/textsimpleportion[2]", "dx0", 41, 0.5);
    assertXPathDoubleValue(pDocument, "/primitive2D/transform/textsimpleportion[2]", "dx1", 51, 0.5);
    assertXPathDoubleValue(pDocument, "/primitive2D/transform/textsimpleportion[2]", "dx2", 62, 0.5);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf156569)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf156569.svg");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "width", u"16");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "height", u"16");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "x", u"0");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "y", u"20");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "text", u"ABC");
    assertXPathDoubleValue(pDocument, "/primitive2D/transform/textsimpleportion[1]", "dx0", 40, 0.5);
    assertXPathDoubleValue(pDocument, "/primitive2D/transform/textsimpleportion[1]", "dx1", 80, 0.5);
    assertXPathDoubleValue(pDocument, "/primitive2D/transform/textsimpleportion[1]", "dx2", 91, 0.5);

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]", "width", u"16");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]", "height", u"16");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]", "x", u"0");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]", "y", u"30");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]", "text", u"ABC");
    assertXPathDoubleValue(pDocument, "/primitive2D/transform/textsimpleportion[2]", "dx0", 40, 0.5);

    // Without the fix in place, this test would have failed with
    // - Expected: 80
    // - Actual  : 51
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]", "dx1", u"80");
    assertXPathDoubleValue(pDocument, "/primitive2D/transform/textsimpleportion[2]", "dx2", 91, 0.5);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf156837)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf156837.svg");

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion", 2);
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "x", u"114");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "y", u"103");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "height", u"16");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "text", u"x ");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]", "x", u"126");

    // Without the fix in place, this test would have failed with
    // - Expected: 94
    // - Actual  : 103
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]", "y", u"94");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]", "height", u"10");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[2]", "text", u"3");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf160773)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf160773.svg");

    // tdf#160773 Check there is a rectangle
    assertXPath(pDocument, "/primitive2D/transform/mask/transform/polypolygoncolor", "color", u"#ff0000");

    // tdf#159661 Check there is text in the right position
    assertXPath(pDocument, "/primitive2D/transform/mask/transform/textsimpleportion", 1);
    assertXPath(pDocument, "/primitive2D/transform/mask/transform/textsimpleportion", "x", u"0");
    assertXPath(pDocument, "/primitive2D/transform/mask/transform/textsimpleportion", "y", u"1");
    assertXPath(pDocument, "/primitive2D/transform/mask/transform/textsimpleportion", "height", u"0");
    assertXPath(pDocument, "/primitive2D/transform/mask/transform/textsimpleportion", "width", u"0");
    assertXPath(pDocument, "/primitive2D/transform/mask/transform/textsimpleportion", "text", u"Red");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf156271)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf156271.svg");

    assertXPath(pDocument, "/primitive2D/transform/mask/textsimpleportion[1]", "width", u"16");
    assertXPath(pDocument, "/primitive2D/transform/mask/textsimpleportion[1]", "height", u"16");
    assertXPath(pDocument, "/primitive2D/transform/mask/textsimpleportion[1]", "x", u"40");
    assertXPath(pDocument, "/primitive2D/transform/mask/textsimpleportion[1]", "y", u"10");
    assertXPath(pDocument, "/primitive2D/transform/mask/textsimpleportion[1]", "text", u"AB");
    assertXPathDoubleValue(pDocument, "/primitive2D/transform/mask/textsimpleportion[1]", "dx0", -30, 0.5);
    assertXPathDoubleValue(pDocument, "/primitive2D/transform/mask/textsimpleportion[1]", "dx1", -19, 0.5);

    assertXPath(pDocument, "/primitive2D/transform/mask/textsimpleportion[2]", "width", u"16");
    assertXPath(pDocument, "/primitive2D/transform/mask/textsimpleportion[2]", "height", u"16");
    assertXPath(pDocument, "/primitive2D/transform/mask/textsimpleportion[2]", "x", u"40");
    assertXPath(pDocument, "/primitive2D/transform/mask/textsimpleportion[2]", "y", u"20");
    assertXPath(pDocument, "/primitive2D/transform/mask/textsimpleportion[2]", "text", u"AB");

    // Without the fix in place, this test would have failed with
    // - Expected: -30
    // - Actual  : 0
    assertXPathDoubleValue(pDocument, "/primitive2D/transform/mask/textsimpleportion[2]", "dx0", -30, 0.5);
    assertXPathDoubleValue(pDocument, "/primitive2D/transform/mask/textsimpleportion[2]", "dx1", -19, 0.5);

    assertXPath(pDocument, "/primitive2D/transform/mask/textsimpleportion[3]", "width", u"16");
    assertXPath(pDocument, "/primitive2D/transform/mask/textsimpleportion[3]", "height", u"16");
    assertXPath(pDocument, "/primitive2D/transform/mask/textsimpleportion[3]", "x", u"40");
    assertXPath(pDocument, "/primitive2D/transform/mask/textsimpleportion[3]", "y", u"30");
    assertXPath(pDocument, "/primitive2D/transform/mask/textsimpleportion[3]", "text", u"AB");
    assertXPathDoubleValue(pDocument, "/primitive2D/transform/mask/textsimpleportion[3]", "dx0", -30, 0.5);
    assertXPathDoubleValue(pDocument, "/primitive2D/transform/mask/textsimpleportion[3]", "dx1", -19, 0.5);

    assertXPath(pDocument, "/primitive2D/transform/mask/textsimpleportion[4]", "width", u"16");
    assertXPath(pDocument, "/primitive2D/transform/mask/textsimpleportion[4]", "height", u"16");
    assertXPath(pDocument, "/primitive2D/transform/mask/textsimpleportion[4]", "x", u"40");
    assertXPath(pDocument, "/primitive2D/transform/mask/textsimpleportion[4]", "y", u"40");
    assertXPath(pDocument, "/primitive2D/transform/mask/textsimpleportion[4]", "text", u"AB");
    assertXPathDoubleValue(pDocument, "/primitive2D/transform/mask/textsimpleportion[4]", "dx0", 12, 0.5);
    assertXPathDoubleValue(pDocument, "/primitive2D/transform/mask/textsimpleportion[4]", "dx1", 22, 0.5);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf159968)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf159968.svg");

    // Check no mask is applied to the marker
    assertXPath(pDocument,
            "/primitive2D/transform/transform/transform/transform/polypolygoncolor", "color", u"#000000");
    assertXPath(pDocument,
            "/primitive2D/transform/transform/transform/transform/polypolygoncolor/polypolygon/polygon/point", 5);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf160517)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf160517.svg");

    assertXPath(pDocument,
            "/primitive2D/transform/bitmap", "height", u"110");
    assertXPath(pDocument,
            "/primitive2D/transform/bitmap", "width", u"110");
    assertXPath(pDocument,
            "/primitive2D/transform/bitmap/data", 110);

    assertXPath(pDocument,
            "/primitive2D/transform/bitmap", "xy11", u"110");
    assertXPath(pDocument,
            "/primitive2D/transform/bitmap", "xy12", u"0");
    assertXPath(pDocument,
            "/primitive2D//transform/bitmap", "xy13", u"0");
    assertXPath(pDocument,
            "/primitive2D//transform/bitmap", "xy21", u"0");
    assertXPath(pDocument,
            "/primitive2D//transform/bitmap", "xy22", u"110");
    assertXPath(pDocument,
            "/primitive2D//transform/bitmap", "xy23", u"0");
    assertXPath(pDocument,
            "/primitive2D//transform/bitmap", "xy31", u"0");
    assertXPath(pDocument,
            "/primitive2D//transform/bitmap", "xy32", u"0");
    assertXPath(pDocument,
            "/primitive2D//transform/bitmap", "xy33", u"1");

    // Check the color of a pixel in the middle
    const OUString sDataRow = getXPath(pDocument, "/primitive2D/transform/bitmap/data[55]", "row");
    std::vector<OUString> aPixels = comphelper::string::split(sDataRow, ',');
    CPPUNIT_ASSERT_EQUAL(u"008100"_ustr, aPixels[55]);
}

CPPUNIT_TEST_FIXTURE(Test, testArithmeticComposite)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/arithmetic.svg");

    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap", "height", u"150");
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap", "width", u"150");
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap/data", 150);

    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap", "xy11", u"150");
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap", "xy12", u"0");
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap", "xy13", u"0");
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap", "xy21", u"0");
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap", "xy22", u"150");
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap", "xy23", u"0");
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap", "xy31", u"0");
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap", "xy32", u"0");
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap", "xy33", u"1");

    // Check the colors in the diagonal
    OUString sDataRow = getXPath(pDocument, "/primitive2D/transform/transform/bitmap/data[25]", "row");
    std::vector<OUString> aPixels = comphelper::string::split(sDataRow, ',');
    CPPUNIT_ASSERT_EQUAL(u"000000"_ustr, aPixels[25]);

    sDataRow = getXPath(pDocument, "/primitive2D/transform/transform/bitmap/data[75]", "row");
    aPixels = comphelper::string::split(sDataRow, ',');
    CPPUNIT_ASSERT_EQUAL(u"ff8000"_ustr, aPixels[75]);

    sDataRow = getXPath(pDocument, "/primitive2D/transform/transform/bitmap/data[125]", "row");
    aPixels = comphelper::string::split(sDataRow, ',');
    CPPUNIT_ASSERT_EQUAL(u"ff8000"_ustr, aPixels[125]);
}

CPPUNIT_TEST_FIXTURE(Test, testArithmeticComposite2)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/arithmetic2.svg");

    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap", "height", u"150");
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap", "width", u"150");
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap/data", 150);

    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap", "xy11", u"150");
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap", "xy12", u"0");
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap", "xy13", u"0");
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap", "xy21", u"0");
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap", "xy22", u"150");
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap", "xy23", u"0");
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap", "xy31", u"0");
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap", "xy32", u"0");
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap", "xy33", u"1");

    // Check the colors in the diagonal
    OUString sDataRow = getXPath(pDocument, "/primitive2D/transform/transform/bitmap/data[25]", "row");
    std::vector<OUString> aPixels = comphelper::string::split(sDataRow, ',');
    CPPUNIT_ASSERT_EQUAL(u"ff0000"_ustr, aPixels[25]);

    sDataRow = getXPath(pDocument, "/primitive2D/transform/transform/bitmap/data[75]", "row");
    aPixels = comphelper::string::split(sDataRow, ',');
    CPPUNIT_ASSERT_EQUAL(u"ff8000"_ustr, aPixels[75]);

    sDataRow = getXPath(pDocument, "/primitive2D/transform/transform/bitmap/data[125]", "row");
    aPixels = comphelper::string::split(sDataRow, ',');
    CPPUNIT_ASSERT_EQUAL(u"008000"_ustr, aPixels[125]);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf160726)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf160726.svg");

    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap", "height", u"250");
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap", "width", u"250");
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap/data", 250);

    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap", "xy11", u"250");
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap", "xy12", u"0");
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap", "xy13", u"0");
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap", "xy21", u"0");
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap", "xy22", u"250");
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap", "xy23", u"0");
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap", "xy31", u"0");
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap", "xy32", u"0");
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap", "xy33", u"1");

    // Check the colors in the diagonal
    OUString sDataRow = getXPath(pDocument, "/primitive2D/transform/transform/bitmap/data[50]", "row");
    std::vector<OUString> aPixels = comphelper::string::split(sDataRow, ',');
    CPPUNIT_ASSERT_EQUAL(u"ff0000"_ustr, aPixels[50]);

    sDataRow = getXPath(pDocument, "/primitive2D/transform/transform/bitmap/data[125]", "row");
    aPixels = comphelper::string::split(sDataRow, ',');
    CPPUNIT_ASSERT_EQUAL(u"ffff00"_ustr, aPixels[125]);

    sDataRow = getXPath(pDocument, "/primitive2D/transform/transform/bitmap/data[200]", "row");
    aPixels = comphelper::string::split(sDataRow, ',');
    CPPUNIT_ASSERT_EQUAL(u"ffff00"_ustr, aPixels[200]);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf160782)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tdf160782.svg");

    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap", "height", u"255");
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap", "width", u"255");
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap/data", 255);

    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap", "xy11", u"255");
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap", "xy12", u"0");
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap", "xy13", u"0");
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap", "xy21", u"0");
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap", "xy22", u"255");
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap", "xy23", u"0");
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap", "xy31", u"0");
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap", "xy32", u"0");
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap", "xy33", u"1");

    // Check the colors in the diagonal
    OUString sDataRow = getXPath(pDocument, "/primitive2D/transform/transform/bitmap/data[50]", "row");
    std::vector<OUString> aPixels = comphelper::string::split(sDataRow, ',');
    CPPUNIT_ASSERT_EQUAL(u"ff0000"_ustr, aPixels[50]);

    sDataRow = getXPath(pDocument, "/primitive2D/transform/transform/bitmap/data[125]", "row");
    aPixels = comphelper::string::split(sDataRow, ',');
    CPPUNIT_ASSERT_EQUAL(u"ffff00"_ustr, aPixels[125]);

    sDataRow = getXPath(pDocument, "/primitive2D/transform/transform/bitmap/data[200]", "row");
    aPixels = comphelper::string::split(sDataRow, ',');
    CPPUNIT_ASSERT_EQUAL(u"ffff00"_ustr, aPixels[200]);
}

CPPUNIT_TEST_FIXTURE(Test, testScreenBlend)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/screenBlend.svg");

    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap", "height", u"150");
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap", "width", u"150");
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap/data", 150);

    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap", "xy11", u"150");
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap", "xy12", u"0");
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap", "xy13", u"0");
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap", "xy21", u"0");
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap", "xy22", u"150");
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap", "xy23", u"0");
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap", "xy31", u"0");
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap", "xy32", u"0");
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap", "xy33", u"1");

    // Check the colors in the diagonal
    OUString sDataRow = getXPath(pDocument, "/primitive2D/transform/transform/bitmap/data[25]", "row");
    std::vector<OUString> aPixels = comphelper::string::split(sDataRow, ',');
    CPPUNIT_ASSERT_EQUAL(u"ff0000"_ustr, aPixels[25]);

    sDataRow = getXPath(pDocument, "/primitive2D/transform/transform/bitmap/data[75]", "row");
    aPixels = comphelper::string::split(sDataRow, ',');
    CPPUNIT_ASSERT_EQUAL(u"ff8000"_ustr, aPixels[75]);

    sDataRow = getXPath(pDocument, "/primitive2D/transform/transform/bitmap/data[125]", "row");
    aPixels = comphelper::string::split(sDataRow, ',');
    CPPUNIT_ASSERT_EQUAL(u"008000"_ustr, aPixels[125]);
}

CPPUNIT_TEST_FIXTURE(Test, testMultiplyBlend)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/multiplyBlend.svg");

    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap", "height", u"150");
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap", "width", u"150");
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap/data", 150);

    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap", "xy11", u"150");
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap", "xy12", u"0");
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap", "xy13", u"0");
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap", "xy21", u"0");
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap", "xy22", u"150");
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap", "xy23", u"0");
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap", "xy31", u"0");
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap", "xy32", u"0");
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap", "xy33", u"1");

    // Check the colors in the diagonal
    OUString sDataRow = getXPath(pDocument, "/primitive2D/transform/transform/bitmap/data[25]", "row");
    std::vector<OUString> aPixels = comphelper::string::split(sDataRow, ',');
    CPPUNIT_ASSERT_EQUAL(u"ff0000"_ustr, aPixels[25]);

    sDataRow = getXPath(pDocument, "/primitive2D/transform/transform/bitmap/data[75]", "row");
    aPixels = comphelper::string::split(sDataRow, ',');
    CPPUNIT_ASSERT_EQUAL(u"000000"_ustr, aPixels[75]);

    sDataRow = getXPath(pDocument, "/primitive2D/transform/transform/bitmap/data[125]", "row");
    aPixels = comphelper::string::split(sDataRow, ',');
    CPPUNIT_ASSERT_EQUAL(u"008000"_ustr, aPixels[125]);
}

CPPUNIT_TEST_FIXTURE(Test, testDarkenBlend)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/darkenBlend.svg");

    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap", "height", u"150");
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap", "width", u"150");
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap/data", 150);

    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap", "xy11", u"150");
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap", "xy12", u"0");
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap", "xy13", u"0");
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap", "xy21", u"0");
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap", "xy22", u"150");
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap", "xy23", u"0");
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap", "xy31", u"0");
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap", "xy32", u"0");
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap", "xy33", u"1");

    // Check the colors in the diagonal
    OUString sDataRow = getXPath(pDocument, "/primitive2D/transform/transform/bitmap/data[25]", "row");
    std::vector<OUString> aPixels = comphelper::string::split(sDataRow, ',');
    CPPUNIT_ASSERT_EQUAL(u"ff0000"_ustr, aPixels[25]);

    sDataRow = getXPath(pDocument, "/primitive2D/transform/transform/bitmap/data[75]", "row");
    aPixels = comphelper::string::split(sDataRow, ',');
    CPPUNIT_ASSERT_EQUAL(u"000000"_ustr, aPixels[75]);

    sDataRow = getXPath(pDocument, "/primitive2D/transform/transform/bitmap/data[125]", "row");
    aPixels = comphelper::string::split(sDataRow, ',');
    CPPUNIT_ASSERT_EQUAL(u"008000"_ustr, aPixels[125]);
}

CPPUNIT_TEST_FIXTURE(Test, testLightenBlend)
{
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/lightenBlend.svg");

    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap", "height", u"150");
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap", "width", u"150");
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap/data", 150);

    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap", "xy11", u"150");
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap", "xy12", u"0");
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap", "xy13", u"0");
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap", "xy21", u"0");
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap", "xy22", u"150");
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap", "xy23", u"0");
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap", "xy31", u"0");
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap", "xy32", u"0");
    assertXPath(pDocument,
            "/primitive2D/transform/transform/bitmap", "xy33", u"1");

    // Check the colors in the diagonal
    OUString sDataRow = getXPath(pDocument, "/primitive2D/transform/transform/bitmap/data[25]", "row");
    std::vector<OUString> aPixels = comphelper::string::split(sDataRow, ',');
    CPPUNIT_ASSERT_EQUAL(u"ff0000"_ustr, aPixels[25]);

    sDataRow = getXPath(pDocument, "/primitive2D/transform/transform/bitmap/data[75]", "row");
    aPixels = comphelper::string::split(sDataRow, ',');
    CPPUNIT_ASSERT_EQUAL(u"ff8000"_ustr, aPixels[75]);

    sDataRow = getXPath(pDocument, "/primitive2D/transform/transform/bitmap/data[125]", "row");
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
            "/primitive2D/transform/mask/unhandled", "id", u"PATTERNFILL");
    assertXPath(pDocument,
            "/primitive2D/transform/mask/unhandled/mask/transform/transform/bitmap", 28);
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

    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "text", u"012");
    assertXPath(pDocument, "/primitive2D/transform/textsimpleportion[1]", "fontcolor", u"#ff0000");
    assertXPath(
        pDocument, "/primitive2D/transform/textsimpleportion[1]", "familyname", u"Open Symbol");
}

CPPUNIT_TEST_FIXTURE(Test, testTspanFillOpacity)
{
    // Given an SVG file with <tspan fill-opacity="0.30">:
    // When rendering that SVG:
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/tspan-fill-opacity.svg");

    // Then make sure that the text portion is wrapped in a transparency primitive with the correct
    // transparency value:
    sal_Int32 nTransparence = getXPath(pDocument, "//textsimpleportion[@text='hello']/parent::unifiedtransparence", "transparence").toInt32();
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

    assertXPath(pDocument, "//textsimpleportion", 2);
    assertXPath(pDocument, "//textsimpleportion[1]", "y", u"20");
    // Then make sure that the vertical offset is based on font-size of tspan, not of its parent.
    // Given the parent's font-size is 16 px, the expected vertical offset is 1.5 * (16 * 0.5) = 12,
    // which means that the resulting y is expected to be 20 + 12 = 32.
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 32
    // - Actual  : 44
    // i.e. the offset was calculated as 1.5 multiplied by the parent's font-size of 16 px,
    // not by the current tspan's half font-size.
    assertXPath(pDocument, "//textsimpleportion[2]", "y", u"32");
}

CPPUNIT_TEST_FIXTURE(Test, testExs)
{
    // tdf#160594, tdf#160717 given an SVG file with <tspan dy="3ex" style="font-size:1ex">:
    xmlDocUniquePtr pDocument = dumpAndParseSvg(u"/svgio/qa/cppunit/data/dy_in_exs.svg");

    assertXPath(pDocument, "//textsimpleportion", 2);
    assertXPath(pDocument, "//textsimpleportion[1]", "height", u"16");
    assertXPath(pDocument, "//textsimpleportion[1]", "y", u"20");

    sal_Int32 nSize = getXPath(pDocument, "//textsimpleportion[2]", "height").toInt32();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected less than: 16
    // - Actual  : 16
    // i.e. the parent font-size was used, instead of its x-size.
    CPPUNIT_ASSERT_LESS(sal_Int32(16), nSize);

    sal_Int32 nYPos = getXPath(pDocument, "//textsimpleportion[2]", "y").toInt32();
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
