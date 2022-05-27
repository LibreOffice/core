# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
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
        with self.ui_test.create_doc_in_start_center("calc") as document:
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")

            enter_text_to_cell(gridwin, "A1", "A")
            gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:A1000"}))

            self.xUITest.executeCommand(".uno:FillDown")

            self.xUITest.executeCommand(".uno:SelectAll")

            with self.ui_test.execute_dialog_through_command(".uno:About", close_button="btnClose"):
                pass

            self.assertEqual(get_cell_by_position(document, 0, 0, 0).getString(), "A")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
