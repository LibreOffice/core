# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import change_measurement_unit
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file

from libreoffice.uno.propertyvalue import mkPropertyValues


# Bug 126673 - Calc: Auto-fit column and auto-fit row don't work with line break
class tdf126673(UITestCase):
    def test_tdf126673_auto_fit_row_height(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf126673.ods")) as calc_doc:
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")

            change_measurement_unit(self, "Centimeter")

            #select A2
            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A2"}))
            #optimal row height
            with self.ui_test.execute_dialog_through_command(".uno:SetOptimalRowHeight") as xDialog:
                xvalue = xDialog.getChild("value")
                self.assertEqual("0.00 cm", get_state_as_dict(xvalue)["Text"])

            #select A2
            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A2"}))
            #row height
            with self.ui_test.execute_dialog_through_command(".uno:RowHeight", close_button="cancel") as xDialog:
                xvalue = xDialog.getChild("value")
                self.assertEqual("1.24 cm", get_state_as_dict(xvalue)["Text"])

# vim: set shiftwidth=4 softtabstop=4 expandtab:
