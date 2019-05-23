#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import select_pos, get_state_as_dict
from com.sun.star.uno import RuntimeException
from libreoffice.uno.propertyvalue import mkPropertyValues

#Bug 125104 - Changing page numbering to "1st, 2nd, 3rd,..." causes crashes when trying to change Page settings later

class tdf125104(UITestCase):

    def test_tdf125104_pageFormat_numbering(self):
        self.ui_test.create_doc_in_start_center("writer")
        document = self.ui_test.get_component()

        self.ui_test.execute_dialog_through_command(".uno:PageDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        tabcontrol = xDialog.getChild("tabcontrol")
        select_pos(tabcontrol, "1")

        comboLayoutFormat = xDialog.getChild("comboLayoutFormat")
        props = {"TEXT": "1st, 2nd, 3rd, ..."}
        actionProps = mkPropertyValues(props)
        comboLayoutFormat.executeAction("SELECT", actionProps)

        okBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(okBtn)

        self.ui_test.execute_dialog_through_command(".uno:PageDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        tabcontrol = xDialog.getChild("tabcontrol")
        select_pos(tabcontrol, "1")
        comboLayoutFormat = xDialog.getChild("comboLayoutFormat")
        self.assertEqual(get_state_as_dict(comboLayoutFormat)["SelectEntryText"], "1st, 2nd, 3rd, ...")
        cancelBtn = xDialog.getChild("cancel")
        self.ui_test.close_dialog_through_button(cancelBtn)

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
