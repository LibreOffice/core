#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase

class tdf93068(UITestCase):

    def test_tdf93068(self):

        self.ui_test.create_doc_in_start_center("writer")

        document = self.ui_test.get_component()

        self.xUITest.getTopFocusWindow()

        # tdf#135950: Character dialog crashes if multiple cells in a
        # table are selected
        self.ui_test.execute_dialog_through_command(".uno:InsertTable")
        xDialog = self.xUITest.getTopFocusWindow()

        xOkBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOkBtn)

        self.xUITest.executeCommand(".uno:SelectAll")

        # Check the table is selected
        self.assertEqual("SwXTextTableCursor", document.CurrentSelection.getImplementationName())

        self.ui_test.execute_dialog_through_command(".uno:FontDialog")
        xFontDlg = self.xUITest.getTopFocusWindow()
        xOKBtn = xFontDlg.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        self.ui_test.execute_dialog_through_command(".uno:FontDialog")
        xFontDlg = self.xUITest.getTopFocusWindow()
        xCancBtn = xFontDlg.getChild("cancel")
        xCancBtn.executeAction("CLICK", tuple())

        self.ui_test.execute_dialog_through_command(".uno:FontDialog")
        xFontDlg = self.xUITest.getTopFocusWindow()
        xDiscardBtn = xFontDlg.getChild("reset")
        xDiscardBtn.executeAction("CLICK", tuple())
        xOKBtn = xFontDlg.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        self.xUITest.getTopFocusWindow()

        # Check the table is still selected after closing the dialog
        self.assertEqual("SwXTextTableCursor", document.CurrentSelection.getImplementationName())

        self.xUITest.executeCommand(".uno:GoDown")

        # Check the table is no longer selected
        self.assertNotEqual("SwXTextTableCursor", document.CurrentSelection.getImplementationName())

        self.ui_test.close_doc()
