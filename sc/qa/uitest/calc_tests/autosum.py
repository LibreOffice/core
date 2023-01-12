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

#AutoSum feature test

class calcAutosum(UITestCase):

    def test_autosum_test8(self):
        #8.Autosum on rows without selected empty cell for result
        with self.ui_test.load_file(get_url_for_data_file("autosum.ods")) as calc_doc:
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")

            gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "B126:D126"}))
            gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "B128:D128", "EXTEND":"1"}))
            gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "B130:D130", "EXTEND":"1"}))
            self.xUITest.executeCommand(".uno:AutoSum")

            self.assertEqual(get_cell_by_position(calc_doc, 0, 4, 125).getValue(), 30)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 4, 125).getFormula(), "=SUM(B126:D126)")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 4, 127).getValue(), 90)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 4, 127).getFormula(), "=SUM(B128:D128)")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 4, 129).getValue(), 150)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 4, 129).getFormula(), "=SUM(B130:D130)")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
