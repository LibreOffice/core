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
from com.sun.star.lang import IllegalArgumentException


class XTextRangeCompare(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        cls._uno = UnoInProcess()
        cls._uno.setUp()

    @classmethod
    def tearDownClass(cls):
        cls._uno.tearDown()

    def test_compareRegionStarts(self):
        xDoc = self._uno.openEmptyWriterDoc()

        cursor = xDoc.Text.createTextCursor()
        xDoc.Text.insertString(cursor, "The first paragraph", 0)

        self.assertEqual(0, xDoc.Text.compareRegionStarts(cursor, cursor))

        with self.assertRaises(IllegalArgumentException):
            xDoc.Text.compareRegionStarts(cursor, None)

        with self.assertRaises(IllegalArgumentException):
            xDoc.Text.compareRegionStarts(None, cursor)

        with self.assertRaises(IllegalArgumentException):
            xDoc.Text.compareRegionStarts(None, None)

        xDoc.close(True)

    def test_compareRegionEnds(self):
        xDoc = self._uno.openEmptyWriterDoc()

        cursor = xDoc.Text.createTextCursor()
        xDoc.Text.insertString(cursor, "The first paragraph", 0)

        self.assertEqual(0, xDoc.Text.compareRegionEnds(cursor, cursor))

        with self.assertRaises(IllegalArgumentException):
            xDoc.Text.compareRegionEnds(cursor, None)

        with self.assertRaises(IllegalArgumentException):
            xDoc.Text.compareRegionEnds(None, cursor)

        with self.assertRaises(IllegalArgumentException):
            xDoc.Text.compareRegionEnds(None, None)

        xDoc.close(True)

    def test_compareRegionStarts_Different(self):
        xDoc = self._uno.openEmptyWriterDoc()

        cursor1 = xDoc.Text.createTextCursor()
        cursor2 = xDoc.Text.createTextCursor()

        xDoc.Text.insertString(cursor1, "The first paragraph", 0)

        cursor1.gotoStart(False)
        cursor2.gotoEnd(False)

        self.assertTrue(cursor1.isCollapsed())
        self.assertTrue(cursor1.isCollapsed())

        self.assertEqual(1, xDoc.Text.compareRegionStarts(cursor1, cursor2))
        self.assertEqual(-1, xDoc.Text.compareRegionStarts(cursor2, cursor1))

        xDoc.close(True)

    def test_compareRegionStarts_DiffSelection(self):
        xDoc = self._uno.openEmptyWriterDoc()

        cursor1 = xDoc.Text.createTextCursor()
        cursor2 = xDoc.Text.createTextCursor()

        xDoc.Text.insertString(cursor1, "The first paragraph", 0)

        cursor1.gotoStart(False)
        cursor1.gotoEnd(True)
        cursor2.gotoEnd(False)
        cursor2.gotoStart(True)

        self.assertFalse(cursor1.isCollapsed())
        self.assertFalse(cursor1.isCollapsed())

        self.assertEqual(0, xDoc.Text.compareRegionStarts(cursor1, cursor2))
        self.assertEqual(0, xDoc.Text.compareRegionEnds(cursor1, cursor2))

        xDoc.close(True)

    def test_compareRegionStarts_DiffSelection(self):
        xDoc = self._uno.openEmptyWriterDoc()

        cursor1 = xDoc.Text.createTextCursor()
        cursor2 = xDoc.Text.createTextCursor()

        xDoc.Text.insertString(cursor1, "The first paragraph", 0)

        cursor1.gotoStart(False)
        cursor1.gotoEnd(True)
        cursor1.goLeft(2, True)
        cursor2.gotoEnd(False)
        cursor2.gotoStart(True)
        cursor2.goRight(2, True)

        self.assertFalse(cursor1.isCollapsed())
        self.assertFalse(cursor1.isCollapsed())

        # whole text:  123456789
        # cursor1:     1234567
        # cursor2:       3456789

        self.assertEqual(1, xDoc.Text.compareRegionStarts(cursor1, cursor2))
        self.assertEqual(1, xDoc.Text.compareRegionEnds(cursor1, cursor2))

        self.assertEqual(-1, xDoc.Text.compareRegionStarts(cursor2, cursor1))
        self.assertEqual(-1, xDoc.Text.compareRegionEnds(cursor2, cursor1))

        xDoc.close(True)

    def test_compareRegionStarts_SameStart(self):
        xDoc = self._uno.openEmptyWriterDoc()

        cursor1 = xDoc.Text.createTextCursor()
        cursor2 = xDoc.Text.createTextCursor()

        xDoc.Text.insertString(cursor1, "The first paragraph", 0)

        cursor1.gotoStart(False)
        cursor1.goRight(2, False)
        cursor1.goRight(5, True)

        cursor2.gotoStart(False)
        cursor2.goRight(2, False)
        cursor2.goRight(7, True)

        self.assertFalse(cursor1.isCollapsed())
        self.assertFalse(cursor1.isCollapsed())

        # whole text:  123456789
        # cursor1:       34567
        # cursor2:       3456789

        self.assertEqual(0, xDoc.Text.compareRegionStarts(cursor1, cursor2))
        self.assertEqual(0, xDoc.Text.compareRegionStarts(cursor2, cursor1))

        self.assertEqual(1, xDoc.Text.compareRegionEnds(cursor1, cursor2))
        self.assertEqual(-1, xDoc.Text.compareRegionEnds(cursor2, cursor1))

        xDoc.close(True)


if __name__ == '__main__':
    unittest.main()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
