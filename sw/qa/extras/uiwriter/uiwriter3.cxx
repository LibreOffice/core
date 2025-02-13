/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <vcl/scheduler.hxx>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/document/XDocumentInsertable.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/table/TableBorder2.hpp>
#include <com/sun/star/text/XDocumentIndex.hpp>
#include <com/sun/star/text/XTextFrame.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/text/XPageCursor.hpp>
#include <comphelper/propertysequence.hxx>
#include <comphelper/seqstream.hxx>
#include <swdtflvr.hxx>
#include <o3tl/string_view.hxx>

#include <view.hxx>
#include <wrtsh.hxx>
#include <unotxdoc.hxx>
#include <ndtxt.hxx>
#include <toxmgr.hxx>
#include <IDocumentFieldsAccess.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <fmtinfmt.hxx>

/// Third set of tests asserting the behavior of Writer user interface shells.
class SwUiWriterTest3 : public SwModelTestBase
{
public:
    SwUiWriterTest3()
        : SwModelTestBase(u"/sw/qa/extras/uiwriter/data/"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testPlaceholderHTMLPaste)
{
    {
        createSwDoc();

        SwWrtShell* const pWrtShell = getSwDocShell()->GetWrtShell();
        pWrtShell->Insert(u"AAA"_ustr);
        pWrtShell->SplitNode();
        pWrtShell->Insert(u"BBB"_ustr);
        pWrtShell->SplitNode();
        pWrtShell->Insert(u"CCC"_ustr);

        dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});
        dispatchCommand(mxComponent, u".uno:Copy"_ustr, {});
    }

    createSwDoc("placeholder-bold.fodt");

    // select placeholder field
    SwWrtShell* const pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/false, 5, /*bBasicCall=*/false);
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/true, 1, /*bBasicCall=*/false);

    // Paste special as HTML
    uno::Sequence<beans::PropertyValue> aPropertyValues = comphelper::InitPropertySequence(
        { { "SelectedFormat", uno::Any(static_cast<sal_uInt32>(SotClipboardFormatId::HTML)) } });

    dispatchCommand(mxComponent, u".uno:ClipboardFormatItems"_ustr, aPropertyValues);

    CPPUNIT_ASSERT_EQUAL(int(4), getParagraphs());

    CPPUNIT_ASSERT_EQUAL(
        awt::FontWeight::NORMAL,
        getProperty<float>(getRun(getParagraph(1), 1, u"Test "_ustr), u"CharWeight"_ustr));
    CPPUNIT_ASSERT_EQUAL(
        awt::FontWeight::BOLD,
        getProperty<float>(getRun(getParagraph(1), 2, u"AAA"_ustr), u"CharWeight"_ustr));
    CPPUNIT_ASSERT_EQUAL(awt::FontWeight::BOLD,
                         getProperty<float>(getParagraph(2), u"CharWeight"_ustr));
    CPPUNIT_ASSERT_EQUAL(awt::FontWeight::BOLD,
                         getProperty<float>(getParagraph(3), u"CharWeight"_ustr));
    CPPUNIT_ASSERT_EQUAL(
        awt::FontWeight::NORMAL,
        getProperty<float>(getRun(getParagraph(4), 1, u" test"_ustr), u"CharWeight"_ustr));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testPlaceholderHTMLInsert)
{
    createSwDoc("placeholder-bold-cs.fodt");

    // select placeholder field
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextCursor> xCursor{ xTextDocument->getText()->createTextCursor() };
    xCursor->gotoStart(false);
    xCursor->goRight(5, false);
    xCursor->goRight(1, true);

    sal_Int8 const html[] = "<meta http-equiv=\"content-type\" content=\"text/html; "
                            "charset=utf-8\"><p>AAA</p><p>BBB</p><p>CCC</p>";
    uno::Reference<io::XInputStream> xStream{ new comphelper::MemoryInputStream{ html,
                                                                                 sizeof(html) } };

    // insert HTML file
    uno::Sequence<beans::PropertyValue> aPropertyValues = comphelper::InitPropertySequence(
        { { "InputStream", uno::Any(xStream) },
          { "Hidden", uno::Any(true) },
          { "FilterName", uno::Any(u"HTML (StarWriter)"_ustr) } });

    uno::Reference<document::XDocumentInsertable> xDocInsert{ xCursor, uno::UNO_QUERY };
    xDocInsert->insertDocumentFromURL("private:stream", aPropertyValues);

    CPPUNIT_ASSERT_EQUAL(int(4), getParagraphs());

    CPPUNIT_ASSERT_EQUAL(
        awt::FontWeight::NORMAL,
        getProperty<float>(getRun(getParagraph(1), 1, u"Test "_ustr), u"CharWeight"_ustr));
    CPPUNIT_ASSERT_EQUAL(
        awt::FontWeight::BOLD,
        getProperty<float>(getRun(getParagraph(1), 2, u"AAA"_ustr), u"CharWeight"_ustr));
    CPPUNIT_ASSERT_EQUAL(awt::FontWeight::BOLD,
                         getProperty<float>(getParagraph(2), u"CharWeight"_ustr));
    CPPUNIT_ASSERT_EQUAL(awt::FontWeight::BOLD,
                         getProperty<float>(getParagraph(3), u"CharWeight"_ustr));
    CPPUNIT_ASSERT_EQUAL(
        awt::FontWeight::NORMAL,
        getProperty<float>(getRun(getParagraph(4), 1, u" test"_ustr), u"CharWeight"_ustr));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testPlaceholderHTMLPasteStyleOverride)
{
    {
        createSwDoc();

        SwWrtShell* const pWrtShell = getSwDocShell()->GetWrtShell();
        pWrtShell->Insert(u"AAA"_ustr);
        pWrtShell->SplitNode();
        pWrtShell->Insert(u"BBB"_ustr);

        dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});
        dispatchCommand(mxComponent, u".uno:Copy"_ustr, {});
    }

    createSwDoc("placeholder-bold-style-override.fodt");

    // select placeholder field
    SwWrtShell* const pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/true, 15, /*bBasicCall=*/false);

    // Paste special as HTML
    uno::Sequence<beans::PropertyValue> aPropertyValues = comphelper::InitPropertySequence(
        { { "SelectedFormat", uno::Any(static_cast<sal_uInt32>(SotClipboardFormatId::HTML)) } });

    dispatchCommand(mxComponent, u".uno:ClipboardFormatItems"_ustr, aPropertyValues);

    // style sets it to bold
    uno::Reference<style::XStyleFamiliesSupplier> xSFS(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XNameContainer> xStyles(
        xSFS->getStyleFamilies()->getByName(u"ParagraphStyles"_ustr), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xStandard(xStyles->getByName("Standard"), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xTextbody(xStyles->getByName("Text body"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(awt::FontWeight::BOLD, getProperty<float>(xStandard, u"CharWeight"_ustr));
    CPPUNIT_ASSERT_EQUAL(awt::FontWeight::BOLD, getProperty<float>(xTextbody, u"CharWeight"_ustr));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(style::ParagraphAdjust_CENTER),
                         getProperty<sal_Int16>(xTextbody, u"ParaAdjust"_ustr));

    CPPUNIT_ASSERT_EQUAL(int(3), getParagraphs());

    // but hard formatting overrides to normal
    CPPUNIT_ASSERT_EQUAL(
        awt::FontWeight::NORMAL,
        getProperty<float>(getRun(getParagraph(1), 1, u"AAA"_ustr), u"CharWeight"_ustr));
    CPPUNIT_ASSERT_EQUAL(
        awt::FontWeight::NORMAL,
        getProperty<float>(getRun(getParagraph(2), 1, u"BBB"_ustr), u"CharWeight"_ustr));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(style::ParagraphAdjust_LEFT),
                         getProperty<sal_Int16>(getParagraph(1), u"ParaAdjust"_ustr));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(style::ParagraphAdjust_LEFT),
                         getProperty<sal_Int16>(getParagraph(2), u"ParaAdjust"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"Text body"_ustr,
                         getProperty<OUString>(getParagraph(1), u"ParaStyleName"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"Text body"_ustr,
                         getProperty<OUString>(getParagraph(2), u"ParaStyleName"_ustr));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf151974)
{
    createSwDoc("tdf151974.odt");

    CPPUNIT_ASSERT_EQUAL(int(8), getParagraphs());

    auto pLayout = parseLayoutDump();
    for (size_t i = 1; i < 9; ++i)
    {
        OString sPath("/root/page[1]/body/txt[" + OString::number(i)
                      + "]/SwParaPortion/SwLineLayout");
        assertXPathChildren(pLayout, sPath, 1);
    }

    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});
    dispatchCommand(mxComponent, u".uno:Copy"_ustr, {});
    dispatchCommand(mxComponent, u".uno:GoDown"_ustr, {});

    SwXTextDocument* pTextDoc = getSwTextDoc();
    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_RETURN);
    Scheduler::ProcessEventsToIdle();

    // Paste special as RTF
    uno::Sequence<beans::PropertyValue> aPropertyValues = comphelper::InitPropertySequence(
        { { "SelectedFormat", uno::Any(static_cast<sal_uInt32>(SotClipboardFormatId::RTF)) } });

    dispatchCommand(mxComponent, u".uno:ClipboardFormatItems"_ustr, aPropertyValues);

    CPPUNIT_ASSERT_EQUAL(int(16), getParagraphs());

    pLayout = parseLayoutDump();
    for (size_t i = 1; i < 16; ++i)
    {
        OString sPath("/root/page[1]/body/txt[" + OString::number(i)
                      + "]/SwParaPortion/SwLineLayout");

        // Without the fix in place, this test would have failed with
        // - Expected: 1
        // - Actual  : 2
        // - In <>, XPath '/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout' number of child-nodes is incorrect
        assertXPathChildren(pLayout, sPath, 1);
    }
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf155685)
{
    createSwDoc("tdf155685.docx");

    CPPUNIT_ASSERT_EQUAL(8, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());

    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});

    dispatchCommand(mxComponent, u".uno:Cut"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(0, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());

    // Without the fix in place, this test would have crashed here
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(8, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf157131)
{
    createSwDoc("tdf157131.docx");

    CPPUNIT_ASSERT_EQUAL(9, getShapes());
    CPPUNIT_ASSERT_EQUAL(6, getPages());

    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});

    dispatchCommand(mxComponent, u".uno:Copy"_ustr, {});

    dispatchCommand(mxComponent, u".uno:Paste"_ustr, {});
    dispatchCommand(mxComponent, u".uno:Paste"_ustr, {});
    dispatchCommand(mxComponent, u".uno:Paste"_ustr, {});

    CPPUNIT_ASSERT_GREATER(9, getShapes());
    CPPUNIT_ASSERT_GREATER(6, getPages());

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    // Without the fix in place, this test would have crashed here
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(9, getShapes());
    CPPUNIT_ASSERT_EQUAL(6, getPages());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf145731)
{
    createSwDoc("tdf145731.odt");

    CPPUNIT_ASSERT_EQUAL(9, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());

    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});

    dispatchCommand(mxComponent, u".uno:Cut"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(0, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());

    // Without the fix in place, this test would have crashed here
    dispatchCommand(mxComponent, u".uno:Paste"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(9, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf147199)
{
    createSwDoc("tdf147199.docx");

    CPPUNIT_ASSERT_EQUAL(7, getShapes());

    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});

    dispatchCommand(mxComponent, u".uno:Cut"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(0, getShapes());

    dispatchCommand(mxComponent, u".uno:Paste"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(7, getShapes());

    dispatchCommand(mxComponent, u".uno:Paste"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(14, getShapes());

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(7, getShapes());

    // Without the fix in place, this test would have crashed here
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(0, getShapes());

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(7, getShapes());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf139843)
{
    createSwDoc("tdf139843.odt");

    int nPages = getPages();

    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});

    dispatchCommand(mxComponent, u".uno:Cut"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(1, getPages());

    // Without the fix in place, this test would have crashed here
    dispatchCommand(mxComponent, u".uno:Paste"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(nPages, getPages());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf143574)
{
    createSwDoc("tdf143574.odt");

    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    uno::Reference<drawing::XShapes> xGroupShape(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xGroupShape->getCount());

    uno::Reference<beans::XPropertySet> xProperties(xGroupShape->getByIndex(2), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(false, xProperties->getPropertyValue(u"TextBox"_ustr).get<bool>());

    selectShape(1);
    dispatchCommand(mxComponent, u".uno:EnterGroup"_ustr, {});

    // Select a shape in the group
    SwXTextDocument* pTextDoc = getSwTextDoc();
    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_TAB);
    Scheduler::ProcessEventsToIdle();

    // Without the fix in place, this test would have crashed
    dispatchCommand(mxComponent, u".uno:AddTextBox"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(true, xProperties->getPropertyValue(u"TextBox"_ustr).get<bool>());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf146848)
{
    // Reuse existing document
    createSwDoc("tdf77014.odt");

    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});

    dispatchCommand(mxComponent, u".uno:Cut"_ustr, {});

    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(
        xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());
    CPPUNIT_ASSERT(!xFields->hasMoreElements());

    // Without the fix in place, this test would have crashed here
    dispatchCommand(mxComponent, u".uno:Paste"_ustr, {});

    xFields = xFieldsAccess->createEnumeration();

    int nFieldsCount = 0;
    while (xFields->hasMoreElements())
    {
        xFields->nextElement();
        nFieldsCount++;
    }
    CPPUNIT_ASSERT_EQUAL(4, nFieldsCount);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf149507)
{
    createSwDoc("tdf149507.docx");

    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());

    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});

    dispatchCommand(mxComponent, u".uno:Cut"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(0, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());

    dispatchCommand(mxComponent, u".uno:Paste"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());

    // Without the fix in place, this test would have crashed here
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(0, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf114973)
{
    createSwDoc("tdf114973.fodt");

    SwWrtShell* const pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->SttEndDoc(true);

    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});

    // bug: cursor jumped into header
    CPPUNIT_ASSERT(!pWrtShell->IsInHeaderFooter());

    dispatchCommand(mxComponent, u".uno:Copy"_ustr, {});

    // check that hidden paragraphs at start and end are deleted
    dispatchCommand(mxComponent, u".uno:Delete"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(int(1), getParagraphs());
    CPPUNIT_ASSERT_EQUAL(OUString(), getParagraph(1)->getString());

    // check that hidden paragraphs at start and end are copied
    dispatchCommand(mxComponent, u".uno:Paste"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(int(3), getParagraphs());
    CPPUNIT_ASSERT_EQUAL(u"hidden first paragraph"_ustr, getParagraph(1)->getString());
    CPPUNIT_ASSERT_EQUAL(u"Press CTRL+A for dispatching .uno:SelectAll. You see that "
                         "nothing will be selected. The cursor jumps to the header"_ustr,
                         getParagraph(2)->getString());
    CPPUNIT_ASSERT_EQUAL(u"hidden last paragraph"_ustr, getParagraph(3)->getString());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf145321)
{
    createSwDoc("tdf145321.odt");

    CPPUNIT_ASSERT_EQUAL(3, getShapes());
    CPPUNIT_ASSERT_EQUAL(3, getPages());

    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});

    dispatchCommand(mxComponent, u".uno:Cut"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(0, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());

    // Without the fix in place, this test would have crashed here
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(3, getShapes());
    CPPUNIT_ASSERT_EQUAL(3, getPages());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testVariableFieldTableRowSplitHeader)
{
    createSwDoc("variable-field-table-row-split-header.fodt");
    SwDoc* const pDoc = getSwDoc();

    // finish layout
    Scheduler::ProcessEventsToIdle();
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // the fields in cell are: variable-get variable-set variable-get
    assertXPath(
        pXmlDoc,
        "/root/page[1]/body/tab/row[1]/cell[2]/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion",
        3);
    assertXPath(
        pXmlDoc,
        "/root/page[1]/body/tab/row[1]/cell[2]/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
        "expand", u"0");
    assertXPath(
        pXmlDoc,
        "/root/page[1]/body/tab/row[1]/cell[2]/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[2]",
        "expand", u"1");
    assertXPath(
        pXmlDoc,
        "/root/page[1]/body/tab/row[1]/cell[2]/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[3]",
        "expand", u"1");
    assertXPath(pXmlDoc, "/root/page[1]/header/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
                "expand", u"0");
    assertXPath(pXmlDoc, "/root/page[1]/footer/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
                "expand", u"1");
    // here the header had shown the wrong value
    assertXPath(
        pXmlDoc,
        "/root/page[2]/body/tab/row[2]/cell[2]/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
        "expand", u"1");
    assertXPath(
        pXmlDoc,
        "/root/page[2]/body/tab/row[2]/cell[2]/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[2]",
        "expand", u"2");
    assertXPath(
        pXmlDoc,
        "/root/page[2]/body/tab/row[2]/cell[2]/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[3]",
        "expand", u"2");
    assertXPath(pXmlDoc, "/root/page[2]/header/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
                "expand", u"1");
    assertXPath(pXmlDoc, "/root/page[2]/footer/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
                "expand", u"2");

    assertXPath(
        pXmlDoc,
        "/root/page[3]/body/tab/row[2]/cell[2]/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
        "expand", u"2");
    assertXPath(
        pXmlDoc,
        "/root/page[3]/body/tab/row[2]/cell[2]/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[2]",
        "expand", u"3");
    assertXPath(
        pXmlDoc,
        "/root/page[3]/body/tab/row[2]/cell[2]/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[3]",
        "expand", u"3");
    assertXPath(pXmlDoc, "/root/page[3]/header/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
                "expand", u"2");
    assertXPath(pXmlDoc, "/root/page[3]/footer/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
                "expand", u"3");

    assertXPath(
        pXmlDoc,
        "/root/page[4]/body/tab/row[2]/cell[2]/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
        "expand", u"3");
    assertXPath(
        pXmlDoc,
        "/root/page[4]/body/tab/row[2]/cell[2]/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[2]",
        "expand", u"4");
    assertXPath(
        pXmlDoc,
        "/root/page[4]/body/tab/row[2]/cell[2]/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[3]",
        "expand", u"4");
    assertXPath(pXmlDoc, "/root/page[4]/header/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
                "expand", u"3");
    assertXPath(pXmlDoc, "/root/page[4]/footer/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
                "expand", u"4");

    assertXPath(pXmlDoc, "/root/page[5]/header/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
                "expand", u"4");
    assertXPath(pXmlDoc, "/root/page[5]/footer/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
                "expand", u"4");

    // update and check again
    pDoc->getIDocumentFieldsAccess().UpdateFields(true);
    Scheduler::ProcessEventsToIdle();
    pXmlDoc = parseLayoutDump();

    // the fields in cell are: variable-get variable-set variable-get
    assertXPath(
        pXmlDoc,
        "/root/page[1]/body/tab/row[1]/cell[2]/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion",
        3);
    assertXPath(
        pXmlDoc,
        "/root/page[1]/body/tab/row[1]/cell[2]/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
        "expand", u"0");
    assertXPath(
        pXmlDoc,
        "/root/page[1]/body/tab/row[1]/cell[2]/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[2]",
        "expand", u"1");
    assertXPath(
        pXmlDoc,
        "/root/page[1]/body/tab/row[1]/cell[2]/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[3]",
        "expand", u"1");
    assertXPath(pXmlDoc, "/root/page[1]/header/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
                "expand", u"0");
    assertXPath(pXmlDoc, "/root/page[1]/footer/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
                "expand", u"1");
    // here the header had shown the wrong value
    assertXPath(
        pXmlDoc,
        "/root/page[2]/body/tab/row[2]/cell[2]/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
        "expand", u"1");
    assertXPath(
        pXmlDoc,
        "/root/page[2]/body/tab/row[2]/cell[2]/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[2]",
        "expand", u"2");
    assertXPath(
        pXmlDoc,
        "/root/page[2]/body/tab/row[2]/cell[2]/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[3]",
        "expand", u"2");
    assertXPath(pXmlDoc, "/root/page[2]/header/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
                "expand", u"1");
    assertXPath(pXmlDoc, "/root/page[2]/footer/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
                "expand", u"2");

    assertXPath(
        pXmlDoc,
        "/root/page[3]/body/tab/row[2]/cell[2]/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
        "expand", u"2");
    assertXPath(
        pXmlDoc,
        "/root/page[3]/body/tab/row[2]/cell[2]/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[2]",
        "expand", u"3");
    assertXPath(
        pXmlDoc,
        "/root/page[3]/body/tab/row[2]/cell[2]/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[3]",
        "expand", u"3");
    assertXPath(pXmlDoc, "/root/page[3]/header/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
                "expand", u"2");
    assertXPath(pXmlDoc, "/root/page[3]/footer/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
                "expand", u"3");

    assertXPath(
        pXmlDoc,
        "/root/page[4]/body/tab/row[2]/cell[2]/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
        "expand", u"3");
    assertXPath(
        pXmlDoc,
        "/root/page[4]/body/tab/row[2]/cell[2]/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[2]",
        "expand", u"4");
    assertXPath(
        pXmlDoc,
        "/root/page[4]/body/tab/row[2]/cell[2]/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[3]",
        "expand", u"4");
    assertXPath(pXmlDoc, "/root/page[4]/header/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
                "expand", u"3");
    assertXPath(pXmlDoc, "/root/page[4]/footer/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
                "expand", u"4");

    assertXPath(pXmlDoc, "/root/page[5]/header/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
                "expand", u"4");
    assertXPath(pXmlDoc, "/root/page[5]/footer/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
                "expand", u"4");
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf147126)
{
    createSwDoc("tdf147126.docx");

    xmlDocUniquePtr pLayoutXML1 = parseLayoutDump();

    for (auto nFly = 1; nFly < 8; ++nFly)
    {
        const auto nFlyLeft = getXPath(pLayoutXML1,
                                       "/root/page/body/txt[2]/anchored/fly["
                                           + OString::number(nFly) + "]/infos/bounds",
                                       "left")
                                  .toInt64();
        const auto nFlyRight = getXPath(pLayoutXML1,
                                        "/root/page/body/txt[2]/anchored/fly["
                                            + OString::number(nFly) + "]/infos/bounds",
                                        "width")
                                   .toInt64();
        const auto nFlyTop = getXPath(pLayoutXML1,
                                      "/root/page/body/txt[2]/anchored/fly[" + OString::number(nFly)
                                          + "]/infos/bounds",
                                      "top")
                                 .toInt64();
        const auto nFlyBottom = getXPath(pLayoutXML1,
                                         "/root/page/body/txt[2]/anchored/fly["
                                             + OString::number(nFly) + "]/infos/bounds",
                                         "height")
                                    .toInt64();

        const auto sDrawRect = getXPath(
            pLayoutXML1,
            "/root/page/body/txt[2]/anchored/SwAnchoredDrawObject/SdrObjGroup/SdrObjList/SdrObject["
                + OString::number(nFly) + "]",
            "aOutRect");

        const auto nComaPos1 = sDrawRect.indexOf(',', 0);
        const auto nComaPos2 = sDrawRect.indexOf(',', nComaPos1 + 1);
        const auto nComaPos3 = sDrawRect.indexOf(',', nComaPos2 + 1);

        const auto nDraw1 = o3tl::toInt64(sDrawRect.subView(0, nComaPos1));
        const auto nDraw2 = o3tl::toInt64(sDrawRect.subView(nComaPos1 + 1, nComaPos2 - nComaPos1));
        const auto nDraw3 = o3tl::toInt64(sDrawRect.subView(nComaPos2 + 1, nComaPos3 - nComaPos2));
        const auto nDraw4 = o3tl::toInt64(
            sDrawRect.subView(nComaPos3 + 1, sDrawRect.getLength() - nComaPos3 - 1));

        CPPUNIT_ASSERT_GREATER(nDraw1, nFlyLeft);
        CPPUNIT_ASSERT_GREATER(nDraw2, nFlyTop);
        CPPUNIT_ASSERT_LESS(nDraw3, nFlyRight);
        CPPUNIT_ASSERT_LESS(nDraw4, nFlyBottom);
    }

    SwXTextDocument* pTextDoc = getSwTextDoc();
    for (auto nLineBreakCount = 0; nLineBreakCount < 4; ++nLineBreakCount)
    {
        pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_RETURN);
        Scheduler::ProcessEventsToIdle();
    }
    for (auto nSpaceCount = 0; nSpaceCount < 10; ++nSpaceCount)
    {
        pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_SPACE);
        Scheduler::ProcessEventsToIdle();
    }

    xmlDocUniquePtr pLayoutXML2 = parseLayoutDump();

    for (auto nFly = 1; nFly < 8; ++nFly)
    {
        const auto nFlyLeft = getXPath(pLayoutXML2,
                                       "/root/page/body/txt[6]/anchored/fly["
                                           + OString::number(nFly) + "]/infos/bounds",
                                       "left")
                                  .toInt64();
        const auto nFlyRight = getXPath(pLayoutXML2,
                                        "/root/page/body/txt[6]/anchored/fly["
                                            + OString::number(nFly) + "]/infos/bounds",
                                        "width")
                                   .toInt64();
        const auto nFlyTop = getXPath(pLayoutXML2,
                                      "/root/page/body/txt[6]/anchored/fly[" + OString::number(nFly)
                                          + "]/infos/bounds",
                                      "top")
                                 .toInt64();
        const auto nFlyBottom = getXPath(pLayoutXML2,
                                         "/root/page/body/txt[6]/anchored/fly["
                                             + OString::number(nFly) + "]/infos/bounds",
                                         "height")
                                    .toInt64();

        const auto sDrawRect = getXPath(
            pLayoutXML2,
            "/root/page/body/txt[6]/anchored/SwAnchoredDrawObject/SdrObjGroup/SdrObjList/SdrObject["
                + OString::number(nFly) + "]",
            "aOutRect");

        const auto nComaPos1 = sDrawRect.indexOf(',', 0);
        const auto nComaPos2 = sDrawRect.indexOf(',', nComaPos1 + 1);
        const auto nComaPos3 = sDrawRect.indexOf(',', nComaPos2 + 1);

        const auto nDraw1 = o3tl::toInt64(sDrawRect.subView(0, nComaPos1));
        const auto nDraw2 = o3tl::toInt64(sDrawRect.subView(nComaPos1 + 1, nComaPos2 - nComaPos1));
        const auto nDraw3 = o3tl::toInt64(sDrawRect.subView(nComaPos2 + 1, nComaPos3 - nComaPos2));
        const auto nDraw4 = o3tl::toInt64(
            sDrawRect.subView(nComaPos3 + 1, sDrawRect.getLength() - nComaPos3 - 1));

        CPPUNIT_ASSERT_GREATER(nDraw1, nFlyLeft);
        CPPUNIT_ASSERT_GREATER(nDraw2, nFlyTop);
        CPPUNIT_ASSERT_LESS(nDraw3, nFlyRight);
        CPPUNIT_ASSERT_LESS(nDraw4, nFlyBottom);
    }
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf148868)
{
    createSwDoc("tdf148868.odt");
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();

    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());

    pWrtShell->EndPg(/*bSelect=*/false);
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/true, 5, /*bBasicCall=*/false);
    pWrtShell->Insert(u"X"_ustr);

    // Without the fix in place, this test would have failed with
    // - Expected: 1
    // - Actual  : 0
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf129382)
{
    createSwDoc("tdf129382.docx");

    CPPUNIT_ASSERT_EQUAL(8, getShapes());
    CPPUNIT_ASSERT_EQUAL(2, getPages());

    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});

    dispatchCommand(mxComponent, u".uno:Cut"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(3, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());

    dispatchCommand(mxComponent, u".uno:Paste"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(8, getShapes());
    CPPUNIT_ASSERT_EQUAL(2, getPages());
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(3, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());

    // without the fix, it crashes
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(8, getShapes());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf135662)
{
    createSwDoc("tdf135662.odt");

    CPPUNIT_ASSERT_EQUAL(2, getShapes());

    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});

    dispatchCommand(mxComponent, u".uno:Cut"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(0, getShapes());

    dispatchCommand(mxComponent, u".uno:Paste"_ustr, {});

    // Without the fix in place, this test would have failed with
    // - Expected: 2
    // - Actual  : 1
    CPPUNIT_ASSERT_EQUAL(2, getShapes());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf134227)
{
    createSwDoc("tdf134227.docx");

    CPPUNIT_ASSERT_EQUAL(4, getShapes());

    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});
    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});

    // Without the fix in place, it would have crashed here
    dispatchCommand(mxComponent, u".uno:Cut"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(0, getShapes());

    dispatchCommand(mxComponent, u".uno:Paste"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(4, getShapes());

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(0, getShapes());

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(4, getShapes());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf139638)
{
    createSwDoc("tdf139638.odt");

    uno::Reference<text::XDocumentIndexesSupplier> xIndexSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexes = xIndexSupplier->getDocumentIndexes();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexes->getCount());
    uno::Reference<text::XDocumentIndex> xTOCIndex(xIndexes->getByIndex(0), uno::UNO_QUERY);

    // Without the fix in place, this test would have crashed
    xTOCIndex->update();
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf114773)
{
    createSwDoc("tdf114773.odt");

    uno::Reference<text::XDocumentIndexesSupplier> xIndexSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexes = xIndexSupplier->getDocumentIndexes();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexes->getCount());
    uno::Reference<text::XDocumentIndex> xTOCIndex(xIndexes->getByIndex(0), uno::UNO_QUERY);

    xTOCIndex->update();
    uno::Reference<text::XTextRange> xTextRange = xTOCIndex->getAnchor();
    uno::Reference<text::XText> xText = xTextRange->getText();
    uno::Reference<text::XTextCursor> xTextCursor = xText->createTextCursor();
    xTextCursor->gotoRange(xTextRange->getStart(), false);
    xTextCursor->gotoRange(xTextRange->getEnd(), true);
    OUString aIndexString(convertLineEnd(xTextCursor->getString(), LineEnd::LINEEND_LF));

    // Without the fix in place, this test would have failed with
    // - Expected: 2  Heading Level 1   2-1
    // - Actual  : 2   Heading Level 1  2 -1
    CPPUNIT_ASSERT_EQUAL(u"Full ToC\n"
                         "2  Heading Level 1\t2-1\n"
                         "2.1  Heading Level 2\t2-2\n"
                         "2.1.1  Heading Level 3\t2-2\n"
                         "2.1.1.1  Heading Level 4\t2-2"_ustr,
                         aIndexString);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf104315)
{
    createSwDoc("tdf104315.odt");

    uno::Reference<text::XDocumentIndexesSupplier> xIndexSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexes = xIndexSupplier->getDocumentIndexes();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexes->getCount());
    uno::Reference<text::XDocumentIndex> xTOCIndex(xIndexes->getByIndex(0), uno::UNO_QUERY);

    xTOCIndex->update();
    uno::Reference<text::XTextRange> xTextRange = xTOCIndex->getAnchor();
    uno::Reference<text::XText> xText = xTextRange->getText();
    uno::Reference<text::XTextCursor> xTextCursor = xText->createTextCursor();
    xTextCursor->gotoRange(xTextRange->getStart(), false);
    xTextCursor->gotoRange(xTextRange->getEnd(), true);
    OUString aIndexString(convertLineEnd(xTextCursor->getString(), LineEnd::LINEEND_LF));

    // Without the fix in place, this test would have failed with
    // - Expected: This is a headlinex  1
    // - Actual  : This is a headlinx   1
    CPPUNIT_ASSERT_EQUAL(u"Table of contents\n"
                         "This is a headlinex\t1\n"
                         "This is another headlinex\t1"_ustr,
                         aIndexString);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf135412)
{
    createSwDoc("tdf135412.docx");

    CPPUNIT_ASSERT_EQUAL(4, getShapes());
    uno::Reference<text::XTextRange> xShape(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"X"_ustr, xShape->getString());

    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});

    dispatchCommand(mxComponent, u".uno:Cut"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(0, getShapes());

    dispatchCommand(mxComponent, u".uno:Paste"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(4, getShapes());

    // Without the fix in place, the text in the shape wouldn't be pasted
    xShape.set(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"X"_ustr, xShape->getString());

    //Without the fix in place, it would have crashed here
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(0, getShapes());
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(4, getShapes());
    xShape.set(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"X"_ustr, xShape->getString());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf138482)
{
    createSwDoc("tdf138482.docx");

    CPPUNIT_ASSERT_EQUAL(2, getShapes());

    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});

    // Without the fix in place, this test would have crashed here
    dispatchCommand(mxComponent, u".uno:Delete"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(0, getShapes());

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(2, getShapes());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf134965)
{
    createSwDoc("tdf134965.odt");

    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(),
                                                         uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xIndexAccess->getCount());

    for (sal_Int32 i = 0; i < 10; ++i)
    {
        dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});

        dispatchCommand(mxComponent, u".uno:Cut"_ustr, {});

        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xIndexAccess->getCount());

        // Without the fix in place, this test would have crashed here
        dispatchCommand(mxComponent, u".uno:Paste"_ustr, {});

        CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xIndexAccess->getCount());
    }
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf128375)
{
    for (sal_Int32 i = 0; i < 2; ++i)
    {
        createSwDoc("tdf128375.docx");

        uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(),
                                                             uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());

        dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});

        // Without the fix in place, this test would have crashed cutting the second document
        dispatchCommand(mxComponent, u".uno:Cut"_ustr, {});

        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xIndexAccess->getCount());
    }
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf135061)
{
    createSwDoc("tdf135061.odt");

    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(),
                                                         uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(4, getShapes());

    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});

    dispatchCommand(mxComponent, u".uno:Copy"_ustr, {});

    for (sal_Int32 i = 0; i < 5; ++i)
    {
        dispatchCommand(mxComponent, u".uno:Paste"_ustr, {});
    }

    CPPUNIT_ASSERT_EQUAL(sal_Int32(6), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(20, getShapes());

    for (sal_Int32 i = 0; i < 5; ++i)
    {
        // Without the fix in place, this test would have crashed here
        dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    }

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(4, getShapes());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf132911)
{
    createSwDoc("tdf132911.odt");

    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(),
                                                         uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(4, getShapes());

    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});

    dispatchCommand(mxComponent, u".uno:Cut"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(0, getShapes());

    dispatchCommand(mxComponent, u".uno:Paste"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(4, getShapes());

    // Without the fix in place, it would have crashed here
    dispatchCommand(mxComponent, u".uno:Paste"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(8, getShapes());

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(4, getShapes());

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(0, getShapes());

    dispatchCommand(mxComponent, u".uno:Paste"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(4, getShapes());

    dispatchCommand(mxComponent, u".uno:Paste"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(8, getShapes());

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(4, getShapes());

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(0, getShapes());

    //tdf#135247: Without the fix in place, this would have crashed
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(4, getShapes());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf61154)
{
    createSwDoc("tdf61154.fodt");
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();

    pWrtShell->GotoNextTOXBase();

    // show changes
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::ShowDelete
                                                      | RedlineFlags::ShowInsert);
    CPPUNIT_ASSERT_MESSAGE("redlining should be off",
                           !pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    const SwTOXBase* pTOXBase = pWrtShell->GetCurTOX();
    pWrtShell->UpdateTableOf(*pTOXBase);
    SwCursorShell* pShell(pDoc->GetEditShell());
    CPPUNIT_ASSERT(pShell);
    SwTextNode* pTitleNode = pShell->GetCursor()->GetPointNode().GetTextNode();
    SwNodeIndex aIdx(*pTitleNode);

    // table of contents node shouldn't contain tracked deletion
    // This was "Text InsertedDeleted\t1"
    SwTextNode* pNext = static_cast<SwTextNode*>(SwNodes::GoNext(&aIdx));
    CPPUNIT_ASSERT_EQUAL(u"Text Inserted\t1"_ustr, pNext->GetText());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf124904)
{
    // don't show deletions in referenced text,
    // (except if the full text is deleted)
    createSwDoc("tdf124904.fodt");
    SwDoc* pDoc = getSwDoc();

    // show changes
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowDelete
                                                      | RedlineFlags::ShowInsert);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    // remove the first word "Heading" (with change tracking) to update the referenced text
    dispatchCommand(mxComponent, u".uno:DelToEndOfWord"_ustr, {});

    // This was "Reference to Heading of document file"
    CPPUNIT_ASSERT_EQUAL(u"Reference to of example document "_ustr, getParagraph(2)->getString());

    // don't hide the wholly deleted referenced text
    dispatchCommand(mxComponent, u".uno:DelToEndOfLine"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(u"Reference to Heading of example document file"_ustr,
                         getParagraph(2)->getString());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf100691)
{
    createSwDoc("tdf100691.fodt");
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();

    pWrtShell->GotoNextTOXBase();

    const SwTOXBase* pTOXBase = pWrtShell->GetCurTOX();
    pWrtShell->UpdateTableOf(*pTOXBase);
    SwCursorShell* pShell(pDoc->GetEditShell());
    CPPUNIT_ASSERT(pShell);
    SwTextNode* pTitleNode = pShell->GetCursor()->GetPointNode().GetTextNode();
    SwNodeIndex aIdx(*pTitleNode);

    // table of contents node shouldn't contain invisible text
    // This was "Text Hidden\t1"
    SwTextNode* pNext = static_cast<SwTextNode*>(SwNodes::GoNext(&aIdx));
    CPPUNIT_ASSERT_EQUAL(u"Text\t1"_ustr, pNext->GetText());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf164620)
{
    createSwDoc("tdf164620.docx");

    CPPUNIT_ASSERT_EQUAL(2, getPages());

    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});

    // Without the fix in place, this test would have crashed here
    dispatchCommand(mxComponent, u".uno:Delete"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(1, getPages());

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf134404)
{
    createSwDoc("tdf134404.odt");

    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());

    dispatchCommand(mxComponent, u".uno:GoToEndOfPage"_ustr, {});
    dispatchCommand(mxComponent, u".uno:InsertPagebreak"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(2, getPages());

    dispatchCommand(mxComponent, u".uno:SwBackspace"_ustr, {});

    // Without the fix in place, the image would have been deleted
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf112342)
{
    createSwDoc("tdf112342.docx");

    //Get the last para
    uno::Reference<text::XTextRange> xPara = getParagraph(3);
    auto xCur = xPara->getText()->createTextCursor();
    //Go to the end of it
    xCur->gotoEnd(false);
    //And let's remove the last 2 chars (the last para with its char).
    xCur->goLeft(2, true);
    xCur->setString(u""_ustr);

    //If the second paragraph on the second page, this will be passed.
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Page break does not match", 2, getPages());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf132321)
{
    createSwDoc("tdf132321.odt");

    CPPUNIT_ASSERT_EQUAL(1, getShapes());

    dispatchCommand(mxComponent, u".uno:GoToEndOfPage"_ustr, {});
    dispatchCommand(mxComponent, u".uno:SwBackspace"_ustr, {});

    // Without the fix in place, the button form would have also been deleted
    CPPUNIT_ASSERT_EQUAL(1, getShapes());

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf135056)
{
    createSwDoc("tdf135056.odt");
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();

    CPPUNIT_ASSERT_EQUAL(sal_uInt16(1), pWrtShell->GetTOXCount());

    const SwTOXBase* pTOX = pWrtShell->GetTOX(0);
    CPPUNIT_ASSERT(pTOX);

    //Without the fix in place, it would have hung here
    pWrtShell->DeleteTOX(*pTOX, true);

    CPPUNIT_ASSERT_EQUAL(sal_uInt16(0), pWrtShell->GetTOXCount());

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(sal_uInt16(1), pWrtShell->GetTOXCount());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf132597)
{
    createSwDoc("tdf132597.odt");

    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());

    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});

    dispatchCommand(mxComponent, u".uno:Copy"_ustr, {});

    // Paste special as RTF
    uno::Sequence<beans::PropertyValue> aPropertyValues = comphelper::InitPropertySequence(
        { { "SelectedFormat", uno::Any(static_cast<sal_uInt32>(SotClipboardFormatId::RTF)) } });

    dispatchCommand(mxComponent, u".uno:ClipboardFormatItems"_ustr, aPropertyValues);

    // Without the fix in place, this test would have crashed here
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf139737)
{
    createSwDoc("tdf139737.fodt");

    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});

    dispatchCommand(mxComponent, u".uno:Copy"_ustr, {});

    // Paste special as RTF
    uno::Sequence<beans::PropertyValue> aPropertyValues = comphelper::InitPropertySequence(
        { { "SelectedFormat", uno::Any(static_cast<sal_uInt32>(SotClipboardFormatId::RTF)) } });

    dispatchCommand(mxComponent, u".uno:ClipboardFormatItems"_ustr, aPropertyValues);

    // Without the fix in place, this test would have crashed here
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf150845)
{
    createSwDoc();

    // Insert text box with ctrl key
    // Without the fix in place, this test would have crashed here
    uno::Sequence<beans::PropertyValue> aArgs(
        comphelper::InitPropertySequence({ { "KeyModifier", uno::Any(KEY_MOD1) } }));
    dispatchCommand(mxComponent, u".uno:Edit"_ustr, aArgs);

    CPPUNIT_ASSERT_EQUAL(1, getShapes());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf147961)
{
    createSwDoc();
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();

    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(),
                                                         uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xIndexAccess->getCount());

    pWrtShell->Insert(u"++"_ustr);

    // Without the fix in place, this test would have crashed here
    SwXTextDocument* pTextDoc = getSwTextDoc();
    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_RETURN);
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf147206)
{
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();

    // insert empty paragraph and heading text
    pWrtShell->SplitNode();
    pWrtShell->Insert(u"abc"_ustr);
    pWrtShell->SplitNode();

    // set one to heading so there will be an entry in the tox
    pWrtShell->Up(false, 1);
    uno::Sequence<beans::PropertyValue> aPropertyValues = comphelper::InitPropertySequence({
        { "Style", uno::Any(u"Heading 1"_ustr) },
        { "FamilyName", uno::Any(u"ParagraphStyles"_ustr) },
    });
    dispatchCommand(mxComponent, u".uno:StyleApply"_ustr, aPropertyValues);

    pWrtShell->EndOfSection(false);

    // tdf#142423: Without the fix in place, this test would have failed with
    // - Expected: 18
    // - Actual  : 18.2
    CPPUNIT_ASSERT_EQUAL(18.0f, getProperty<float>(getParagraph(2), u"CharHeight"_ustr));

    // insert table of contents
    SwTOXMgr mgr(pWrtShell);
    SwTOXDescription desc{ TOX_CONTENT };
    mgr.UpdateOrInsertTOX(desc, nullptr, nullptr);

    // get url of heading cross reference mark
    IDocumentMarkAccess& rIDMA(*pDoc->getIDocumentMarkAccess());
    auto const headingMark
        = std::find_if(rIDMA.getAllMarksBegin(), rIDMA.getAllMarksEnd(), [](auto const* const it) {
              return it->GetName().startsWith(
                  IDocumentMarkAccess::GetCrossRefHeadingBookmarkNamePrefix());
          });
    CPPUNIT_ASSERT(headingMark != rIDMA.getAllMarksEnd());
    OUString const headingLink("#" + (*headingMark)->GetName());

    // select tox entry
    pWrtShell->SttEndDoc(false);
    pWrtShell->Up(false, 1);
    pWrtShell->EndPara(true);

    rtl::Reference<SwTransferable> xTransfer = new SwTransferable(*pWrtShell);
    xTransfer->Copy();

    pWrtShell->SttEndDoc(true);

    // Paste special as RTF
    TransferableDataHelper helper(xTransfer);
    SwTransferable::PasteFormat(*pWrtShell, helper, SotClipboardFormatId::RTF);
    Scheduler::ProcessEventsToIdle();

    // check hyperlinking
    CPPUNIT_ASSERT_EQUAL(
        headingLink,
        getProperty<OUString>(getRun(getParagraph(1), 1, u"abc\t1"_ustr), u"HyperLinkURL"_ustr));
    CPPUNIT_ASSERT_EQUAL(OUString(), getProperty<OUString>(getRun(getParagraph(2), 1, OUString()),
                                                           u"HyperLinkURL"_ustr));
    CPPUNIT_ASSERT_EQUAL(
        OUString(), getProperty<OUString>(getRun(getParagraph(3), 1, u"Table of Contents"_ustr),
                                          u"HyperLinkURL"_ustr));
    CPPUNIT_ASSERT_EQUAL(
        headingLink,
        getProperty<OUString>(getRun(getParagraph(4), 1, u"abc\t1"_ustr), u"HyperLinkURL"_ustr));
    CPPUNIT_ASSERT_EQUAL(OUString(), getProperty<OUString>(getRun(getParagraph(5), 1, OUString()),
                                                           u"HyperLinkURL"_ustr));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf48459)
{
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    // insert paragraph text
    pWrtShell->Insert(u"Heading and normal text"_ustr);

    // select the first word (proposed for inline heading)
    pWrtShell->SttEndDoc(/*bStart=*/true);
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/true, 7, /*bBasicCall=*/false);

    // apply styles only on the selected word -> create inline heading
    uno::Sequence<beans::PropertyValue> aPropertyValues = comphelper::InitPropertySequence({
        { "Style", uno::Any(u"Heading 1"_ustr) },
        { "FamilyName", uno::Any(u"ParagraphStyles"_ustr) },
    });
    dispatchCommand(mxComponent, u".uno:StyleApply"_ustr, aPropertyValues);

    uno::Reference<frame::XFrames> xFrames = mxDesktop->getFrames();

    // inline heading frame
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xFrames->getCount());

    pWrtShell->EndOfSection(false);

    // insert table of contents to check ToC content (containing only the inline heading)
    SwTOXMgr mgr(pWrtShell);
    SwTOXDescription desc{ TOX_CONTENT };
    mgr.UpdateOrInsertTOX(desc, nullptr, nullptr);

    CPPUNIT_ASSERT_EQUAL(int(3), getParagraphs());

    // first paragraph: selected text moved to the inline heading frame
    CPPUNIT_ASSERT_EQUAL(u" and normal text"_ustr, getParagraph(1)->getString());

    // ToC title
    CPPUNIT_ASSERT_EQUAL(u"Table of Contents"_ustr, getParagraph(2)->getString());

    // ToC contains only the inline heading
    CPPUNIT_ASSERT_EQUAL(u"Heading\t1"_ustr, getParagraph(3)->getString());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf144840)
{
    createSwDoc("tdf144840.odt");

    CPPUNIT_ASSERT_EQUAL(1, getPages());
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(),
                                                         uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());

    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});

    dispatchCommand(mxComponent, u".uno:Cut"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xIndexAccess->getCount());

    // Paste special as RTF
    uno::Sequence<beans::PropertyValue> aPropertyValues = comphelper::InitPropertySequence(
        { { "SelectedFormat", uno::Any(static_cast<sal_uInt32>(SotClipboardFormatId::RTF)) } });

    dispatchCommand(mxComponent, u".uno:ClipboardFormatItems"_ustr, aPropertyValues);

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());

    uno::Reference<text::XTextTable> xTextTable(xIndexAccess->getByIndex(0), uno::UNO_QUERY);

    table::TableBorder2 tableBorder
        = getProperty<table::TableBorder2>(xTextTable, u"TableBorder2"_ustr);

    // Without the fix in place, this test would have failed with
    // - Expected: 4
    // - Actual  : 0
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(4), tableBorder.TopLine.LineWidth);
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(4), tableBorder.LeftLine.LineWidth);
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(4), tableBorder.RightLine.LineWidth);
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(4), tableBorder.BottomLine.LineWidth);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf131963)
{
    createSwDoc("tdf131963.docx");

    CPPUNIT_ASSERT(11 == getPages() || 12 == getPages());

    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});

    dispatchCommand(mxComponent, u".uno:Copy"_ustr, {});

    // Paste special as RTF
    uno::Sequence<beans::PropertyValue> aPropertyValues = comphelper::InitPropertySequence(
        { { "SelectedFormat", uno::Any(static_cast<sal_uInt32>(SotClipboardFormatId::RTF)) } });

    dispatchCommand(mxComponent, u".uno:ClipboardFormatItems"_ustr, aPropertyValues);

    // Without the fix in place, this test would have crashed here

    // tdf#133169: without the fix in place, it would have been 2 instead of 11
    CPPUNIT_ASSERT(11 == getPages() || 12 == getPages());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf132596)
{
    createSwDoc("tdf132596.docx");

    CPPUNIT_ASSERT_EQUAL(2, getPages());

    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});

    dispatchCommand(mxComponent, u".uno:Copy"_ustr, {});

    // Paste special as RTF
    uno::Sequence<beans::PropertyValue> aPropertyValues = comphelper::InitPropertySequence(
        { { "SelectedFormat", uno::Any(static_cast<sal_uInt32>(SotClipboardFormatId::RTF)) } });

    dispatchCommand(mxComponent, u".uno:ClipboardFormatItems"_ustr, aPropertyValues);

    // Without the fix in place, this test would have crashed here
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf126626)
{
    createSwDoc("tdf126626.docx");

    CPPUNIT_ASSERT_EQUAL(2, getShapes());

    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});

    dispatchCommand(mxComponent, u".uno:Copy"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(2, getShapes());

    dispatchCommand(mxComponent, u".uno:Paste"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(2, getShapes());

    dispatchCommand(mxComponent, u".uno:Paste"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(4, getShapes());

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(2, getShapes());

    // without the fix, it crashes
    dispatchCommand(mxComponent, u".uno:Paste"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(4, getShapes());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf133967)
{
    createSwDoc("tdf133967.odt");

    CPPUNIT_ASSERT_EQUAL(6, getPages());

    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});

    dispatchCommand(mxComponent, u".uno:Cut"_ustr, {});

    for (sal_Int32 i = 0; i < 10; ++i)
    {
        dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

        dispatchCommand(mxComponent, u".uno:Redo"_ustr, {});
    }

    // Without the fix in place, this test would have failed with:
    //- Expected: 1
    //- Actual  : 45
    CPPUNIT_ASSERT_EQUAL(1, getPages());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf132187)
{
    createSwDoc("tdf132187.odt");

    CPPUNIT_ASSERT_EQUAL(1, getPages());

    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});

    dispatchCommand(mxComponent, u".uno:Copy"_ustr, {});

    dispatchCommand(mxComponent, u".uno:GoToEndOfDoc"_ustr, {});

    for (sal_Int32 i = 0; i < 10; ++i)
    {
        dispatchCommand(mxComponent, u".uno:Paste"_ustr, {});
    }

    //without the fix in place, this test would fail with:
    //- Expected: 1
    //- Actual  : 70

    CPPUNIT_ASSERT_EQUAL(1, getPages());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf130094)
{
    createSwDoc("tdf130094.fodt");
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();

    CPPUNIT_ASSERT_EQUAL(u"First"_ustr, getParagraph(1)->getString());
    CPPUNIT_ASSERT_EQUAL(u"Second"_ustr, getParagraph(2)->getString());
    CPPUNIT_ASSERT_EQUAL(u"Third"_ustr, getParagraph(3)->getString());

    // Select the first and second lines
    pWrtShell->Down(/*bSelect=*/true);
    pWrtShell->Down(/*bSelect=*/true);

    dispatchCommand(mxComponent, u".uno:Copy"_ustr, {});

    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});

    dispatchCommand(mxComponent, u".uno:Paste"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(u"First"_ustr, getParagraph(1)->getString());
    CPPUNIT_ASSERT_EQUAL(u"Second"_ustr, getParagraph(2)->getString());
    CPPUNIT_ASSERT_EQUAL(u""_ustr, getParagraph(3)->getString());

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(u"First"_ustr, getParagraph(1)->getString());
    CPPUNIT_ASSERT_EQUAL(u"Second"_ustr, getParagraph(2)->getString());
    CPPUNIT_ASSERT_EQUAL(u"Third"_ustr, getParagraph(3)->getString());

    dispatchCommand(mxComponent, u".uno:Paste"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(u"First"_ustr, getParagraph(1)->getString());
    CPPUNIT_ASSERT_EQUAL(u"Second"_ustr, getParagraph(2)->getString());

    // Without the fix in place, this test would have failed with
    // - Expected:
    // - Actual  : First
    CPPUNIT_ASSERT_EQUAL(u""_ustr, getParagraph(3)->getString());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf135733)
{
    createSwDoc("tdf135733.odt");
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();

    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(),
                                                         uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    CPPUNIT_ASSERT_EQUAL(2, getShapes());

    //Move the cursor inside the table
    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->Down(/*bSelect=*/false);

    //Select first column
    pWrtShell->Down(/*bSelect=*/true);
    pWrtShell->Down(/*bSelect=*/true);
    pWrtShell->Down(/*bSelect=*/true);

    dispatchCommand(mxComponent, u".uno:Cut"_ustr, {});

    pWrtShell->SttPg(/*bSelect=*/false);

    dispatchCommand(mxComponent, u".uno:Paste"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xIndexAccess->getCount());
    // the table is inserted before the first paragraph, which has a pagedesc
    CPPUNIT_ASSERT_EQUAL(2, getPages());
    CPPUNIT_ASSERT_EQUAL(2, getShapes());

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    CPPUNIT_ASSERT_EQUAL(0, getShapes());

    // without the fix, it crashes
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    CPPUNIT_ASSERT_EQUAL(2, getShapes());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf128739)
{
    createSwDoc("tdf128739.docx");

    CPPUNIT_ASSERT_EQUAL(u"Fehler: Verweis nicht gefunden"_ustr, getParagraph(1)->getString());

    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});

    dispatchCommand(mxComponent, u".uno:Cut"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(u""_ustr, getParagraph(1)->getString());

    dispatchCommand(mxComponent, u".uno:Paste"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(u"Fehler: Verweis nicht gefunden"_ustr, getParagraph(1)->getString());

    // without the fix, it crashes
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(u""_ustr, getParagraph(1)->getString());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf124722)
{
    createSwDoc("tdf124722.rtf");

    CPPUNIT_ASSERT_EQUAL(22, getPages());

    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});

    dispatchCommand(mxComponent, u".uno:Copy"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(22, getPages());

    dispatchCommand(mxComponent, u".uno:Paste"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(43, getPages());

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(22, getPages());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testToxmarkLinks)
{
    createSwDoc("udindex3.odt");
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    SwView& rView(*getSwDocShell()->GetView());

    // update indexes
    for (auto i = pWrtShell->GetTOXCount(); 0 < i;)
    {
        --i;
        pWrtShell->UpdateTableOf(*pWrtShell->GetTOX(i));
    }

    // click on the links...
    {
        static OUString constexpr tmp(u"Table of Contents"_ustr);
        pWrtShell->GotoNextTOXBase(&tmp);
    }

    { // ToC toxmark
        pWrtShell->Down(false);
        SfxItemSet aSet(pWrtShell->GetAttrPool(),
                        svl::Items<RES_TXTATR_INETFMT, RES_TXTATR_INETFMT>);
        pWrtShell->GetCurAttr(aSet);
        CPPUNIT_ASSERT(aSet.HasItem(RES_TXTATR_INETFMT));
        pWrtShell->Push();
        OUString const url(aSet.GetItem<SwFormatINetFormat>(RES_TXTATR_INETFMT)->GetValue());
        CPPUNIT_ASSERT_EQUAL(u"#1%19the%20tocmark%19C%7Ctoxmark"_ustr, url);
        rView.JumpToSwMark(url.subView(1)); // SfxApplication::OpenDocExec_Impl eats the "#"
        CPPUNIT_ASSERT_EQUAL(OUString(OUStringChar(CH_TXTATR_INWORD) + "tocmark"),
                             pWrtShell->GetCursor()->GetPointNode().GetTextNode()->GetText());
        pWrtShell->Pop(SwCursorShell::PopMode::DeleteCurrent);
    }

    { // ToC heading
        pWrtShell->Down(false);
        SfxItemSet aSet(pWrtShell->GetAttrPool(),
                        svl::Items<RES_TXTATR_INETFMT, RES_TXTATR_INETFMT>);
        pWrtShell->GetCurAttr(aSet);
        CPPUNIT_ASSERT(aSet.HasItem(RES_TXTATR_INETFMT));
        pWrtShell->Push();
        OUString const url(aSet.GetItem<SwFormatINetFormat>(RES_TXTATR_INETFMT)->GetValue());
        CPPUNIT_ASSERT_EQUAL(u"#__RefHeading___Toc105_706348105"_ustr, url);
        rView.JumpToSwMark(url.subView(1));
        CPPUNIT_ASSERT_EQUAL(u"foo"_ustr,
                             pWrtShell->GetCursor()->GetPointNode().GetTextNode()->GetText());
        pWrtShell->Pop(SwCursorShell::PopMode::DeleteCurrent);
    }

    {
        static OUString constexpr tmp(u"User-Defined1"_ustr);
        pWrtShell->GotoNextTOXBase(&tmp);
    }

    { // UD1 toxmark 1
        pWrtShell->Down(false);
        SfxItemSet aSet(pWrtShell->GetAttrPool(),
                        svl::Items<RES_TXTATR_INETFMT, RES_TXTATR_INETFMT>);
        pWrtShell->GetCurAttr(aSet);
        CPPUNIT_ASSERT(aSet.HasItem(RES_TXTATR_INETFMT));
        pWrtShell->Push();
        OUString const url(aSet.GetItem<SwFormatINetFormat>(RES_TXTATR_INETFMT)->GetValue());
        CPPUNIT_ASSERT_EQUAL(u"#1%19the%20udmark%19UUser-Defined%20Index%7Ctoxmark"_ustr,
                             url); // User-Defined => User-Defined Index, tdf#161574
        rView.JumpToSwMark(url.subView(1));
        CPPUNIT_ASSERT_EQUAL(OUString(OUStringChar(CH_TXTATR_INWORD) + "udmark the first"),
                             pWrtShell->GetCursor()->GetPointNode().GetTextNode()->GetText());
        pWrtShell->Pop(SwCursorShell::PopMode::DeleteCurrent);
    }

    { // UD1 toxmark 2 (with same text)
        pWrtShell->Down(false);
        SfxItemSet aSet(pWrtShell->GetAttrPool(),
                        svl::Items<RES_TXTATR_INETFMT, RES_TXTATR_INETFMT>);
        pWrtShell->GetCurAttr(aSet);
        CPPUNIT_ASSERT(aSet.HasItem(RES_TXTATR_INETFMT));
        pWrtShell->Push();
        OUString const url(aSet.GetItem<SwFormatINetFormat>(RES_TXTATR_INETFMT)->GetValue());
        CPPUNIT_ASSERT_EQUAL(u"#2%19the%20udmark%19UUser-Defined%20Index%7Ctoxmark"_ustr,
                             url); // User-Defined => User-Defined Index, tdf#161574
        rView.JumpToSwMark(url.subView(1));
        CPPUNIT_ASSERT_EQUAL(OUString(OUStringChar(CH_TXTATR_INWORD) + "udmark the 2nd"),
                             pWrtShell->GetCursor()->GetPointNode().GetTextNode()->GetText());
        pWrtShell->Pop(SwCursorShell::PopMode::DeleteCurrent);
    }

    { // UD heading
        pWrtShell->Down(false);
        SfxItemSet aSet(pWrtShell->GetAttrPool(),
                        svl::Items<RES_TXTATR_INETFMT, RES_TXTATR_INETFMT>);
        pWrtShell->GetCurAttr(aSet);
        CPPUNIT_ASSERT(aSet.HasItem(RES_TXTATR_INETFMT));
        pWrtShell->Push();
        OUString const url(aSet.GetItem<SwFormatINetFormat>(RES_TXTATR_INETFMT)->GetValue());
        CPPUNIT_ASSERT_EQUAL(u"#__RefHeading___Toc105_706348105"_ustr, url);
        rView.JumpToSwMark(url.subView(1));
        CPPUNIT_ASSERT_EQUAL(u"foo"_ustr,
                             pWrtShell->GetCursor()->GetPointNode().GetTextNode()->GetText());
        pWrtShell->Pop(SwCursorShell::PopMode::DeleteCurrent);
    }

    {
        static OUString constexpr tmp(u"NewUD!|1"_ustr);
        pWrtShell->GotoNextTOXBase(&tmp);
    }

    { // UD2 toxmark, with same text as those in other UD
        pWrtShell->Down(false);
        SfxItemSet aSet(pWrtShell->GetAttrPool(),
                        svl::Items<RES_TXTATR_INETFMT, RES_TXTATR_INETFMT>);
        pWrtShell->GetCurAttr(aSet);
        CPPUNIT_ASSERT(aSet.HasItem(RES_TXTATR_INETFMT));
        pWrtShell->Push();
        OUString const url(aSet.GetItem<SwFormatINetFormat>(RES_TXTATR_INETFMT)->GetValue());
        CPPUNIT_ASSERT_EQUAL(u"#1%19the%20udmark%19UNewUD!%7C%7Ctoxmark"_ustr, url);
        rView.JumpToSwMark(url.subView(1));
        CPPUNIT_ASSERT_EQUAL(u"the udmark"_ustr,
                             pWrtShell->GetCursor()->GetPointNode().GetTextNode()->GetText());
        pWrtShell->Pop(SwCursorShell::PopMode::DeleteCurrent);
    }
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf125261)
{
    createSwDoc("tdf125261.odt");

    CPPUNIT_ASSERT_EQUAL(u"https://www.example.com/"_ustr,
                         getProperty<OUString>(getRun(getParagraph(1), 1), u"HyperLinkURL"_ustr));
    //apply autocorrect StartAutoCorrect
    dispatchCommand(mxComponent, u".uno:AutoFormatApply"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(u"https://www.example.com/"_ustr,
                         getProperty<OUString>(getRun(getParagraph(1), 1), u"HyperLinkURL"_ustr));
    // without the fix, it hangs
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(u"https://www.example.com/"_ustr,
                         getProperty<OUString>(getRun(getParagraph(1), 1), u"HyperLinkURL"_ustr));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf141175)
{
    createSwDoc("tdf141175.odt");

    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(),
                                                         uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(1, getPages());

    //Use selectAll 3 times in a row
    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});
    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});
    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});

    //Without the fix in place, this test would have crashed here
    dispatchCommand(mxComponent, u".uno:Cut"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(1, getPages());

    dispatchCommand(mxComponent, u".uno:Paste"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf133990)
{
    createSwDoc("tdf133990.odt");

    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(),
                                                         uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(6), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(1, getPages());

    //Use selectAll 3 times in a row
    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});
    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});
    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});

    dispatchCommand(mxComponent, u".uno:Delete"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xIndexAccess->getCount());

    //Without the fix in place, it would have crashed here
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(6), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf126504)
{
    createSwDoc("tdf126504.odt");

    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(),
                                                         uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(1, getPages());

    //Use selectAll 2 times in a row
    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});
    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});
    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});

    dispatchCommand(mxComponent, u".uno:Copy"_ustr, {});

    dispatchCommand(mxComponent, u".uno:GoToEndOfPage"_ustr, {});

    dispatchCommand(mxComponent, u".uno:Paste"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(1, getPages());

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(1, getPages());

    dispatchCommand(mxComponent, u".uno:Redo"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(1, getPages());

    //Without the fix in place, it would have crashed here
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf133982)
{
    createSwDoc("tdf133982.docx");

    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(),
                                                         uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xIndexAccess->getCount());

    //Use selectAll 3 times in a row
    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});
    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});
    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});
    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});

    //Without the fix in place, it would have crashed here
    dispatchCommand(mxComponent, u".uno:Cut"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xIndexAccess->getCount());

    dispatchCommand(mxComponent, u".uno:Paste"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xIndexAccess->getCount());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf134253)
{
    createSwDoc("tdf134253.odt");

    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(),
                                                         uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(6, getPages());

    //Use selectAll 3 times in a row
    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});
    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});
    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});

    dispatchCommand(mxComponent, u".uno:Copy"_ustr, {});
    dispatchCommand(mxComponent, u".uno:Paste"_ustr, {});

    //Without the fix in place, it would have crashed here
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(6, getPages());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testNotebookBar)
{
    createSwDoc();

    //tdf#154282: Without the fix in place, this test would have crashed
    dispatchCommand(mxComponent, u".uno:ToolbarMode?Mode:string=notebookbar.ui"_ustr, {});
    dispatchCommand(mxComponent, u".uno:ToolbarMode?Mode:string=Single"_ustr, {});
    dispatchCommand(mxComponent, u".uno:ToolbarMode?Mode:string=Sidebar"_ustr, {});
    dispatchCommand(mxComponent, u".uno:ToolbarMode?Mode:string=notebookbar_compact.ui"_ustr, {});
    dispatchCommand(mxComponent,
                    u".uno:ToolbarMode?Mode:string=notebookbar_groupedbar_compact.ui"_ustr, {});
    dispatchCommand(mxComponent, u".uno:ToolbarMode?Mode:string=notebookbar_single.ui"_ustr, {});
    dispatchCommand(mxComponent, u".uno:ToolbarMode?Mode:string=Default"_ustr, {});
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, TestAsCharTextBox)
{
    // Related tickets:
    // tdf#138598 Replace vertical alignment of As_char textboxes in footer
    // tdf#140158 Remove horizontal positioning of As_char textboxes, because
    // the anchor moving does the same for it.

    createSwDoc("AsCharTxBxTest.docx");

    // Add 3x tab to the doc
    SwXTextDocument* pTextDoc = getSwTextDoc();
    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_TAB);
    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_TAB);
    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_TAB);
    Scheduler::ProcessEventsToIdle();

    auto pExportDump = parseLayoutDump();
    CPPUNIT_ASSERT(pExportDump);

    // Check if the texbox fallen apart due to the tabs
    const double nLeftSideOfShape1
        = getXPath(pExportDump, "/root/page/body/txt/anchored/SwAnchoredDrawObject/bounds", "left")
              .toDouble();
    const double nLeftSideOfTxBx1
        = getXPath(pExportDump, "/root/page/body/txt/anchored/fly/infos/bounds", "left").toDouble();

    CPPUNIT_ASSERT(nLeftSideOfShape1 < nLeftSideOfTxBx1);

    // Another test is for the tdf#138598: Check footer textbox
    const double nLeftSideOfShape2
        = getXPath(pExportDump, "/root/page[2]/footer/txt/anchored/SwAnchoredDrawObject/bounds",
                   "left")
              .toDouble();
    const double nLeftSideOfTxBx2
        = getXPath(pExportDump, "/root/page[2]/footer/txt/anchored/fly/infos/bounds", "left")
              .toDouble();

    CPPUNIT_ASSERT(nLeftSideOfShape2 < nLeftSideOfTxBx2);

    const double nTopSideOfShape2
        = getXPath(pExportDump, "/root/page[2]/footer/txt/anchored/SwAnchoredDrawObject/bounds",
                   "top")
              .toDouble();
    const double nTopSideOfTxBx2
        = getXPath(pExportDump, "/root/page[2]/footer/txt/anchored/fly/infos/bounds", "top")
              .toDouble();

    CPPUNIT_ASSERT(nTopSideOfShape2 < nTopSideOfTxBx2);
    // Without the fix in place the two texboxes has been fallen apart, and  asserts will broken.
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf155028)
{
    createSwDoc("tdf155028.odt");

    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    uno::Reference<drawing::XShapes> xGroupShape(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(10), xGroupShape->getCount());

    selectShape(1);
    dispatchCommand(mxComponent, u".uno:EnterGroup"_ustr, {});

    // Select a shape in the group
    SwXTextDocument* pTextDoc = getSwTextDoc();
    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_TAB);
    Scheduler::ProcessEventsToIdle();

    dispatchCommand(mxComponent, u".uno:Copy"_ustr, {});

    // Without the fix in place, this test would have crashed
    dispatchCommand(mxComponent, u".uno:Paste"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(sal_Int32(11), xGroupShape->getCount());

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(sal_Int32(10), xGroupShape->getCount());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf135581)
{
    createSwDoc("tdf135581.odt");

    selectShape(1);
    dispatchCommand(mxComponent, u".uno:SetAnchorAtChar"_ustr, {}); // this is "to char"

    // Without the fix, the image was moving when the anchor changed, letting text flow back.
    CPPUNIT_ASSERT_EQUAL(2, getPages());
    // Make sure that the image is still on the first page, and the text is on the second
    auto pExportDump = parseLayoutDump();
    assertXPath(pExportDump, "//page[1]/body/txt/anchored/fly");
    assertXPath(pExportDump, "//page[1]/body/txt//SwLineLayout", "length", u"0");
    assertXPath(pExportDump, "//page[2]/body/txt//SwLineLayout", "length", u"26");
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf140975)
{
    // Load the bugdoc
    createSwDoc("tdf140975.docx");

    // Set the Anchor of the shape to As_Char
    selectShape(1);
    dispatchCommand(mxComponent, u".uno:SetAnchorToChar"_ustr, {});

    // Get the layout of the textbox
    auto pExportDump = parseLayoutDump();
    CPPUNIT_ASSERT(pExportDump);

    const sal_Int32 nShpTop
        = getXPath(pExportDump, "/root/page/body/txt[4]/anchored/SwAnchoredDrawObject/bounds",
                   "top")
              .toInt32();
    const sal_Int32 nFrmTop
        = getXPath(pExportDump, "/root/page/body/txt[4]/anchored/fly/infos/bounds", "top")
              .toInt32();

    // Without the fix in place, the frame has less value for Top than
    // the shape. This means the frame is outside from the shape.
    CPPUNIT_ASSERT_GREATER(nShpTop, nFrmTop);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf76636)
{
    createSwDoc("tdf76636.doc");

    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(),
                                                         uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTextTable(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xTextTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(6), xTextTable->getColumns()->getCount());

    //go to middle row
    SwXTextDocument* pTextDoc = getSwTextDoc();
    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_DOWN);
    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_DOWN);
    Scheduler::ProcessEventsToIdle();

    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_TAB);
    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_TAB);
    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_TAB);
    Scheduler::ProcessEventsToIdle();

    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_LEFT | KEY_SHIFT);
    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_LEFT | KEY_SHIFT);
    Scheduler::ProcessEventsToIdle();

    dispatchCommand(mxComponent, u".uno:MergeCells"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xTextTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), xTextTable->getColumns()->getCount());

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xTextTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(6), xTextTable->getColumns()->getCount());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf76636_2)
{
    createSwDoc("tdf76636.doc");

    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(),
                                                         uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTextTable(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xTextTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(6), xTextTable->getColumns()->getCount());

    SwXTextDocument* pTextDoc = getSwTextDoc();
    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_DOWN);
    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_DOWN);
    Scheduler::ProcessEventsToIdle();

    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_TAB);
    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_TAB);
    Scheduler::ProcessEventsToIdle();

    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_RIGHT | KEY_SHIFT);
    Scheduler::ProcessEventsToIdle();

    dispatchCommand(mxComponent, u".uno:MergeCells"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xTextTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), xTextTable->getColumns()->getCount());

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xTextTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(6), xTextTable->getColumns()->getCount());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf140828)
{
    createSwDoc("tdf140828.docx");

    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    uno::Reference<drawing::XShape> xShp = getShape(1);
    CPPUNIT_ASSERT(xShp);

    uno::Reference<beans::XPropertySet> ShpProps(xShp, uno::UNO_QUERY_THROW);
    selectShape(1);

    dispatchCommand(mxComponent, u".uno:SetAnchorAtChar"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(
        text::TextContentAnchorType::TextContentAnchorType_AT_CHARACTER,
        ShpProps->getPropertyValue(u"AnchorType"_ustr).get<text::TextContentAnchorType>());

    uno::Reference<text::XTextFrame> xTxBx(SwTextBoxHelper::getUnoTextFrame(xShp));
    CPPUNIT_ASSERT(xTxBx);

    uno::Reference<beans::XPropertySet> TxBxProps(xTxBx, uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(u"top left image"_ustr, xTxBx->getText()->getString());

    CPPUNIT_ASSERT_MESSAGE("Bad Relative Orientation and Position!",
                           TxBxProps->getPropertyValue(u"HoriOrientRelation"_ustr).get<sal_Int16>()
                               != text::RelOrientation::CHAR);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf132725)
{
    createSwDoc("tdf132725.odt");

    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(u"AA"_ustr, getParagraph(1)->getString());

    dispatchCommand(mxComponent, u".uno:GoToEndOfPara"_ustr, {});

    dispatchCommand(mxComponent, u".uno:SwBackspace"_ustr, {});
    dispatchCommand(mxComponent, u".uno:SwBackspace"_ustr, {});

    // tdf#137587 fly is no longer deleted by backspace
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(u""_ustr, getParagraph(1)->getString());

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(u"AA"_ustr, getParagraph(1)->getString());

    dispatchCommand(mxComponent, u".uno:Redo"_ustr, {});
    dispatchCommand(mxComponent, u".uno:Redo"_ustr, {});

    // tdf#137587 fly is no longer deleted by backspace
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(u""_ustr, getParagraph(1)->getString());

    //Without the fix in place, it would crash here
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(u"A"_ustr, getParagraph(1)->getString());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf126340)
{
    createSwDoc("tdf126340.odt");

    dispatchCommand(mxComponent, u".uno:GoDown"_ustr, {});
    // without the fix, it crashes
    dispatchCommand(mxComponent, u".uno:Delete"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(u"foo"_ustr, getParagraph(1)->getString());
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(u"foo"_ustr, getParagraph(1)->getString());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf124397)
{
    createSwDoc("tdf124397.docx");

    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextFramesSupplier->getTextFrames(),
                                                         uno::UNO_QUERY);

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());

    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});
    dispatchCommand(mxComponent, u".uno:Delete"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xIndexAccess->getCount());
    // Without the fix in place, it would crash here
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf108124)
{
    createSwDoc("tdf108124.odt");

    uno::Reference<text::XTextGraphicObjectsSupplier> xTextGraphicObjectsSupplier(mxComponent,
                                                                                  uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(
        xTextGraphicObjectsSupplier->getGraphicObjects(), uno::UNO_QUERY);

    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xIndexAccess->getCount());

    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});

    dispatchCommand(mxComponent, u".uno:Copy"_ustr, {});

    dispatchCommand(mxComponent, u".uno:Paste"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xIndexAccess->getCount());

    uno::Reference<drawing::XShape> xOldShape1(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xOldShape2(xIndexAccess->getByIndex(1), uno::UNO_QUERY);

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xIndexAccess->getCount());

    uno::Reference<drawing::XShape> xNewShape1(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xNewShape2(xIndexAccess->getByIndex(1), uno::UNO_QUERY);

    // there should be 2 different objects now but they have the same names,
    // so rely on the object identity for testing...
    CPPUNIT_ASSERT(xOldShape1.get() != xNewShape1.get());
    CPPUNIT_ASSERT(xOldShape1.get() != xNewShape2.get());
    CPPUNIT_ASSERT(xOldShape2.get() != xNewShape1.get());
    CPPUNIT_ASSERT(xOldShape2.get() != xNewShape2.get());

    // Without the fix in place, this test would have crashed here
    dispatchCommand(mxComponent, u".uno:Redo"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xIndexAccess->getCount());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf134021)
{
    createSwDoc("tdf134021.docx");

    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(),
                                                         uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(12, getPages());

    dispatchCommand(mxComponent, u".uno:JumpToNextTable"_ustr, {});

    dispatchCommand(mxComponent, u".uno:DeleteTable"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(1, getPages());

    // Without the fix in place, it would have crashed here
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(12, getPages());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf136778)
{
    createSwDoc("tdf136778.docx");

    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(),
                                                         uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(1, getPages());

    dispatchCommand(mxComponent, u".uno:JumpToNextTable"_ustr, {});

    dispatchCommand(mxComponent, u".uno:DeleteTable"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(1, getPages());

    // Without the fix in place, it would have crashed here
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf123285)
{
    createSwDoc("tdf123285.odt");

    CPPUNIT_ASSERT_EQUAL(true,
                         getParagraph(1)->getString().endsWith(
                             ".  Here is a short sentence demonstrating this very peculiar bug"
                             ".  Here is a short sentence demonstrating this very peculiar bug."));

    dispatchCommand(mxComponent, u".uno:GoToEndOfPage"_ustr, {});

    SwXTextDocument* pTextDoc = getSwTextDoc();
    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_RETURN);
    Scheduler::ProcessEventsToIdle();

    // Without the fix in place, the last "sentence" would have been changed to " entence"
    CPPUNIT_ASSERT_EQUAL(true,
                         getParagraph(1)->getString().endsWith(
                             ". Here is a short sentence demonstrating this very peculiar bug"
                             ".  Here is a short sentence demonstrating this very peculiar bug."));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf130746)
{
    createSwDoc("tdf130746.odt");

    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(),
                                                         uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());

    dispatchCommand(mxComponent, u".uno:JumpToNextTable"_ustr, {});

    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(
        xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<text::XPageCursor> xCursor(xTextViewCursorSupplier->getViewCursor(),
                                              uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(1), xCursor->getPage());

    SwXTextDocument* pTextDoc = getSwTextDoc();
    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_RETURN);
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(sal_Int16(2), xCursor->getPage());

    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_UP | KEY_MOD2);
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(sal_Int16(2), xCursor->getPage());

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(sal_Int16(1), xCursor->getPage());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf129805)
{
    createSwDoc("tdf129805.docx");

    CPPUNIT_ASSERT_EQUAL(u"x"_ustr, getParagraph(1)->getString());
    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});

    // without the fix in place, it would crash here
    dispatchCommand(mxComponent, u".uno:Cut"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(u""_ustr, getParagraph(1)->getString());

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(u"x"_ustr, getParagraph(1)->getString());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf130685)
{
    createSwDoc("tdf130685.odt");

    CPPUNIT_ASSERT_EQUAL(2, getPages());
    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});

    dispatchCommand(mxComponent, u".uno:Cut"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(1, getPages());

    dispatchCommand(mxComponent, u".uno:Paste"_ustr, {});
    dispatchCommand(mxComponent, u".uno:Paste"_ustr, {});

    // Without fix in place, this test would have failed with:
    //- Expected: 2
    //- Actual  : 4
    CPPUNIT_ASSERT_EQUAL(2, getPages());

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(1, getPages());

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf132944)
{
    createSwDoc("tdf132944.odt");

    CPPUNIT_ASSERT_EQUAL(1, getPages());

    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});
    dispatchCommand(mxComponent, u".uno:Delete"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(1, getPages());

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    // Without the fix in place, the document would have had 2 pages
    CPPUNIT_ASSERT_EQUAL(1, getPages());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf104649)
{
    createSwDoc("tdf104649.docx");

    CPPUNIT_ASSERT_EQUAL(u"Test"_ustr, getParagraph(1)->getString());

    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});
    dispatchCommand(mxComponent, u".uno:Delete"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(1, getPages());

    // Without the fix in place, this test would have crashed here
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(u"Test"_ustr, getParagraph(1)->getString());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf111969_lastHalfcharOfSelection)
{
    // Given a document with a selected character,
    // the last half of the character should also be considered to be "in the selection"
    createSwDoc();
    SwDocShell* pDocShell = getSwDocShell();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    // move the cursor after the "o" (this is better/safer than testing cursor at end of paragraph)
    pWrtShell->Insert2(u"Hello!"_ustr);
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    // get last pixel that will be part of the selection (current position 1pt wide).
    Point aLogicL(pWrtShell->GetCharRect().Center());
    // cursor pos WOULD be in the selection, but just reduce by one for good measure...
    aLogicL.AdjustX(-1);
    // sanity check - pixel after should NOT be in the selection
    Point aLogicR(pWrtShell->GetCharRect().Center());
    aLogicR.AdjustX(1);
    SwRect aStartRect;
    SwRect aEndRect;
    //now select the letter 'o'
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/true, 1, /*bBasicCall=*/false);

    // sanity check - the selection really does contain (or not) the two logic points
    pWrtShell->GetCursor_()->FillStartEnd(aStartRect, aEndRect);
    SwRect aSel(aStartRect.TopLeft(), aEndRect.BottomRight());
    CPPUNIT_ASSERT(aSel.Contains(aLogicL));
    CPPUNIT_ASSERT(!aSel.Contains(aLogicR));

    // the pixel just at the end of the selection is considered to be in ModelPositionForViewPoint
    CPPUNIT_ASSERT(pWrtShell->TestCurrPam(aLogicL));
    // the pixel just past the end of the selection is not considered to be inside.
    CPPUNIT_ASSERT(!pWrtShell->TestCurrPam(aLogicR));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf134931)
{
    createSwDoc("tdf134931.odt");

    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(),
                                                         uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(1, getPages());

    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});

    dispatchCommand(mxComponent, u".uno:Copy"_ustr, {});

    dispatchCommand(mxComponent, u".uno:GoDown"_ustr, {});

    for (sal_Int32 i = 0; i < 10; ++i)
    {
        dispatchCommand(mxComponent, u".uno:Paste"_ustr, {});
    }

    CPPUNIT_ASSERT_EQUAL(sal_Int32(11), xIndexAccess->getCount());

    // Without the fix in place, this test would have failed with:
    // - Expected: 4
    // - Actual  : 1
    // Because the tables are pasted but not displayed

    CPPUNIT_ASSERT_EQUAL(4, getPages());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf130680)
{
    createSwDoc("tdf130680.odt");

    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(),
                                                         uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(23), xIndexAccess->getCount());

    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});

    rtl::Reference<SwDoc> xClpDoc(new SwDoc());
    xClpDoc->SetClipBoard(true);

    // without the fix, it crashes
    dispatchCommand(mxComponent, u".uno:Cut"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xIndexAccess->getCount());

    dispatchCommand(mxComponent, u".uno:Paste"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    xClpDoc.clear();

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xIndexAccess->getCount());

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(23), xIndexAccess->getCount());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf150457)
{
    createSwDoc();

    emulateTyping(u"a");
    dispatchCommand(mxComponent, u".uno:InsertFootnote"_ustr, {});
    emulateTyping(u"abc");

    uno::Reference<text::XFootnotesSupplier> xFootnotesSupplier(mxComponent, uno::UNO_QUERY);
    auto xFootnotes = xFootnotesSupplier->getFootnotes();
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), xFootnotes->getCount());
    auto xParagraph = uno::Reference<text::XTextRange>(xFootnotes->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"abc"_ustr, xParagraph->getString());

    SwXTextDocument* pTextDoc = getSwTextDoc();
    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_PAGEUP);
    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_RETURN);
    emulateTyping(u"d");

    dispatchCommand(mxComponent, u".uno:InsertFootnote"_ustr, {});
    emulateTyping(u"def");

    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2), xFootnotes->getCount());
    xParagraph = uno::Reference<text::XTextRange>(xFootnotes->getByIndex(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"def"_ustr, xParagraph->getString());

    // This key sequence deletes a footnote and its number (without the fix applied)
    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_UP);
    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_HOME);
    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_SHIFT | KEY_DOWN);
    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_DELETE);

    // Page up moves the cursor from the footnote area to the main text, then
    // doing select all and pressing delete removes all the text and footnote references,
    // thus removing all the footnotes
    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_PAGEUP);
    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, awt::Key::SELECT_ALL);
    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_DELETE);

    // Without having fix in place, segfault happens after running next line
    Scheduler::ProcessEventsToIdle();

    // Without the fix, the above action should have already created a crash,
    // but here we also check to make sure that the number of footnotes are
    // exactly zero, as expected
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), xFootnotes->getCount());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf140061)
{
    createSwDoc("tdf140061.odt");

    CPPUNIT_ASSERT_EQUAL(2, getPages());

    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});

    dispatchCommand(mxComponent, u".uno:Copy"_ustr, {});

    createSwDoc();
    dispatchCommand(mxComponent, u".uno:Paste"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(2, getPages());

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(1, getPages());

    // Paste special as RTF
    uno::Sequence<beans::PropertyValue> aPropertyValues = comphelper::InitPropertySequence(
        { { "SelectedFormat", uno::Any(static_cast<sal_uInt32>(SotClipboardFormatId::RTF)) } });

    dispatchCommand(mxComponent, u".uno:ClipboardFormatItems"_ustr, aPropertyValues);

    CPPUNIT_ASSERT_EQUAL(2, getPages());

    // Without the fix in place, this test would have crashed here
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(1, getPages());
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
