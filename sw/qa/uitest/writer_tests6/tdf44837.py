#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import select_pos, get_state_as_dict, type_text
from com.sun.star.uno import RuntimeException
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.debug import sleep
#Bug 44837 - EDITING 'Replace All' clears current selection

class tdf44837(UITestCase):

    def test_tdf448373_Replace_All_clears_current_selection(self):
        self.ui_test.create_doc_in_start_center("writer")
        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")
        document = self.ui_test.get_component()

        type_text(xWriterEdit, "asd asd")
        self.xUITest.executeCommand(".uno:SelectAll")

        self.ui_test.execute_modeless_dialog_through_command(".uno:SearchDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        searchterm = xDialog.getChild("searchterm")
        searchterm.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
        searchterm.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
        searchterm.executeAction("TYPE", mkPropertyValues({"TEXT":"a"}))
        replaceterm = xDialog.getChild("replaceterm")
        replaceterm.executeAction("TYPE", mkPropertyValues({"TEXT":"b"})) #replace textbox

        selectionOnly = xDialog.getChild("selection")
        selectionOnly.executeAction("CLICK", tuple())
        replaceall = xDialog.getChild("replaceall")
        replaceall.executeAction("CLICK", tuple())
        xcloseBtn = xDialog.getChild("close")
        self.ui_test.close_dialog_through_button(xcloseBtn)

        self.assertEqual(document.Text.String[0:8], "bsd bsd")
        self.assertEqual(len(document.CurrentSelection.getByIndex(0).String) > 1, True)
        #follow-up bug 125663
        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
