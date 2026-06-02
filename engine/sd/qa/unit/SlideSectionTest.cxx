/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "sdmodeltestbase.hxx"

#include <unomodel.hxx>
#include <drawdoc.hxx>
#include <SlideSectionManager.hxx>
#include <UndoSlideSection.hxx>
#include <DrawDocShell.hxx>
#include <sfx2/objsh.hxx>
#include <svl/undo.hxx>
#include <sdpage.hxx>

using namespace css;

// Size + membership implies set equality, and avoids needing an ostream
// overload for std::set<SdPage*> purely to satisfy CPPUNIT_ASSERT_EQUAL.
static void assertSameSlides(const std::set<SdPage*>& rExpected, const std::set<SdPage*>& rActual)
{
    CPPUNIT_ASSERT_EQUAL(rExpected.size(), rActual.size());
    for (SdPage* p : rExpected)
        CPPUNIT_ASSERT(rActual.contains(p));
}

class SlideSectionTest : public SdModelTestBase
{
public:
    SlideSectionTest()
        : SdModelTestBase(u"/sd/qa/unit/data/"_ustr)
    {
    }

    SdDrawDocument* getDoc()
    {
        auto* pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
        CPPUNIT_ASSERT(pXImpressDocument);
        SdDrawDocument* pDoc = pXImpressDocument->GetDoc();
        CPPUNIT_ASSERT(pDoc);
        return pDoc;
    }

    // Go through the docshell for Undo/Redo: sd::UndoManager deliberately hides
    // its Undo/Redo overloads; SfxObjectShell::GetUndoManager() returns an
    // SfxUndoManager* on which they are public.
    SfxUndoManager& getUndoManager()
    {
        auto* pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
        CPPUNIT_ASSERT(pXImpressDocument);
        SfxUndoManager* pMgr = pXImpressDocument->GetDocShell()->GetUndoManager();
        CPPUNIT_ASSERT(pMgr);
        return *pMgr;
    }

    sd::SlideSectionManager& getSectionManager() { return getDoc()->GetSectionManager(); }

    // Collect pointers to the slides currently belonging to a section
    // (i.e. the pages between this section's start and the next section's start).
    // Pointers are stable across MoveSection — MovePages() reorders the page
    // vector in place — so they are a reliable identity check for "the same
    // slides are still in this section."
    //
    // Returned as std::set because MovePages() iterates selected pages one by
    // one and does not guarantee their relative order is preserved across the
    // move. What we care about is membership: the same *set* of pages must be
    // under the section after the move.
    std::set<SdPage*> getSectionSlides(sal_Int32 nSectionIndex)
    {
        SdDrawDocument* pDoc = getDoc();
        sd::SlideSectionManager& rMgr = pDoc->GetSectionManager();
        const sal_Int32 nStart = rMgr.GetSection(nSectionIndex).mnStartIndex;
        const sal_Int32 nEnd
            = (nSectionIndex + 1 < rMgr.GetSectionCount())
                  ? rMgr.GetSection(nSectionIndex + 1).mnStartIndex
                  : static_cast<sal_Int32>(pDoc->GetSdPageCount(PageKind::Standard));
        std::set<SdPage*> aPages;
        for (sal_Int32 i = nStart; i < nEnd; ++i)
            aPages.insert(pDoc->GetSdPage(static_cast<sal_uInt16>(i), PageKind::Standard));
        return aPages;
    }
};

// Verify adding a section mid-deck splits the slide distribution correctly in PPTX
CPPUNIT_TEST_FIXTURE(SlideSectionTest, testAddSectionPPTX)
{
    createSdImpressDoc("pptx/slide-section-test.pptx");

    sd::SlideSectionManager& rMgr = getSectionManager();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), rMgr.GetSectionCount());

    rMgr.AddSection(5, u"New Section"_ustr);

    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), rMgr.GetSectionCount());
    CPPUNIT_ASSERT_EQUAL(u"Section-1"_ustr, rMgr.GetSection(0).maName);
    CPPUNIT_ASSERT_EQUAL(u"Section-2"_ustr, rMgr.GetSection(1).maName);
    CPPUNIT_ASSERT_EQUAL(u"New Section"_ustr, rMgr.GetSection(2).maName);
    CPPUNIT_ASSERT_EQUAL(u"Section-3"_ustr, rMgr.GetSection(3).maName);

    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), rMgr.GetSection(0).mnStartIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), rMgr.GetSection(1).mnStartIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), rMgr.GetSection(2).mnStartIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(11), rMgr.GetSection(3).mnStartIndex);

    save(TestFilter::PPTX);
    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/presentation.xml"_ustr);

    static constexpr OString sPath = "/p:presentation/p:extLst/p:ext/p14:sectionLst"_ostr;
    assertXPath(pXmlDoc, sPath + "/p14:section", 4);
    assertXPath(pXmlDoc, sPath + "/p14:section[1]", "name", u"Section-1");
    assertXPath(pXmlDoc, sPath + "/p14:section[2]", "name", u"Section-2");
    assertXPath(pXmlDoc, sPath + "/p14:section[3]", "name", u"New Section");
    assertXPath(pXmlDoc, sPath + "/p14:section[4]", "name", u"Section-3");
    assertXPath(pXmlDoc, sPath + "/p14:section[1]/p14:sldIdLst/p14:sldId", 4);
    assertXPath(pXmlDoc, sPath + "/p14:section[2]/p14:sldIdLst/p14:sldId", 1);
    assertXPath(pXmlDoc, sPath + "/p14:section[3]/p14:sldIdLst/p14:sldId", 6);
    assertXPath(pXmlDoc, sPath + "/p14:section[4]/p14:sldIdLst/p14:sldId", 2);
}

// Verify adding a section mid-deck splits the slide distribution correctly in ODP
CPPUNIT_TEST_FIXTURE(SlideSectionTest, testAddSectionODP)
{
    createSdImpressDoc("pptx/slide-section-test.pptx");
    skipValidation();
    saveAndReload(TestFilter::ODP);

    sd::SlideSectionManager& rMgr = getSectionManager();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), rMgr.GetSectionCount());

    rMgr.AddSection(5, u"New Section"_ustr);

    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), rMgr.GetSectionCount());
    CPPUNIT_ASSERT_EQUAL(u"New Section"_ustr, rMgr.GetSection(2).maName);

    save(TestFilter::ODP);
    xmlDocUniquePtr pXmlDoc = parseExport(u"content.xml"_ustr);

    static constexpr OString sPath
        = "/office:document-content/office:body/office:presentation/loext:section-list"_ostr;
    assertXPath(pXmlDoc, sPath + "/loext:section", 4);
    assertXPath(pXmlDoc, sPath + "/loext:section[1]", "name", u"Section-1");
    assertXPath(pXmlDoc, sPath + "/loext:section[2]", "name", u"Section-2");
    assertXPath(pXmlDoc, sPath + "/loext:section[3]", "name", u"New Section");
    assertXPath(pXmlDoc, sPath + "/loext:section[4]", "name", u"Section-3");
    assertXPath(pXmlDoc, sPath + "/loext:section[1]/loext:section-slide", 4);
    assertXPath(pXmlDoc, sPath + "/loext:section[2]/loext:section-slide", 1);
    assertXPath(pXmlDoc, sPath + "/loext:section[3]/loext:section-slide", 6);
    assertXPath(pXmlDoc, sPath + "/loext:section[4]/loext:section-slide", 2);
}

// Verify renaming a section persists through PPTX export without affecting slide counts
CPPUNIT_TEST_FIXTURE(SlideSectionTest, testRenameSectionPPTX)
{
    createSdImpressDoc("pptx/slide-section-test.pptx");

    sd::SlideSectionManager& rMgr = getSectionManager();
    rMgr.RenameSection(1, u"Renamed-Section"_ustr);

    save(TestFilter::PPTX);
    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/presentation.xml"_ustr);

    static constexpr OString sPath = "/p:presentation/p:extLst/p:ext/p14:sectionLst"_ostr;
    assertXPath(pXmlDoc, sPath + "/p14:section[1]", "name", u"Section-1");
    assertXPath(pXmlDoc, sPath + "/p14:section[2]", "name", u"Renamed-Section");
    assertXPath(pXmlDoc, sPath + "/p14:section[3]", "name", u"Section-3");

    assertXPath(pXmlDoc, sPath + "/p14:section[1]/p14:sldIdLst/p14:sldId", 4);
    assertXPath(pXmlDoc, sPath + "/p14:section[2]/p14:sldIdLst/p14:sldId", 7);
    assertXPath(pXmlDoc, sPath + "/p14:section[3]/p14:sldIdLst/p14:sldId", 2);
}

// Verify renaming a section persists through ODP export
CPPUNIT_TEST_FIXTURE(SlideSectionTest, testRenameSectionODP)
{
    createSdImpressDoc("pptx/slide-section-test.pptx");
    skipValidation();
    saveAndReload(TestFilter::ODP);

    sd::SlideSectionManager& rMgr = getSectionManager();
    rMgr.RenameSection(0, u"ODP-Renamed"_ustr);

    save(TestFilter::ODP);
    xmlDocUniquePtr pXmlDoc = parseExport(u"content.xml"_ustr);

    static constexpr OString sPath
        = "/office:document-content/office:body/office:presentation/loext:section-list"_ostr;
    assertXPath(pXmlDoc, sPath + "/loext:section[1]", "name", u"ODP-Renamed");
    assertXPath(pXmlDoc, sPath + "/loext:section[2]", "name", u"Section-2");
    assertXPath(pXmlDoc, sPath + "/loext:section[3]", "name", u"Section-3");
}

// Verify moving a section up reorders names and slide counts, and that the
// individual slides travel with their section (the exact set of pages in
// Section-2 before the move is exactly the set in Section-2 after the move).
CPPUNIT_TEST_FIXTURE(SlideSectionTest, testMoveSectionUpPPTX)
{
    createSdImpressDoc("pptx/slide-section-test.pptx");

    // Capture which slides each section owns before the move.
    const auto aSection1Before = getSectionSlides(0);
    const auto aSection2Before = getSectionSlides(1);
    const auto aSection3Before = getSectionSlides(2);

    sd::SlideSectionManager& rMgr = getSectionManager();
    rMgr.MoveSection(1, 0);

    // Section-2 is now at index 0 but must contain the same slides as before.
    assertSameSlides(aSection2Before, getSectionSlides(0));
    assertSameSlides(aSection1Before, getSectionSlides(1));
    assertSameSlides(aSection3Before, getSectionSlides(2));

    save(TestFilter::PPTX);
    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/presentation.xml"_ustr);

    static constexpr OString sPath = "/p:presentation/p:extLst/p:ext/p14:sectionLst"_ostr;
    assertXPath(pXmlDoc, sPath + "/p14:section[1]", "name", u"Section-2");
    assertXPath(pXmlDoc, sPath + "/p14:section[2]", "name", u"Section-1");
    assertXPath(pXmlDoc, sPath + "/p14:section[3]", "name", u"Section-3");

    assertXPath(pXmlDoc, sPath + "/p14:section[1]/p14:sldIdLst/p14:sldId", 7);
    assertXPath(pXmlDoc, sPath + "/p14:section[2]/p14:sldIdLst/p14:sldId", 4);
    assertXPath(pXmlDoc, sPath + "/p14:section[3]/p14:sldIdLst/p14:sldId", 2);
}

// Verify moving a section up reorders names and slide counts, and that the
// individual slides travel with their section in ODP.
CPPUNIT_TEST_FIXTURE(SlideSectionTest, testMoveSectionUpODP)
{
    createSdImpressDoc("pptx/slide-section-test.pptx");
    skipValidation();
    saveAndReload(TestFilter::ODP);

    const auto aSection1Before = getSectionSlides(0);
    const auto aSection2Before = getSectionSlides(1);
    const auto aSection3Before = getSectionSlides(2);

    sd::SlideSectionManager& rMgr = getSectionManager();
    rMgr.MoveSection(2, 1);

    // Section-3 is now at index 1 but must contain the same slides.
    assertSameSlides(aSection1Before, getSectionSlides(0));
    assertSameSlides(aSection3Before, getSectionSlides(1));
    assertSameSlides(aSection2Before, getSectionSlides(2));

    save(TestFilter::ODP);
    xmlDocUniquePtr pXmlDoc = parseExport(u"content.xml"_ustr);

    static constexpr OString sPath
        = "/office:document-content/office:body/office:presentation/loext:section-list"_ostr;
    assertXPath(pXmlDoc, sPath + "/loext:section[1]", "name", u"Section-1");
    assertXPath(pXmlDoc, sPath + "/loext:section[2]", "name", u"Section-3");
    assertXPath(pXmlDoc, sPath + "/loext:section[3]", "name", u"Section-2");

    assertXPath(pXmlDoc, sPath + "/loext:section[1]/loext:section-slide", 4);
    assertXPath(pXmlDoc, sPath + "/loext:section[2]/loext:section-slide", 2);
    assertXPath(pXmlDoc, sPath + "/loext:section[3]/loext:section-slide", 7);
}

// Verify moving a section down reorders names and slide counts, and that
// the individual slides travel with their section in PPTX.
CPPUNIT_TEST_FIXTURE(SlideSectionTest, testMoveSectionDownPPTX)
{
    createSdImpressDoc("pptx/slide-section-test.pptx");

    const auto aSection1Before = getSectionSlides(0);
    const auto aSection2Before = getSectionSlides(1);
    const auto aSection3Before = getSectionSlides(2);

    sd::SlideSectionManager& rMgr = getSectionManager();
    rMgr.MoveSection(0, 1);

    // Section-1 is now at index 1 with the same slides; Section-2 at index 0.
    assertSameSlides(aSection2Before, getSectionSlides(0));
    assertSameSlides(aSection1Before, getSectionSlides(1));
    assertSameSlides(aSection3Before, getSectionSlides(2));

    save(TestFilter::PPTX);
    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/presentation.xml"_ustr);

    static constexpr OString sPath = "/p:presentation/p:extLst/p:ext/p14:sectionLst"_ostr;
    assertXPath(pXmlDoc, sPath + "/p14:section[1]", "name", u"Section-2");
    assertXPath(pXmlDoc, sPath + "/p14:section[2]", "name", u"Section-1");
    assertXPath(pXmlDoc, sPath + "/p14:section[3]", "name", u"Section-3");

    assertXPath(pXmlDoc, sPath + "/p14:section[1]/p14:sldIdLst/p14:sldId", 7);
    assertXPath(pXmlDoc, sPath + "/p14:section[2]/p14:sldIdLst/p14:sldId", 4);
    assertXPath(pXmlDoc, sPath + "/p14:section[3]/p14:sldIdLst/p14:sldId", 2);
}

// Verify moving a section down reorders names and slide counts, and that
// the individual slides travel with their section in ODP.
CPPUNIT_TEST_FIXTURE(SlideSectionTest, testMoveSectionDownODP)
{
    createSdImpressDoc("pptx/slide-section-test.pptx");
    skipValidation();
    saveAndReload(TestFilter::ODP);

    const auto aSection1Before = getSectionSlides(0);
    const auto aSection2Before = getSectionSlides(1);
    const auto aSection3Before = getSectionSlides(2);

    sd::SlideSectionManager& rMgr = getSectionManager();
    rMgr.MoveSection(0, 1);

    assertSameSlides(aSection2Before, getSectionSlides(0));
    assertSameSlides(aSection1Before, getSectionSlides(1));
    assertSameSlides(aSection3Before, getSectionSlides(2));

    save(TestFilter::ODP);
    xmlDocUniquePtr pXmlDoc = parseExport(u"content.xml"_ustr);

    static constexpr OString sPath
        = "/office:document-content/office:body/office:presentation/loext:section-list"_ostr;
    assertXPath(pXmlDoc, sPath + "/loext:section[1]", "name", u"Section-2");
    assertXPath(pXmlDoc, sPath + "/loext:section[2]", "name", u"Section-1");
    assertXPath(pXmlDoc, sPath + "/loext:section[3]", "name", u"Section-3");

    assertXPath(pXmlDoc, sPath + "/loext:section[1]/loext:section-slide", 7);
    assertXPath(pXmlDoc, sPath + "/loext:section[2]/loext:section-slide", 4);
    assertXPath(pXmlDoc, sPath + "/loext:section[3]/loext:section-slide", 2);
}

// Moving a section must respect slides that belong to no section. Slides
// before the first section ("leading" slides) stay at the top of the deck and
// the moved sections keep their own slides. Regression: MoveSection used to
// recompute section start indices from 0, ignoring the leading slides, so the
// section headers landed on the wrong slides and a slide was orphaned at the
// end.
CPPUNIT_TEST_FIXTURE(SlideSectionTest, testMoveSectionWithLeadingNonSectionSlides)
{
    createSdImpressDoc("pptx/slide-section-test.pptx");

    SdDrawDocument* pDoc = getDoc();
    sd::SlideSectionManager& rMgr = getSectionManager();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), rMgr.GetSectionCount());

    // Capture the pages that will become leading (section-less) slides. The
    // fixture's Section-1 spans indices 0..3; dropping it un-owns those four.
    std::vector<SdPage*> aLeadingPages;
    for (sal_uInt16 i = 0; i < 4; ++i)
        aLeadingPages.push_back(pDoc->GetSdPage(i, PageKind::Standard));

    // Drop Section-1's metadata. Slides 0..3 are now leading orphans; the
    // remaining sections are Section-2 (start 4) and Section-3 (start 11).
    rMgr.RemoveSection(0);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), rMgr.GetSectionCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), rMgr.GetSection(0).mnStartIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(11), rMgr.GetSection(1).mnStartIndex);

    const auto aSection2Before = getSectionSlides(0);
    const auto aSection3Before = getSectionSlides(1);

    // Move Section-3 above Section-2.
    rMgr.MoveSection(1, 0);

    // Section-3 now sits at index 0 but must start at 4 (right after the four
    // leading slides), and Section-2 at index 1 must start at 6. The unfixed
    // code produced 0 and 2 here.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), rMgr.GetSection(0).mnStartIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(6), rMgr.GetSection(1).mnStartIndex);

    // The same slides travel with each section.
    assertSameSlides(aSection3Before, getSectionSlides(0));
    assertSameSlides(aSection2Before, getSectionSlides(1));

    // The leading slides did not move: indices 0..3 are still the same pages.
    for (sal_uInt16 i = 0; i < 4; ++i)
        CPPUNIT_ASSERT_EQUAL(aLeadingPages[i], pDoc->GetSdPage(i, PageKind::Standard));
}

// Undo/redo of AddSection restores the original section layout.
CPPUNIT_TEST_FIXTURE(SlideSectionTest, testUndoRedoAddSection)
{
    createSdImpressDoc("pptx/slide-section-test.pptx");

    SdDrawDocument* pDoc = getDoc();
    CPPUNIT_ASSERT(pDoc->IsUndoEnabled());
    sd::SlideSectionManager& rMgr = pDoc->GetSectionManager();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), rMgr.GetSectionCount());

    auto pUndo = std::make_unique<sd::UndoSlideSection>(*pDoc, u"Add section"_ustr);
    rMgr.AddSection(5, u"New Section"_ustr);
    pDoc->AddUndo(std::move(pUndo));

    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), rMgr.GetSectionCount());
    CPPUNIT_ASSERT_EQUAL(u"New Section"_ustr, rMgr.GetSection(2).maName);

    getUndoManager().Undo();

    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), rMgr.GetSectionCount());
    CPPUNIT_ASSERT_EQUAL(u"Section-1"_ustr, rMgr.GetSection(0).maName);
    CPPUNIT_ASSERT_EQUAL(u"Section-2"_ustr, rMgr.GetSection(1).maName);
    CPPUNIT_ASSERT_EQUAL(u"Section-3"_ustr, rMgr.GetSection(2).maName);

    getUndoManager().Redo();

    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), rMgr.GetSectionCount());
    CPPUNIT_ASSERT_EQUAL(u"New Section"_ustr, rMgr.GetSection(2).maName);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), rMgr.GetSection(2).mnStartIndex);
}

// Undo/redo of RemoveSection brings a removed section back with its original name and start.
CPPUNIT_TEST_FIXTURE(SlideSectionTest, testUndoRedoRemoveSection)
{
    createSdImpressDoc("pptx/slide-section-test.pptx");

    SdDrawDocument* pDoc = getDoc();
    sd::SlideSectionManager& rMgr = pDoc->GetSectionManager();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), rMgr.GetSectionCount());
    const OUString aRemovedName = rMgr.GetSection(1).maName;
    const sal_Int32 nRemovedStart = rMgr.GetSection(1).mnStartIndex;

    auto pUndo = std::make_unique<sd::UndoSlideSection>(*pDoc, u"Remove section"_ustr);
    rMgr.RemoveSection(1);
    pDoc->AddUndo(std::move(pUndo));

    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), rMgr.GetSectionCount());

    getUndoManager().Undo();

    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), rMgr.GetSectionCount());
    CPPUNIT_ASSERT_EQUAL(aRemovedName, rMgr.GetSection(1).maName);
    CPPUNIT_ASSERT_EQUAL(nRemovedStart, rMgr.GetSection(1).mnStartIndex);

    getUndoManager().Redo();

    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), rMgr.GetSectionCount());
}

// Undo/redo of RenameSection round-trips the name.
CPPUNIT_TEST_FIXTURE(SlideSectionTest, testUndoRedoRenameSection)
{
    createSdImpressDoc("pptx/slide-section-test.pptx");

    SdDrawDocument* pDoc = getDoc();
    sd::SlideSectionManager& rMgr = pDoc->GetSectionManager();
    const OUString aOldName = rMgr.GetSection(1).maName;

    auto pUndo = std::make_unique<sd::UndoSlideSection>(*pDoc, u"Rename section"_ustr);
    rMgr.RenameSection(1, u"Renamed"_ustr);
    pDoc->AddUndo(std::move(pUndo));

    CPPUNIT_ASSERT_EQUAL(u"Renamed"_ustr, rMgr.GetSection(1).maName);

    getUndoManager().Undo();
    CPPUNIT_ASSERT_EQUAL(aOldName, rMgr.GetSection(1).maName);

    getUndoManager().Redo();
    CPPUNIT_ASSERT_EQUAL(u"Renamed"_ustr, rMgr.GetSection(1).maName);
}

// Undo/redo of MoveSection reverts both the section order and the underlying page
// order. Mirrors the production grouping (BegUndo/EndUndo) so the page-reorder undo
// recorded by MovePages() is atomic with the section-metadata undo.
CPPUNIT_TEST_FIXTURE(SlideSectionTest, testUndoRedoMoveSection)
{
    createSdImpressDoc("pptx/slide-section-test.pptx");

    SdDrawDocument* pDoc = getDoc();
    sd::SlideSectionManager& rMgr = pDoc->GetSectionManager();
    CPPUNIT_ASSERT_EQUAL(u"Section-1"_ustr, rMgr.GetSection(0).maName);
    CPPUNIT_ASSERT_EQUAL(u"Section-2"_ustr, rMgr.GetSection(1).maName);
    CPPUNIT_ASSERT_EQUAL(u"Section-3"_ustr, rMgr.GetSection(2).maName);

    pDoc->BegUndo(u"Move section"_ustr);
    auto pUndo = std::make_unique<sd::UndoSlideSection>(*pDoc, u"Move section"_ustr);
    rMgr.MoveSection(0, 1);
    pDoc->AddUndo(std::move(pUndo));
    pDoc->EndUndo();

    CPPUNIT_ASSERT_EQUAL(u"Section-2"_ustr, rMgr.GetSection(0).maName);
    CPPUNIT_ASSERT_EQUAL(u"Section-1"_ustr, rMgr.GetSection(1).maName);
    CPPUNIT_ASSERT_EQUAL(u"Section-3"_ustr, rMgr.GetSection(2).maName);

    getUndoManager().Undo();

    CPPUNIT_ASSERT_EQUAL(u"Section-1"_ustr, rMgr.GetSection(0).maName);
    CPPUNIT_ASSERT_EQUAL(u"Section-2"_ustr, rMgr.GetSection(1).maName);
    CPPUNIT_ASSERT_EQUAL(u"Section-3"_ustr, rMgr.GetSection(2).maName);

    getUndoManager().Redo();

    CPPUNIT_ASSERT_EQUAL(u"Section-2"_ustr, rMgr.GetSection(0).maName);
    CPPUNIT_ASSERT_EQUAL(u"Section-1"_ustr, rMgr.GetSection(1).maName);
    CPPUNIT_ASSERT_EQUAL(u"Section-3"_ustr, rMgr.GetSection(2).maName);
}

// Direct unit tests for the OnSlideInserted / OnSlideRemoved primitives that
// the SdDrawDocument insert/remove paths call after a standard slide is
// added or deleted. The fixture's three sections start at standard indices
// 0, 4 and 11 (Section-1 has 4 slides, Section-2 has 7, Section-3 has 2).

// The original bug: duplicating the last slide of a section pushed the new
// slide into the next section because the next section's startIndex did not
// shift. Inserting at the end-of-section boundary (index 4 = start of
// Section-2) must shift Section-2 and Section-3 so the new slide stays in
// Section-1.
CPPUNIT_TEST_FIXTURE(SlideSectionTest, testOnSlideInsertedAtSectionBoundary)
{
    createSdImpressDoc("pptx/slide-section-test.pptx");

    sd::SlideSectionManager& rMgr = getSectionManager();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), rMgr.GetSection(0).mnStartIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), rMgr.GetSection(1).mnStartIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(11), rMgr.GetSection(2).mnStartIndex);

    rMgr.OnSlideInserted(4);

    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), rMgr.GetSection(0).mnStartIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), rMgr.GetSection(1).mnStartIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(12), rMgr.GetSection(2).mnStartIndex);
    // The new slide at index 4 belongs to Section-1, not Section-2.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), rMgr.GetSectionIndexForSlide(4));
}

// Inserting in the middle of a section keeps both sides in the same section
// and shifts everything beyond.
CPPUNIT_TEST_FIXTURE(SlideSectionTest, testOnSlideInsertedInMiddleOfSection)
{
    createSdImpressDoc("pptx/slide-section-test.pptx");

    sd::SlideSectionManager& rMgr = getSectionManager();

    rMgr.OnSlideInserted(2);

    // Section-1 still starts at 0; later sections shift up by 1.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), rMgr.GetSection(0).mnStartIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), rMgr.GetSection(1).mnStartIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(12), rMgr.GetSection(2).mnStartIndex);
}

// Insertion at index 0 must not push the first section off slide 0, or the
// new slide would be section-less.
CPPUNIT_TEST_FIXTURE(SlideSectionTest, testOnSlideInsertedAtFrontKeepsFirstSection)
{
    createSdImpressDoc("pptx/slide-section-test.pptx");

    sd::SlideSectionManager& rMgr = getSectionManager();

    rMgr.OnSlideInserted(0);

    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), rMgr.GetSection(0).mnStartIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), rMgr.GetSection(1).mnStartIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(12), rMgr.GetSection(2).mnStartIndex);
}

// Insertion past the end of all sections leaves them unchanged.
CPPUNIT_TEST_FIXTURE(SlideSectionTest, testOnSlideInsertedPastEndIsNoop)
{
    createSdImpressDoc("pptx/slide-section-test.pptx");

    sd::SlideSectionManager& rMgr = getSectionManager();
    const auto aBefore = rMgr.GetSectionsSnapshot();

    rMgr.OnSlideInserted(100);

    const auto aAfter = rMgr.GetSectionsSnapshot();
    CPPUNIT_ASSERT_EQUAL(aBefore.size(), aAfter.size());
    for (size_t i = 0; i < aBefore.size(); ++i)
        CPPUNIT_ASSERT_EQUAL(aBefore[i].mnStartIndex, aAfter[i].mnStartIndex);
}

// Removing a slide that is not a section start shifts subsequent section
// starts down by one and drops no section.
CPPUNIT_TEST_FIXTURE(SlideSectionTest, testOnSlideRemovedFromMiddle)
{
    createSdImpressDoc("pptx/slide-section-test.pptx");

    sd::SlideSectionManager& rMgr = getSectionManager();

    rMgr.OnSlideRemoved(2);

    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), rMgr.GetSectionCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), rMgr.GetSection(0).mnStartIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), rMgr.GetSection(1).mnStartIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(10), rMgr.GetSection(2).mnStartIndex);
}

// Removing the first slide of a multi-slide section keeps that section.
// Its start index stays the same because the slide that was at start+1 is
// now at start (the section's effective anchor has shifted in place).
CPPUNIT_TEST_FIXTURE(SlideSectionTest, testOnSlideRemovedFromSectionStart)
{
    createSdImpressDoc("pptx/slide-section-test.pptx");

    sd::SlideSectionManager& rMgr = getSectionManager();

    // Drive the manager primitive directly to keep the assertion focused on
    // the index arithmetic; the full integration path (via
    // SdDrawDocument::RemovePage) is covered by
    // testRemoveSingletonSlideDropsSection below.
    rMgr.OnSlideRemoved(0);

    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), rMgr.GetSectionCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), rMgr.GetSection(0).mnStartIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), rMgr.GetSection(1).mnStartIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(10), rMgr.GetSection(2).mnStartIndex);
}

// Removing the only slide of a singleton section drops that section. Uses
// the public XDrawPages UNO interface, which routes through
// SdDrawDocument::RemovePage and exercises the integration path.
CPPUNIT_TEST_FIXTURE(SlideSectionTest, testRemoveSingletonSlideDropsSection)
{
    createSdImpressDoc("pptx/slide-section-test.pptx");

    SdDrawDocument* pDoc = getDoc();
    sd::SlideSectionManager& rMgr = pDoc->GetSectionManager();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), rMgr.GetSectionCount());

    // Make Section-2 a singleton by adding a section at index 5. The fixture
    // originally has Section-2 spanning indices 4..10; AddSection(5) splits
    // it so the original Section-2 is just slide 4.
    rMgr.AddSection(5, u"Inserted"_ustr);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), rMgr.GetSectionCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), rMgr.GetSection(1).mnStartIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), rMgr.GetSection(2).mnStartIndex);

    uno::Reference<drawing::XDrawPagesSupplier> xPagesSup(mxComponent,
                                                         uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XDrawPages> xPages = xPagesSup->getDrawPages();
    uno::Reference<drawing::XDrawPage> xPage(xPages->getByIndex(4),
                                             uno::UNO_QUERY_THROW);
    xPages->remove(xPage);

    // Section-2 (the singleton) must be dropped; the others remain with
    // their starts shifted down by one for everything after the deletion.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), rMgr.GetSectionCount());
    CPPUNIT_ASSERT_EQUAL(u"Section-1"_ustr, rMgr.GetSection(0).maName);
    CPPUNIT_ASSERT_EQUAL(u"Inserted"_ustr, rMgr.GetSection(1).maName);
    CPPUNIT_ASSERT_EQUAL(u"Section-3"_ustr, rMgr.GetSection(2).maName);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), rMgr.GetSection(0).mnStartIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), rMgr.GetSection(1).mnStartIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(10), rMgr.GetSection(2).mnStartIndex);
}

// Undoing a delete that dropped a singleton section must bring the section
// back. Without the UndoSlideSection that OnSlideRemoved records, the page
// would reappear on undo but its section would stay gone.
CPPUNIT_TEST_FIXTURE(SlideSectionTest, testUndoRestoresDroppedSection)
{
    createSdImpressDoc("pptx/slide-section-test.pptx");

    SdDrawDocument* pDoc = getDoc();
    sd::SlideSectionManager& rMgr = pDoc->GetSectionManager();

    // Make Section-2 a singleton so deleting slide 4 will drop it entirely.
    rMgr.AddSection(5, u"Inserted"_ustr);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), rMgr.GetSectionCount());

    uno::Reference<drawing::XDrawPagesSupplier> xPagesSup(mxComponent,
                                                         uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XDrawPages> xPages = xPagesSup->getDrawPages();
    uno::Reference<drawing::XDrawPage> xPage(xPages->getByIndex(4),
                                             uno::UNO_QUERY_THROW);
    xPages->remove(xPage);

    // Singleton Section-2 has been dropped.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), rMgr.GetSectionCount());
    CPPUNIT_ASSERT_EQUAL(u"Section-1"_ustr, rMgr.GetSection(0).maName);
    CPPUNIT_ASSERT_EQUAL(u"Inserted"_ustr, rMgr.GetSection(1).maName);
    CPPUNIT_ASSERT_EQUAL(u"Section-3"_ustr, rMgr.GetSection(2).maName);

    getUndoManager().Undo();

    // The dropped section must be back, with its original name and anchor.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), rMgr.GetSectionCount());
    CPPUNIT_ASSERT_EQUAL(u"Section-1"_ustr, rMgr.GetSection(0).maName);
    CPPUNIT_ASSERT_EQUAL(u"Section-2"_ustr, rMgr.GetSection(1).maName);
    CPPUNIT_ASSERT_EQUAL(u"Inserted"_ustr, rMgr.GetSection(2).maName);
    CPPUNIT_ASSERT_EQUAL(u"Section-3"_ustr, rMgr.GetSection(3).maName);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), rMgr.GetSection(1).mnStartIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), rMgr.GetSection(2).mnStartIndex);

    getUndoManager().Redo();

    // Redo drops the singleton again.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), rMgr.GetSectionCount());
    CPPUNIT_ASSERT_EQUAL(u"Section-1"_ustr, rMgr.GetSection(0).maName);
    CPPUNIT_ASSERT_EQUAL(u"Inserted"_ustr, rMgr.GetSection(1).maName);
    CPPUNIT_ASSERT_EQUAL(u"Section-3"_ustr, rMgr.GetSection(2).maName);
}

#if 0

TODO: reenable me

// Inserting a slide via the full integration path (XDrawPages::insertNewByIndex)
// and then undoing must restore the section anchors to their original state.
CPPUNIT_TEST_FIXTURE(SlideSectionTest, testUndoRestoresSectionsAfterSlideInsert)
{
    createSdImpressDoc("pptx/slide-section-test.pptx");

    SdDrawDocument* pDoc = getDoc();
    sd::SlideSectionManager& rMgr = pDoc->GetSectionManager();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), rMgr.GetSectionCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), rMgr.GetSection(0).mnStartIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), rMgr.GetSection(1).mnStartIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(11), rMgr.GetSection(2).mnStartIndex);

    uno::Reference<drawing::XDrawPagesSupplier> xPagesSup(mxComponent,
                                                         uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XDrawPages> xPages = xPagesSup->getDrawPages();
    const sal_Int32 nOrigPageCount = xPages->getCount();

    xPages->insertNewByIndex(4);

    CPPUNIT_ASSERT_EQUAL(nOrigPageCount + 1, xPages->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), rMgr.GetSection(0).mnStartIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), rMgr.GetSection(1).mnStartIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(12), rMgr.GetSection(2).mnStartIndex);

    getUndoManager().Undo();

    CPPUNIT_ASSERT_EQUAL(nOrigPageCount, xPages->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), rMgr.GetSectionCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), rMgr.GetSection(0).mnStartIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), rMgr.GetSection(1).mnStartIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(11), rMgr.GetSection(2).mnStartIndex);

    getUndoManager().Redo();

    CPPUNIT_ASSERT_EQUAL(nOrigPageCount + 1, xPages->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), rMgr.GetSection(0).mnStartIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), rMgr.GetSection(1).mnStartIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(12), rMgr.GetSection(2).mnStartIndex);
}
#endif

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
