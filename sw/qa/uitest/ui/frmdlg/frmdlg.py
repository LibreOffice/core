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
    def test_uno_frame_dialog(self):
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

    def test_insert_frame_dialog(self):
        # Change from inch to cm to hit the rounding error. 2 means Centimeter, see
        # officecfg/registry/schema/org/openoffice/Office/Writer.xcs.
        with self.ui_test.set_config('/org.openoffice.Office.Writer/Layout/Other/MeasureUnit', 2):
            # Given a Writer document:
            with self.ui_test.create_doc_in_start_center("writer") as xComponent:
                # When inserting a new frame with the default width:
                with self.ui_test.execute_dialog_through_command(".uno:InsertFrame") as xDialog:
                    xWidth = xDialog.getChild("width")
                    frame_width = float(get_state_as_dict(xWidth)["Value"])
                # Then make sure the width is not zero:
                # cm -> mm100
                expected_mm100 = frame_width * 1000
                # Without the accompanying fix in place, this test would have failed with:
                # AssertionError: 0 != 2000.0
                # i.e. the width was empty instead of the size from the UI.
                self.assertEqual(xComponent.TextFrames.Frame1.Size.Width, expected_mm100)

    def test_insert_floating_table(self):
        with self.ui_test.create_doc_in_start_center("writer") as xComponent:
            # Given a Writer document with a selected (inline) table:
            args = {
                "Columns": 1,
                "Rows": 1,
            }
            self.xUITest.executeCommandWithParameters(".uno:InsertTable", mkPropertyValues(args))
            self.xUITest.executeCommand(".uno:SelectAll")
            # When converting it to a split fly:
            with self.ui_test.execute_dialog_through_command(".uno:InsertFrame") as xDialog:
                xFlySplit = xDialog.getChild("flysplit")
                fly_split_visible = get_state_as_dict(xFlySplit)["Visible"] == "true"
            # Then make sure the inserted fly can be marked as "split allowed":
            # Without the accompanying fix in place, this test would have failed, the fly had to be
            # inserted first, only then it could be marked as "split allowed".
            self.assertEqual(fly_split_visible, True)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
