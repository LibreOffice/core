/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

/**
 * implements the {@link DocumentTest} interface on top of a spreadsheet document
 */
class WriterDocumentTest : public DocumentTestBase
{
public:
    WriterDocumentTest(const Reference<XComponent>& rxDoc)
        : DocumentTestBase(rxDoc)
    {
    }

    OUString getDocumentDescription() { return u"text document"_ustr; }

    void initializeDocument()
    {
        // TODO?
    }

    void doSingleModification()
    {
        Reference<XTextDocument> textDoc(getDocument(), UNO_QUERY);
        Reference<XText> docText = textDoc->getText();
        docText->setString(s_blindText);
    }

    void verifyInitialDocumentState()
    {
        Reference<XTextDocument> textDoc(getDocument(), UNO_QUERY);
        Reference<XText> docText = textDoc->getText();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("document should be empty", u""_ustr, docText->getString());
    }

    void verifySingleModificationDocumentState()
    {
        Reference<XTextDocument> textDoc(getDocument(), UNO_QUERY);
        Reference<XText> docText = textDoc->getText();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("blind text not found", s_blindText, docText->getString());
    }

    int doMultipleModifications()
    {
        Reference<XTextDocument> textDoc(getDocument(), UNO_QUERY);
        Reference<XText> docText = textDoc->getText();

        int expectedUndoActions = 0;

        // create a cursor
        Reference<XTextCursor> cursor = docText->createTextCursor();

        // create a table
        Reference<XMultiServiceFactory> xMSF(getDocument(), UNO_QUERY_THROW);
        Reference<XTextTable> textTable(xMSF->createInstance(u"com.sun.star.text.TextTable"_ustr),
                                        UNO_QUERY);
        textTable->initialize(3, 3);
        Reference<XPropertySet> tableProps(textTable, UNO_QUERY);
        tableProps->setPropertyValue(u"BackColor"_ustr, Any(sal_Int32(0xCCFF44)));

        // insert the table into the doc
        docText->insertTextContent(cursor, textTable, false);
        ++expectedUndoActions; //FIXME this will create 2 actions! currently the event is sent for every individual action; should it be sent for top-level actions only? how many internal actions are created is an implementation detail!
        ++expectedUndoActions;

        // write some content into the center cell
        Reference<XCellRange> cellRange(textTable, UNO_QUERY);
        Reference<XCell> centerCell = cellRange->getCellByPosition(1, 1);
        Reference<XTextRange> cellText(centerCell, UNO_QUERY);
        cellText->setString(u"Undo Manager API Test"_ustr);
        ++expectedUndoActions;

        // give it another color
        Reference<XPropertySet> cellProps(centerCell, UNO_QUERY);
        cellProps->setPropertyValue(u"BackColor"_ustr, Any(sal_Int32(0x44CCFF)));
        ++expectedUndoActions;

        return expectedUndoActions;
    }

    OUString s_blindText
        = u"Lorem ipsum dolor. Sit amet penatibus. A cum turpis. Aenean ac eu. "
          "Ligula est urna nulla vestibulum ullamcorper. Nec sit in amet tincidunt mus. "
          "Tellus sagittis mi. Suscipit cursus in vestibulum in eros ipsum felis cursus lectus "
          "nunc quis condimentum in risus nec wisi aenean luctus hendrerit magna habitasse commodo "
          "orci. "
          "Nisl etiam quis. Vestibulum justo eleifend aliquet luctus sed turpis volutpat "
          "ullamcorper "
          "aliquam penatibus sagittis pede tincidunt egestas. Nibh massa lectus. Sem mattis purus "
          "morbi "
          "scelerisque turpis donec urna phasellus. Quis at lacus. Viverra mauris mollis. "
          "Dolor tincidunt condimentum."_ustr;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
