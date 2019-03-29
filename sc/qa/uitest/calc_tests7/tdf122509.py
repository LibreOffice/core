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
from libreoffice.calc.document import get_sheet_from_doc
from libreoffice.calc.conditional_format import get_conditional_format_from_sheet
from uitest.debug import sleep
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues
import org.libreoffice.unotest
import pathlib

def get_url_for_data_file(file_name):
    return pathlib.Path(org.libreoffice.unotest.makeCopyFromTDOC(file_name)).as_uri()

#Bug 122509 - EDITING Calc crashes when manipulating Negative numbers in red on Currency ( not gtk3 )

class tdf122509(UITestCase):
    def test_tdf122509_format_cell_negative_numbers(self):
        #numberingformatpage.ui
        calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf122509.ods"))
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        #select cell A1
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
        #format - cell
        self.ui_test.execute_dialog_through_command(".uno:FormatCellDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        xTabs = xDialog.getChild("tabcontrol")
        select_pos(xTabs, "0")  #tab Numbers
        xliststore1 = xDialog.getChild("categorylb")  #1st list / Category
        xliststore2 = xDialog.getChild("formatlb")  #2nd list / Format
        xdecimalsed = xDialog.getChild("decimalsed")
        xleadzerosed = xDialog.getChild("leadzerosed")
        xnegnumred = xDialog.getChild("negnumred")
        xthousands = xDialog.getChild("thousands")
        xlanguagelb = xDialog.getChild("languagelb")
        xformatted = xDialog.getChild("formatted")

        self.assertEqual(get_state_as_dict(xformatted)["Text"], "#,##0.00 [$USD];[RED]-#,##0.00 [$USD]")
        #click negative
        xnegnumred.executeAction("CLICK", tuple())
        #format check; https://bugs.documentfoundation.org/show_bug.cgi?id=122509#c7
        self.assertEqual(get_state_as_dict(xformatted)["Text"], "#,##0.00 [$USD];-#,##0.00 [$USD]")
        # click negative
        xnegnumred.executeAction("CLICK", tuple())
        #OK
        xOk = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOk)
        #verify; no crashes
        self.assertEqual(get_cell_by_position(document, 0, 0, 0).getValue(), 2)

        self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab:
