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
#Bug 73006 - EDITING: Results filled in wrong cells after Text to Columns

class tdf73006(UITestCase):
    def test_tdf73006_text_to_columns(self):
        with self.ui_test.create_doc_in_start_center("calc") as document:
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")

            #Add data
            enter_text_to_cell(gridwin, "A2", "A B")
            #select column A
            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
            self.xUITest.executeCommand(".uno:SelectColumn")
            # Data - Text to Columns
            with self.ui_test.execute_dialog_through_command(".uno:TextToColumns") as xDialog:
                xspace = xDialog.getChild("space")
                if (get_state_as_dict(xspace)["Selected"]) == "false":
                    xspace.executeAction("CLICK", tuple())
                # Click Ok

            #Verify
            self.assertEqual(get_cell_by_position(document, 0, 0, 1).getString(), "A")
            self.assertEqual(get_cell_by_position(document, 0, 1, 1).getString(), "B")


# vim: set shiftwidth=4 softtabstop=4 expandtab:
