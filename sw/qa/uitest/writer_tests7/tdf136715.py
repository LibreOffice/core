# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import select_pos

# Bug 136715 - Undoing table rows caused inconsistent table look when using table styles

class tdf136715(UITestCase):
    def test_tdf136715(self):
        self.ui_test.load_file(get_url_for_data_file("tdf136715.odt"))

        self.xUITest.executeCommand(".uno:GoDown")
        self.xUITest.executeCommand(".uno:GoDown")
        self.xUITest.executeCommand(".uno:LineDownSel")
        self.xUITest.executeCommand(".uno:DeleteRows")
        self.xUITest.executeCommand(".uno:Undo")

        self.ui_test.execute_dialog_through_command(".uno:FontDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        xTabs = xDialog.getChild("tabcontrol")
        select_pos(xTabs, "0")
        xweststylebcjk = xDialog.getChild("weststylelb-cjk")
        #without the fix in place, the row would have bold character formatting
        self.assertEqual(get_state_as_dict(xweststylebcjk)["Text"], "Regular")
        xOkBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOkBtn)

        self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab:
