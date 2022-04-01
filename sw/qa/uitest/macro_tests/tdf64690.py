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
from libreoffice.uno.propertyvalue import mkPropertyValues

class tdf64690(UITestCase):

    def test_tdf64690(self):

        with self.ui_test.create_doc_in_start_center("writer"):

            with self.ui_test.execute_dialog_through_command(".uno:MacroDialog", close_button="edit"):
                pass

            xMacroWin = self.xUITest.getTopFocusWindow()
            xEditWin = xMacroWin.getChild('EditorWindow')

            self.xUITest.executeCommand(".uno:SelectAll")
            xEditWin.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))

            self.assertEqual("\n", get_state_as_dict(xEditWin)['Text'])

            xEditWin.executeAction("TYPE", mkPropertyValues({"TEXT":"'abc"}))
            xEditWin.executeAction("TYPE", mkPropertyValues({"KEYCODE":"RETURN"}))
            xEditWin.executeAction("TYPE", mkPropertyValues({"TEXT":"'def"}))

            self.assertEqual("'abc\n'def\n", get_state_as_dict(xEditWin)['Text'])

            self.xUITest.executeCommand(".uno:SelectAll")

            with self.ui_test.execute_modeless_dialog_through_command(".uno:SearchDialog", close_button="close") as xDialog:

                searchterm = xDialog.getChild("searchterm")
                searchterm.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
                searchterm.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
                searchterm.executeAction("TYPE", mkPropertyValues({"TEXT":"."}))

                replaceterm = xDialog.getChild("replaceterm")
                replaceterm.executeAction("TYPE", mkPropertyValues({"TEXT":"ABC"}))

                regexp = xDialog.getChild("regexp")
                regexp.executeAction("CLICK", tuple())
                self.assertEqual("true", get_state_as_dict(regexp)['Selected'])

                selection = xDialog.getChild("selection")
                if get_state_as_dict(selection)['Selected'] == 'false':
                    selection.executeAction("CLICK", tuple())
                self.assertEqual("true", get_state_as_dict(selection)['Selected'])

                replaceall = xDialog.getChild("replaceall")

                # Without the fix in place, this test would have hung here
                with self.ui_test.execute_blocking_action(replaceall.executeAction, args=('CLICK', ())):
                    pass

                # Deselect regex button, otherwise it might affect other tests
                regexp.executeAction("CLICK", tuple())
                self.assertEqual("false", get_state_as_dict(regexp)['Selected'])


            self.assertEqual("ABCABCABCABC\nABCABCABCABC\n", get_state_as_dict(xEditWin)['Text'])



# vim: set shiftwidth=4 softtabstop=4 expandtab:
