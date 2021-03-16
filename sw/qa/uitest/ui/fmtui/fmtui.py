# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

"""Covers sw/source/ui/fmtui/ fixes."""

from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict
from uitest.uihelper.common import select_pos

class TestTmpdlg(UITestCase):

    def test_para_style_semi_transparent_text(self):

        self.ui_test.create_doc_in_start_center("writer")
        xWriterDoc = self.xUITest.getTopFocusWindow()
        document = self.ui_test.get_component()
        self.ui_test.execute_dialog_through_command(".uno:EditStyle")
        xDialog = self.xUITest.getTopFocusWindow()

        xTabs = xDialog.getChild("tabcontrol")
        # Select RID_SVXPAGE_CHAR_EFFECTS.
        select_pos(xTabs, "1")
        xFontTransparency = xDialog.getChild("fonttransparencymtr")
        # Without the accompanying fix in place, this test would have failed with:
        # AssertionError: 'false' != 'true'
        # i.e. the control was hidden in the paragraph style dialog.
        self.assertEqual(get_state_as_dict(xFontTransparency)["Visible"], "true")

        xCancBtn = xDialog.getChild("cancel")
        self.ui_test.close_dialog_through_button(xCancBtn)
        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
