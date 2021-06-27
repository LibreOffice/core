# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

from uitest.framework import UITestCase
from uitest.uihelper.common import get_url_for_data_file
from libreoffice.uno.propertyvalue import mkPropertyValues

class Tdf95554(UITestCase):

   def test_tdf95554(self):
        calc_doc = self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()

        enter_text_to_cell(gridwin, "A1", "A")
        enter_text_to_cell(gridwin, "A2", "B")
        enter_text_to_cell(gridwin, "A3", "C")
        enter_text_to_cell(gridwin, "A4", "D")
        enter_text_to_cell(gridwin, "A5", "E")
        enter_text_to_cell(gridwin, "A6", "F")

        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A3"}))
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A5"}))
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A6"}))
        self.xUITest.executeCommand(".uno:Copy")

        with self.ui_test.load_empty_file("writer") as writer_document:

            self.xUITest.getTopFocusWindow()
            self.xUITest.executeCommand(".uno:PasteUnformatted")
            self.assertEqual('A\nC\nE\nF',
                    writer_document.Text.String)
