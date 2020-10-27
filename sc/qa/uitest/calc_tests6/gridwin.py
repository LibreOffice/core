# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict

from uitest.framework import UITestCase

class GridWinTest(UITestCase):

    def test_select_sheet(self):

        self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        xGridWindow = xCalcDoc.getChild("grid_window")

        for i in range(3):
            self.ui_test.execute_dialog_through_command(".uno:Insert")
            current_dialog = self.xUITest.getTopFocusWindow()

            xOkButton = current_dialog.getChild("ok")
            self.ui_test.close_dialog_through_button(xOkButton)

        xGridWindow.executeAction("SELECT", mkPropertyValues({"TABLE": "2"}))
        self.assertEqual(get_state_as_dict(xGridWindow)["SelectedTable"], "2")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
