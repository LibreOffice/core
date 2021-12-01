# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
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

        with self.ui_test.create_doc_in_start_center("writer"):
            xWriterDoc = self.xUITest.getTopFocusWindow()
            with self.ui_test.execute_dialog_through_command(".uno:EditStyle", close_button="cancel") as xDialog:

                xTabs = xDialog.getChild("tabcontrol")
                # Select RID_SVXPAGE_CHAR_EFFECTS.
                select_pos(xTabs, "1")
                xFontTransparency = xDialog.getChild("fonttransparencymtr")
                # Without the accompanying fix in place, this test would have failed with:
                # AssertionError: 'false' != 'true'
                # i.e. the control was hidden in the paragraph style dialog.
                self.assertEqual(get_state_as_dict(xFontTransparency)["Visible"], "true")


# vim: set shiftwidth=4 softtabstop=4 expandtab:
