# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_url_for_data_file, select_pos

from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues


# Bug 99627 - Calc freezes when applying natural sorting on text columns
class tdf99627(UITestCase):
    def test_td99627_natural_sort(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf99627.ods")) as calc_doc:
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")

            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))

            #Open sort dialog by DATA - SORT
            with self.ui_test.execute_dialog_through_command(".uno:DataSort") as xDialog:
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "1")
                xNatural = xDialog.getChild("naturalsort")
                xdown = xDialog.getChild("down")
                xNatural.executeAction("CLICK", tuple())
                select_pos(xTabs, "0")
                xdown.executeAction("CLICK", tuple())
            #Verify
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 1).getString(), "2998")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 2998).getString(), "1")
            #UNDO
            self.xUITest.executeCommand(".uno:Undo")
            #Verify
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 1).getString(), "1")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 2998).getString(), "2998")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
