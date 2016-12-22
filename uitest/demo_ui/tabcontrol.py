# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from libreoffice.uno.propertyvalue import mkPropertyValues

from uitest.uihelper.calc import enter_text_to_cell
from uitest.uihelper.common import select_pos

from uitest.framework import UITestCase

class TabControlTest(UITestCase):

    def test_select_pos(self):

        self.ui_test.create_doc_in_start_center("calc")

        xCalcDoc = self.xUITest.getTopFocusWindow()
        xGridWindow = xCalcDoc.getChild("grid_window")
        enter_text_to_cell(xGridWindow, "B2", "=2+3+4")
        xGridWindow.executeAction("SELECT", mkPropertyValues({"CELL": "B2"}))

        self.ui_test.execute_modeless_dialog_through_command(".uno:FunctionDialog")

        xFunctionDlg = self.xUITest.getTopFocusWindow()

        xTabs = xFunctionDlg.getChild("tabs")
        select_pos(xTabs, "1")

        xCancelBtn = xFunctionDlg.getChild("cancel")
        xCancelBtn.executeAction("CLICK", tuple())

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
