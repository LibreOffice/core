#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict
import time
from uitest.path import get_srcdir_url

def get_url_for_data_file(file_name):
    return get_srcdir_url() + "/uitest/writer_tests/data/" + file_name

class tdf120115(UITestCase):

    def test_undo_crash(self):
        repeats = 3
        self.ui_test.load_file(get_url_for_data_file("endnote-in-table.odt"))
        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")
        self.xUITest.executeCommand(".uno:SelectAll")
        self.xUITest.executeCommand(".uno:Copy")
        # self.xUITest.executeCommand(".uno:NewDoc") seems to open the template manager instead?
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+n"}))
        for x in range(repeats):
            self.xUITest.executeCommand(".uno:Paste")
        for x in range(repeats):
            self.xUITest.executeCommand(".uno:Undo")
        for x in range(repeats):
            self.xUITest.executeCommand(".uno:Paste")
        self.ui_test.close_doc()
