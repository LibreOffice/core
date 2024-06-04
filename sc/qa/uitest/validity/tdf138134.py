# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.calc import enter_text_to_cell
from uitest.uihelper.common import select_by_text, select_pos

from libreoffice.uno.propertyvalue import mkPropertyValues


class DetectiveCircle(UITestCase):

    def test_delete_circle_at_formula(self):
        with self.ui_test.create_doc_in_start_center("calc") as document:
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")
            enter_text_to_cell(gridwin, "A1", "1")
            enter_text_to_cell(gridwin, "A2", "3")
            enter_text_to_cell(gridwin, "A3", "=SUM(A1:A2)")

            #Select the cells to be validated
            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A3"}))
            #Apply Data > Validity ... > Integer
            with self.ui_test.execute_dialog_through_command(".uno:Validation") as xDialog:
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "0")
                xallow = xDialog.getChild("allow")
                xallowempty = xDialog.getChild("allowempty")
                xdata = xDialog.getChild("data")
                xmin = xDialog.getChild("min")

                select_by_text(xallow, "Integer")
                xallowempty.executeAction("CLICK", tuple())
                select_by_text(xdata, "equal")
                xmin.executeAction("TYPE", mkPropertyValues({"TEXT":"5"}))

            self.xUITest.executeCommand(".uno:ShowInvalid")

            detectiveCircle1 = len(document.Sheets.getByName("Sheet1").DrawPage)
            #There should be 1 detective circle object!
            self.assertEqual(detectiveCircle1, 1)

            enter_text_to_cell(gridwin, "A1", "2")

            detectiveCircle2 = len(document.Sheets.getByName("Sheet1").DrawPage)
            #There should not be a detective circle object!
            self.assertEqual(detectiveCircle2, 0)


# vim: set shiftwidth=4 softtabstop=4 expandtab:
