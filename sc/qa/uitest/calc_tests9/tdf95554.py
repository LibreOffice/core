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

import platform

class Tdf95554(UITestCase):

   def test_tdf95554(self):
        with self.ui_test.create_doc_in_start_center("calc"):
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")

            enter_text_to_cell(gridwin, "A1", "A")
            enter_text_to_cell(gridwin, "A2", "B")
            enter_text_to_cell(gridwin, "A3", "C")
            enter_text_to_cell(gridwin, "A4", "D")
            enter_text_to_cell(gridwin, "A5", "E")
            enter_text_to_cell(gridwin, "A6", "F")

            gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:A1"}))
            gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A3:A3", "EXTEND":"1"}))
            gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A5:A5", "EXTEND":"1"}))
            gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A6:A6", "EXTEND":"1"}))
            self.xUITest.executeCommand(".uno:Copy")

        with self.ui_test.load_empty_file("writer") as writer_document:

            self.xUITest.getTopFocusWindow()
            self.xUITest.executeCommand(".uno:PasteUnformatted")

            # Without the fix, the test breaks here with:
            #AssertionError: 'A\n\nC\n\nE\nF' != 'A'

            if platform.system() == "Windows":
                self.assertEqual('A\r\n\r\nC\r\n\r\nE\r\nF',
                        writer_document.Text.String)
            else:
                self.assertEqual('A\n\nC\n\nE\nF',
                        writer_document.Text.String)

# vim: set shiftwidth=4 softtabstop=4 expandtab: