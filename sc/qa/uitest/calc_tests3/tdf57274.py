# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
import org.libreoffice.unotest
import os
import pathlib
from uitest.uihelper.common import get_state_as_dict
from libreoffice.calc.document import get_sheet_from_doc
from libreoffice.calc.conditional_format import get_conditional_format_from_sheet
from uitest.debug import sleep
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues
#Bug:  Paste Special Link Checkbox fails to insert cell references when the source cell is blank
def get_url_for_data_file(file_name):
    return pathlib.Path(org.libreoffice.unotest.makeCopyFromTDOC(file_name)).as_uri()

class tdf57274(UITestCase):

    def test_tdf57274_tdf116385_row_only(self):
        calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf57274.ods"))
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()

        #* Source Cells, range B6..E6
        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "B6:E6"}))
        self.xUITest.executeCommand(".uno:Copy")
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "B11"}))
        self.ui_test.execute_dialog_through_command(".uno:PasteSpecial")
        xDialog = self.xUITest.getTopFocusWindow()
        #We paste here using Paste Special with 'Link' Checkbox activated
        xLink = xDialog.getChild("link")
        xLink.executeAction("CLICK", tuple())
        xOkBtn = xDialog.getChild("ok")
        # self.ui_test.close_dialog_through_button(xOkBtn)
        def handle_confirm_dlg(dialog):
            xOKBtn = dialog.getChild("yes")
            self.ui_test.close_dialog_through_button(xOKBtn)

        self.ui_test.execute_blocking_action(xOkBtn.executeAction, args=('CLICK', ()),
                dialog_handler=handle_confirm_dlg)
        #we would expect a reference to cell E6 here and a zero being displayed, but the cell is also simply blank.
        self.assertEqual(get_cell_by_position(document, 0, 4, 10).getValue(), 0)
        self.assertEqual(get_cell_by_position(document, 0, 4, 10).getFormula(), "=$Sheet1.$E$6")
        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab: