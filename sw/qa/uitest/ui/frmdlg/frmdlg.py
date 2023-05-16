# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

"""Covers sw/source/ui/frmdlg/ fixes."""

import time
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict
from uitest.uihelper.common import get_url_for_data_file


class Test(UITestCase):
    def test_content_control_dialog(self):
        with self.ui_test.create_doc_in_start_center("writer") as xComponent:
            # Given a document with a floating table:
            args = {
                "Columns": 1,
                "Rows": 1,
            }
            self.xUITest.executeCommandWithParameters(".uno:InsertTable", mkPropertyValues(args))
            self.xUITest.executeCommand(".uno:SelectAll")
            args = {
                "AnchorType": 0,
            }
            self.xUITest.executeCommandWithParameters(".uno:InsertFrame", mkPropertyValues(args))
            # Wait until SwTextShell is replaced with SwDrawShell after 120 ms, as set in the SwView
            # ctor.
            time.sleep(0.2)
            self.assertEqual(xComponent.TextFrames.Frame1.IsSplitAllowed, False)

            # When allowing it to split on the UI:
            with self.ui_test.execute_dialog_through_command(".uno:FrameDialog") as xDialog:
                xFlysplit = xDialog.getChild("flysplit")
                self.assertEqual(get_state_as_dict(xFlysplit)['Visible'], "true")
                self.assertEqual(get_state_as_dict(xFlysplit)['Selected'], "false")
                xFlysplit.executeAction("CLICK", tuple())

            # Then make sure the doc model is updated correctly:
            self.assertEqual(xComponent.TextFrames.Frame1.IsSplitAllowed, True)

    def test_chained_fly_split(self):
        # Given a document with 2 chained fly frames:
        with self.ui_test.load_file(get_url_for_data_file("chained-frames.odt")):
            # When selecting the first and opening the fly frame properties dialog:
            self.xUITest.executeCommand(".uno:JumpToNextFrame")
            # Wait until SwTextShell is replaced with SwDrawShell after 120 ms, as set in the SwView
            # ctor.
            time.sleep(0.2)
            with self.ui_test.execute_dialog_through_command(".uno:FrameDialog") as xDialog:
                # Then make sure that the 'split' checkbox is hidden:
                xFlysplit = xDialog.getChild("flysplit")
                # Without the accompanying fix in place, this test would have failed with:
                # AssertionError: 'true' != 'false'
                # i.e. the UI didn't hide this option, leading to some weird mix of chained shapes
                # and floating tables.
                self.assertEqual(get_state_as_dict(xFlysplit)['Visible'], "false")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
