# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from sys import stderr
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
        toolkit_ex = self.xContext.ServiceManager.createInstanceWithContext(
            "com.sun.star.awt.Toolkit", self.xContext) # supports css.awt.XToolkitExperimental
        print("DEBUG.1", file=stderr);
        self.xUITest.executeCommand(".uno:GoDown")
        print("DEBUG.2", file=stderr);
        toolkit_ex.processEventsToIdle()
        print("DEBUG.3", file=stderr);
        self.xUITest.executeCommand(".uno:GoDown")
        print("DEBUG.4", file=stderr);
        toolkit_ex.processEventsToIdle()
        print("DEBUG.5", file=stderr);
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "CTRL+END"}))
        print("DEBUG.6", file=stderr);
        toolkit_ex.processEventsToIdle()
        print("DEBUG.7", file=stderr);
        self.xUITest.executeCommand(".uno:GoRight")
        print("DEBUG.8", file=stderr);
        toolkit_ex.processEventsToIdle()
        print("DEBUG.9", file=stderr);
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "CTRL+END"}))
        print("DEBUG.10", file=stderr);
        toolkit_ex.processEventsToIdle()
        print("DEBUG.11", file=stderr);
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "SHIFT+RIGHT"}))
        print("DEBUG.12", file=stderr);
        toolkit_ex.processEventsToIdle()
        print("DEBUG.13", file=stderr);
        self.xUITest.executeCommand(".uno:MergeCells")
        print("DEBUG.14", file=stderr);
        toolkit_ex.processEventsToIdle()
        print("DEBUG.15", file=stderr);
        self.xUITest.executeCommand(".uno:Undo")
        print("DEBUG.16", file=stderr);
        toolkit_ex.processEventsToIdle()
        print("DEBUG.17", file=stderr);
        self.xUITest.executeCommand(".uno:Redo")
        print("DEBUG.18", file=stderr);
        toolkit_ex.processEventsToIdle()
        print("DEBUG.19", file=stderr);
        sleep(3)
        self.xUITest.executeCommand(".uno:Undo")
        print("DEBUG.20", file=stderr);
        toolkit_ex.processEventsToIdle()
        print("DEBUG.21", file=stderr);

        self.assertEqual(document.TextTables.getCount(), 1)

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
