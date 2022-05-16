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


# Bug 95192 - SORTING Natural sorting not working with non-letter,non-number content
class tdf95192(UITestCase):
    def test_td99627_natural_sort(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf95192.ods")) as calc_doc:
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")

            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))

            #Open sort dialog by DATA - SORT
            with self.ui_test.execute_dialog_through_command(".uno:DataSort") as xDialog:
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "1")
                xNatural = xDialog.getChild("naturalsort")
                xNatural.executeAction("CLICK", tuple())
            #Verify
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 0).getString(), "Sal. Capra 1/17")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 1).getString(), "Sal. Capra 1/20")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 2).getString(), "Sal. Oregina 1/2")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 41).getString(), "Vico Chiuso Cinque Santi 18/10")
            #UNDO
            self.xUITest.executeCommand(".uno:Undo")
            #Verify
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 0).getString(), "Sal. Oregina 1/2")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 1).getString(), "Via A. Centurione 11/7")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 41).getString(), "Vico Chiuso Cinque Santi 18/10")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
