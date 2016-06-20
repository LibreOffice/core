# -*- Mode: python; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest_helper import UITest

from helper import mkPropertyValues

import time

try:
    import pyuno
    import uno
    import unohelper
except ImportError:
    print("pyuno not found: try to set PYTHONPATH and URE_BOOTSTRAP variables")
    print("PYTHONPATH=/installation/opt/program")
    print("URE_BOOTSTRAP=file:///installation/opt/program/fundamentalrc")
    raise

def add_content_to_cell(gridwin, cell, content):
    selectProps = mkPropertyValues({"CELL": cell})
    gridwin.executeAction("SELECT", selectProps)

    contentProps = mkPropertyValues({"TEXT": content})
    gridwin.executeAction("TYPE", contentProps)


def fill_spreadsheet(xUITest):
    xCalcDoc = xUITest.getTopFocusWindow()
    xGridWindow = xCalcDoc.getChild("grid_window")

    add_content_to_cell(xGridWindow, "A1", "col1")
    add_content_to_cell(xGridWindow, "B1", "col2")
    add_content_to_cell(xGridWindow, "C1", "col3")
    add_content_to_cell(xGridWindow, "A2", "1")
    add_content_to_cell(xGridWindow, "B2", "3")
    add_content_to_cell(xGridWindow, "C2", "5")

    xGridWindow.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:C2"}))

def cancel_immediately(xContext):
    xUITest = xContext.ServiceManager.createInstanceWithContext(
            "org.libreoffice.uitest.UITest", xContext)

    ui_test = UITest(xUITest, xContext)

    ui_test.create_doc_in_start_center("calc")

    fill_spreadsheet(xUITest)

    ui_test.execute_dialog_through_command(".uno:InsertObjectChart")

    xChartDlg = xUITest.getTopFocusWindow()

    xCancelBtn = xChartDlg.getChild("cancel")
    xCancelBtn.executeAction("CLICK", tuple())

    ui_test.close_doc()

def create_from_first_page(xContext):
    xUITest = xContext.ServiceManager.createInstanceWithContext(
            "org.libreoffice.uitest.UITest", xContext)

    ui_test = UITest(xUITest, xContext)

    ui_test.create_doc_in_start_center("calc")

    fill_spreadsheet(xUITest)

    ui_test.execute_dialog_through_command(".uno:InsertObjectChart")

    xChartDlg = xUITest.getTopFocusWindow()
    print(xChartDlg.getChildren())
    time.sleep(2)

    xOkBtn = xChartDlg.getChild("finish")
    xOkBtn.executeAction("CLICK", tuple())

    time.sleep(2)

    ui_test.close_doc()

def create_from_second_page(xContext):
    xUITest = xContext.ServiceManager.createInstanceWithContext(
            "org.libreoffice.uitest.UITest", xContext)

    ui_test = UITest(xUITest, xContext)

    ui_test.create_doc_in_start_center("calc")

    fill_spreadsheet(xUITest)

    ui_test.execute_dialog_through_command(".uno:InsertObjectChart")

    xChartDlg = xUITest.getTopFocusWindow()
    print(xChartDlg.getChildren())
    time.sleep(2)

    xNextBtn = xChartDlg.getChild("next")
    xNextBtn.executeAction("CLICK", tuple())

    print(xChartDlg.getChildren())

    time.sleep(2)

    xDataInRows = xChartDlg.getChild("RB_DATAROWS")
    xDataInRows.executeAction("CLICK", tuple())

    time.sleep(2)

    xDataInCols = xChartDlg.getChild("RB_DATACOLS")
    xDataInCols.executeAction("CLICK", tuple())

    time.sleep(2)

    xCancelBtn = xChartDlg.getChild("finish")
    xCancelBtn.executeAction("CLICK", tuple())

    time.sleep(5)

    ui_test.close_doc()

def deselect_chart(xContext):
    xUITest = xContext.ServiceManager.createInstanceWithContext(
            "org.libreoffice.uitest.UITest", xContext)

    ui_test = UITest(xUITest, xContext)

    ui_test.create_doc_in_start_center("calc")

    fill_spreadsheet(xUITest)

    xCalcDoc = xUITest.getTopFocusWindow()
    xGridWindow = xCalcDoc.getChild("grid_window")

    ui_test.execute_dialog_through_command(".uno:InsertObjectChart")

    xChartDlg = xUITest.getTopFocusWindow()

    xNextBtn = xChartDlg.getChild("finish")
    xNextBtn.executeAction("CLICK", tuple())

    xGridWindow.executeAction("DESELECT", mkPropertyValues({"OBJECT": ""}))

    time.sleep(2)

    ui_test.close_doc()

def activate_chart(xContext):
    xUITest = xContext.ServiceManager.createInstanceWithContext(
            "org.libreoffice.uitest.UITest", xContext)

    ui_test = UITest(xUITest, xContext)

    ui_test.create_doc_in_start_center("calc")

    fill_spreadsheet(xUITest)

    xCalcDoc = xUITest.getTopFocusWindow()
    xGridWindow = xCalcDoc.getChild("grid_window")

    ui_test.execute_dialog_through_command(".uno:InsertObjectChart")

    xChartDlg = xUITest.getTopFocusWindow()

    xNextBtn = xChartDlg.getChild("finish")
    xNextBtn.executeAction("CLICK", tuple())

    xGridWindow.executeAction("DESELECT", mkPropertyValues({"OBJECT": ""}))

    time.sleep(2)

    xGridWindow.executeAction("SELECT", mkPropertyValues({"OBJECT": "Object 1"}))
    xGridWindow.executeAction("ACTIVATE", tuple())

    time.sleep(2)

    xGridWindow.executeAction("DESELECT", mkPropertyValues({"OBJECT": ""}))

    ui_test.close_doc()

def select_chart_element(xContext):
    xUITest = xContext.ServiceManager.createInstanceWithContext(
            "org.libreoffice.uitest.UITest", xContext)

    ui_test = UITest(xUITest, xContext)

    ui_test.create_doc_in_start_center("calc")

    fill_spreadsheet(xUITest)

    xCalcDoc = xUITest.getTopFocusWindow()
    xGridWindow = xCalcDoc.getChild("grid_window")

    ui_test.execute_dialog_through_command(".uno:InsertObjectChart")

    xChartDlg = xUITest.getTopFocusWindow()

    xNextBtn = xChartDlg.getChild("finish")
    xNextBtn.executeAction("CLICK", tuple())

    xGridWindow.executeAction("DESELECT", mkPropertyValues({"OBJECT": ""}))

    time.sleep(2)

    xGridWindow.executeAction("SELECT", mkPropertyValues({"OBJECT": "Object 1"}))
    xGridWindow.executeAction("ACTIVATE", tuple())

    time.sleep(2)
    xCalcDoc = xUITest.getTopFocusWindow()
    print(xCalcDoc.getChildren())
    time.sleep(1)
    ui_test.close_doc()

# vim:set shiftwidth=4 softtabstop=4 expandtab: */
