# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict
from uitest.uihelper.common import select_pos
from uitest.uihelper.calc import enter_text_to_cell
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues
#uitest sc / Show sheet dialog

class hideShowSheet(UITestCase):
    def test_hide_show_sheet(self):
        writer_doc = self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        #insert sheet
        self.ui_test.execute_dialog_through_command(".uno:Insert")
        current_dialog = self.xUITest.getTopFocusWindow()
        xOkButton = current_dialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOkButton)
        #select sheet
        gridwin.executeAction("SELECT", mkPropertyValues({"TABLE":"1"}))
        self.assertEqual(get_state_as_dict(gridwin)["SelectedTable"], "1")
        #hide sheet
        self.xUITest.executeCommand(".uno:Hide")
        #show sheet Dialog
        self.ui_test.execute_dialog_through_command(".uno:Show")
        xDialog = self.xUITest.getTopFocusWindow()
        treeview = xDialog.getChild("treeview")
        self.assertEqual(get_state_as_dict(treeview)["Children"], "1")
        xcancel = xDialog.getChild("cancel")
        self.ui_test.close_dialog_through_button(xcancel)
        #insert 2nd sheet
        self.ui_test.execute_dialog_through_command(".uno:Insert")
        current_dialog = self.xUITest.getTopFocusWindow()
        xOkButton = current_dialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOkButton)
        #select sheet
        gridwin.executeAction("SELECT", mkPropertyValues({"TABLE":"2"}))
        self.assertEqual(get_state_as_dict(gridwin)["SelectedTable"], "1")
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
