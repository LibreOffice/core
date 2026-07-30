# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.calc import enter_text_to_cell
from uitest.uihelper.common import get_state_as_dict
from libreoffice.uno.propertyvalue import mkPropertyValues

class tdf97354(UITestCase):

    def test_tdf97354_formula_bar_visible_ref_dialog(self):
        with self.ui_test.create_doc_in_start_center("calc"):
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")
            enter_text_to_cell(gridwin, "A1", "=1")

            xInputWin = xCalcDoc.getChild("sc_input_window")
            # Re-enter cell edit mode so the content of the formula is still being edited
            gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "F2"}))
            self.assertEqual("=1", get_state_as_dict(xInputWin)["Text"])

            with self.ui_test.execute_modeless_dialog_through_command(".uno:ConditionalFormatDialog", close_button="cancel"):
                self.ui_test.wait_until_property_is_updated(xInputWin, "Enabled", "false")

                # Without the fix in place, this test would have failed with
                # AssertionError: '=1' != ''
                self.assertEqual("=1", get_state_as_dict(xInputWin)["Text"])

# vim: set shiftwidth=4 softtabstop=4 expandtab:
