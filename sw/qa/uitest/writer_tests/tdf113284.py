# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict
import org.libreoffice.unotest
import pathlib
import time
from uitest.debug import sleep

def get_url_for_data_file(file_name):
    return pathlib.Path(org.libreoffice.unotest.makeCopyFromTDOC(file_name)).as_uri()

class tdf113284(UITestCase):

   def test_tdf113284(self):
    writer_doc = self.ui_test.load_file(get_url_for_data_file("tdf113284.odt"))
    document = self.ui_test.get_component()
    xWriterDoc = self.xUITest.getTopFocusWindow()
    xWriterEdit = xWriterDoc.getChild("writer_edit")

    sleep(2)  #we need this sleep,because without this is pagecount counted without index. Load index takes time probably.
    xPageCount = document.CurrentController.PageCount
    self.ui_test.execute_dialog_through_command(".uno:GotoPage")
    xDialog = self.xUITest.getTopFocusWindow()
    xPageText = xDialog.getChild("page")
    xPageText.executeAction("TYPE", mkPropertyValues({"TEXT":str(xPageCount)})) # goto last page
    xOkBtn = xDialog.getChild("ok")
    self.ui_test.close_dialog_through_button(xOkBtn)

    self.assertEqual(get_state_as_dict(xWriterEdit)["CurrentPage"], str(xPageCount))
    self.ui_test.execute_dialog_through_command(".uno:EditCurIndex")  #open index dialog
    xDiagIndex = self.xUITest.getTopFocusWindow()
    xCancBtn = xDiagIndex.getChild("cancel")
    self.ui_test.close_dialog_through_button(xCancBtn)   # close dialog

    #page count  is not constant
    #self.assertEqual(get_state_as_dict(xWriterEdit)["CurrentPage"], "66")    #page 66 start of the Index
    #pagecount unchanged
    self.assertEqual(document.CurrentController.PageCount, xPageCount)

    self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab: