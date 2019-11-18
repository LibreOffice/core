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

#Bug 126626 - CRASH: pasting fly at-char selection

class tdf126626(UITestCase):
   def test_tdf126626_crash_paste(self):
        writer_doc = self.ui_test.load_file(get_url_for_data_file("tdf126626.docx"))
        document = self.ui_test.get_component()
        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")

        # 2. Select all
        self.xUITest.executeCommand(".uno:SelectAll")
        # 3. Copy
        self.xUITest.executeCommand(".uno:Copy")
        # 4. Paste TWO TIMES
        self.xUITest.executeCommand(".uno:Paste")
        self.xUITest.executeCommand(".uno:Paste")
        # 5. Undo once
        self.xUITest.executeCommand(".uno:Undo")
        # 6. Paste
        self.xUITest.executeCommand(".uno:Paste")
        #-> Crash
        self.assertEqual(document.CurrentController.PageCount, 1)

        self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab:
