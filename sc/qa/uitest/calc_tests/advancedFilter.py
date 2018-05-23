# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict
from uitest.uihelper.common import select_pos
from uitest.uihelper.calc import enter_text_to_cell
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues
# import org.libreoffice.unotest
# import pathlib
from uitest.path import get_srcdir_url

def get_url_for_data_file(file_name):
#    return pathlib.Path(org.libreoffice.unotest.makeCopyFromTDOC(file_name)).as_uri()
    return get_srcdir_url() + "/sc/qa/uitest/calc_tests/data/" + file_name

#advancedfilterdialog.ui
class advancedFilter(UITestCase):
    def test_advanced_filter_named_area(self):
        calc_doc = self.ui_test.load_file(get_url_for_data_file("advancedFilter.ods"))
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:B6"}))

        self.ui_test.execute_modeless_dialog_through_command(".uno:DataFilterSpecialFilter")
        xDialog = self.xUITest.getTopFocusWindow()
        xlbfilterarea  = xDialog.getChild("lbfilterarea")
        xedfilterarea = xDialog.getChild("edfilterarea")
        xregexp  = xDialog.getChild("regexp")
        if (get_state_as_dict(xregexp)["Selected"]) == "true":
            xregexp.executeAction("CLICK", tuple())
        props = {"TEXT": "name1"}
        actionProps = mkPropertyValues(props)
        xlbfilterarea.executeAction("SELECT", actionProps)

        self.assertEqual(get_state_as_dict(xedfilterarea)["Text"], "$Sheet1.$E$11:$F$12")
        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)
        #verify
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
        gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "DOWN"}))
        gridWinState = get_state_as_dict(gridwin)
        self.assertEqual(gridWinState["CurrentRow"], "1")
        gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "DOWN"}))
        gridWinState = get_state_as_dict(gridwin)
        self.assertEqual(gridWinState["CurrentRow"], "4")
        #reopen filter
        self.ui_test.execute_modeless_dialog_through_command(".uno:DataFilterSpecialFilter")
        xDialog = self.xUITest.getTopFocusWindow()
        xedfilterarea = xDialog.getChild("edfilterarea")
        self.assertEqual(get_state_as_dict(xedfilterarea)["Text"], "$Sheet1.$E$11:$F$12")
        xCancelBtn = xDialog.getChild("cancel")
        self.ui_test.close_dialog_through_button(xCancelBtn)
        #undo
        self.xUITest.executeCommand(".uno:Undo")
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
        gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "DOWN"}))
        gridWinState = get_state_as_dict(gridwin)
        self.assertEqual(gridWinState["CurrentRow"], "1")
        gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "DOWN"}))
        gridWinState = get_state_as_dict(gridwin)
        self.assertEqual(gridWinState["CurrentRow"], "2")

        self.ui_test.close_doc()

    def test_advanced_filter(self):
        calc_doc = self.ui_test.load_file(get_url_for_data_file("advancedFilter.ods"))
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:B6"}))

        self.ui_test.execute_modeless_dialog_through_command(".uno:DataFilterSpecialFilter")
        xDialog = self.xUITest.getTopFocusWindow()
        xlbfilterarea  = xDialog.getChild("lbfilterarea")
        xedfilterarea = xDialog.getChild("edfilterarea")
        xregexp  = xDialog.getChild("regexp")
        if (get_state_as_dict(xregexp)["Selected"]) == "true":
            xregexp.executeAction("CLICK", tuple())
        xedfilterarea.executeAction("TYPE", mkPropertyValues({"TEXT":"$Sheet1.$E$11:$F$12"}))
        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)
        #verify
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
        gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "DOWN"}))
        gridWinState = get_state_as_dict(gridwin)
        self.assertEqual(gridWinState["CurrentRow"], "1")
        gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "DOWN"}))
        gridWinState = get_state_as_dict(gridwin)
        self.assertEqual(gridWinState["CurrentRow"], "4")
        #reopen filter
        self.ui_test.execute_modeless_dialog_through_command(".uno:DataFilterSpecialFilter")
        xDialog = self.xUITest.getTopFocusWindow()
        xedfilterarea = xDialog.getChild("edfilterarea")
        self.assertEqual(get_state_as_dict(xedfilterarea)["Text"], "$Sheet1.$E$11:$F$12")
        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)
        self.ui_test.close_doc()

    def test_advanced_filter_2(self):
        calc_doc = self.ui_test.load_file(get_url_for_data_file("advancedFilter.ods"))
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:B6"}))

        self.ui_test.execute_modeless_dialog_through_command(".uno:DataFilterSpecialFilter")
        xDialog = self.xUITest.getTopFocusWindow()
        xlbfilterarea  = xDialog.getChild("lbfilterarea")
        xedfilterarea = xDialog.getChild("edfilterarea")
        xregexp  = xDialog.getChild("regexp")
        if (get_state_as_dict(xregexp)["Selected"]) == "true":
            xregexp.executeAction("CLICK", tuple())
        xedfilterarea.executeAction("TYPE", mkPropertyValues({"TEXT":"$Sheet1.$E$16:$F$17"}))
        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)
        #verify
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
        gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "DOWN"}))
        gridWinState = get_state_as_dict(gridwin)
        self.assertEqual(gridWinState["CurrentRow"], "3")
        gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "DOWN"}))
        gridWinState = get_state_as_dict(gridwin)
        self.assertEqual(gridWinState["CurrentRow"], "4")
        #reopen filter
        self.ui_test.execute_modeless_dialog_through_command(".uno:DataFilterSpecialFilter")
        xDialog = self.xUITest.getTopFocusWindow()
        xedfilterarea = xDialog.getChild("edfilterarea")
        self.assertEqual(get_state_as_dict(xedfilterarea)["Text"], "$Sheet1.$E$16:$F$17")
        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)
        self.ui_test.close_doc()

    def test_advanced_filter_two_values(self):
        calc_doc = self.ui_test.load_file(get_url_for_data_file("advancedFilter.ods"))
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:B6"}))

        self.ui_test.execute_modeless_dialog_through_command(".uno:DataFilterSpecialFilter")
        xDialog = self.xUITest.getTopFocusWindow()
        xlbfilterarea  = xDialog.getChild("lbfilterarea")
        xedfilterarea = xDialog.getChild("edfilterarea")
        xregexp  = xDialog.getChild("regexp")
        if (get_state_as_dict(xregexp)["Selected"]) == "true":
            xregexp.executeAction("CLICK", tuple())
        xedfilterarea.executeAction("TYPE", mkPropertyValues({"TEXT":"$Sheet1.$E$20:$G$21"}))
        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)
        #verify
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
        gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "DOWN"}))
        gridWinState = get_state_as_dict(gridwin)
        self.assertEqual(gridWinState["CurrentRow"], "3")
        gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "DOWN"}))
        gridWinState = get_state_as_dict(gridwin)
        self.assertEqual(gridWinState["CurrentRow"], "4")
        #reopen filter
        self.ui_test.execute_modeless_dialog_through_command(".uno:DataFilterSpecialFilter")
        xDialog = self.xUITest.getTopFocusWindow()
        xedfilterarea = xDialog.getChild("edfilterarea")
        self.assertEqual(get_state_as_dict(xedfilterarea)["Text"], "$Sheet1.$E$20:$G$21")
        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)
        self.ui_test.close_doc()

    def test_advanced_filter_regular_expression(self):
        calc_doc = self.ui_test.load_file(get_url_for_data_file("advancedFilter.ods"))
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:B6"}))

        self.ui_test.execute_modeless_dialog_through_command(".uno:DataFilterSpecialFilter")
        xDialog = self.xUITest.getTopFocusWindow()
        xregexp  = xDialog.getChild("regexp")
        xedfilterarea = xDialog.getChild("edfilterarea")
        xedfilterarea.executeAction("TYPE", mkPropertyValues({"TEXT":"$Sheet1.$E$24:$F$25"}))
        xregexp.executeAction("CLICK", tuple())
        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)
        #verify
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
        gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "DOWN"}))
        gridWinState = get_state_as_dict(gridwin)
        self.assertEqual(gridWinState["CurrentRow"], "1")
        gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "DOWN"}))
        gridWinState = get_state_as_dict(gridwin)
        self.assertEqual(gridWinState["CurrentRow"], "4")
        #reopen filter
        self.ui_test.execute_modeless_dialog_through_command(".uno:DataFilterSpecialFilter")
        xDialog = self.xUITest.getTopFocusWindow()
        xedfilterarea = xDialog.getChild("edfilterarea")
        xregexp  = xDialog.getChild("regexp")
        self.assertEqual(get_state_as_dict(xedfilterarea)["Text"], "$Sheet1.$E$24:$F$25")
        self.assertEqual(get_state_as_dict(xregexp)["Selected"], "true")
        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)
        self.ui_test.close_doc()

    def test_advanced_filter_case_sensitive(self):
        calc_doc = self.ui_test.load_file(get_url_for_data_file("advancedFilter.ods"))
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:B6"}))

        self.ui_test.execute_modeless_dialog_through_command(".uno:DataFilterSpecialFilter")
        xDialog = self.xUITest.getTopFocusWindow()
        xregexp  = xDialog.getChild("regexp")
        xedfilterarea = xDialog.getChild("edfilterarea")
        xedfilterarea.executeAction("TYPE", mkPropertyValues({"TEXT":"$Sheet1.$E$24:$F$25"}))
        xregexp.executeAction("CLICK", tuple())
        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)
        #verify
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
        gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "DOWN"}))
        gridWinState = get_state_as_dict(gridwin)
        self.assertEqual(gridWinState["CurrentRow"], "1")
        gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "DOWN"}))
        gridWinState = get_state_as_dict(gridwin)
        self.assertEqual(gridWinState["CurrentRow"], "4")
        #reopen filter
        self.ui_test.execute_modeless_dialog_through_command(".uno:DataFilterSpecialFilter")
        xDialog = self.xUITest.getTopFocusWindow()
        xedfilterarea = xDialog.getChild("edfilterarea")
        xregexp  = xDialog.getChild("regexp")
        self.assertEqual(get_state_as_dict(xedfilterarea)["Text"], "$Sheet1.$E$24:$F$25")
        self.assertEqual(get_state_as_dict(xregexp)["Selected"], "true")
        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)
        self.ui_test.close_doc()

    def test_advanced_filter_copy_result(self):
        calc_doc = self.ui_test.load_file(get_url_for_data_file("advancedFilter.ods"))
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:B6"}))

        self.ui_test.execute_modeless_dialog_through_command(".uno:DataFilterSpecialFilter")
        xDialog = self.xUITest.getTopFocusWindow()
        xlbfilterarea  = xDialog.getChild("lbfilterarea")
        xedfilterarea = xDialog.getChild("edfilterarea")
        xcopyresult  = xDialog.getChild("copyresult")
        xedcopyarea  = xDialog.getChild("edcopyarea")
        if (get_state_as_dict(xcopyresult)["Selected"]) == "false":
            xcopyresult.executeAction("CLICK", tuple())
        xedfilterarea.executeAction("TYPE", mkPropertyValues({"TEXT":"$Sheet1.$E$11:$F$12"}))
        xedcopyarea.executeAction("TYPE", mkPropertyValues({"TEXT":"E1"}))
        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)
        #verify
        self.assertEqual(get_cell_by_position(document, 0, 4, 0).getString(), "Name")
        self.assertEqual(get_cell_by_position(document, 0, 4, 1).getString(), "AAA")
        self.assertEqual(get_cell_by_position(document, 0, 4, 2).getString(), "AAA")
        self.assertEqual(get_cell_by_position(document, 0, 5, 0).getString(), "Value")
        self.assertEqual(get_cell_by_position(document, 0, 5, 1).getValue(), 1)
        self.assertEqual(get_cell_by_position(document, 0, 5, 2).getValue(), 4)

        self.ui_test.close_doc()

    def test_advanced_filter_named_area_OOo122862(self):
        calc_doc = self.ui_test.load_file(get_url_for_data_file("advancedFilter2.ods"))
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A2:F15"}))

        self.ui_test.execute_modeless_dialog_through_command(".uno:DataFilterSpecialFilter")
        xDialog = self.xUITest.getTopFocusWindow()
        xlbfilterarea  = xDialog.getChild("lbfilterarea")
        xedfilterarea = xDialog.getChild("edfilterarea")
        xregexp  = xDialog.getChild("regexp")
        if (get_state_as_dict(xregexp)["Selected"]) == "true":
            xregexp.executeAction("CLICK", tuple())
        props = {"TEXT": "Anywhere1"}
        actionProps = mkPropertyValues(props)
        xlbfilterarea.executeAction("SELECT", actionProps)

        self.assertEqual(get_state_as_dict(xedfilterarea)["Text"], "$FC.$A$2:$F$7")
        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)
        #verify
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A2"}))
        gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "DOWN"}))
        gridWinState = get_state_as_dict(gridwin)
        self.assertEqual(gridWinState["CurrentRow"], "6")
        gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "DOWN"}))
        gridWinState = get_state_as_dict(gridwin)
        self.assertEqual(gridWinState["CurrentRow"], "7")
        #reopen filter
        self.ui_test.execute_modeless_dialog_through_command(".uno:DataFilterSpecialFilter")
        xDialog = self.xUITest.getTopFocusWindow()
        xedfilterarea = xDialog.getChild("edfilterarea")
        self.assertEqual(get_state_as_dict(xedfilterarea)["Text"], "$FC.$A$2:$F$7")
        xCancelBtn = xDialog.getChild("cancel")
        self.ui_test.close_dialog_through_button(xCancelBtn)
        #undo
        self.xUITest.executeCommand(".uno:Undo")
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A2"}))
        gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "DOWN"}))
        gridWinState = get_state_as_dict(gridwin)
        self.assertEqual(gridWinState["CurrentRow"], "2")
        gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "DOWN"}))
        gridWinState = get_state_as_dict(gridwin)
        self.assertEqual(gridWinState["CurrentRow"], "3")

        self.ui_test.close_doc()

    def test_advanced_filter_not_equal(self):
        calc_doc = self.ui_test.load_file(get_url_for_data_file("advancedFilter.ods"))
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:B6"}))

        self.ui_test.execute_modeless_dialog_through_command(".uno:DataFilterSpecialFilter")
        xDialog = self.xUITest.getTopFocusWindow()
        xlbfilterarea  = xDialog.getChild("lbfilterarea")
        xedfilterarea = xDialog.getChild("edfilterarea")
        xregexp  = xDialog.getChild("regexp")
        if (get_state_as_dict(xregexp)["Selected"]) == "true":
            xregexp.executeAction("CLICK", tuple())
        xedfilterarea.executeAction("TYPE", mkPropertyValues({"TEXT":"$Sheet1.$E$32:$F$33"}))
        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)
        #verify
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
        gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "DOWN"}))
        gridWinState = get_state_as_dict(gridwin)
        self.assertEqual(gridWinState["CurrentRow"], "1")
        gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "DOWN"}))
        gridWinState = get_state_as_dict(gridwin)
        self.assertEqual(gridWinState["CurrentRow"], "3")
        #reopen filter
        self.ui_test.execute_modeless_dialog_through_command(".uno:DataFilterSpecialFilter")
        xDialog = self.xUITest.getTopFocusWindow()
        xedfilterarea = xDialog.getChild("edfilterarea")
        self.assertEqual(get_state_as_dict(xedfilterarea)["Text"], "$Sheet1.$E$32:$F$33")
        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)
        self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab:
