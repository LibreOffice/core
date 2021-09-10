# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file

#Bug 126627 - CRASH: undoing redlinehide deletion

class tdf126627(UITestCase):
    def test_tdf126627_crash_undo_deletion(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf126627.odt")) as writer_doc:
            xWriterDoc = self.xUITest.getTopFocusWindow()
            xWriterEdit = xWriterDoc.getChild("writer_edit")
            with self.ui_test.execute_modeless_dialog_through_command_guarded(".uno:SearchDialog", close_button="close") as xDialog:

                searchterm = xDialog.getChild("searchterm")
                searchterm.executeAction("TYPE", mkPropertyValues({"TEXT":"bar"}))
                xsearch = xDialog.getChild("search")
                xsearch.executeAction("CLICK", tuple())

            #2. Place the cursor before 'bar'
            xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "LEFT"}))
            xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "LEFT"}))
            xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "LEFT"}))
            #3. Hit backspace 4 times -> Foo and bar are together
            xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "BACKSPACE"}))
            xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "BACKSPACE"}))
            xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "BACKSPACE"}))
            xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "BACKSPACE"}))
            self.assertEqual(writer_doc.Text.String[0:6], "foobar")
            #4. Undo once-> Crash
            self.xUITest.executeCommand(".uno:Undo")
            #verify that we have now two words
            with self.ui_test.execute_modeless_dialog_through_command_guarded(".uno:WordCountDialog", close_button="close") as xDialog:
                xselectwords = xDialog.getChild("selectwords")
                self.assertEqual(get_state_as_dict(xselectwords)["Text"], "2")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
