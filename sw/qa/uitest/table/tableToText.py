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
from uitest.uihelper.common import get_url_for_data_file

#Writer Table to text

class tableToText(UITestCase):
    def test_table_to_text(self):
        with self.ui_test.load_file(get_url_for_data_file("tableToText.odt")) as writer_doc:
            xWriterDoc = self.xUITest.getTopFocusWindow()
            #dialog Table to text - Tabs; verify
            with self.ui_test.execute_dialog_through_command(".uno:ConvertTableToText") as xDialog:
                tabs = xDialog.getChild("tabs")
                tabs.executeAction("CLICK", tuple())
            #verify
            self.assertEqual(writer_doc.Text.String[0:3], "a\ta")
            self.assertEqual(writer_doc.TextTables.getCount(), 0)
            #undo
            self.xUITest.executeCommand(".uno:Undo")
            self.assertEqual(writer_doc.TextTables.getCount(), 1)

            #dialog Table to text - Paragraph; verify
            with self.ui_test.execute_dialog_through_command(".uno:ConvertTableToText") as xDialog:
                paragraph = xDialog.getChild("paragraph")
                paragraph.executeAction("CLICK", tuple())
            #verify
            self.assertEqual(writer_doc.Text.String.replace('\r\n', '\n')[0:4], "a\na\n")
            self.assertEqual(writer_doc.TextTables.getCount(), 0)
            #undo
            self.xUITest.executeCommand(".uno:Undo")
            self.assertEqual(writer_doc.TextTables.getCount(), 1)

            #dialog Table to text - Semicolons; verify
            with self.ui_test.execute_dialog_through_command(".uno:ConvertTableToText") as xDialog:
                semicolons = xDialog.getChild("semicolons")
                semicolons.executeAction("CLICK", tuple())
            #verify
            self.assertEqual(writer_doc.Text.String.replace('\r\n', '\n')[0:6], "a;a\n;\n")
            self.assertEqual(writer_doc.TextTables.getCount(), 0)
            #undo
            self.xUITest.executeCommand(".uno:Undo")
            self.assertEqual(writer_doc.TextTables.getCount(), 1)

            #dialog Table to text - other; verify
            with self.ui_test.execute_dialog_through_command(".uno:ConvertTableToText") as xDialog:
                other = xDialog.getChild("other")
                other.executeAction("CLICK", tuple())
                othered = xDialog.getChild("othered")
                othered.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
                othered.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
                othered.executeAction("TYPE", mkPropertyValues({"TEXT":":"}))
            #verify
            self.assertEqual(writer_doc.Text.String.replace('\r\n', '\n')[0:6], "a:a\n:\n")
            self.assertEqual(writer_doc.TextTables.getCount(), 0)
            #undo
            self.xUITest.executeCommand(".uno:Undo")
            self.assertEqual(writer_doc.TextTables.getCount(), 1)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
