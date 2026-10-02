# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file, select_by_text, mkPropertyValues

class printDialog(UITestCase):
    def test_printDialog(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf155218.ods")):
            with self.ui_test.execute_dialog_through_command(".uno:Print", close_button="cancel") as xDialog:
                # The number of pages is updated on an idle handler after creating the dialog. Let’s
                # wait until the main loop is truly idle again to make sure the text has been
                # updated before reading it
                xToolkit = self.xContext.ServiceManager.createInstance('com.sun.star.awt.Toolkit')
                xToolkit.waitUntilAllIdlesDispatched()

                xPortraiTotalNumberPages = xDialog.getChild("totalnumpages")
                self.assertEqual(get_state_as_dict(xPortraiTotalNumberPages)["Text"], "/ 2")

                xPageRange = xDialog.getChild("pagerange")
                self.assertEqual(get_state_as_dict(xPageRange)["Text"], "")

                xPageOrientationBox = xDialog.getChild("pageorientationbox")
                select_by_text(xPageOrientationBox, "Landscape")

                # Without the fix in place, this test would have failed with
                # Expected: "/ 1"
                # Actual  : "/ 2"
                xLandscapeTotalNumberPages = xDialog.getChild("totalnumpages")
                self.assertEqual(get_state_as_dict(xLandscapeTotalNumberPages)["Text"], "/ 1")

                # Page range is empty by default and must not be populated automatically
                self.assertEqual(get_state_as_dict(xPageRange)["Text"], "")

                select_by_text(xPageOrientationBox, "Portrait")
                # Page range must remain empty when the number of pages changes.
                self.assertEqual(get_state_as_dict(xPageRange)["Text"], "")

                # User enters a page range.
                xPageRange.executeAction("TYPE", mkPropertyValues({"TEXT": "2-3"}))

                select_by_text(xPageOrientationBox, "Landscape")
                select_by_text(xPageOrientationBox, "Portrait")

                self.assertEqual(get_state_as_dict(xPageRange)["Text"], "2-3")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
