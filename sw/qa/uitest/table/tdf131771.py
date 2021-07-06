# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import select_pos

# Bug 131771 - Copying+Pasting a table: caused the loss of table style setting.


class tdf131771(UITestCase):
    def test_tdf131771(self):
        with self.ui_test.create_doc_in_start_center("writer"):
            xWriterDoc = self.xUITest.getTopFocusWindow()
            xWriterEdit = xWriterDoc.getChild("writer_edit")
            # generate a 2x2 tables with the same autoformat table style (Default Table Style)
            # Note that this style is different than applying nothing!
            with self.ui_test.execute_dialog_through_command(".uno:InsertTable") as xDialog:
                formatlbinstable = xDialog.getChild("formatlbinstable")
                entry = formatlbinstable.getChild("1")
                entry.executeAction("SELECT", tuple())

            # select the table
            self.xUITest.executeCommand(".uno:SelectAll")

            # copying and pasting the table below
            self.xUITest.executeCommand(".uno:Copy")
            self.xUITest.executeCommand(".uno:GoDown")
            self.xUITest.executeCommand(".uno:Paste")

            document = self.ui_test.get_component()
            tables = document.getTextTables()
            self.assertEqual(tables[0].TableTemplateName, 'Default Style')
            self.assertEqual(tables[1].TableTemplateName, 'Default Style')

# vim: set shiftwidth=4 softtabstop=4 expandtab:
