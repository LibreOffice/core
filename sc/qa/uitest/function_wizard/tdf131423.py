# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file
from uitest.uihelper.common import select_pos
from uitest.uihelper.calc import enter_text_to_cell
from libreoffice.calc.document import get_sheet_from_doc
from libreoffice.calc.conditional_format import get_conditional_format_from_sheet
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues


#Bug 131423 - Function Wizard in Calc allows too many parameters

class tdf131423(UITestCase):
    def test_tdf131423_improduct_too_many_parameters(self):
        calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf131423.ods"))
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        #for 255 complex numbers
        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "B1"}))
        #Open Formula Wizard (Ctrl+F2)
        self.ui_test.execute_modeless_dialog_through_command(".uno:FunctionDialog")
        xDialog = self.xUITest.getTopFocusWindow()

        xFormula = xDialog.getChild("ed_formula")
        xFormula.executeAction("TYPE", mkPropertyValues({"TEXT": 'IMPRODUCT(A1:A255)'}))

        #check formula wizard data
        xOk = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOk)

        #verify; no crashes
        self.assertEqual(get_cell_by_position(document, 0, 1, 0).getString(), "-j")

        #for 256 complex numbers
        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "B2"}))
        #Open Formula Wizard (Ctrl+F2)
        self.ui_test.execute_modeless_dialog_through_command(".uno:FunctionDialog")
        xDialog = self.xUITest.getTopFocusWindow()

        xFormula = xDialog.getChild("ed_formula")
        xFormula.executeAction("TYPE", mkPropertyValues({"TEXT": 'IMPRODUCT(A1:A256)'}))

        #check formula wizard data
        xOk = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOk)

        #verify; no crashes
        self.assertEqual(get_cell_by_position(document, 0, 1, 1).getString(), "1")

        self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab:
