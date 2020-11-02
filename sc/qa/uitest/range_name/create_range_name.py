# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
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

    def insert_nameRange(self, gridwin, cellPos):
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": cellPos}))

        self.ui_test.execute_dialog_through_command(".uno:InsertName")
        xDialog = self.xUITest.getTopFocusWindow()

        xCtrl = xDialog.getChild('ctrl')
        self.assertEqual(1, len(xCtrl.getChildren()))
        self.assertEqual("localRangeName\t$Sheet1.$A$1\tSheet1", get_state_as_dict(xCtrl.getChild('0'))['Text'])
        xCtrl.getChild('0').executeAction("SELECT", tuple())

        xPasteBtn = xDialog.getChild("paste")
        self.ui_test.close_dialog_through_button(xPasteBtn)

        # use return key to paste the name range
        gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "RETURN"}))

    def test_create_range_name(self):

        self.ui_test.create_doc_in_start_center("calc")

        calcDoc = self.xUITest.getTopFocusWindow()
        xPosWindow = calcDoc.getChild('pos_window')
        self.assertEqual('A1', get_state_as_dict(xPosWindow)['Text'])

        self.ui_test.execute_modeless_dialog_through_command(".uno:AddName")

        xAddNameDlg = self.xUITest.getTopFocusWindow()

        xEdit = xAddNameDlg.getChild("edit")
        type_text(xEdit, "globalRangeName")

        xAddBtn = xAddNameDlg.getChild("add")
        self.ui_test.close_dialog_through_button(xAddBtn)

        self.assertEqual('globalRangeName', get_state_as_dict(xPosWindow)['Text'])

        self.ui_test.close_doc()

    def test_create_local_range_name(self):

        self.ui_test.create_doc_in_start_center("calc")

        calcDoc = self.xUITest.getTopFocusWindow()
        xPosWindow = calcDoc.getChild('pos_window')
        self.assertEqual('A1', get_state_as_dict(xPosWindow)['Text'])

        self.ui_test.execute_modeless_dialog_through_command(".uno:AddName")

        xAddNameDlg = self.xUITest.getTopFocusWindow()

        xEdit = xAddNameDlg.getChild("edit")
        type_text(xEdit, "localRangeName")

        xScope = xAddNameDlg.getChild("scope")
        select_pos(xScope, "1")

        xAddBtn = xAddNameDlg.getChild("add")
        self.ui_test.close_dialog_through_button(xAddBtn)

        # tdf#67007: Without the fix in place, this test would have failed with
        # AssertionError: 'localRangeName' != 'A1'
        self.assertEqual('localRangeName', get_state_as_dict(xPosWindow)['Text'])

        gridwin = calcDoc.getChild("grid_window")
        enter_text_to_cell(gridwin, "A1", "1")

        # Insert a name range in the current sheet
        self.insert_nameRange(gridwin, 'B1')

        document = self.ui_test.get_component()
        self.assertEqual("1", get_cell_by_position(document, 0, 1, 0).getString())
        self.assertEqual("=localRangeName", get_cell_by_position(document, 0, 1, 0).getFormula())

        self.ui_test.execute_dialog_through_command(".uno:Insert")  #insert sheet
        xDialog = self.xUITest.getTopFocusWindow()
        xOKButton = xDialog.getChild("ok")
        xOKButton.executeAction("CLICK", tuple())

        # Insert a name range in the new sheet
        self.insert_nameRange(gridwin, 'B1')

        # tdf#137896: Without the fix in place, this test would have failed with
        # AssertionError: '1' != '#NAME?'
        self.assertEqual("1", get_cell_by_position(document, 0, 1, 0).getString())

        # and AssertionError: '=Sheet1.localRangeName' != '=localrangename'
        self.assertEqual("=Sheet1.localRangeName", get_cell_by_position(document, 0, 1, 0).getFormula())

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
