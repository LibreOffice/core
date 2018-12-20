# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.debug import sleep
# import org.libreoffice.unotest
# import pathlib
from uitest.path import get_srcdir_url

def get_url_for_data_file(file_name):
#    return pathlib.Path(org.libreoffice.unotest.makeCopyFromTDOC(file_name)).as_uri()
    return get_srcdir_url() + "/sw/qa/uitest/writer_tests/data/" + file_name

#Bug 76636 - Crash in .doc when trying to merge cells

class tdf76636(UITestCase):

    def test_tdf76636_merge_cells_doc(self):

        writer_doc = self.ui_test.load_file(get_url_for_data_file("tdf76636.doc"))
        document = self.ui_test.get_component()
        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")
        #* open Writer
        #* merge the three top cells on the right
        #--> crash

        self.assertEqual(document.TextTables.getCount(), 1)
        #go to middle row
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "DOWN"}))
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "DOWN"}))

        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "TAB"}))
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "TAB"}))
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "TAB"}))

        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "SHIFT+LEFT"}))
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "SHIFT+LEFT"}))

        #merge
        self.xUITest.executeCommand(".uno:MergeCells")
        self.assertEqual(document.TextTables.getCount(), 1)
        #undo
        self.xUITest.executeCommand(".uno:Undo")
        self.assertEqual(document.TextTables.getCount(), 1)
        self.ui_test.close_doc()

    def test_tdf76636_merge_cells_doc_undo(self):

        writer_doc = self.ui_test.load_file(get_url_for_data_file("tdf76636.doc"))
        document = self.ui_test.get_component()
        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")
        # 1) open crash.doc
        # 2) merge the cell with 'punteggio per corso (valutazione singola)' in it with the empty cell to its right
        # 3) press undo
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "DOWN"}))
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "DOWN"}))

        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "TAB"}))
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "TAB"}))

        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "SHIFT+RIGHT"}))
        #merge
        self.xUITest.executeCommand(".uno:MergeCells")
        self.assertEqual(document.TextTables.getCount(), 1)
        #undo
        self.xUITest.executeCommand(".uno:Undo")
        self.assertEqual(document.TextTables.getCount(), 1)

        self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab:
