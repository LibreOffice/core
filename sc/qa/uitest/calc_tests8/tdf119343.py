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

from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues


# Bug 119343 - EDITING: copied and pasted cells containing formulas are not recalculated (2)
class tdf119343(UITestCase):
    def test_tdf119343_copy_paste_formula(self):
        #numberingformatpage.ui
        with self.ui_test.load_file(get_url_for_data_file("tdf119343.ods")) as calc_doc:
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")

            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "M295"}))
            self.xUITest.executeCommand(".uno:Copy")
            gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "DOWN"}))
            self.xUITest.executeCommand(".uno:Paste")
            #verify
            self.assertEqual(get_cell_by_position(calc_doc, 0, 12, 295).getString(), "Q11005355")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 12, 294).getString(), "Q1099565")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 12, 293).getString(), "Q108420")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
