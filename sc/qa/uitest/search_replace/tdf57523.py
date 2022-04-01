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
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict

class tdf57523(UITestCase):

   def test_tdf57523(self):
        with self.ui_test.create_doc_in_start_center("calc") as document:
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")

            enter_text_to_cell(gridwin, "A1", "AAA")
            enter_text_to_cell(gridwin, "A4", "AAA")

            gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:A4"}))

            with self.ui_test.execute_modeless_dialog_through_command(".uno:SearchDialog", close_button="close") as xDialog:

                # More Options is expanded when there is a selection with data
                xExpander = xDialog.getChild("OptionsExpander")
                self.assertEqual("true", get_state_as_dict(xExpander)['Expanded'])

                searchterm = xDialog.getChild("searchterm")
                searchterm.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
                searchterm.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
                searchterm.executeAction("TYPE", mkPropertyValues({"TEXT":"^$"}))

                replaceterm = xDialog.getChild("replaceterm")
                replaceterm.executeAction("TYPE", mkPropertyValues({"TEXT":"BBB"}))
                regexp = xDialog.getChild("regexp")

                regexp.executeAction("CLICK", tuple())
                self.assertEqual("true", get_state_as_dict(regexp)['Selected'])

                replaceall = xDialog.getChild("replaceall")
                replaceall.executeAction("CLICK", tuple())

                # Deselect regex button, otherwise it might affect other tests
                regexp.executeAction("CLICK", tuple())
                self.assertEqual("false", get_state_as_dict(regexp)['Selected'])


            # Without the fix in place, this test would have failed with
            # AssertionError: '' != 'BBB'
            self.assertEqual(get_cell_by_position(document, 0, 0, 1).getString(), "BBB")
            self.assertEqual(get_cell_by_position(document, 0, 0, 1).getString(), "BBB")


# vim: set shiftwidth=4 softtabstop=4 expandtab:
