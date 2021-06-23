#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues

class WriterInsertBreakDialog(UITestCase):

    def launch_dialog_and_select_option(self, child):

        self.ui_test.execute_dialog_through_command(".uno:InsertBreak")
        xDialog = self.xUITest.getTopFocusWindow()

        xOption = xDialog.getChild(child)
        xOption.executeAction("CLICK", tuple())

        return xDialog

    def getPages(self, total):
        document = self.ui_test.get_component()

        self.assertEqual(document.CurrentController.PageCount, total)

    def test_insert_line_break(self):

        self.ui_test.create_doc_in_start_center("writer")

        xDialog = self.launch_dialog_and_select_option("linerb")
        xOkBtn = xDialog.getChild("ok")
        xOkBtn.executeAction("CLICK", tuple())

        self.getPages(1)

        self.ui_test.close_doc()

    def test_insert_column_break(self):

        self.ui_test.create_doc_in_start_center("writer")

        xDialog = self.launch_dialog_and_select_option("columnrb")
        xOkBtn = xDialog.getChild("ok")
        xOkBtn.executeAction("CLICK", tuple())

        self.getPages(1)

        self.ui_test.close_doc()

    def test_insert_page_break(self):

        self.ui_test.create_doc_in_start_center("writer")

        for i in range(9):
            with self.subTest(i=i):
                xDialog = self.launch_dialog_and_select_option("pagerb")

                xStyleList = xDialog.getChild("stylelb")
                xStyleList.executeAction("SELECT", mkPropertyValues({"POS": str(i)}))

                xOkBtn = xDialog.getChild("ok")
                xOkBtn.executeAction("CLICK", tuple())

                self.getPages(i + 2)

        self.ui_test.close_doc()

    def test_cancel_button_insert_line_break_dialog(self):

        self.ui_test.create_doc_in_start_center("writer")

        self.ui_test.execute_dialog_through_command(".uno:InsertBreak")
        xDialog = self.xUITest.getTopFocusWindow()
        xCancelBtn = xDialog.getChild("cancel")
        self.ui_test.close_dialog_through_button(xCancelBtn)

        self.getPages(1)

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
