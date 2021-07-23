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

# Bug 143244 - Redo of adding table rows breaks table style after cut/paste action


class tdf143244(UITestCase):
    def test_tdf143244(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf143244.odt")) as writer_doc:
            self.xUITest.executeCommand(".uno:SelectAll")
            self.xUITest.executeCommand(".uno:Cut")
            self.xUITest.executeCommand(".uno:Paste")
            self.xUITest.executeCommand(".uno:GoUp")
            xWriterDoc = self.xUITest.getTopFocusWindow()
            xWriterEdit = xWriterDoc.getChild("writer_edit")
            for i in range(0,6):
                xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "TAB"}))
            for i in range(0,4):
                self.xUITest.executeCommand(".uno:Undo")
            for i in range(0,4):
                self.xUITest.executeCommand(".uno:Redo")
            for i in range (0,3):
                self.xUITest.executeCommand(".uno:GoUp")
                with self.ui_test.execute_dialog_through_command(".uno:TableDialog") as xDialog:
                    xTabs = xDialog.getChild("tabcontrol")
                    select_pos(xTabs, "4")   #tab Background
                    btncolor = xDialog.getChild("btncolor")
                    btncolor.executeAction("CLICK", tuple())
                    hex_custom = xDialog.getChild("hex_custom")
                    if i == 0:
                        self.assertEqual(get_state_as_dict(hex_custom)["Text"], "bee3d3")
                    if i == 1:
                        # Without the fix in place, this test would have failed with
                        # AssertionError: 'bee3d3' != 'ffffff'
                        self.assertEqual(get_state_as_dict(hex_custom)["Text"], "ffffff")
                    if i == 2:
                        self.assertEqual(get_state_as_dict(hex_custom)["Text"], "dddddd")
# vim: set shiftwidth=4 softtabstop=4 expandtab:
