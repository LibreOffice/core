# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.calc import enter_text_to_cell
from libreoffice.calc.document import get_cell_by_position

class tdf118308(UITestCase):

    def test_tdf118308(self):
        calc_doc = self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()

        enter_text_to_cell(gridwin, "A1", "A")
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
        self.xUITest.executeCommand(".uno:Copy")

        self.ui_test.close_doc()

        calc_doc = self.ui_test.load_empty_file("calc")

        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()

        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))

        self.ui_test.execute_dialog_through_command(".uno:PasteSpecial")
        xDialog = self.xUITest.getTopFocusWindow()

        xText = xDialog.getChild("text")
        xNumbers = xDialog.getChild("numbers")
        xDatetime = xDialog.getChild("datetime")
        xFormats = xDialog.getChild("formats")

        self.assertEqual("true", get_state_as_dict(xText)["Selected"])
        self.assertEqual("true", get_state_as_dict(xNumbers)["Selected"])
        self.assertEqual("true", get_state_as_dict(xDatetime)["Selected"])
        self.assertEqual("false", get_state_as_dict(xFormats)["Selected"])

        xOkBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOkBtn)

        self.assertEqual("A", get_cell_by_position(document, 0, 0, 0).getString())
        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:

