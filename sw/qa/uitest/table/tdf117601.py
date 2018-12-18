# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.debug import sleep
#Bug 117601 - Crash in: libstdc++.so.6.0.22 on merge cells in writer with TDF .debs on Debian 9

class tdf117601(UITestCase):

    def test_tdf117601_merge_middle_column(self):

        self.ui_test.create_doc_in_start_center("writer")
        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")
        document = self.ui_test.get_component()
        #* open Writer
        #* create table 3x5
        #* merge the middle row
        #--> crash
        self.ui_test.execute_dialog_through_command(".uno:InsertTable")
        xDialog = self.xUITest.getTopFocusWindow()

        column = xDialog.getChild("colspin")
        column.executeAction("UP", tuple())
        row = xDialog.getChild("rowspin")
        row.executeAction("UP", tuple())
        row.executeAction("UP", tuple())
        row.executeAction("UP", tuple())
        xOkBtn = xDialog.getChild("ok")
        xOkBtn.executeAction("CLICK", tuple())

        self.assertEqual(document.TextTables.getCount(), 1)
        #go to middle row
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "UP"}))
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "RIGHT"}))
        #Table - Select Column .uno:EntireColumn
        self.xUITest.executeCommand(".uno:EntireColumn")
        #merge
        self.xUITest.executeCommand(".uno:MergeCells")
        self.assertEqual(document.TextTables.getCount(), 1)

        self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab:
