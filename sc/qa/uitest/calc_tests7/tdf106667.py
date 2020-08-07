# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.calc import enter_text_to_cell
from libreoffice.uno.propertyvalue import mkPropertyValues
from libreoffice.calc.document import get_cell_by_position

class tdf106667(UITestCase):

    def test_tdf106667_about_dlg_all(self):
        calc_doc = self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()

        enter_text_to_cell(gridwin, "A1", "A")
        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:A1000"}))

        self.xUITest.executeCommand(".uno:FillDown")

        self.xUITest.executeCommand(".uno:SelectAll")

        self.ui_test.execute_dialog_through_command(".uno:About")
        xAboutDlg = self.xUITest.getTopFocusWindow()
        xCloseBtn = xAboutDlg.getChild("btnClose")
        self.ui_test.close_dialog_through_button(xCloseBtn)

        self.assertEqual(get_cell_by_position(document, 0, 0, 0).getString(), "A")
        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
