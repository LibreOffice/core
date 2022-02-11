/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

//#include <com/sun/star/text/XTextFrame.hpp>
//#include <com/sun/star/linguistic2/XHyphenator.hpp>

//#include <comphelper/scopeguard.hxx>
#include <comphelper/propertysequence.hxx>
#include <comphelper/sequence.hxx>
//#include <unotools/syslocaleoptions.hxx>
//#include <editeng/unolingu.hxx>

#include <unotxdoc.hxx>
//#include <rootfrm.hxx>
//#include <wrtsh.hxx>
//#include <IDocumentLayoutAccess.hxx>
//#include <IDocumentRedlineAccess.hxx>
//#include <frameformats.hxx>
//#include <unotextrange.hxx>
//#include <fmtanchr.hxx>

static char const DATA_DIRECTORY[] = "/sw/qa/extras/layout/data/";

/// Test to assert layout / rendering result of Writer.
class SwLayoutWriter2 : public SwModelTestBase
{
public:
    void testTableCellInvalidate();

    CPPUNIT_TEST_SUITE(SwLayoutWriter2);
    CPPUNIT_TEST(testTableCellInvalidate);
    CPPUNIT_TEST_SUITE_END();
};

void SwLayoutWriter2::testTableCellInvalidate()
{
    discardDumpedLayout();
    if (mxComponent.is())
        mxComponent->dispose();

    OUString const pName("table_cell_overlap.fodt");

    OUString const url(m_directories.getURLFromSrc(DATA_DIRECTORY) + pName);

    // note: must set Hidden property, so that SfxFrameViewWindow_Impl::Resize()
    // does *not* forward initial VCL Window Resize and thereby triggers a
    // layout which does not happen on soffice --convert-to pdf.
    std::vector<beans::PropertyValue> aFilterOptions = {
        { beans::PropertyValue("Hidden", -1, uno::Any(true), beans::PropertyState_DIRECT_VALUE) },
    };

    std::cout << pName << ":\n";

    // inline the loading because currently properties can't be passed...
    mxComponent = loadFromDesktop(url, "com.sun.star.text.TextDocument",
                                  comphelper::containerToSequence(aFilterOptions));
    uno::Sequence<beans::PropertyValue> props(comphelper::InitPropertySequence({
        { "FilterName", uno::Any(OUString("writer_pdf_Export")) },
    }));
    utl::TempFile aTempFile;
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    xStorable->storeToURL(aTempFile.GetURL(), props);

    xmlDocPtr pXmlDoc = parseLayoutDump();
    // somehow these 2 rows overlapped in the PDF unless CalcLayout() runs
    assertXPath(pXmlDoc, "/root/page[1]/anchored/fly/tab[1]/row[1]/infos/bounds", "top", "6969");
    assertXPath(pXmlDoc, "/root/page[1]/anchored/fly/tab[1]/row[1]/infos/bounds", "height", "231");
    assertXPath(pXmlDoc, "/root/page[1]/anchored/fly/tab[1]/row[1]/cell[1]/infos/bounds", "top",
                "6969");
    assertXPath(pXmlDoc, "/root/page[1]/anchored/fly/tab[1]/row[1]/cell[1]/infos/bounds", "height",
                "231");
    assertXPath(pXmlDoc, "/root/page[1]/anchored/fly/tab[1]/row[1]/cell[1]/txt[1]/infos/bounds",
                "top", "6969");
    assertXPath(pXmlDoc, "/root/page[1]/anchored/fly/tab[1]/row[1]/cell[1]/txt[1]/infos/bounds",
                "height", "231");
    assertXPath(pXmlDoc, "/root/page[1]/anchored/fly/tab[1]/row[2]/infos/bounds", "top", "7200");
    assertXPath(pXmlDoc, "/root/page[1]/anchored/fly/tab[1]/row[2]/infos/bounds", "height", "231");
    // this was 6969, causing the overlap
    assertXPath(pXmlDoc, "/root/page[1]/anchored/fly/tab[1]/row[2]/cell[1]/infos/bounds", "top",
                "7200");
    assertXPath(pXmlDoc, "/root/page[1]/anchored/fly/tab[1]/row[2]/cell[1]/infos/bounds", "height",
                "231");
    assertXPath(pXmlDoc, "/root/page[1]/anchored/fly/tab[1]/row[2]/cell[1]/txt[1]/infos/bounds",
                "top", "7200");
    assertXPath(pXmlDoc, "/root/page[1]/anchored/fly/tab[1]/row[2]/cell[1]/txt[1]/infos/bounds",
                "height", "231");

    aTempFile.EnableKillingFile();
}

CPPUNIT_TEST_SUITE_REGISTRATION(SwLayoutWriter2);
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
