# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file
from uitest.uihelper.common import select_pos
from uitest.uihelper.calc import enter_text_to_cell
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues

#Bug 118189 - EDITING Crashes on Undo after Cut conditional formatting data

class tdf118189(UITestCase):
    def test_tdf118189(self):
        calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf118189.xlsx"))
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        #1. Open attached document
        #2. Copy Column A
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
        self.xUITest.executeCommand(".uno:SelectColumn")
        self.xUITest.executeCommand(".uno:Copy")
        #3. New Spreadsheet
        calc_doc2 = self.ui_test.load_file(get_url_for_data_file("emptyFile.ods"))
        gridwin2 = xCalcDoc.getChild("grid_window")
        document2 = self.ui_test.get_component()

        frames = self.ui_test.get_frames()
        frames[1].activate()
        #4. Paste it
        gridwin2.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
        self.xUITest.executeCommand(".uno:Paste")
        #5. Cut it
        self.xUITest.executeCommand(".uno:Cut")
        #6. Undo
        self.xUITest.executeCommand(".uno:Undo")

        #-> CRASH
        self.assertEqual(get_cell_by_position(document2, 0, 0, 0).getString(), "On Back Order")

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab: