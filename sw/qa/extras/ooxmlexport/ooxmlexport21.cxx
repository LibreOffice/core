/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/text/XDocumentIndex.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/style/LineSpacing.hpp>
#include <com/sun/star/style/LineSpacingMode.hpp>

#include <comphelper/configuration.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <officecfg/Office/Common.hxx>

#include <pam.hxx>
#include <unotxdoc.hxx>
#include <docsh.hxx>

namespace
{
class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase("/sw/qa/extras/ooxmlexport/data/", "Office Open XML Text")
    {
    }
};

DECLARE_OOXMLEXPORT_TEST(testTdf158597, "tdf158597.docx")
{
    // test with 2 properties: font size, italic (toggle)
    {
        uno::Reference<text::XTextRange> xParagraph(getParagraph(1));
        CPPUNIT_ASSERT_EQUAL(OUString("No style"), xParagraph->getString());
        uno::Reference<beans::XPropertySet> xProps(xParagraph, uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("List Paragraph"),
                             getProperty<OUString>(xProps, "ParaStyleName"));
        uno::Reference<text::XTextRange> xRun(getRun(xParagraph, 1));
        CPPUNIT_ASSERT_EQUAL(OUString(""), getProperty<OUString>(xRun, "CharStyleName"));
        CPPUNIT_ASSERT_EQUAL(11.f, getProperty<float>(xRun, "CharHeight"));
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_NONE, getProperty<awt::FontSlant>(xRun, "CharPosture"));
        CPPUNIT_ASSERT(!xProps->getPropertyValue("ListAutoFormat").hasValue());
    }
    {
        uno::Reference<text::XTextRange> xParagraph(getParagraph(2));
        CPPUNIT_ASSERT_EQUAL(OUString("Char style mark"), xParagraph->getString());
        uno::Reference<beans::XPropertySet> xProps(xParagraph, uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("List Paragraph"),
                             getProperty<OUString>(xProps, "ParaStyleName"));
        uno::Reference<text::XTextRange> xRun(getRun(xParagraph, 1));
        CPPUNIT_ASSERT_EQUAL(OUString(""), getProperty<OUString>(xRun, "CharStyleName"));
        CPPUNIT_ASSERT_EQUAL(11.f, getProperty<float>(xRun, "CharHeight"));
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_NONE, getProperty<awt::FontSlant>(xRun, "CharPosture"));
        comphelper::SequenceAsHashMap listAutoFormat(xProps->getPropertyValue("ListAutoFormat"));
        CPPUNIT_ASSERT_EQUAL(OUString("Emphasis"), listAutoFormat["CharStyleName"].get<OUString>());
        CPPUNIT_ASSERT(!listAutoFormat["CharHeight"].hasValue());
        CPPUNIT_ASSERT(!listAutoFormat["CharPosture"].hasValue());
    }
    {
        uno::Reference<text::XTextRange> xParagraph(getParagraph(3));
        CPPUNIT_ASSERT_EQUAL(OUString("Char style mark and text"), xParagraph->getString());
        uno::Reference<beans::XPropertySet> xProps(xParagraph, uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("List Paragraph"),
                             getProperty<OUString>(xProps, "ParaStyleName"));
        uno::Reference<text::XTextRange> xRun(getRun(xParagraph, 1));
        CPPUNIT_ASSERT_EQUAL(OUString("Emphasis"), getProperty<OUString>(xRun, "CharStyleName"));
        CPPUNIT_ASSERT_EQUAL(16.f, getProperty<float>(xRun, "CharHeight"));
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_ITALIC,
                             getProperty<awt::FontSlant>(xRun, "CharPosture"));
        comphelper::SequenceAsHashMap listAutoFormat(xProps->getPropertyValue("ListAutoFormat"));
        CPPUNIT_ASSERT_EQUAL(OUString("Emphasis"), listAutoFormat["CharStyleName"].get<OUString>());
        CPPUNIT_ASSERT(!listAutoFormat["CharHeight"].hasValue());
        CPPUNIT_ASSERT(!listAutoFormat["CharPosture"].hasValue());
    }
    {
        uno::Reference<text::XTextRange> xParagraph(getParagraph(4));
        CPPUNIT_ASSERT_EQUAL(OUString("Char style text"), xParagraph->getString());
        uno::Reference<beans::XPropertySet> xProps(xParagraph, uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("List Paragraph"),
                             getProperty<OUString>(xProps, "ParaStyleName"));
        uno::Reference<text::XTextRange> xRun(getRun(xParagraph, 1));
        CPPUNIT_ASSERT_EQUAL(OUString("Emphasis"), getProperty<OUString>(xRun, "CharStyleName"));
        CPPUNIT_ASSERT_EQUAL(16.f, getProperty<float>(xRun, "CharHeight"));
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_ITALIC,
                             getProperty<awt::FontSlant>(xRun, "CharPosture"));
        CPPUNIT_ASSERT(!xProps->getPropertyValue("ListAutoFormat").hasValue());
    }

    {
        uno::Reference<text::XTextRange> xParagraph(getParagraph(5));
        CPPUNIT_ASSERT_EQUAL(OUString("Para style"), xParagraph->getString());
        uno::Reference<beans::XPropertySet> xProps(xParagraph, uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("List Paragraph E"),
                             getProperty<OUString>(xProps, "ParaStyleName"));
        uno::Reference<text::XTextRange> xRun(getRun(xParagraph, 1));
        CPPUNIT_ASSERT_EQUAL(OUString(""), getProperty<OUString>(xRun, "CharStyleName"));
        CPPUNIT_ASSERT_EQUAL(16.f, getProperty<float>(xRun, "CharHeight"));
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_ITALIC,
                             getProperty<awt::FontSlant>(xRun, "CharPosture"));
        CPPUNIT_ASSERT(!xProps->getPropertyValue("ListAutoFormat").hasValue());
    }
    {
        uno::Reference<text::XTextRange> xParagraph(getParagraph(6));
        CPPUNIT_ASSERT_EQUAL(OUString("Para style + Char style mark"), xParagraph->getString());
        uno::Reference<beans::XPropertySet> xProps(xParagraph, uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("List Paragraph E"),
                             getProperty<OUString>(xProps, "ParaStyleName"));
        uno::Reference<text::XTextRange> xRun(getRun(xParagraph, 1));
        CPPUNIT_ASSERT_EQUAL(OUString(""), getProperty<OUString>(xRun, "CharStyleName"));
        CPPUNIT_ASSERT_EQUAL(16.f, getProperty<float>(xRun, "CharHeight"));
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_ITALIC,
                             getProperty<awt::FontSlant>(xRun, "CharPosture"));
        comphelper::SequenceAsHashMap listAutoFormat(xProps->getPropertyValue("ListAutoFormat"));
        CPPUNIT_ASSERT_EQUAL(OUString("Emphasis"), listAutoFormat["CharStyleName"].get<OUString>());
        CPPUNIT_ASSERT(!listAutoFormat["CharHeight"].hasValue());
        // bug was that here the toggle property was not overwritten
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_NONE,
                             listAutoFormat["CharPosture"].get<awt::FontSlant>());
    }
    {
        uno::Reference<text::XTextRange> xParagraph(getParagraph(7));
        CPPUNIT_ASSERT_EQUAL(OUString("Para style + Char style mark and text"),
                             xParagraph->getString());
        uno::Reference<beans::XPropertySet> xProps(xParagraph, uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("List Paragraph E"),
                             getProperty<OUString>(xProps, "ParaStyleName"));
        uno::Reference<text::XTextRange> xRun(getRun(xParagraph, 1));
        CPPUNIT_ASSERT_EQUAL(OUString("Emphasis"), getProperty<OUString>(xRun, "CharStyleName"));
        CPPUNIT_ASSERT_EQUAL(16.f, getProperty<float>(xRun, "CharHeight"));
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_NONE, getProperty<awt::FontSlant>(xRun, "CharPosture"));
        comphelper::SequenceAsHashMap listAutoFormat(xProps->getPropertyValue("ListAutoFormat"));
        CPPUNIT_ASSERT_EQUAL(OUString("Emphasis"), listAutoFormat["CharStyleName"].get<OUString>());
        CPPUNIT_ASSERT(!listAutoFormat["CharHeight"].hasValue());
        // bug was that here the toggle property was not overwritten
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_NONE,
                             listAutoFormat["CharPosture"].get<awt::FontSlant>());
    }
    {
        uno::Reference<text::XTextRange> xParagraph(getParagraph(8));
        CPPUNIT_ASSERT_EQUAL(OUString("Para style + Char style text"), xParagraph->getString());
        uno::Reference<beans::XPropertySet> xProps(xParagraph, uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("List Paragraph E"),
                             getProperty<OUString>(xProps, "ParaStyleName"));
        uno::Reference<text::XTextRange> xRun(getRun(xParagraph, 1));
        CPPUNIT_ASSERT_EQUAL(OUString("Emphasis"), getProperty<OUString>(xRun, "CharStyleName"));
        CPPUNIT_ASSERT_EQUAL(16.f, getProperty<float>(xRun, "CharHeight"));
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_NONE, getProperty<awt::FontSlant>(xRun, "CharPosture"));
        CPPUNIT_ASSERT(!xProps->getPropertyValue("ListAutoFormat").hasValue());
    }

    {
        uno::Reference<text::XTextRange> xParagraph(getParagraph(9));
        CPPUNIT_ASSERT_EQUAL(OUString("No style + direct mark"), xParagraph->getString());
        uno::Reference<beans::XPropertySet> xProps(xParagraph, uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("List Paragraph"),
                             getProperty<OUString>(xProps, "ParaStyleName"));
        uno::Reference<text::XTextRange> xRun(getRun(xParagraph, 1));
        CPPUNIT_ASSERT_EQUAL(OUString(""), getProperty<OUString>(xRun, "CharStyleName"));
        CPPUNIT_ASSERT_EQUAL(11.f, getProperty<float>(xRun, "CharHeight"));
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_NONE, getProperty<awt::FontSlant>(xRun, "CharPosture"));
        comphelper::SequenceAsHashMap listAutoFormat(xProps->getPropertyValue("ListAutoFormat"));
        CPPUNIT_ASSERT(!listAutoFormat["CharStyleName"].hasValue());
        CPPUNIT_ASSERT_EQUAL(16.f, listAutoFormat["CharHeight"].get<float>());
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_ITALIC,
                             listAutoFormat["CharPosture"].get<awt::FontSlant>());
    }
    {
        uno::Reference<text::XTextRange> xParagraph(getParagraph(10));
        CPPUNIT_ASSERT_EQUAL(OUString("Char style + direct mark"), xParagraph->getString());
        uno::Reference<beans::XPropertySet> xProps(xParagraph, uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("List Paragraph"),
                             getProperty<OUString>(xProps, "ParaStyleName"));
        uno::Reference<text::XTextRange> xRun(getRun(xParagraph, 1));
        CPPUNIT_ASSERT_EQUAL(OUString(""), getProperty<OUString>(xRun, "CharStyleName"));
        CPPUNIT_ASSERT_EQUAL(11.f, getProperty<float>(xRun, "CharHeight"));
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_NONE, getProperty<awt::FontSlant>(xRun, "CharPosture"));
        comphelper::SequenceAsHashMap listAutoFormat(xProps->getPropertyValue("ListAutoFormat"));
        CPPUNIT_ASSERT_EQUAL(OUString("Emphasis"), listAutoFormat["CharStyleName"].get<OUString>());
        CPPUNIT_ASSERT_EQUAL(16.f, listAutoFormat["CharHeight"].get<float>());
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_ITALIC,
                             listAutoFormat["CharPosture"].get<awt::FontSlant>());
    }
    {
        uno::Reference<text::XTextRange> xParagraph(getParagraph(11));
        CPPUNIT_ASSERT_EQUAL(OUString("Char style + direct mark and text"),
                             xParagraph->getString());
        uno::Reference<beans::XPropertySet> xProps(xParagraph, uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("List Paragraph"),
                             getProperty<OUString>(xProps, "ParaStyleName"));
        uno::Reference<text::XTextRange> xRun(getRun(xParagraph, 1));
        CPPUNIT_ASSERT_EQUAL(OUString("Emphasis"), getProperty<OUString>(xRun, "CharStyleName"));
        CPPUNIT_ASSERT_EQUAL(16.f, getProperty<float>(xRun, "CharHeight"));
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_ITALIC,
                             getProperty<awt::FontSlant>(xRun, "CharPosture"));
        comphelper::SequenceAsHashMap listAutoFormat(xProps->getPropertyValue("ListAutoFormat"));
        CPPUNIT_ASSERT_EQUAL(OUString("Emphasis"), listAutoFormat["CharStyleName"].get<OUString>());
        CPPUNIT_ASSERT_EQUAL(16.f, listAutoFormat["CharHeight"].get<float>());
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_ITALIC,
                             listAutoFormat["CharPosture"].get<awt::FontSlant>());
    }
    {
        uno::Reference<text::XTextRange> xParagraph(getParagraph(12));
        CPPUNIT_ASSERT_EQUAL(OUString("Char style + direct text"), xParagraph->getString());
        uno::Reference<beans::XPropertySet> xProps(xParagraph, uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("List Paragraph"),
                             getProperty<OUString>(xProps, "ParaStyleName"));
        uno::Reference<text::XTextRange> xRun(getRun(xParagraph, 1));
        CPPUNIT_ASSERT_EQUAL(OUString("Emphasis"), getProperty<OUString>(xRun, "CharStyleName"));
        CPPUNIT_ASSERT_EQUAL(16.f, getProperty<float>(xRun, "CharHeight"));
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_ITALIC,
                             getProperty<awt::FontSlant>(xRun, "CharPosture"));
        CPPUNIT_ASSERT(!xProps->getPropertyValue("ListAutoFormat").hasValue());
    }

    {
        uno::Reference<text::XTextRange> xParagraph(getParagraph(13));
        CPPUNIT_ASSERT_EQUAL(OUString("Para style + direct mark"), xParagraph->getString());
        uno::Reference<beans::XPropertySet> xProps(xParagraph, uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("List Paragraph E"),
                             getProperty<OUString>(xProps, "ParaStyleName"));
        uno::Reference<text::XTextRange> xRun(getRun(xParagraph, 1));
        CPPUNIT_ASSERT_EQUAL(OUString(""), getProperty<OUString>(xRun, "CharStyleName"));
        CPPUNIT_ASSERT_EQUAL(16.f, getProperty<float>(xRun, "CharHeight"));
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_ITALIC,
                             getProperty<awt::FontSlant>(xRun, "CharPosture"));
        comphelper::SequenceAsHashMap listAutoFormat(xProps->getPropertyValue("ListAutoFormat"));
        CPPUNIT_ASSERT(!listAutoFormat["CharStyleName"].hasValue());
        CPPUNIT_ASSERT_EQUAL(16.f, listAutoFormat["CharHeight"].get<float>());
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_ITALIC,
                             listAutoFormat["CharPosture"].get<awt::FontSlant>());
    }
    {
        uno::Reference<text::XTextRange> xParagraph(getParagraph(14));
        CPPUNIT_ASSERT_EQUAL(OUString("Para style + Char style + direct mark"),
                             xParagraph->getString());
        uno::Reference<beans::XPropertySet> xProps(xParagraph, uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("List Paragraph E"),
                             getProperty<OUString>(xProps, "ParaStyleName"));
        uno::Reference<text::XTextRange> xRun(getRun(xParagraph, 1));
        CPPUNIT_ASSERT_EQUAL(OUString(""), getProperty<OUString>(xRun, "CharStyleName"));
        CPPUNIT_ASSERT_EQUAL(16.f, getProperty<float>(xRun, "CharHeight"));
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_ITALIC,
                             getProperty<awt::FontSlant>(xRun, "CharPosture"));
        comphelper::SequenceAsHashMap listAutoFormat(xProps->getPropertyValue("ListAutoFormat"));
        CPPUNIT_ASSERT_EQUAL(OUString("Emphasis"), listAutoFormat["CharStyleName"].get<OUString>());
        CPPUNIT_ASSERT_EQUAL(16.f, listAutoFormat["CharHeight"].get<float>());
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_ITALIC,
                             listAutoFormat["CharPosture"].get<awt::FontSlant>());
    }
    {
        uno::Reference<text::XTextRange> xParagraph(getParagraph(15));
        CPPUNIT_ASSERT_EQUAL(OUString("Para style + Char style + direct mark and text"),
                             xParagraph->getString());
        uno::Reference<beans::XPropertySet> xProps(xParagraph, uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("List Paragraph E"),
                             getProperty<OUString>(xProps, "ParaStyleName"));
        uno::Reference<text::XTextRange> xRun(getRun(xParagraph, 1));
        CPPUNIT_ASSERT_EQUAL(OUString("Emphasis"), getProperty<OUString>(xRun, "CharStyleName"));
        CPPUNIT_ASSERT_EQUAL(16.f, getProperty<float>(xRun, "CharHeight"));
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_ITALIC,
                             getProperty<awt::FontSlant>(xRun, "CharPosture"));
        comphelper::SequenceAsHashMap listAutoFormat(xProps->getPropertyValue("ListAutoFormat"));
        CPPUNIT_ASSERT_EQUAL(OUString("Emphasis"), listAutoFormat["CharStyleName"].get<OUString>());
        CPPUNIT_ASSERT_EQUAL(16.f, listAutoFormat["CharHeight"].get<float>());
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_ITALIC,
                             listAutoFormat["CharPosture"].get<awt::FontSlant>());
    }
    {
        uno::Reference<text::XTextRange> xParagraph(getParagraph(16));
        CPPUNIT_ASSERT_EQUAL(OUString("Para style + Char style + direct text"),
                             xParagraph->getString());
        uno::Reference<beans::XPropertySet> xProps(xParagraph, uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("List Paragraph E"),
                             getProperty<OUString>(xProps, "ParaStyleName"));
        uno::Reference<text::XTextRange> xRun(getRun(xParagraph, 1));
        CPPUNIT_ASSERT_EQUAL(OUString("Emphasis"), getProperty<OUString>(xRun, "CharStyleName"));
        CPPUNIT_ASSERT_EQUAL(16.f, getProperty<float>(xRun, "CharHeight"));
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_ITALIC,
                             getProperty<awt::FontSlant>(xRun, "CharPosture"));
        CPPUNIT_ASSERT(!xProps->getPropertyValue("ListAutoFormat").hasValue());
    }
}

DECLARE_OOXMLEXPORT_TEST(testTdf153909_followTextFlow, "tdf153909_followTextFlow.docx")
{
    // Although MSO's UI reports "layoutInCell" for the rectangle, it isn't specified or honored
    CPPUNIT_ASSERT(!getProperty<bool>(getShape(1), "IsFollowingTextFlow"));

    // Given a table with a rectangle anchored in it (wrap-through) that appears above the table...
    xmlDocUniquePtr pDump = parseLayoutDump();
    sal_Int32 nRectBottom
        = getXPath(pDump, "//anchored/SwAnchoredDrawObject/bounds"_ostr, "bottom"_ostr).toInt32();
    sal_Int32 nTableTop = getXPath(pDump, "//tab/row/infos/bounds"_ostr, "top"_ostr).toInt32();
    // The entire table must be below the rectangle
    CPPUNIT_ASSERT(nTableTop > nRectBottom);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf159207_footerFramePrBorder)
{
    loadFromFile(u"tdf159207_footerFramePrBorder.docx"); // re-imports as editeng Frame/Shape

    // given a doc with footer paragraphs frame (with a top border, but no left border)
    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextFramesSupplier->getTextFrames(),
                                                         uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xFrame0(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    auto aBorder = getProperty<table::BorderLine2>(xFrame0, "LeftBorder");
    sal_uInt32 nBorderWidth
        = aBorder.OuterLineWidth + aBorder.InnerLineWidth + aBorder.LineDistance;
    // Without patch it failed with Expected 0, Actual 26
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Left border:", static_cast<sal_uInt32>(0), nBorderWidth);

    // TODO: there SHOULD BE a top border, and even if loaded, it would be lost on re-import...
}

CPPUNIT_TEST_FIXTURE(Test, testPersonalMetaData)
{
    // 1. Remove all personal info
    auto pBatch(comphelper::ConfigurationChanges::create());
    officecfg::Office::Common::Security::Scripting::RemovePersonalInfoOnSaving::set(true, pBatch);
    pBatch->commit();
    loadAndSave("personalmetadata.docx");

    xmlDocUniquePtr pAppDoc = parseExport("docProps/app.xml");
    assertXPath(pAppDoc, "/extended-properties:Properties/extended-properties:Template"_ostr, 0);
    assertXPath(pAppDoc, "/extended-properties:Properties/extended-properties:TotalTime"_ostr, 0);
    xmlDocUniquePtr pCoreDoc = parseExport("docProps/core.xml");
    assertXPath(pCoreDoc, "/cp:coreProperties/dcterms:created"_ostr, 0);
    assertXPath(pCoreDoc, "/cp:coreProperties/dcterms:modified"_ostr, 0);
    assertXPath(pCoreDoc, "/cp:coreProperties/dc:creator"_ostr, 0);
    assertXPath(pCoreDoc, "/cp:coreProperties/cp:lastModifiedBy"_ostr, 0);
    assertXPath(pCoreDoc, "/cp:coreProperties/cp:lastPrinted"_ostr, 0);
    assertXPath(pCoreDoc, "/cp:coreProperties/cp:revision"_ostr, 0);

    // 2. Remove personal information, keep user information
    officecfg::Office::Common::Security::Scripting::KeepDocUserInfoOnSaving::set(true, pBatch);
    pBatch->commit();
    loadAndSave("personalmetadata.docx");

    pAppDoc = parseExport("docProps/app.xml");
    assertXPath(pAppDoc, "/extended-properties:Properties/extended-properties:Template"_ostr, 0);
    assertXPath(pAppDoc, "/extended-properties:Properties/extended-properties:TotalTime"_ostr, 0);
    pCoreDoc = parseExport("docProps/core.xml");
    assertXPath(pCoreDoc, "/cp:coreProperties/dcterms:created"_ostr, 1);
    assertXPath(pCoreDoc, "/cp:coreProperties/dcterms:modified"_ostr, 1);
    assertXPath(pCoreDoc, "/cp:coreProperties/dc:creator"_ostr, 1);
    assertXPath(pCoreDoc, "/cp:coreProperties/cp:lastModifiedBy"_ostr, 1);
    assertXPath(pCoreDoc, "/cp:coreProperties/cp:lastPrinted"_ostr, 1);
    assertXPath(pCoreDoc, "/cp:coreProperties/cp:revision"_ostr, 0);
}

} // end of anonymous namespace
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
