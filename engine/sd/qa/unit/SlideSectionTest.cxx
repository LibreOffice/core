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

using namespace css;

class SlideSectionTest : public SdModelTestBase
{
public:
    SlideSectionTest()
        : SdModelTestBase(u"/sd/qa/unit/data/"_ustr)
    {
    }

    sd::SlideSectionManager& getSectionManager()
    {
        auto* pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
        CPPUNIT_ASSERT(pXImpressDocument);
        SdDrawDocument* pDoc = pXImpressDocument->GetDoc();
        CPPUNIT_ASSERT(pDoc);
        return pDoc->GetSectionManager();
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

// Verify moving a section up reorders both names and slide counts in PPTX
CPPUNIT_TEST_FIXTURE(SlideSectionTest, testMoveSectionUpPPTX)
{
    createSdImpressDoc("pptx/slide-section-test.pptx");

    sd::SlideSectionManager& rMgr = getSectionManager();
    rMgr.MoveSection(1, 0);

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

// Verify moving a section up reorders both names and slide counts in ODP
CPPUNIT_TEST_FIXTURE(SlideSectionTest, testMoveSectionUpODP)
{
    createSdImpressDoc("pptx/slide-section-test.pptx");
    skipValidation();
    saveAndReload(TestFilter::ODP);

    sd::SlideSectionManager& rMgr = getSectionManager();
    rMgr.MoveSection(2, 1);

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

// Verify moving a section down reorders both names and slide counts in PPTX
CPPUNIT_TEST_FIXTURE(SlideSectionTest, testMoveSectionDownPPTX)
{
    createSdImpressDoc("pptx/slide-section-test.pptx");

    sd::SlideSectionManager& rMgr = getSectionManager();
    rMgr.MoveSection(0, 1);

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

// Verify moving a section down reorders both names and slide counts in ODP
CPPUNIT_TEST_FIXTURE(SlideSectionTest, testMoveSectionDownODP)
{
    createSdImpressDoc("pptx/slide-section-test.pptx");
    skipValidation();
    saveAndReload(TestFilter::ODP);

    sd::SlideSectionManager& rMgr = getSectionManager();
    rMgr.MoveSection(0, 1);

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

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
