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
from com.sun.star.lang import IllegalArgumentException
import uno


class TestXTextContent(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        cls._uno = UnoInProcess()
        cls._uno.setUp()

    @classmethod
    def tearDownClass(cls):
        cls._uno.tearDown()

    def test_anchorOperations(self):
        xDoc = self._uno.openDocFromTDOC("xtextcontent.odt")
        self.assertIsNotNone(xDoc)

        # getAnchor for both text frames and ensure we receive ranges we expect
        xFrame1 = self.getTextFrame("Frame1")
        xRange1 = xFrame1.getAnchor()
        self.assertIsNotNone(xRange1)
        self.compareRange(xRange1, "String1")

        xFrame2 = self.getTextFrame("Frame2")
        xRange2 = xFrame2.getAnchor()
        self.assertIsNotNone(xRange2)
        self.compareRange(xRange2, "String2")

        # Check how XTextContent::attach works. Try to exchange anchors
        xFrame1.attach(xRange2)
        xFrame2.attach(xRange1)
        self.compareRange(xFrame1.getAnchor(), "String2")
        self.compareRange(xFrame2.getAnchor(), "String1")

        # Try to attach to None
        with self.assertRaises(IllegalArgumentException):
            xFrame1.attach(None)

        # Trying to attach frame to range from other document
        xDoc2 = self._uno.openDocFromTDOC("xcontrolshape.odt")
        with self.assertRaises(IllegalArgumentException):
            xFrame1.attach(xDoc2.getText())

        xDoc2.close(True)
        xDoc.close(True)

    def getTextFrame(self, frameName):
        xTextFrames = self._uno.getDoc().getTextFrames()
        self.assertIsNotNone(xTextFrames)
        xTextFrame = xTextFrames[frameName]
        self.assertIsNotNone(xTextFrame)
        return xTextFrame

    # Helper to extract text content from range and compare to expected string
    def compareRange(self, xRange, expectedContent):
        xCursor = xRange.getText().createTextCursor()
        self.assertIsNotNone(xCursor)
        xCursor.collapseToStart()
        xCursor.goRight(len(expectedContent), True)
        self.assertEqual(xCursor.getString(), expectedContent)


if __name__ == '__main__':
    unittest.main()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
