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

#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>

#include <svx/svdpage.hxx>
#include <svx/svdorect.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xflclit.hxx>
#include <svx/xlineit0.hxx>
#include <svx/xlnclit.hxx>

#include <DrawDocShell.hxx>
#include <drawdoc.hxx>
#include <unomodel.hxx>

#include <osl/file.hxx>

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

} // namespace

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
