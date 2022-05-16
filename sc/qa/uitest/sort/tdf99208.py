# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file
from uitest.uihelper.common import select_by_text, select_pos

from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues


# Bug 99208 - Spreadsheet sort hangs
class tdf99208(UITestCase):
    def test_td99627_natural_sort(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf99208.ods")) as calc_doc:
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")
            #select A3:C245
            gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:C245"}))

            #Menu 'Data -> Sort
            #Column A - Ascending' <ok>
            with self.ui_test.execute_dialog_through_command(".uno:DataSort") as xDialog:
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "1")
                xNatural = xDialog.getChild("naturalsort")
                xFormats = xDialog.getChild("formats")
                if (get_state_as_dict(xNatural)["Selected"]) == "false":
                    xNatural.executeAction("CLICK", tuple())
                if (get_state_as_dict(xFormats)["Selected"]) == "false":
                    xFormats.executeAction("CLICK", tuple())
                select_pos(xTabs, "0")
                xtopdown = xDialog.getChild("rbTopDown")
                xHeader = xDialog.getChild("cbHeader")
                if (get_state_as_dict(xHeader)["Selected"]) == "false":
                    xHeader.executeAction("CLICK", tuple())
                xtopdown.executeAction("CLICK", tuple())

                xSortKey1 = xDialog.getChild("sortlb")
                xAsc = xDialog.getChild("up")
                select_by_text(xSortKey1, "FODMAP")
                xAsc.executeAction("CLICK", tuple())
            #Verify Expected: Values column B sorted ascending, column "control" unsorted
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 0).getString(), "FODMAP")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 1).getString(), "agave")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 2).getString(), "almond milk")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 244).getString(), "zucchini")
            # UNDO
            self.xUITest.executeCommand(".uno:Undo")
            # Verify
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 0).getString(), "FODMAP")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 1).getString(), "beef (grass fed, no breadcrumbs)")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 244).getString(), "salsa")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
