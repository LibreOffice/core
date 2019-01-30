# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.debug import sleep
from uitest.uihelper.common import get_state_as_dict, type_text
import org.libreoffice.unotest
import pathlib

def get_url_for_data_file(file_name):
    return pathlib.Path(org.libreoffice.unotest.makeCopyFromTDOC(file_name)).as_uri()

#Bug 122449 - Crash in: mergedlo.dll when closing "Edit Index Entry" dialog (gen/gtk)

class tdf122449(UITestCase):

    def test_tdf122449_crash_edit_index_entry(self):
        writer_doc = self.ui_test.load_file(get_url_for_data_file("tdf122449.odt"))
        document = self.ui_test.get_component()
        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")
        #search word Autocorrect (second find)   .uno:SearchDialog
        self.ui_test.execute_modeless_dialog_through_command(".uno:SearchDialog")
        xDialog = self.xUITest.getTopFocusWindow()

        searchterm = xDialog.getChild("searchterm")
        searchterm.executeAction("TYPE", mkPropertyValues({"TEXT":"Autocorrection"}))
        xsearch = xDialog.getChild("search")
        xsearch.executeAction("CLICK", tuple())  #first search
        xsearch.executeAction("CLICK", tuple())  #2nd search
        self.assertEqual(get_state_as_dict(xWriterEdit)["CurrentPage"], "6")
        xcloseBtn = xDialog.getChild("close")
        self.ui_test.close_dialog_through_button(xcloseBtn)
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "HOME"}))
        # invoke Index dialog Index entry   .uno:IndexEntryDialog
        self.ui_test.execute_dialog_through_command(".uno:IndexEntryDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        xcloseBtn = xDialog.getChild("close")
        self.ui_test.close_dialog_through_button(xcloseBtn)
        # close
        # Go to page 2
        self.ui_test.execute_dialog_through_command(".uno:GotoPage")
        xDialog = self.xUITest.getTopFocusWindow()
        xPageText = xDialog.getChild("page")
        xPageText.executeAction("TYPE", mkPropertyValues({"TEXT":"2"}))
        xOkBtn = xDialog.getChild("ok")
        xOkBtn.executeAction("CLICK", tuple())
        # verify
        self.assertEqual(get_state_as_dict(xWriterEdit)["CurrentPage"], "2")

        self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab:
