/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <memory>

#include <docsh.hxx>
#include <wrtsh.hxx>
#include <ndtxt.hxx>

namespace
{
/// Covers sw/source/core/text/itrpaint.cxx fixes.
class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase(u"/sw/qa/core/text/data/"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(Test, testRedlineRenderModeOmitInsertDelete)
{
    // Default rendering:
    createSwDoc("redline.docx");

    SwDocShell* pDocShell = getSwDocShell();
    std::shared_ptr<GDIMetaFile> xMetaFile = pDocShell->GetPreviewMetaFile();

    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    OUString aContent = getXPathContent(pXmlDoc, "(//textarray)[1]/text");
    assertXPath(pXmlDoc, "//textarray", 3);
    sal_Int32 nIndex1 = getXPath(pXmlDoc, "(//textarray)[1]", "index").toInt32();
    sal_Int32 nLength1 = getXPath(pXmlDoc, "(//textarray)[1]", "length").toInt32();
    CPPUNIT_ASSERT_EQUAL(u"baseline "_ustr, aContent.copy(nIndex1, nLength1));
    sal_Int32 nIndex2 = getXPath(pXmlDoc, "(//textarray)[2]", "index").toInt32();
    sal_Int32 nLength2 = getXPath(pXmlDoc, "(//textarray)[2]", "length").toInt32();
    CPPUNIT_ASSERT_EQUAL(u"oldcontent"_ustr, aContent.copy(nIndex2, nLength2));
    sal_Int32 nIndex3 = getXPath(pXmlDoc, "(//textarray)[3]", "index").toInt32();
    sal_Int32 nLength3 = getXPath(pXmlDoc, "(//textarray)[3]", "length").toInt32();
    CPPUNIT_ASSERT_EQUAL(u"newcontent"_ustr, aContent.copy(nIndex3, nLength3));

    // Omit inserts:
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    SwViewOption aOpt(*pWrtShell->GetViewOptions());
    aOpt.SetRedlineRenderMode(SwRedlineRenderMode::OmitInserts);
    pWrtShell->ApplyViewOptions(aOpt);

    xMetaFile = pDocShell->GetPreviewMetaFile();

    pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 2
    // - Actual  : 3
    // i.e. the inserts were not omitted.
    assertXPath(pXmlDoc, "//textarray", 2);
    nIndex1 = getXPath(pXmlDoc, "(//textarray)[1]", "index").toInt32();
    nLength1 = getXPath(pXmlDoc, "(//textarray)[1]", "length").toInt32();
    CPPUNIT_ASSERT_EQUAL(u"baseline "_ustr, aContent.copy(nIndex1, nLength1));
    nIndex2 = getXPath(pXmlDoc, "(//textarray)[2]", "index").toInt32();
    nLength2 = getXPath(pXmlDoc, "(//textarray)[2]", "length").toInt32();
    CPPUNIT_ASSERT_EQUAL(u"oldcontent"_ustr, aContent.copy(nIndex2, nLength2));

    // Omit deletes:
    aOpt.SetRedlineRenderMode(SwRedlineRenderMode::OmitDeletes);
    pWrtShell->ApplyViewOptions(aOpt);

    xMetaFile = pDocShell->GetPreviewMetaFile();

    pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    assertXPath(pXmlDoc, "//textarray", 2);
    nIndex1 = getXPath(pXmlDoc, "(//textarray)[1]", "index").toInt32();
    nLength1 = getXPath(pXmlDoc, "(//textarray)[1]", "length").toInt32();
    CPPUNIT_ASSERT_EQUAL(u"baseline "_ustr, aContent.copy(nIndex1, nLength1));
    nIndex2 = getXPath(pXmlDoc, "(//textarray)[2]", "index").toInt32();
    nLength2 = getXPath(pXmlDoc, "(//textarray)[2]", "length").toInt32();
    CPPUNIT_ASSERT_EQUAL(u"newcontent"_ustr, aContent.copy(nIndex2, nLength2));
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
