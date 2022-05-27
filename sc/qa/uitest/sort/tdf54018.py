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
from uitest.uihelper.common import select_by_text, select_pos

from libreoffice.calc.document import get_cell_by_position


# Bug 54018 - EDITING: CRASH sorting cells range with Comments
class tdf54018(UITestCase):
    def test_td54018_sort_with_comments(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf54018.ods")) as calc_doc:
            #click top left columns / rows heading field to select all cells
            self.xUITest.executeCommand(".uno:SelectAll")
            #Menu 'Data -> Sort -> Column D -> Descending' <ok>
            with self.ui_test.execute_dialog_through_command(".uno:DataSort") as xDialog:
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "0")
                xSortKey1 = xDialog.getChild("sortlb")
                xdown = xDialog.getChild("down")
                select_by_text(xSortKey1, "Column B")
                xdown.executeAction("CLICK", tuple())
            #Bug: When progress bar reaches 40% LibO Stops responding
            #Verify
            self.assertEqual(get_cell_by_position(calc_doc, 0, 1, 0).getString(), "7")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 1, 1).getString(), "6")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 1, 2).getString(), "5")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 1, 3).getString(), "4")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 1, 4).getString(), "3")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 1, 5).getString(), "2")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 1, 6).getString(), "1")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
