# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict

import time
import unittest

class tdf124295(UITestCase):

    def test_tdf124295(self):
        # Open spreadsheet and insert chart
        self.ui_test.create_doc_in_start_center("calc")
        self.ui_test.execute_dialog_through_command(".uno:InsertObjectChart")
        xChartDlg = self.xUITest.getTopFocusWindow()

        # Click 3D look check button
        x3DCheckBtn = xChartDlg.getChild("3dlook")
        x3DCheckBtn.executeAction("CLICK", tuple())

        x3DSchemeLstBox = xChartDlg.getChild("3dscheme")
        # Without the fix in place, this test would have failed with:
        # AssertionError: 'Realistic' != ''
        self.assertEqual("Realistic", get_state_as_dict(x3DSchemeLstBox)["SelectEntryText"])

        xCancelBtn = xChartDlg.getChild("cancel")
        self.ui_test.close_dialog_through_button(xCancelBtn)

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
