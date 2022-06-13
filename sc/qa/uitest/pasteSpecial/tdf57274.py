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
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues
from libreoffice.calc.paste_special import reset_default_values

#Bug:  Paste Special Link Checkbox fails to insert cell references when the source cell is blank

class tdf57274(UITestCase):

    def test_tdf57274_tdf116385_row_only(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf57274.ods")) as calc_doc:
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")

            #* Source Cells, range B6..E6
            gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "B6:E6"}))
            self.xUITest.executeCommand(".uno:Copy")
            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "B11"}))
            with self.ui_test.execute_dialog_through_command(".uno:PasteSpecial", close_button="") as xDialog:
                reset_default_values(self, xDialog)
                #We paste here using Paste Special with 'Link' Checkbox activated
                xLink = xDialog.getChild("link")
                xLink.executeAction("CLICK", tuple())

                xOkBtn = xDialog.getChild("ok")
                with self.ui_test.execute_blocking_action(xOkBtn.executeAction, args=('CLICK', ()), close_button="yes"):
                    pass

            #we would expect a reference to cell E6 here and a zero being displayed, but the cell is also simply blank.
            self.assertEqual(get_cell_by_position(calc_doc, 0, 4, 10).getValue(), 0)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 4, 10).getFormula(), "=$Sheet1.$E$6")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
