#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase

class tdf104158(UITestCase):

    def test_number_format_dialog(self):

        self.ui_test.create_doc_in_start_center("writer")

        self.ui_test.execute_dialog_through_command(".uno:InsertTable")

        xInsertDlg = self.xUITest.getTopFocusWindow()

        xOkBtn = xInsertDlg.getChild("ok")
        xOkBtn.executeAction("CLICK", tuple())

        document = self.ui_test.get_component()

        tables = document.getTextTables()
        self.assertEqual(len(tables[0].getRows()), 2)
        self.assertEqual(len(tables[0].getColumns()), 2)

        self.ui_test.execute_dialog_through_command(".uno:TableNumberFormatDialog")

        xNumberFormatDlg = self.xUITest.getTopFocusWindow()

        xOkBtn = xNumberFormatDlg.getChild("ok")
        xOkBtn.executeAction("CLICK", tuple())

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
