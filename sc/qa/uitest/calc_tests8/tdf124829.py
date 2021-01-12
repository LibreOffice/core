# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file
from uitest.uihelper.common import select_pos
from uitest.uihelper.calc import enter_text_to_cell
from libreoffice.calc.document import get_sheet_from_doc
from libreoffice.calc.conditional_format import get_conditional_format_from_sheet
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues

#Bug 124829 - CRASH: cutting and undoing a sheet with external links

class tdf124829(UITestCase):
    def test_tdf124829_Crash_cutting_undo_sheet_external_links(self):
        #numberingformatpage.ui
        calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf124829.ods"))
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()

        self.xUITest.executeCommand(".uno:SelectAll")
        self.xUITest.executeCommand(".uno:Cut")
        for i in range(40):
            self.xUITest.executeCommand(".uno:Undo")

        #verify; no crashes
        self.assertEqual(document.Sheets.getCount(), 6)

        self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab:
