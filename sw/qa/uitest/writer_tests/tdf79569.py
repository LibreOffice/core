# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.debug import sleep
from uitest.path import get_srcdir_url

def get_url_for_data_file(file_name):
   return get_srcdir_url() + "/sw/qa/uitest/writer_tests/data/" + file_name

class tdf79569(UITestCase):

   def test_tdf81457_table_merge_undo(self):
        writer_doc = self.ui_test.load_file(get_url_for_data_file("tdf79569.doc"))
        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")
        document = self.ui_test.get_component()
        self.xUITest.executeCommand(".uno:GoDown")
        self.xUITest.executeCommand(".uno:GoDown")
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "CTRL+END"}))
        self.xUITest.executeCommand(".uno:GoRight")
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "CTRL+END"}))
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "SHIFT+RIGHT"}))
        self.xUITest.executeCommand(".uno:MergeCells")
        self.xUITest.executeCommand(".uno:Undo")
        self.xUITest.executeCommand(".uno:Redo")
        self.xUITest.executeCommand(".uno:Undo")

        self.assertEqual(document.TextTables.getCount(), 1)

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
