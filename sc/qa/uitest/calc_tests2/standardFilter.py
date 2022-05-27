# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file
from uitest.uihelper.common import select_by_text
from uitest.uihelper.calc import enter_text_to_cell
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues

#Bug 113979 - Paste unformatted text does not ignore empty cells

class standardFilter(UITestCase):
    def test_standard_filter(self):
        with self.ui_test.load_file(get_url_for_data_file("standardFilter.ods")):
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")
            gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:C8"}))

            with self.ui_test.execute_modeless_dialog_through_command(".uno:DataFilterStandardFilter") as xDialog:
                xfield1 = xDialog.getChild("field1")
                xval1 = xDialog.getChild("val1")
                xconnect2 = xDialog.getChild("connect2")
                xfield2 = xDialog.getChild("field2")
                xval2 = xDialog.getChild("val2")

                select_by_text(xfield1, "a")
                xval1.executeAction("TYPE", mkPropertyValues({"TEXT":"1"}))
                select_by_text(xconnect2, "OR")
                select_by_text(xfield2, "b")
                xval2.executeAction("TYPE", mkPropertyValues({"TEXT":"3"}))
            #3x down - should be on row 9
            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
            gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "DOWN"}))
            gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "DOWN"}))
            gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "DOWN"}))
            gridWinState = get_state_as_dict(gridwin)
            self.assertEqual(gridWinState["CurrentRow"], "8")
            #reopen filter and verify
            gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "UP"}))
            gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "UP"}))
            gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "UP"}))
            with self.ui_test.execute_modeless_dialog_through_command(".uno:DataFilterStandardFilter") as xDialog:
                xfield1 = xDialog.getChild("field1")
                xval1 = xDialog.getChild("val1")
                xconnect2 = xDialog.getChild("connect2")
                xfield2 = xDialog.getChild("field2")
                xval2 = xDialog.getChild("val2")

                self.assertEqual(get_state_as_dict(xfield1)["SelectEntryText"], "a")
                self.assertEqual(get_state_as_dict(xfield2)["SelectEntryText"], "b")
                self.assertEqual(get_state_as_dict(xconnect2)["SelectEntryText"], "OR")
                self.assertEqual(get_state_as_dict(xval1)["Text"], "1")
                self.assertEqual(get_state_as_dict(xval2)["Text"], "3")

    def test_standard_filter_copy_result(self):
        with self.ui_test.load_file(get_url_for_data_file("standardFilter.ods")) as calc_doc:
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")
            gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:C8"}))

            with self.ui_test.execute_modeless_dialog_through_command(".uno:DataFilterStandardFilter") as xDialog:
                xfield1 = xDialog.getChild("field1")
                xval1 = xDialog.getChild("val1")
                xconnect2 = xDialog.getChild("connect2")
                xfield2 = xDialog.getChild("field2")
                xval2 = xDialog.getChild("val2")
                xcopyresult = xDialog.getChild("copyresult")
                xedcopyarea = xDialog.getChild("edcopyarea")
                select_by_text(xfield1, "a")
                xval1.executeAction("TYPE", mkPropertyValues({"TEXT":"1"}))
                select_by_text(xconnect2, "OR")
                select_by_text(xfield2, "b")
                xval2.executeAction("TYPE", mkPropertyValues({"TEXT":"3"}))
                xcopyresult.executeAction("CLICK", tuple())
                xedcopyarea.executeAction("TYPE", mkPropertyValues({"TEXT":"F1"}))
            #verify
            self.assertEqual(get_cell_by_position(calc_doc, 0, 5, 0).getString(), "a")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 6, 0).getString(), "b")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 7, 0).getString(), "c")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 5, 1).getValue(), 1)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 6, 1).getValue(), 2)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 7, 1).getValue(), 3)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 5, 2).getValue(), 2)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 6, 2).getValue(), 3)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 7, 2).getValue(), 4)

    def test_standard_filter_copy_result_next_sheet(self):
        with self.ui_test.load_file(get_url_for_data_file("standardFilter.ods")) as calc_doc:
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")
            gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:C8"}))

            with self.ui_test.execute_modeless_dialog_through_command(".uno:DataFilterStandardFilter") as xDialog:
                xfield1 = xDialog.getChild("field1")
                xval1 = xDialog.getChild("val1")
                xconnect2 = xDialog.getChild("connect2")
                xfield2 = xDialog.getChild("field2")
                xval2 = xDialog.getChild("val2")
                xcopyresult = xDialog.getChild("copyresult")
                xedcopyarea = xDialog.getChild("edcopyarea")
                select_by_text(xfield1, "a")
                xval1.executeAction("TYPE", mkPropertyValues({"TEXT":"1"}))
                select_by_text(xconnect2, "OR")
                select_by_text(xfield2, "b")
                xval2.executeAction("TYPE", mkPropertyValues({"TEXT":"3"}))
                xcopyresult.executeAction("CLICK", tuple())
                xedcopyarea.executeAction("TYPE", mkPropertyValues({"TEXT":"$Sheet2.$F$1"}))
            #verify
            self.assertEqual(get_cell_by_position(calc_doc, 1, 5, 0).getString(), "a")
            self.assertEqual(get_cell_by_position(calc_doc, 1, 6, 0).getString(), "b")
            self.assertEqual(get_cell_by_position(calc_doc, 1, 7, 0).getString(), "c")
            self.assertEqual(get_cell_by_position(calc_doc, 1, 5, 1).getValue(), 1)
            self.assertEqual(get_cell_by_position(calc_doc, 1, 6, 1).getValue(), 2)
            self.assertEqual(get_cell_by_position(calc_doc, 1, 7, 1).getValue(), 3)
            self.assertEqual(get_cell_by_position(calc_doc, 1, 5, 2).getValue(), 2)
            self.assertEqual(get_cell_by_position(calc_doc, 1, 6, 2).getValue(), 3)
            self.assertEqual(get_cell_by_position(calc_doc, 1, 7, 2).getValue(), 4)

    def test_standard_filter_case_sensitive(self):
        with self.ui_test.create_doc_in_start_center("calc"):
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")

            enter_text_to_cell(gridwin, "A1", "first")
            enter_text_to_cell(gridwin, "B1", "second")
            enter_text_to_cell(gridwin, "A2", "a1")
            enter_text_to_cell(gridwin, "A3", "A1")
            enter_text_to_cell(gridwin, "A4", "A1")
            enter_text_to_cell(gridwin, "B2", "4")
            enter_text_to_cell(gridwin, "B3", "5")
            enter_text_to_cell(gridwin, "B4", "6")
            gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:B4"}))
            with self.ui_test.execute_modeless_dialog_through_command(".uno:DataFilterStandardFilter") as xDialog:
                xfield1 = xDialog.getChild("field1")
                xval1 = xDialog.getChild("val1")
                xcase = xDialog.getChild("case")

                select_by_text(xfield1, "first")
                xval1.executeAction("TYPE", mkPropertyValues({"TEXT":"a1"}))
                xcase.executeAction("CLICK", tuple())
            #2x down - should be on row 5
            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
            gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "DOWN"}))
            gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "DOWN"}))
            gridWinState = get_state_as_dict(gridwin)
            self.assertEqual(gridWinState["CurrentRow"], "4")
            #reopen filter and verify
            gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "UP"}))
            gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "UP"}))
            with self.ui_test.execute_modeless_dialog_through_command(".uno:DataFilterStandardFilter") as xDialog:
                xfield1 = xDialog.getChild("field1")
                xval1 = xDialog.getChild("val1")
                xcase = xDialog.getChild("case")

                self.assertEqual(get_state_as_dict(xfield1)["SelectEntryText"], "first")
                self.assertEqual(get_state_as_dict(xval1)["Text"], "a1")
                self.assertEqual(get_state_as_dict(xcase)["Selected"], "true")

    def test_standard_filter_regular_expression(self):
        with self.ui_test.create_doc_in_start_center("calc"):
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")

            enter_text_to_cell(gridwin, "A1", "first")
            enter_text_to_cell(gridwin, "B1", "second")
            enter_text_to_cell(gridwin, "A2", "aa")
            enter_text_to_cell(gridwin, "A3", "aaa")
            enter_text_to_cell(gridwin, "A4", "abbb")
            enter_text_to_cell(gridwin, "A5", "accc")
            enter_text_to_cell(gridwin, "A6", "a*")
            enter_text_to_cell(gridwin, "B2", "1")
            enter_text_to_cell(gridwin, "B3", "2")
            enter_text_to_cell(gridwin, "B4", "3")
            enter_text_to_cell(gridwin, "B5", "4")
            enter_text_to_cell(gridwin, "B6", "5")
            gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:B6"}))
            with self.ui_test.execute_modeless_dialog_through_command(".uno:DataFilterStandardFilter") as xDialog:
                xfield1 = xDialog.getChild("field1")
                xval1 = xDialog.getChild("val1")
                xregexp = xDialog.getChild("regexp")

                select_by_text(xfield1, "first")
                xval1.executeAction("TYPE", mkPropertyValues({"TEXT":"a*"}))
                xregexp.executeAction("CLICK", tuple())
            #3x down - should be on row 7
            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
            gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "DOWN"}))
            gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "DOWN"}))
            gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "DOWN"}))
            gridWinState = get_state_as_dict(gridwin)
            self.assertEqual(gridWinState["CurrentRow"], "6")
            #reopen filter and verify
            gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "UP"}))
            gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "UP"}))
            gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "UP"}))
            with self.ui_test.execute_modeless_dialog_through_command(".uno:DataFilterStandardFilter") as xDialog:
                xfield1 = xDialog.getChild("field1")
                xval1 = xDialog.getChild("val1")
                xregexp = xDialog.getChild("regexp")

                self.assertEqual(get_state_as_dict(xfield1)["SelectEntryText"], "first")
                self.assertEqual(get_state_as_dict(xval1)["Text"], "a*")
                self.assertEqual(get_state_as_dict(xregexp)["Selected"], "true")

    def test_standard_filter_condition_contains(self):
        with self.ui_test.create_doc_in_start_center("calc"):
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")

            enter_text_to_cell(gridwin, "A1", "first")
            enter_text_to_cell(gridwin, "B1", "second")
            enter_text_to_cell(gridwin, "A2", "aa")
            enter_text_to_cell(gridwin, "A3", "aaa")
            enter_text_to_cell(gridwin, "A4", "abbb")
            enter_text_to_cell(gridwin, "A5", "accc")
            enter_text_to_cell(gridwin, "A6", "a*")
            enter_text_to_cell(gridwin, "B2", "1")
            enter_text_to_cell(gridwin, "B3", "2")
            enter_text_to_cell(gridwin, "B4", "3")
            enter_text_to_cell(gridwin, "B5", "4")
            enter_text_to_cell(gridwin, "B6", "5")
            gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:B6"}))
            with self.ui_test.execute_modeless_dialog_through_command(".uno:DataFilterStandardFilter") as xDialog:
                xfield1 = xDialog.getChild("field1")
                xval1 = xDialog.getChild("val1")
                xregexp = xDialog.getChild("regexp")

                select_by_text(xfield1, "first")
                xval1.executeAction("TYPE", mkPropertyValues({"TEXT":"a*"}))
                xregexp.executeAction("CLICK", tuple())
            #3x down - should be on row 7
            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
            gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "DOWN"}))
            gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "DOWN"}))
            gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "DOWN"}))
            gridWinState = get_state_as_dict(gridwin)
            self.assertEqual(gridWinState["CurrentRow"], "6")
            #reopen filter and verify
            gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "UP"}))
            gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "UP"}))
            gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "UP"}))
            with self.ui_test.execute_modeless_dialog_through_command(".uno:DataFilterStandardFilter") as xDialog:
                xfield1 = xDialog.getChild("field1")
                xval1 = xDialog.getChild("val1")
                xregexp = xDialog.getChild("regexp")

                self.assertEqual(get_state_as_dict(xfield1)["SelectEntryText"], "first")
                self.assertEqual(get_state_as_dict(xval1)["Text"], "a*")
                self.assertEqual(get_state_as_dict(xregexp)["Selected"], "true")

        #from testcasespecification OOo
    def test_standard_filter_condition_contains2(self):
        with self.ui_test.create_doc_in_start_center("calc"):
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")

            enter_text_to_cell(gridwin, "A1", "A")
            enter_text_to_cell(gridwin, "B1", "B")
            enter_text_to_cell(gridwin, "C1", "C")
            enter_text_to_cell(gridwin, "D1", "D")
            enter_text_to_cell(gridwin, "A2", "economics")
            enter_text_to_cell(gridwin, "B2", "34")
            enter_text_to_cell(gridwin, "C2", "67")
            enter_text_to_cell(gridwin, "D2", "122")
            enter_text_to_cell(gridwin, "A3", "socioeconomic")
            enter_text_to_cell(gridwin, "B3", "45")
            enter_text_to_cell(gridwin, "C3", "77")
            enter_text_to_cell(gridwin, "D3", "333")
            enter_text_to_cell(gridwin, "A4", "sociology")
            enter_text_to_cell(gridwin, "B4", "78")
            enter_text_to_cell(gridwin, "C4", "89")
            enter_text_to_cell(gridwin, "D4", "56")
            enter_text_to_cell(gridwin, "A5", "humanities")
            enter_text_to_cell(gridwin, "B5", "45")
            enter_text_to_cell(gridwin, "C5", "67")
            enter_text_to_cell(gridwin, "D5", "89")
            #Select
            gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:D5"}))
            #Choose DATA-FILTER-STANDARDFILTER
            #Choose field name "A"/ Choose condition "Contains"/Enter value "cio"/Press OK button
            with self.ui_test.execute_modeless_dialog_through_command(".uno:DataFilterStandardFilter") as xDialog:
                xfield1 = xDialog.getChild("field1")
                xval1 = xDialog.getChild("val1")
                xcond1 = xDialog.getChild("cond1")

                select_by_text(xfield1, "A")
                select_by_text(xcond1, "Contains")
                xval1.executeAction("TYPE", mkPropertyValues({"TEXT":"cio"}))

            #Verify that row 1,3, 4 are visible (2 and 5 are hidden)
            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
            gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "DOWN"}))
            gridWinState = get_state_as_dict(gridwin)
            self.assertEqual(gridWinState["CurrentRow"], "2")
            gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "DOWN"}))
            gridWinState = get_state_as_dict(gridwin)
            self.assertEqual(gridWinState["CurrentRow"], "3")
            #reopen filter and verify
            gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "UP"}))
            gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "UP"}))
            with self.ui_test.execute_modeless_dialog_through_command(".uno:DataFilterStandardFilter", close_button="cancel") as xDialog:
                xfield1 = xDialog.getChild("field1")
                xval1 = xDialog.getChild("val1")
                xcond1 = xDialog.getChild("cond1")
                self.assertEqual(get_state_as_dict(xfield1)["SelectEntryText"], "A")
                self.assertEqual(get_state_as_dict(xval1)["Text"], "cio")
                self.assertEqual(get_state_as_dict(xcond1)["SelectEntryText"], "Contains")

    def test_standard_filter_condition_does_not_contains(self):
        with self.ui_test.create_doc_in_start_center("calc"):
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")

            enter_text_to_cell(gridwin, "A1", "A")
            enter_text_to_cell(gridwin, "B1", "B")
            enter_text_to_cell(gridwin, "C1", "C")
            enter_text_to_cell(gridwin, "D1", "D")
            enter_text_to_cell(gridwin, "A2", "economics")
            enter_text_to_cell(gridwin, "B2", "34")
            enter_text_to_cell(gridwin, "C2", "67")
            enter_text_to_cell(gridwin, "D2", "122")
            enter_text_to_cell(gridwin, "A3", "socioeconomic")
            enter_text_to_cell(gridwin, "B3", "45")
            enter_text_to_cell(gridwin, "C3", "77")
            enter_text_to_cell(gridwin, "D3", "333")
            enter_text_to_cell(gridwin, "A4", "sociology")
            enter_text_to_cell(gridwin, "B4", "78")
            enter_text_to_cell(gridwin, "C4", "89")
            enter_text_to_cell(gridwin, "D4", "56")
            enter_text_to_cell(gridwin, "A5", "humanities")
            enter_text_to_cell(gridwin, "B5", "45")
            enter_text_to_cell(gridwin, "C5", "67")
            enter_text_to_cell(gridwin, "D5", "89")
            #Select
            gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:D5"}))
            #Choose DATA-FILTER-STANDARDFILTER
            #Choose field name "A"/ Choose condition "Does not contain"/Enter value "cio"/Press OK button
            with self.ui_test.execute_modeless_dialog_through_command(".uno:DataFilterStandardFilter") as xDialog:
                xfield1 = xDialog.getChild("field1")
                xval1 = xDialog.getChild("val1")
                xcond1 = xDialog.getChild("cond1")

                select_by_text(xfield1, "A")
                select_by_text(xcond1, "Does not contain")
                xval1.executeAction("TYPE", mkPropertyValues({"TEXT":"cio"}))

            #Verify that row 1,2, 5 are visible (3 and 4 are hidden)
            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
            gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "DOWN"}))
            gridWinState = get_state_as_dict(gridwin)
            self.assertEqual(gridWinState["CurrentRow"], "1")
            gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "DOWN"}))
            gridWinState = get_state_as_dict(gridwin)
            self.assertEqual(gridWinState["CurrentRow"], "4")
            #reopen filter and verify
            gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "UP"}))
            gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "UP"}))
            with self.ui_test.execute_modeless_dialog_through_command(".uno:DataFilterStandardFilter", close_button="cancel") as xDialog:
                xfield1 = xDialog.getChild("field1")
                xval1 = xDialog.getChild("val1")
                xcond1 = xDialog.getChild("cond1")
                self.assertEqual(get_state_as_dict(xfield1)["SelectEntryText"], "A")
                self.assertEqual(get_state_as_dict(xval1)["Text"], "cio")
                self.assertEqual(get_state_as_dict(xcond1)["SelectEntryText"], "Does not contain")


    def test_standard_filter_condition_Begins_with(self):
        with self.ui_test.create_doc_in_start_center("calc"):
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")

            enter_text_to_cell(gridwin, "A1", "A")
            enter_text_to_cell(gridwin, "B1", "B")
            enter_text_to_cell(gridwin, "C1", "C")
            enter_text_to_cell(gridwin, "D1", "D")
            enter_text_to_cell(gridwin, "A2", "economics")
            enter_text_to_cell(gridwin, "B2", "34")
            enter_text_to_cell(gridwin, "C2", "67")
            enter_text_to_cell(gridwin, "D2", "122")
            enter_text_to_cell(gridwin, "A3", "socioeconomic")
            enter_text_to_cell(gridwin, "B3", "45")
            enter_text_to_cell(gridwin, "C3", "77")
            enter_text_to_cell(gridwin, "D3", "333")
            enter_text_to_cell(gridwin, "A4", "sociology")
            enter_text_to_cell(gridwin, "B4", "78")
            enter_text_to_cell(gridwin, "C4", "89")
            enter_text_to_cell(gridwin, "D4", "56")
            enter_text_to_cell(gridwin, "A5", "humanities")
            enter_text_to_cell(gridwin, "B5", "45")
            enter_text_to_cell(gridwin, "C5", "67")
            enter_text_to_cell(gridwin, "D5", "89")
            #Select
            gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:D5"}))
            #Choose DATA-FILTER-STANDARDFILTER
            #Choose field name "A"/ Choose condition "Begins with"/Enter value "si"/Press OK button
            with self.ui_test.execute_modeless_dialog_through_command(".uno:DataFilterStandardFilter") as xDialog:
                xfield1 = xDialog.getChild("field1")
                xval1 = xDialog.getChild("val1")
                xcond1 = xDialog.getChild("cond1")

                select_by_text(xfield1, "A")
                select_by_text(xcond1, "Begins with")
                xval1.executeAction("TYPE", mkPropertyValues({"TEXT":"so"}))

            #Verify that row 1,3, 4 are visible (2 and 5 are hidden)
            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
            gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "DOWN"}))
            gridWinState = get_state_as_dict(gridwin)
            self.assertEqual(gridWinState["CurrentRow"], "2")
            gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "DOWN"}))
            gridWinState = get_state_as_dict(gridwin)
            self.assertEqual(gridWinState["CurrentRow"], "3")
            #reopen filter and verify
            gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "UP"}))
            gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "UP"}))
            with self.ui_test.execute_modeless_dialog_through_command(".uno:DataFilterStandardFilter", close_button="cancel") as xDialog:
                xfield1 = xDialog.getChild("field1")
                xval1 = xDialog.getChild("val1")
                xcond1 = xDialog.getChild("cond1")
                self.assertEqual(get_state_as_dict(xfield1)["SelectEntryText"], "A")
                self.assertEqual(get_state_as_dict(xval1)["Text"], "so")
                self.assertEqual(get_state_as_dict(xcond1)["SelectEntryText"], "Begins with")


    def test_standard_filter_condition_Does_not_begin_with(self):
        with self.ui_test.create_doc_in_start_center("calc"):
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")

            enter_text_to_cell(gridwin, "A1", "A")
            enter_text_to_cell(gridwin, "B1", "B")
            enter_text_to_cell(gridwin, "C1", "C")
            enter_text_to_cell(gridwin, "D1", "D")
            enter_text_to_cell(gridwin, "A2", "economics")
            enter_text_to_cell(gridwin, "B2", "34")
            enter_text_to_cell(gridwin, "C2", "67")
            enter_text_to_cell(gridwin, "D2", "122")
            enter_text_to_cell(gridwin, "A3", "socioeconomic")
            enter_text_to_cell(gridwin, "B3", "45")
            enter_text_to_cell(gridwin, "C3", "77")
            enter_text_to_cell(gridwin, "D3", "333")
            enter_text_to_cell(gridwin, "A4", "sociology")
            enter_text_to_cell(gridwin, "B4", "78")
            enter_text_to_cell(gridwin, "C4", "89")
            enter_text_to_cell(gridwin, "D4", "56")
            enter_text_to_cell(gridwin, "A5", "humanities")
            enter_text_to_cell(gridwin, "B5", "45")
            enter_text_to_cell(gridwin, "C5", "67")
            enter_text_to_cell(gridwin, "D5", "89")
            #Select
            gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:D5"}))
            #Choose DATA-FILTER-STANDARDFILTER
            #Choose field name "A"/ Choose condition "Does not contain"/Enter value "cio"/Press OK button
            with self.ui_test.execute_modeless_dialog_through_command(".uno:DataFilterStandardFilter") as xDialog:
                xfield1 = xDialog.getChild("field1")
                xval1 = xDialog.getChild("val1")
                xcond1 = xDialog.getChild("cond1")

                select_by_text(xfield1, "A")
                select_by_text(xcond1, "Does not begin with")
                xval1.executeAction("TYPE", mkPropertyValues({"TEXT":"so"}))

            #Verify that row 1,2, 5 are visible (3 and 4 are hidden)
            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
            gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "DOWN"}))
            gridWinState = get_state_as_dict(gridwin)
            self.assertEqual(gridWinState["CurrentRow"], "1")
            gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "DOWN"}))
            gridWinState = get_state_as_dict(gridwin)
            self.assertEqual(gridWinState["CurrentRow"], "4")
            #reopen filter and verify
            gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "UP"}))
            gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "UP"}))
            with self.ui_test.execute_modeless_dialog_through_command(".uno:DataFilterStandardFilter", close_button="cancel") as xDialog:
                xfield1 = xDialog.getChild("field1")
                xval1 = xDialog.getChild("val1")
                xcond1 = xDialog.getChild("cond1")
                self.assertEqual(get_state_as_dict(xfield1)["SelectEntryText"], "A")
                self.assertEqual(get_state_as_dict(xval1)["Text"], "so")
                self.assertEqual(get_state_as_dict(xcond1)["SelectEntryText"], "Does not begin with")


    def test_standard_filter_condition_Ends_with(self):
        with self.ui_test.create_doc_in_start_center("calc"):
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")

            enter_text_to_cell(gridwin, "A1", "A")
            enter_text_to_cell(gridwin, "B1", "B")
            enter_text_to_cell(gridwin, "C1", "C")
            enter_text_to_cell(gridwin, "D1", "D")
            enter_text_to_cell(gridwin, "A2", "economics")
            enter_text_to_cell(gridwin, "B2", "34")
            enter_text_to_cell(gridwin, "C2", "67")
            enter_text_to_cell(gridwin, "D2", "122")
            enter_text_to_cell(gridwin, "A3", "socioeconomic")
            enter_text_to_cell(gridwin, "B3", "45")
            enter_text_to_cell(gridwin, "C3", "77")
            enter_text_to_cell(gridwin, "D3", "333")
            enter_text_to_cell(gridwin, "A4", "sociology")
            enter_text_to_cell(gridwin, "B4", "78")
            enter_text_to_cell(gridwin, "C4", "89")
            enter_text_to_cell(gridwin, "D4", "56")
            enter_text_to_cell(gridwin, "A5", "humanities")
            enter_text_to_cell(gridwin, "B5", "45")
            enter_text_to_cell(gridwin, "C5", "67")
            enter_text_to_cell(gridwin, "D5", "89")
            #Select
            gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:D5"}))
            #Choose DATA-FILTER-STANDARDFILTER
            #Choose field name "A"/ Choose condition "Does not contain"/Enter value "cio"/Press OK button
            with self.ui_test.execute_modeless_dialog_through_command(".uno:DataFilterStandardFilter") as xDialog:
                xfield1 = xDialog.getChild("field1")
                xval1 = xDialog.getChild("val1")
                xcond1 = xDialog.getChild("cond1")

                select_by_text(xfield1, "A")
                select_by_text(xcond1, "Ends with")
                xval1.executeAction("TYPE", mkPropertyValues({"TEXT":"s"}))

            #Verify that row 1,2, 5 are visible (3 and 4 are hidden)
            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
            gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "DOWN"}))
            gridWinState = get_state_as_dict(gridwin)
            self.assertEqual(gridWinState["CurrentRow"], "1")
            gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "DOWN"}))
            gridWinState = get_state_as_dict(gridwin)
            self.assertEqual(gridWinState["CurrentRow"], "4")
            #reopen filter and verify
            gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "UP"}))
            gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "UP"}))
            with self.ui_test.execute_modeless_dialog_through_command(".uno:DataFilterStandardFilter", close_button="cancel") as xDialog:
                xfield1 = xDialog.getChild("field1")
                xval1 = xDialog.getChild("val1")
                xcond1 = xDialog.getChild("cond1")
                self.assertEqual(get_state_as_dict(xfield1)["SelectEntryText"], "A")
                self.assertEqual(get_state_as_dict(xval1)["Text"], "s")
                self.assertEqual(get_state_as_dict(xcond1)["SelectEntryText"], "Ends with")


    def test_standard_filter_condition_Does_not_end_with(self):
        with self.ui_test.create_doc_in_start_center("calc"):
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")

            enter_text_to_cell(gridwin, "A1", "A")
            enter_text_to_cell(gridwin, "B1", "B")
            enter_text_to_cell(gridwin, "C1", "C")
            enter_text_to_cell(gridwin, "D1", "D")
            enter_text_to_cell(gridwin, "A2", "economics")
            enter_text_to_cell(gridwin, "B2", "34")
            enter_text_to_cell(gridwin, "C2", "67")
            enter_text_to_cell(gridwin, "D2", "122")
            enter_text_to_cell(gridwin, "A3", "socioeconomic")
            enter_text_to_cell(gridwin, "B3", "45")
            enter_text_to_cell(gridwin, "C3", "77")
            enter_text_to_cell(gridwin, "D3", "333")
            enter_text_to_cell(gridwin, "A4", "sociology")
            enter_text_to_cell(gridwin, "B4", "78")
            enter_text_to_cell(gridwin, "C4", "89")
            enter_text_to_cell(gridwin, "D4", "56")
            enter_text_to_cell(gridwin, "A5", "humanities")
            enter_text_to_cell(gridwin, "B5", "45")
            enter_text_to_cell(gridwin, "C5", "67")
            enter_text_to_cell(gridwin, "D5", "89")
            #Select
            gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:D5"}))
            #Choose DATA-FILTER-STANDARDFILTER
            #Choose field name "A"/ Choose condition "Begins with"/Enter value "si"/Press OK button
            with self.ui_test.execute_modeless_dialog_through_command(".uno:DataFilterStandardFilter") as xDialog:
                xfield1 = xDialog.getChild("field1")
                xval1 = xDialog.getChild("val1")
                xcond1 = xDialog.getChild("cond1")

                select_by_text(xfield1, "A")
                select_by_text(xcond1, "Does not end with")
                xval1.executeAction("TYPE", mkPropertyValues({"TEXT":"s"}))

            #Verify that row 1,3, 4 are visible (2 and 5 are hidden)
            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
            gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "DOWN"}))
            gridWinState = get_state_as_dict(gridwin)
            self.assertEqual(gridWinState["CurrentRow"], "2")
            gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "DOWN"}))
            gridWinState = get_state_as_dict(gridwin)
            self.assertEqual(gridWinState["CurrentRow"], "3")
            #reopen filter and verify
            gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "UP"}))
            gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "UP"}))
            with self.ui_test.execute_modeless_dialog_through_command(".uno:DataFilterStandardFilter", close_button="cancel") as xDialog:
                xfield1 = xDialog.getChild("field1")
                xval1 = xDialog.getChild("val1")
                xcond1 = xDialog.getChild("cond1")
                self.assertEqual(get_state_as_dict(xfield1)["SelectEntryText"], "A")
                self.assertEqual(get_state_as_dict(xval1)["Text"], "s")
                self.assertEqual(get_state_as_dict(xcond1)["SelectEntryText"], "Does not end with")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
