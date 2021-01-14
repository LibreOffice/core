#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file

class GoToPage_dialog(UITestCase):

   def test_go_to_page(self):
    writer_doc = self.ui_test.load_file(get_url_for_data_file("3pages.odt"))
    xWriterDoc = self.xUITest.getTopFocusWindow()
    xWriterEdit = xWriterDoc.getChild("writer_edit")

    self.ui_test.execute_dialog_through_command(".uno:GotoPage")
    xDialog = self.xUITest.getTopFocusWindow()
    xPageText = xDialog.getChild("page")
    xPageText.executeAction("TYPE", mkPropertyValues({"TEXT":"2"}))
    xOkBtn = xDialog.getChild("ok")
    xOkBtn.executeAction("CLICK", tuple())

    self.assertEqual(get_state_as_dict(xWriterEdit)["CurrentPage"], "2")

    self.ui_test.execute_dialog_through_command(".uno:GotoPage")
    xDialog = self.xUITest.getTopFocusWindow()
    xPageText = xDialog.getChild("page")
    xPageText.executeAction("TYPE", mkPropertyValues({"TEXT":"3a"}))
    xOkBtn = xDialog.getChild("ok")
    xOkBtn.executeAction("CLICK", tuple())

    self.assertEqual(get_state_as_dict(xWriterEdit)["CurrentPage"], "3")

    # check cancel button
    self.ui_test.execute_dialog_through_command(".uno:GotoPage")
    xDialog = self.xUITest.getTopFocusWindow()
    xCancelBtn = xDialog.getChild("cancel")
    self.ui_test.close_dialog_through_button(xCancelBtn)

    self.assertEqual(get_state_as_dict(xWriterEdit)["CurrentPage"], "3")

    self.ui_test.close_doc()
