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


#Bug 123479 - Crash in: ScFormulaResult::GetMatrixFormulaCellToken()

class tdf123479(UITestCase):
    def test_tdf123479_Crash_ScFormulaResult_GetMatrixFormulaCellToken(self):
        #numberingformatpage.ui
        calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf123479.ods"))
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        #Select D14:D16
        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "D14:D16"}))
        #Open Formula Wizard (Ctrl+F2)
        self.ui_test.execute_modeless_dialog_through_command(".uno:FunctionDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        edArg1 = xDialog.getChild("ED_ARG1")
        edArg2 = xDialog.getChild("ED_ARG2")
        formulaRes = xDialog.getChild("formula_result")

        #check formula wizard data
        self.assertEqual(get_state_as_dict(edArg1)["Text"], "CHAR(10)")
        self.assertEqual(get_state_as_dict(edArg2)["Text"], "OFFSET($Data.$A$2:$Data.$A$4,0,COLUMN()-3)")
        self.assertEqual(get_state_as_dict(formulaRes)["Text"], "{4;4;4}")

        xOk = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOk)
        #verify; no crashes
        self.assertEqual(get_cell_by_position(document, 0, 1, 0).getString(), "Pass/Fail")

        self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab:
