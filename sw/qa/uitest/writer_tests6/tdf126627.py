# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import select_pos
from uitest.uihelper.calc import enter_text_to_cell
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file, type_text

#Bug 126627 - CRASH: undoing redlinehide deletion

class tdf126627(UITestCase):
    def test_tdf126627_crash_undo_deletion(self):
        writer_doc = self.ui_test.load_file(get_url_for_data_file("tdf126627.odt"))
        document = self.ui_test.get_component()
        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")
        self.ui_test.execute_modeless_dialog_through_command(".uno:SearchDialog")
        xDialog = self.xUITest.getTopFocusWindow()

        searchterm = xDialog.getChild("searchterm")
        searchterm.executeAction("TYPE", mkPropertyValues({"TEXT":"bar"}))
        xsearch = xDialog.getChild("search")
        xsearch.executeAction("CLICK", tuple())

        xcloseBtn = xDialog.getChild("close")
        self.ui_test.close_dialog_through_button(xcloseBtn)
        #2. Place the cursor before 'bar'
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "LEFT"}))
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "LEFT"}))
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "LEFT"}))
        #3. Hit backspace 4 times -> Foo and bar are together
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "BACKSPACE"}))
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "BACKSPACE"}))
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "BACKSPACE"}))
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "BACKSPACE"}))
        self.assertEqual(document.Text.String[0:6], "foobar")
        #4. Undo once-> Crash
        self.xUITest.executeCommand(".uno:Undo")
        #verify that we have now two words
        self.ui_test.execute_modeless_dialog_through_command(".uno:WordCountDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        xselectwords = xDialog.getChild("selectwords")
        self.assertEqual(get_state_as_dict(xselectwords)["Text"], "2")
        xCloseBtn = xDialog.getChild("close")
        self.ui_test.close_dialog_through_button(xCloseBtn)

        self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab:
