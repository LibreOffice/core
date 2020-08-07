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
#deletecontents.ui
#+ Bug 101904 - Delete Contents dialog -- won't delete cell content "Date & time"
class clearCells(UITestCase):
    def test_clear_cells_text(self):
        calc_doc = self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        enter_text_to_cell(gridwin, "A1", "aa")
        enter_text_to_cell(gridwin, "A2", "1")

        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:A2"}))
        self.ui_test.execute_dialog_through_command(".uno:Delete")
        xDialog = self.xUITest.getTopFocusWindow()
        xdeleteall = xDialog.getChild("deleteall")
        xtext = xDialog.getChild("text")
        xdatetime = xDialog.getChild("datetime")
        xcomments = xDialog.getChild("comments")
        xobjects = xDialog.getChild("objects")
        xnumbers = xDialog.getChild("numbers")
        xformulas = xDialog.getChild("formulas")
        xformats = xDialog.getChild("formats")

        if (get_state_as_dict(xdeleteall)["Selected"]) == "true":
            xdeleteall.executeAction("CLICK", tuple())
        if (get_state_as_dict(xtext)["Selected"]) == "false":
            xtext.executeAction("CLICK", tuple())
        if (get_state_as_dict(xdatetime)["Selected"]) == "true":
            xdatetime.executeAction("CLICK", tuple())
        if (get_state_as_dict(xcomments)["Selected"]) == "true":
            xcomments.executeAction("CLICK", tuple())
        if (get_state_as_dict(xobjects)["Selected"]) == "true":
            xobjects.executeAction("CLICK", tuple())
        if (get_state_as_dict(xnumbers)["Selected"]) == "true":
            xnumbers.executeAction("CLICK", tuple())
        if (get_state_as_dict(xformulas)["Selected"]) == "true":
            xformulas.executeAction("CLICK", tuple())
        if (get_state_as_dict(xformats)["Selected"]) == "true":
            xformats.executeAction("CLICK", tuple())

        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)
        #Verify
        self.assertEqual(get_cell_by_position(document, 0, 0, 0).getString() , "")
        self.assertEqual(get_cell_by_position(document, 0, 0, 1).getValue() , 1)

        self.ui_test.close_doc()

    def test_clear_cells_date_tdf101904(self):
        calc_doc = self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        enter_text_to_cell(gridwin, "A1", "01/01/2000")
        enter_text_to_cell(gridwin, "A2", "1")

        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:A2"}))
        self.ui_test.execute_dialog_through_command(".uno:Delete")
        xDialog = self.xUITest.getTopFocusWindow()
        xdeleteall = xDialog.getChild("deleteall")
        xtext = xDialog.getChild("text")
        xdatetime = xDialog.getChild("datetime")
        xcomments = xDialog.getChild("comments")
        xobjects = xDialog.getChild("objects")
        xnumbers = xDialog.getChild("numbers")
        xformulas = xDialog.getChild("formulas")
        xformats = xDialog.getChild("formats")

        if (get_state_as_dict(xdeleteall)["Selected"]) == "true":
            xdeleteall.executeAction("CLICK", tuple())
        if (get_state_as_dict(xtext)["Selected"]) == "true":
            xtext.executeAction("CLICK", tuple())
        if (get_state_as_dict(xdatetime)["Selected"]) == "false":
            xdatetime.executeAction("CLICK", tuple())
        if (get_state_as_dict(xcomments)["Selected"]) == "true":
            xcomments.executeAction("CLICK", tuple())
        if (get_state_as_dict(xobjects)["Selected"]) == "true":
            xobjects.executeAction("CLICK", tuple())
        if (get_state_as_dict(xnumbers)["Selected"]) == "true":
            xnumbers.executeAction("CLICK", tuple())
        if (get_state_as_dict(xformulas)["Selected"]) == "true":
            xformulas.executeAction("CLICK", tuple())
        if (get_state_as_dict(xformats)["Selected"]) == "true":
            xformats.executeAction("CLICK", tuple())

        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)
        #Verify
        self.assertEqual(get_cell_by_position(document, 0, 0, 0).getString() , "")
        self.assertEqual(get_cell_by_position(document, 0, 0, 1).getValue() , 1)

        self.ui_test.close_doc()

    def test_clear_cells_number(self):
        calc_doc = self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        enter_text_to_cell(gridwin, "A1", "aa")
        enter_text_to_cell(gridwin, "A2", "1")

        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:A2"}))
        self.ui_test.execute_dialog_through_command(".uno:Delete")
        xDialog = self.xUITest.getTopFocusWindow()
        xdeleteall = xDialog.getChild("deleteall")
        xtext = xDialog.getChild("text")
        xdatetime = xDialog.getChild("datetime")
        xcomments = xDialog.getChild("comments")
        xobjects = xDialog.getChild("objects")
        xnumbers = xDialog.getChild("numbers")
        xformulas = xDialog.getChild("formulas")
        xformats = xDialog.getChild("formats")

        if (get_state_as_dict(xdeleteall)["Selected"]) == "true":
            xdeleteall.executeAction("CLICK", tuple())
        if (get_state_as_dict(xtext)["Selected"]) == "true":
            xtext.executeAction("CLICK", tuple())
        if (get_state_as_dict(xdatetime)["Selected"]) == "true":
            xdatetime.executeAction("CLICK", tuple())
        if (get_state_as_dict(xcomments)["Selected"]) == "true":
            xcomments.executeAction("CLICK", tuple())
        if (get_state_as_dict(xobjects)["Selected"]) == "true":
            xobjects.executeAction("CLICK", tuple())
        if (get_state_as_dict(xnumbers)["Selected"]) == "false":
            xnumbers.executeAction("CLICK", tuple())
        if (get_state_as_dict(xformulas)["Selected"]) == "true":
            xformulas.executeAction("CLICK", tuple())
        if (get_state_as_dict(xformats)["Selected"]) == "true":
            xformats.executeAction("CLICK", tuple())

        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)
        #Verify
        self.assertEqual(get_cell_by_position(document, 0, 0, 0).getString() , "aa")
        self.assertEqual(get_cell_by_position(document, 0, 0, 1).getValue() , 0)

        self.ui_test.close_doc()
    def test_clear_cells_formulas(self):
        calc_doc = self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        enter_text_to_cell(gridwin, "A1", "1")
        enter_text_to_cell(gridwin, "A2", "=A1+1")

        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:A2"}))
        self.ui_test.execute_dialog_through_command(".uno:Delete")
        xDialog = self.xUITest.getTopFocusWindow()
        xdeleteall = xDialog.getChild("deleteall")
        xtext = xDialog.getChild("text")
        xdatetime = xDialog.getChild("datetime")
        xcomments = xDialog.getChild("comments")
        xobjects = xDialog.getChild("objects")
        xnumbers = xDialog.getChild("numbers")
        xformulas = xDialog.getChild("formulas")
        xformats = xDialog.getChild("formats")

        if (get_state_as_dict(xdeleteall)["Selected"]) == "true":
            xdeleteall.executeAction("CLICK", tuple())
        if (get_state_as_dict(xtext)["Selected"]) == "true":
            xtext.executeAction("CLICK", tuple())
        if (get_state_as_dict(xdatetime)["Selected"]) == "true":
            xdatetime.executeAction("CLICK", tuple())
        if (get_state_as_dict(xcomments)["Selected"]) == "true":
            xcomments.executeAction("CLICK", tuple())
        if (get_state_as_dict(xobjects)["Selected"]) == "true":
            xobjects.executeAction("CLICK", tuple())
        if (get_state_as_dict(xnumbers)["Selected"]) == "true":
            xnumbers.executeAction("CLICK", tuple())
        if (get_state_as_dict(xformulas)["Selected"]) == "false":
            xformulas.executeAction("CLICK", tuple())
        if (get_state_as_dict(xformats)["Selected"]) == "true":
            xformats.executeAction("CLICK", tuple())

        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)
        #Verify
        self.assertEqual(get_cell_by_position(document, 0, 0, 0).getString() , "1")
        self.assertEqual(get_cell_by_position(document, 0, 0, 1).getString() , "")

        self.ui_test.close_doc()

    def test_clear_cells_formats(self):
        calc_doc = self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        enter_text_to_cell(gridwin, "A1", "aa")
        enter_text_to_cell(gridwin, "A2", "1")

        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:A2"}))
        self.xUITest.executeCommand(".uno:Bold")
        self.ui_test.execute_dialog_through_command(".uno:Delete")
        xDialog = self.xUITest.getTopFocusWindow()
        xdeleteall = xDialog.getChild("deleteall")
        xtext = xDialog.getChild("text")
        xdatetime = xDialog.getChild("datetime")
        xcomments = xDialog.getChild("comments")
        xobjects = xDialog.getChild("objects")
        xnumbers = xDialog.getChild("numbers")
        xformulas = xDialog.getChild("formulas")
        xformats = xDialog.getChild("formats")

        if (get_state_as_dict(xdeleteall)["Selected"]) == "true":
            xdeleteall.executeAction("CLICK", tuple())
        if (get_state_as_dict(xtext)["Selected"]) == "true":
            xtext.executeAction("CLICK", tuple())
        if (get_state_as_dict(xdatetime)["Selected"]) == "true":
            xdatetime.executeAction("CLICK", tuple())
        if (get_state_as_dict(xcomments)["Selected"]) == "true":
            xcomments.executeAction("CLICK", tuple())
        if (get_state_as_dict(xobjects)["Selected"]) == "true":
            xobjects.executeAction("CLICK", tuple())
        if (get_state_as_dict(xnumbers)["Selected"]) == "true":
            xnumbers.executeAction("CLICK", tuple())
        if (get_state_as_dict(xformulas)["Selected"]) == "true":
            xformulas.executeAction("CLICK", tuple())
        if (get_state_as_dict(xformats)["Selected"]) == "false":
            xformats.executeAction("CLICK", tuple())

        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)
        #Verify
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
        self.ui_test.execute_dialog_through_command(".uno:FormatCellDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        xTabs = xDialog.getChild("tabcontrol")
        select_pos(xTabs, "1")  #tab Font

        xstylelb = xDialog.getChild("weststylelb-cjk")
        print(get_state_as_dict(xstylelb))


        self.assertEqual(get_state_as_dict(xstylelb)["Text"], "Regular")

        xOK = xDialog.getChild("ok")
        xOK.executeAction("CLICK", tuple())

        self.ui_test.close_doc()

    def test_clear_cells_all(self):
        calc_doc = self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        enter_text_to_cell(gridwin, "A1", "aa")
        enter_text_to_cell(gridwin, "A2", "1")

        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:A2"}))
        self.xUITest.executeCommand(".uno:Bold")
        self.ui_test.execute_dialog_through_command(".uno:Delete")
        xDialog = self.xUITest.getTopFocusWindow()
        xdeleteall = xDialog.getChild("deleteall")
        xtext = xDialog.getChild("text")
        xdatetime = xDialog.getChild("datetime")
        xcomments = xDialog.getChild("comments")
        xobjects = xDialog.getChild("objects")
        xnumbers = xDialog.getChild("numbers")
        xformulas = xDialog.getChild("formulas")
        xformats = xDialog.getChild("formats")

        if (get_state_as_dict(xdeleteall)["Selected"]) == "false":
            xdeleteall.executeAction("CLICK", tuple())

        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)
        #Verify
        self.assertEqual(get_cell_by_position(document, 0, 0, 0).getString() , "")
        self.assertEqual(get_cell_by_position(document, 0, 0, 1).getString() , "")

        self.ui_test.close_doc()

    def test_cancel_clear_cells_all(self):
        calc_doc = self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        enter_text_to_cell(gridwin, "A1", "aa")
        enter_text_to_cell(gridwin, "A2", "1")

        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:A2"}))
        self.ui_test.execute_dialog_through_command(".uno:Delete")
        xDialog = self.xUITest.getTopFocusWindow()
        xdeleteall = xDialog.getChild("deleteall")

        if (get_state_as_dict(xdeleteall)["Selected"]) == "false":
            xdeleteall.executeAction("CLICK", tuple())

        xCancelBtn = xDialog.getChild("cancel")
        self.ui_test.close_dialog_through_button(xCancelBtn)
        #Verify
        self.assertEqual(get_cell_by_position(document, 0, 0, 0).getString() , "aa")
        self.assertEqual(get_cell_by_position(document, 0, 0, 1).getString() , "1")

        self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab:
