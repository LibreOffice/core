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
from uitest.uihelper.calc import enter_text_to_cell

class tdf164722(UITestCase):
    def test_tdf164722(self):

        with self.ui_test.create_doc_in_start_center("calc") as document:
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")

            cond_formats = document.Sheets[0].ConditionalFormats
            self.assertEqual(0, cond_formats.Length)

            enter_text_to_cell(gridwin, "A1", "1")
            enter_text_to_cell(gridwin, "A2", "2")
            enter_text_to_cell(gridwin, "A3", "3")

            self.xUITest.executeCommand(".uno:SelectColumn")

            with self.ui_test.execute_dialog_through_command(".uno:ColorScaleFormatDialog") as xCondFormat:
                pass

            self.assertEqual(1, cond_formats.Length)

            self.xUITest.executeCommand(".uno:Undo")

            # Without the fix in place, it would have failed here
            with self.ui_test.execute_dialog_through_command(".uno:ColorScaleFormatDialog") as xCondFormat:
                pass

            self.assertEqual(1, cond_formats.Length)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
