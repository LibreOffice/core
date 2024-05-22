# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict
from uitest.uihelper.common import select_by_text, select_pos
from libreoffice.calc.document import get_cell_by_position

from libreoffice.uno.propertyvalue import mkPropertyValues

class tdf115474(UITestCase):
    def test_tdf115474_Warning_Dialog(self):
        with self.ui_test.create_doc_in_start_center("calc") as document:
            gridwin = self.xUITest.getTopFocusWindow().getChild("grid_window")
            gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:C1000000"}))
            with self.ui_test.execute_dialog_through_command(".uno:AutoFormat", close_button="") as xDialog:
                formatlb = xDialog.getChild("formatlb")
                entry = formatlb.getChild("2") # pick Box list blue
                entry.executeAction("SELECT", tuple())

                xOKBtn = xDialog.getChild("ok")
                with self.ui_test.execute_blocking_action(xOKBtn.executeAction, args=('CLICK', ()), close_button="no"):
                    # no background color set, we've skipped autoformat!
                    self.assertEqual(get_cell_by_position(document, 0, 0, 0).CellBackColor, -1)
