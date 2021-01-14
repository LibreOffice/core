# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file

class tdf131963(UITestCase):

    def test_tdf131963(self):
        writer_doc = self.ui_test.load_file(get_url_for_data_file("tdf131963.docx"))

        document = self.ui_test.get_component()
        self.assertEqual(11, document.CurrentController.PageCount)

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

        # Without the fix in place, it would have crashed here

        # tdf#133169: without the fix in place, it would have been 2 instead of 11
        self.assertEqual(11, document.CurrentController.PageCount)

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:

