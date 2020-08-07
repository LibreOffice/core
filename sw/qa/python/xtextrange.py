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
from org.libreoffice.unotest import UnoInProcess


class TestXTextRange(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        cls._uno = UnoInProcess()
        cls._uno.setUp()
        cls._uno.openDocFromTDOC("xtextrange.odt")

    @classmethod
    def tearDownClass(cls):
        cls._uno.tearDown()

    def test_textRangesInPlainText(self):
        xCursor = self._uno.getDoc().getText().createTextCursor()
        xCursor.goRight(10, True)
        xTextRange = xCursor
        self.assertEqual(xTextRange.getString(), "0123456789")

        # getStart
        xTextRangeStart = xTextRange.getStart()
        self.assertIsNotNone(xTextRangeStart)
        self.assertEqual(xTextRangeStart.getString(), "")
        xTextRangeStart.setString("before")
        self.assertEqual(xTextRangeStart.getString(), "before")

        # Overwrite strings
        xTextRangeStart.setString("beforebeforebeforebefore")
        xTextRangeStart.setString("before2")
        xTextRangeStart.setString("before3")
        xTextRangeStart.setString("before")
        self.assertEqual(xTextRangeStart.getString(), "before")
        xCursor = self._uno.getDoc().getText().createTextCursor()
        xCursor.goRight(16, True)
        self.assertEqual(xCursor.getString(), "before0123456789")

        # getEnd
        xTextRangeEnd = xTextRange.getEnd()
        self.assertIsNotNone(xTextRangeEnd)
        self.assertEqual(xTextRangeEnd.getString(), "")
        xTextRangeEnd.setString("after")
        self.assertEqual(xTextRangeEnd.getString(), "after")

        # getText
        xText = xTextRange.getText()
        self.assertIsNotNone(xText)

        # Final check of what we have inserted
        xCursor = self._uno.getDoc().getText().createTextCursor()
        xCursor.goRight(21, True)
        self.assertEqual(xCursor.getString(), "before0123456789after")

    def test_textRangesInTable(self):
        xTextTables = self._uno.getDoc().getTextTables()
        self.assertIsNotNone(xTextTables)
        xTextTable = xTextTables[0]
        self.assertIsNotNone(xTextTable)

        xTextRange = xTextTable.getCellByName("A1")
        self.assertIsNotNone(xTextRange)
        self.assertEqual(xTextRange.getString(), "C1")

        xTextRangeStart = xTextRange.getStart()
        self.assertIsNotNone(xTextRangeStart)
        self.assertEqual(xTextRangeStart.getString(), "")
        xTextRangeStart.setString("before")
        self.assertEqual(xTextRangeStart.getString(), "before")

        xTextRangeEnd = xTextRange.getEnd()
        self.assertIsNotNone(xTextRangeEnd)
        self.assertEqual(xTextRangeEnd.getString(), "")
        xTextRangeEnd.setString("after")
        self.assertEqual(xTextRangeEnd.getString(), "after")

        # Ensure that what we inserted is in cell
        xTextRange2 = xTextTable.getCellByName("A1")
        self.assertEqual(xTextRange2.getString(), "beforeC1after")

    def test_textRangesCompare(self):
        doc = self._uno.getDoc()
        # Bookmark in body text
        bookmark1 = doc.getBookmarks().getByIndex(0).getAnchor()

        # Bookmarks in table
        bookmark2 = doc.getBookmarks().getByIndex(1).getAnchor()
        bookmark3 = doc.getBookmarks().getByIndex(2).getAnchor()

        res = doc.Text.compareRegionStarts(bookmark1, bookmark2)
        self.assertEqual(res, 1)

        res = doc.Text.compareRegionStarts(bookmark2, bookmark1)
        self.assertEqual(res, -1)

        res = doc.Text.compareRegionStarts(bookmark2, bookmark3)
        self.assertEqual(res, 1)

        res = doc.Text.compareRegionStarts(bookmark1, bookmark3)
        self.assertEqual(res, 1)

if __name__ == '__main__':
    unittest.main()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
