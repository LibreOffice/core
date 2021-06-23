# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues

class tdf53460(UITestCase):

    def test_resize_table_with_keyboard_tdf53460(self):

        self.ui_test.create_doc_in_start_center("writer")
        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")
        document = self.ui_test.get_component()
        #-insert a table (by default 2x2)
        self.ui_test.execute_dialog_through_command(".uno:InsertTable")
        xDialog = self.xUITest.getTopFocusWindow()
        xOkBtn = xDialog.getChild("ok")
        xOkBtn.executeAction("CLICK", tuple())

        #-put the cursor inside first cell top left for example
        #-insert an inner table (by default 2x2) inside this cell
        self.ui_test.execute_dialog_through_command(".uno:InsertTable")
        xDialog = self.xUITest.getTopFocusWindow()
        xOkBtn = xDialog.getChild("ok")
        xOkBtn.executeAction("CLICK", tuple())
        #-still in top left cell, go to the line after the inner table
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "DOWN"}))
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "DOWN"}))
        #- <ALT>+up key => crash
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "ALT+UP"}))
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "ALT+DOWN"}))
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "ALT+DOWN"}))
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "ALT+RIGHT"}))
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "ALT+RIGHT"}))
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "ALT+LEFT"}))

        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "DOWN"}))
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "ALT+UP"}))
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "ALT+DOWN"}))
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "ALT+DOWN"}))
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "ALT+RIGHT"}))
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "ALT+RIGHT"}))
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "ALT+LEFT"}))

        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "RIGHT"}))
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "ALT+UP"}))
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "ALT+DOWN"}))
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "ALT+DOWN"}))
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "ALT+RIGHT"}))
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "ALT+RIGHT"}))
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "ALT+LEFT"}))

        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "UP"}))
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "ALT+UP"}))
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "ALT+DOWN"}))
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "ALT+DOWN"}))
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "ALT+RIGHT"}))
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "ALT+RIGHT"}))
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "ALT+LEFT"}))

        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "LEFT"}))
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "LEFT"}))
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "ALT+UP"}))
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "ALT+DOWN"}))
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "ALT+DOWN"}))
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "ALT+RIGHT"}))
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "ALT+RIGHT"}))
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "ALT+LEFT"}))

        self.assertEqual(document.TextTables.getCount(), 2)
        self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab:
