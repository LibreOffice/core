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


class PageTabTest(UITestCase):
    # Tests that the “Resize all pages” checkbox is only visible when the dialog is opened from a
    # Draw or Impress document and not from Writer or Calc. See tdf#173170
    def test_resize_all_visibility(self):
        with self.ui_test.create_doc_in_start_center("impress") as xComponent:
            xTemplate = self.xUITest.getTopFocusWindow()
            self.ui_test.close_dialog_through_button(xTemplate.getChild("close"))

            with self.ui_test.execute_dialog_through_command(".uno:PageSetup") as xDialog:
                xResizeCheckBox = xDialog.getChild("checkResizeAllPages")
                self.assertEqual(get_state_as_dict(xResizeCheckBox)["ReallyVisible"], "true")

        with self.ui_test.create_doc_in_start_center("draw") as xComponent:
            with self.ui_test.execute_dialog_through_command(".uno:PageSetup") as xDialog:
                xResizeCheckBox = xDialog.getChild("checkResizeAllPages")
                self.assertEqual(get_state_as_dict(xResizeCheckBox)["ReallyVisible"], "true")

        with self.ui_test.create_doc_in_start_center("writer") as xComponent:
            with self.ui_test.execute_dialog_through_command(".uno:PageDialog") as xDialog:
                xResizeCheckBox = xDialog.getChild("checkResizeAllPages")
                self.assertEqual(get_state_as_dict(xResizeCheckBox)["ReallyVisible"], "false")

        with self.ui_test.create_doc_in_start_center("calc") as xComponent:
            with self.ui_test.execute_dialog_through_command(".uno:PageFormatDialog") as xDialog:
                xResizeCheckBox = xDialog.getChild("checkResizeAllPages")
                self.assertEqual(get_state_as_dict(xResizeCheckBox)["ReallyVisible"], "false")


# vim: set shiftwidth=4 softtabstop=4 expandtab:
