# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import select_pos
from uitest.uihelper.common import select_by_text
from uitest.uihelper.common import get_state_as_dict

class tdf115665(UITestCase):

    def test_tdf115665(self):

        with self.ui_test.create_doc_in_start_center("writer"):
            xWriterDoc = self.xUITest.getTopFocusWindow()
            xWriterEdit = xWriterDoc.getChild("writer_edit")

            # Type a line to use for search
            xWriterEdit.executeAction("TYPE", mkPropertyValues({"TEXT": "This is happiness."}))

            # Open the F&R dialog and set to find search term with attribute format UPPERCASE
            with self.ui_test.execute_modeless_dialog_through_command(".uno:SearchDialog", close_button="close") as xDialog:
                format = xDialog.getChild("format")
                with self.ui_test.execute_blocking_action(format.executeAction, args=('CLICK', ())) as dialog:
                    xTabs = dialog.getChild("tabcontrol")
                    select_pos(xTabs, "1")
                    xEffectsLb = dialog.getChild("effectslb")
                    select_by_text(xEffectsLb, "UPPERCASE")

            # Open the find bar
            xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "CTRL+f"}))

            # Type "happiness" in the find bar search edit box
            xfind = xWriterDoc.getChild("find")
            xfind.executeAction("TYPE", mkPropertyValues({"TEXT": "happiness"}))

            xfind_bar = xWriterDoc.getChild("FindBar")

            # Press on Find All in the find bar
            xfind_bar.executeAction("CLICK", mkPropertyValues({"POS": "4"})) # 4 is Find All

            # Without the fix in place the test would fail with
            # AssertionError: '' != 'happiness'
            self.assertEqual(get_state_as_dict(xWriterEdit)["SelectedText"], "happiness")

            # Close the find bar
            xfind_bar.executeAction("CLICK", mkPropertyValues({"POS": "0"}))  # 0 is pos for close

# vim: set shiftwidth=4 softtabstop=4 expandtab:
