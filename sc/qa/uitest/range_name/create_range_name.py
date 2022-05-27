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
from uitest.uihelper.common import type_text, select_pos
from uitest.uihelper.common import get_state_as_dict
from libreoffice.calc.document import get_cell_by_position
from uitest.uihelper.calc import enter_text_to_cell

class CreateRangeNameTest(UITestCase):

    def test_create_range_name(self):

        with self.ui_test.create_doc_in_start_center("calc"):

            calcDoc = self.xUITest.getTopFocusWindow()
            xPosWindow = calcDoc.getChild('pos_window')
            self.assertEqual('A1', get_state_as_dict(xPosWindow)['Text'])

            with self.ui_test.execute_modeless_dialog_through_command(".uno:AddName", close_button="add") as xAddNameDlg:


                xEdit = xAddNameDlg.getChild("edit")
                type_text(xEdit, "globalRangeName")


            self.assertEqual('globalRangeName', get_state_as_dict(xPosWindow)['Text'])


    def test_create_range_name_from_ui(self):

        with self.ui_test.create_doc_in_start_center("calc") as document:

            calcDoc = self.xUITest.getTopFocusWindow()
            gridwin = calcDoc.getChild("grid_window")

            enter_text_to_cell(gridwin, "A1", "1")
            enter_text_to_cell(gridwin, "B1", "1")
            enter_text_to_cell(gridwin, "C1", "1")

            gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:C1"}))
            xPosWindow = calcDoc.getChild('pos_window')
            self.assertEqual('A1:C1', get_state_as_dict(xPosWindow)['Text'])

            xPosWindow.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
            xPosWindow.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
            xPosWindow.executeAction("TYPE", mkPropertyValues({"TEXT":"RANGE1"}))
            xPosWindow.executeAction("TYPE", mkPropertyValues({"KEYCODE":"RETURN"}))

            self.assertEqual('RANGE1', get_state_as_dict(xPosWindow)['Text'])

            calcDoc = self.xUITest.getTopFocusWindow()
            gridwin = calcDoc.getChild("grid_window")

            enter_text_to_cell(gridwin, "A2", "=SUM(RANGE1)")
            self.assertEqual(3.0, get_cell_by_position(document, 0, 0, 1).getValue())

            # Change the name
            with self.ui_test.execute_dialog_through_command(".uno:DefineName") as xDialog:
                xNamesList = xDialog.getChild('names')
                self.assertEqual(1, len(xNamesList.getChildren()))

                xName = xDialog.getChild('name')
                self.assertEqual( 'RANGE1', get_state_as_dict(xName)["Text"])

                xName.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
                xName.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
                xName.executeAction("TYPE", mkPropertyValues({"TEXT":"RANGE2"}))


            # tdf#87474 check the formula is updated after changing the name
            self.assertEqual("=SUM(RANGE2)", get_cell_by_position(document, 0, 0, 1).getFormula())

            self.xUITest.executeCommand(".uno:Undo")

            self.assertEqual("=SUM(RANGE1)", get_cell_by_position(document, 0, 0, 1).getFormula())


    def test_create_local_range_name(self):

        with self.ui_test.create_doc_in_start_center("calc") as document:

            calcDoc = self.xUITest.getTopFocusWindow()
            xPosWindow = calcDoc.getChild('pos_window')
            self.assertEqual('A1', get_state_as_dict(xPosWindow)['Text'])

            with self.ui_test.execute_modeless_dialog_through_command(".uno:AddName", close_button="add") as xAddNameDlg:


                xEdit = xAddNameDlg.getChild("edit")
                type_text(xEdit, "localRangeName")

                xScope = xAddNameDlg.getChild("scope")
                select_pos(xScope, "1")


            # tdf#67007: Without the fix in place, this test would have failed with
            # AssertionError: 'localRangeName' != 'A1'
            # Additionally, newly check a sheet-local scoped name has " (sheetname)" appended.
            self.assertEqual('localRangeName (Sheet1)', get_state_as_dict(xPosWindow)['Text'])

            gridwin = calcDoc.getChild("grid_window")
            enter_text_to_cell(gridwin, "A1", "1")

            # Use the name range in the current sheet
            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "B1"}))

            with self.ui_test.execute_dialog_through_command(".uno:InsertName", close_button="paste") as xDialog:

                xCtrl = xDialog.getChild('ctrl')
                self.assertEqual(1, len(xCtrl.getChildren()))
                self.assertEqual("localRangeName\t$Sheet1.$A$1\tSheet1", get_state_as_dict(xCtrl.getChild('0'))['Text'])
                xCtrl.getChild('0').executeAction("SELECT", tuple())

            # use return key to paste the name range
            gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "RETURN"}))

            self.assertEqual("1", get_cell_by_position(document, 0, 1, 0).getString())
            self.assertEqual("=localRangeName", get_cell_by_position(document, 0, 1, 0).getFormula())

            # Insert a new sheet
            with self.ui_test.execute_dialog_through_command(".uno:Insert"):
                pass

            # Use the name range in the new sheet
            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "B1"}))

            with self.ui_test.execute_dialog_through_command(".uno:InsertName", close_button="paste") as xDialog:

                xCtrl = xDialog.getChild('ctrl')
                self.assertEqual(1, len(xCtrl.getChildren()))
                self.assertEqual("localRangeName\t$Sheet1.$A$1\tSheet1", get_state_as_dict(xCtrl.getChild('0'))['Text'])
                xCtrl.getChild('0').executeAction("SELECT", tuple())

            # use return key to paste the name range
            gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "RETURN"}))

            # tdf#137896: Without the fix in place, this test would have failed with
            # AssertionError: '1' != '#NAME?'
            self.assertEqual("1", get_cell_by_position(document, 0, 1, 0).getString())

            # and AssertionError: '=Sheet1.localRangeName' != '=localrangename'
            self.assertEqual("=Sheet1.localRangeName", get_cell_by_position(document, 0, 1, 0).getFormula())

            with self.ui_test.execute_dialog_through_command(".uno:DefineName") as xDialog:

                # tdf#138851: Without the fix in place, this test would have failed with
                # AssertionError: 'Sheet1' != 'Document (Global)'
                xScope = xDialog.getChild("scope")
                self.assertEqual("Sheet1", get_state_as_dict(xScope)['SelectEntryText'])



# vim: set shiftwidth=4 softtabstop=4 expandtab:
