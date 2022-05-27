# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict
from uitest.uihelper.calc import enter_text_to_cell
from libreoffice.uno.propertyvalue import mkPropertyValues

class tdf37864(UITestCase):

    def test_tdf37864(self):

        with self.ui_test.create_doc_in_start_center("calc"):
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")
            enter_text_to_cell(gridwin, "A1", "test")

            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))

            self.xUITest.executeCommand(".uno:Copy")

            with self.ui_test.execute_modeless_dialog_through_command(".uno:FunctionDialog") as xDialog:
                xFormula = xDialog.getChild("ed_formula")
                xFormula.executeAction("TYPE", mkPropertyValues({"KEYCODE": 'CTRL+v'}))

                # Without the fix in place, this test would have failed with
                # AssertionError: '=test' != '=test\n'
                self.assertEqual("=test", get_state_as_dict(xFormula)["Text"])

# vim: set shiftwidth=4 softtabstop=4 expandtab:
