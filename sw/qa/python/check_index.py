#! /usr/bin/env python
# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
import unittest
import unohelper
from org.libreoffice.unotest import UnoInProcess
from com.sun.star.text.ControlCharacter import PARAGRAPH_BREAK
from com.sun.star.util import XRefreshListener


class RefreshListener(XRefreshListener, unohelper.Base):
    def __init__(self):
        self.m_refreshed = False
        self.m_disposed = False

    # Gets called when index is disposed
    def disposing(self, event):
        self.m_disposed = True

    # Gets called when index is refreshed
    def refreshed(self, event):
        self.m_refreshed = True


class CheckIndex(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls._uno = UnoInProcess()
        cls._uno.setUp()

    @classmethod
    def tearDownClass(cls):
        cls._uno.tearDown()

    # Gets called every time a new test is run
    def setUp(self):
        """
        Every test should run with a new Instance of an Index and
        Refresh Listener
        So we need to reload the text of the document and initialize
        the corresponding Cursor
        """
        self.xDoc = self.__class__._uno.openEmptyWriterDoc()
        self.xIndex = self.xDoc.createInstance(
            "com.sun.star.text.ContentIndex")
        self.xBodyText = self.xDoc.getText()
        self.xCursor = self.xBodyText.createTextCursor()
        self.xIndex.setPropertyValue("CreateFromOutline", True)
        self.xBodyText.insertTextContent(self.xCursor, self.xIndex, True)
        # Create Refresh Listener and bind it to the Index
        self.listener = RefreshListener()
        self.xIndex.addRefreshListener(self.listener)

    def tearDown(self):
        """
        Dispose Index and Document and check if the index was
        deleted while the tests
        """
        self.assertFalse(self.listener.m_disposed,
                         "Unexpected disparue of the Refresh Listener!")
        self.xIndex.dispose()
        self.assertTrue(self.listener.m_disposed,
                        "Could not dispose Refresh Listener")
        self.xDoc.dispose()

    def insert_heading(self, text):
        """
        Insert a Heading at the end of the document
        """
        self.xCursor.gotoEnd(False)
        self.xBodyText.insertControlCharacter(self.xCursor,
                                              PARAGRAPH_BREAK, False)
        self.xCursor.gotoEnd(False)
        self.xCursor.setString(text)
        self.xCursor.gotoStartOfParagraph(True)
        self.xCursor.setPropertyValue("ParaStyleName", "Heading 1")

    def test_index_refresh(self):
        """
        Try to insert a heading at the index, refresh the index and
        retrieve the heading again
        """
        heading = "The best test heading you have seen in your entire life"
        self.insert_heading(heading)
        self.xIndex.refresh()
        self.assertTrue(self.listener.m_refreshed, "Failed to refresh index!")
        self.listener.m_refreshed = False
        self.xCursor.gotoRange(self.xIndex.getAnchor().getEnd(), False)
        self.xCursor.gotoStartOfParagraph(True)
        # Get String at current position and search for the heading
        text = self.xCursor.getString()
        self.assertGreaterEqual(text.find(heading), 0,
                                "Failed to insert heading at index "
                                "and retrieve it again!")

    def test_index_update(self):
        """
        Try to insert a heading at the index, update the index
        and retrieve the heading again
        """
        heading = "Heading to test the index update"
        self.insert_heading(heading)
        self.xIndex.update()
        self.assertTrue(self.listener.m_refreshed, "Failed to update index!")
        self.listener.m_refreshed = False
        self.xCursor.gotoRange(self.xIndex.getAnchor().getEnd(), False)
        self.xCursor.gotoStartOfParagraph(True)
        # Get String at current position and search for the heading
        text = self.xCursor.getString()
        self.assertGreaterEqual(text.find(heading), 0,
                                "Failed to insert a heading at Index and "
                                "retrieve it again!")


if __name__ == "__main__":
    unittest.main()
