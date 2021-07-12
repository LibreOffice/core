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

class Tdf92503(UITestCase):

    def test_tdf92503(self):

        # Load file from Open dialog
        with self.ui_test.execute_dialog_through_command(".uno:Open", close_button="open") as xOpenDialog:

            xFileName = xOpenDialog.getChild("file_name")
            xFileName.executeAction("TYPE", mkPropertyValues({"TEXT": get_url_for_data_file("tdf92503.csv")}))

        xDialog = self.ui_test.wait_for_top_focus_window('TextImportCsvDialog')
        xFixedWidth = xDialog.getChild("tofixedwidth")
        xGrid = xDialog.getChild("csvgrid")
        xColumnType = xDialog.getChild("columntype")

        self.assertEqual('false', get_state_as_dict(xFixedWidth)['Checked'])
        xFixedWidth.executeAction("CLICK", tuple())
        self.assertEqual('true', get_state_as_dict(xFixedWidth)['Checked'])

        # Use the right arrow to put the focus in the grid
        xGrid.executeAction("TYPE", mkPropertyValues({"KEYCODE":"RIGHT"}))

        self.assertEqual('true', get_state_as_dict(xColumnType)['Enabled'])

        xColumnType.executeAction("SELECT", mkPropertyValues({"TEXT": "Date (DMY)"}))

        self.assertEqual('Date (DMY)', get_state_as_dict(xColumnType)['SelectEntryText'])

        xOK = xDialog.getChild('ok')
        with self.ui_test.wait_until_component_loaded():
            self.ui_test.close_dialog_through_button(xOK)

        document = self.ui_test.get_component()

        self.assertEqual("03/31/13 01:00 AM", get_cell_by_position(document, 0, 0, 0).getString())

        # Without the fix in place, this test would have failed with
        # AssertionError: '03/31/13 02:00 AM' != '31 Mar 2013, 02:00'
        self.assertEqual("03/31/13 02:00 AM", get_cell_by_position(document, 0, 0, 1).getString())
        self.assertEqual("03/31/13 03:00 AM", get_cell_by_position(document, 0, 0, 2).getString())
        self.assertEqual("03/30/14 01:00 AM", get_cell_by_position(document, 0, 0, 3).getString())
        self.assertEqual("03/30/14 02:00 AM", get_cell_by_position(document, 0, 0, 4).getString())
        self.assertEqual("03/30/14 03:00 AM", get_cell_by_position(document, 0, 0, 5).getString())
        self.assertEqual("03/29/15 01:00 AM", get_cell_by_position(document, 0, 0, 6).getString())
        self.assertEqual("03/29/15 02:00 AM", get_cell_by_position(document, 0, 0, 7).getString())
        self.assertEqual("03/29/15 03:00 AM", get_cell_by_position(document, 0, 0, 8).getString())

# vim: set shiftwidth=4 softtabstop=4 expandtab:
