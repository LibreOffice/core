# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict
from uitest.uihelper.common import select_pos

class Tdf140528(UITestCase):

    def test_tdf140528(self):
        self.ui_test.create_doc_in_start_center("writer")
        document = self.ui_test.get_component()

        self.ui_test.execute_dialog_through_command(".uno:ChapterNumberingDialog")
        xDialog = self.xUITest.getTopFocusWindow()

        xTab = xDialog.getChild("tabcontrol")
        select_pos(xTab, "0")

        xFormat = xDialog.getChild("format")

        def handle_format_dlg(dialog):
            xEntry = dialog.getChild("entry")
            self.assertEqual("Untitled 1", get_state_as_dict(xEntry)['Text'])

            xEntry.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
            xEntry.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
            xEntry.executeAction("TYPE", mkPropertyValues({"TEXT" : "newFormat"}))

            xOKBtn = dialog.getChild("ok")
            self.ui_test.close_dialog_through_button(xOKBtn)

        self.ui_test.execute_blocking_action(xFormat.executeAction, args=('OPENFROMLIST', mkPropertyValues({"POS": "10"})),
                dialog_handler=handle_format_dlg)

        self.assertEqual("saveas", get_state_as_dict(xFormat)['CurrentItem'])

        # Go to Position tab
        select_pos(xTab, "1")

        # Go back to Numbering tab
        select_pos(xTab, "0")

        xFormat.executeAction('OPENFROMLIST', mkPropertyValues({"POS": "0"}))
        self.assertEqual("form1", get_state_as_dict(xFormat)['CurrentItem'])

        # Without the fix in place, this test would have crashed here
        select_pos(xTab, "1")

        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
