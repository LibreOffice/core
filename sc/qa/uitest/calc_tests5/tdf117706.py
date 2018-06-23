# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict
from uitest.uihelper.common import select_pos
from uitest.uihelper.calc import enter_text_to_cell
from uitest.debug import sleep
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues
import org.libreoffice.unotest
import pathlib

def get_url_for_data_file(file_name):
    return pathlib.Path(org.libreoffice.unotest.makeCopyFromTDOC(file_name)).as_uri()

#Bug 117706 - Copying multiple rows from one sheet to another causes the first row to be overwritten by the last one

class tdf117706(UITestCase):
    def test_tdf117706(self):
        calc_doc = self.ui_test.load_file(get_url_for_data_file("emptyFile.ods"))
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        #add data to first sheet
        enter_text_to_cell(gridwin, "A1", "A1")
        enter_text_to_cell(gridwin, "B1", "B1")
        enter_text_to_cell(gridwin, "C1", "C1")

        enter_text_to_cell(gridwin, "A2", "A2")
        enter_text_to_cell(gridwin, "B2", "B2")
        enter_text_to_cell(gridwin, "C2", "C2")

        enter_text_to_cell(gridwin, "A4", "A4")
        enter_text_to_cell(gridwin, "B4", "B4")
        enter_text_to_cell(gridwin, "C4", "C4")

        #select rows 1-4
        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:A4"}))
        self.xUITest.executeCommand(".uno:SelectRow")
        self.xUITest.executeCommand(".uno:Copy")
        #3. New Spreadsheet
        calc_doc2 = self.ui_test.load_file(get_url_for_data_file("emptyFile2.ods"))
        gridwin2 = xCalcDoc.getChild("grid_window")
        document2 = self.ui_test.get_component()

        frames = self.ui_test.get_frames()
        frames[1].activate()
        #4. Paste
        gridwin2.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
        self.xUITest.executeCommand(".uno:Paste")
        #Verify
        self.assertEqual(get_cell_by_position(document2, 0, 0, 0).getString(), "A1")
        self.assertEqual(get_cell_by_position(document2, 0, 1, 0).getString(), "B1")
        self.assertEqual(get_cell_by_position(document2, 0, 2, 0).getString(), "C1")

        self.assertEqual(get_cell_by_position(document2, 0, 0, 1).getString(), "A2")
        self.assertEqual(get_cell_by_position(document2, 0, 1, 1).getString(), "B2")
        self.assertEqual(get_cell_by_position(document2, 0, 2, 1).getString(), "C2")

        self.assertEqual(get_cell_by_position(document2, 0, 0, 2).getString(), "")
        self.assertEqual(get_cell_by_position(document2, 0, 1, 2).getString(), "")
        self.assertEqual(get_cell_by_position(document2, 0, 2, 2).getString(), "")

        self.assertEqual(get_cell_by_position(document2, 0, 0, 3).getString(), "A4")
        self.assertEqual(get_cell_by_position(document2, 0, 1, 3).getString(), "B4")
        self.assertEqual(get_cell_by_position(document2, 0, 2, 3).getString(), "C4")
        #6. Undo
        self.xUITest.executeCommand(".uno:Undo")

        self.assertEqual(get_cell_by_position(document2, 0, 0, 0).getString(), "")

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
