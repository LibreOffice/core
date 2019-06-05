# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict
from uitest.debug import sleep
import org.libreoffice.unotest
import pathlib

def get_url_for_data_file(file_name):
   return pathlib.Path(org.libreoffice.unotest.makeCopyFromTDOC(file_name)).as_uri()

#Bug 118540 - LO6.1b2: DOCX crashes when properties are opened in print preview mode

class tdf118540(UITestCase):
   def test_tdf118540_preview_document_properties(self):
        # FIXME flaky test, usually passes, but breaks regularly.
        return
        writer_doc = self.ui_test.load_file(get_url_for_data_file("tdf118540.docx"))
        document = self.ui_test.get_component()
        self.xUITest.executeCommand(".uno:PrintPreview")  #open print preview
        self.ui_test.execute_dialog_through_command(".uno:SetDocumentProperties") #open properties dialog
        xDialog = self.xUITest.getTopFocusWindow()
        xOkBtn = xDialog.getChild("ok")
        xOkBtn.executeAction("CLICK", tuple())
        self.xUITest.executeCommand(".uno:ClosePreview")  # close print preview

        self.xUITest.getTopFocusWindow() #Get focus after closing preview

        #verify
        self.assertEqual(document.Text.String[0:4], "Test")
        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
