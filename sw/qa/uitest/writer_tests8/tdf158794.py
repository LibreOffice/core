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

class tdf158794(UITestCase):
    def test_tdf158794(self):
        with self.ui_test.create_doc_in_start_center("calc"):

            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")

            enter_text_to_cell(gridwin, "A1", "1")

            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))

            self.xUITest.executeCommand(".uno:Copy")

        with self.ui_test.load_empty_file("writer") as document:

            self.xUITest.executeCommand(".uno:InsertFootnote")

            # Paste as DDE link
            # Without the fix in place, this test would have crashed here
            formatProperty = mkPropertyValues({"SelectedFormat": 59})
            self.xUITest.executeCommandWithParameters(".uno:ClipboardFormatItems", formatProperty)

            self.assertEqual("", document.Footnotes[0].String)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
