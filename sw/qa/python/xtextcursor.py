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


class TestXTextCursor(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        cls._uno = UnoInProcess()
        cls._uno.setUp()
        cls._uno.openDocFromTDOC("xtextcursor.odt")

    @classmethod
    def tearDownClass(cls):
        cls._uno.tearDown()

    def createTextCursorInFrame(self, frameName):
        xTextFrames = self._uno.getDoc().getTextFrames()
        self.assertIsNotNone(xTextFrames)
        xTextFrame = xTextFrames[frameName]
        self.assertIsNotNone(xTextFrame)
        xCursor = xTextFrame.getText().createTextCursor()
        self.assertIsNotNone(xCursor)
        return xCursor

    def test_cursorMoveInText(self):
        # Create cursor in frame with simple text (to avoid moving beyond)
        xCursor = self.createTextCursorInFrame("FrameSimple")

        xCursor.collapseToStart()
        self.assertTrue(xCursor.isCollapsed())
        self.assertTrue(xCursor.goRight(1, True))
        self.assertFalse(xCursor.isCollapsed())
        # Try to move right 10 characters, but we really can just 3, so partial move
        self.assertFalse(xCursor.goRight(10, True))
        self.assertFalse(xCursor.isCollapsed())
        # Ensure that all line text is selected
        self.assertEqual(xCursor.getString(), "1234")

        self.assertFalse(xCursor.goRight(-10, True))
        self.assertEqual(xCursor.getString(), "1234")

        xCursor.collapseToEnd()
        self.assertTrue(xCursor.isCollapsed())
        self.assertTrue(xCursor.goLeft(2, True))
        self.assertFalse(xCursor.isCollapsed())
        self.assertEqual(xCursor.getString(), "34")

        # Move to start without selection
        self.assertTrue(xCursor.goLeft(2, False))
        self.assertEqual(xCursor.getString(), "")

        self.assertTrue(xCursor.isCollapsed())

        # Select all text
        xCursor.gotoStart(False)
        self.assertTrue(xCursor.isCollapsed())
        xCursor.gotoEnd(True)
        self.assertFalse(xCursor.isCollapsed())
        self.assertEqual(xCursor.getString(), "1234")

        # Select all text from behind
        xCursor.gotoEnd(False)
        self.assertTrue(xCursor.isCollapsed())
        xCursor.gotoStart(True)
        self.assertFalse(xCursor.isCollapsed())
        self.assertEqual(xCursor.getString(), "1234")

        # Select all text, alternative way via gotoRange
        xCursor2 = self.createTextCursorInFrame("FrameSimple")
        xCursor2.gotoEnd(False)
        xCursor2.gotoStart(True)
        xCursor.gotoEnd(False)
        self.assertTrue(xCursor.isCollapsed())
        xCursor.gotoRange(xCursor2, True)
        self.assertFalse(xCursor.isCollapsed())
        self.assertEqual(xCursor.getString(), "1234")

    def test_cursorMoveInTable(self):
        # Create cursor in frame with table
        xCursor = self.createTextCursorInFrame("FrameTable")

        # Nothing is selected
        xCursor.collapseToEnd()
        self.assertTrue(xCursor.isCollapsed())
        self.assertEqual(xCursor.getString(), "")
        self.assertFalse(xCursor.goLeft(1, False))
        self.assertFalse(xCursor.goLeft(1, True))
        self.assertEqual(xCursor.getString(), "")


if __name__ == '__main__':
    unittest.main()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
