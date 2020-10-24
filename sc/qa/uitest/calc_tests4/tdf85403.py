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

#Bug 85403 - no broadcast after text to columns to convert text to numbers
def get_url_for_data_file(file_name):
    return pathlib.Path(org.libreoffice.unotest.makeCopyFromTDOC(file_name)).as_uri()

class tdf85403(UITestCase):
    def test_tdf85403_text_to_columns(self):
        calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf85403.ods"))
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()

        #'123 in A1, SUM(A1) in B1, result is 0 as expected,
        #now select A1 and use data->text to columns->ok and B1 is not updated,
        #putting a new SUM(A1) in C1 will show 123
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
        # Data - Text to Columns
        self.ui_test.execute_dialog_through_command(".uno:TextToColumns")
        xDialog = self.xUITest.getTopFocusWindow()
        # Click Ok
        xOK = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOK)

        #Verify
        self.assertEqual(get_cell_by_position(document, 0, 1, 0).getValue(), 123)

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
