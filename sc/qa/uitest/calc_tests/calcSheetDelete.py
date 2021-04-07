# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.calc import enter_text_to_cell

class calcSheetDelete(UITestCase):

    def test_tdf114228_insert_and_delete_sheet(self):

        self.ui_test.create_doc_in_start_center("calc")

        xCalcDoc = self.xUITest.getTopFocusWindow()
        xGridWindow = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()

        xGridWindow.executeAction("SELECT", mkPropertyValues({"CELL": "L12"}))
        nrSheets = document.Sheets.getCount()  #default number

        self.ui_test.execute_dialog_through_command(".uno:Insert")  #insert sheet
        xDialog = self.xUITest.getTopFocusWindow()
        xOKButton = xDialog.getChild("ok")
        xOKButton.executeAction("CLICK", tuple())

        self.assertEqual(document.Sheets.getCount(), nrSheets + 1)

        self.ui_test.execute_dialog_through_command(".uno:Remove")  #delete sheet
        xDialog = self.xUITest.getTopFocusWindow()
        xOKButton = xDialog.getChild("yes")
        xOKButton.executeAction("CLICK", tuple())

        self.assertEqual(document.Sheets.getCount(), nrSheets)
        self.xUITest.executeCommand(".uno:Undo")
        self.assertEqual(document.Sheets.getCount(), nrSheets + 1)
        self.xUITest.executeCommand(".uno:Redo")
        self.assertEqual(document.Sheets.getCount(), nrSheets)

        self.ui_test.close_doc()

    def test_tdf43078_insert_and_delete_sheet_insert_text(self):

        self.ui_test.create_doc_in_start_center("calc")
        document = self.ui_test.get_component()

        nrSheets = document.Sheets.getCount()  #default number of sheets

        self.ui_test.execute_dialog_through_command(".uno:Insert")  #insert sheet
        xDialog = self.xUITest.getTopFocusWindow()
        xOKButton = xDialog.getChild("ok")
        xOKButton.executeAction("CLICK", tuple())

        self.ui_test.execute_dialog_through_command(".uno:Insert")  #insert sheet
        xDialog = self.xUITest.getTopFocusWindow()
        xOKButton = xDialog.getChild("ok")
        xOKButton.executeAction("CLICK", tuple())

        self.assertEqual(document.Sheets.getCount(), nrSheets + 2)
        xCalcDoc = self.xUITest.getTopFocusWindow()
        xGridWindow = xCalcDoc.getChild("grid_window")
        enter_text_to_cell(xGridWindow, "B2", "abcd")

        self.ui_test.execute_dialog_through_command(".uno:Remove")  #delete sheet
        xDialog = self.xUITest.getTopFocusWindow()
        xOKButton = xDialog.getChild("yes")
        xOKButton.executeAction("CLICK", tuple())

        self.assertEqual(document.Sheets.getCount(), nrSheets + 1)
        self.xUITest.executeCommand(".uno:Undo")
        self.assertEqual(document.Sheets.getCount(), nrSheets + 2)
        self.xUITest.executeCommand(".uno:Redo")
        self.assertEqual(document.Sheets.getCount(), nrSheets + 1)

        self.ui_test.close_doc()

    def test_delete_more_sheets_at_once(self):

        self.ui_test.create_doc_in_start_center("calc")

        xCalcDoc = self.xUITest.getTopFocusWindow()
        xGridWindow = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        nrSheets = document.Sheets.getCount()  #default number
        i = 0
        while i < 6:
            self.ui_test.execute_dialog_through_command(".uno:Insert")  #insert sheet
            xDialog = self.xUITest.getTopFocusWindow()
            xOKButton = xDialog.getChild("ok")
            xOKButton.executeAction("CLICK", tuple())
            i = i + 1
        self.assertEqual(document.Sheets.getCount(), nrSheets + 6)

        i = 0
        while i < 5:
            self.xUITest.executeCommand(".uno:JumpToNextTableSel")  #select next sheet
            i = i + 1

        self.ui_test.execute_dialog_through_command(".uno:Remove")  #delete selected sheets
        xDialog = self.xUITest.getTopFocusWindow()
        xOKButton = xDialog.getChild("yes")
        xOKButton.executeAction("CLICK", tuple())

        self.assertEqual(document.Sheets.getCount(), nrSheets)
        self.xUITest.executeCommand(".uno:Undo")
        self.assertEqual(document.Sheets.getCount(), nrSheets + 6)
        self.xUITest.executeCommand(".uno:Redo")
        self.assertEqual(document.Sheets.getCount(), nrSheets)

        self.ui_test.close_doc()

    def test_tdf105105_delete_lots_of_sheets_at_once(self):

        self.ui_test.create_doc_in_start_center("calc")

        xCalcDoc = self.xUITest.getTopFocusWindow()
        xGridWindow = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        nrSheets = document.Sheets.getCount()  #default number
        i = 0
        while i < 100:
            self.ui_test.execute_dialog_through_command(".uno:Insert")  #insert sheet
            xDialog = self.xUITest.getTopFocusWindow()
            xOKButton = xDialog.getChild("ok")
            xOKButton.executeAction("CLICK", tuple())
            i = i + 1
        self.assertEqual(document.Sheets.getCount(), nrSheets + 100)

        i = 0
        while i < 99:
            self.xUITest.executeCommand(".uno:JumpToNextTableSel")  #select next sheet
            i = i + 1

        self.ui_test.execute_dialog_through_command(".uno:Remove")  #delete selected sheets
        xDialog = self.xUITest.getTopFocusWindow()
        xOKButton = xDialog.getChild("yes")
        xOKButton.executeAction("CLICK", tuple())

        self.assertEqual(document.Sheets.getCount(), nrSheets)
        self.xUITest.executeCommand(".uno:Undo")
        self.assertEqual(document.Sheets.getCount(), nrSheets + 100)
        self.xUITest.executeCommand(".uno:Redo")
        self.assertEqual(document.Sheets.getCount(), nrSheets)

        self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab:
