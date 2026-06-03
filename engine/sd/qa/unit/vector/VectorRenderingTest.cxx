/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sdmodeltestbase.hxx>

#include <test/JsonTestTools.hxx>
#include <tools/JsonPath.hxx>
#include <tools/json_writer.hxx>
#include <tools/color.hxx>
#include <tools/degree.hxx>

#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>

#include <svx/svdopage.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdorect.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xflclit.hxx>
#include <svx/xfltrit.hxx>
#include <svx/xlineit0.hxx>
#include <svx/xlnclit.hxx>

#include <DrawDocShell.hxx>
#include <drawdoc.hxx>
#include <unomodel.hxx>

#include <osl/file.hxx>

#include <cmath>
#include <fstream>
#include <string_view>

namespace
{
class VectorRenderingTest : public SdModelTestBase, public JsonTestTools
{
public:
    VectorRenderingTest()
        : SdModelTestBase(u""_ustr)
    {
    }

protected:
    /// Load a fresh empty Impress document. Clears the first slide and
    /// its master page of all default placeholder objects so tests
    /// only see what they explicitly add.
    void createBlankDoc()
    {
        createSdImpressDoc();
        SdrPage* pPage = page(1);
        pPage->ClearSdrObjList();
        if (pPage->TRG_HasMasterPage())
            pPage->TRG_GetMasterPage().ClearSdrObjList();
    }

    /// Page at the given 0-based index. Page 0 is the handout page,
    /// page 1 is the first user-visible slide.
    SdrPage* page(sal_uInt16 nIndex)
    {
        SdDrawDocument* pDrawDoc = getSdDocShell()->GetDoc();
        CPPUNIT_ASSERT(pDrawDoc);
        SdrPage* pPage = pDrawDoc->GetPage(nIndex);
        CPPUNIT_ASSERT(pPage);
        return pPage;
    }

    /// Add a filled rectangle with a border to the first slide.
    /// rRect is in 1/100 mm.
    void addRectangle(const tools::Rectangle& rRect, Color aFillColor, Color aStrokeColor)
    {
        SdrPage* pPage = page(1);
        rtl::Reference<SdrRectObj> pRect = new SdrRectObj(pPage->getSdrModelFromSdrPage(), rRect);

        pRect->SetMergedItem(XFillStyleItem(drawing::FillStyle_SOLID));
        pRect->SetMergedItem(XFillColorItem(OUString(), aFillColor));
        pRect->SetMergedItem(XLineStyleItem(drawing::LineStyle_SOLID));
        pRect->SetMergedItem(XLineColorItem(OUString(), aStrokeColor));

        pPage->NbcInsertObject(pRect.get());
    }

    /// Add a page-object placeholder (slide embedded in slide) to the
    /// first slide. Its view-independent decomposition is a single
    /// polygonHairline outline (drawn yellow by the engine), which is
    /// exactly what the vector tile pipeline emits.
    void addPageObject(const tools::Rectangle& rRect)
    {
        SdrPage* pPage = page(1);
        rtl::Reference<SdrPageObj> pPageObj
            = new SdrPageObj(pPage->getSdrModelFromSdrPage(), rRect);
        pPage->NbcInsertObject(pPageObj.get());
    }

    /// Add a stroke-only rectangle (no fill) to the first slide.
    void addStrokedRectangle(const tools::Rectangle& rRect, Color aStrokeColor)
    {
        SdrPage* pPage = page(1);
        rtl::Reference<SdrRectObj> pRect = new SdrRectObj(pPage->getSdrModelFromSdrPage(), rRect);

        pRect->SetMergedItem(XFillStyleItem(drawing::FillStyle_NONE));
        pRect->SetMergedItem(XLineStyleItem(drawing::LineStyle_SOLID));
        pRect->SetMergedItem(XLineColorItem(OUString(), aStrokeColor));

        pPage->NbcInsertObject(pRect.get());
    }

    /// Add a filled rectangle with a non-zero fill transparency
    /// (0..100 percent).
    void addTransparentRectangle(const tools::Rectangle& rRect, Color aFillColor,
                                 sal_uInt16 nTransparencePercent)
    {
        SdrPage* pPage = page(1);
        rtl::Reference<SdrRectObj> pRect = new SdrRectObj(pPage->getSdrModelFromSdrPage(), rRect);

        pRect->SetMergedItem(XFillStyleItem(drawing::FillStyle_SOLID));
        pRect->SetMergedItem(XFillColorItem(OUString(), aFillColor));
        pRect->SetMergedItem(XFillTransparenceItem(nTransparencePercent));
        pRect->SetMergedItem(XLineStyleItem(drawing::LineStyle_NONE));

        pPage->NbcInsertObject(pRect.get());
    }

    /// Add a filled rectangle with object metadata (name, title,
    /// description) on the first slide.
    void addRectangleWithObjectInfo(const tools::Rectangle& rRect, Color aFillColor,
                                    const OUString& rName, const OUString& rTitle,
                                    const OUString& rDesc)
    {
        SdrPage* pPage = page(1);
        rtl::Reference<SdrRectObj> pRect = new SdrRectObj(pPage->getSdrModelFromSdrPage(), rRect);

        pRect->SetMergedItem(XFillStyleItem(drawing::FillStyle_SOLID));
        pRect->SetMergedItem(XFillColorItem(OUString(), aFillColor));
        pRect->SetMergedItem(XLineStyleItem(drawing::LineStyle_NONE));

        pRect->SetName(rName);
        pRect->SetTitle(rTitle);
        pRect->SetDescription(rDesc);

        pPage->NbcInsertObject(pRect.get());
    }

    /// Add a filled rectangle rotated by the given angle (in 1/100 of
    /// a degree) around its centre.
    void addRotatedRectangle(const tools::Rectangle& rRect, Color aFillColor, Degree100 nAngle)
    {
        SdrPage* pPage = page(1);
        rtl::Reference<SdrRectObj> pRect = new SdrRectObj(pPage->getSdrModelFromSdrPage(), rRect);

        pRect->SetMergedItem(XFillStyleItem(drawing::FillStyle_SOLID));
        pRect->SetMergedItem(XFillColorItem(OUString(), aFillColor));
        pRect->SetMergedItem(XLineStyleItem(drawing::LineStyle_NONE));

        // Degree100 is 1/100 of a degree, so divide by 18000 for radians per pi.
        const double fAngleRad = nAngle.get() * M_PI / 18000.0;
        pRect->NbcRotate(rRect.Center(), nAngle, std::sin(fAngleRad), std::cos(fAngleRad));

        pPage->NbcInsertObject(pRect.get());
    }

    /// Request for the first slide. The raw JSON is written as a reference.
    tools::JsonPath getVectorTile(std::u16string_view sName)
    {
        SdXImpressDocument* pDoc = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
        CPPUNIT_ASSERT(pDoc);

        tools::JsonWriter aJsonWriter;
        // Explicitly get only part 0 -> first slide
        pDoc->getCommandValues(aJsonWriter, ".uno:VectorTile?part=0");
        OString aResult = aJsonWriter.finishAndGetAsOString();
        CPPUNIT_ASSERT(!aResult.isEmpty());

        // Write the wire-format JSON to workdir.
        static constexpr OUString sFolder = u"/VectorRenderingReference/"_ustr;
        osl::Directory::createPath(m_directories.getURLFromWorkdir(sFolder));
        OUString aPath = m_directories.getPathFromWorkdir(sFolder) + sName + u".json";
        std::ofstream aOut(OUStringToOString(aPath, RTL_TEXTENCODING_UTF8).getStr());
        CPPUNIT_ASSERT_MESSAGE("cannot open reference file for writing", aOut.is_open());
        aOut.write(aResult.getStr(), aResult.getLength());
        CPPUNIT_ASSERT_MESSAGE("write to reference file failed", aOut.good());

        auto oJson
            = tools::JsonPath::parse(std::string_view(aResult.getStr(), aResult.getLength()));
        CPPUNIT_ASSERT_MESSAGE("JSON parse error", oJson.has_value());

        return *oJson;
    }
};

CPPUNIT_TEST_FIXTURE(VectorRenderingTest, testSingleRectangle)
{
    // A single blue filled rectangle with a black border. The primitive
    // tree should contain a polyPolygonColor for the fill and a
    // polygonStroke for the border, under exactly one slide object.
    createBlankDoc();
    addRectangle(tools::Rectangle(Point(5000, 5000), Size(5000, 3000)), Color(0x4472c4), COL_BLACK);

    auto aJson = getVectorTile(u"testSingleRectangle");

    assertJsonPath(aJson, "/type", "vectortile");
    assertJsonPath(aJson, "/part", sal_Int64(0));

    // The master page has been cleared, so it contributes only the
    // page background fill and the page fill itself.
    CPPUNIT_ASSERT_EQUAL(size_t(2), aJson.getSize("/masterPage/primitives").value_or(0));
    assertJsonPath(aJson, "/masterPage/primitives/0/type", "backgroundcolor");
    assertJsonPath(aJson, "/masterPage/primitives/1/type", "polyPolygonColor");
    assertJsonPath(aJson, "/masterPage/primitives/1/color", "#ffffff");

    // Exactly one slide object, our rectangle
    CPPUNIT_ASSERT_EQUAL(size_t(1), aJson.getSize("/objects").value_or(0));
    assertJsonPath(aJson, "/objects/0/primitives/0/type", "svx:9");
    assertJsonPath(aJson, "/objects/0/primitives/0/children/0/type", "group");

    // Fill primitive.
    auto oFill = aJson.at("/objects/0/primitives/0/children/0/children/0");
    CPPUNIT_ASSERT(oFill.has_value());
    assertJsonPath(*oFill, "type", "polyPolygonColor");
    assertJsonPath(*oFill, "color", "#4472c4");
    assertJsonPathExists(*oFill, "path");

    // Stroke primitive.
    auto oStroke = aJson.at("/objects/0/primitives/0/children/0/children/1");
    CPPUNIT_ASSERT(oStroke.has_value());
    assertJsonPath(*oStroke, "type", "polygonStroke");
    assertJsonPath(*oStroke, "line/color", "#000000");
    assertJsonPathExists(*oStroke, "path");
}

CPPUNIT_TEST_FIXTURE(VectorRenderingTest, testStrokedRectangle)
{
    // A stroke-only rectangle decomposes to a polygonStroke primitive
    // under the slide object.
    createBlankDoc();
    addStrokedRectangle(tools::Rectangle(Point(5000, 5000), Size(5000, 3000)), COL_BLACK);

    auto aJson = getVectorTile(u"testStrokedRectangle");

    assertJsonPath(aJson, "/type", "vectortile");
    CPPUNIT_ASSERT_EQUAL(size_t(1), aJson.getSize("/objects").value_or(0));

    auto oStroke = aJson.at("/objects/0/primitives/0/children/0/children/0");
    CPPUNIT_ASSERT(oStroke.has_value());
    assertJsonPath(*oStroke, "type", "polygonStroke");
    assertJsonPath(*oStroke, "line/color", "#000000");
    assertJsonPathExists(*oStroke, "path");
}

CPPUNIT_TEST_FIXTURE(VectorRenderingTest, testRotatedRectangle)
{
    // A rotated rectangle decomposes through a TransformPrimitive2D
    // wrapper that carries the rotation matrix.
    createBlankDoc();
    addRotatedRectangle(tools::Rectangle(Point(5000, 5000), Size(5000, 3000)), Color(0x4472c4),
                        Degree100(4500));

    auto aJson = getVectorTile(u"testRotatedRectangle");

    assertJsonPath(aJson, "/type", "vectortile");
    CPPUNIT_ASSERT_EQUAL(size_t(1), aJson.getSize("/objects").value_or(0));
}

CPPUNIT_TEST_FIXTURE(VectorRenderingTest, testObjectInfo)
{
    // A rectangle with a name, title and description wraps its
    // decomposition in an ObjectInfoPrimitive2D.
    createBlankDoc();
    addRectangleWithObjectInfo(tools::Rectangle(Point(5000, 5000), Size(5000, 3000)),
                               Color(0x4472c4), u"Rectangle 1"_ustr, u"My title"_ustr,
                               u"My description"_ustr);

    auto aJson = getVectorTile(u"testObjectInfo");

    assertJsonPath(aJson, "/type", "vectortile");
    CPPUNIT_ASSERT_EQUAL(size_t(1), aJson.getSize("/objects").value_or(0));

    // The object info wraps the SdrObject's primitive sequence, so it
    // is the outermost node. The wrapping path is objectInfo -> svx:N
    // -> group -> [fill].
    auto oObjectInfo = aJson.at("/objects/0/primitives/0");
    CPPUNIT_ASSERT(oObjectInfo.has_value());
    assertJsonPath(*oObjectInfo, "type", "objectInfo");
    assertJsonPath(*oObjectInfo, "name", "Rectangle 1");
    assertJsonPath(*oObjectInfo, "title", "My title");
    assertJsonPath(*oObjectInfo, "desc", "My description");
}

CPPUNIT_TEST_FIXTURE(VectorRenderingTest, testPolyPolygonRGBA)
{
    // A solid-fill rectangle with a non-zero fill transparency
    // decomposes directly to a PolyPolygonRGBAPrimitive2D that
    // carries both the colour and the transparency, instead of
    // wrapping a PolyPolygonColorPrimitive2D in a transparency
    // wrapper. The reference JSON captures that wire shape.
    createBlankDoc();
    // 25 percent transparency.
    addTransparentRectangle(tools::Rectangle(Point(5000, 5000), Size(5000, 3000)), Color(0x4472c4),
                            25);

    auto aJson = getVectorTile(u"testPolyPolygonRGBA");

    assertJsonPath(aJson, "/type", "vectortile");
    CPPUNIT_ASSERT_EQUAL(size_t(1), aJson.getSize("/objects").value_or(0));

    // The wrapping path is svx:N -> group -> polyPolygonRGBA.
    auto oRGBA = aJson.at("/objects/0/primitives/0/children/0/children/0");
    CPPUNIT_ASSERT(oRGBA.has_value());
    assertJsonPath(*oRGBA, "type", "polyPolygonRGBA");
    assertJsonPath(*oRGBA, "color", "#4472c4");
    assertJsonPathExists(*oRGBA, "transparency");
}

CPPUNIT_TEST_FIXTURE(VectorRenderingTest, testPolygonHairline)
{
    // An SdrPageObj's view-independent decomposition is a single
    // yellow polygonHairline outline. The vector tile pipeline goes
    // through that exact path, so the reference JSON for the browser
    // mocha test gets a real polygonHairline emitted by the engine.
    createBlankDoc();
    addPageObject(tools::Rectangle(Point(5000, 5000), Size(5000, 3000)));

    auto aJson = getVectorTile(u"testPolygonHairline");

    assertJsonPath(aJson, "/type", "vectortile");
    CPPUNIT_ASSERT_EQUAL(size_t(1), aJson.getSize("/objects").value_or(0));

    auto oHairline = aJson.at("/objects/0/primitives/0");
    CPPUNIT_ASSERT(oHairline.has_value());
    assertJsonPath(*oHairline, "type", "polygonHairline");
    assertJsonPathExists(*oHairline, "color");
    assertJsonPathExists(*oHairline, "path");
}

} // namespace

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
