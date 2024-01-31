# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file, select_by_text

class printDialog(UITestCase):
    def test_printDialog(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf155218.ods")):
            with self.ui_test.execute_dialog_through_command(".uno:Print", close_button="cancel") as xDialog:

                xPortraiTotalNumberPages = xDialog.getChild("totalnumpages")
                self.assertEqual(get_state_as_dict(xPortraiTotalNumberPages)["Text"], "/ 2")

                xPortraiPageRange = xDialog.getChild("pagerange")
                self.assertEqual(get_state_as_dict(xPortraiPageRange)["Text"], "1-2")

                xpageorientationbox = xDialog.getChild("pageorientationbox")
                select_by_text(xpageorientationbox, "Landscape")

                # Without the fix in place, this test would have failed with
                # Expected: "/ 1"
                # Actual  : "/ 2"
                xLandscapeTotalNumberPages = xDialog.getChild("totalnumpages")
                self.assertEqual(get_state_as_dict(xLandscapeTotalNumberPages)["Text"], "/ 1")

                # Without the fix in place, this test would have failed with
                # Expected: "1"
                # Actual  : "1-2"
                xLandscapePageRange = xDialog.getChild("pagerange")
                self.assertEqual(get_state_as_dict(xLandscapePageRange)["Text"], "1")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
