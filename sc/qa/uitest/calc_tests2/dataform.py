# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues

class dataform(UITestCase):
    def test_dataform(self):
        calc_doc = self.ui_test.create_doc_in_start_center("calc")

        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:A10"}))

        self.ui_test.execute_dialog_through_command(".uno:DataForm")
        xDialog = self.xUITest.getTopFocusWindow()

        xCloseBtn = xDialog.getChild("close")
        self.ui_test.close_dialog_through_button(xCloseBtn)
        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
