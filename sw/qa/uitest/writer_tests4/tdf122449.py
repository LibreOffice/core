# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file

#Bug 122449 - Crash in: mergedlo.dll when closing "Edit Index Entry" dialog (gen/gtk)

class tdf122449(UITestCase):

    def test_tdf122449_crash_edit_index_entry(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf122449.odt")) as writer_doc:
            xWriterDoc = self.xUITest.getTopFocusWindow()
            xWriterEdit = xWriterDoc.getChild("writer_edit")
            #search word Autocorrect (second find)   .uno:SearchDialog
            with self.ui_test.execute_modeless_dialog_through_command_guarded(".uno:SearchDialog", close_button="close") as xDialog:

                searchterm = xDialog.getChild("searchterm")
                searchterm.executeAction("TYPE", mkPropertyValues({"TEXT":"Autocorrection"}))
                xsearch = xDialog.getChild("search")
                xsearch.executeAction("CLICK", tuple())  #first search
                xsearch.executeAction("CLICK", tuple())  #2nd search
                self.assertEqual(get_state_as_dict(xWriterEdit)["CurrentPage"], "6")
            xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "HOME"}))
            # invoke Index dialog Index entry   .uno:IndexEntryDialog
            with self.ui_test.execute_dialog_through_command(".uno:IndexEntryDialog", close_button="close"):
                pass
            # close
            # Go to page 2
            with self.ui_test.execute_dialog_through_command(".uno:GotoPage") as xDialog:
                xPageText = xDialog.getChild("page")
                xPageText.executeAction("TYPE", mkPropertyValues({"TEXT":"2"}))
            # verify
            self.assertEqual(get_state_as_dict(xWriterEdit)["CurrentPage"], "2")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
