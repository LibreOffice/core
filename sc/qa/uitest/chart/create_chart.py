# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from libreoffice.uno.propertyvalue import mkPropertyValues

from uitest.framework import UITestCase
from uitest.uihelper.calc import enter_text_to_cell

class CalcChartUIDemo(UITestCase):

    def fill_spreadsheet(self):
        xCalcDoc = self.xUITest.getTopFocusWindow()
        xGridWindow = xCalcDoc.getChild("grid_window")

        enter_text_to_cell(xGridWindow, "A1", "col1")
        enter_text_to_cell(xGridWindow, "B1", "col2")
        enter_text_to_cell(xGridWindow, "C1", "col3")
        enter_text_to_cell(xGridWindow, "A2", "1")
        enter_text_to_cell(xGridWindow, "B2", "3")
        enter_text_to_cell(xGridWindow, "C2", "5")

        xGridWindow.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:C2"}))

    def test_cancel_immediately(self):

        with self.ui_test.create_doc_in_start_center("calc"):

            self.fill_spreadsheet()

            with self.ui_test.execute_dialog_through_command(".uno:InsertObjectChart", close_button="cancel"):
                pass


    def test_create_from_first_page(self):

        with self.ui_test.create_doc_in_start_center("calc"):

            self.fill_spreadsheet()

            with self.ui_test.execute_dialog_through_command(".uno:InsertObjectChart", close_button="finish"):
                pass


    def test_create_from_second_page(self):

        with self.ui_test.create_doc_in_start_center("calc"):

            self.fill_spreadsheet()

            with self.ui_test.execute_dialog_through_command(".uno:InsertObjectChart", close_button="finish") as xChartDlg:
                xNextBtn = xChartDlg.getChild("next")
                xNextBtn.executeAction("CLICK", tuple())

                xDataInRows = xChartDlg.getChild("RB_DATAROWS")
                xDataInRows.executeAction("CLICK", tuple())

                xDataInCols = xChartDlg.getChild("RB_DATACOLS")
                xDataInCols.executeAction("CLICK", tuple())


    def test_deselect_chart(self):
        with self.ui_test.create_doc_in_start_center("calc"):

            self.fill_spreadsheet()

            xCalcDoc = self.xUITest.getTopFocusWindow()
            xGridWindow = xCalcDoc.getChild("grid_window")

            with self.ui_test.execute_dialog_through_command(".uno:InsertObjectChart", close_button="finish"):
                pass

            xGridWindow.executeAction("DESELECT", mkPropertyValues({"OBJECT": ""}))


    def test_activate_chart(self):

        with self.ui_test.create_doc_in_start_center("calc"):

            self.fill_spreadsheet()

            xCalcDoc = self.xUITest.getTopFocusWindow()
            xGridWindow = xCalcDoc.getChild("grid_window")

            with self.ui_test.execute_dialog_through_command(".uno:InsertObjectChart", close_button="finish"):
                pass

            xGridWindow.executeAction("DESELECT", mkPropertyValues({"OBJECT": ""}))

            xGridWindow.executeAction("SELECT", mkPropertyValues({"OBJECT": "Object 1"}))
            xGridWindow.executeAction("ACTIVATE", tuple())

            xGridWindow.executeAction("DESELECT", mkPropertyValues({"OBJECT": ""}))


    def select_chart_element(self):

        with self.ui_test.create_doc_in_start_center("calc"):

            self.fill_spreadsheet()

            xCalcDoc = self.xUITest.getTopFocusWindow()
            xGridWindow = xCalcDoc.getChild("grid_window")

            with self.ui_test.execute_dialog_through_command(".uno:InsertObjectChart", close_button="finish"):
                pass

            xGridWindow.executeAction("DESELECT", mkPropertyValues({"OBJECT": ""}))

            xGridWindow.executeAction("SELECT", mkPropertyValues({"OBJECT": "Object 1"}))
            xGridWindow.executeAction("ACTIVATE", tuple())


# vim: set shiftwidth=4 softtabstop=4 expandtab:
