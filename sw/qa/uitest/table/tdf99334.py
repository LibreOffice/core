# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file, type_text

#Bug 99334 - Crashes when sorting table by columns with umlauts

class tdf99334(UITestCase):

   def test_tdf99334_table_sort_umlauts(self):
        writer_doc = self.ui_test.load_file(get_url_for_data_file("tdf99334.odt"))
        document = self.ui_test.get_component()
        xWriterDoc = self.xUITest.getTopFocusWindow()
        #select whole table
        self.xUITest.executeCommand(".uno:SelectTable")
        #Tools - Sort
        self.ui_test.execute_dialog_through_command(".uno:SortDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        # xDown = xDialog.getChild("down1")
        # xDown.executeAction("CLICK", tuple())
        xOK = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOK)
        #verify
        tables = document.getTextTables()
        table = tables[0]
        tableText = table.getCellByName("B1")
        b1 = tableText.getString()
        self.assertEqual(b1, "two")
        #undo
        self.xUITest.executeCommand(".uno:Undo")
        b1 = tableText.getString()
        self.assertEqual(b1, "one")

        self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab:
