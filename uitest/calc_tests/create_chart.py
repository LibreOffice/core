# -*- Mode: python; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from libreoffice.uno.propertyvalue import mkPropertyValues

from uitest.framework import UITestCase

import time

class CalcChartUIDemo(UITestCase):

    def add_content_to_cell(self, gridwin, cell, content):
        selectProps = mkPropertyValues({"CELL": cell})
        gridwin.executeAction("SELECT", selectProps)

        contentProps = mkPropertyValues({"TEXT": content})
        gridwin.executeAction("TYPE", contentProps)

    def fill_spreadsheet(self):
        xCalcDoc = self.xUITest.getTopFocusWindow()
        xGridWindow = xCalcDoc.getChild("grid_window")

        self.add_content_to_cell(xGridWindow, "A1", "col1")
        self.add_content_to_cell(xGridWindow, "B1", "col2")
        self.add_content_to_cell(xGridWindow, "C1", "col3")
        self.add_content_to_cell(xGridWindow, "A2", "1")
        self.add_content_to_cell(xGridWindow, "B2", "3")
        self.add_content_to_cell(xGridWindow, "C2", "5")

        xGridWindow.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:C2"}))

    def test_cancel_immediately(self):

        self.ui_test.create_doc_in_start_center("calc")

        self.fill_spreadsheet()

        self.ui_test.execute_dialog_through_command(".uno:InsertObjectChart")

        xChartDlg = self.xUITest.getTopFocusWindow()

        xCancelBtn = xChartDlg.getChild("cancel")
        xCancelBtn.executeAction("CLICK", tuple())

        self.ui_test.close_doc()

    def test_create_from_first_page(self):

        self.ui_test.create_doc_in_start_center("calc")

        self.fill_spreadsheet()

        self.ui_test.execute_dialog_through_command(".uno:InsertObjectChart")

        xChartDlg = self.xUITest.getTopFocusWindow()

        xOkBtn = xChartDlg.getChild("finish")
        xOkBtn.executeAction("CLICK", tuple())

        self.ui_test.close_doc()

    def test_create_from_second_page(self):

        self.ui_test.create_doc_in_start_center("calc")

        self.fill_spreadsheet()

        self.ui_test.execute_dialog_through_command(".uno:InsertObjectChart")

        xChartDlg = self.xUITest.getTopFocusWindow()

        xNextBtn = xChartDlg.getChild("next")
        xNextBtn.executeAction("CLICK", tuple())

        xDataInRows = xChartDlg.getChild("RB_DATAROWS")
        xDataInRows.executeAction("CLICK", tuple())

        xDataInCols = xChartDlg.getChild("RB_DATACOLS")
        xDataInCols.executeAction("CLICK", tuple())

        xCancelBtn = xChartDlg.getChild("finish")
        xCancelBtn.executeAction("CLICK", tuple())

        self.ui_test.close_doc()

    def test_deselect_chart(self):
        self.ui_test.create_doc_in_start_center("calc")

        self.fill_spreadsheet()

        xCalcDoc = self.xUITest.getTopFocusWindow()
        xGridWindow = xCalcDoc.getChild("grid_window")

        self.ui_test.execute_dialog_through_command(".uno:InsertObjectChart")

        xChartDlg = self.xUITest.getTopFocusWindow()

        xNextBtn = xChartDlg.getChild("finish")
        xNextBtn.executeAction("CLICK", tuple())

        xGridWindow.executeAction("DESELECT", mkPropertyValues({"OBJECT": ""}))

        time.sleep(2)

        self.ui_test.close_doc()

    def test_activate_chart(self):

        self.ui_test.create_doc_in_start_center("calc")

        self.fill_spreadsheet()

        xCalcDoc = self.xUITest.getTopFocusWindow()
        xGridWindow = xCalcDoc.getChild("grid_window")

        self.ui_test.execute_dialog_through_command(".uno:InsertObjectChart")

        xChartDlg = self.xUITest.getTopFocusWindow()

        xNextBtn = xChartDlg.getChild("finish")
        xNextBtn.executeAction("CLICK", tuple())

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

        self.ui_test.execute_dialog_through_command(".uno:InsertObjectChart")

        xChartDlg = self.xUITest.getTopFocusWindow()

        xNextBtn = xChartDlg.getChild("finish")
        xNextBtn.executeAction("CLICK", tuple())

        xGridWindow.executeAction("DESELECT", mkPropertyValues({"OBJECT": ""}))

        xGridWindow.executeAction("SELECT", mkPropertyValues({"OBJECT": "Object 1"}))
        xGridWindow.executeAction("ACTIVATE", tuple())

        xCalcDoc = self.xUITest.getTopFocusWindow()
        self.ui_test.close_doc()

# vim:set shiftwidth=4 softtabstop=4 expandtab: */
