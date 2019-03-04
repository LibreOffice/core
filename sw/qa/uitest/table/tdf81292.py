# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict, type_text
import time
from uitest.debug import sleep
import org.libreoffice.unotest
import pathlib

#Bug 81292 - TABLE: Crashes on sorting table

def get_url_for_data_file(file_name):
   return pathlib.Path(org.libreoffice.unotest.makeCopyFromTDOC(file_name)).as_uri()

class tdf81292(UITestCase):

   def test_tdf81292_table_sort(self):
        writer_doc = self.ui_test.load_file(get_url_for_data_file("tdf81292.odt"))
        document = self.ui_test.get_component()
        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")
        #select whole table
        self.xUITest.executeCommand(".uno:SelectTable")
        #Tools - Sort
        self.ui_test.execute_dialog_through_command(".uno:SortDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        xDown = xDialog.getChild("down1")
        xDown.executeAction("CLICK", tuple())
        xOK = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOK)
        #verify
        tables = document.getTextTables()
        table = tables[0]
        tableText = table.getCellByName("B3")
        b3 = tableText.getString()
        tableTextc1 = table.getCellByName("C1").getString()
        self.assertEqual(b3, "https://www.hightail.com/")
        self.assertEqual(tableTextc1, "inlognaam")
        #undo
        self.xUITest.executeCommand(".uno:Undo")
        b3 = tableText.getString()
        self.assertEqual(b3, "www.comicstripshop.com")

        self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab:
