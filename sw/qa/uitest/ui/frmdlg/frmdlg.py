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


# vim: set shiftwidth=4 softtabstop=4 expandtab:
