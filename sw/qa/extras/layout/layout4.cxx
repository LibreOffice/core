/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>
#include <comphelper/propertysequence.hxx>
#include <com/sun/star/linguistic2/XHyphenator.hpp>
#include <com/sun/star/text/XTextSectionsSupplier.hpp>
#include <vcl/scheduler.hxx>
#include <editeng/unolingu.hxx>
#include <editeng/editobj.hxx>
#include <comphelper/propertyvalue.hxx>

#include <wrtsh.hxx>
#include <pagefrm.hxx>
#include <sortedobjs.hxx>
#include <ndtxt.hxx>
#include <unotxdoc.hxx>
#include <rootfrm.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <unoframe.hxx>
#include <drawdoc.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdotext.hxx>
#include <dcontact.hxx>
#include <frameformats.hxx>

namespace
{
/// Test to assert layout / rendering result of Writer.
class SwLayoutWriter4 : public SwModelTestBase
{
public:
    SwLayoutWriter4()
        : SwModelTestBase(u"/sw/qa/extras/layout/data/"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(SwLayoutWriter4, testHiddenSectionPageDescs)
{
    createSwDoc("hidden-sections-with-pagestyles.odt");

    // hide these just so that the height of the section is what is expected;
    // otherwise height depends on which tests run previously
    uno::Sequence<beans::PropertyValue> argsSH(
        comphelper::InitPropertySequence({ { "ShowHiddenParagraphs", uno::Any(false) } }));
    dispatchCommand(mxComponent, ".uno:ShowHiddenParagraphs", argsSH);
    uno::Sequence<beans::PropertyValue> args(
        comphelper::InitPropertySequence({ { "Fieldnames", uno::Any(false) } }));
    dispatchCommand(mxComponent, ".uno:Fieldnames", args);
    Scheduler::ProcessEventsToIdle();

    {
        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        assertXPath(pXmlDoc, "/root/page", 2);
        assertXPath(pXmlDoc, "/root/page[1]", "formatName", u"Hotti");
        assertXPath(pXmlDoc, "/root/page[1]/body/section", 1);
        assertXPath(pXmlDoc, "/root/page[1]/body/section[1]", "formatName", u"Verfügung");
        assertXPath(pXmlDoc, "/root/page[2]/body/section", 2);
        assertXPath(pXmlDoc, "/root/page[2]/body/section[1]", "formatName", u"Verfügung");
        // should be > 0, no idea why it's different on Windows
#ifdef _WIN32
        assertXPath(pXmlDoc, "/root/page[2]/body/section[1]/infos/bounds", "height", u"552");
#else
        assertXPath(pXmlDoc, "/root/page[2]/body/section[1]/infos/bounds", "height", u"532");
#endif
        assertXPath(pXmlDoc, "/root/page[2]/body/section[2]", "formatName", u"Rueckantwort");
        assertXPath(pXmlDoc, "/root/page[2]/body/section[2]/infos/bounds", "height", u"0");
        assertXPath(pXmlDoc, "/root/page[2]", "formatName", u"Folgeseite");
    }

    // toggle one section hidden and other visible
    executeMacro(
        u"vnd.sun.star.script:Standard.Module1.Main?language=Basic&location=document"_ustr);
    Scheduler::ProcessEventsToIdle();

    {
        xmlDocUniquePtr pXmlDoc = parseLayoutDump();

        // tdf#152919: Without the fix in place, this test would have failed with
        // - Expected: 3
        // - Actual  : 2
        assertXPath(pXmlDoc, "/root/page", 3);
        assertXPath(pXmlDoc, "/root/page[1]", "formatName", u"Hotti");
        assertXPath(pXmlDoc, "/root/page[1]/body/section", 2);
        assertXPath(pXmlDoc, "/root/page[1]/body/section[1]", "formatName", u"Verfügung");
        assertXPath(pXmlDoc, "/root/page[1]/body/section[2]", "formatName", u"Rueckantwort");
        assertXPath(pXmlDoc, "/root/page[2]", "formatName", u"Empty Page");
        assertXPath(pXmlDoc, "/root/page[3]/body/section", 1);
        assertXPath(pXmlDoc, "/root/page[3]/body/section[1]", "formatName", u"Rueckantwort");
        // should be > 0, no idea why it's different on Windows
#ifdef _WIN32
        assertXPath(pXmlDoc, "/root/page[3]/body/section[1]/infos/bounds", "height", u"552");
#else
        assertXPath(pXmlDoc, "/root/page[3]/body/section[1]/infos/bounds", "height", u"532");
#endif
        assertXPath(pXmlDoc, "/root/page[3]", "formatName", u"RueckantwortRechts");
    }

    // toggle one section hidden and other visible
    executeMacro(
        u"vnd.sun.star.script:Standard.Module1.Main?language=Basic&location=document"_ustr);
    Scheduler::ProcessEventsToIdle();

    {
        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        assertXPath(pXmlDoc, "/root/page", 2);
        assertXPath(pXmlDoc, "/root/page[1]", "formatName", u"Hotti");
        assertXPath(pXmlDoc, "/root/page[1]/body/section", 1);
        assertXPath(pXmlDoc, "/root/page[1]/body/section[1]", "formatName", u"Verfügung");
        assertXPath(pXmlDoc, "/root/page[2]/body/section", 2);
        assertXPath(pXmlDoc, "/root/page[2]/body/section[1]", "formatName", u"Verfügung");
        // should be > 0, no idea why it's different on Windows
#ifdef _WIN32
        assertXPath(pXmlDoc, "/root/page[2]/body/section[1]/infos/bounds", "height", u"552");
#else
        assertXPath(pXmlDoc, "/root/page[2]/body/section[1]/infos/bounds", "height", u"532");
#endif
        assertXPath(pXmlDoc, "/root/page[2]/body/section[2]", "formatName", u"Rueckantwort");
        assertXPath(pXmlDoc, "/root/page[2]/body/section[2]/infos/bounds", "height", u"0");
        assertXPath(pXmlDoc, "/root/page[2]", "formatName", u"Folgeseite");
    }
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter4, testSectionPageBreaksWithNestedSectionWithColumns)
{
    createSwDoc("section-nested-with-pagebreaks.fodt");

    auto xTextSectionsSupplier = mxComponent.queryThrow<css::text::XTextSectionsSupplier>();
    auto xSections = xTextSectionsSupplier->getTextSections();
    CPPUNIT_ASSERT(xSections);
    auto xSection1 = xSections->getByName(u"Section1"_ustr).queryThrow<css::beans::XPropertySet>();
    auto xSection2 = xSections->getByName(u"Section2"_ustr).queryThrow<css::beans::XPropertySet>();
    CPPUNIT_ASSERT(getProperty<bool>(xSection1, "IsVisible"));
    CPPUNIT_ASSERT(getProperty<bool>(xSection2, "IsVisible"));

    {
        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        assertXPath(pXmlDoc, "/root/page[1]/body/section", 1);
        assertXPath(pXmlDoc, "/root/page[1]/body/section[1]/txt", 2);
        assertXPath(pXmlDoc, "/root/page[2]/body/section", 1);
        assertXPath(pXmlDoc, "/root/page[2]/body/section[1]/txt", 1);
        assertXPath(pXmlDoc, "/root/page[2]/body/section[1]/txt/SwParaPortion/SwLineLayout",
                    "portion", u"3");
        assertXPath(pXmlDoc, "/root/page[3]/body/section", 2);
        assertXPath(pXmlDoc, "/root/page[3]/body/section[1]/txt", 1);
        assertXPath(pXmlDoc, "/root/page[3]/body/section[1]/txt/SwParaPortion/SwLineLayout",
                    "portion", u"4");
        assertXPath(pXmlDoc, "/root/page[3]/body/section[2]/column", 2);
        assertXPath(pXmlDoc, "/root/page[3]/body/section[2]/column/body/txt", 2);
        assertXPath(pXmlDoc,
                    "/root/page[3]/body/section[2]/column/body/txt[2]/SwParaPortion/SwLineLayout",
                    "portion", u"6");
        assertXPath(pXmlDoc, "/root/page[4]/body/section", 2);
        assertXPath(pXmlDoc, "/root/page[4]/body/section[1]/column/body/txt", 2);
        assertXPath(pXmlDoc, "/root/page[4]/body/section[1]/column/body/txt", 2);
        assertXPath(
            pXmlDoc,
            "/root/page[4]/body/section[1]/column[1]/body/txt[1]/SwParaPortion/SwLineLayout",
            "portion", u"7");
        assertXPath(
            pXmlDoc,
            "/root/page[4]/body/section[1]/column[2]/body/txt[1]/SwParaPortion/SwLineLayout",
            "portion", u"8");
        assertXPath(pXmlDoc, "/root/page[4]/body/section[2]/txt", 1);
        assertXPath(pXmlDoc, "/root/page[4]/body/section[2]/txt/SwParaPortion/SwLineLayout",
                    "portion", u"Text following inner section");
        assertXPath(pXmlDoc, "/root/page[4]/body/txt[1]/SwParaPortion/SwLineLayout", "portion",
                    u"Text following outer section");
    }

    xSection1->setPropertyValue(u"IsVisible"_ustr, css::uno::Any(false));
    Scheduler::ProcessEventsToIdle();

    {
        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        assertXPath(pXmlDoc, "/root/page[1]/body/txt", 2);
        assertXPath(pXmlDoc, "/root/page[1]/body/section", 3);
        assertXPath(pXmlDoc, "/root/page[1]/body/section[1]/txt", 4);
        assertXPath(pXmlDoc, "/root/page[1]/body/section[2]/column", 2);
        assertXPath(pXmlDoc, "/root/page[1]/body/section[2]/column/body/txt", 4);
        assertXPath(pXmlDoc, "/root/page[1]/body/section[3]/txt", 1);
        assertXPath(pXmlDoc, "/root/page[1]/body/section[1]/infos/bounds", "height", u"0");
        assertXPath(pXmlDoc, "/root/page[1]/body/section[2]/infos/bounds", "height", u"0");
        assertXPath(pXmlDoc, "/root/page[1]/body/section[3]/infos/bounds", "height", u"0");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/SwParaPortion/SwLineLayout", "portion",
                    u"Text following outer section");
    }

    xSection1->setPropertyValue(u"IsVisible"_ustr, css::uno::Any(true));
    Scheduler::ProcessEventsToIdle();

    {
        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        assertXPath(pXmlDoc, "/root/page[1]/body/section", 1);
        assertXPath(pXmlDoc, "/root/page[1]/body/section[1]/txt", 2);
        assertXPath(pXmlDoc, "/root/page[2]/body/section", 1);
        assertXPath(pXmlDoc, "/root/page[2]/body/section[1]/txt", 1);
        assertXPath(pXmlDoc, "/root/page[2]/body/section[1]/txt/SwParaPortion/SwLineLayout",
                    "portion", u"3");
        assertXPath(pXmlDoc, "/root/page[3]/body/section", 2);
        assertXPath(pXmlDoc, "/root/page[3]/body/section[1]/txt", 1);
        assertXPath(pXmlDoc, "/root/page[3]/body/section[1]/txt/SwParaPortion/SwLineLayout",
                    "portion", u"4");
        assertXPath(pXmlDoc, "/root/page[3]/body/section[2]/column", 2);
        assertXPath(pXmlDoc, "/root/page[3]/body/section[2]/column/body/txt", 2);
        assertXPath(pXmlDoc,
                    "/root/page[3]/body/section[2]/column/body/txt[2]/SwParaPortion/SwLineLayout",
                    "portion", u"6");
        assertXPath(pXmlDoc, "/root/page[4]/body/section", 2);
        assertXPath(pXmlDoc, "/root/page[4]/body/section[1]/column/body/txt", 2);
        assertXPath(pXmlDoc, "/root/page[4]/body/section[1]/column/body/txt", 2);
        assertXPath(
            pXmlDoc,
            "/root/page[4]/body/section[1]/column[1]/body/txt[1]/SwParaPortion/SwLineLayout",
            "portion", u"7");
        assertXPath(
            pXmlDoc,
            "/root/page[4]/body/section[1]/column[2]/body/txt[1]/SwParaPortion/SwLineLayout",
            "portion", u"8");
        assertXPath(pXmlDoc, "/root/page[4]/body/section[2]/txt", 1);
        assertXPath(pXmlDoc, "/root/page[4]/body/section[2]/txt/SwParaPortion/SwLineLayout",
                    "portion", u"Text following inner section");
        assertXPath(pXmlDoc, "/root/page[4]/body/txt[1]/SwParaPortion/SwLineLayout", "portion",
                    u"Text following outer section");
    }

    xSection2->setPropertyValue(u"IsVisible"_ustr, css::uno::Any(false));
    Scheduler::ProcessEventsToIdle();

    {
        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        assertXPath(pXmlDoc, "/root/page[1]/body/section", 1);
        assertXPath(pXmlDoc, "/root/page[1]/body/section[1]/txt", 2);
        assertXPath(pXmlDoc, "/root/page[2]/body/section", 1);
        assertXPath(pXmlDoc, "/root/page[2]/body/section[1]/txt", 1);
        assertXPath(pXmlDoc, "/root/page[2]/body/section[1]/txt/SwParaPortion/SwLineLayout",
                    "portion", u"3");
        assertXPath(pXmlDoc, "/root/page[3]/body/section", 3);
        assertXPath(pXmlDoc, "/root/page[3]/body/section[1]/txt", 1);
        assertXPath(pXmlDoc, "/root/page[3]/body/section[1]/txt/SwParaPortion/SwLineLayout",
                    "portion", u"4");
        assertXPath(pXmlDoc, "/root/page[3]/body/section[2]/column", 2);
        assertXPath(pXmlDoc, "/root/page[3]/body/section[2]/column/body/txt", 4);
        assertXPath(pXmlDoc, "/root/page[3]/body/section[2]/infos/bounds", "height", u"0");
        assertXPath(pXmlDoc, "/root/page[3]/body/section[3]/txt", 1);
        assertXPath(pXmlDoc, "/root/page[3]/body/section[3]/txt/SwParaPortion/SwLineLayout",
                    "portion", u"Text following inner section");
        assertXPath(pXmlDoc, "/root/page[3]/body/txt[1]/SwParaPortion/SwLineLayout", "portion",
                    u"Text following outer section");
    }

    xSection2->setPropertyValue(u"IsVisible"_ustr, css::uno::Any(true));
    Scheduler::ProcessEventsToIdle();

    {
        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        assertXPath(pXmlDoc, "/root/page[1]/body/section", 1);
        assertXPath(pXmlDoc, "/root/page[1]/body/section[1]/txt", 2);
        assertXPath(pXmlDoc, "/root/page[2]/body/section", 1);
        assertXPath(pXmlDoc, "/root/page[2]/body/section[1]/txt", 1);
        assertXPath(pXmlDoc, "/root/page[2]/body/section[1]/txt/SwParaPortion/SwLineLayout",
                    "portion", u"3");
        assertXPath(pXmlDoc, "/root/page[3]/body/section", 2);
        assertXPath(pXmlDoc, "/root/page[3]/body/section[1]/txt", 1);
        assertXPath(pXmlDoc, "/root/page[3]/body/section[1]/txt/SwParaPortion/SwLineLayout",
                    "portion", u"4");
        assertXPath(pXmlDoc, "/root/page[3]/body/section[2]/column", 2);
        assertXPath(pXmlDoc, "/root/page[3]/body/section[2]/column/body/txt", 2);
        assertXPath(pXmlDoc,
                    "/root/page[3]/body/section[2]/column/body/txt[2]/SwParaPortion/SwLineLayout",
                    "portion", u"6");
        assertXPath(pXmlDoc, "/root/page[4]/body/section", 2);
        assertXPath(pXmlDoc, "/root/page[4]/body/section[1]/column/body/txt", 2);
        assertXPath(pXmlDoc, "/root/page[4]/body/section[1]/column/body/txt", 2);
        assertXPath(
            pXmlDoc,
            "/root/page[4]/body/section[1]/column[1]/body/txt[1]/SwParaPortion/SwLineLayout",
            "portion", u"7");
        assertXPath(
            pXmlDoc,
            "/root/page[4]/body/section[1]/column[2]/body/txt[1]/SwParaPortion/SwLineLayout",
            "portion", u"8");
        assertXPath(pXmlDoc, "/root/page[4]/body/section[2]/txt", 1);
        assertXPath(pXmlDoc, "/root/page[4]/body/section[2]/txt/SwParaPortion/SwLineLayout",
                    "portion", u"Text following inner section");
        assertXPath(pXmlDoc, "/root/page[4]/body/txt[1]/SwParaPortion/SwLineLayout", "portion",
                    u"Text following outer section");
    }
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter4, testTdf156725)
{
    createSwDoc("tdf156725.fodt");

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page", 2);
    // the fly has 2 columns, the section in it has 2 columns, and is split
    // across the fly columns => 4 columns with 1 text frame each
    assertXPath(pXmlDoc, "/root/page[2]/body/txt/anchored/fly/column", 2);
    assertXPath(pXmlDoc, "/root/page[2]/body/txt/anchored/fly/column[1]/body/section/column", 2);
    assertXPath(pXmlDoc,
                "/root/page[2]/body/txt/anchored/fly/column[1]/body/section/column[1]/body/txt", 1);
    assertXPath(pXmlDoc,
                "/root/page[2]/body/txt/anchored/fly/column[1]/body/section/column[2]/body/txt", 1);
    assertXPath(pXmlDoc, "/root/page[2]/body/txt/anchored/fly/column[2]/body/section/column", 2);
    assertXPath(pXmlDoc,
                "/root/page[2]/body/txt/anchored/fly/column[2]/body/section/column[1]/body/txt", 1);
    assertXPath(pXmlDoc,
                "/root/page[2]/body/txt/anchored/fly/column[2]/body/section/column[2]/body/txt", 1);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter4, testTdf156419)
{
    createSwDoc("linked_frames_section_bug.odt");

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page", 2);
    // there are 2 flys on page 1, and 1 on page 2, all linked
    assertXPath(pXmlDoc, "/root/page[1]/body/txt/anchored/fly[1]/section/column", 2);
    assertXPath(pXmlDoc, "/root/page[1]/body/txt/anchored/fly[1]/section/column[1]/body/txt", 11);
    assertXPath(pXmlDoc, "/root/page[1]/body/txt/anchored/fly[1]/section/column[2]/body/txt", 11);
    assertXPath(pXmlDoc, "/root/page[1]/body/txt/anchored/fly[2]/section/column", 2);
    assertXPath(pXmlDoc, "/root/page[1]/body/txt/anchored/fly[2]/section/column[1]/body/txt", 12);
    assertXPath(pXmlDoc, "/root/page[1]/body/txt/anchored/fly[2]/section/column[2]/body/txt", 12);
    assertXPath(pXmlDoc, "/root/page[2]/body/txt/anchored/fly[1]/section/column", 2);
    assertXPath(pXmlDoc, "/root/page[2]/body/txt/anchored/fly[1]/section/column[1]/body/txt", 2);
    assertXPath(pXmlDoc, "/root/page[2]/body/txt/anchored/fly[1]/section/column[2]/body/txt", 1);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter4, testTdf145826)
{
    createSwDoc("tdf145826.odt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/root/page/body/section/column", 2);

    // Without the fix in place, this test would have failed with
    // - Expected: 1
    // - Actual  : 0
    assertXPath(pXmlDoc, "/root/page/body/section/column[1]/ftncont", 1);
    assertXPath(pXmlDoc, "/root/page/body/section/column[2]/ftncont", 1);
    assertXPath(pXmlDoc, "/root/page/body/section/column[1]/ftncont/ftn", 3);
    assertXPath(pXmlDoc, "/root/page/body/section/column[2]/ftncont/ftn", 3);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter4, testTable0HeightRows)
{
    createSwDoc("table-0-height-rows.fodt");

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    CPPUNIT_ASSERT(pXmlDoc);

    // the problem was that the table was erroneously split across 2 or 3 pages
    assertXPath(pXmlDoc, "/root/page[1]/body/tab", 1);
    assertXPath(pXmlDoc, "/root/page[1]/body/tab/row", 28);
    assertXPath(pXmlDoc, "/root/page[1]/body/tab/row/infos/bounds[@height='0']", 25);
    assertXPath(pXmlDoc, "/root/page", 1);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter4, testTdf105481)
{
    createSwDoc("tdf105481.odt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    CPPUNIT_ASSERT(pXmlDoc);

    // Without the accompanying fix in place, this test would have failed
    // because the vertical position of the as-char shape object and the
    // as-char math object will be wrong (below/beyond the text frame's bottom).

    SwTwips nTxtTop = getXPath(pXmlDoc,
                               "/root/page/anchored/fly/txt[2]"
                               "/infos/bounds",
                               "top")
                          .toInt32();
    SwTwips nTxtBottom = nTxtTop
                         + getXPath(pXmlDoc,
                                    "/root/page/anchored/fly/txt[2]"
                                    "/infos/bounds",
                                    "height")
                               .toInt32();

    SwTwips nFormula1Top = getXPath(pXmlDoc,
                                    "/root/page/anchored/fly/txt[2]"
                                    "/anchored/fly[1]/infos/bounds",
                                    "top")
                               .toInt32();
    SwTwips nFormula1Bottom = nFormula1Top
                              + getXPath(pXmlDoc,
                                         "/root/page/anchored/fly/txt[2]"
                                         "/anchored/fly[1]/infos/bounds",
                                         "height")
                                    .toInt32();

    SwTwips nFormula2Top = getXPath(pXmlDoc,
                                    "/root/page/anchored/fly/txt[2]"
                                    "/anchored/fly[2]/infos/bounds",
                                    "top")
                               .toInt32();
    SwTwips nFormula2Bottom = nFormula2Top
                              + getXPath(pXmlDoc,
                                         "/root/page/anchored/fly/txt[2]"
                                         "/anchored/fly[2]/infos/bounds",
                                         "height")
                                    .toInt32();

    // Ensure that the two formula positions are at least between top and bottom of the text frame.
    // The below two are satisfied even without the fix.
    CPPUNIT_ASSERT_GREATEREQUAL(nTxtTop, nFormula1Top);
    CPPUNIT_ASSERT_GREATEREQUAL(nTxtTop, nFormula2Top);

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected less than or equal to : 14423
    // - Actual  : 14828
    // that is, the formula is below the text-frame's y bound.
    CPPUNIT_ASSERT_LESSEQUAL(nTxtBottom, nFormula1Bottom);
    // Similarly for formula # 2 :
    // - Expected less than or equal to : 14423
    // - Actual  : 15035
    // that is, the formula is below the text-frame's y bound.
    CPPUNIT_ASSERT_LESSEQUAL(nTxtBottom, nFormula2Bottom);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter4, testTdf117982)
{
    createSwDoc("tdf117982.docx");
    SwDocShell* pShell = getSwDocShell();
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    assertXPathContent(pXmlDoc, "/metafile/push[1]/push[1]/push[1]/textarray[1]/text", u"FOO AAA");
    //The first cell must be "FOO AAA". If not, this means the first cell content not visible in
    //the source document.
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter4, testTdf128959)
{
    // no orphan/widow control in table cells
    createSwDoc("tdf128959.docx");
    SwDoc* pDoc = getSwDoc();
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // first two lines of the paragraph in the split table cell on the first page
    // (these lines were completely lost)
    assertXPath(
        pXmlDoc, "/root/page[1]/body/tab[1]/row[1]/cell[1]/txt[1]/SwParaPortion/SwLineLayout[1]",
        "portion",
        u"Lorem ipsum dolor sit amet, consectetuer adipiscing elit. Maecenas porttitor congue ");
    assertXPath(
        pXmlDoc, "/root/page[1]/body/tab[1]/row[1]/cell[1]/txt[1]/SwParaPortion/SwLineLayout[2]",
        "portion",
        u"massa. Fusce posuere, magna sed pulvinar ultricies, purus lectus malesuada libero, sit ");
    // last line of the paragraph in the split table cell on the second page
    assertXPath(pXmlDoc,
                "/root/page[2]/body/tab[1]/row[1]/cell[1]/txt[1]/SwParaPortion/SwLineLayout[1]",
                "portion", u"amet commodo magna eros quis urna.");

    // Also check that the widow control for the paragraph is not turned off:
    sw::TableFrameFormats& rTableFormats = *pDoc->GetTableFrameFormats();
    SwFrameFormat* pTableFormat = rTableFormats[0];
    SwTable* pTable = SwTable::FindTable(pTableFormat);
    const SwTableBox* pCell = pTable->GetTableBox(u"A1"_ustr);
    const SwStartNode* pStartNode = pCell->GetSttNd();
    SwNodeIndex aNodeIndex(*pStartNode);
    ++aNodeIndex;
    const SwTextNode* pTextNode = aNodeIndex.GetNode().GetTextNode();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 2
    // - Actual  : 0
    // i.e. the original fix only worked as the entire widow / orphan control was switched off.
    CPPUNIT_ASSERT_EQUAL(2, static_cast<int>(pTextNode->GetSwAttrSet().GetWidows().GetValue()));
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter4, testTdf121658)
{
    uno::Reference<linguistic2::XHyphenator> xHyphenator = LinguMgr::GetHyphenator();
    if (!xHyphenator->hasLocale(lang::Locale(u"en"_ustr, u"US"_ustr, OUString())))
        return;

    createSwDoc("tdf121658.odt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // Only 2 hyphenated words should appear in the document (in the lowercase words).
    // Uppercase words should not be hyphenated.
    assertXPath(pXmlDoc, "//SwLineLayout/child::*[@type='PortionType::Hyphen']", 2);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter4, testTdf149420)
{
    uno::Reference<linguistic2::XHyphenator> xHyphenator = LinguMgr::GetHyphenator();
    if (!xHyphenator->hasLocale(lang::Locale(u"en"_ustr, u"US"_ustr, OUString())))
        return;

    createSwDoc("tdf149420.odt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // Only 3 hyphenated words should appear in the document (last paragraph
    // has got a 1 cm hyphenation zone, removing two hyphenations, which visible
    // in the second paragraph).
    assertXPath(pXmlDoc, "//SwLineLayout/child::*[@type='PortionType::Hyphen']", 8);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter4, testTdf149324)
{
    uno::Reference<linguistic2::XHyphenator> xHyphenator = LinguMgr::GetHyphenator();
    if (!xHyphenator->hasLocale(lang::Locale(u"en"_ustr, u"US"_ustr, OUString())))
        return;

    createSwDoc("tdf149324.odt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // Only 3 hyphenated words should appear in the document (last paragraph
    // has got a 7-character word limit for hyphenation, removing the
    // hyphenation "ex-cept".
    assertXPath(pXmlDoc, "//SwLineLayout/child::*[@type='PortionType::Hyphen']", 3);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter4, testTdf149248)
{
    uno::Reference<linguistic2::XHyphenator> xHyphenator = LinguMgr::GetHyphenator();
    if (!xHyphenator->hasLocale(lang::Locale(u"en"_ustr, u"US"_ustr, OUString())))
        return;

    createSwDoc("tdf149248.odt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // Only 1 hyphenated word should appear in the document (last word of the second
    // paragraph). Last word should not be hyphenated for the fourth paragraph
    // (the same paragraph, but with forbidden hyphenation of the last word).
    assertXPath(pXmlDoc, "//SwLineLayout/child::*[@type='PortionType::Hyphen']", 1);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter4, testWriterImageNoCapture)
{
    createSwDoc("writer-image-no-capture.docx");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    CPPUNIT_ASSERT(pXmlDoc);
    sal_Int32 nPageLeft = getXPath(pXmlDoc, "//page/infos/bounds", "left").toInt32();
    sal_Int32 nImageLeft = getXPath(pXmlDoc, "//anchored/fly/infos/bounds", "left").toInt32();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected less than: 284
    // - Actual  : 284
    // i.e. the image position was modified to be inside the page frame ("captured"), even if Word
    // does not do that.
    CPPUNIT_ASSERT_LESS(nPageLeft, nImageLeft);
}

SwRect lcl_getVisibleFlyObjRect(SwWrtShell* pWrtShell)
{
    SwRootFrame* pRoot = pWrtShell->GetLayout();
    SwPageFrame* pPage = static_cast<SwPageFrame*>(pRoot->GetLower());
    pPage = static_cast<SwPageFrame*>(pPage->GetNext());
    pPage = static_cast<SwPageFrame*>(pPage->GetNext());
    SwSortedObjs* pDrawObjs = pPage->GetDrawObjs();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pDrawObjs->size());
    SwAnchoredObject* pDrawObj = (*pDrawObjs)[0];
    CPPUNIT_ASSERT_EQUAL(u"Rahmen8"_ustr, pDrawObj->GetFrameFormat()->GetName());
    pPage = static_cast<SwPageFrame*>(pPage->GetNext());
    pDrawObjs = pPage->GetDrawObjs();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pDrawObjs->size());
    pDrawObj = (*pDrawObjs)[0];
    CPPUNIT_ASSERT_EQUAL(u"Rahmen123"_ustr, pDrawObj->GetFrameFormat()->GetName());
    SwRect aFlyRect = pDrawObj->GetObjRect();
    CPPUNIT_ASSERT(pPage->getFrameArea().Contains(aFlyRect));
    return aFlyRect;
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter4, testStableAtPageAnchoredFlyPosition)
{
    // this doc has two page-anchored frames: one tiny on page 3 and one large on page 4.
    // it also has a style:master-page named "StandardEntwurf", which contains some fields.
    // if you add a break to page 2, or append some text to page 4 (or just toggle display field names),
    // the page anchored frame on page 4 vanishes, as it is incorrectly moved out of the page bounds.
    createSwDoc("stable-at-page-anchored-fly-position.odt");
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();

    // look up the layout position of the page-bound frame on page four
    SwRect aOrigRect = lcl_getVisibleFlyObjRect(pWrtShell);

    // append some text to the document to trigger bug / relayout
    pWrtShell->SttEndDoc(false);
    pWrtShell->Insert(u"foo"_ustr);

    // get the current position of the frame on page four
    SwRect aRelayoutRect = lcl_getVisibleFlyObjRect(pWrtShell);

    // the anchored frame should not have moved
    CPPUNIT_ASSERT_EQUAL(aOrigRect, aRelayoutRect);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter4, testTdf134548)
{
    createSwDoc("tdf134548.odt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // Second paragraph has two non zero width tabs in beginning of line
    {
        OUString sNodeType = getXPath(
            pXmlDoc, "/root/page/body/txt[2]/SwParaPortion/SwLineLayout/SwFixPortion[1]", "type");
        CPPUNIT_ASSERT_EQUAL(u"PortionType::TabLeft"_ustr, sNodeType);
        sal_Int32 nWidth
            = getXPath(pXmlDoc, "/root/page/body/txt[2]/SwParaPortion/SwLineLayout/SwFixPortion[1]",
                       "width")
                  .toInt32();
        CPPUNIT_ASSERT_GREATER(sal_Int32(0), nWidth);
    }
    {
        OUString sNodeType = getXPath(
            pXmlDoc, "/root/page/body/txt[2]/SwParaPortion/SwLineLayout/SwFixPortion[2]", "type");
        CPPUNIT_ASSERT_EQUAL(u"PortionType::TabLeft"_ustr, sNodeType);
        sal_Int32 nWidth
            = getXPath(pXmlDoc, "/root/page/body/txt[2]/SwParaPortion/SwLineLayout/SwFixPortion[2]",
                       "width")
                  .toInt32();
        CPPUNIT_ASSERT_GREATER(sal_Int32(0), nWidth);
    }
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter4, testTdf124423)
{
    createSwDoc("tdf124423.docx");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    sal_Int32 nFly1Width
        = getXPath(pXmlDoc, "(//anchored/fly)[1]/infos/prtBounds", "width").toInt32();
    sal_Int32 nFly2Width
        = getXPath(pXmlDoc, "(//anchored/fly)[2]/infos/prtBounds", "width").toInt32();
    sal_Int32 nPageWidth = getXPath(pXmlDoc, "//page/infos/prtBounds", "width").toInt32();
    CPPUNIT_ASSERT_EQUAL(nPageWidth, nFly2Width);
    CPPUNIT_ASSERT_LESS(nPageWidth / 2, nFly1Width);

    createSwDoc("tdf124423.odt");
    pXmlDoc = parseLayoutDump();
    nFly1Width = getXPath(pXmlDoc, "(//anchored/fly)[1]/infos/prtBounds", "width").toInt32();
    nFly2Width = getXPath(pXmlDoc, "(//anchored/fly)[2]/infos/prtBounds", "width").toInt32();
    nPageWidth = getXPath(pXmlDoc, "//page/infos/prtBounds", "width").toInt32();
    CPPUNIT_ASSERT_LESS(nPageWidth / 2, nFly2Width);
    CPPUNIT_ASSERT_LESS(nPageWidth / 2, nFly1Width);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter4, testTdf137185)
{
    // First load the sample bugdoc
    createSwDoc("tdf137185.odt");
    // Get the doc shell
    SwDoc* pDoc(getSwDoc());

    // Get the DrawObject from page
    auto pModel = pDoc->getIDocumentDrawModelAccess().GetDrawModel();
    CPPUNIT_ASSERT(pModel);
    auto pPage = pModel->GetPage(0);
    CPPUNIT_ASSERT(pPage);
    auto pObj = pPage->GetObj(0);
    CPPUNIT_ASSERT(pObj);

    // Get the format of the draw object
    auto pShape = FindFrameFormat(pObj);
    CPPUNIT_ASSERT(pShape);

    // Check the text of the shape
    uno::Reference<text::XText> xTxt(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Align me!"_ustr, xTxt->getText()->getString());

    // Add a textbox to the shape
    SwTextBoxHelper::create(pShape, pShape->FindRealSdrObject(), true);

    // Check if the text moved from the shape to the frame
    auto pFormat = SwTextBoxHelper::getOtherTextBoxFormat(getShape(1));
    auto xTextFrame = SwXTextFrame::CreateXTextFrame(*pFormat->GetDoc(), pFormat);

    CPPUNIT_ASSERT_EQUAL(u"Align me!"_ustr, xTextFrame->getText()->getString());
    SdrTextObj* pTextObj = DynCastSdrTextObj(pObj);
    CPPUNIT_ASSERT(pTextObj);
    const auto& aOutStr = pTextObj->GetOutlinerParaObject()->GetTextObject();

    CPPUNIT_ASSERT(aOutStr.GetText(0).isEmpty());
    // Before the patch it failed, because the text appeared 2 times on each other.
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter4, testTdf138782)
{
    createSwDoc("tdf138782.docx");
    auto pXml = parseLayoutDump();
    CPPUNIT_ASSERT(pXml);

    // Without the fix it failed because the 3rd shape was outside the page:
    // - Expected less than: 13327
    // - Actual  : 14469

    CPPUNIT_ASSERT_LESS(
        getXPath(pXml, "/root/page/infos/bounds", "right").toInt32(),
        getXPath(pXml, "/root/page/body/txt[8]/anchored/SwAnchoredDrawObject/bounds", "right")
            .toInt32());
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter4, testTdf135035)
{
    createSwDoc("tdf135035.docx");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    sal_Int32 nFly1Width
        = getXPath(pXmlDoc, "(//anchored/fly)[1]/infos/prtBounds", "width").toInt32();
    sal_Int32 nFly2Width
        = getXPath(pXmlDoc, "(//anchored/fly)[2]/infos/prtBounds", "width").toInt32();
    sal_Int32 nFly3Width
        = getXPath(pXmlDoc, "(//anchored/fly)[3]/infos/prtBounds", "width").toInt32();
    sal_Int32 nParentWidth = getXPath(pXmlDoc, "(//txt)[1]/infos/prtBounds", "width").toInt32();
    CPPUNIT_ASSERT_EQUAL(nParentWidth, nFly2Width);
    CPPUNIT_ASSERT_EQUAL(nParentWidth, nFly3Width);
    CPPUNIT_ASSERT_LESS(nParentWidth / 2, nFly1Width);

    createSwDoc("tdf135035.odt");
    pXmlDoc = parseLayoutDump();
    nFly1Width = getXPath(pXmlDoc, "(//anchored/fly)[1]/infos/prtBounds", "width").toInt32();
    nFly2Width = getXPath(pXmlDoc, "(//anchored/fly)[2]/infos/prtBounds", "width").toInt32();
    nFly3Width = getXPath(pXmlDoc, "(//anchored/fly)[3]/infos/prtBounds", "width").toInt32();
    nParentWidth = getXPath(pXmlDoc, "(//txt)[1]/infos/prtBounds", "width").toInt32();
    CPPUNIT_ASSERT_LESS(nParentWidth / 2, nFly2Width);
    CPPUNIT_ASSERT_LESS(nParentWidth / 2, nFly1Width);
    CPPUNIT_ASSERT_GREATER(nParentWidth, nFly3Width);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter4, testTdf146704_EndnoteInSection)
{
    createSwDoc("tdf146704_EndnoteInSection.odt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // Without the fix, the endnote placed to 2. page
    assertXPath(pXmlDoc, "/root/page", 1);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter4, testTdf139336_ColumnsWithFootnoteDoNotOccupyEntirePage)
{
    createSwDoc("tdf139336_ColumnsWithFootnoteDoNotOccupyEntirePage.docx");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // Without the fix, it would be 5 pages, but with the fix the whole document
    // would fit into 1 page, but it will be 2 pages right now, because
    // when writer import (from docx) the last section with columns, then it does not set
    // the evenly distributed settings, and this settings is required for the fix now, to
    // avoid some regression.
    assertXPath(pXmlDoc, "/root/page", 2);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter4, testTdf54465_ColumnsWithFootnoteDoNotOccupyEntirePage)
{
    // Old odt files should keep their original layout, as it was before Tdf139336 fix.
    // The new odt file is only 1 page long, while the old odt file (with the same content)
    // was more than 1 page long.
    // Note: Somewhy this test miscalculates the layout of the old odt file.
    // It will be 4 pages long, while opened in Writer it is 5 pages long.
    createSwDoc("tdf54465_ColumnsWithFootnoteDoNotOccupyEntirePage_Old.odt");
    Scheduler::ProcessEventsToIdle();
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    xmlXPathObjectPtr pXmlObj = getXPathNode(pXmlDoc, "/root/page");
    xmlNodeSetPtr pXmlNodes = pXmlObj->nodesetval;
    CPPUNIT_ASSERT_GREATER(1, xmlXPathNodeSetGetLength(pXmlNodes));
    xmlXPathFreeObject(pXmlObj);

    createSwDoc("tdf54465_ColumnsWithFootnoteDoNotOccupyEntirePage_New.odt");
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page", 1);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter4, testTdf138124)
{
    // When the only portion after the footnote number is a FlyCnt, and it doesn't fit into
    // the page width, it should be moved to the next line without the footnote number, and
    // not loop, nor OOM, nor fail assertions.

    createSwDoc("wideBoxInFootnote.fodt");
    Scheduler::ProcessEventsToIdle();

    // Without the fix in place, the layout would loop, creating new FootnoteNum portions
    // indefinitely, until OOM.
    // If the footnote paragraph had no orphan control, then the loop would finally end,
    // but an assertion in SwTextPainter::DrawTextLine would fail during paint.

    xmlDocUniquePtr pXml = parseLayoutDump();
    assertXPath(pXml, "/root/page", 1);
    assertXPath(pXml, "/root/page/ftncont/ftn/txt/anchored", 1);

    // And finally, if there were no assertion in SwTextPainter::DrawTextLine, it would have
    // produced multiple lines with FootnoteNum portions, failing the following check like
    // - Expected: 1
    // - Actual  : 49

    assertXPath(pXml,
                "/root/page/ftncont/ftn/txt//SwFieldPortion[@type='PortionType::FootnoteNum']", 1);
    assertXPath(pXml, "/root/page/ftncont/ftn/txt//SwLinePortion[@type='PortionType::FlyCnt']", 1);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter4, TestTdf161348)
{
    createSwDoc("fdo48718-1.docx");

    xmlDocUniquePtr pXml = parseLayoutDump();

    // the floating table is on page 1
    // apparently both parts of the split table are on this text frame
    assertXPath(pXml, "/root/page[1]/body/txt[2]/anchored/fly", 2);
    assertXPath(pXml, "/root/page[1]/body/txt[2]/anchored/fly/tab", 2);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter4, testTdf154113)
{
    createSwDoc("three_sections.fodt");
    Scheduler::ProcessEventsToIdle();

    dispatchCommand(mxComponent, u".uno:GoToStartOfDoc"_ustr, {});
    dispatchCommand(mxComponent, u".uno:GoToNextPara"_ustr, {});
    dispatchCommand(mxComponent, u".uno:EndOfDocumentSel"_ustr,
                    {}); // to the end of current section!
    dispatchCommand(mxComponent, u".uno:EndOfDocumentSel"_ustr, {}); // to the end of the document.

    auto xModel = mxComponent.queryThrow<frame::XModel>();
    auto xSelected = xModel->getCurrentSelection().queryThrow<container::XIndexAccess>();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xSelected->getCount());
    auto xRange = xSelected->getByIndex(0).queryThrow<text::XTextRange>();
    CPPUNIT_ASSERT_EQUAL(u"<-- Start selection here. Section1" SAL_NEWLINE_STRING
                         "Section2" SAL_NEWLINE_STRING "Section3. End selection here -->"_ustr,
                         xRange->getString());

    dispatchCommand(mxComponent, u".uno:Cut"_ustr, {});

    xSelected = xModel->getCurrentSelection().queryThrow<container::XIndexAccess>();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xSelected->getCount());
    xRange = xSelected->getByIndex(0).queryThrow<text::XTextRange>();
    CPPUNIT_ASSERT_EQUAL(OUString(), xRange->getString());

    dispatchCommand(mxComponent, u".uno:Paste"_ustr, {});

    xmlDocUniquePtr pXml = parseLayoutDump();

    // Without the fix in place, this would fail with
    // - Expected: 3
    // - Actual  : 2
    assertXPath(pXml, "/root/page/body/section", 3);
    assertXPath(pXml, "/root/page/body/section[1]/txt/SwParaPortion/SwLineLayout", "portion",
                u"<-- Start selection here. Section1");
    assertXPath(pXml, "/root/page/body/section[2]/txt/SwParaPortion/SwLineLayout", "portion",
                u"Section2");
    assertXPath(pXml, "/root/page/body/section[3]/txt/SwParaPortion/SwLineLayout", "portion",
                u"Section3. End selection here -->");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter4, testTdf155611)
{
    createSwDoc("tdf155611_table_and_nested_section.fodt");
    Scheduler::ProcessEventsToIdle();

    xmlDocUniquePtr pXml = parseLayoutDump();
    CPPUNIT_ASSERT(pXml);

    // Check the layout: single page, two section frames (no section frames after the one for Inner
    // section), correct table structure and content in the first section frame, including nested
    // table in the last cell, and the last section text.
    assertXPath(pXml, "/root/page");
    // Without the fix in place, this would fail with
    // - Expected: 2
    // - Actual  : 3
    assertXPath(pXml, "/root/page/body/section", 2);
    assertXPath(pXml, "/root/page/body/section[1]/tab");
    assertXPath(pXml, "/root/page/body/section[1]/tab/row");
    assertXPath(pXml, "/root/page/body/section[1]/tab/row/cell", 2);
    assertXPath(pXml, "/root/page/body/section[1]/tab/row/cell[1]/txt/SwParaPortion/SwLineLayout/"
                      "SwParaPortion[@portion='foo']");
    assertXPath(pXml, "/root/page/body/section[1]/tab/row/cell[2]/txt/SwParaPortion/SwLineLayout/"
                      "SwParaPortion[@portion='bar']");
    assertXPath(pXml, "/root/page/body/section[1]/tab/row/cell[2]/tab/row/cell/txt/SwParaPortion/"
                      "SwLineLayout/SwParaPortion[@portion='baz']");
    assertXPath(pXml, "/root/page/body/section[2]/txt[1]/SwParaPortion/SwLineLayout/"
                      "SwParaPortion[@portion='abc']");

    // Also must not crash on close because of a frame that accidentally fell off of the layout
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter4, testTdf152307)
{
    // Problem: On a given Writer document a table layout changed
    // after doing Tools -> Update -> Update All. The last table row on page 13
    // was bigger than the page size allowed and thus was hidden behind the footer.

    // load the document
    createSwDoc("tdf152307.odt");

    // do Tools -> Update -> Update All
    dispatchCommand(mxComponent, u".uno:UpdateAllIndexes"_ustr, {});

    // XML dump and some basic assertions
    sal_Int32 nPage = 7, nPages = 0;
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    nPages = countXPathNodes(pXmlDoc, "/root/page");
    CPPUNIT_ASSERT_MESSAGE("tdf152307.odt / testTdf152307: Not enough pages.", nPage < nPages);
    assertXPath(pXmlDoc, "/root/page[" + OString::number(nPage) + "]/body/section", 1);

    // Actual test procedure:
    // On page 7, check:
    // How much tables do we have? How much rows does the last table have?
    int nTables
        = countXPathNodes(pXmlDoc, "/root/page[" + OString::number(nPage) + "]/body/section/tab");
    int nRowsLastTable
        = countXPathNodes(pXmlDoc, "/root/page[" + OString::number(nPage) + "]/body/section/tab["
                                       + OString::number(nTables) + "]/row");
    // What is the bottom value of the last table row?
    sal_Int32 nTabBottom = getXPath(pXmlDoc,
                                    "/root/page[" + OString::number(nPage) + "]/body/section/tab["
                                        + OString::number(nTables) + "]/row["
                                        + OString::number(nRowsLastTable) + "]/infos/bounds",
                                    "bottom")
                               .toInt32();
    // Where does the footer start (footer/info/bounds/top)?
    sal_Int32 nFooterTop
        = getXPath(pXmlDoc, "/root/page[" + OString::number(nPage) + "]/footer/infos/bounds", "top")
              .toInt32();
    // Is the bottom value of the last row above the top value of the footer?
    OString aMsg = "tdf152307.odt / testTdf152307: Bottom value of last table row on page "
                   + OString::number(nPage) + " is below top value of footer: "
                   + OString::number(nTabBottom) + " > " + OString::number(nFooterTop);
    CPPUNIT_ASSERT_MESSAGE(aMsg.getStr(), nTabBottom < nFooterTop);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter4, testTdf57187_Tdf158900)
{
    // Given a document with a single paragraph, having some long space runs and line breaks
    createSwDoc("space+break.fodt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // Make sure there is only one page, one paragraph, and five lines
    assertXPath(pXmlDoc, "/root/page", 1);
    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion", 1);
    // Without the fix in place, this would fail: there used to be 6 lines
    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout", 5);

    // tdf#57187: Check that relatively short lines have spaces not participating in layout.
    // First line has 11 spaces in the end, and then a manual line break. It is rather short:
    // without block justification, it is narrower than the available space.
    // It uses the "first check if everything fits to line" return path in SwTextGuess::Guess.
    // Check that the spaces are put into a Hole portion, thus not participating in layout.
    // Without the fix, this would fail: there were only 2 portions, no Hole nor Margin portions.
    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout[1]/*", 4);
    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout[1]/*[1]", "type",
                u"PortionType::Text");
    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout[1]/*[1]", "length", u"11");
    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout[1]/*[2]", "type",
                u"PortionType::Hole");
    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout[1]/*[2]", "length", u"11");
    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout[1]/*[3]", "type",
                u"PortionType::Break");
    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout[1]/*[4]", "type",
                u"PortionType::Margin");
    // Second line has 101 spaces in the end, and then a manual line break.
    // It uses the "second check if everything fits to line" return path in SwTextGuess::Guess.
    // Check that the spaces are put into a Hole portion, thus not participating in layout.
    // Without the fix, this would fail: there were only 2 portions, no Hole portion.
    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout[2]/*", 3);
    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout[2]/*[1]", "type",
                u"PortionType::Text");
    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout[2]/*[1]", "length", u"11");
    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout[2]/*[2]", "type",
                u"PortionType::Hole");
    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout[2]/*[2]", "length",
                u"101");
    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout[2]/*[3]", "type",
                u"PortionType::Break");

    // tdf#158900: Check that the break after a long line with trailing spaces is kept on same line.
    // Without the fix in place, this would fail: the line had only 2 portions (text + hole),
    // and the break was on a separate third line
    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout[3]/*", 3);
    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout[3]/*[1]", "type",
                u"PortionType::Text");
    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout[3]/*[2]", "type",
                u"PortionType::Hole");
    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout[3]/*[3]", "type",
                u"PortionType::Break");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter4, testTdf147666)
{
    createSwDoc("tdf147666.odt");

    // Move cursor into position to insert image
    dispatchCommand(mxComponent, u".uno:GoToEndOfPara"_ustr, {});
    dispatchCommand(mxComponent, u".uno:GoDown"_ustr, {});

    save(u"writer8"_ustr);
    sal_Int32 nNonInsertedViewTop = getXPathContent(parseExport(u"settings.xml"_ustr),
                                                    "//config:config-item[@config:name='ViewTop']")
                                        .toInt32();

    // Insert image below the end of the paragraph on page one
    uno::Sequence<beans::PropertyValue> aArgs = {
        comphelper::makePropertyValue(u"FileName"_ustr, createFileURL(u"tdf147666.png")),
    };
    dispatchCommand(mxComponent, u".uno:InsertGraphic"_ustr, aArgs);

    save(u"writer8"_ustr);
    sal_Int32 nInsertedViewTop = getXPathContent(parseExport(u"settings.xml"_ustr),
                                                 "//config:config-item[@config:name='ViewTop']")
                                     .toInt32();

    // Without the fix in place this will fail with
    // nInsertedViewTop = nNonInsertedViewTop
    // i.e. when the image is inserted, the view doesn't
    // focus to the inserted graphic
    CPPUNIT_ASSERT_LESS(nInsertedViewTop, nNonInsertedViewTop);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter4, testTdf159050)
{
    // Given a document with a justified paragraph and a box with optimal wrapping
    createSwDoc("tdf159050-wrap-adjust.fodt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // Make sure there is only one page, one anchored object, one paragraph, and two lines
    assertXPath(pXmlDoc, "/root/page", 1);
    assertXPath(pXmlDoc, "/root/page/body/txt/anchored/SwAnchoredDrawObject", 1);
    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion", 1);
    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout", 2);

    // Without the fix, this would fail: there was an unexpected second fly portion.
    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout[1]/*", 4);
    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout[1]/*[1]", "type",
                u"PortionType::Text");
    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout[1]/*[1]", "length", u"91");
    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout[1]/*[2]", "type",
                u"PortionType::Hole");
    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout[1]/*[2]", "length", u"1");
    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout[1]/*[3]", "type",
                u"PortionType::Fly");
    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout[1]/*[4]", "type",
                u"PortionType::Margin");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter4, testTdf159271)
{
    // Given a document with a field with several spaces in a field content
    createSwDoc("fld-in-tbl.docx");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // Make sure there is only one page, one table with one row and two cells, and one paragraph
    assertXPath(pXmlDoc, "/root/page", 1);
    assertXPath(pXmlDoc, "/root/page/body/tab", 1);
    assertXPath(pXmlDoc, "/root/page/body/tab/row", 1);
    assertXPath(pXmlDoc, "/root/page/body/tab/row/cell", 2);
    assertXPath(pXmlDoc, "/root/page/body/txt", 1);
    assertXPath(pXmlDoc, "/root/page/body/tab/row/cell[2]/txt/SwParaPortion", 1);

    // Without the fix, this would fail:
    // - Expected: 1
    // - Actual  : 16
    // - In <>, XPath '/root/page/body/tab/row/cell[2]/txt//SwLineLayout' number of nodes is incorrect
    assertXPath(pXmlDoc, "/root/page/body/tab/row/cell[2]/txt//SwLineLayout", 1);
    assertXPath(pXmlDoc, "/root/page/body/tab/row/cell[2]/txt//SwFieldPortion", 1);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter4, testTdf159259)
{
    // Given a document with a block sdt with a single field, having framePr aligned to right
    createSwDoc("sdt+framePr.docx");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // Make sure there is only one page and one paragraph with one line and one anchored object
    assertXPath(pXmlDoc, "/root/page", 1);
    // Without the fix, this would fail: there were two paragraphs
    assertXPath(pXmlDoc, "/root/page/body/txt", 1);
    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion", 1);
    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout", 1);
    // Without the fix, this would fail: there was a field portion in the line
    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout/SwFieldPortion", 0);
    // Without the fix, this would fail: there was no anchored objects
    assertXPath(pXmlDoc, "/root/page/body/txt/anchored", 1);
    assertXPath(pXmlDoc, "/root/page/body/txt/anchored/fly", 1);

    const sal_Int32 paraRight
        = getXPath(pXmlDoc, "/root/page/body/txt/infos/bounds", "right").toInt32();
    const sal_Int32 paraHeight
        = getXPath(pXmlDoc, "/root/page/body/txt/infos/bounds", "height").toInt32();

    CPPUNIT_ASSERT_GREATER(sal_Int32(0), paraRight);
    CPPUNIT_ASSERT_GREATER(sal_Int32(0), paraHeight);

    const sal_Int32 flyRight
        = getXPath(pXmlDoc, "/root/page/body/txt/anchored/fly/infos/bounds", "right").toInt32();
    const sal_Int32 flyHeight
        = getXPath(pXmlDoc, "/root/page/body/txt/anchored/fly/infos/bounds", "height").toInt32();

    CPPUNIT_ASSERT_EQUAL(paraRight, flyRight); // The fly is right-aligned
    CPPUNIT_ASSERT_EQUAL(paraHeight, flyHeight);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter4, testLargeTopParaMarginAfterHiddenSection)
{
    // Given a large top margin in Standard paragraph style, and the first section hidden
    createSwDoc("largeTopMarginAndHiddenFirstSection.fodt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // Make sure there is only one page and two sections, first hidden (zero-height)
    assertXPath(pXmlDoc, "//page", 1);
    assertXPath(pXmlDoc, "//page/body/section", 2);
    assertXPath(pXmlDoc, "//page/body/section[1]/infos/bounds", "height", u"0");
    // Check that the top margin (1 in = 1440 twip) is added to line height (12 pt = 240 twip)
    assertXPath(pXmlDoc, "//page/body/section[2]/infos/bounds", "height", u"1680");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter4, testPageBreakInHiddenSection)
{
    // Given a paragraph with page-break-before with page style and page number
    createSwDoc("pageBreakInHiddenSection.fodt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page", 4);
    assertXPath(pXmlDoc, "//section", 4);
    assertXPath(pXmlDoc, "//page[1]/body/txt", 1);
    // The page break inside the hidden section is ignored (otherwise, there would be one section
    // on the first page)
    assertXPath(pXmlDoc, "//page[1]/body/section", 2);
    // The first section is hidden
    assertXPath(pXmlDoc, "//page[1]/body/section[1]/infos/bounds", "height", u"0");

    // Page 2 is empty even page (generated by the next page's section with page-break-before)
    assertXPath(pXmlDoc, "//page[2]/body", 0);

    // The section on page 3 is not hidden, only text in it is, therefore its page break works
    assertXPath(pXmlDoc, "//page[3]/body/section", 1);
    assertXPath(pXmlDoc, "//page[3]/body/section/infos/bounds", "height", u"0");

    // The section on page 4 is hidden, thus page break in it is ignored (no further pages, where
    // the section would be moved to otherwise)
    assertXPath(pXmlDoc, "//page[4]/body/section", 1);
    assertXPath(pXmlDoc, "//page[4]/body/section/infos/bounds", "height", u"0");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter4, testTdf159443)
{
    // Given a document with chart, which have a datatable
    createSwDoc("tdf159443.odt");
    SwDocShell* pShell = getSwDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);
    //// Without the fix, this would fail:
    //// - Expected: DataSeries1
    //// - Actual  : 1.25
    //// - In <>, XPath contents of child does not match
    assertXPathContent(
        pXmlDoc,
        "/metafile/push[1]/push[1]/push[1]/push[3]/push[1]/push[1]/push[1]/push[47]/textarray/text",
        u"DataSeries1");
    assertXPathContent(
        pXmlDoc,
        "/metafile/push[1]/push[1]/push[1]/push[3]/push[1]/push[1]/push[1]/push[49]/textarray/text",
        u"Category1");
    assertXPathContent(
        pXmlDoc,
        "/metafile/push[1]/push[1]/push[1]/push[3]/push[1]/push[1]/push[1]/push[51]/textarray/text",
        u"4.3");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter4, testTdf159422)
{
    // Given a document with chart, which have a datatable
    createSwDoc("charttable.odt");
    SwDocShell* pShell = getSwDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);
    //// Without the fix, this would fail:
    //// - Expected: 5877
    //// - Actual  : 5649
    //// - Delta   : 20
    sal_Int32 nYSymbol1 = getXPath(pXmlDoc,
                                   "/metafile/push[1]/push[1]/push[1]/push[3]/push[1]/push[1]/"
                                   "push[1]/push[99]/polypolygon/polygon/point[1]",
                                   "y")
                              .toInt32();
    CPPUNIT_ASSERT_DOUBLES_EQUAL(5877, nYSymbol1, 20);
    sal_Int32 nYSymbol2 = getXPath(pXmlDoc,
                                   "/metafile/push[1]/push[1]/push[1]/push[3]/push[1]/push[1]/"
                                   "push[1]/push[100]/polypolygon/polygon/point[1]",
                                   "y")
                              .toInt32();
    CPPUNIT_ASSERT_DOUBLES_EQUAL(6225, nYSymbol2, 20);
    sal_Int32 nYSymbol3 = getXPath(pXmlDoc,
                                   "/metafile/push[1]/push[1]/push[1]/push[3]/push[1]/push[1]/"
                                   "push[1]/push[101]/polypolygon/polygon/point[1]",
                                   "y")
                              .toInt32();
    CPPUNIT_ASSERT_DOUBLES_EQUAL(6573, nYSymbol3, 20);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter4, testTdf159456)
{
    // Given a document with chart, which have a datatable
    createSwDoc("charttable.odt");
    SwDocShell* pShell = getSwDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    //// Without the fix, this would fail:
    //// - Expected: 1
    //// - Actual  : 1.5
    //// - In <>, XPath contents of child does not match
    assertXPathContent(pXmlDoc,
                       "/metafile/push[1]/push[1]/push[1]/push[3]/push[1]/push[1]/push[1]/"
                       "push[103]/textarray/text",
                       u"1");
    assertXPathContent(pXmlDoc,
                       "/metafile/push[1]/push[1]/push[1]/push[3]/push[1]/push[1]/push[1]/"
                       "push[104]/textarray/text",
                       u"2");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter4, test_i84870)
{
    // Given a document with a large as-char object, alone in its paragraph, shifted down by a
    // header object: it must not hang in a layout loop on import
    createSwDoc("i84870.fodt");
    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter4, testTdf160549)
{
    // Given a document with a large as-char object, alone in its paragraph, shifted down by a
    // header object: it must not hang in a layout loop on import (similar to i84870, but not
    // fixed by its fix)
    createSwDoc("tdf160549.fodt");
    // The object is the first in the document; it must not move to the next page
    CPPUNIT_ASSERT_EQUAL(1, getPages());
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter4, testTdf160526)
{
    // Given a document with a large as-char object, alone in its paragraph, shifted down by
    // another body object
    createSwDoc("tdf160526.fodt");
    // It must move to the next page
    CPPUNIT_ASSERT_EQUAL(2, getPages());
    auto pExportDump = parseLayoutDump();
    assertXPath(pExportDump, "//page[2]/body/txt/anchored/SwAnchoredDrawObject");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter4, testTdf160958_page_break)
{
    // Given a document with a section with the first paragraph having a page break
    createSwDoc("tdf160958_page_break.fodt");
    auto pExportDump = parseLayoutDump();
    assertXPath(pExportDump, "//page", 2);
    // A single paragraph on the first page, with 6 lines
    assertXPath(pExportDump, "//page[1]/body/txt", 1);
    assertXPath(pExportDump, "//page[1]/body/txt/SwParaPortion/SwLineLayout", 6);
    // A section with 7 paragraphs, and two more paragraphs after the section
    assertXPath(pExportDump, "//page[2]/body/section", 1);
    assertXPath(pExportDump, "//page[2]/body/section/txt", 7);
    assertXPath(pExportDump, "//page[2]/body/section/txt[1]/SwParaPortion", 0);
    assertXPath(pExportDump, "//page[2]/body/section/txt[2]/SwParaPortion", 0);
    assertXPath(pExportDump, "//page[2]/body/section/txt[3]/SwParaPortion", 0);
    assertXPath(pExportDump, "//page[2]/body/section/txt[4]/SwParaPortion/SwLineLayout", 5);
    assertXPath(pExportDump, "//page[2]/body/section/txt[5]/SwParaPortion", 0);
    assertXPath(pExportDump, "//page[2]/body/section/txt[6]/SwParaPortion", 0);
    assertXPath(pExportDump, "//page[2]/body/section/txt[7]/SwParaPortion", 0);
    assertXPath(pExportDump, "//page[2]/body/txt", 2);
    assertXPath(pExportDump, "//page[2]/body/txt[1]/SwParaPortion/SwLineLayout", 7);
    assertXPath(pExportDump, "//page[2]/body/txt[2]/SwParaPortion", 0);

    // Hide the section
    auto xTextSectionsSupplier = mxComponent.queryThrow<css::text::XTextSectionsSupplier>();
    auto xSections = xTextSectionsSupplier->getTextSections();
    CPPUNIT_ASSERT(xSections);
    auto xSection = xSections->getByName(u"Section1"_ustr).queryThrow<css::beans::XPropertySet>();
    xSection->setPropertyValue(u"IsVisible"_ustr, css::uno::Any(false));

    calcLayout();
    pExportDump = parseLayoutDump();
    assertXPath(pExportDump, "//page", 1);
    // Three paragraphs and a hidden section on the first page
    assertXPath(pExportDump, "//page/body/txt", 3);
    assertXPath(pExportDump, "//page/body/section", 1);

    assertXPath(pExportDump, "//page/body/section/infos/bounds", "height", u"0");
    assertXPath(pExportDump, "//page/body/txt[1]/SwParaPortion/SwLineLayout", 6);
    assertXPath(pExportDump, "//page/body/section/txt", 7);
    assertXPath(pExportDump, "//page/body/section/txt[1]/SwParaPortion", 0);
    assertXPath(pExportDump, "//page/body/section/txt[2]/SwParaPortion", 0);
    assertXPath(pExportDump, "//page/body/section/txt[3]/SwParaPortion", 0);
    assertXPath(pExportDump, "//page/body/section/txt[4]/SwParaPortion", 0);
    assertXPath(pExportDump, "//page/body/section/txt[5]/SwParaPortion", 0);
    assertXPath(pExportDump, "//page/body/section/txt[6]/SwParaPortion", 0);
    assertXPath(pExportDump, "//page/body/section/txt[7]/SwParaPortion", 0);

    assertXPath(pExportDump, "//page/body/txt[2]/SwParaPortion/SwLineLayout", 7);
    assertXPath(pExportDump, "//page/body/txt[3]/SwParaPortion", 0);

    // Show the section again
    xSection->setPropertyValue(u"IsVisible"_ustr, css::uno::Any(true));

    // Check that the layout has been restored
    calcLayout();
    pExportDump = parseLayoutDump();
    assertXPath(pExportDump, "//page", 2);
    assertXPath(pExportDump, "//page[1]/body/txt", 1);
    assertXPath(pExportDump, "//page[1]/body/txt/SwParaPortion/SwLineLayout", 6);
    assertXPath(pExportDump, "//page[2]/body/section", 1);
    assertXPath(pExportDump, "//page[2]/body/section/txt", 7);
    assertXPath(pExportDump, "//page[2]/body/section/txt[1]/SwParaPortion", 0);
    assertXPath(pExportDump, "//page[2]/body/section/txt[2]/SwParaPortion", 0);
    assertXPath(pExportDump, "//page[2]/body/section/txt[3]/SwParaPortion", 0);
    assertXPath(pExportDump, "//page[2]/body/section/txt[4]/SwParaPortion/SwLineLayout", 5);
    assertXPath(pExportDump, "//page[2]/body/section/txt[5]/SwParaPortion", 0);
    assertXPath(pExportDump, "//page[2]/body/section/txt[6]/SwParaPortion", 0);
    assertXPath(pExportDump, "//page[2]/body/section/txt[7]/SwParaPortion", 0);
    assertXPath(pExportDump, "//page[2]/body/txt", 2);
    assertXPath(pExportDump, "//page[2]/body/txt[1]/SwParaPortion/SwLineLayout", 7);
    assertXPath(pExportDump, "//page[2]/body/txt[2]/SwParaPortion", 0);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter4, testTdf160958_orphans)
{
    // Given a document with a section which moves to the next page as a whole, because of orphans
    createSwDoc("tdf160958_orphans_move_section.fodt");
    auto pExportDump = parseLayoutDump();
    assertXPath(pExportDump, "//page", 2);
    // 21 paragraphs on the first page
    assertXPath(pExportDump, "//page[1]/body/txt", 21);
    assertXPath(pExportDump, "//page[1]/body/txt[1]/SwParaPortion/SwLineLayout", 6);
    assertXPath(pExportDump, "//page[1]/body/txt[2]/SwParaPortion/SwLineLayout", 5);
    assertXPath(pExportDump, "//page[1]/body/txt[3]/SwParaPortion/SwLineLayout", 7);
    assertXPath(pExportDump, "//page[1]/body/txt[4]/SwParaPortion/SwLineLayout", 16);
    assertXPath(pExportDump, "//page[1]/body/txt[5]/SwParaPortion/SwLineLayout", 1);
    assertXPath(pExportDump, "//page[1]/body/txt[6]/SwParaPortion/SwLineLayout", 1);
    assertXPath(pExportDump, "//page[1]/body/txt[7]/SwParaPortion/SwLineLayout", 1);
    assertXPath(pExportDump, "//page[1]/body/txt[8]/SwParaPortion/SwLineLayout", 1);
    assertXPath(pExportDump, "//page[1]/body/txt[9]/SwParaPortion/SwLineLayout", 1);
    assertXPath(pExportDump, "//page[1]/body/txt[10]/SwParaPortion/SwLineLayout", 1);
    assertXPath(pExportDump, "//page[1]/body/txt[11]/SwParaPortion/SwLineLayout", 1);
    assertXPath(pExportDump, "//page[1]/body/txt[12]/SwParaPortion/SwLineLayout", 1);
    assertXPath(pExportDump, "//page[1]/body/txt[13]/SwParaPortion/SwLineLayout", 1);
    assertXPath(pExportDump, "//page[1]/body/txt[14]/SwParaPortion/SwLineLayout", 1);
    assertXPath(pExportDump, "//page[1]/body/txt[15]/SwParaPortion/SwLineLayout", 1);
    assertXPath(pExportDump, "//page[1]/body/txt[16]/SwParaPortion/SwLineLayout", 1);
    assertXPath(pExportDump, "//page[1]/body/txt[17]/SwParaPortion/SwLineLayout", 1);
    assertXPath(pExportDump, "//page[1]/body/txt[18]/SwParaPortion/SwLineLayout", 1);
    assertXPath(pExportDump, "//page[1]/body/txt[19]/SwParaPortion/SwLineLayout", 1);
    assertXPath(pExportDump, "//page[1]/body/txt[20]/SwParaPortion/SwLineLayout", 1);
    assertXPath(pExportDump, "//page[1]/body/txt[21]/SwParaPortion/SwLineLayout", 1);
    // A section and one more paragraph after the section
    assertXPath(pExportDump, "//page[2]/body/section", 1);
    assertXPath(pExportDump, "//page[2]/body/section/txt", 3);
    assertXPath(pExportDump, "//page[2]/body/section/txt[1]/SwParaPortion/SwLineLayout", 6);
    assertXPath(pExportDump, "//page[2]/body/section/txt[2]/SwParaPortion/SwLineLayout", 5);
    assertXPath(pExportDump, "//page[2]/body/section/txt[3]/SwParaPortion/SwLineLayout", 7);
    assertXPath(pExportDump, "//page[2]/body/txt", 1);
    assertXPath(pExportDump, "//page[2]/body/txt[1]/SwParaPortion/SwLineLayout", 1);

    // Hide the section
    auto xTextSectionsSupplier = mxComponent.queryThrow<css::text::XTextSectionsSupplier>();
    auto xSections = xTextSectionsSupplier->getTextSections();
    CPPUNIT_ASSERT(xSections);
    auto xSection = xSections->getByName(u"Section1"_ustr).queryThrow<css::beans::XPropertySet>();
    xSection->setPropertyValue(u"IsVisible"_ustr, css::uno::Any(false));

    calcLayout();
    pExportDump = parseLayoutDump();
    assertXPath(pExportDump, "//page", 1);
    assertXPath(pExportDump, "//page/body/txt", 22);
    assertXPath(pExportDump, "//page/body/section", 1);
    assertXPath(pExportDump, "//page/body/section/infos/bounds", "height", u"0");

    // Show the section again
    xSection->setPropertyValue(u"IsVisible"_ustr, css::uno::Any(true));

    // Check that the layout has been restored
    calcLayout();
    pExportDump = parseLayoutDump();
    assertXPath(pExportDump, "//page", 2);
    assertXPath(pExportDump, "//page[1]/body/txt", 21);
    assertXPath(pExportDump, "//page[1]/body/txt[1]/SwParaPortion/SwLineLayout", 6);
    assertXPath(pExportDump, "//page[1]/body/txt[2]/SwParaPortion/SwLineLayout", 5);
    assertXPath(pExportDump, "//page[1]/body/txt[3]/SwParaPortion/SwLineLayout", 7);
    assertXPath(pExportDump, "//page[1]/body/txt[4]/SwParaPortion/SwLineLayout", 16);
    assertXPath(pExportDump, "//page[1]/body/txt[5]/SwParaPortion/SwLineLayout", 1);
    assertXPath(pExportDump, "//page[1]/body/txt[6]/SwParaPortion/SwLineLayout", 1);
    assertXPath(pExportDump, "//page[1]/body/txt[7]/SwParaPortion/SwLineLayout", 1);
    assertXPath(pExportDump, "//page[1]/body/txt[8]/SwParaPortion/SwLineLayout", 1);
    assertXPath(pExportDump, "//page[1]/body/txt[9]/SwParaPortion/SwLineLayout", 1);
    assertXPath(pExportDump, "//page[1]/body/txt[10]/SwParaPortion/SwLineLayout", 1);
    assertXPath(pExportDump, "//page[1]/body/txt[11]/SwParaPortion/SwLineLayout", 1);
    assertXPath(pExportDump, "//page[1]/body/txt[12]/SwParaPortion/SwLineLayout", 1);
    assertXPath(pExportDump, "//page[1]/body/txt[13]/SwParaPortion/SwLineLayout", 1);
    assertXPath(pExportDump, "//page[1]/body/txt[14]/SwParaPortion/SwLineLayout", 1);
    assertXPath(pExportDump, "//page[1]/body/txt[15]/SwParaPortion/SwLineLayout", 1);
    assertXPath(pExportDump, "//page[1]/body/txt[16]/SwParaPortion/SwLineLayout", 1);
    assertXPath(pExportDump, "//page[1]/body/txt[17]/SwParaPortion/SwLineLayout", 1);
    assertXPath(pExportDump, "//page[1]/body/txt[18]/SwParaPortion/SwLineLayout", 1);
    assertXPath(pExportDump, "//page[1]/body/txt[19]/SwParaPortion/SwLineLayout", 1);
    assertXPath(pExportDump, "//page[1]/body/txt[20]/SwParaPortion/SwLineLayout", 1);
    assertXPath(pExportDump, "//page[1]/body/txt[21]/SwParaPortion/SwLineLayout", 1);
    assertXPath(pExportDump, "//page[2]/body/section", 1);
    assertXPath(pExportDump, "//page[2]/body/section/txt", 3);
    assertXPath(pExportDump, "//page[2]/body/section/txt[1]/SwParaPortion/SwLineLayout", 6);
    assertXPath(pExportDump, "//page[2]/body/section/txt[2]/SwParaPortion/SwLineLayout", 5);
    assertXPath(pExportDump, "//page[2]/body/section/txt[3]/SwParaPortion/SwLineLayout", 7);
    assertXPath(pExportDump, "//page[2]/body/txt", 1);
    assertXPath(pExportDump, "//page[2]/body/txt[1]/SwParaPortion/SwLineLayout", 1);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter4, testTdf161368)
{
    // Given a document with a text body width of 116 mm, greater than 65535 twips (115.6 mm)
    createSwDoc("tdf161368.fodt");
    auto pExportDump = parseLayoutDump();
    // one page, three paragraphs, each one line (it was four pages, each paragraph split into
    // tens of short (<= 4 mm wide) lines)
    assertXPath(pExportDump, "//page", 1);
    assertXPath(pExportDump, "//page[1]/body/txt", 3);
    assertXPath(pExportDump, "//page[1]/body/txt[1]/SwParaPortion/SwLineLayout", 1);
    assertXPath(pExportDump, "//page[1]/body/txt[2]/SwParaPortion/SwLineLayout", 1);
    assertXPath(pExportDump, "//page[1]/body/txt[3]/SwParaPortion/SwLineLayout", 1);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter4, TestCrashHyphenation)
{
    //just care it doesn't crash/assert
    createSwDoc("crashHyphen.fodt");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter4, TestTdf161508)
{
    // This document must not hang on load.
    createSwDoc("tdf161508.fodt");
    auto pExportDump = parseLayoutDump();
    // The table must move completely to the second page
    assertXPath(pExportDump, "//page[1]/body/tab", 0);
    assertXPath(pExportDump, "//page[2]/body/tab", 1);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter4, TestTdf92091)
{
    // This test verifies that RTL text following an LTR footnote is measured correctly
    createSwDoc("tdf92091.fodt");
    auto pXmlDoc = parseLayoutDump();

    sal_Int32 nLayoutWidth
        = getXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout", "width")
              .toInt32();
    CPPUNIT_ASSERT_GREATER(sal_Int32(3210), nLayoutWidth);

    sal_Int32 nPor1Width
        = getXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                   "width")
              .toInt32();
    CPPUNIT_ASSERT_GREATER(sal_Int32(55), nPor1Width);

    sal_Int32 nPor2Width
        = getXPath(pXmlDoc,
                   "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
                   "width")
              .toInt32();
    CPPUNIT_ASSERT_GREATER(sal_Int32(75), nPor2Width);

    sal_Int32 nPor3Width
        = getXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                   "width")
              .toInt32();
    CPPUNIT_ASSERT_GREATER(sal_Int32(2870), nPor3Width);

    sal_Int32 nPor4Width
        = getXPath(pXmlDoc,
                   "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[2]",
                   "width")
              .toInt32();
    CPPUNIT_ASSERT_GREATER(sal_Int32(75), nPor4Width);

    sal_Int32 nPor5Width
        = getXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[3]",
                   "width")
              .toInt32();
    CPPUNIT_ASSERT_GREATER(sal_Int32(110), nPor5Width);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter4, TestTdf104209VertLTR)
{
    // Verify that vertical left-to-right text after a fly portion will overflow to the next page.
    createSwDoc("tdf107209-vert-ltr.fodt");
    auto pXmlDoc = parseLayoutDump();

    assertXPath(pXmlDoc, "//page", 2);

    assertXPath(pXmlDoc, "/root/page[1]/body/txt[4]/SwParaPortion/SwLineLayout", "portion",
                u"AAAAAAAAAAAAAAAAAAA");
    assertXPath(pXmlDoc, "/root/page[2]/body/txt[1]/SwParaPortion/SwLineLayout[1]", "portion",
                u"BBBBBBBBBBBBBBBBBBBBBB");
    assertXPath(pXmlDoc, "/root/page[2]/body/txt[1]/SwParaPortion/SwLineLayout[2]", "portion",
                u"B");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter4, TestTdf104209VertRTL)
{
    // Verify that vertical right-to-left text after a fly portion will overflow to the next page.
    createSwDoc("tdf107209-vert-rtl.fodt");
    auto pXmlDoc = parseLayoutDump();

    assertXPath(pXmlDoc, "//page", 2);

    assertXPath(pXmlDoc, "/root/page[1]/body/txt[4]/SwParaPortion/SwLineLayout", "portion",
                u"AAAAAAAAAAAAAAAAAAA");
    assertXPath(pXmlDoc, "/root/page[2]/body/txt[1]/SwParaPortion/SwLineLayout[1]", "portion",
                u"BBBBBBBBBBBBBBBBBBBBBB");
    assertXPath(pXmlDoc, "/root/page[2]/body/txt[1]/SwParaPortion/SwLineLayout[2]", "portion",
                u"B");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter4, TestTdf56408LTR)
{
    // Verify that line breaking a first bidi portion correctly underflows in LTR text
    createSwDoc("tdf56408-ltr.fodt");
    auto pXmlDoc = parseLayoutDump();

    assertXPath(pXmlDoc, "//page", 1);

    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout[1]", "portion",
                u"English English English ");
    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout[2]", "portion",
                u"((((עברית)))) English");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter4, TestTdf56408RTL)
{
    // Verify that line breaking a first bidi portion correctly underflows in RTL text
    createSwDoc("tdf56408-rtl.fodt");
    auto pXmlDoc = parseLayoutDump();

    assertXPath(pXmlDoc, "//page", 1);

    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout[1]", "portion",
                u"עברית עברית עברית ");
    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout[2]", "portion",
                u"((((English)))) עברית");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter4, TestTdf56408NoUnderflow)
{
    // The fix for tdf#56408 introduced a change to line breaking between text with
    // direction changes. This test verifies behavior in the trivial case, when a
    // break opportunity exists at the direction change boundary.
    createSwDoc("tdf56408-no-underflow.fodt");
    auto pXmlDoc = parseLayoutDump();

    assertXPath(pXmlDoc, "//page", 1);

    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout[1]", "portion",
                u"English English English ");
    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout[2]", "portion",
                u"עברית English");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter4, TestTdf56408AfterFieldCrash)
{
    // Verify there is no crash/assertion for underflow after a number field
    createSwDoc("tdf56408-after-field.fodt");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter4, TestTdf146081)
{
    // Verifies that proportional line spacing is consistent with the
    // PropLineSpacingShrinksFirstLine compatibility flag set
    createSwDoc("tdf146081-prop-spacing.fodt");
    auto pXmlDoc = parseLayoutDump();

    SwTwips nTotalHeight
        = getXPath(pXmlDoc, "/root/page/body/txt/infos/bounds", "height").toInt32();

    SwTwips nHeight1
        = getXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout[1]", "height")
              .toInt32();
    SwTwips nHeight2
        = getXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout[2]", "height")
              .toInt32();
    SwTwips nHeight3
        = getXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout[3]", "height")
              .toInt32();
    SwTwips nHeight4
        = getXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout[4]", "height")
              .toInt32();

    // All of the lines must have the same height
    CPPUNIT_ASSERT_EQUAL(nHeight1, nHeight2);
    CPPUNIT_ASSERT_EQUAL(nHeight1, nHeight3);
    CPPUNIT_ASSERT_EQUAL(nHeight1, nHeight4);

    // The total height of the paragraph must be equal to the sum of all lines
    CPPUNIT_ASSERT_EQUAL(nTotalHeight, nHeight1 * 4);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter4, TestTdf155229RowAtLeast)
{
    createSwDoc("tdf155229_row_height_at_least.docx");

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    sal_Int32 nTableHeight
        = getXPath(pXmlDoc, "/root/page[1]/body/tab[1]/row[11]/infos/bounds", "bottom").toInt32();

    // Without the fix, this was Actual  : 14174
    CPPUNIT_ASSERT_EQUAL(sal_Int32(15494), nTableHeight);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter4, TestTdf164907_rowHeightAtLeast)
{
    createSwDoc("tdf164907_rowHeightAtLeast.docx");

    // TODO: import row top/bottom height of 0 overrides
    // CPPUNIT_ASSERT_EQUAL(1, getPages());
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter4, TestTdf157829LTR)
{
    // Verify that line breaking inside a bidi portion triggers underflow to previous bidi portions
    createSwDoc("tdf157829-ltr.fodt");
    auto pXmlDoc = parseLayoutDump();

    assertXPath(pXmlDoc, "//page", 1);

    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout[1]", "portion",
                u"English English English ");
    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout[2]", "portion",
                u"עברית English");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter4, TestTdf157829RTL)
{
    // Verify that line breaking inside a bidi portion triggers underflow to previous bidi portions
    createSwDoc("tdf157829-rtl.fodt");
    auto pXmlDoc = parseLayoutDump();

    assertXPath(pXmlDoc, "//page", 1);

    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout[1]", "portion",
                u"עברית עברית עברית עברית ");
    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout[2]", "portion",
                u"English עברית");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter4, TestTdf162314)
{
    // Regression test for bidi portion line breaking where the portion layout ends with underflow,
    // but the bidi portion should not be truncated.
    createSwDoc("tdf162314.fodt");
    auto pXmlDoc = parseLayoutDump();

    assertXPath(pXmlDoc, "//page", 1);

    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout[1]", "portion",
                u"Aa aa aaaa ﷽ ");
    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout[2]", "portion", u"aaaa");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter4, TestTdf162614)
{
    // Given a table inside another table, having a fixed-height last row, with a merged cell
    // spanning two rows, with a text (having a spacing below) wrapping inside that merged cell,
    // positioned so that the first line of the text in on the first page, and the second line
    // flows onto the second page:
    createSwDoc("tdf162614.fodt");
    auto pXmlDoc = parseLayoutDump();

    // Make sure that all the tables have the correct positions and sizes
    // I find the clang-formatted version of the following awful (it is already ugly enough)
    // clang-format off

    assertXPath(pXmlDoc, "//page", 2);
    // One top-level table on page 1 (Table1), with a single row and a single cell
    assertXPath(pXmlDoc, "//page[1]/body/tab", 1);
    OUString sTable1PrecedeId = getXPath(pXmlDoc, "//page[1]/body/tab", "id");
    OUString sTable1FollowId = getXPath(pXmlDoc, "//page[1]/body/tab", "follow");
    assertXPath(pXmlDoc, "//page[1]/body/tab/infos/bounds", "top", u"2261");
    assertXPath(pXmlDoc, "//page[1]/body/tab/infos/bounds", "height", u"810");
    assertXPath(pXmlDoc, "//page[1]/body/tab/row", 1);
    assertXPath(pXmlDoc, "//page[1]/body/tab/row/cell", 1);
    OUString sTable1A1PrecedeId = getXPath(pXmlDoc, "//page[1]/body/tab/row/cell", "id");
    OUString sTable1A1FollowId = getXPath(pXmlDoc, "//page[1]/body/tab/row/cell", "follow");
    // One sub-table inside it (Table2):
    assertXPath(pXmlDoc, "//page[1]/body/tab/row/cell/tab", 1);
    OUString sTable2PrecedeId = getXPath(pXmlDoc, "//page[1]/body/tab/row/cell/tab", "id");
    OUString sTable2FollowId = getXPath(pXmlDoc, "//page[1]/body/tab/row/cell/tab", "follow");
    assertXPath(pXmlDoc, "//page[1]/body/tab/row/cell/tab/infos/bounds", "top", u"2508");
    assertXPath(pXmlDoc, "//page[1]/body/tab/row/cell/tab/infos/bounds", "height", u"543");
    assertXPath(pXmlDoc, "//page[1]/body/tab/row/cell/tab/row", 1);
    assertXPath(pXmlDoc, "//page[1]/body/tab/row/cell/tab/row/cell", 2);
    // A1
    assertXPathNoAttribute(pXmlDoc, "//page[1]/body/tab/row/cell/tab/row/cell[1]", "follow");
    assertXPath(pXmlDoc, "//page[1]/body/tab/row/cell/tab/row/cell[1]", "rowspan", u"1");
    assertXPath(pXmlDoc, "//page[1]/body/tab/row/cell/tab/row/cell[1]/txt/SwParaPortion/SwLineLayout/*", 1);
    assertXPath(pXmlDoc, "//page[1]/body/tab/row/cell/tab/row/cell[1]/txt/SwParaPortion/SwLineLayout/*[1]", "type", u"PortionType::Para");
    assertXPath(pXmlDoc, "//page[1]/body/tab/row/cell/tab/row/cell[1]/txt/SwParaPortion/SwLineLayout/*[1]", "portion", u"Table2.A1");
    // B1
    OUString sTable2B1PrecedeId = getXPath(pXmlDoc, "//page[1]/body/tab/row/cell/tab/row/cell[2]", "id");
    OUString sTable2B1FollowId = getXPath(pXmlDoc, "//page[1]/body/tab/row/cell/tab/row/cell[2]", "follow");
    assertXPath(pXmlDoc, "//page[1]/body/tab/row/cell/tab/row/cell[2]", "rowspan", u"2");
    assertXPath(pXmlDoc, "//page[1]/body/tab/row/cell/tab/row/cell[2]/infos/bounds", "height", u"523");
    assertXPath(pXmlDoc, "//page[1]/body/tab/row/cell/tab/row/cell[2]/infos/prtBounds", "height", u"503");
    OUString sTable2B1TextPrecedeId = getXPath(pXmlDoc, "//page[1]/body/tab/row/cell/tab/row/cell[2]/txt", "id");
    OUString sTable2B1TextFollowId = getXPath(pXmlDoc, "//page[1]/body/tab/row/cell/tab/row/cell[2]/txt", "follow");
    assertXPath(pXmlDoc, "//page[1]/body/tab/row/cell/tab/row/cell[2]/txt", "offset", u"0");
    assertXPath(pXmlDoc, "//page[1]/body/tab/row/cell/tab/row/cell[2]/txt/infos/bounds", "height", u"276");
    assertXPath(pXmlDoc, "//page[1]/body/tab/row/cell/tab/row/cell[2]/txt/infos/prtBounds", "height", u"276");
    assertXPath(pXmlDoc, "//page[1]/body/tab/row/cell/tab/row/cell[2]/txt/SwParaPortion/SwLineLayout/*", 2);
    assertXPath(pXmlDoc, "//page[1]/body/tab/row/cell/tab/row/cell[2]/txt/SwParaPortion/SwLineLayout/*[1]", "type", u"PortionType::Text");
    assertXPath(pXmlDoc, "//page[1]/body/tab/row/cell/tab/row/cell[2]/txt/SwParaPortion/SwLineLayout/*[1]", "portion", u"Table2.B1");
    assertXPath(pXmlDoc, "//page[1]/body/tab/row/cell/tab/row/cell[2]/txt/SwParaPortion/SwLineLayout/*[2]", "type", u"PortionType::Hole");

    // Two top-level tables on page 2
    assertXPath(pXmlDoc, "//page[2]/body/tab", 2);
    // Table1 (follow)
    CPPUNIT_ASSERT_EQUAL(sTable1FollowId, getXPath(pXmlDoc, "//page[2]/body/tab[1]", "id"));
    CPPUNIT_ASSERT_EQUAL(sTable1PrecedeId, getXPath(pXmlDoc, "//page[2]/body/tab[1]", "precede"));
    assertXPath(pXmlDoc, "//page[2]/body/tab[1]/infos/bounds", "top", u"3403");
    assertXPath(pXmlDoc, "//page[2]/body/tab[1]/infos/bounds", "height", u"514");
    assertXPath(pXmlDoc, "//page[2]/body/tab[1]/row", 1);
    assertXPath(pXmlDoc, "//page[2]/body/tab[1]/row/cell", 1);
    CPPUNIT_ASSERT_EQUAL(sTable1A1FollowId, getXPath(pXmlDoc, "//page[2]/body/tab[1]/row/cell", "id"));
    CPPUNIT_ASSERT_EQUAL(sTable1A1PrecedeId, getXPath(pXmlDoc, "//page[2]/body/tab[1]/row/cell", "precede"));
    // Table2 (follow)
    assertXPath(pXmlDoc, "//page[2]/body/tab[1]/row/cell/tab", 1);
    CPPUNIT_ASSERT_EQUAL(sTable2FollowId, getXPath(pXmlDoc, "//page[2]/body/tab[1]/row/cell/tab", "id"));
    CPPUNIT_ASSERT_EQUAL(sTable2PrecedeId, getXPath(pXmlDoc, "//page[2]/body/tab[1]/row/cell/tab", "precede"));
    assertXPath(pXmlDoc, "//page[2]/body/tab[1]/row/cell/tab/infos/bounds", "top", u"3423");
    assertXPath(pXmlDoc, "//page[2]/body/tab[1]/row/cell/tab/infos/bounds", "height", u"474");
    assertXPath(pXmlDoc, "//page[2]/body/tab[1]/row/cell/tab/row", 2);
    // Table2 row 1 (continued)
    assertXPath(pXmlDoc, "//page[2]/body/tab[1]/row/cell/tab/row[1]/cell", 2);
    // Placeholder for the cell in column 1
    assertXPath(pXmlDoc, "//page[2]/body/tab[1]/row/cell/tab/row[1]/cell[1]/infos/bounds", "height", u"0");
    // B1 (follow)
    CPPUNIT_ASSERT_EQUAL(sTable2B1FollowId, getXPath(pXmlDoc, "//page[2]/body/tab[1]/row/cell/tab/row[1]/cell[2]", "id"));
    CPPUNIT_ASSERT_EQUAL(sTable2B1PrecedeId, getXPath(pXmlDoc, "//page[2]/body/tab[1]/row/cell/tab/row[1]/cell[2]", "precede"));
    assertXPath(pXmlDoc, "//page[2]/body/tab[1]/row/cell/tab/row[1]/cell[2]", "rowspan", u"2");
    // Without the fix, this failed with
    // - Expected: 1
    // - Actual  : 2
    // - In <>, XPath '//page[2]/body/tab[1]/row/cell/tab/row[1]/cell[2]/txt' number of nodes is incorrect
    CPPUNIT_ASSERT_EQUAL(sTable2B1TextFollowId, getXPath(pXmlDoc, "//page[2]/body/tab[1]/row/cell/tab/row[1]/cell[2]/txt", "id"));
    CPPUNIT_ASSERT_EQUAL(sTable2B1TextPrecedeId, getXPath(pXmlDoc, "//page[2]/body/tab[1]/row/cell/tab/row[1]/cell[2]/txt", "precede"));
    assertXPath(pXmlDoc, "//page[2]/body/tab[1]/row/cell/tab/row[1]/cell[2]/txt/SwParaPortion/SwLineLayout/*", 1);
    assertXPath(pXmlDoc, "//page[2]/body/tab[1]/row/cell/tab/row[1]/cell[2]/txt/SwParaPortion/SwLineLayout/*[1]", "type", u"PortionType::Para");
    assertXPath(pXmlDoc, "//page[2]/body/tab[1]/row/cell/tab/row[1]/cell[2]/txt/SwParaPortion/SwLineLayout/*[1]", "portion", u"(contd.)");
    // Table2 row 2
    assertXPath(pXmlDoc, "//page[2]/body/tab[1]/row/cell/tab/row[2]/cell", 2);
    // A2
    assertXPath(pXmlDoc, "//page[2]/body/tab[1]/row/cell/tab/row[2]/cell[1]/txt/SwParaPortion/SwLineLayout/*", 1);
    assertXPath(pXmlDoc, "//page[2]/body/tab[1]/row/cell/tab/row[2]/cell[1]/txt/SwParaPortion/SwLineLayout/*[1]", "type", u"PortionType::Para");
    assertXPath(pXmlDoc, "//page[2]/body/tab[1]/row/cell/tab/row[2]/cell[1]/txt/SwParaPortion/SwLineLayout/*[1]", "portion", u"Table2.A2");
    // B2 (covered cell)
    assertXPath(pXmlDoc, "//page[2]/body/tab[1]/row/cell/tab/row[2]/cell[2]", "rowspan", u"-1");

    // Table3 (must not be collapsed)
    assertXPath(pXmlDoc, "//page[2]/body/tab[2]/infos/bounds", "top", u"4696");
    // Without the fix, this failed with
    // - Expected: 770
    // - Actual  : 267
    assertXPath(pXmlDoc, "//page[2]/body/tab[2]/infos/bounds", "height", u"770");

    // Now a test for a case that took me some time to fix when creating the patch.
    // It is the greatly simplified tdf124795-5.

    createSwDoc("C4_must_start_on_p1.fodt");
    pXmlDoc = parseLayoutDump();

    // The first line of C4 text must start on the first page - the initial version of the fix
    // moved it to page 2.

    assertXPath(pXmlDoc, "//page[1]/body/tab/row[4]/cell[3]/txt/SwParaPortion/SwLineLayout/*", 1);
    assertXPath(pXmlDoc, "//page[1]/body/tab/row[4]/cell[3]/txt/SwParaPortion/SwLineLayout/*[1]", "type", u"PortionType::Para");
    assertXPath(pXmlDoc, "//page[1]/body/tab/row[4]/cell[3]/txt/SwParaPortion/SwLineLayout/*[1]", "portion", u"C4_xxxxxxxxxxxxxxxxxxxx");

    // clang-format on
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter4, TestTdf152142)
{
    // Regression test for textbox positioning when anchored as-char in RTL context.
    createSwDoc("tdf152142.fodt");
    auto pXmlDoc = parseLayoutDump();

    assertXPath(pXmlDoc, "//page", 1);

    SwTwips nTextBoxBegin
        = getXPath(pXmlDoc, "/root/page/body/txt[2]/anchored/fly/txt/infos/bounds", "left")
              .toInt32();
    SwTwips nTextBoxEnd
        = getXPath(pXmlDoc, "/root/page/body/txt[2]/anchored/fly/txt/infos/bounds", "right")
              .toInt32();

    SwTwips nShapeBegin
        = getXPath(pXmlDoc, "/root/page/body/txt[2]/anchored/SwAnchoredDrawObject/bounds", "left")
              .toInt32();
    SwTwips nShapeEnd
        = getXPath(pXmlDoc, "/root/page/body/txt[2]/anchored/SwAnchoredDrawObject/bounds", "right")
              .toInt32();

    CPPUNIT_ASSERT_GREATER(nShapeBegin, nTextBoxBegin);
    CPPUNIT_ASSERT_LESS(nShapeEnd, nTextBoxEnd);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter4, TestTdf152142DoNotMirrorRtlDrawObjs)
{
    // Regression test for textbox positioning when anchored as-char in RTL context, with the
    // DoNotMirrorRtlDrawObjs compatibility flag set.
    createSwDoc("tdf152142-donotmirror.fodt");
    auto pXmlDoc = parseLayoutDump();

    assertXPath(pXmlDoc, "//page", 1);

    SwTwips nTextBoxBegin
        = getXPath(pXmlDoc, "/root/page/body/txt[2]/anchored/fly/txt/infos/bounds", "left")
              .toInt32();
    SwTwips nTextBoxEnd
        = getXPath(pXmlDoc, "/root/page/body/txt[2]/anchored/fly/txt/infos/bounds", "right")
              .toInt32();

    SwTwips nShapeBegin
        = getXPath(pXmlDoc, "/root/page/body/txt[2]/anchored/SwAnchoredDrawObject/bounds", "left")
              .toInt32();
    SwTwips nShapeEnd
        = getXPath(pXmlDoc, "/root/page/body/txt[2]/anchored/SwAnchoredDrawObject/bounds", "right")
              .toInt32();

    CPPUNIT_ASSERT_GREATER(nShapeBegin, nTextBoxBegin);
    CPPUNIT_ASSERT_LESS(nShapeEnd, nTextBoxEnd);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter4, testTdf152298)
{
    createSwDoc("tdf152298.docx");
    auto pDump = parseLayoutDump();

    assertXPath(pDump, "//page", 2);
    // Without the fix, this was 39
    assertXPath(pDump, "//page[1]/body/tab/row", 38);
    assertXPath(pDump, "//page[1]/body/tab/row[38]/cell[1]", "rowspan", u"4");
    OUString a38_id = getXPath(pDump, "//page[1]/body/tab/row[38]/cell[1]", "id");
    OUString follow_id = getXPath(pDump, "//page[1]/body/tab/row[38]/cell[1]", "follow");
    // The text of A38, that spans four rows, must be split: empty paragraph here
    assertXPathContent(pDump, "//page[1]/body/tab/row[38]/cell[1]/txt", u"");
    // First row is the repeating line
    assertXPathContent(pDump, "//page[2]/body/tab/row[1]/cell[1]/txt", u"1");
    assertXPathContent(pDump, "//page[2]/body/tab/row[1]/cell[2]/txt", u"2");
    assertXPathContent(pDump, "//page[2]/body/tab/row[1]/cell[3]/txt", u"3");
    // The text in the follow row's first cell is the second paragraph of A38, "10"
    assertXPath(pDump, "//page[2]/body/tab/row[2]/cell[1]", "id", follow_id);
    assertXPath(pDump, "//page[2]/body/tab/row[2]/cell[1]", "precede", a38_id);
    assertXPathContent(pDump, "//page[2]/body/tab/row[2]/cell[1]/txt", u"10");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter4, TestTdf163230)
{
    createSwDoc("tdf163230.fodt");
    auto pExportDump = parseLayoutDump();
    // The first row must split across pages, despite its "do not break" attribute, because it
    // doesn't fit on the page. Before the fix, the document had only two pages.
    assertXPath(pExportDump, "//page", 3);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter4, TestTdf163285)
{
    createSwDoc("tdf163285.fodt");
    auto pDump = parseLayoutDump();
    // The first row must split across three pages, despite its "do not break" attribute, because it
    // doesn't fit on the whole page.
    // A1 text is created such that its "pg_1", "pg_2" and "pg_3" must start the respective pages.
    assertXPath(pDump, "//page", 3);
    OUString topText1 = getXPathContent(pDump, "//page[1]/body/tab/row[1]/cell[1]/txt[1]");
    CPPUNIT_ASSERT(topText1.startsWith("pg_1"));
    OUString topText2 = getXPathContent(pDump, "//page[2]/body/tab/row[1]/cell[1]/txt[1]");
    CPPUNIT_ASSERT(topText2.startsWith("pg_2"));
    OUString topText3 = getXPathContent(pDump, "//page[3]/body/tab/row[1]/cell[1]/txt[1]");
    // Without the fix, this failed:
    CPPUNIT_ASSERT(topText3.startsWith("pg_3"));
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter4, TestTdf152839_firstRows)
{
    createSwDoc("tdf152839_firstrows.rtf");

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    sal_Int32 nHeight
        = getXPath(pXmlDoc, "/root/page[1]/body/tab[1]/row[1]/cell[2]/txt/infos/bounds", "height")
              .toInt32();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(223), nHeight);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter4, TestTdf164098)
{
    createSwDoc("tdf164098.fodt");
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();

    pWrtShell->StartOfSection(false);
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect*/ false, 6, /*bBasicCall*/ false);

    // Without the fix, this line will cause a freeze:
    pWrtShell->Insert(u"ـ"_ustr);
}

} // end of anonymous namespace

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
