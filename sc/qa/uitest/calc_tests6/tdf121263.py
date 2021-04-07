# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from libreoffice.uno.propertyvalue import mkPropertyValues

from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict

class tdf121263(UITestCase):

    def test_tdf121263_hide_more_sheets(self):
        calc_doc = self.ui_test.create_doc_in_start_center("calc")
        xTopWindow = self.xUITest.getTopFocusWindow()
        document = self.ui_test.get_component()
        gridwin = xTopWindow.getChild("grid_window")

        self.ui_test.execute_dialog_through_command(".uno:Insert")  #insert sheet
        xDialog = self.xUITest.getTopFocusWindow()
        xOKButton = xDialog.getChild("ok")
        xOKButton.executeAction("CLICK", tuple())
        self.ui_test.execute_dialog_through_command(".uno:Insert")  #insert sheet
        xDialog = self.xUITest.getTopFocusWindow()
        xOKButton = xDialog.getChild("ok")
        xOKButton.executeAction("CLICK", tuple())

        gridwin.executeAction("SELECT", mkPropertyValues({"TABLE": "2"}))
        #select previous sheet
        self.xUITest.executeCommand(".uno:JumpToPrevTableSel")

        #hide sheet
        self.xUITest.executeCommand(".uno:Hide")

        #show sheet Dialog
        self.ui_test.execute_dialog_through_command(".uno:Show")
        xDialog = self.xUITest.getTopFocusWindow()
        treeview = xDialog.getChild("treeview")
        self.assertEqual(get_state_as_dict(treeview)["Children"], "2")
        xcancel = xDialog.getChild("cancel")
        self.ui_test.close_dialog_through_button(xcancel)

        self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab:
