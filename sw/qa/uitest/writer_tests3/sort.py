# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict, type_text
from uitest.uihelper.common import select_by_text
import time
from uitest.debug import sleep
#Tools -Sort dialog + tdf81292

class WriterSort(UITestCase):

   def test_sort(self):
        self.ui_test.create_doc_in_start_center("writer")
        document = self.ui_test.get_component()
        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")

        type_text(xWriterEdit, "a")
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "RETURN"}))
        type_text(xWriterEdit, "c")
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "RETURN"}))
        type_text(xWriterEdit, "v")

        selection = self.xUITest.executeCommand(".uno:SelectAll")  #select whole text
        #Tools - Sort
        self.ui_test.execute_dialog_through_command(".uno:SortDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        xDown = xDialog.getChild("down1")
        xDown.executeAction("CLICK", tuple())
        xOK = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOK)
        #check
        self.assertEqual(document.Text.String[0:1], "v")

        self.ui_test.close_doc()

   def test_sort_numerical(self):
        self.ui_test.create_doc_in_start_center("writer")
        document = self.ui_test.get_component()
        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")

        type_text(xWriterEdit, "1;2;3")
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "RETURN"}))
        type_text(xWriterEdit, "2;8;3")

        selection = self.xUITest.executeCommand(".uno:SelectAll")  #select whole text
        #Tools - Sort
        self.ui_test.execute_dialog_through_command(".uno:SortDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        xDown = xDialog.getChild("down1")
        xcolsb1 = xDialog.getChild("colsb1")
        xtypelb1 = xDialog.getChild("typelb1")
        xcharacter = xDialog.getChild("character")
        xseparator = xDialog.getChild("separator")
        xDown.executeAction("CLICK", tuple())
        select_by_text(xtypelb1, "Numerical")
        xcharacter.executeAction("CLICK", tuple())
        xseparator.executeAction("TYPE", mkPropertyValues({"TEXT":";"}))
        xOK = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOK)
        #check
        self.assertEqual(document.Text.String[0:5], "2;8;3")

        self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab:
