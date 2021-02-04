# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file

class pivotTable(UITestCase):

   def test_cancelButton(self):

        # This is basically a test for cf93998eb5abc193d95ae5433bf4dfd11a9d62d8
        # Without the fix in place, this test would have crashed

        self.ui_test.load_file(get_url_for_data_file("basicPivotTable.ods"))

        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")

        gridwin.executeAction("SELECT", mkPropertyValues({"TABLE": "1"}))

        self.xUITest.executeCommand(".uno:GoUp")


        self.ui_test.execute_dialog_through_command(".uno:DataDataPilotRun")

        xDialog = self.xUITest.getTopFocusWindow()

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

        def handle_dataField_dialog(dialog):
            optionBtn = dialog.getChild("options")

            def handle_options_dialog_first_time(dialog2):
                xEmptyLine = dialog2.getChild("emptyline")

                xEmptyLine.executeAction("CLICK", tuple())
                self.assertEqual('true', get_state_as_dict(xEmptyLine)['Selected'])

                xOKBtn = dialog2.getChild("ok")
                self.ui_test.close_dialog_through_button(xOKBtn)

            def handle_options_dialog_second_time(dialog2):
                xEmptyLine = dialog2.getChild("emptyline")

                xEmptyLine.executeAction("CLICK", tuple())
                self.assertEqual('false', get_state_as_dict(xEmptyLine)['Selected'])

                xCancelBtn = dialog2.getChild("cancel")
                self.ui_test.close_dialog_through_button(xCancelBtn)

            def handle_options_dialog_third_time(dialog2):
                xEmptyLine = dialog2.getChild("emptyline")

                self.assertEqual('true', get_state_as_dict(xEmptyLine)['Selected'])

                xOKBtn = dialog2.getChild("ok")
                self.ui_test.close_dialog_through_button(xOKBtn)

            self.ui_test.execute_blocking_action(optionBtn.executeAction, args=('CLICK', ()),
                    dialog_handler=handle_options_dialog_first_time)

            self.ui_test.execute_blocking_action(optionBtn.executeAction, args=('CLICK', ()),
                    dialog_handler=handle_options_dialog_second_time)

            self.ui_test.execute_blocking_action(optionBtn.executeAction, args=('CLICK', ()),
                    dialog_handler=handle_options_dialog_third_time)

            xOkBtn = dialog.getChild("ok")
            self.ui_test.close_dialog_through_button(xOkBtn)

        self.ui_test.execute_blocking_action(xPageChild.executeAction, args=('DOUBLECLICK', ()),
                dialog_handler=handle_dataField_dialog)

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
