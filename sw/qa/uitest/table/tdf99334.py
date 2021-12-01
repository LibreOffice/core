# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import get_url_for_data_file

#Bug 99334 - Crashes when sorting table by columns with umlauts

class tdf99334(UITestCase):

   def test_tdf99334_table_sort_umlauts(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf99334.odt")) as writer_doc:
            xWriterDoc = self.xUITest.getTopFocusWindow()
            #select whole table
            self.xUITest.executeCommand(".uno:SelectTable")
            #Tools - Sort
            with self.ui_test.execute_dialog_through_command(".uno:SortDialog"):
                pass
            #verify
            tables = writer_doc.getTextTables()
            table = tables[0]
            tableText = table.getCellByName("B1")
            b1 = tableText.getString()
            self.assertEqual(b1, "two")
            #undo
            self.xUITest.executeCommand(".uno:Undo")
            b1 = tableText.getString()
            self.assertEqual(b1, "one")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
