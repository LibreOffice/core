# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict
from uitest.uihelper.calc import enter_text_to_cell
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues
#Bug 51700 - Text to columns puts result into first line if whole column is selected

class tdf51700(UITestCase):
    def test_tdf51700_text_to_columns(self):
        with self.ui_test.create_doc_in_start_center("calc") as document:
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")

            #Add data
            enter_text_to_cell(gridwin, "A2", "3242,43242,3242,2342")
            enter_text_to_cell(gridwin, "A3", "fdsfa,afsdfa,adfdas,fsad")
            enter_text_to_cell(gridwin, "A4", "21312,1111,1111,111")
            #select column A
            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
            self.xUITest.executeCommand(".uno:SelectColumn")
            # Data - Text to Columns
            with self.ui_test.execute_dialog_through_command(".uno:TextToColumns") as xDialog:
                xcomma = xDialog.getChild("comma")
                if (get_state_as_dict(xcomma)["Selected"]) == "false":
                    xcomma.executeAction("CLICK", tuple())
                # Click Ok

            #Verify
            self.assertEqual(get_cell_by_position(document, 0, 0, 1).getValue(), 3242)
            self.assertEqual(get_cell_by_position(document, 0, 0, 2).getString(), "fdsfa")
            self.assertEqual(get_cell_by_position(document, 0, 0, 3).getValue(), 21312)
            self.assertEqual(get_cell_by_position(document, 0, 1, 1).getValue(), 43242)
            self.assertEqual(get_cell_by_position(document, 0, 1, 2).getString(), "afsdfa")
            self.assertEqual(get_cell_by_position(document, 0, 1, 3).getValue(), 1111)
            self.assertEqual(get_cell_by_position(document, 0, 2, 1).getValue(), 3242)
            self.assertEqual(get_cell_by_position(document, 0, 2, 2).getString(), "adfdas")
            self.assertEqual(get_cell_by_position(document, 0, 2, 3).getValue(), 1111)
            self.assertEqual(get_cell_by_position(document, 0, 3, 1).getValue(), 2342)
            self.assertEqual(get_cell_by_position(document, 0, 3, 2).getString(), "fsad")
            self.assertEqual(get_cell_by_position(document, 0, 3, 3).getValue(), 111)


# vim: set shiftwidth=4 softtabstop=4 expandtab:
