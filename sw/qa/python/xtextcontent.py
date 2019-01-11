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

class TestXTextContent(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        cls._uno = UnoInProcess()
        cls._uno.setUp()

    @classmethod
    def tearDownClass(cls):
        cls._uno.tearDown()

    def test_anchor_operations(self):
        x_doc = self._uno.openDocFromTDOC("xtextcontent.odt")
        self.assertIsNotNone(x_doc)

        # getAnchor for both text frames and ensure we receive ranges we expect
        x_frame_1 = self.get_text_frame("Frame1")
        x_range_1 = x_frame_1.getAnchor()
        self.assertIsNotNone(x_range_1)
        self.compare_range(x_range_1, "String1")

        x_frame_2 = self.get_text_frame("Frame2")
        x_range_2 = x_frame_2.getAnchor()
        self.assertIsNotNone(x_range_2)
        self.compare_range(x_range_2, "String2")

        # Check how XTextContent::attach works. Try to exchange anchors
        x_frame_1.attach(x_range_2)
        x_frame_2.attach(x_range_1)
        self.compare_range(x_frame_1.getAnchor(), "String2")
        self.compare_range(x_frame_2.getAnchor(), "String1")

        # Try to attach to None
        with self.assertRaises(IllegalArgumentException):
            x_frame_1.attach(None)

        # Trying to attach frame to range from other document
        x_doc_2 = self._uno.openDocFromTDOC("xcontrolshape.odt")
        with self.assertRaises(IllegalArgumentException):
            x_frame_1.attach(x_doc_2.getText())

        x_doc_2.close(True)
        x_doc.close(True)

    def get_text_frame(self, frame_name):
        x_test_frames = self._uno.getDoc().getTextFrames()
        self.assertIsNotNone(x_test_frames)

        x_test_frame = x_test_frames[frame_name]
        self.assertIsNotNone(x_test_frame)

        return x_test_frame

    # Helper to extract text content from range and compare to expected string
    def compare_range(self, x_range, expected_content):
        x_cursor = x_range.getText().createTextCursor()
        self.assertIsNotNone(x_cursor)

        x_cursor.collapseToStart()
        x_cursor.goRight(len(expected_content), True)
        self.assertEqual(x_cursor.getString(), expected_content)

if __name__ == '__main__':
    unittest.main()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
