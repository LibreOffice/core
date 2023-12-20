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
from libreoffice.calc.document import get_cell_by_position
from uitest.uihelper.calc import enter_text_to_cell
from uitest.uihelper.common import get_state_as_dict, select_pos

class tdf148437(UITestCase):
    def test_tdf148437_formula_cell(self):
        with self.ui_test.create_doc_in_start_center("calc") as document:
            xCalcDoc = self.xUITest.getTopFocusWindow()
            xGridWindow = xCalcDoc.getChild("grid_window")

            # Select a cell including a formula and insert a hyperlink
            enter_text_to_cell(xGridWindow, "A1", "=HYPERLINK(\"www.libreoffice.org\";\"LibreOffice\")")
            # Move focus to ensure cell is not in edit mode
            xGridWindow.executeAction("SELECT", mkPropertyValues({"CELL": "A2"}))
            xGridWindow.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
            with self.ui_test.execute_dialog_through_command(".uno:HyperlinkDialog") as xDialog:
                xTab = xDialog.getChild("tabcontrol")
                select_pos(xTab, "0")

                # Text should contain the text of the cell
                xTarget = xDialog.getChild("target")
                self.assertEqual(get_state_as_dict(xTarget)["Text"], "")
                xIndication = xDialog.getChild("indication")
                self.assertEqual(get_state_as_dict(xIndication)["Text"], "LibreOffice")

                # Insert a sample hyperlink
                xTarget.executeAction("TYPE", mkPropertyValues({"TEXT": "https://www.documentfoundation.org/"}))
                xIndication.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
                xIndication.executeAction("TYPE", mkPropertyValues({"TEXT": "LibreOffice Document Foundation"}))

            # Move focus to ensure cell is not in edit mode
            xGridWindow.executeAction("SELECT", mkPropertyValues({"CELL": "A2"}))
            xGridWindow.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))

            # Check contents of the cell
            xCell = get_cell_by_position(document, 0, 0, 0)
            self.assertEqual(xCell.getString(), "LibreOffice Document Foundation")
            xTextFields = xCell.getTextFields()
            self.assertEqual(len(xTextFields), 1)
            self.assertEqual(xTextFields[0].URL, "https://www.documentfoundation.org/")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
