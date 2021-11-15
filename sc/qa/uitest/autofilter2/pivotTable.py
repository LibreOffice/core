# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file
from libreoffice.uno.propertyvalue import mkPropertyValues
from libreoffice.calc.document import get_cell_by_position

class pivotTable(UITestCase):

    def test_pivotTable(self):
        with self.ui_test.load_file(get_url_for_data_file("pivotTable.ods")) as calc_doc:

            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")

            self.assertEqual("a", get_cell_by_position(calc_doc, 0, 3, 1).getString())
            self.assertEqual("b", get_cell_by_position(calc_doc, 0, 3, 2).getString())
            self.assertEqual("m", get_cell_by_position(calc_doc, 0, 4, 1).getString())
            self.assertEqual("n", get_cell_by_position(calc_doc, 0, 4, 2).getString())
            self.assertEqual("1", get_cell_by_position(calc_doc, 0, 5, 1).getString())
            self.assertEqual("1", get_cell_by_position(calc_doc, 0, 5, 2).getString())

            gridwin.executeAction("LAUNCH", mkPropertyValues({"PIVOTTABLE": "", "COL": "3", "ROW": "0"}))
            xFloatWindow = self.xUITest.getFloatWindow()
            xCheckListMenu = xFloatWindow.getChild("check_list_menu")

            xTreeList = xCheckListMenu.getChild("check_list_box")
            xFirstEntry = xTreeList.getChild("0")

            xFirstEntry.executeAction("CLICK", tuple())

            xOkBtn = xFloatWindow.getChild("ok")
            xOkBtn.executeAction("CLICK", tuple())

            self.assertEqual("b", get_cell_by_position(calc_doc, 0, 3, 1).getString())
            self.assertEqual("Total Result", get_cell_by_position(calc_doc, 0, 3, 2).getString())
            self.assertEqual("n", get_cell_by_position(calc_doc, 0, 4, 1).getString())
            self.assertEqual("", get_cell_by_position(calc_doc, 0, 4, 2).getString())
            self.assertEqual("1", get_cell_by_position(calc_doc, 0, 5, 1).getString())
            self.assertEqual("1", get_cell_by_position(calc_doc, 0, 5, 2).getString())

# vim: set shiftwidth=4 softtabstop=4 expandtab:
