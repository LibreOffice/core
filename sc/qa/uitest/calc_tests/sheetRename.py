# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict
from libreoffice.uno.propertyvalue import mkPropertyValues

class sheetRename(UITestCase):
    def test_sheet_rename(self):
        calc_doc = self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        self.ui_test.execute_dialog_through_command(".uno:RenameTable")
        xDialog = self.xUITest.getTopFocusWindow()
        xname_entry = xDialog.getChild("name_entry")
        xname_entry.executeAction("TYPE", mkPropertyValues({"TEXT":"NewName"}))
        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)
        #Verify
        self.ui_test.execute_dialog_through_command(".uno:RenameTable")
        xDialog = self.xUITest.getTopFocusWindow()
        xname_entry = xDialog.getChild("name_entry")
        self.assertEqual(get_state_as_dict(xname_entry)["Text"], "NewName")
        xCancelBtn = xDialog.getChild("cancel")
        self.ui_test.close_dialog_through_button(xCancelBtn)

        self.ui_test.close_doc()

    def test_sheet_rename_invalid_sheet_name(self):
        calc_doc = self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        self.ui_test.execute_dialog_through_command(".uno:RenameTable")
        xDialog = self.xUITest.getTopFocusWindow()
        xname_entry = xDialog.getChild("name_entry")
        nameVal = get_state_as_dict(xname_entry)["Text"]
        xname_entry.executeAction("TYPE", mkPropertyValues({"TEXT":"NewName**"}))
        xOKBtn = xDialog.getChild("ok")
        def handle_warn_dlg(dialog):
            #show warning
            xok = dialog.getChild("ok")
            self.ui_test.close_dialog_through_button(xok)

        self.ui_test.execute_blocking_action(xOKBtn.executeAction, args=('CLICK', ()),
                dialog_handler=handle_warn_dlg)
        xCancelBtn = xDialog.getChild("cancel")
        self.ui_test.close_dialog_through_button(xCancelBtn)

        #Verify
        self.ui_test.execute_dialog_through_command(".uno:RenameTable")
        xDialog = self.xUITest.getTopFocusWindow()
        xname_entry = xDialog.getChild("name_entry")
        self.assertEqual(get_state_as_dict(xname_entry)["Text"], nameVal)
        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        self.ui_test.close_doc()

#    def test_tdf81431_rename_sheet_clipboard_content_wiped_out(self):
#        calc_doc = self.ui_test.create_doc_in_start_center("calc")
#        xCalcDoc = self.xUITest.getTopFocusWindow()
#        gridwin = xCalcDoc.getChild("grid_window")
#        document = self.ui_test.get_component()
#        #enter text and copy text to clipboard
#        enter_text_to_cell(gridwin, "A1", "String")
#        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
#        self.xUITest.executeCommand(".uno:Copy")
#        #rename sheet
#        self.ui_test.execute_dialog_through_command(".uno:RenameTable")
#        xDialog = self.xUITest.getTopFocusWindow()
#        xname_entry = xDialog.getChild("name_entry")
#        nameVal = get_state_as_dict(xname_entry)["Text"]
#        xname_entry.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
#        xname_entry.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
#        xname_entry.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+V"}))

#        #fails here - text is not pasted
#        self.assertEqual(get_state_as_dict(xname_entry)["Text"], "String")

#        xOKBtn = xDialog.getChild("ok")
#        self.ui_test.close_dialog_through_button(xOKBtn)
#        #paste text to cell
#        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "C1"}))
#        self.xUITest.executeCommand(".uno:Paste")
#        self.assertEqual(get_cell_by_position(document, 0, 2, 0).getString(), "String")
#        #undo
#        self.xUITest.executeCommand(".uno:Undo")
#        self.xUITest.executeCommand(".uno:Undo")
#        #verify undo cell paste
#        self.assertEqual(get_cell_by_position(document, 0, 2, 0).getString(), "")
#        #verify undo sheet rename
#        self.ui_test.execute_dialog_through_command(".uno:RenameTable")
#        xDialog = self.xUITest.getTopFocusWindow()
#        xname_entry = xDialog.getChild("name_entry")
#        self.assertEqual(get_state_as_dict(xname_entry)["Text"], nameVal)
#        xOKBtn = xDialog.getChild("ok")
#        self.ui_test.close_dialog_through_button(xOKBtn)

#        self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab:
