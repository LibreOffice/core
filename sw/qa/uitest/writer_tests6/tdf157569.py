# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_url_for_data_file
from libreoffice.uno.propertyvalue import mkPropertyValues
import platform

class tdf157569(UITestCase):

    def test_tdf157569(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf157569.docx")) as document:

            xShape = document.getDrawPages()[0][0]
            nHeight = xShape.getSize().Height
            nWidth = xShape.getSize().Width

            # Without the fix in place, this test would have failed with
            # AssertionError: 1663 != 944
            self.assertEqual(1663, nHeight)
            if platform.system() == "Windows":
                self.assertEqual(2155, nWidth) # no idea why it's different on Windows
            else:
                self.assertEqual(2118, nWidth)

            xDoc = self.xUITest.getTopFocusWindow()
            xEditWin = xDoc.getChild("writer_edit")

            # Select the formula
            self.xUITest.executeCommand(".uno:JumpToNextFrame")
            xEditWin.executeAction("SELECT", mkPropertyValues({"OBJECT":"Object1"}))
            self.assertEqual("SwXTextEmbeddedObject", document.CurrentSelection.getImplementationName())

            # Go into edit mode
            xEditWin.executeAction("TYPE", mkPropertyValues({"KEYCODE":"RETURN"}))

            self.assertEqual(nHeight, xShape.getSize().Height)
            self.assertEqual(nWidth, xShape.getSize().Width)

            # leave edit mode
            xEditWin.executeAction("TYPE", mkPropertyValues({"KEYCODE":"ESC"}))

            self.assertEqual(nHeight, xShape.getSize().Height)
            self.assertEqual(nWidth, xShape.getSize().Width)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
