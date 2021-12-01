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
from uitest.uihelper.common import select_by_text
from uitest.uihelper.common import type_text

class WriterSort(UITestCase):

   def test_sort(self):
        with self.ui_test.create_doc_in_start_center("writer") as document:
            xWriterDoc = self.xUITest.getTopFocusWindow()
            xWriterEdit = xWriterDoc.getChild("writer_edit")

            type_text(xWriterEdit, "a")
            xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "RETURN"}))
            type_text(xWriterEdit, "c")
            xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "RETURN"}))
            type_text(xWriterEdit, "v")

            selection = self.xUITest.executeCommand(".uno:SelectAll")  #select whole text
            #Tools - Sort
            with self.ui_test.execute_dialog_through_command(".uno:SortDialog") as xDialog:
                xDown = xDialog.getChild("down1")
                xDown.executeAction("CLICK", tuple())
            #check
            self.assertEqual(document.Text.String[0:1], "v")


   def test_sort_numerical(self):
        with self.ui_test.create_doc_in_start_center("writer") as document:
            xWriterDoc = self.xUITest.getTopFocusWindow()
            xWriterEdit = xWriterDoc.getChild("writer_edit")

            type_text(xWriterEdit, "1;2;3")
            xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "RETURN"}))
            type_text(xWriterEdit, "2;8;3")

            selection = self.xUITest.executeCommand(".uno:SelectAll")  #select whole text
            #Tools - Sort
            with self.ui_test.execute_dialog_through_command(".uno:SortDialog") as xDialog:
                xDown = xDialog.getChild("down1")
                xcolsb1 = xDialog.getChild("colsb1")
                xtypelb1 = xDialog.getChild("typelb1")
                xcharacter = xDialog.getChild("character")
                xseparator = xDialog.getChild("separator")
                xDown.executeAction("CLICK", tuple())
                select_by_text(xtypelb1, "Numerical")
                xcharacter.executeAction("CLICK", tuple())
                xseparator.executeAction("TYPE", mkPropertyValues({"TEXT":";"}))
            #check
            self.assertEqual(document.Text.String[0:5], "2;8;3")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
