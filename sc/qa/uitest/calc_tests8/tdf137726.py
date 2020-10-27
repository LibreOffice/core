# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase

class tdf137726(UITestCase):

    def test_tdf137726(self):
        calc_doc = self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")

        self.ui_test.execute_dialog_through_command(".uno:DataDataPilotRun")

        # three dialogs are displayed one after the other, click OK in all of them
        for i in range(3):
            xDialog = self.xUITest.getTopFocusWindow()
            xOKBtn = xDialog.getChild('ok')
            self.ui_test.close_dialog_through_button(xOKBtn)

        # Without the fix in place, this test would have hung here

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
