# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict
import org.libreoffice.unotest
import pathlib

def get_url_for_data_file(file_name):
    return pathlib.Path(org.libreoffice.unotest.makeCopyFromTDOC(file_name)).as_uri()

class tdf132596(UITestCase):

    def test_tdf132596(self):
        writer_doc = self.ui_test.load_file(get_url_for_data_file("tdf132596.docx"))

        document = self.ui_test.get_component()
        self.assertEqual(2, document.CurrentController.PageCount)

        self.xUITest.executeCommand(".uno:SelectAll")
        self.xUITest.executeCommand(".uno:Copy")
        self.ui_test.execute_dialog_through_command(".uno:PasteSpecial")
        xDialog = self.xUITest.getTopFocusWindow()

        xList = xDialog.getChild('list')

        for childName in xList.getChildren():
            xChild = xList.getChild(childName)
            if get_state_as_dict(xChild)['Text'] == "Rich text formatting (RTF)":
                break

        xChild.executeAction("SELECT", tuple())
        self.assertEqual(get_state_as_dict(xList)['SelectEntryText'], "Rich text formatting (RTF)")

        xOkBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOkBtn)

        self.xUITest.executeCommand(".uno:Undo")
        # Without the fix in place, it would have crashed here

        self.assertEqual(2, document.CurrentController.PageCount)

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:

