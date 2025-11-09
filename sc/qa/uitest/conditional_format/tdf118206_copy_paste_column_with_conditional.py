# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_url_for_data_file

from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues


# Bug 118206 - [GTK3] Calc hangs copying/cutting a conditional format column
class tdf118206(UITestCase):
    def test_tdf118206(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf118206.xlsx")) as calc_doc:
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")
            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
            self.xUITest.executeCommand(".uno:SelectColumn")
            self.xUITest.executeCommand(".uno:Copy")
            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "B1"}))
            self.xUITest.executeCommand(".uno:SelectColumn")
            self.xUITest.executeCommand(".uno:Paste")

            #verify
            self.assertEqual(get_cell_by_position(calc_doc, 0, 1, 0).getString(), "On Back Order")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 1, 1).getValue(), 0)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 1, 7).getValue(), 1)

            #Undo
            self.xUITest.executeCommand(".uno:Undo")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 0).getString(), "On Back Order")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 1).getValue(), 0)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 7).getValue(), 1)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 1, 0).getString(), "")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 1, 1).getString(), "")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 1, 7).getString(), "")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
