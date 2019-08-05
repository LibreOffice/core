# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
import org.libreoffice.unotest
import pathlib
from uitest.uihelper.common import get_state_as_dict
import time
from uitest.debug import sleep
from uitest.uihelper.common import select_pos

def get_url_for_data_file(file_name):
   return pathlib.Path(org.libreoffice.unotest.makeCopyFromTDOC(file_name)).as_uri()

class tdf106746(UITestCase):

   def test_tdf106746_copy_pasting_revisions_copy_deleted_words(self):
        writer_doc = self.ui_test.load_file(get_url_for_data_file("tdf106746.docx"))
        document = self.ui_test.get_component()
        xWriterDoc = self.xUITest.getTopFocusWindow()
        self.xUITest.executeCommand(".uno:SelectAll")
        self.xUITest.executeCommand(".uno:Copy")
        self.xUITest.executeCommand(".uno:Delete")
        self.xUITest.executeCommand(".uno:Paste")

        self.assertEqual(document.Text.String[39:48], "are routi")
        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
