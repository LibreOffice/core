# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from libreoffice.uno.propertyvalue import mkPropertyValues

from uitest.framework import UITestCase
from uitest.uihelper.calc import enter_text_to_cell

import time
import unittest

class CalcChartUIDemo(UITestCase):

    def create_insert_chart_dialog(self):
        self.ui_test.execute_dialog_through_command(".uno:InsertObjectChart")
        # time.sleep(1) # ideally wait for a creation event
        return self.xUITest.getTopFocusWindow()

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

        self.ui_test.create_doc_in_start_center("calc")

        self.fill_spreadsheet()

        xChartDlg = self.create_insert_chart_dialog();

        xCancelBtn = xChartDlg.getChild("cancel")
        self.ui_test.close_dialog_through_button(xCancelBtn)

        self.ui_test.close_doc()

    def test_create_from_first_page(self):

        self.ui_test.create_doc_in_start_center("calc")

        self.fill_spreadsheet()

        xChartDlg = self.create_insert_chart_dialog();

        xOkBtn = xChartDlg.getChild("finish")
        self.ui_test.close_dialog_through_button(xOkBtn)

        self.ui_test.close_doc()

    def test_create_from_second_page(self):

        self.ui_test.create_doc_in_start_center("calc")

        self.fill_spreadsheet()

        xChartDlg = self.create_insert_chart_dialog();

        xNextBtn = xChartDlg.getChild("next")
        xNextBtn.executeAction("CLICK", tuple())

        xDataInRows = xChartDlg.getChild("RB_DATAROWS")
        xDataInRows.executeAction("CLICK", tuple())

        xDataInCols = xChartDlg.getChild("RB_DATACOLS")
        xDataInCols.executeAction("CLICK", tuple())

        xCancelBtn = xChartDlg.getChild("finish")
        self.ui_test.close_dialog_through_button(xCancelBtn)

        self.ui_test.close_doc()

    def test_deselect_chart(self):
        self.ui_test.create_doc_in_start_center("calc")

        self.fill_spreadsheet()

        xCalcDoc = self.xUITest.getTopFocusWindow()
        xGridWindow = xCalcDoc.getChild("grid_window")

        xChartDlg = self.create_insert_chart_dialog();

        xNextBtn = xChartDlg.getChild("finish")
        self.ui_test.close_dialog_through_button(xNextBtn)

        xGridWindow.executeAction("DESELECT", mkPropertyValues({"OBJECT": ""}))

        time.sleep(2)

        self.ui_test.close_doc()

    def test_activate_chart(self):

        self.ui_test.create_doc_in_start_center("calc")

        self.fill_spreadsheet()

        xCalcDoc = self.xUITest.getTopFocusWindow()
        xGridWindow = xCalcDoc.getChild("grid_window")

        xChartDlg = self.create_insert_chart_dialog();

        xNextBtn = xChartDlg.getChild("finish")
        self.ui_test.close_dialog_through_button(xNextBtn)

        xGridWindow.executeAction("DESELECT", mkPropertyValues({"OBJECT": ""}))

        xGridWindow.executeAction("SELECT", mkPropertyValues({"OBJECT": "Object 1"}))
        xGridWindow.executeAction("ACTIVATE", tuple())

        xGridWindow.executeAction("DESELECT", mkPropertyValues({"OBJECT": ""}))

        self.ui_test.close_doc()

    def select_chart_element(self):

        self.ui_test.create_doc_in_start_center("calc")

        self.fill_spreadsheet()

        xCalcDoc = self.xUITest.getTopFocusWindow()
        xGridWindow = xCalcDoc.getChild("grid_window")

        xChartDlg = self.create_insert_chart_dialog();

        xNextBtn = xChartDlg.getChild("finish")
        self.ui_test.close_dialog_through_button(xNextBtn)

        xGridWindow.executeAction("DESELECT", mkPropertyValues({"OBJECT": ""}))

        xGridWindow.executeAction("SELECT", mkPropertyValues({"OBJECT": "Object 1"}))
        xGridWindow.executeAction("ACTIVATE", tuple())

        xCalcDoc = self.xUITest.getTopFocusWindow()
        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
