# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_url_for_data_file
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues

class tdf37623(UITestCase):

    def test_tdf37623_autofill_rows_hidden(self):
        calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf37623.ods"))
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A2:A6"}))
        self.ui_test.execute_dialog_through_command(".uno:FillSeries")
        xDialog = self.xUITest.getTopFocusWindow()
        xautofill = xDialog.getChild("autofill")
        xautofill.executeAction("CLICK", tuple())
        xOK = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOK)

        self.assertEqual(get_cell_by_position(document, 0, 0, 1).getValue(), 1)
        self.assertEqual(get_cell_by_position(document, 0, 0, 2).getValue(), 0)
        self.assertEqual(get_cell_by_position(document, 0, 0, 3).getValue(), 0)
        self.assertEqual(get_cell_by_position(document, 0, 0, 4).getValue(), 2)
        self.assertEqual(get_cell_by_position(document, 0, 0, 5).getValue(), 3)
        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
