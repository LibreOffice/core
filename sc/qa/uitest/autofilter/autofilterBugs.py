# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file
from uitest.uihelper.calc import enter_text_to_cell

class autofilter(UITestCase):

   def test_tdf115046(self):
        calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf115046.ods"))
        document = self.ui_test.get_component()
        #autofilter exist
        self.assertEqual(document.getPropertyValue("UnnamedDatabaseRanges").getByTable(0).AutoFilter, True)

        self.ui_test.execute_dialog_through_command(".uno:DataSort")
        xDialog = self.xUITest.getTopFocusWindow()

        xcancBtn = xDialog.getChild("cancel")
        self.ui_test.close_dialog_through_button(xcancBtn)
        #autofilter still exist
        self.assertEqual(document.getPropertyValue("UnnamedDatabaseRanges").getByTable(0).AutoFilter, True)

        self.ui_test.close_doc()

   def test_tdf94055(self):
        self.ui_test.create_doc_in_start_center("calc")
        document = self.ui_test.get_component()
        calcDoc = self.xUITest.getTopFocusWindow()
        xGridWindow = calcDoc.getChild("grid_window")
        enter_text_to_cell(xGridWindow, "A1", "X")
        enter_text_to_cell(xGridWindow, "B1", "Y")
        enter_text_to_cell(xGridWindow, "A2", "a")
        enter_text_to_cell(xGridWindow, "B2", "b")
        enter_text_to_cell(xGridWindow, "A3", "c")
        enter_text_to_cell(xGridWindow, "B3", "d")
        xGridWindow.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:B1"}))

        self.ui_test.execute_dialog_through_command(".uno:DataFilterAutoFilter")
        xDialog = self.xUITest.getTopFocusWindow()
        xYesBtn = xDialog.getChild("yes")
        self.ui_test.close_dialog_through_button(xYesBtn)

        xGridWindow.executeAction("SELECT", mkPropertyValues({"RANGE": "A2:A3"}))
        self.xUITest.executeCommand(".uno:SelectRow") #select two rows

        self.ui_test.execute_dialog_through_command(".uno:DataSort")
        xDialog = self.xUITest.getTopFocusWindow()
        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)
        #autofilter still exist
        self.assertEqual(document.getPropertyValue("UnnamedDatabaseRanges").getByTable(0).AutoFilter, True)

        self.ui_test.close_doc()

        #tdf77479.ods
   def test_tdf77479(self):
        calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf77479.ods"))
        document = self.ui_test.get_component()
        calcDoc = self.xUITest.getTopFocusWindow()
        xGridWindow = calcDoc.getChild("grid_window")
        #autofilter exist
        self.assertEqual(document.getPropertyValue("UnnamedDatabaseRanges").getByTable(0).AutoFilter, True)

        xGridWindow.executeAction("SELECT", mkPropertyValues({"RANGE": "A2:E6"}))

        self.ui_test.execute_dialog_through_command(".uno:DataSort")
        xDialog = self.xUITest.getTopFocusWindow()
        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)
        #autofilter still exist
        self.assertEqual(document.getPropertyValue("UnnamedDatabaseRanges").getByTable(0).AutoFilter, True)

        self.ui_test.close_doc()

        #112656
   def test_tdf112656(self):
        calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf112656.ods"))
        document = self.ui_test.get_component()
        calcDoc = self.xUITest.getTopFocusWindow()
        xGridWindow = calcDoc.getChild("grid_window")

        xGridWindow.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": "2", "ROW": "0"}))
        xFloatWindow = self.xUITest.getFloatWindow()
        xTreeList = xFloatWindow.getChild("check_list_box")
        xFirstEntry = xTreeList.getChild("0")
        xSecondEntry = xTreeList.getChild("1")
        self.assertEqual(get_state_as_dict(xFirstEntry)["Text"], "Fruit")
        self.assertEqual(get_state_as_dict(xSecondEntry)["Text"], "Vegetables")

        self.ui_test.close_doc()

        #tdf81124
   def test_tdf81124(self):
        calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf81124.ods"))
        document = self.ui_test.get_component()
        calcDoc = self.xUITest.getTopFocusWindow()
        xGridWindow = calcDoc.getChild("grid_window")

        xGridWindow.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": "0", "ROW": "0"}))
        xFloatWindow = self.xUITest.getFloatWindow()

        xUnselect = xFloatWindow.getChild("unselect_current")
        xUnselect.executeAction("CLICK", tuple())
        xokBtn = xFloatWindow.getChild("ok")
        self.assertEqual(get_state_as_dict(xokBtn)["Enabled"], "false")

        xSelect = xFloatWindow.getChild("select_current")
        xSelect.executeAction("CLICK", tuple())
        xokBtn = xFloatWindow.getChild("ok")
        self.assertEqual(get_state_as_dict(xokBtn)["Enabled"], "true")

        xCancel = xFloatWindow.getChild("cancel")
        xCancel.executeAction("CLICK", tuple())

        self.ui_test.close_doc()

#tdf73565
   def test_tdf73565(self):
        calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf73565.ods"))
        document = self.ui_test.get_component()
        calcDoc = self.xUITest.getTopFocusWindow()
        xGridWindow = calcDoc.getChild("grid_window")

        xGridWindow.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": "0", "ROW": "0"}))
        xFloatWindow = self.xUITest.getFloatWindow()
        xTreeList = xFloatWindow.getChild("check_tree_box")
        xFirstEntry = xTreeList.getChild("0")
        self.assertEqual(get_state_as_dict(xFirstEntry)["Text"], "2014")

        xCancel = xFloatWindow.getChild("cancel")
        xCancel.executeAction("CLICK", tuple())

        self.ui_test.close_doc()

        #tdf65505
   def test_tdf65505(self):
        calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf81124.ods"))
        document = self.ui_test.get_component()
        calcDoc = self.xUITest.getTopFocusWindow()
        xGridWindow = calcDoc.getChild("grid_window")

        xGridWindow.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": "0", "ROW": "0"}))
        xFloatWindow = self.xUITest.getFloatWindow()
        xTreeList = xFloatWindow.getChild("check_list_box")
        xFirstEntry = xTreeList.getChild("0")
        self.assertEqual(get_state_as_dict(xFirstEntry)["Text"], "(empty)")

        xCancel = xFloatWindow.getChild("cancel")
        xCancel.executeAction("CLICK", tuple())

        self.ui_test.close_doc()

#tdf74857
   def test_tdf74857(self):
        calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf74857.ods"))
        document = self.ui_test.get_component()
        calcDoc = self.xUITest.getTopFocusWindow()
        xGridWindow = calcDoc.getChild("grid_window")

        xGridWindow.executeAction("TYPE", mkPropertyValues({"KEYCODE":"SHIFT+CTRL+DOWN"}))
        # Assert that the correct range has been selected
        gridWinState = get_state_as_dict(xGridWindow)
        self.assertEqual(gridWinState["MarkedArea"], "Sheet1.A1:Sheet1.A25")

        self.ui_test.close_doc()

        #tdf35294
   def test_tdf35294(self):
        calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf35294.ods"))
        document = self.ui_test.get_component()
        calcDoc = self.xUITest.getTopFocusWindow()
        xGridWindow = calcDoc.getChild("grid_window")

        xGridWindow.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": "0", "ROW": "0"}))
        xFloatWindow = self.xUITest.getFloatWindow()
        xTreeList = xFloatWindow.getChild("check_list_box")
        x1Entry = xTreeList.getChild("0")
        x2Entry = xTreeList.getChild("1")
        x3Entry = xTreeList.getChild("2")
        x4Entry = xTreeList.getChild("3")
        x5Entry = xTreeList.getChild("4")
        x6Entry = xTreeList.getChild("5")
        x7Entry = xTreeList.getChild("6")

        self.assertEqual(get_state_as_dict(x1Entry)["Text"], "a")
        self.assertEqual(get_state_as_dict(x2Entry)["Text"], "á")
        self.assertEqual(get_state_as_dict(x3Entry)["Text"], "b")
        self.assertEqual(get_state_as_dict(x4Entry)["Text"], "č")
        self.assertEqual(get_state_as_dict(x5Entry)["Text"], "é")
        self.assertEqual(get_state_as_dict(x6Entry)["Text"], "ř")
        self.assertEqual(get_state_as_dict(x7Entry)["Text"], "ž")

        xCancel = xFloatWindow.getChild("cancel")
        xCancel.executeAction("CLICK", tuple())

        self.ui_test.close_doc()

        #tdf55712
   def test_tdf55712(self):
        calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf55712.ods"))
        document = self.ui_test.get_component()
        calcDoc = self.xUITest.getTopFocusWindow()
        xGridWindow = calcDoc.getChild("grid_window")

        xGridWindow.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": "0", "ROW": "0"}))
        xFloatWindow = self.xUITest.getFloatWindow()
        xTreeList = xFloatWindow.getChild("check_list_box")
        x1Entry = xTreeList.getChild("0")
        x2Entry = xTreeList.getChild("1")
        x3Entry = xTreeList.getChild("2")
        x4Entry = xTreeList.getChild("3")
        x5Entry = xTreeList.getChild("4")
        x6Entry = xTreeList.getChild("5")
        x7Entry = xTreeList.getChild("6")
        x8Entry = xTreeList.getChild("7")
        x9Entry = xTreeList.getChild("8")

        self.assertEqual(get_state_as_dict(x1Entry)["Text"], "0")
        self.assertEqual(get_state_as_dict(x2Entry)["Text"], "0.1")
        self.assertEqual(get_state_as_dict(x3Entry)["Text"], "0.2")
        self.assertEqual(get_state_as_dict(x4Entry)["Text"], "0.3")
        self.assertEqual(get_state_as_dict(x5Entry)["Text"], "0.5")
        self.assertEqual(get_state_as_dict(x6Entry)["Text"], "0.8")
        self.assertEqual(get_state_as_dict(x7Entry)["Text"], "0.9")
        self.assertEqual(get_state_as_dict(x8Entry)["Text"], "1")
        self.assertEqual(get_state_as_dict(x9Entry)["Text"], "(empty)")
        self.assertEqual(get_state_as_dict(xTreeList)["Children"], "9")

        xCancel = xFloatWindow.getChild("cancel")
        xCancel.executeAction("CLICK", tuple())

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
