# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict
from uitest.uihelper.common import select_pos
from uitest.uihelper.common import change_measurement_unit
from uitest.uihelper.calc import enter_text_to_cell
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues
import org.libreoffice.unotest
import pathlib
def get_url_for_data_file(file_name):
    return pathlib.Path(org.libreoffice.unotest.makeCopyFromTDOC(file_name)).as_uri()

#Bug 126673 - Calc: Auto-fit column and auto-fit row don't work with line break

class tdf126673(UITestCase):
    def test_tdf126673_auto_fit_row_height(self):
        calc_doc =  self.ui_test.load_file(get_url_for_data_file("tdf126673.ods"))
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()

        change_measurement_unit(self, "Centimeter")

        #select A2
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A2"}))
        #optimal row height
        self.ui_test.execute_dialog_through_command(".uno:SetOptimalRowHeight")
        xDialog = self.xUITest.getTopFocusWindow()
        # Click Ok
        xOK = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOK)
        #select A2
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A2"}))
        #row height
        self.ui_test.execute_dialog_through_command(".uno:RowHeight")
        xDialog = self.xUITest.getTopFocusWindow()
        xvalue = xDialog.getChild("value")
        heightStrOrig = get_state_as_dict(xvalue)["Text"]
        heightVal = heightStrOrig[:4]  #default 0.45 cm,
        self.assertEqual(heightVal > '0.45', True)  #new value is bigger then default
        # Click Ok
        xcancel = xDialog.getChild("cancel")
        self.ui_test.close_dialog_through_button(xcancel)

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
