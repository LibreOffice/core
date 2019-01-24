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
#include <wrtsh.hxx>
#include <ndtxt.hxx>
#include <swdtflvr.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;

namespace
{
char const DATA_DIRECTORY[] = "/sw/qa/extras/unowriter/data/";

/// Listener implementation for testPasteListener.
class PasteListener : public cppu::WeakImplHelper<text::XPasteListener>
{
    OUString m_aString;

public:
    void SAL_CALL notifyPasteEvent(const uno::Sequence<beans::PropertyValue>& rEvent) override;

    OUString& GetString();
};

void PasteListener::notifyPasteEvent(const uno::Sequence<beans::PropertyValue>& rEvent)
{
    comphelper::SequenceAsHashMap aMap(rEvent);
    auto it = aMap.find("TextRange");
    if (it != aMap.end())
    {
        auto xTextRange = it->second.get<uno::Reference<text::XTextRange>>();
        if (xTextRange.is())
            m_aString = xTextRange->getString();
    }
}

OUString& PasteListener::GetString() { return m_aString; }
}

/// Test to assert UNO API call results of Writer.
class SwUnoWriter : public SwModelTestBase
{
public:
    void testDefaultCharStyle();
    void testSelectionInTableEnum();
    void testSelectionInTableEnumEnd();
    void testPasteListener();

    CPPUNIT_TEST_SUITE(SwUnoWriter);
    CPPUNIT_TEST(testDefaultCharStyle);
    CPPUNIT_TEST(testSelectionInTableEnum);
    CPPUNIT_TEST(testSelectionInTableEnumEnd);
    CPPUNIT_TEST(testPasteListener);
    CPPUNIT_TEST_SUITE_END();
};

void SwUnoWriter::testDefaultCharStyle()
{
    // Create a new document, type a character, set its char style to Emphasis
    // and assert the style was set.
    loadURL("private:factory/swriter", nullptr);

    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XSimpleText> xBodyText(xTextDocument->getText(), uno::UNO_QUERY);
    xBodyText->insertString(xBodyText->getStart(), "x", false);

    uno::Reference<text::XTextCursor> xCursor(xBodyText->createTextCursor());
    xCursor->goLeft(1, true);

    uno::Reference<beans::XPropertySet> xCursorProps(xCursor, uno::UNO_QUERY);
    xCursorProps->setPropertyValue("CharStyleName", uno::makeAny(OUString("Emphasis")));
    CPPUNIT_ASSERT_EQUAL(awt::FontSlant_ITALIC,
                         getProperty<awt::FontSlant>(xCursorProps, "CharPosture"));

    // Now reset the char style and assert that the font slant is back to none.
    // This resulted in a lang.IllegalArgumentException, Standard was not
    // mapped to 'Default Style'.
    xCursorProps->setPropertyValue("CharStyleName", uno::makeAny(OUString("Standard")));
    CPPUNIT_ASSERT_EQUAL(awt::FontSlant_NONE,
                         getProperty<awt::FontSlant>(xCursorProps, "CharPosture"));
}

void SwUnoWriter::testSelectionInTableEnum()
{
    load(DATA_DIRECTORY, "selection-in-table-enum.odt");

    // Select the A1 cell's text.
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwWrtShell* pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);
    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->EndPara(/*bSelect=*/true);
    CPPUNIT_ASSERT_EQUAL(OUString("A1"),
                         pWrtShell->GetCursor()->GetNode().GetTextNode()->GetText());

    // Access the selection.
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xModel.is());
    uno::Reference<container::XIndexAccess> xSelections(xModel->getCurrentSelection(),
                                                        uno::UNO_QUERY);
    CPPUNIT_ASSERT(xSelections.is());
    uno::Reference<text::XTextRange> xSelection(xSelections->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xSelection.is());

    // Enumerate paragraphs in the selection.
    uno::Reference<container::XEnumerationAccess> xCursor(
        xSelection->getText()->createTextCursorByRange(xSelection), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xCursor.is());
    uno::Reference<container::XEnumeration> xEnum = xCursor->createEnumeration();
    xEnum->nextElement();
    // Without the accompanying fix in place, this test would have failed: i.e.
    // the enumeration contained a second paragraph, even if the cell has only
    // one paragraph.
    CPPUNIT_ASSERT(!xEnum->hasMoreElements());
}

void SwUnoWriter::testSelectionInTableEnumEnd()
{
    load(DATA_DIRECTORY, "selection-in-table-enum.odt");

    // Select from "Before" till the table end.
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwWrtShell* pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);
    pWrtShell->Down(/*bSelect=*/true);

    // Access the selection.
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xModel.is());
    uno::Reference<container::XIndexAccess> xSelections(xModel->getCurrentSelection(),
                                                        uno::UNO_QUERY);
    CPPUNIT_ASSERT(xSelections.is());
    uno::Reference<text::XTextRange> xSelection(xSelections->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xSelection.is());
    OUString aExpectedSelection
        = "Before" SAL_NEWLINE_STRING "A1" SAL_NEWLINE_STRING "B1" SAL_NEWLINE_STRING
          "C2" SAL_NEWLINE_STRING "A2" SAL_NEWLINE_STRING "B2" SAL_NEWLINE_STRING
          "C2" SAL_NEWLINE_STRING;
    CPPUNIT_ASSERT_EQUAL(aExpectedSelection, xSelection->getString());

    // Enumerate paragraphs in the selection.
    uno::Reference<container::XEnumerationAccess> xCursor(
        xSelection->getText()->createTextCursorByRange(xSelection), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xCursor.is());
    uno::Reference<container::XEnumeration> xEnum = xCursor->createEnumeration();
    // Before.
    xEnum->nextElement();
    // Table.
    xEnum->nextElement();
    // Without the accompanying fix in place, this test would have failed: i.e.
    // the enumeration contained the paragraph after the table, but no part of
    // that paragraph was part of the selection.
    CPPUNIT_ASSERT(!xEnum->hasMoreElements());
}

void SwUnoWriter::testPasteListener()
{
    loadURL("private:factory/swriter", nullptr);

    // Insert initial string.
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XSimpleText> xBodyText(xTextDocument->getText(), uno::UNO_QUERY);
    xBodyText->insertString(xBodyText->getStart(), "ABCDEF", false);

    // Add paste listener.
    uno::Reference<text::XPasteBroadcaster> xBroadcaster(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XPasteListener> xListener(new PasteListener);
    auto pListener = static_cast<PasteListener*>(xListener.get());
    xBroadcaster->addPasteEventListener(xListener);

    // Cut "DE" and then paste it.
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwWrtShell* pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);
    pWrtShell->Left(CRSR_SKIP_CHARS, /*bSelect=*/false, 3, /*bBasicCall=*/false);
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/true, 2, /*bBasicCall=*/false);
    rtl::Reference<SwTransferable> pTransfer = new SwTransferable(*pWrtShell);
    pTransfer->Cut();
    TransferableDataHelper aHelper(pTransfer.get());
    SwTransferable::Paste(*pWrtShell, aHelper);
    // Without working listener registration in place, this test would have
    // failed with 'Expected: DE; Actual:', i.e. the paste listener was not
    // invoked.
    CPPUNIT_ASSERT_EQUAL(OUString("DE"), pListener->GetString());

    // Make sure that paste did not overwrite anything.
    CPPUNIT_ASSERT_EQUAL(OUString("ABCDEF"), xBodyText->getString());

    // Paste again, this time overwriting "BC".
    pWrtShell->Left(CRSR_SKIP_CHARS, /*bSelect=*/false, 4, /*bBasicCall=*/false);
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/true, 2, /*bBasicCall=*/false);
    pListener->GetString().clear();
    SwTransferable::Paste(*pWrtShell, aHelper);
    CPPUNIT_ASSERT_EQUAL(OUString("DE"), pListener->GetString());

    // Make sure that paste overwrote "BC".
    CPPUNIT_ASSERT_EQUAL(OUString("ADEDEF"), xBodyText->getString());

    // Deregister paste listener, make sure it's not invoked.
    xBroadcaster->removePasteEventListener(xListener);
    pListener->GetString().clear();
    SwTransferable::Paste(*pWrtShell, aHelper);
    CPPUNIT_ASSERT(pListener->GetString().isEmpty());
}

CPPUNIT_TEST_SUITE_REGISTRATION(SwUnoWriter);
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
