# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file
from libreoffice.calc.document import get_cell_by_position

class pivotTable(UITestCase):

    def test_cancelButton(self):

        # This is basically a test for cf93998eb5abc193d95ae5433bf4dfd11a9d62d8
        # Without the fix in place, this test would have crashed

        with self.ui_test.load_file(get_url_for_data_file("basicPivotTable.ods")):

            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")

            gridwin.executeAction("SELECT", mkPropertyValues({"TABLE": "1"}))

            self.xUITest.executeCommand(".uno:GoUp")


            with self.ui_test.execute_dialog_through_command(".uno:DataDataPilotRun") as xDialog:
                xFields = xDialog.getChild("listbox-fields")
                self.assertEqual(2, len(xFields.getChildren()))
                self.assertEqual("qtX", get_state_as_dict(xFields.getChild('0'))['Text'])
                self.assertEqual("qtY", get_state_as_dict(xFields.getChild('1'))['Text'])

                xColumns = xDialog.getChild("listbox-column")
                self.assertEqual(1, len(xColumns.getChildren()))
                self.assertEqual("Data", get_state_as_dict(xColumns.getChild('0'))['Text'])

                xPage = xDialog.getChild("listbox-page")
                self.assertEqual(1, len(xPage.getChildren()))
                xPageChild = xPage.getChild('0')
                self.assertEqual("qtX", get_state_as_dict(xPageChild)['Text'])

                with self.ui_test.execute_blocking_action(xPageChild.executeAction, args=('DOUBLECLICK', ())) as dialog:
                    optionBtn = dialog.getChild("options")

                    with self.ui_test.execute_blocking_action(optionBtn.executeAction, args=('CLICK', ())) as dialog2:
                        xEmptyLine = dialog2.getChild("emptyline")

                        xEmptyLine.executeAction("CLICK", tuple())
                        self.assertEqual('true', get_state_as_dict(xEmptyLine)['Selected'])

                    with self.ui_test.execute_blocking_action(optionBtn.executeAction, args=('CLICK', ()), close_button="cancel") as dialog2:
                        xEmptyLine = dialog2.getChild("emptyline")

                        xEmptyLine.executeAction("CLICK", tuple())
                        self.assertEqual('false', get_state_as_dict(xEmptyLine)['Selected'])

                    with self.ui_test.execute_blocking_action(optionBtn.executeAction, args=('CLICK', ())) as dialog2:
                        xEmptyLine = dialog2.getChild("emptyline")

                        self.assertEqual('true', get_state_as_dict(xEmptyLine)['Selected'])

    def test_popup(self):
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
            xCheckListMenu = xFloatWindow.getChild("FilterDropDown")

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
