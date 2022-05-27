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
from uitest.uihelper.common import get_url_for_data_file

class Tdf1254400(UITestCase):

   def test_tdf125440(self):
        # In order to simulate copy&paste from external source,
        # import the text in Writer, copy it and paste it in Calc
        with self.ui_test.load_file(get_url_for_data_file("tdf125440.odt")):

            self.xUITest.executeCommand(".uno:SelectAll")
            self.xUITest.executeCommand(".uno:Copy")

        with self.ui_test.load_empty_file("calc") as calc_document:

            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")

            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))

            self.xUITest.executeCommand(".uno:PasteUnformatted")

            # Without the fix in place, this test would have failed with
            # AssertionError: 'Row' != '"Row"'
            self.assertEqual("Row", get_cell_by_position(calc_document, 0, 0, 0).getString())
            self.assertEqual("", get_cell_by_position(calc_document, 0, 1, 0).getString())
            self.assertEqual("", get_cell_by_position(calc_document, 0, 2, 0).getString())
            self.assertEqual("50", get_cell_by_position(calc_document, 0, 3, 0).getString())

# vim: set shiftwidth=4 softtabstop=4 expandtab:
