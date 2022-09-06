# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, type_text
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues

class tdf142031(UITestCase):

    def test_tdf142031(self):
        with self.ui_test.create_doc_in_start_center("calc") as document:
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")
            xPosWindow = xCalcDoc.getChild('pos_window')

            with self.ui_test.execute_modeless_dialog_through_command(".uno:AddName", close_button="add") as xAddNameDlg:
                xEdit = xAddNameDlg.getChild("edit")
                type_text(xEdit, "crédit")

            self.assertEqual('crédit', get_state_as_dict(xPosWindow)['Text'])

            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "B1"}))

            xInputWin = xCalcDoc.getChild("sc_input_window")
            xInputWin.executeAction("TYPE", mkPropertyValues({"TEXT":"=cré"}))
            xInputWin.executeAction("TYPE", mkPropertyValues({"KEYCODE":"RETURN"}))

            # Without the fix in place, this test would have failed here
            self.assertEqual("=crédit", get_state_as_dict(xInputWin)["Text"])

            xInputWin.executeAction("TYPE", mkPropertyValues({"KEYCODE":"RETURN"}))

            self.assertEqual("0", get_cell_by_position(document, 0, 1, 0).getString())

# vim: set shiftwidth=4 softtabstop=4 expandtab:
