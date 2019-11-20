# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict, type_text
import time
from uitest.debug import sleep
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import select_pos
import org.libreoffice.unotest
import pathlib

def get_url_for_data_file(file_name):
    return pathlib.Path(org.libreoffice.unotest.makeCopyFromTDOC(file_name)).as_uri()

#Bug 126340 - Crash when removing entry in bullet list (change tracking involved)

class tdf126340(UITestCase):
   def test_tdf126340_crash_paste(self):
        writer_doc = self.ui_test.load_file(get_url_for_data_file("tdf126340.odt"))
        document = self.ui_test.get_component()
        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")

        # Put the cursor to the empty bullet list entry
        self.xUITest.executeCommand(".uno:GoDown")
        # Hit delete
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "DELETE"}))
        #-> Crash
        self.assertEqual(document.Text.String[0:3], "foo")
        self.xUITest.executeCommand(".uno:Undo")
        self.assertEqual(document.Text.String[0:3], "foo")

        self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab:
