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

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
