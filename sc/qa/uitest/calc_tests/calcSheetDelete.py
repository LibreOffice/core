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
from uitest.uihelper.calc import enter_text_to_cell

class calcSheetDelete(UITestCase):

    def test_tdf114228_insert_and_delete_sheet(self):

        with self.ui_test.create_doc_in_start_center("calc") as document:

            xCalcDoc = self.xUITest.getTopFocusWindow()
            xGridWindow = xCalcDoc.getChild("grid_window")

            xGridWindow.executeAction("SELECT", mkPropertyValues({"CELL": "L12"}))
            nrSheets = document.Sheets.getCount()  #default number

            with self.ui_test.execute_dialog_through_command(".uno:Insert"):
                pass

            self.assertEqual(document.Sheets.getCount(), nrSheets + 1)

            with self.ui_test.execute_dialog_through_command(".uno:Remove", close_button="yes"):
                pass
            xToolkit = self.xContext.ServiceManager.createInstance('com.sun.star.awt.Toolkit')
            xToolkit.processEventsToIdle()

            self.assertEqual(document.Sheets.getCount(), nrSheets)
            self.xUITest.executeCommand(".uno:Undo")
            self.assertEqual(document.Sheets.getCount(), nrSheets + 1)
            self.xUITest.executeCommand(".uno:Redo")
            self.assertEqual(document.Sheets.getCount(), nrSheets)


    def test_tdf43078_insert_and_delete_sheet_insert_text(self):

        with self.ui_test.create_doc_in_start_center("calc") as document:

            nrSheets = document.Sheets.getCount()  #default number of sheets

            with self.ui_test.execute_dialog_through_command(".uno:Insert"):
                pass

            with self.ui_test.execute_dialog_through_command(".uno:Insert"):
                pass
            xToolkit = self.xContext.ServiceManager.createInstance('com.sun.star.awt.Toolkit')
            xToolkit.processEventsToIdle()

            self.assertEqual(document.Sheets.getCount(), nrSheets + 2)
            xCalcDoc = self.xUITest.getTopFocusWindow()
            xGridWindow = xCalcDoc.getChild("grid_window")
            enter_text_to_cell(xGridWindow, "B2", "abcd")

            with self.ui_test.execute_dialog_through_command(".uno:Remove", close_button="yes"):
                pass

            self.assertEqual(document.Sheets.getCount(), nrSheets + 1)
            self.xUITest.executeCommand(".uno:Undo")
            self.assertEqual(document.Sheets.getCount(), nrSheets + 2)
            self.xUITest.executeCommand(".uno:Redo")
            self.assertEqual(document.Sheets.getCount(), nrSheets + 1)


    def test_delete_more_sheets_at_once(self):

        with self.ui_test.create_doc_in_start_center("calc") as document:
            nrSheets = document.Sheets.getCount()  #default number
            i = 0
            while i < 6:
                with self.ui_test.execute_dialog_through_command(".uno:Insert"):
                    pass
                i = i + 1
            self.assertEqual(document.Sheets.getCount(), nrSheets + 6)

            i = 0
            while i < 5:
                self.xUITest.executeCommand(".uno:JumpToNextTableSel")  #select next sheet
                i = i + 1

            with self.ui_test.execute_dialog_through_command(".uno:Remove", close_button="yes"):
                pass

            xToolkit = self.xContext.ServiceManager.createInstance('com.sun.star.awt.Toolkit')
            xToolkit.processEventsToIdle()
            self.assertEqual(document.Sheets.getCount(), nrSheets)
            self.xUITest.executeCommand(".uno:Undo")
            self.assertEqual(document.Sheets.getCount(), nrSheets + 6)
            self.xUITest.executeCommand(".uno:Redo")
            self.assertEqual(document.Sheets.getCount(), nrSheets)


    def test_tdf105105_delete_lots_of_sheets_at_once(self):

        with self.ui_test.create_doc_in_start_center("calc") as document:
            nrSheets = document.Sheets.getCount()  #default number
            i = 0
            while i < 100:
                with self.ui_test.execute_dialog_through_command(".uno:Insert"):
                    pass
                i = i + 1
            xToolkit = self.xContext.ServiceManager.createInstance('com.sun.star.awt.Toolkit')
            xToolkit.processEventsToIdle()
            self.assertEqual(document.Sheets.getCount(), nrSheets + 100)

            i = 0
            while i < 99:
                self.xUITest.executeCommand(".uno:JumpToNextTableSel")  #select next sheet
                i = i + 1

            with self.ui_test.execute_dialog_through_command(".uno:Remove", close_button="yes"):
                pass

            self.assertEqual(document.Sheets.getCount(), nrSheets)
            self.xUITest.executeCommand(".uno:Undo")
            self.assertEqual(document.Sheets.getCount(), nrSheets + 100)
            self.xUITest.executeCommand(".uno:Redo")
            self.assertEqual(document.Sheets.getCount(), nrSheets)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
