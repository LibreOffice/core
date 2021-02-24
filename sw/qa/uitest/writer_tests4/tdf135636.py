# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file

class tdf135636(UITestCase):

    def test_tdf135636(self):

        self.ui_test.load_file(get_url_for_data_file("tdf135636.odt"))

        document = self.ui_test.get_component()

        self.assertEqual(document.CurrentController.PageCount, 2)

        xPageBreak = self.ui_test.wait_until_child_is_available('PageBreak')
        self.ui_test.execute_dialog_through_action(xPageBreak, "EDIT")

        xDialog = self.xUITest.getTopFocusWindow()

        xBreak = xDialog.getChild("break")
        self.assertEqual("true", get_state_as_dict(xBreak)["Selected"])

        xBreak.executeAction("CLICK", tuple())

        self.assertEqual("false", get_state_as_dict(xBreak)["Selected"])

        okBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(okBtn)

        # Without the fix in place, this test would have failed with
        # AssertionError: 2 != 1
        self.assertEqual(document.CurrentController.PageCount, 1)

        self.xUITest.executeCommand(".uno:Undo")

        self.assertEqual(document.CurrentController.PageCount, 2)

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
