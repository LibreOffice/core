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

from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues


# Bug 123479 - Crash in: ScFormulaResult::GetMatrixFormulaCellToken()
class tdf123479(UITestCase):
    def test_tdf123479_Crash_ScFormulaResult_GetMatrixFormulaCellToken(self):
        #numberingformatpage.ui
        with self.ui_test.load_file(get_url_for_data_file("tdf123479.ods")) as calc_doc:
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")
            #Select D14:D16
            gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "D14:D16"}))
            #Open Formula Wizard (Ctrl+F2)
            with self.ui_test.execute_modeless_dialog_through_command(".uno:FunctionDialog") as xDialog:
                edArg1 = xDialog.getChild("ED_ARG1")
                edArg2 = xDialog.getChild("ED_ARG2")
                formulaRes = xDialog.getChild("formula_result")

                #check formula wizard data
                self.assertEqual(get_state_as_dict(edArg1)["Text"], "CHAR(10)")
                self.assertEqual(get_state_as_dict(edArg2)["Text"], "OFFSET($Data.$A$2:$Data.$A$4,0,COLUMN()-3)")
                self.assertEqual(get_state_as_dict(formulaRes)["Text"], "{4;4;4}")

            #verify; no crashes
            self.assertEqual(get_cell_by_position(calc_doc, 0, 1, 0).getString(), "Pass/Fail")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
