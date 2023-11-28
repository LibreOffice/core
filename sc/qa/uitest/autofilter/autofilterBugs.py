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
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file
from uitest.uihelper.calc import enter_text_to_cell

class autofilter(UITestCase):

   def test_tdf115046(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf115046.ods")) as calc_doc:
            #autofilter exist
            self.assertEqual(calc_doc.getPropertyValue("UnnamedDatabaseRanges").getByTable(0).AutoFilter, True)

            with self.ui_test.execute_dialog_through_command(".uno:DataSort", close_button="cancel"):
                pass

            #autofilter still exist
            self.assertEqual(calc_doc.getPropertyValue("UnnamedDatabaseRanges").getByTable(0).AutoFilter, True)

   def test_tdf123095(self):
        with self.ui_test.create_doc_in_start_center("calc"):
            calcDoc = self.xUITest.getTopFocusWindow()
            xGridWindow = calcDoc.getChild("grid_window")
            enter_text_to_cell(xGridWindow, "A1", "乙二醇(进口料件)")
            enter_text_to_cell(xGridWindow, "A2", "乙二醇（进口料件）")
            xGridWindow.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:A2"}))

            with self.ui_test.execute_dialog_through_command(".uno:DataFilterAutoFilter", close_button="no"):
                pass

            xGridWindow.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": "0", "ROW": "0"}))
            xFloatWindow = self.xUITest.getFloatWindow()
            xTreeList = xFloatWindow.getChild("check_list_box")

            # Without the fix in place, the second entry would not exist
            self.assertEqual(2, len(xTreeList.getChildren()))
            self.assertEqual(get_state_as_dict(xTreeList.getChild("0"))["Text"], "乙二醇(进口料件)")
            self.assertEqual(get_state_as_dict(xTreeList.getChild("1"))["Text"], "乙二醇（进口料件）")

   def test_tdf125363(self):
        with self.ui_test.create_doc_in_start_center("calc"):
            calcDoc = self.xUITest.getTopFocusWindow()
            xGridWindow = calcDoc.getChild("grid_window")
            enter_text_to_cell(xGridWindow, "A1", "guet")
            enter_text_to_cell(xGridWindow, "A2", "guͤt")
            enter_text_to_cell(xGridWindow, "A3", "tuon")
            enter_text_to_cell(xGridWindow, "A4", "tuͦn")
            enter_text_to_cell(xGridWindow, "A5", "vröude")
            enter_text_to_cell(xGridWindow, "A6", "vröudᵉ")
            xGridWindow.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:A6"}))

            with self.ui_test.execute_dialog_through_command(".uno:DataFilterAutoFilter", close_button="no"):
                pass

            xGridWindow.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": "0", "ROW": "0"}))
            xFloatWindow = self.xUITest.getFloatWindow()
            xTreeList = xFloatWindow.getChild("check_list_box")

            # Without the fix in place, the entries with superscript/modifier letters would not exist
            self.assertEqual(6, len(xTreeList.getChildren()))
            self.assertEqual(get_state_as_dict(xTreeList.getChild("0"))["Text"], "guet")
            self.assertEqual(get_state_as_dict(xTreeList.getChild("1"))["Text"], "guͤt")
            self.assertEqual(get_state_as_dict(xTreeList.getChild("2"))["Text"], "tuon")
            self.assertEqual(get_state_as_dict(xTreeList.getChild("3"))["Text"], "tuͦn")
            self.assertEqual(get_state_as_dict(xTreeList.getChild("4"))["Text"], "vröude")
            self.assertEqual(get_state_as_dict(xTreeList.getChild("5"))["Text"], "vröudᵉ")

   def test_tdf158326(self):
        with self.ui_test.create_doc_in_start_center("calc"):
            calcDoc = self.xUITest.getTopFocusWindow()
            xGridWindow = calcDoc.getChild("grid_window")
            enter_text_to_cell(xGridWindow, "A1", "vröude")
            enter_text_to_cell(xGridWindow, "A2", "vröudᵉ")
            enter_text_to_cell(xGridWindow, "A3", "vröude")
            enter_text_to_cell(xGridWindow, "A4", "vröudᵉ")
            enter_text_to_cell(xGridWindow, "A5", "vröude")
            enter_text_to_cell(xGridWindow, "A6", "vröudᵉ")
            xGridWindow.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:A6"}))

            with self.ui_test.execute_dialog_through_command(".uno:DataFilterAutoFilter", close_button="no"):
                pass

            xGridWindow.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": "0", "ROW": "0"}))
            xFloatWindow = self.xUITest.getFloatWindow()
            xTreeList = xFloatWindow.getChild("check_list_box")

            # Without the fix in place, there would be 5 items since they will not be removed
            self.assertEqual(2, len(xTreeList.getChildren()))
            self.assertEqual(get_state_as_dict(xTreeList.getChild("0"))["Text"], "vröude")
            self.assertEqual(get_state_as_dict(xTreeList.getChild("1"))["Text"], "vröudᵉ")

   def test_tdf94055(self):
        with self.ui_test.create_doc_in_start_center("calc") as document:
            calcDoc = self.xUITest.getTopFocusWindow()
            xGridWindow = calcDoc.getChild("grid_window")
            enter_text_to_cell(xGridWindow, "A1", "X")
            enter_text_to_cell(xGridWindow, "B1", "Y")
            enter_text_to_cell(xGridWindow, "A2", "a")
            enter_text_to_cell(xGridWindow, "B2", "b")
            enter_text_to_cell(xGridWindow, "A3", "c")
            enter_text_to_cell(xGridWindow, "B3", "d")
            xGridWindow.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:B1"}))

            with self.ui_test.execute_dialog_through_command(".uno:DataFilterAutoFilter", close_button="yes"):
                pass

            xGridWindow.executeAction("SELECT", mkPropertyValues({"RANGE": "A2:A3"}))
            self.xUITest.executeCommand(".uno:SelectRow") #select two rows

            with self.ui_test.execute_dialog_through_command(".uno:DataSort"):
                pass

            #autofilter still exist
            self.assertEqual(document.getPropertyValue("UnnamedDatabaseRanges").getByTable(0).AutoFilter, True)


        #tdf77479.ods
   def test_tdf77479(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf77479.ods")) as calc_doc:
            calcDoc = self.xUITest.getTopFocusWindow()
            xGridWindow = calcDoc.getChild("grid_window")
            #autofilter exist
            self.assertEqual(calc_doc.getPropertyValue("UnnamedDatabaseRanges").getByTable(0).AutoFilter, True)

            xGridWindow.executeAction("SELECT", mkPropertyValues({"RANGE": "A2:E6"}))

            with self.ui_test.execute_dialog_through_command(".uno:DataSort"):
                pass

            #autofilter still exist
            self.assertEqual(calc_doc.getPropertyValue("UnnamedDatabaseRanges").getByTable(0).AutoFilter, True)

        #112656
   def test_tdf112656(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf112656.ods")):
            calcDoc = self.xUITest.getTopFocusWindow()
            xGridWindow = calcDoc.getChild("grid_window")

            xGridWindow.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": "2", "ROW": "0"}))
            xFloatWindow = self.xUITest.getFloatWindow()
            xTreeList = xFloatWindow.getChild("check_list_box")
            xFirstEntry = xTreeList.getChild("0")
            xSecondEntry = xTreeList.getChild("1")
            self.assertEqual(get_state_as_dict(xFirstEntry)["Text"], "Fruit")
            self.assertEqual(get_state_as_dict(xSecondEntry)["Text"], "Vegetables")

        #tdf81124
   def test_tdf81124(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf81124.ods")):
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

#tdf73565
   def test_tdf73565(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf73565.ods")):
            calcDoc = self.xUITest.getTopFocusWindow()
            xGridWindow = calcDoc.getChild("grid_window")

            xGridWindow.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": "0", "ROW": "0"}))
            xFloatWindow = self.xUITest.getFloatWindow()
            xTreeList = xFloatWindow.getChild("check_tree_box")
            xFirstEntry = xTreeList.getChild("0")
            self.assertEqual(get_state_as_dict(xFirstEntry)["Text"], "2014")

            xCancel = xFloatWindow.getChild("cancel")
            xCancel.executeAction("CLICK", tuple())

        #tdf65505
   def test_tdf65505(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf81124.ods")):
            calcDoc = self.xUITest.getTopFocusWindow()
            xGridWindow = calcDoc.getChild("grid_window")

            xGridWindow.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": "0", "ROW": "0"}))
            xFloatWindow = self.xUITest.getFloatWindow()
            xTreeList = xFloatWindow.getChild("check_list_box")
            xFirstEntry = xTreeList.getChild("0")
            self.assertEqual(get_state_as_dict(xFirstEntry)["Text"], "(empty)")

            xCancel = xFloatWindow.getChild("cancel")
            xCancel.executeAction("CLICK", tuple())

#tdf74857
   def test_tdf74857(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf74857.ods")):
            calcDoc = self.xUITest.getTopFocusWindow()
            xGridWindow = calcDoc.getChild("grid_window")

            xGridWindow.executeAction("TYPE", mkPropertyValues({"KEYCODE":"SHIFT+CTRL+DOWN"}))
            # Assert that the correct range has been selected
            gridWinState = get_state_as_dict(xGridWindow)
            self.assertEqual(gridWinState["MarkedArea"], "Sheet1.A1:Sheet1.A25")

        #tdf35294
   def test_tdf35294(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf35294.ods")):
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

        #tdf55712
   def test_tdf55712(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf55712.ods")):
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

            self.assertEqual(get_state_as_dict(x1Entry)["Text"], "(empty)")
            self.assertEqual(get_state_as_dict(x2Entry)["Text"], "0")
            self.assertEqual(get_state_as_dict(x3Entry)["Text"], "0.1")
            self.assertEqual(get_state_as_dict(x4Entry)["Text"], "0.2")
            self.assertEqual(get_state_as_dict(x5Entry)["Text"], "0.3")
            self.assertEqual(get_state_as_dict(x6Entry)["Text"], "0.5")
            self.assertEqual(get_state_as_dict(x7Entry)["Text"], "0.8")
            self.assertEqual(get_state_as_dict(x8Entry)["Text"], "0.9")
            self.assertEqual(get_state_as_dict(x9Entry)["Text"], "1")
            self.assertEqual(get_state_as_dict(xTreeList)["Children"], "9")

            xCancel = xFloatWindow.getChild("cancel")
            xCancel.executeAction("CLICK", tuple())

        #tdf152082
   def test_tdf152082(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf152082.ods")):
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")

            gridwin.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": "0", "ROW": "0"}))
            xFloatWindow = self.xUITest.getFloatWindow()
            xCheckListMenu = xFloatWindow.getChild("FilterDropDown")
            xTreeList = xCheckListMenu.getChild("check_list_box")
            self.assertEqual(4, len(xTreeList.getChildren()))
            self.assertEqual('true', get_state_as_dict(xTreeList.getChild('0'))['IsChecked'])
            self.assertEqual('true', get_state_as_dict(xTreeList.getChild('1'))['IsChecked'])
            self.assertEqual('true', get_state_as_dict(xTreeList.getChild('2'))['IsChecked'])
            self.assertEqual('false', get_state_as_dict(xTreeList.getChild('3'))['IsChecked'])
            xCancelBtn = xFloatWindow.getChild("cancel")
            xCancelBtn.executeAction("CLICK", tuple())

# vim: set shiftwidth=4 softtabstop=4 expandtab:
