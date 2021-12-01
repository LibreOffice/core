# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues
#Bug 46138 - Calc Frozen after "undo" "Filling Series Number in a Column"

class tdf46138(UITestCase):
    def test_tdf46138(self):
        with self.ui_test.create_doc_in_start_center("calc") as document:
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")
            #Choose a column,
            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
            self.xUITest.executeCommand(".uno:SelectColumn")
            #select 'edit'>'fill'>'series'>Starting Value '1' then "OK",
            with self.ui_test.execute_dialog_through_command(".uno:FillSeries") as xDialog:
                xStart = xDialog.getChild("startValue")
                xStart.executeAction("TYPE", mkPropertyValues({"TEXT":"1"}))
            self.assertEqual(get_cell_by_position(document, 0, 0, 0).getString(), "1")
            #then "Ctrl+Z"/ undo
            self.xUITest.executeCommand(".uno:Undo")
            self.assertEqual(get_cell_by_position(document, 0, 0, 0).getString(), "")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
