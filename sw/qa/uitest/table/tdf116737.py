# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict
from uitest.uihelper.common import select_pos

#Bug 116737 - INSERT TABLE: Can't select table style (gen/gtk)
class tdf116737(UITestCase):
    def test_tdf116737_select_table_style(self):

        self.ui_test.create_doc_in_start_center("writer")
        xWriterDoc = self.xUITest.getTopFocusWindow()
        document = self.ui_test.get_component()

        #Insert => Insert Table / It's not possible to select a table style
        self.ui_test.execute_dialog_through_command(".uno:InsertTable")
        xDialog = self.xUITest.getTopFocusWindow()

        formatlbinstable = xDialog.getChild("formatlbinstable")
        entry = formatlbinstable.getChild("11") #Simple List Shaded
        entry.executeAction("SELECT", tuple())

        xOkBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOkBtn)

        #verify .uno:TableDialog
        self.ui_test.execute_dialog_through_command(".uno:TableDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        xTabs = xDialog.getChild("tabcontrol")
        select_pos(xTabs, "4")   #tab Background

        btncolor = xDialog.getChild("btncolor")
        btncolor.executeAction("CLICK", tuple())

        R_custom = xDialog.getChild("R_custom")
        G_custom = xDialog.getChild("G_custom")
        B_custom = xDialog.getChild("B_custom")
        #"Simple List Shaded" -> header should be black
        self.assertEqual(get_state_as_dict(R_custom)["Text"], "0")
        self.assertEqual(get_state_as_dict(G_custom)["Text"], "0")
        self.assertEqual(get_state_as_dict(B_custom)["Text"], "0")

        xOkBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOkBtn)

        self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab:
