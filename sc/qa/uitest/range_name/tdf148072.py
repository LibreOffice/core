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
from uitest.uihelper.common import get_state_as_dict, type_text, select_by_text
from libreoffice.calc.document import get_cell_by_position
from uitest.uihelper.calc import enter_text_to_cell

class Tdf148072(UITestCase):

    def test_tdf148072(self):
        with self.ui_test.create_doc_in_start_center("calc") as calc_doc:

            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")
            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "B2"}))

            with self.ui_test.execute_modeless_dialog_through_command(".uno:AddName", close_button="add") as xAddNameDlg:
                xEdit = xAddNameDlg.getChild("edit")
                type_text(xEdit, "testfield_1")

                xRange = xAddNameDlg.getChild("range")
                self.assertEqual("$Sheet1.$B$2", get_state_as_dict(xRange)['Text'])

                xScope = xAddNameDlg.getChild("scope")
                select_by_text(xScope, "Sheet1")

            enter_text_to_cell(gridwin, "B2", "1")
            enter_text_to_cell(gridwin, "B3", "=testfield_1")

            self.assertEqual("1", get_cell_by_position(calc_doc, 0, 1, 1).getString())
            self.assertEqual("1", get_cell_by_position(calc_doc, 0, 1, 2).getString())

            gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "B2:B3"}))

            self.xUITest.executeCommand(".uno:Cut")

            # Insert a new sheet
            with self.ui_test.execute_dialog_through_command(".uno:Add") as xDialog:
                xName = xDialog.getChild('name_entry')
                self.assertEqual( 'Sheet2', get_state_as_dict(xName)["Text"])

            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "$Sheet2.B2"}))

            self.xUITest.executeCommand(".uno:Paste")

            self.assertEqual("", get_cell_by_position(calc_doc, 0, 1, 1).getString())
            self.assertEqual("", get_cell_by_position(calc_doc, 0, 1, 2).getString())
            self.assertEqual("1", get_cell_by_position(calc_doc, 1, 1, 1).getString())
            self.assertEqual("1", get_cell_by_position(calc_doc, 1, 1, 2).getString())

            with self.ui_test.execute_dialog_through_command(".uno:DefineName") as xDialog:
                xNamesList = xDialog.getChild('names')
                self.assertEqual(1, len(xNamesList.getChildren()))

                xName = xDialog.getChild('name')
                self.assertEqual( 'testfield_1', get_state_as_dict(xName)["Text"])

                xRange = xDialog.getChild("range")
                self.assertEqual("$Sheet2.$B$2", get_state_as_dict(xRange)['Text'])

                xScope = xDialog.getChild("scope")
                self.assertEqual("$Sheet2.$B$2", get_state_as_dict(xRange)['Text'])

            self.xUITest.executeCommand(".uno:Undo")
            self.xUITest.executeCommand(".uno:Undo")
            self.xUITest.executeCommand(".uno:Undo")

            self.assertEqual("1", get_cell_by_position(calc_doc, 0, 1, 1).getString())

            # Without the fix in place, this test would have failed with
            # AssertionError: '1' != '#REF!'
            self.assertEqual("1", get_cell_by_position(calc_doc, 0, 1, 2).getString())

            with self.ui_test.execute_dialog_through_command(".uno:DefineName") as xDialog:
                xNamesList = xDialog.getChild('names')
                self.assertEqual(1, len(xNamesList.getChildren()))

                xName = xDialog.getChild('name')
                self.assertEqual( 'testfield_1', get_state_as_dict(xName)["Text"])

                xRange = xDialog.getChild("range")
                self.assertEqual("$Sheet1.$B$2", get_state_as_dict(xRange)['Text'])

                xScope = xDialog.getChild("scope")
                self.assertEqual("$Sheet1.$B$2", get_state_as_dict(xRange)['Text'])

# vim: set shiftwidth=4 softtabstop=4 expandtab:
