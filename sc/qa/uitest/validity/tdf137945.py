# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import select_pos
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file

class Tdf137945(UITestCase):

    def test_tdf137945(self):
        calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf137945.ods"))

        self.ui_test.execute_dialog_through_command(".uno:Validation")
        xDialog = self.xUITest.getTopFocusWindow()
        xTabs = xDialog.getChild("tabcontrol")
        select_pos(xTabs, "1")
        xInput = xDialog.getChild("inputhelp")
        xTitle = xDialog.getChild("title")

        self.assertEqual("test", get_state_as_dict(xTitle)['Text'])
        # Without the fix in place, this test would have failed with
        # '1 2  3   4    5    10         end' != '1 2   3     4       5       10                 end'
        self.assertEqual("1 2  3   4    5    10         end", get_state_as_dict(xInput)['Text'])

        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
