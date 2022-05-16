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


class tdf124295(UITestCase):

    def test_tdf124295(self):
        # Open spreadsheet and insert chart
        with self.ui_test.create_doc_in_start_center("calc"):
            with self.ui_test.execute_dialog_through_command(".uno:InsertObjectChart", close_button="cancel") as xChartDlg:

                # Click 3D look check button
                x3DCheckBtn = xChartDlg.getChild("3dlook")
                x3DCheckBtn.executeAction("CLICK", tuple())

                x3DSchemeLstBox = xChartDlg.getChild("3dscheme")
                # Without the fix in place, this test would have failed with:
                # AssertionError: 'Realistic' != ''
                self.assertEqual("Realistic", get_state_as_dict(x3DSchemeLstBox)["SelectEntryText"])



# vim: set shiftwidth=4 softtabstop=4 expandtab:
