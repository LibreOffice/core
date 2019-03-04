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
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues
import org.libreoffice.unotest
import pathlib
#Bug 54018 - EDITING: CRASH sorting cells range with Comments
def get_url_for_data_file(file_name):
    return pathlib.Path(org.libreoffice.unotest.makeCopyFromTDOC(file_name)).as_uri()

class tdf54018(UITestCase):
    def test_td54018_sort_with_comments(self):
        calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf54018.ods"))
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()

        #click top left columns / rows heading field to select all cells
        self.xUITest.executeCommand(".uno:SelectAll")
        #Menu 'Data -> Sort -> Column D -> Descending' <ok>
        self.ui_test.execute_dialog_through_command(".uno:DataSort")
        xDialog = self.xUITest.getTopFocusWindow()
        xTabs = xDialog.getChild("tabcontrol")
        select_pos(xTabs, "0")
        xSortKey1 = xDialog.getChild("sortlb")
        xdown = xDialog.getChild("down")
        props = {"TEXT": "Column B"}
        actionProps = mkPropertyValues(props)
        xSortKey1.executeAction("SELECT", actionProps)
        xdown.executeAction("CLICK", tuple())
        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)
        #Bug: When progress bar reaches 40% LibO Stops responding
        #Verify
        self.assertEqual(get_cell_by_position(document, 0, 1, 0).getString(), "7")
        self.assertEqual(get_cell_by_position(document, 0, 1, 1).getString(), "6")
        self.assertEqual(get_cell_by_position(document, 0, 1, 2).getString(), "5")
        self.assertEqual(get_cell_by_position(document, 0, 1, 3).getString(), "4")
        self.assertEqual(get_cell_by_position(document, 0, 1, 4).getString(), "3")
        self.assertEqual(get_cell_by_position(document, 0, 1, 5).getString(), "2")
        self.assertEqual(get_cell_by_position(document, 0, 1, 6).getString(), "1")

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab: