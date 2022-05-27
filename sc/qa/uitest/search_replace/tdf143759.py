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
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict

class tdf143759(UITestCase):

   def test_tdf143759(self):
        with self.ui_test.create_doc_in_start_center("calc"):
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")

            enter_text_to_cell(gridwin, "C2", "A")
            enter_text_to_cell(gridwin, "E3", "1")
            enter_text_to_cell(gridwin, "B5", "2")

            gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:F6"}))

            with self.ui_test.execute_modeless_dialog_through_command(".uno:SearchDialog", close_button="close") as xDialog:

                # More Options is expanded when there is a selection with data
                xExpander = xDialog.getChild("OptionsExpander")
                self.assertEqual("true", get_state_as_dict(xExpander)['Expanded'])

                xSearchterm = xDialog.getChild("searchterm")
                xSearchterm.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
                xSearchterm.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
                xSearchterm.executeAction("TYPE", mkPropertyValues({"TEXT":"^$"}))

                xRegexp = xDialog.getChild("regexp")
                xRegexp.executeAction("CLICK", tuple())
                self.assertEqual("true", get_state_as_dict(xRegexp)['Selected'])

                xSelection = xDialog.getChild("selection")
                if get_state_as_dict(xSelection)['Selected'] == 'false':
                    xSelection.executeAction("CLICK", tuple())
                self.assertEqual("true", get_state_as_dict(xSelection)['Selected'])

                xSearchall = xDialog.getChild("searchall")
                with self.ui_test.execute_dialog_through_action(
                        xSearchall, "CLICK", event_name = "ModelessDialogVisible", close_button="close") as dialog:
                    # Without the fix in place, this test would have failed with
                    # AssertionError: 13 != 0
                    self.assertEqual(13, len(dialog.getChild("results").getChildren()))

                # Deselect regex button, otherwise it might affect other tests
                xRegexp.executeAction("CLICK", tuple())
                self.assertEqual("false", get_state_as_dict(xRegexp)['Selected'])

# vim: set shiftwidth=4 softtabstop=4 expandtab:
