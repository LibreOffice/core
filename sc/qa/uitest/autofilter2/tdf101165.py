# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_url_for_data_file
from uitest.uihelper.common import get_state_as_dict

#Bug 101165 - Crashing on a filter selection, every time

class tdf101165(UITestCase):
    def test_tdf101165_autofilter(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf101165.ods")) as calc_doc:
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")

            gridwin.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": "1", "ROW": "0"}))
            xFloatWindow = self.xUITest.getFloatWindow()
            xAll = xFloatWindow.getChild("toggle_all")
            xAll.executeAction("CLICK", tuple())

            xCheckListMenu = xFloatWindow.getChild("FilterDropDown")
            xTreeList = xCheckListMenu.getChild("check_tree_box")
            self.assertEqual(3, len(xTreeList.getChildren()))
            for i in range(3):
                xChild = xTreeList.getChild(str(i))
                self.assertEqual("false", get_state_as_dict(xChild)["IsChecked"])

                if i == 0 :
                    self.assertEqual(2, len(xChild.getChildren()))
                    for j in range(2):
                        self.assertEqual("false", get_state_as_dict(xChild.getChild(str(j)))["IsChecked"])
                elif i == 1:
                    self.assertEqual(6, len(xChild.getChildren()))
                    for j in range(6):
                        self.assertEqual("false", get_state_as_dict(xChild.getChild(str(j)))["IsChecked"])
                else:
                    self.assertEqual(0, len(xChild.getChildren()))


            self.assertEqual(get_cell_by_position(calc_doc, 1, 0, 1).getValue(), 6494)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
