#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict
from uitest.uihelper.common import select_pos

class tdf131715(UITestCase):

    def test_tdf131715(self):
        self.ui_test.create_doc_in_start_center("calc")
        self.ui_test.execute_dialog_through_command(".uno:InsertObjectChart")
        xChartDlg = self.xUITest.getTopFocusWindow()

        xWizard = xChartDlg.getChild('Wizard')

        for i in range(10):
            #without the fix in place, it would crash here
            select_pos(xWizard, "2")

        self.assertEqual(get_state_as_dict(xWizard)['CurrentStep'], "2")

        xOkBtn = xChartDlg.getChild("finish")
        self.ui_test.close_dialog_through_button(xOkBtn)

        self.ui_test.close_doc()
