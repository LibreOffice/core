# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file
from uitest.uihelper.common import select_pos
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues

#Bug 122509 - EDITING Calc crashes when manipulating Negative numbers in red on Currency ( not gtk3 )

class tdf122509(UITestCase):
    def test_tdf122509_format_cell_negative_numbers(self):
        #numberingformatpage.ui
        with self.ui_test.load_file(get_url_for_data_file("tdf122509.ods")) as calc_doc:
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")
            #select cell A1
            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
            #format - cell
            with self.ui_test.execute_dialog_through_command(".uno:FormatCellDialog") as xDialog:
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "0")  #tab Numbers
                xnegnumred = xDialog.getChild("negnumred")
                xformatted = xDialog.getChild("formatted")

                self.assertEqual(get_state_as_dict(xformatted)["Text"], "#,##0.00 [$USD];[RED]-#,##0.00 [$USD]")
                #click negative
                xnegnumred.executeAction("CLICK", tuple())
                #format check; https://bugs.documentfoundation.org/show_bug.cgi?id=122509#c7
                self.assertEqual(get_state_as_dict(xformatted)["Text"], "#,##0.00 [$USD];-#,##0.00 [$USD]")
                # click negative
                xnegnumred.executeAction("CLICK", tuple())
                #OK
            #verify; no crashes
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 0).getValue(), 2)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
