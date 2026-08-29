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

#include <vcl/metric.hxx>
#include <vcl/virdev.hxx>

#include <editeng/adjustitem.hxx>
#include <editeng/eeitem.hxx>

#include <svx/svdobjkind.hxx>
#include <svx/svdogrp.hxx>
#include <svx/svdopage.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdorect.hxx>
#include <svx/svdview.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xflclit.hxx>
#include <svx/xfltrit.hxx>
#include <svx/xlineit0.hxx>
#include <svx/xlnclit.hxx>

#include <editeng/editview.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/outliner.hxx>
#include <sfx2/viewsh.hxx>

#include <DrawDocShell.hxx>
#include <ViewShell.hxx>
#include <drawdoc.hxx>
#include <unomodel.hxx>

#include <osl/file.hxx>
#include <boost/property_tree/ptree.hpp>

#include <cmath>
#include <fstream>
#include <optional>
#include <set>
#include <string_view>

using namespace css;
using namespace ::cpo::uno;

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

    /// The entry of the object with the given id, or nothing when the response has none.
    static std::optional<tools::JsonPath> findEntryOfObject(const tools::JsonPath& rJson,
                                                            sal_uInt64 nObjectId)
    {
        const size_t nCount = rJson.getSize("/objects").value_or(0);
        for (size_t nIndex = 0; nIndex < nCount; ++nIndex)
        {
            const auto oEntry = rJson.at(rtl::Concat2View(
                "/objects/" + OString::number(sal_Int32(nIndex))));
            if (oEntry && oEntry->getInt("id").value_or(-1) == sal_Int64(nObjectId))
                return oEntry;
        }
        return std::nullopt;
    }

    /// The master of the first slide, with a placeholder of every kind that takes an area
    /// name in master view.
    SdPage* createMasterPlaceholders()
    {
        SdPage* pMasterPage = static_cast<SdPage*>(&page(1)->TRG_GetMasterPage());
        for (PresObjKind eKind : { PresObjKind::Title, PresObjKind::Header, PresObjKind::Footer,
                                   PresObjKind::DateTime, PresObjKind::SlideNumber })
        {
            pMasterPage->CreateDefaultPresObj(eKind);
        }
        return pMasterPage;
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

    /// Add a group holding one filled rectangle to the first slide.
    /// Returns the rectangle inside the group.
    SdrObject* addGroupedRectangle(const tools::Rectangle& rRect, Color aFillColor)
    {
        SdrPage* pPage = page(1);
        rtl::Reference<SdrRectObj> pRect = new SdrRectObj(pPage->getSdrModelFromSdrPage(), rRect);

        pRect->SetMergedItem(XFillStyleItem(drawing::FillStyle_SOLID));
        pRect->SetMergedItem(XFillColorItem(OUString(), aFillColor));
        pRect->SetMergedItem(XLineStyleItem(drawing::LineStyle_NONE));

        rtl::Reference<SdrObjGroup> pGroup = new SdrObjGroup(pPage->getSdrModelFromSdrPage());
        pGroup->GetSubList()->NbcInsertObject(pRect.get());
        pPage->NbcInsertObject(pGroup.get());
        return pRect.get();
    }

    /// Move the object and fire the object change the model sends on a real
    /// edit. A broadcast on its own says only that something may have
    /// happened, and the writer compares the object to find out.
    static void moveObject(SdrObject* pObject, const Size& rDistance)
    {
        pObject->NbcMove(rDistance);
        pObject->BroadcastObjectChange();
    }

    /// The first node under rNode whose type is sType, anywhere in the tree.
    static std::optional<tools::JsonPath> findNodeOfType(const tools::JsonPath& rNode,
                                                         const OString& rType)
    {
        if (rNode.getString("type").value_or(OString()) == rType)
            return rNode;
        for (const auto& rChild : rNode.tree())
        {
            auto oFound = findNodeOfType(rNode.sub(rChild.second), rType);
            if (oFound)
                return oFound;
        }
        return std::nullopt;
    }

    /// True when a node of the type with the given color is anywhere under rNode.
    static bool hasNodeWithColor(const tools::JsonPath& rNode, const OString& rType,
                                 const OString& rColor)
    {
        if (rNode.getString("type").value_or(OString()) == rType
            && rNode.getString("color").value_or(OString()) == rColor)
            return true;
        for (const auto& rChild : rNode.tree())
        {
            if (hasNodeWithColor(rNode.sub(rChild.second), rType, rColor))
                return true;
        }
        return false;
    }

    /// True when every entry that names a parent names one the same response
    /// reports, so a client can always resolve a member to its group.
    static bool everyParentResolves(const tools::JsonPath& rJson)
    {
        const size_t nCount = rJson.getSize("/objects").value_or(0);
        std::set<sal_Int64> aReported;
        for (size_t nIndex = 0; nIndex < nCount; ++nIndex)
        {
            aReported.insert(rJson.getInt(rtl::Concat2View("/objects/"
                                                           + OString::number(sal_Int32(nIndex))
                                                           + "/id"))
                                 .value_or(-1));
        }

        for (size_t nIndex = 0; nIndex < nCount; ++nIndex)
        {
            const sal_Int64 nParent
                = rJson.getInt(rtl::Concat2View("/objects/" + OString::number(sal_Int32(nIndex))
                                                + "/parent"))
                      .value_or(0);
            if (nParent != 0 && !aReported.contains(nParent))
                return false;
        }

        return true;
    }

    /// The entry of the given kind in the objects array, or nothing when the response carries
    /// none. An entry is named by its kind, while a primitive inside one is named by its type.
    static std::optional<tools::JsonPath> findEntryOfKind(const tools::JsonPath& rJson,
                                                          std::string_view sKind)
    {
        const OString aWanted(sKind);
        const size_t nCount = rJson.getSize("/objects").value_or(0);
        for (size_t nIndex = 0; nIndex < nCount; ++nIndex)
        {
            const auto oEntry = rJson.at(rtl::Concat2View(
                "/objects/" + OString::number(sal_Int32(nIndex))));
            if (oEntry && oEntry->getString("kind").value_or(OString()) == aWanted)
                return oEntry;
        }
        return std::nullopt;
    }

    /// True when the objects array of the given response carries the id.
    static bool carriesObject(const tools::JsonPath& rJson, sal_uInt64 nObjectId)
    {
        const size_t nCount = rJson.getSize("/objects").value_or(0);
        for (size_t nIndex = 0; nIndex < nCount; ++nIndex)
        {
            if (rJson.getInt(rtl::Concat2View("/objects/" + OString::number(sal_Int32(nIndex))
                                              + "/id"))
                    .value_or(-1)
                == sal_Int64(nObjectId))
                return true;
        }
        return false;
    }

    /// Add a page-object placeholder (slide embedded in slide) to the
    /// first slide. Its view-independent decomposition is a single
    /// polygonHairline outline (drawn yellow by the engine), which is
    /// exactly what the vector primitives pipeline emits.
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

    /// Raw JSON written by the most recent getVectorPrimitives call.
    OString m_aLastVectorJson;

    /// Add a text box to the first slide. Newlines in rText start new
    /// paragraphs. When oAdjust is set, it becomes the paragraph
    /// alignment.
    void addTextBox(const tools::Rectangle& rRect, const OUString& rText,
                    std::optional<SvxAdjust> oAdjust = std::nullopt)
    {
        SdrPage* pPage = page(1);
        rtl::Reference<SdrRectObj> pText
            = new SdrRectObj(pPage->getSdrModelFromSdrPage(), rRect, SdrObjKind::Text);
        pText->SetMergedItem(XFillStyleItem(drawing::FillStyle_NONE));
        pText->SetMergedItem(XLineStyleItem(drawing::LineStyle_NONE));
        pText->SetText(rText);
        if (oAdjust)
            pText->SetMergedItem(SvxAdjustItem(*oAdjust, EE_PARA_JUST));
        pPage->NbcInsertObject(pText.get());
    }

    /// Depth-first search for a text portion node whose text contains rText,
    /// anywhere under rNode.
    static std::optional<tools::JsonPath> findTextPortionUnder(const tools::JsonPath& rNode,
                                                               const OString& rText)
    {
        const OString sType = rNode.getString("type").value_or(OString());
        if ((sType == "textSimplePortion" || sType == "textDecoratedPortion")
            && rNode.getString("text").value_or(OString()).indexOf(rText) >= 0)
            return rNode;
        for (const auto& rChild : rNode.tree())
        {
            auto oFound = findTextPortionUnder(rNode.sub(rChild.second), rText);
            if (oFound)
                return oFound;
        }
        return std::nullopt;
    }

    /// The first text portion in the last vector JSON whose text contains
    /// rText, or std::nullopt when none does.
    std::optional<tools::JsonPath> findTextPortion(const OString& rText) const
    {
        auto oJson = tools::JsonPath::parse(
            std::string_view(m_aLastVectorJson.getStr(), m_aLastVectorJson.getLength()));
        if (!oJson)
            return std::nullopt;
        return findTextPortionUnder(*oJson, rText);
    }

    /// True when the environment resolves a real bold cut. Without one the
    /// drawing makes the weight up instead.
    static bool familyHasBoldCut()
    {
        ScopedVclPtrInstance<VirtualDevice> pProbeDevice;
        vcl::Font aProbeFont(u"Liberation Sans"_ustr, Size(0, 2000));
        aProbeFont.SetWeight(WEIGHT_BOLD);
        pProbeDevice->SetFont(aProbeFont);
        return !pProbeDevice->GetCurrentFontRawData().isEmpty()
               && pProbeDevice->GetFontMetric().GetWeight() >= WEIGHT_SEMIBOLD;
    }

    /// Request for part 0 of the page list nMode names. The raw JSON is
    /// written as a reference. A non-negative nSince asks for a delta against
    /// that version instead of the full page.
    tools::JsonPath getVectorPrimitives(std::u16string_view sName, sal_Int64 nSince = -1,
                                        sal_Int32 nMode = 0)
    {
        SdXImpressDocument* pDoc = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
        CPPUNIT_ASSERT(pDoc);

        tools::JsonWriter aJsonWriter;
        // Explicitly get only part 0 -> first page of the mode's list.
        OString aCommand = ".uno:VectorPrimitives?part=0&mode=" + OString::number(nMode);
        if (nSince >= 0)
            aCommand = aCommand + "&since=" + OString::number(nSince);
        pDoc->getCommandValues(aJsonWriter,
                               std::string_view(aCommand.getStr(), aCommand.getLength()));
        OString aResult = aJsonWriter.finishAndGetAsOString();
        CPPUNIT_ASSERT(!aResult.isEmpty());
        m_aLastVectorJson = aResult;

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

    auto aJson = getVectorPrimitives(u"testSingleRectangle");

    assertJsonPath(aJson, "/type", "vectorprimitives");
    assertJsonPath(aJson, "/part", sal_Int64(0));

    // The page comes first. Its master page has been cleared, so it
    // contributes only the page background fill and the page fill itself.
    CPPUNIT_ASSERT_EQUAL(size_t(2), aJson.getSize("/objects").value_or(0));
    assertJsonPath(aJson, "/objects/0/kind", "page");
    // The page is entry zero, the id an object never has.
    CPPUNIT_ASSERT_EQUAL(sal_Int64(0), aJson.getInt("/objects/0/id").value_or(-1));
    CPPUNIT_ASSERT_EQUAL(size_t(2), aJson.getSize("/objects/0/primitives").value_or(0));
    assertJsonPath(aJson, "/objects/0/primitives/0/type", "backgroundcolor");
    assertJsonPath(aJson, "/objects/0/primitives/1/type", "polyPolygonColor");
    assertJsonPath(aJson, "/objects/0/primitives/1/color", "#ffffff");

    // Then the one slide object, our rectangle.
    assertJsonPath(aJson, "/objects/1/primitives/0/type", "svx:9");
    assertJsonPath(aJson, "/objects/1/primitives/0/children/0/type", "group");

    // Fill primitive.
    auto oFill = aJson.at("/objects/1/primitives/0/children/0/children/0");
    CPPUNIT_ASSERT(oFill.has_value());
    assertJsonPath(*oFill, "type", "polyPolygonColor");
    assertJsonPath(*oFill, "color", "#4472c4");
    assertJsonPathExists(*oFill, "path");

    // Stroke primitive.
    auto oStroke = aJson.at("/objects/1/primitives/0/children/0/children/1");
    CPPUNIT_ASSERT(oStroke.has_value());
    assertJsonPath(*oStroke, "type", "polygonStroke");
    assertJsonPath(*oStroke, "line/color", "#000000");
    assertJsonPathExists(*oStroke, "path");
}

CPPUNIT_TEST_FIXTURE(VectorRenderingTest, testPartVersionRisesOnObjectChange)
{
    // Changing an object on a slide must raise that slide's reported
    // content version.
    createBlankDoc();
    addRectangle(tools::Rectangle(Point(5000, 5000), Size(5000, 3000)), Color(0x4472c4), COL_BLACK);

    const sal_Int64 nBefore
        = getVectorPrimitives(u"testPartVersion").getInt("/version").value_or(-1);

    // A broadcast on its own says only that something may have happened, and
    // the comparison then finds nothing, so move the rectangle for real.
    moveObject(page(1)->GetObj(0), Size(500, 0));

    const sal_Int64 nAfter
        = getVectorPrimitives(u"testPartVersion").getInt("/version").value_or(-1);

    CPPUNIT_ASSERT_EQUAL(nBefore + 1, nAfter);
}

CPPUNIT_TEST_FIXTURE(VectorRenderingTest, testPartVersionRisesOnMasterChange)
{
    // Changing an object on a slide master must raise the reported
    // content version of the slide that uses that master.
    createBlankDoc();
    CPPUNIT_ASSERT(page(1)->TRG_HasMasterPage());

    const sal_Int64 nBefore
        = getVectorPrimitives(u"testMasterVersion").getInt("/version").value_or(-1);

    // Put a rectangle on the master of the first slide and fire the
    // object change the model would send on a real edit.
    SdrPage& rMasterPage = page(1)->TRG_GetMasterPage();
    rtl::Reference<SdrRectObj> pRect = new SdrRectObj(
        rMasterPage.getSdrModelFromSdrPage(), tools::Rectangle(Point(0, 0), Size(4000, 2000)));
    rMasterPage.NbcInsertObject(pRect.get());
    pRect->BroadcastObjectChange();

    const sal_Int64 nAfter
        = getVectorPrimitives(u"testMasterVersion").getInt("/version").value_or(-1);

    CPPUNIT_ASSERT_EQUAL(nBefore + 1, nAfter);
}

CPPUNIT_TEST_FIXTURE(VectorRenderingTest, testMasterViewCarriesTheMasterObjects)
{
    // In master view the master page is the page being shown, so its own
    // objects come back as the page objects rather than as master content
    // behind a slide.
    createBlankDoc();
    CPPUNIT_ASSERT(page(1)->TRG_HasMasterPage());

    SdrPage& rMasterPage = page(1)->TRG_GetMasterPage();
    const size_t nPlaceholderCount = rMasterPage.GetObjCount();
    rtl::Reference<SdrRectObj> pRect = new SdrRectObj(
        rMasterPage.getSdrModelFromSdrPage(), tools::Rectangle(Point(0, 0), Size(4000, 2000)));
    rMasterPage.NbcInsertObject(pRect.get());

    auto aMaster = getVectorPrimitives(u"testMasterView", -1, 1);
    assertJsonPath(aMaster, "/type", "vectorprimitives");
    CPPUNIT_ASSERT_EQUAL(sal_Int64(1), aMaster.getInt("/mode").value_or(-1));
    // The page entry, then the placeholders and the new rectangle.
    CPPUNIT_ASSERT_EQUAL(nPlaceholderCount + 2, aMaster.getSize("/objects").value_or(0));
    assertJsonPath(aMaster, "/objects/0/kind", "page");
    CPPUNIT_ASSERT_EQUAL(sal_Int64(pRect->GetUniqueID()),
                         aMaster
                             .getInt(rtl::Concat2View(
                                 "/objects/" + OString::number(sal_Int32(nPlaceholderCount + 1))
                                 + "/id"))
                             .value_or(-1));

    // The slide itself is still blank, and says so in the slide mode: only
    // the page entry.
    auto aSlide = getVectorPrimitives(u"testMasterViewSlide");
    CPPUNIT_ASSERT_EQUAL(sal_Int64(0), aSlide.getInt("/mode").value_or(-1));
    CPPUNIT_ASSERT_EQUAL(size_t(1), aSlide.getSize("/objects").value_or(0));
    assertJsonPath(aSlide, "/objects/0/kind", "page");
}

CPPUNIT_TEST_FIXTURE(VectorRenderingTest, testMasterEditRaisesBothVersions)
{
    // An edit on a master shows in master view and on every slide that uses
    // the master, so both parts report a higher version.
    createBlankDoc();
    CPPUNIT_ASSERT(page(1)->TRG_HasMasterPage());

    const sal_Int64 nMasterBefore
        = getVectorPrimitives(u"testMasterPartVersion", -1, 1).getInt("/version").value_or(-1);
    const sal_Int64 nSlideBefore
        = getVectorPrimitives(u"testMasterPartVersionSlide").getInt("/version").value_or(-1);

    SdrPage& rMasterPage = page(1)->TRG_GetMasterPage();
    rtl::Reference<SdrRectObj> pRect = new SdrRectObj(
        rMasterPage.getSdrModelFromSdrPage(), tools::Rectangle(Point(0, 0), Size(4000, 2000)));
    rMasterPage.NbcInsertObject(pRect.get());
    pRect->BroadcastObjectChange();

    CPPUNIT_ASSERT_GREATER(
        nMasterBefore,
        getVectorPrimitives(u"testMasterPartVersion", -1, 1).getInt("/version").value_or(-1));
    CPPUNIT_ASSERT_GREATER(
        nSlideBefore,
        getVectorPrimitives(u"testMasterPartVersionSlide").getInt("/version").value_or(-1));
}

CPPUNIT_TEST_FIXTURE(VectorRenderingTest, testNotesEditRaisesTheNotesVersion)
{
    // An edit on a notes page raises the notes part and leaves the slide's
    // part alone.
    createBlankDoc();
    const sal_Int64 nNotesBefore
        = getVectorPrimitives(u"testNotesVersion", -1, 2).getInt("/version").value_or(-1);
    const sal_Int64 nSlideBefore
        = getVectorPrimitives(u"testNotesVersionSlide").getInt("/version").value_or(-1);

    SdrPage* pNotesPage = page(2);
    rtl::Reference<SdrRectObj> pRect = new SdrRectObj(
        pNotesPage->getSdrModelFromSdrPage(), tools::Rectangle(Point(0, 0), Size(4000, 2000)));
    pNotesPage->NbcInsertObject(pRect.get());
    pRect->BroadcastObjectChange();

    CPPUNIT_ASSERT_GREATER(
        nNotesBefore,
        getVectorPrimitives(u"testNotesVersion", -1, 2).getInt("/version").value_or(-1));
    CPPUNIT_ASSERT_EQUAL(
        nSlideBefore,
        getVectorPrimitives(u"testNotesVersionSlide").getInt("/version").value_or(-1));
}

CPPUNIT_TEST_FIXTURE(VectorRenderingTest, testNotesMasterEditRaisesTheNotesVersion)
{
    // The notes master shows on every notes page that uses it, so an edit on
    // it raises the notes part and leaves the slide's part alone.
    createBlankDoc();
    SdrPage* pNotesPage = page(2);
    CPPUNIT_ASSERT(pNotesPage->TRG_HasMasterPage());

    const sal_Int64 nNotesBefore
        = getVectorPrimitives(u"testNotesMasterVersion", -1, 2).getInt("/version").value_or(-1);
    const sal_Int64 nSlideBefore
        = getVectorPrimitives(u"testNotesMasterVersionSlide").getInt("/version").value_or(-1);

    SdrPage& rNotesMaster = pNotesPage->TRG_GetMasterPage();
    rtl::Reference<SdrRectObj> pRect = new SdrRectObj(
        rNotesMaster.getSdrModelFromSdrPage(), tools::Rectangle(Point(0, 0), Size(4000, 2000)));
    rNotesMaster.NbcInsertObject(pRect.get());
    pRect->BroadcastObjectChange();

    CPPUNIT_ASSERT_GREATER(
        nNotesBefore,
        getVectorPrimitives(u"testNotesMasterVersion", -1, 2).getInt("/version").value_or(-1));
    CPPUNIT_ASSERT_EQUAL(
        nSlideBefore,
        getVectorPrimitives(u"testNotesMasterVersionSlide").getInt("/version").value_or(-1));
}

CPPUNIT_TEST_FIXTURE(VectorRenderingTest, testMasterViewDeltaCarriesChangedObject)
{
    // A delta in master view behaves as it does for a slide: the order lists
    // every object, only the changed one carries content.
    createBlankDoc();
    SdrPage& rMasterPage = page(1)->TRG_GetMasterPage();
    rtl::Reference<SdrRectObj> pRect = new SdrRectObj(
        rMasterPage.getSdrModelFromSdrPage(), tools::Rectangle(Point(0, 0), Size(4000, 2000)));
    rMasterPage.NbcInsertObject(pRect.get());
    pRect->BroadcastObjectChange();

    auto aFull = getVectorPrimitives(u"testMasterDeltaFull", -1, 1);
    const sal_Int64 nVersion = aFull.getInt("/version").value_or(-1);
    const size_t nObjectCount = aFull.getSize("/objects").value_or(0);

    moveObject(pRect.get(), Size(500, 0));

    auto aDelta = getVectorPrimitives(u"testMasterDeltaSince", nVersion, 1);
    assertJsonPath(aDelta, "/type", "vectorprimitivesdelta");
    CPPUNIT_ASSERT(carriesObject(aDelta, pRect->GetUniqueID()));
    CPPUNIT_ASSERT_EQUAL(nObjectCount, aFull.getSize("/objects").value_or(0));
}

CPPUNIT_TEST_FIXTURE(VectorRenderingTest, testTextEditEntryNamesItsView)
{
    // A view runs at most one text edit and several views can be editing at
    // once, even the same object, so an entry is named by the view whose edit
    // it carries rather than by what that edit is running on.
    createBlankDoc();
    addTextBox(tools::Rectangle(Point(1000, 1000), Size(6000, 3000)), u"Named"_ustr);
    SdrObject* pObject = page(1)->GetObj(0);

    SdrView* pView = getSdDocShell()->GetViewShell()->GetView();
    CPPUNIT_ASSERT(pView);
    pView->SdrBeginTextEdit(pObject);

    auto aJson = getVectorPrimitives(u"testTextEditEntryView");
    const auto oEntry = findEntryOfKind(aJson, "texteditoverlay");

    const SfxViewShell* pViewShell = SfxViewShell::Current();
    CPPUNIT_ASSERT(pViewShell);
    const sal_Int32 nViewId = sal_Int32(pViewShell->GetViewShellId().get());

    pView->SdrEndTextEdit();

    CPPUNIT_ASSERT(oEntry.has_value());
    CPPUNIT_ASSERT_EQUAL(sal_Int64(nViewId), oEntry->getInt("viewId").value_or(-1));
    // Below zero, where no object and no page can reach.
    CPPUNIT_ASSERT_EQUAL(sal_Int64(-1 - nViewId), oEntry->getInt("id").value_or(0));
    CPPUNIT_ASSERT_EQUAL(sal_Int64(pObject->GetUniqueID()), oEntry->getInt("parent").value_or(-1));
}

CPPUNIT_TEST_FIXTURE(VectorRenderingTest, testMasterTextEditIsServedInMasterView)
{
    // A master page is a part of master view, so an edit on one of its objects
    // travels there: the entry that carries the edit comes with the page, and
    // a keystroke moves the version of that part. The slide that uses the
    // master carries no such entry.
    createBlankDoc();
    SdPage* pMasterPage = createMasterPlaceholders();
    SdrObject* pTitle = pMasterPage->GetPresObj(PresObjKind::Title);
    CPPUNIT_ASSERT(pTitle);

    // The first pull is what marks the model as drawn from.
    const sal_Int64 nBefore
        = getVectorPrimitives(u"testMasterTextEditBase", -1, 1).getInt("/version").value_or(-1);

    SdrView* pView = getSdDocShell()->GetViewShell()->GetView();
    CPPUNIT_ASSERT(pView);
    CPPUNIT_ASSERT(pView->SdrBeginTextEdit(pTitle));

    auto aMaster = getVectorPrimitives(u"testMasterTextEdit", -1, 1);
    const auto oEntry = findEntryOfKind(aMaster, "texteditoverlay");
    const sal_Int64 nAtBegin = aMaster.getInt("/version").value_or(-1);

    pView->GetTextEditOutlinerView()->GetEditView().InsertText(u"X"_ustr);
    const sal_Int64 nAfterKey
        = getVectorPrimitives(u"testMasterTextEditKey", -1, 1).getInt("/version").value_or(-1);

    auto aSlide = getVectorPrimitives(u"testMasterTextEditSlide");
    const bool bSlideCarriesEdit = findEntryOfKind(aSlide, "texteditoverlay").has_value();

    pView->SdrEndTextEdit();

    CPPUNIT_ASSERT_MESSAGE("master view carries no entry for the edit", oEntry.has_value());
    CPPUNIT_ASSERT_EQUAL(sal_Int64(pTitle->GetUniqueID()), oEntry->getInt("parent").value_or(-1));
    CPPUNIT_ASSERT_GREATER(nBefore, nAtBegin);
    CPPUNIT_ASSERT_GREATER(nAtBegin, nAfterKey);
    CPPUNIT_ASSERT(!bSlideCarriesEdit);
}

CPPUNIT_TEST_FIXTURE(VectorRenderingTest, testEmptyTextEditKeepsItsEntry)
{
    // The order names the entry of a running edit for as long as the edit
    // runs, so the entry is written even when the edit holds no text. A client
    // that has the text from before a deletion then replaces it with nothing,
    // rather than keeping it or dropping the part because the order names an
    // entry it never got.
    createBlankDoc();
    addTextBox(tools::Rectangle(Point(1000, 1000), Size(6000, 3000)), u"Gone"_ustr);

    // The first pull is what marks the model as drawn from.
    const sal_Int64 nBase
        = getVectorPrimitives(u"testEmptyEditBase").getInt("/version").value_or(-1);

    SdrView* pView = getSdDocShell()->GetViewShell()->GetView();
    CPPUNIT_ASSERT(pView);
    CPPUNIT_ASSERT(pView->SdrBeginTextEdit(page(1)->GetObj(0)));
    const sal_Int64 nAtBegin
        = getVectorPrimitives(u"testEmptyEditBegin").getInt("/version").value_or(-1);

    EditView& rEditView = pView->GetTextEditOutlinerView()->GetEditView();
    rEditView.SetSelection(ESelection::All());
    rEditView.DeleteSelected();

    auto aDelta = getVectorPrimitives(u"testEmptyEditDelta", nAtBegin);
    // Since before the edit began, so the order the edit added its entry to travels.
    auto aFull = getVectorPrimitives(u"testEmptyEditSinceBase", nBase);
    const sal_Int64 nEntryId = -1 - sal_Int64(SfxViewShell::Current()->GetViewShellId().get());

    pView->SdrEndTextEdit();

    const auto oDeltaEntry = findEntryOfKind(aDelta, "texteditoverlay");
    CPPUNIT_ASSERT_MESSAGE("the delta carries no entry for the emptied edit",
                           oDeltaEntry.has_value());
    // What is left of the text is an empty portion, or nothing at all.
    const auto oPortion = findNodeOfType(*oDeltaEntry, "textSimplePortion"_ostr);
    CPPUNIT_ASSERT_EQUAL(OString(),
                         oPortion ? oPortion->getString("text").value_or(OString()) : OString());

    const auto oFullEntry = findEntryOfKind(aFull, "texteditoverlay");
    CPPUNIT_ASSERT_MESSAGE("the delta since before the edit carries no entry for it",
                           oFullEntry.has_value());
    bool bOrdered = false;
    const size_t nOrderCount = aFull.getSize("/order").value_or(0);
    for (size_t nIndex = 0; nIndex < nOrderCount; ++nIndex)
    {
        if (aFull.getInt(rtl::Concat2View("/order/" + OString::number(sal_Int32(nIndex))))
                .value_or(0)
            == nEntryId)
            bOrdered = true;
    }
    CPPUNIT_ASSERT_MESSAGE("the order does not name the entry", bOrdered);
}

CPPUNIT_TEST_FIXTURE(VectorRenderingTest, testTypingMovesThePartVersionAtOnce)
{
    // The model change the shapes follow waits for a pause in the typing, which
    // is far too slow for the text being typed. A keystroke moves the part's
    // version, so the entry that carries the edit travels at once and a client
    // does not drop the delta as one it already holds.
    createBlankDoc();
    addTextBox(tools::Rectangle(Point(1000, 1000), Size(6000, 3000)), u"Typed"_ustr);

    // The first pull is what marks the model as drawn from.
    getVectorPrimitives(u"testTypingBase");

    SdrView* pView = getSdDocShell()->GetViewShell()->GetView();
    CPPUNIT_ASSERT(pView);
    pView->SdrBeginTextEdit(page(1)->GetObj(0));

    const sal_Int64 nBefore
        = getVectorPrimitives(u"testTypingBeforeKey").getInt("/version").value_or(-1);

    pView->GetTextEditOutlinerView()->GetEditView().InsertText(u"X"_ustr);

    const sal_Int64 nAfter
        = getVectorPrimitives(u"testTypingAfterKey").getInt("/version").value_or(-1);

    // A model change follows a pause in the typing, and by then the text has
    // already travelled, so it adds nothing.
    auto aSettled = getVectorPrimitives(u"testTypingSettled", nAfter);
    CPPUNIT_ASSERT(!aSettled.has("/type"));

    pView->SdrEndTextEdit();

    CPPUNIT_ASSERT_GREATER(nBefore, nAfter);
}

CPPUNIT_TEST_FIXTURE(VectorRenderingTest, testObjectUnderTextEditSaysSo)
{
    // The object a text edit runs on hides its own text and says the edit is
    // running. An edit that ends having changed no text says nothing else, so
    // the object has to come again or it keeps the look it had while hidden.
    createBlankDoc();
    addTextBox(tools::Rectangle(Point(1000, 1000), Size(6000, 3000)), u"Framed"_ustr);
    SdrObject* pObject = page(1)->GetObj(0);

    CPPUNIT_ASSERT(!getVectorPrimitives(u"testTextEditFlagBefore")
                        .getBool("/objects/1/textEdit")
                        .has_value());

    SdrView* pView = getSdDocShell()->GetViewShell()->GetView();
    CPPUNIT_ASSERT(pView);
    pView->SdrBeginTextEdit(pObject);

    CPPUNIT_ASSERT_EQUAL(true, getVectorPrimitives(u"testTextEditFlagDuring")
                                   .getBool("/objects/1/textEdit")
                                   .value_or(false));

    // Nothing was typed, so ending the edit changes no text. The object still
    // has to come back with the text it had.
    pView->SdrEndTextEdit();

    auto aAfter = getVectorPrimitives(u"testTextEditFlagAfter");
    CPPUNIT_ASSERT(!aAfter.getBool("/objects/1/textEdit").has_value());

    const auto oObject = aAfter.at("/objects/1");
    CPPUNIT_ASSERT(oObject.has_value());
    CPPUNIT_ASSERT_MESSAGE("the object came back without its text",
                           findTextPortionUnder(*oObject, "Framed"_ostr).has_value());
}

CPPUNIT_TEST_FIXTURE(VectorRenderingTest, testEditViewOnlyContentTravels)
{
    // The prompt of an empty placeholder sits inside a wrapper the drawing
    // layer empties unless the output says it is an editing view. The payload
    // serves one, so the wrapper arrives full and a thumbnail leaves it shut.
    createSdImpressDoc();

    auto aJson = getVectorPrimitives(u"testEditViewOnlyContent");

    const auto oWrapper = findNodeOfType(aJson, "exclusiveEditView"_ostr);
    CPPUNIT_ASSERT_MESSAGE("no edit-view-only content in the payload", oWrapper.has_value());
    CPPUNIT_ASSERT_MESSAGE("the edit-view-only content arrived empty",
                           oWrapper->getSize("children").value_or(0) > 0);
}

CPPUNIT_TEST_FIXTURE(VectorRenderingTest, testNotesPageShowsTheSlide)
{
    // The page object on a notes page shows the slide, so its entry carries the
    // slide's content rather than the outline the drawing layer stands in with,
    // and a change on the slide reaches the notes page as a delta carrying it.
    createBlankDoc();
    addRectangle(tools::Rectangle(Point(1000, 1000), Size(3000, 2000)), Color(0x4472c4), COL_BLACK);
    SdPage* pNotesPage = static_cast<SdPage*>(page(2));
    SdrObject* pPreview = pNotesPage->GetPresObj(PresObjKind::Page);
    CPPUNIT_ASSERT(pPreview);

    auto aNotes = getVectorPrimitives(u"testNotesPreview", -1, 2);
    const sal_Int64 nVersion = aNotes.getInt("/version").value_or(-1);
    const auto oEntry = findEntryOfObject(aNotes, pPreview->GetUniqueID());
    CPPUNIT_ASSERT(oEntry.has_value());
    CPPUNIT_ASSERT_MESSAGE("the preview does not show the slide's rectangle",
                           hasNodeWithColor(*oEntry, "polyPolygonColor"_ostr, "#4472c4"_ostr));

    moveObject(page(1)->GetObj(0), Size(500, 0));

    auto aDelta = getVectorPrimitives(u"testNotesPreviewDelta", nVersion, 2);
    assertJsonPath(aDelta, "/type", "vectorprimitivesdelta");
    CPPUNIT_ASSERT_MESSAGE("the notes delta does not carry the preview",
                           findEntryOfObject(aDelta, pPreview->GetUniqueID()).has_value());
}

CPPUNIT_TEST_FIXTURE(VectorRenderingTest, testNotesPreviewIsFramed)
{
    // The office draws a gray frame around a page object, and the preview carries one too.
    createBlankDoc();
    SdPage* pNotesPage = static_cast<SdPage*>(page(2));
    const SdrObject* pPreview = pNotesPage->GetPresObj(PresObjKind::Page);
    CPPUNIT_ASSERT(pPreview);

    auto aNotes = getVectorPrimitives(u"testNotesPreviewFrame", -1, 2);
    const auto oEntry = findEntryOfObject(aNotes, pPreview->GetUniqueID());
    CPPUNIT_ASSERT(oEntry.has_value());
    CPPUNIT_ASSERT(findNodeOfType(*oEntry, "polygonHairline"_ostr).has_value());
}

CPPUNIT_TEST_FIXTURE(VectorRenderingTest, testEveryParentNamesAReportedObject)
{
    // A member names the group it sits in, and a client resolves that name
    // against the same response, so every name has to be in it.
    createBlankDoc();
    SdrObject* pMember = addGroupedRectangle(
        tools::Rectangle(Point(1000, 1000), Size(3000, 2000)), Color(0x4472c4));
    CPPUNIT_ASSERT(pMember);
    addRectangle(tools::Rectangle(Point(6000, 1000), Size(3000, 2000)), Color(0xc00000), COL_BLACK);

    CPPUNIT_ASSERT(everyParentResolves(getVectorPrimitives(u"testParentsResolve")));
}

CPPUNIT_TEST_FIXTURE(VectorRenderingTest, testNotesPageIsServedInItsOwnMode)
{
    // The notes pages are the third page list, so a view that shows them asks
    // in mode 2 and is served the notes page at that index rather than the
    // slide.
    createBlankDoc();

    auto aNotes = getVectorPrimitives(u"testNotesPage", -1, 2);
    assertJsonPath(aNotes, "/type", "vectorprimitives");
    CPPUNIT_ASSERT_EQUAL(sal_Int64(2), aNotes.getInt("/mode").value_or(-1));

    // The page comes first here as it does in the other modes, and the notes
    // page keeps the placeholders a blank slide has none of.
    assertJsonPath(aNotes, "/objects/0/kind", "page");
    CPPUNIT_ASSERT(aNotes.getSize("/objects").value_or(0) > 1);

    // The slide at the same index is blank apart from its page entry, so the
    // two modes really are different pages.
    auto aSlide = getVectorPrimitives(u"testNotesPageSlide");
    CPPUNIT_ASSERT_EQUAL(size_t(1), aSlide.getSize("/objects").value_or(0));
}

CPPUNIT_TEST_FIXTURE(VectorRenderingTest, testMissingPageStillAnswers)
{
    // A client whose page list has not caught up with a mode switch asks for
    // a master that is not there. The answer carries the header, so the
    // client can tell the request was seen instead of waiting forever.
    createBlankDoc();

    tools::JsonWriter aJsonWriter;
    SdXImpressDocument* pDoc = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pDoc);
    static constexpr OString aCommand = ".uno:VectorPrimitives?part=12&mode=1"_ostr;
    pDoc->getCommandValues(aJsonWriter, std::string_view(aCommand.getStr(), aCommand.getLength()));
    const OString aResult = aJsonWriter.finishAndGetAsOString();

    auto oJson = tools::JsonPath::parse(std::string_view(aResult.getStr(), aResult.getLength()));
    CPPUNIT_ASSERT_MESSAGE("JSON parse error", oJson.has_value());
    assertJsonPath(*oJson, "/type", "vectorprimitives");
    CPPUNIT_ASSERT_EQUAL(sal_Int64(12), oJson->getInt("/part").value_or(-1));
    CPPUNIT_ASSERT_EQUAL(sal_Int64(1), oJson->getInt("/mode").value_or(-1));
    // No page means no content, so nothing describes one.
    CPPUNIT_ASSERT(!oJson->has("/order"));
    CPPUNIT_ASSERT(!oJson->has("/objects"));
}

CPPUNIT_TEST_FIXTURE(VectorRenderingTest, testUnservedModeCarriesNoPage)
{
    // A mode outside the page lists the command serves names no page, so the
    // answer carries the header and no content rather than the slide at that
    // index.
    createBlankDoc();
    addRectangle(tools::Rectangle(Point(1000, 1000), Size(3000, 2000)), Color(0x4472c4), COL_BLACK);

    auto aJson = getVectorPrimitives(u"testUnservedMode", -1, 3);
    CPPUNIT_ASSERT_EQUAL(sal_Int64(3), aJson.getInt("/mode").value_or(-1));
    CPPUNIT_ASSERT(!aJson.has("/order"));
    CPPUNIT_ASSERT(!aJson.has("/objects"));
}

CPPUNIT_TEST_FIXTURE(VectorRenderingTest, testPartVersionRisesOnBackgroundChange)
{
    // Changing the slide's background fill must raise the slide's
    // reported content version.
    createBlankDoc();

    const sal_Int64 nBefore
        = getVectorPrimitives(u"testBackgroundVersion").getInt("/version").value_or(-1);

    // Set a solid background fill on the slide's page properties, the
    // same change the page dialog applies.
    SdrPageProperties& rPageProperties = page(1)->getSdrPageProperties();
    rPageProperties.PutItem(XFillStyleItem(drawing::FillStyle_SOLID));
    rPageProperties.PutItem(XFillColorItem(OUString(), Color(0x4472c4)));

    const sal_Int64 nAfter
        = getVectorPrimitives(u"testBackgroundVersion").getInt("/version").value_or(-1);

    CPPUNIT_ASSERT_GREATER(nBefore, nAfter);
}

CPPUNIT_TEST_FIXTURE(VectorRenderingTest, testDeltaCarriesResizedPage)
{
    // The page rectangle rides on the page entry, so a page that was resized
    // reaches a client that already holds the slide as a delta carrying that
    // entry with its new box.
    createBlankDoc();
    auto aFull = getVectorPrimitives(u"testResizeFull");
    const sal_Int64 nVersion = aFull.getInt("/version").value_or(-1);
    const sal_Int64 nWidthBefore = aFull.getInt("/objects/0/width").value_or(-1);

    // Through the drawing layer's page, which dispatches to the slide's own override.
    SdrPage* pPage = page(1);
    const Size aSize = pPage->GetSize();
    pPage->SetSize(Size(aSize.Width() * 2, aSize.Height()));

    auto aDelta = getVectorPrimitives(u"testResizeDelta", nVersion);
    assertJsonPath(aDelta, "/type", "vectorprimitivesdelta");
    // The page entry comes first whenever a response carries it.
    assertJsonPath(aDelta, "/objects/0/kind", "page");
    CPPUNIT_ASSERT_GREATER(nWidthBefore, aDelta.getInt("/objects/0/width").value_or(-1));
}

// A delta since a version carries full content only for objects that
// changed after it, while the order array still lists every object.
CPPUNIT_TEST_FIXTURE(VectorRenderingTest, testDeltaCarriesOnlyChangedObjects)
{
    createBlankDoc();
    addRectangle(tools::Rectangle(Point(1000, 1000), Size(3000, 2000)), Color(0x4472c4), COL_BLACK);
    addRectangle(tools::Rectangle(Point(6000, 1000), Size(3000, 2000)), Color(0xc00000), COL_BLACK);

    SdrObject* pFirst = page(1)->GetObj(0);
    // Look at both objects once so the part holds what was written for them.
    pFirst->BroadcastObjectChange();
    page(1)->GetObj(1)->BroadcastObjectChange();

    auto aFull = getVectorPrimitives(u"testDeltaFull");
    assertJsonPath(aFull, "/type", "vectorprimitives");
    const sal_Int64 nVersion = aFull.getInt("/version").value_or(-1);
    CPPUNIT_ASSERT_EQUAL(size_t(3), aFull.getSize("/objects").value_or(0));

    // Change only the first object after that version.
    moveObject(pFirst, Size(500, 0));

    auto aDelta = getVectorPrimitives(u"testDeltaSince", nVersion);
    assertJsonPath(aDelta, "/type", "vectorprimitivesdelta");
    // The object set did not move, so the client keeps the order it has and
    // only the changed object travels.
    CPPUNIT_ASSERT(!aDelta.has("/order"));
    CPPUNIT_ASSERT_EQUAL(size_t(1), aDelta.getSize("/objects").value_or(0));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int64>(pFirst->GetUniqueID()),
                         aDelta.getInt("/objects/0/id").value_or(-1));
}

CPPUNIT_TEST_FIXTURE(VectorRenderingTest, testEveryObjectKeepsItsEntry)
{
    // Every live object gets an entry in the objects array, even one that
    // draws nothing, so the ids the order array carries always resolve
    // against the object set the client holds.
    createBlankDoc();
    addRectangle(tools::Rectangle(Point(1000, 1000), Size(3000, 2000)), Color(0x4472c4), COL_BLACK);
    // A group with no members draws nothing on the slide.
    rtl::Reference<SdrObjGroup> pGroup = new SdrObjGroup(page(1)->getSdrModelFromSdrPage());
    page(1)->NbcInsertObject(pGroup.get());

    auto aFull = getVectorPrimitives(u"testObjectEntryFull");
    CPPUNIT_ASSERT_EQUAL(size_t(3), aFull.getSize("/objects").value_or(0));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int64>(pGroup->GetUniqueID()),
                         aFull.getInt("/objects/2/id").value_or(-1));
}

CPPUNIT_TEST_FIXTURE(VectorRenderingTest, testDeltaSkipsAnObjectThatOnlyBroadcast)
{
    // A broadcast says an object may have changed, not that it did. An object
    // that looks the same and sits in the same place as when it was last
    // written costs a comparison and travels no further.
    createBlankDoc();
    addRectangle(tools::Rectangle(Point(1000, 1000), Size(3000, 2000)), Color(0x4472c4), COL_BLACK);
    SdrObject* pObject = page(1)->GetObj(0);

    auto aFull = getVectorPrimitives(u"testUnchangedFull");
    const sal_Int64 nVersion = aFull.getInt("/version").value_or(-1);

    pObject->BroadcastObjectChange();

    // With nothing left to say the response is empty rather than a header
    // over empty arrays, so the push it came from sends no frame at all.
    auto aDelta = getVectorPrimitives(u"testUnchangedDelta", nVersion);
    CPPUNIT_ASSERT(!aDelta.has("/type"));
    CPPUNIT_ASSERT(!aDelta.has("/objects"));

    // The version did not move either, so the next delta starts from here.
    CPPUNIT_ASSERT_EQUAL(nVersion,
                         getVectorPrimitives(u"testUnchangedFullAgain")
                             .getInt("/version")
                             .value_or(-1));

    // Moving it really does change it, so then it travels.
    moveObject(pObject, Size(500, 0));
    auto aMoved = getVectorPrimitives(u"testUnchangedMovedDelta", nVersion);
    CPPUNIT_ASSERT_EQUAL(size_t(1), aMoved.getSize("/objects").value_or(0));
    CPPUNIT_ASSERT_EQUAL(sal_Int64(pObject->GetUniqueID()),
                         aMoved.getInt("/objects/0/id").value_or(-1));
}

// Moving a shape inside a group moves the box of the group as well, so a
// delta carries both of them and nothing else.
CPPUNIT_TEST_FIXTURE(VectorRenderingTest, testDeltaCarriesGroupOnMemberChange)
{
    createBlankDoc();
    SdrObject* pMember = addGroupedRectangle(
        tools::Rectangle(Point(1000, 1000), Size(3000, 2000)), Color(0x4472c4));
    SdrObject* pGroup = page(1)->GetObj(0);
    addRectangle(tools::Rectangle(Point(6000, 1000), Size(3000, 2000)), Color(0xc00000), COL_BLACK);

    const sal_Int64 nVersion
        = getVectorPrimitives(u"testGroupFull").getInt("/version").value_or(-1);

    // Move only the member inside the group after that version.
    moveObject(pMember, Size(500, 0));

    auto aDelta = getVectorPrimitives(u"testGroupDelta", nVersion);
    assertJsonPath(aDelta, "/type", "vectorprimitivesdelta");
    CPPUNIT_ASSERT_EQUAL(size_t(2), aDelta.getSize("/objects").value_or(0));
    CPPUNIT_ASSERT_EQUAL(sal_Int64(pGroup->GetUniqueID()),
                         aDelta.getInt("/objects/0/id").value_or(-1));
    CPPUNIT_ASSERT_EQUAL(sal_Int64(pMember->GetUniqueID()),
                         aDelta.getInt("/objects/1/id").value_or(-1));
}

// Raising an object above another changes nothing about the object itself,
// only where it paints in the order, so the delta carries the order.
CPPUNIT_TEST_FIXTURE(VectorRenderingTest, testDeltaCarriesOrderOnZOrderChange)
{
    createBlankDoc();
    addRectangle(tools::Rectangle(Point(1000, 1000), Size(3000, 2000)), Color(0x4472c4), COL_BLACK);
    addRectangle(tools::Rectangle(Point(2000, 2000), Size(3000, 2000)), Color(0xc00000), COL_BLACK);
    SdrObject* pLower = page(1)->GetObj(0);
    SdrObject* pUpper = page(1)->GetObj(1);

    const sal_Int64 nVersion
        = getVectorPrimitives(u"testZOrderFull").getInt("/version").value_or(-1);

    // Bring the lower one to the front.
    page(1)->SetObjectOrdNum(0, 1);

    auto aDelta = getVectorPrimitives(u"testZOrderDelta", nVersion);
    assertJsonPath(aDelta, "/type", "vectorprimitivesdelta");
    CPPUNIT_ASSERT_GREATER(nVersion, aDelta.getInt("/version").value_or(-1));
    // The page entry first, then the two in their new order.
    CPPUNIT_ASSERT_EQUAL(size_t(3), aDelta.getSize("/order").value_or(0));
    CPPUNIT_ASSERT_EQUAL(sal_Int64(pUpper->GetUniqueID()), aDelta.getInt("/order/1").value_or(-1));
    CPPUNIT_ASSERT_EQUAL(sal_Int64(pLower->GetUniqueID()), aDelta.getInt("/order/2").value_or(-1));
}

// The first paint order recorded for a part is what the first pull writes, so
// recording it moves nothing. A part pulled twice with nothing changed between
// reports the same version, and the delta between them carries no object.
CPPUNIT_TEST_FIXTURE(VectorRenderingTest, testFirstOrderRecordedMovesNothing)
{
    createBlankDoc();
    addRectangle(tools::Rectangle(Point(1000, 1000), Size(3000, 2000)), Color(0x4472c4), COL_BLACK);
    addRectangle(tools::Rectangle(Point(2000, 2000), Size(3000, 2000)), Color(0xc00000), COL_BLACK);

    const sal_Int64 nFirst
        = getVectorPrimitives(u"testFirstOrderFirst").getInt("/version").value_or(-1);
    const sal_Int64 nSecond
        = getVectorPrimitives(u"testFirstOrderSecond").getInt("/version").value_or(-1);
    CPPUNIT_ASSERT_EQUAL(nFirst, nSecond);

    auto aDelta = getVectorPrimitives(u"testFirstOrderDelta", nFirst);
    CPPUNIT_ASSERT_EQUAL(size_t(0), aDelta.getSize("/objects").value_or(0));
}

// Removing a member of a group shrinks the box of the group, so the delta
// carries the group entry with its new box along with the order.
CPPUNIT_TEST_FIXTURE(VectorRenderingTest, testDeltaCarriesGroupOnMemberRemoval)
{
    createBlankDoc();
    addGroupedRectangle(tools::Rectangle(Point(1000, 1000), Size(3000, 2000)), Color(0x4472c4));
    SdrObject* pGroup = page(1)->GetObj(0);
    rtl::Reference<SdrRectObj> pSecond = new SdrRectObj(
        page(1)->getSdrModelFromSdrPage(), tools::Rectangle(Point(6000, 1000), Size(3000, 2000)));
    pSecond->SetMergedItem(XFillStyleItem(drawing::FillStyle_SOLID));
    pSecond->SetMergedItem(XFillColorItem(OUString(), Color(0xc00000)));
    pGroup->GetSubList()->NbcInsertObject(pSecond.get());

    auto aFull = getVectorPrimitives(u"testGroupRemovalFull");
    const sal_Int64 nVersion = aFull.getInt("/version").value_or(-1);
    const auto oGroupBefore = findEntryOfObject(aFull, pGroup->GetUniqueID());
    CPPUNIT_ASSERT(oGroupBefore.has_value());
    const sal_Int64 nWidthBefore = oGroupBefore->getInt("width").value_or(-1);

    pGroup->GetSubList()->RemoveObject(1);

    auto aDelta = getVectorPrimitives(u"testGroupRemovalDelta", nVersion);
    assertJsonPath(aDelta, "/type", "vectorprimitivesdelta");
    const auto oGroupAfter = findEntryOfObject(aDelta, pGroup->GetUniqueID());
    CPPUNIT_ASSERT_MESSAGE("the delta does not carry the group", oGroupAfter.has_value());
    CPPUNIT_ASSERT_LESS(nWidthBefore, oGroupAfter->getInt("width").value_or(-1));
}

// A group is one entry and each member another, the members right after the
// group in the order, each naming the group as its parent. The members draw
// the group's content, so the group entry paints nothing itself.
CPPUNIT_TEST_FIXTURE(VectorRenderingTest, testGroupMembersAreOwnEntries)
{
    createBlankDoc();
    SdrObject* pMember = addGroupedRectangle(
        tools::Rectangle(Point(1000, 1000), Size(3000, 2000)), Color(0x4472c4));
    SdrObject* pGroup = page(1)->GetObj(0);

    auto aFull = getVectorPrimitives(u"testGroupMembers");
    CPPUNIT_ASSERT_EQUAL(size_t(3), aFull.getSize("/objects").value_or(0));
    CPPUNIT_ASSERT_EQUAL(sal_Int64(pGroup->GetUniqueID()),
                         aFull.getInt("/objects/1/id").value_or(-1));
    CPPUNIT_ASSERT_EQUAL(sal_Int64(0), aFull.getInt("/objects/1/parent").value_or(-1));
    CPPUNIT_ASSERT_EQUAL(size_t(0), aFull.getSize("/objects/1/primitives").value_or(SIZE_MAX));
    CPPUNIT_ASSERT_EQUAL(sal_Int64(pMember->GetUniqueID()),
                         aFull.getInt("/objects/2/id").value_or(-1));
    CPPUNIT_ASSERT_EQUAL(sal_Int64(pGroup->GetUniqueID()),
                         aFull.getInt("/objects/2/parent").value_or(-1));
    CPPUNIT_ASSERT(aFull.getSize("/objects/2/primitives").value_or(0) > 0);

    // Adding an object moves the set, so the next delta hands the client the
    // whole order: the page first, then the group and the member, then the
    // new object.
    const sal_Int64 nVersion = aFull.getInt("/version").value_or(-1);
    rtl::Reference<SdrRectObj> pAdded = new SdrRectObj(
        page(1)->getSdrModelFromSdrPage(), tools::Rectangle(Point(6000, 1000), Size(1000, 1000)));
    // Inserting through InsertObject is what tells the model a new object is there.
    page(1)->InsertObject(pAdded.get());

    auto aDelta = getVectorPrimitives(u"testGroupMembersDelta", nVersion);
    CPPUNIT_ASSERT_EQUAL(size_t(4), aDelta.getSize("/order").value_or(0));
    CPPUNIT_ASSERT_EQUAL(sal_Int64(0), aDelta.getInt("/order/0").value_or(-1));
    CPPUNIT_ASSERT_EQUAL(sal_Int64(pGroup->GetUniqueID()), aDelta.getInt("/order/1").value_or(-1));
    CPPUNIT_ASSERT_EQUAL(sal_Int64(pMember->GetUniqueID()), aDelta.getInt("/order/2").value_or(-1));
    CPPUNIT_ASSERT_EQUAL(sal_Int64(pAdded->GetUniqueID()), aDelta.getInt("/order/3").value_or(-1));
}

// Every entry carries where the object paints and how the unit rectangle
// maps onto it, both in twips.
CPPUNIT_TEST_FIXTURE(VectorRenderingTest, testObjectEntryCarriesGeometry)
{
    createBlankDoc();
    // No border, so the painted rectangle is the fill alone.
    addTransparentRectangle(tools::Rectangle(Point(1000, 2000), Size(4000, 3000)),
                            Color(0x4472c4), 0);

    auto aJson = getVectorPrimitives(u"testObjectGeometry");
    CPPUNIT_ASSERT_EQUAL(size_t(2), aJson.getSize("/objects").value_or(0));

    // 1/100 mm to twips is 1440 / 2540.
    CPPUNIT_ASSERT_EQUAL(sal_Int64(567), aJson.getInt("/objects/1/x").value_or(-1));
    CPPUNIT_ASSERT_EQUAL(sal_Int64(1134), aJson.getInt("/objects/1/y").value_or(-1));
    CPPUNIT_ASSERT_EQUAL(sal_Int64(2268), aJson.getInt("/objects/1/width").value_or(-1));
    CPPUNIT_ASSERT_EQUAL(sal_Int64(1701), aJson.getInt("/objects/1/height").value_or(-1));

    // An unrotated rectangle scales the unit square to its size and moves it
    // to its top-left corner.
    CPPUNIT_ASSERT_EQUAL(size_t(6), aJson.getSize("/objects/1/transform").value_or(0));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2267.7, aJson.getDouble("/objects/1/transform/0").value_or(0),
                                 0.1);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, aJson.getDouble("/objects/1/transform/1").value_or(1),
                                 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, aJson.getDouble("/objects/1/transform/2").value_or(1),
                                 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1700.8, aJson.getDouble("/objects/1/transform/3").value_or(0),
                                 0.1);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(566.9, aJson.getDouble("/objects/1/transform/4").value_or(0),
                                 0.1);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1133.9, aJson.getDouble("/objects/1/transform/5").value_or(0),
                                 0.1);
}

// A rotation shows up in the transform, while the painted rectangle stays
// the axis-aligned box around the rotated shape.
CPPUNIT_TEST_FIXTURE(VectorRenderingTest, testRotatedObjectTransform)
{
    createBlankDoc();
    addRotatedRectangle(tools::Rectangle(Point(5000, 5000), Size(4000, 4000)), Color(0x4472c4),
                        Degree100(9000));

    auto aJson = getVectorPrimitives(u"testRotatedTransform");
    CPPUNIT_ASSERT_EQUAL(size_t(2), aJson.getSize("/objects").value_or(0));

    // A quarter turn puts the whole scale into the off-diagonal entries.
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, aJson.getDouble("/objects/1/transform/0").value_or(1),
                                 0.5);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2267.7,
                                 std::abs(aJson.getDouble("/objects/1/transform/1").value_or(0)),
                                 0.5);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2267.7,
                                 std::abs(aJson.getDouble("/objects/1/transform/2").value_or(0)),
                                 0.5);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, aJson.getDouble("/objects/1/transform/3").value_or(1),
                                 0.5);

    // A square turned by a quarter covers the same box. The box is rounded to
    // whole twips, so a twip either way is what the rounding leaves.
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2835.0, double(aJson.getInt("/objects/1/x").value_or(-1)), 1.0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2835.0, double(aJson.getInt("/objects/1/y").value_or(-1)), 1.0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2268.0, double(aJson.getInt("/objects/1/width").value_or(-1)),
                                 1.0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2268.0, double(aJson.getInt("/objects/1/height").value_or(-1)),
                                 1.0);
}

// Each entry names the layer the object is on, and a placeholder that holds
// no content yet is flagged as such.
CPPUNIT_TEST_FIXTURE(VectorRenderingTest, testObjectEntryCarriesLayerAndPlaceholderFlag)
{
    createBlankDoc();
    addRectangle(tools::Rectangle(Point(1000, 1000), Size(3000, 2000)), Color(0x4472c4), COL_BLACK);
    addRectangle(tools::Rectangle(Point(5000, 1000), Size(3000, 2000)), Color(0x4472c4), COL_BLACK);

    SdrObject* pOnOtherLayer = page(1)->GetObj(0);
    pOnOtherLayer->NbcSetLayer(SdrLayerID(3));
    SdrObject* pPlaceholder = page(1)->GetObj(1);
    pPlaceholder->SetEmptyPresObj(true);

    auto aJson = getVectorPrimitives(u"testLayerAndPlaceholder");
    CPPUNIT_ASSERT_EQUAL(size_t(3), aJson.getSize("/objects").value_or(0));
    CPPUNIT_ASSERT_EQUAL(sal_Int64(3), aJson.getInt("/objects/1/layer").value_or(-1));
    CPPUNIT_ASSERT(!aJson.getBool("/objects/1/emptyPlaceholder").has_value());
    CPPUNIT_ASSERT_EQUAL(sal_Int64(pPlaceholder->GetLayer().get()),
                         aJson.getInt("/objects/2/layer").value_or(-1));
    CPPUNIT_ASSERT_EQUAL(true, aJson.getBool("/objects/2/emptyPlaceholder").value_or(false));
}

// Text in the automatic color resolves against the page background, so it
// comes out light on a dark page and dark on a light one.
CPPUNIT_TEST_FIXTURE(VectorRenderingTest, testAutoColorFollowsPageBackground)
{
    createBlankDoc();
    addTextBox(tools::Rectangle(Point(1000, 1000), Size(8000, 2000)), u"Hello"_ustr);

    getVectorPrimitives(u"testAutoColorLight");
    auto oLight = findTextPortion("Hello"_ostr);
    CPPUNIT_ASSERT(oLight.has_value());
    assertJsonPath(*oLight, "fontcolor", "#000000");

    SdrPageProperties& rProperties = page(1)->getSdrPageProperties();
    rProperties.PutItem(XFillStyleItem(drawing::FillStyle_SOLID));
    rProperties.PutItem(XFillColorItem(OUString(), COL_BLACK));

    getVectorPrimitives(u"testAutoColorDark");
    auto oDark = findTextPortion("Hello"_ostr);
    CPPUNIT_ASSERT(oDark.has_value());
    assertJsonPath(*oDark, "fontcolor", "#ffffff");
}

// A master-page change is not an object on the slide, so a delta whose
// baseline predates it must carry the master page content.
CPPUNIT_TEST_FIXTURE(VectorRenderingTest, testDeltaCarriesChangedMasterPage)
{
    createBlankDoc();
    addRectangle(tools::Rectangle(Point(1000, 1000), Size(3000, 2000)), Color(0x4472c4), COL_BLACK);
    page(1)->GetObj(0)->BroadcastObjectChange();

    auto aFull = getVectorPrimitives(u"testMasterDeltaFull");
    const sal_Int64 nVersion = aFull.getInt("/version").value_or(-1);

    // Put a rectangle on the master after that version and fire the
    // object change the model would send on a real edit.
    SdrPage& rMasterPage = page(1)->TRG_GetMasterPage();
    rtl::Reference<SdrRectObj> pRect = new SdrRectObj(
        rMasterPage.getSdrModelFromSdrPage(), tools::Rectangle(Point(0, 0), Size(4000, 2000)));
    rMasterPage.NbcInsertObject(pRect.get());
    pRect->BroadcastObjectChange();

    auto aDelta = getVectorPrimitives(u"testMasterDelta", nVersion);
    assertJsonPath(aDelta, "/type", "vectorprimitivesdelta");
    // The slide object itself is unchanged, so the only entry is the page,
    // whose master page content changed.
    CPPUNIT_ASSERT_EQUAL(size_t(1), aDelta.getSize("/objects").value_or(SIZE_MAX));
    assertJsonPath(aDelta, "/objects/0/kind", "page");
}

CPPUNIT_TEST_FIXTURE(VectorRenderingTest, testDeltaCarriesChangedBackground)
{
    // The slide background is serialized with the master page content, so
    // a delta whose baseline predates a background change must carry that
    // content anew.

    createBlankDoc();
    addRectangle(tools::Rectangle(Point(1000, 1000), Size(3000, 2000)), Color(0x4472c4), COL_BLACK);
    page(1)->GetObj(0)->BroadcastObjectChange();

    auto aFull = getVectorPrimitives(u"testBackgroundDeltaFull");
    const sal_Int64 nVersion = aFull.getInt("/version").value_or(-1);

    // Change the slide background after that version.
    SdrPageProperties& rPageProperties = page(1)->getSdrPageProperties();
    rPageProperties.PutItem(XFillStyleItem(drawing::FillStyle_SOLID));
    rPageProperties.PutItem(XFillColorItem(OUString(), Color(0xc00000)));

    auto aDelta = getVectorPrimitives(u"testBackgroundDelta", nVersion);
    assertJsonPath(aDelta, "/type", "vectorprimitivesdelta");
    // The slide object itself is unchanged, so the only entry is the page,
    // with the new background.
    CPPUNIT_ASSERT_EQUAL(size_t(1), aDelta.getSize("/objects").value_or(SIZE_MAX));
    assertJsonPath(aDelta, "/objects/0/kind", "page");
}

CPPUNIT_TEST_FIXTURE(VectorRenderingTest, testPullSetsPushBaseline)
{
    // A full pull records the requesting view's content version as that
    // view's push baseline. The first push to the view then carries only
    // later changes, not the whole slide the view already pulled.

    createBlankDoc();
    addRectangle(tools::Rectangle(Point(1000, 1000), Size(3000, 2000)), Color(0x4472c4), COL_BLACK);
    addRectangle(tools::Rectangle(Point(6000, 1000), Size(3000, 2000)), Color(0xc00000), COL_BLACK);
    page(1)->GetObj(0)->BroadcastObjectChange();
    page(1)->GetObj(1)->BroadcastObjectChange();

    // The full pull gives the view the page and both objects and sets its
    // baseline.
    auto aFull = getVectorPrimitives(u"testPullBaseline");
    CPPUNIT_ASSERT_EQUAL(size_t(3), aFull.getSize("/objects").value_or(0));

    const SfxViewShell* pView = SfxViewShell::Current();
    CPPUNIT_ASSERT(pView);
    const sal_Int32 nViewId = static_cast<sal_Int32>(pView->GetViewShellId().get());

    SdXImpressDocument* pDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pDocument);
    const OString aCommand
        = ".uno:VectorPrimitives?part=0&pushdelta=1&viewid=" + OString::number(nViewId);
    tools::JsonWriter aJsonWriter;
    pDocument->getCommandValues(aJsonWriter,
                                std::string_view(aCommand.getStr(), aCommand.getLength()));
    const OString aResult = aJsonWriter.finishAndGetAsOString();
    auto oDelta = tools::JsonPath::parse(std::string_view(aResult.getStr(), aResult.getLength()));
    CPPUNIT_ASSERT(oDelta.has_value());

    // Nothing changed since the pull, so the push has nothing to carry and
    // the response is empty.
    CPPUNIT_ASSERT(!oDelta->has("/type"));
    CPPUNIT_ASSERT(!oDelta->has("/order"));
    CPPUNIT_ASSERT(!oDelta->has("/objects"));
}

CPPUNIT_TEST_FIXTURE(VectorRenderingTest, testEditedTextAppearsInPrimitives)
{
    // While a text object is being edited, what has been typed so far must
    // appear in the payload, on the entry that carries the running edit rather
    // than on the object, which hides its own text while the edit runs.
    createBlankDoc();
    addRectangle(tools::Rectangle(Point(1000, 1000), Size(6000, 3000)), Color(0x4472c4), COL_BLACK);

    // Enter text edit on the object and type a word.
    SdrView* pView = getSdDocShell()->GetViewShell()->GetView();
    CPPUNIT_ASSERT(pView);
    pView->SdrBeginTextEdit(page(1)->GetObj(0));
    EditView& rEditView = pView->GetTextEditOutlinerView()->GetEditView();
    rEditView.InsertText(u"Hello"_ustr);

    // Serialize while the edit is still active.
    SdXImpressDocument* pDoc = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pDoc);
    tools::JsonWriter aJsonWriter;
    pDoc->getCommandValues(aJsonWriter, ".uno:VectorPrimitives?part=0");
    const OString aResult = aJsonWriter.finishAndGetAsOString();

    pView->SdrEndTextEdit();

    CPPUNIT_ASSERT_MESSAGE(aResult.getStr(), aResult.indexOf("Hello") >= 0);

    auto oJson = tools::JsonPath::parse(std::string_view(aResult.getStr(), aResult.getLength()));
    CPPUNIT_ASSERT(oJson.has_value());

    // The running edit is the last entry, named by the object it runs on.
    const size_t nCount = oJson->getSize("/objects").value_or(0);
    assertJsonPath(*oJson,
                   rtl::Concat2View("/objects/" + OString::number(sal_Int32(nCount - 1))
                                    + "/kind"),
                   "texteditoverlay");

    const auto oEntry = findEntryOfKind(*oJson, "texteditoverlay");
    CPPUNIT_ASSERT(oEntry.has_value());
    CPPUNIT_ASSERT_EQUAL(sal_Int64(page(1)->GetObj(0)->GetUniqueID()),
                         oEntry->getInt("parent").value_or(-1));

    // The object it runs on no longer carries the text, so the only "Hello"
    // in the payload is the one on that entry.
    const auto oEdited = oJson->at("/objects/1");
    CPPUNIT_ASSERT(oEdited.has_value());
    CPPUNIT_ASSERT(!findTextPortionUnder(*oEdited, "Hello"_ostr).has_value());
}

CPPUNIT_TEST_FIXTURE(VectorRenderingTest, testDeltaIncludesEditedObject)
{
    // Text typed into an object appears in the delta before the edit ends,
    // so the view follows the typing rather than waiting for the edit to be
    // committed.
    createBlankDoc();
    addRectangle(tools::Rectangle(Point(1000, 1000), Size(6000, 3000)), Color(0x4472c4), COL_BLACK);
    SdrObject* pObject = page(1)->GetObj(0);

    // The first pull is what marks the model as drawn from, which is what
    // makes an open edit broadcast its changes.
    const sal_Int64 nVersion
        = getVectorPrimitives(u"testEditDeltaBase").getInt("/version").value_or(-1);

    SdrView* pView = getSdDocShell()->GetViewShell()->GetView();
    CPPUNIT_ASSERT(pView);
    pView->SdrBeginTextEdit(pObject);
    pView->GetTextEditOutlinerView()->GetEditView().InsertText(u"Hello"_ustr);

    auto aDelta = getVectorPrimitives(u"testEditDelta", nVersion);

    pView->SdrEndTextEdit();

    assertJsonPath(aDelta, "/type", "vectorprimitivesdelta");
    // The typed text rides on the entry for the running edit, which names the
    // object it runs on.
    const auto oEntry = findEntryOfKind(aDelta, "texteditoverlay");
    CPPUNIT_ASSERT(oEntry.has_value());
    CPPUNIT_ASSERT_EQUAL(sal_Int64(pObject->GetUniqueID()), oEntry->getInt("parent").value_or(-1));
}

CPPUNIT_TEST_FIXTURE(VectorRenderingTest, testDeltaIncludesGroupWithEditedObject)
{
    // An edit on an object inside a group changes that member the same way as
    // one on the page: it hides its own text while the edit runs, so the delta
    // carries the member.
    createBlankDoc();
    SdrObject* pInner
        = addGroupedRectangle(tools::Rectangle(Point(1000, 1000), Size(6000, 3000)), COL_BLUE);

    const sal_Int64 nVersion
        = getVectorPrimitives(u"testGroupEditDeltaBase").getInt("/version").value_or(-1);

    SdrView* pView = getSdDocShell()->GetViewShell()->GetView();
    CPPUNIT_ASSERT(pView);
    pView->SdrBeginTextEdit(pInner);
    pView->GetTextEditOutlinerView()->GetEditView().InsertText(u"Hello"_ustr);

    auto aDelta = getVectorPrimitives(u"testGroupEditDelta", nVersion);

    pView->SdrEndTextEdit();

    assertJsonPath(aDelta, "/type", "vectorprimitivesdelta");
    CPPUNIT_ASSERT(carriesObject(aDelta, pInner->GetUniqueID()));
}

CPPUNIT_TEST_FIXTURE(VectorRenderingTest, testGraphicsResponseKeepsTypeOnUnknownChecksum)
{
    // A graphics request for a checksum the document does not know still
    // gets a typed response naming the checksum, only without image data.
    createBlankDoc();

    SdXImpressDocument* pDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pDocument);

    tools::JsonWriter aJsonWriter;
    pDocument->getCommandValues(aJsonWriter, ".uno:VectorRenderingGraphics?checksum=12345");
    OString aResult = aJsonWriter.finishAndGetAsOString();

    auto oJson = tools::JsonPath::parse(std::string_view(aResult.getStr(), aResult.getLength()));
    CPPUNIT_ASSERT(oJson.has_value());
    assertJsonPath(*oJson, "/type", "vectorrenderinggraphics");
    CPPUNIT_ASSERT_EQUAL(sal_Int64(12345), oJson->getInt("/checksum").value_or(-1));
}

CPPUNIT_TEST_FIXTURE(VectorRenderingTest, testPullMarksTheModelAsDrawnFrom)
{
    // Asking for the primitives is what tells the model it is drawn from.
    createBlankDoc();
    CPPUNIT_ASSERT(!page(1)->getSdrModelFromSdrPage().IsDrawnFromModel());

    getVectorPrimitives(u"testDrawnFromModel");

    CPPUNIT_ASSERT(page(1)->getSdrModelFromSdrPage().IsDrawnFromModel());
}

CPPUNIT_TEST_FIXTURE(VectorRenderingTest, testTheLastReaderLeavingClearsTheMark)
{
    // The mark follows the reader, so letting the last one go clears it.
    createBlankDoc();
    getVectorPrimitives(u"testDrawnFromModelCleared");
    CPPUNIT_ASSERT(page(1)->getSdrModelFromSdrPage().IsDrawnFromModel());

    page(1)->getSdrModelFromSdrPage().SetDrawnFromModel(false);

    CPPUNIT_ASSERT(!page(1)->getSdrModelFromSdrPage().IsDrawnFromModel());
}

CPPUNIT_TEST_FIXTURE(VectorRenderingTest, testBoldRunNamesADifferentFace)
{
    // Bold and regular text of one family are different faces, so they name
    // different files. One id for both would draw the two runs alike.
    if (!familyHasBoldCut())
        return;

    createBlankDoc();
    addRectangle(tools::Rectangle(Point(1000, 1000), Size(6000, 3000)), Color(0x4472c4), COL_BLACK);
    addRectangle(tools::Rectangle(Point(1000, 5000), Size(6000, 3000)), Color(0x4472c4), COL_BLACK);
    page(1)->GetObj(1)->SetMergedItem(SvxWeightItem(WEIGHT_BOLD, EE_CHAR_WEIGHT));

    SdrView* pView = getSdDocShell()->GetViewShell()->GetView();
    CPPUNIT_ASSERT(pView);
    for (size_t nObject = 0; nObject < 2; ++nObject)
    {
        pView->SdrBeginTextEdit(page(1)->GetObj(nObject));
        pView->GetTextEditOutlinerView()->GetEditView().InsertText(u"Hello"_ustr);
        pView->SdrEndTextEdit();
    }

    auto aJson = getVectorPrimitives(u"testBoldRunFace");
    const std::optional<tools::JsonPath> oPlainObject = aJson.at("/objects/1");
    const std::optional<tools::JsonPath> oBoldObject = aJson.at("/objects/2");
    CPPUNIT_ASSERT(oPlainObject.has_value());
    CPPUNIT_ASSERT(oBoldObject.has_value());
    const std::optional<tools::JsonPath> oPlain = oPlainObject->findFirst("fontId");
    const std::optional<tools::JsonPath> oBold = oBoldObject->findFirst("fontId");
    CPPUNIT_ASSERT(oPlain.has_value());
    CPPUNIT_ASSERT(oBold.has_value());
    CPPUNIT_ASSERT_MESSAGE("the bold run named the same face as the plain run",
                           oPlain->getString() != oBold->getString());

    // Which run is which, so the ids above are known to differ for the
    // reason the test is about.
    const std::optional<tools::JsonPath> oPlainWeight = oPlainObject->findFirst("weight");
    const std::optional<tools::JsonPath> oBoldWeight = oBoldObject->findFirst("weight");
    CPPUNIT_ASSERT(oPlainWeight.has_value());
    CPPUNIT_ASSERT(oBoldWeight.has_value());
    CPPUNIT_ASSERT_EQUAL(sal_Int64(WEIGHT_NORMAL), oPlainWeight->getInt().value_or(-1));
    CPPUNIT_ASSERT_EQUAL(sal_Int64(WEIGHT_BOLD), oBoldWeight->getInt().value_or(-1));
}

CPPUNIT_TEST_FIXTURE(VectorRenderingTest, testRealBoldFaceNeedsNoThickening)
{
    // Bold text resolves to the family's own bold cut. Saying the weight has
    // to be made up would thicken a face that is already bold.
    if (!familyHasBoldCut())
        return;

    createBlankDoc();
    addRectangle(tools::Rectangle(Point(1000, 1000), Size(6000, 3000)), Color(0x4472c4), COL_BLACK);

    SdrObject* pObject = page(1)->GetObj(0);
    pObject->SetMergedItem(SvxWeightItem(WEIGHT_BOLD, EE_CHAR_WEIGHT));

    SdrView* pView = getSdDocShell()->GetViewShell()->GetView();
    CPPUNIT_ASSERT(pView);
    pView->SdrBeginTextEdit(pObject);
    pView->GetTextEditOutlinerView()->GetEditView().InsertText(u"Hello"_ustr);

    SdXImpressDocument* pDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pDocument);
    tools::JsonWriter aJsonWriter;
    pDocument->getCommandValues(aJsonWriter, ".uno:VectorPrimitives?part=0");
    const OString aResult = aJsonWriter.finishAndGetAsOString();

    pView->SdrEndTextEdit();

    auto oJson = tools::JsonPath::parse(std::string_view(aResult.getStr(), aResult.getLength()));
    CPPUNIT_ASSERT(oJson.has_value());
    CPPUNIT_ASSERT_MESSAGE(aResult.getStr(), oJson->findFirst("fontId").has_value());
    CPPUNIT_ASSERT_MESSAGE("a real bold face was marked as needing thickening",
                           !oJson->findFirst("syntheticBold").has_value());
}

CPPUNIT_TEST_FIXTURE(VectorRenderingTest, testTextPortionCarriesFont)
{
    // A text portion names its face by id and the font command returns
    // that face's file, so a client loads the exact font instead of
    // guessing from the family name.

    // The id is read from a realized face, so the test needs at least
    // one usable font. Pass trivially when the environment has none.
    ScopedVclPtrInstance<VirtualDevice> pProbeDevice;
    pProbeDevice->SetFont(vcl::Font(u"Liberation Sans"_ustr, Size(0, 2000)));
    if (pProbeDevice->GetCurrentFontRawData().isEmpty())
        return;

    createBlankDoc();
    addRectangle(tools::Rectangle(Point(1000, 1000), Size(6000, 3000)), Color(0x4472c4), COL_BLACK);

    SdrView* pView = getSdDocShell()->GetViewShell()->GetView();
    CPPUNIT_ASSERT(pView);
    pView->SdrBeginTextEdit(page(1)->GetObj(0));
    pView->GetTextEditOutlinerView()->GetEditView().InsertText(u"Hello"_ustr);

    SdXImpressDocument* pDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pDocument);
    tools::JsonWriter aJsonWriter;
    pDocument->getCommandValues(aJsonWriter, ".uno:VectorPrimitives?part=0");
    const OString aResult = aJsonWriter.finishAndGetAsOString();

    pView->SdrEndTextEdit();

    // The text portion must carry a font id.
    auto oJson = tools::JsonPath::parse(std::string_view(aResult.getStr(), aResult.getLength()));
    CPPUNIT_ASSERT(oJson.has_value());
    const std::optional<tools::JsonPath> oFontId = oJson->findFirst("fontId");
    CPPUNIT_ASSERT_MESSAGE(aResult.getStr(), oFontId.has_value());
    const OString aFontId = oFontId->getString().value_or(""_ostr);
    CPPUNIT_ASSERT(aFontId.toUInt64(16) != 0);

    // The font command returns that face's bytes as base64.
    const OString aFontCommand = ".uno:VectorRenderingFont?id=" + aFontId;
    tools::JsonWriter aFontWriter;
    pDocument->getCommandValues(aFontWriter,
                           std::string_view(aFontCommand.getStr(), aFontCommand.getLength()));
    const OString aFontResult = aFontWriter.finishAndGetAsOString();
    auto oFont
        = tools::JsonPath::parse(std::string_view(aFontResult.getStr(), aFontResult.getLength()));
    CPPUNIT_ASSERT(oFont.has_value());
    assertJsonPath(*oFont, "/type", "vectorrenderingfont");
    // A real font file base64-encodes to far more than this. The bound only
    // guards against an empty or missing payload.
    CPPUNIT_ASSERT(oFont->getString("/data").value_or(OString()).getLength() > 1000);
}

CPPUNIT_TEST_FIXTURE(VectorRenderingTest, testTextParagraphIndex)
{
    // A text box with two paragraphs. Every text portion carries the
    // index of the paragraph it belongs to.
    createBlankDoc();
    addTextBox(tools::Rectangle(Point(2000, 2000), Size(8000, 4000)),
               u"First paragraph\nSecond paragraph"_ustr);

    getVectorPrimitives(u"testTextParagraphIndex");

    const auto oFirst = findTextPortion("First paragraph"_ostr);
    CPPUNIT_ASSERT_MESSAGE("first paragraph portion missing", oFirst.has_value());
    CPPUNIT_ASSERT_EQUAL(sal_Int64(0), oFirst->getInt("paragraph").value_or(-1));
    const auto oSecond = findTextPortion("Second paragraph"_ostr);
    CPPUNIT_ASSERT_MESSAGE("second paragraph portion missing", oSecond.has_value());
    CPPUNIT_ASSERT_EQUAL(sal_Int64(1), oSecond->getInt("paragraph").value_or(-1));
}

CPPUNIT_TEST_FIXTURE(VectorRenderingTest, testTextParagraphAlign)
{
    // A centered paragraph reports its alignment on the text portions.
    createBlankDoc();
    addTextBox(tools::Rectangle(Point(2000, 2000), Size(8000, 4000)), u"Centered text"_ustr,
               SvxAdjust::Center);

    getVectorPrimitives(u"testTextParagraphAlign");

    const auto oPortion = findTextPortion("Centered text"_ostr);
    CPPUNIT_ASSERT_MESSAGE("centered portion missing", oPortion.has_value());
    CPPUNIT_ASSERT_EQUAL("center"_ostr, oPortion->getString("align").value_or(OString()));
}

CPPUNIT_TEST_FIXTURE(VectorRenderingTest, testTextAdvancesAreRelative)
{
    // Each advance is the step from the glyph before it, not the distance
    // from the start of the run, so the numbers stay small however long the
    // run is.
    createBlankDoc();
    addTextBox(tools::Rectangle(Point(2000, 2000), Size(8000, 4000)), u"Advances"_ustr);

    getVectorPrimitives(u"testTextAdvances");

    const auto oPortion = findTextPortion("Advances"_ostr);
    CPPUNIT_ASSERT_MESSAGE("text portion missing", oPortion.has_value());

    const size_t nCount = oPortion->getSize("advances").value_or(0);
    CPPUNIT_ASSERT_EQUAL(size_t(8), nCount);

    sal_Int64 nTotal = 0;
    sal_Int64 nLast = 0;
    for (size_t nIndex = 0; nIndex < nCount; ++nIndex)
    {
        nLast = oPortion
                    ->getInt(rtl::Concat2View("advances/" + OString::number(sal_Int32(nIndex))))
                    .value_or(0);
        CPPUNIT_ASSERT_MESSAGE("an advance is not a forward step", nLast > 0);
        nTotal += nLast;
    }

    // Measured from the start of the run the last value would be the whole
    // width. As a step it is one glyph's worth of it.
    CPPUNIT_ASSERT_MESSAGE("the advances read as distances from the start",
                           nLast < nTotal / 2);
}

CPPUNIT_TEST_FIXTURE(VectorRenderingTest, testTextLineMetrics)
{
    // Laid-out text reports the height and ascent of its line on the
    // portions.
    createBlankDoc();
    addTextBox(tools::Rectangle(Point(2000, 2000), Size(8000, 4000)), u"Line metrics"_ustr);

    getVectorPrimitives(u"testTextLineMetrics");

    const auto oPortion = findTextPortion("Line metrics"_ostr);
    CPPUNIT_ASSERT_MESSAGE("text portion missing", oPortion.has_value());
    const double fHeight = oPortion->getDouble("lineHeight").value_or(0.0);
    const double fAscent = oPortion->getDouble("lineAscent").value_or(0.0);
    CPPUNIT_ASSERT_MESSAGE("line height missing", fHeight > 0.0);
    CPPUNIT_ASSERT_MESSAGE("line ascent missing", fAscent > 0.0);
    // The ascent is the baseline's offset within the line, so it fits
    // inside the height.
    CPPUNIT_ASSERT(fAscent <= fHeight);
}

CPPUNIT_TEST_FIXTURE(VectorRenderingTest, testTextAreaSize)
{
    // Laid-out text reports the size of the area it wraps within.
    createBlankDoc();
    addTextBox(tools::Rectangle(Point(2000, 2000), Size(8000, 4000)), u"Box size"_ustr);

    getVectorPrimitives(u"testTextAreaSize");

    const auto oPortion = findTextPortion("Box size"_ostr);
    CPPUNIT_ASSERT_MESSAGE("text portion missing", oPortion.has_value());
    CPPUNIT_ASSERT_MESSAGE("text area width missing",
                           oPortion->getDouble("textAreaWidth").value_or(0.0) > 0.0);
    CPPUNIT_ASSERT_MESSAGE("text area height missing",
                           oPortion->getDouble("textAreaHeight").value_or(0.0) > 0.0);
}

CPPUNIT_TEST_FIXTURE(VectorRenderingTest, testStrokedRectangle)
{
    // A stroke-only rectangle decomposes to a polygonStroke primitive
    // under the slide object.
    createBlankDoc();
    addStrokedRectangle(tools::Rectangle(Point(5000, 5000), Size(5000, 3000)), COL_BLACK);

    auto aJson = getVectorPrimitives(u"testStrokedRectangle");

    assertJsonPath(aJson, "/type", "vectorprimitives");
    CPPUNIT_ASSERT_EQUAL(size_t(2), aJson.getSize("/objects").value_or(0));

    auto oStroke = aJson.at("/objects/1/primitives/0/children/0/children/0");
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

    auto aJson = getVectorPrimitives(u"testRotatedRectangle");

    assertJsonPath(aJson, "/type", "vectorprimitives");
    CPPUNIT_ASSERT_EQUAL(size_t(2), aJson.getSize("/objects").value_or(0));
}

CPPUNIT_TEST_FIXTURE(VectorRenderingTest, testObjectInfo)
{
    // A rectangle with a name, title and description wraps its
    // decomposition in an ObjectInfoPrimitive2D.
    createBlankDoc();
    addRectangleWithObjectInfo(tools::Rectangle(Point(5000, 5000), Size(5000, 3000)),
                               Color(0x4472c4), u"Rectangle 1"_ustr, u"My title"_ustr,
                               u"My description"_ustr);

    auto aJson = getVectorPrimitives(u"testObjectInfo");

    assertJsonPath(aJson, "/type", "vectorprimitives");
    CPPUNIT_ASSERT_EQUAL(size_t(2), aJson.getSize("/objects").value_or(0));

    // The object info wraps the SdrObject's primitive sequence, so it
    // is the outermost node. The wrapping path is objectInfo -> svx:N
    // -> group -> [fill].
    auto oObjectInfo = aJson.at("/objects/1/primitives/0");
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

    auto aJson = getVectorPrimitives(u"testPolyPolygonRGBA");

    assertJsonPath(aJson, "/type", "vectorprimitives");
    CPPUNIT_ASSERT_EQUAL(size_t(2), aJson.getSize("/objects").value_or(0));

    // The wrapping path is svx:N -> group -> polyPolygonRGBA.
    auto oRGBA = aJson.at("/objects/1/primitives/0/children/0/children/0");
    CPPUNIT_ASSERT(oRGBA.has_value());
    assertJsonPath(*oRGBA, "type", "polyPolygonRGBA");
    assertJsonPath(*oRGBA, "color", "#4472c4");
    assertJsonPathExists(*oRGBA, "transparency");
}

CPPUNIT_TEST_FIXTURE(VectorRenderingTest, testPolygonHairline)
{
    // An SdrPageObj's view-independent decomposition is a single
    // yellow polygonHairline outline. The vector primitives pipeline goes
    // through that exact path, so the reference JSON for the browser
    // mocha test gets a real polygonHairline emitted by the engine.
    createBlankDoc();
    addPageObject(tools::Rectangle(Point(5000, 5000), Size(5000, 3000)));

    auto aJson = getVectorPrimitives(u"testPolygonHairline");

    assertJsonPath(aJson, "/type", "vectorprimitives");
    CPPUNIT_ASSERT_EQUAL(size_t(2), aJson.getSize("/objects").value_or(0));

    auto oHairline = aJson.at("/objects/1/primitives/0");
    CPPUNIT_ASSERT(oHairline.has_value());
    assertJsonPath(*oHairline, "type", "polygonHairline");
    assertJsonPathExists(*oHairline, "color");
    assertJsonPathExists(*oHairline, "path");
}

} // namespace

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
