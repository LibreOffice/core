# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.calc import enter_text_to_cell

class tdf164299(UITestCase):
    def test_tdf164299(self):
        with self.ui_test.create_doc_in_start_center("calc"):

            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")

            enter_text_to_cell(gridwin, "A1", "Test 1")
            enter_text_to_cell(gridwin, "A2", "Test 2")
            enter_text_to_cell(gridwin, "A3", "Test 3")
            enter_text_to_cell(gridwin, "A4", "Test 4")

            gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:A4"}))

            self.xUITest.executeCommand(".uno:Copy")

        with self.ui_test.load_empty_file("impress") as document:

            # Paste as HTML
            # Without the fix in place, this test would have crashed here
            formatProperty = mkPropertyValues({"SelectedFormat": 51})
            self.xUITest.executeCommandWithParameters(".uno:ClipboardFormatItems", formatProperty)

            xDrawPage = document.getDrawPages()[0]
            self.assertEqual(3, len(xDrawPage))
            xShape = xDrawPage[2]
            self.assertEqual(1000, xShape.getSize().Width)
            self.assertEqual(3697, xShape.getSize().Height)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
