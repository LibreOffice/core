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
from uitest.uihelper.common import select_pos
from uitest.uihelper.calc import enter_text_to_cell
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues

#Bug 91217 - Crash when deleting rows

class tdf91217(UITestCase):
    def test_tdf91217_crash_deleting_rows(self):
        # FIXME disable this will it's clear what existing problem did this test uncover.
        return
        with self.ui_test.load_file(get_url_for_data_file("tdf91217.ods")) as calc_doc:
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")
            #Select rows 76-1125 in sheet 3 (Daily), right click, Delete Rows.  Instant crash.
            gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A76:A1125"}))
            self.xUITest.executeCommand(".uno:SelectRow")
            #delete rows
            self.xUITest.executeCommand(".uno:DeleteRows")
            #A1 should be "Dist"
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 0).getString(), "Dist")
            #- Undo-redo
            self.xUITest.executeCommand(".uno:Undo")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 0).getString(), "Dist")

            self.xUITest.executeCommand(".uno:Redo")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 0).getString(), "Dist")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
