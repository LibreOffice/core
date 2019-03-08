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

#Bug 123547 - Systematic crash after opening Tools > Chapter Numbering

class tdf123547(UITestCase):
   def test_tdf123547_numbering_dialog_crash(self):
        writer_doc = self.ui_test.load_file(get_url_for_data_file("tdf123547.docx"))
        document = self.ui_test.get_component()
        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")

        self.ui_test.execute_dialog_through_command(".uno:ChapterNumberingDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        xTabs = xDialog.getChild("tabcontrol")
        select_pos(xTabs, "1")
        xokbtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xokbtn)
        #verify we didn't crash
        self.assertEqual(document.CurrentController.PageCount, 1)

        self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab:
