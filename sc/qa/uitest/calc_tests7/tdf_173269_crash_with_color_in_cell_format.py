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

class tdf173269(UITestCase):
    def test_tdf173269_crash_with_color_in_cell_format(self):

        with self.ui_test.create_doc_in_start_center("calc") as calc_doc:
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")

            enter_text_to_cell(gridwin, "A1", "-400.02")

            with self.ui_test.execute_dialog_through_command(".uno:FormatCellDialog") as xDialog:
                xformatted = xDialog.getChild("formatted")

                xformatted.executeAction("CLEAR", tuple())

                # Without the fix in place, this test would have crashed here
                xformatted.executeAction("TYPE", mkPropertyValues({"TEXT":"#.##0,00 €;[COLOR 10]-#.##0,00 €"}))

            self.assertEqual("-400.02000 €", get_cell_by_position(calc_doc, 0, 0, 0).getString())

# vim: set shiftwidth=4 softtabstop=4 expandtab:
