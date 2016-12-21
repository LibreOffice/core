# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase

from libreoffice.uno.propertyvalue import mkPropertyValues

from uitest.uihelper.common import get_state_as_dict

import time

class ManualCalcTests(UITestCase):

    # http://manual-test.libreoffice.org/manage/case/189/
    def test_define_database_range(self):

        self.ui_test.create_doc_in_start_center("calc")

        # Select range A1:D10
        xGridWin = self.xUITest.getTopFocusWindow().getChild("grid_window")
        xGridWin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:D10"}))

        # Execute "Define DB Range dialog"
        self.ui_test.execute_modeless_dialog_through_command(".uno:DefineDBName")

        xDefineNameDlg = self.xUITest.getTopFocusWindow()

        xEntryBox = xDefineNameDlg.getChild("entry")
        xEntryBox.executeAction("TYPE", mkPropertyValues({"TEXT": "my_database"}))

        xOkBtn = xDefineNameDlg.getChild("ok")
        self.ui_test.close_dialog_through_button(xOkBtn)

        # Deselect range
        xGridWin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))

        # Execute "Select DB Range dialog"
        self.ui_test.execute_dialog_through_command(".uno:SelectDB")
        xSelectNameDlg = self.xUITest.getTopFocusWindow()

        xListBox = xSelectNameDlg.getChild("treeview")
        xListBoxState = get_state_as_dict(xListBox)
        self.assertEqual(xListBoxState["SelectEntryCount"], "1")
        self.assertEqual(xListBoxState["SelectEntryText"], "my_database")

        xOkBtn = xSelectNameDlg.getChild("ok")
        self.ui_test.close_dialog_through_button(xOkBtn)

        # Assert that the correct range has been selected
        gridWinState = get_state_as_dict(xGridWin)
        self.assertEqual(gridWinState["MarkedArea"], "Sheet1.A1:Sheet1.D10")

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
