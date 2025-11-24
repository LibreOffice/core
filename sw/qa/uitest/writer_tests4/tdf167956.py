# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file
import time

class tdf167956(UITestCase):

    def wait_until_styles_are_displayed(self, xStylesView):
        while True:
            nChildren = len(xStylesView.getChildren())
            if nChildren > 0:
                break
            time.sleep(self.ui_test.get_default_sleep())

    def test_tdf167956(self):
        with self.ui_test.create_doc_in_start_center("writer"):
            self.xUITest.executeCommand(".uno:ToolbarMode?Mode:string=notebookbar.ui")

            xWriterDoc = self.xUITest.getTopFocusWindow()
            xStylesView = xWriterDoc.getChild("stylesview")
            self.wait_until_styles_are_displayed(xStylesView)
            self.assertLessEqual("10", get_state_as_dict(xStylesView)["Children"])
            self.assertEqual("Default Paragraph Style", get_state_as_dict(xStylesView)["SelectEntryText"])

        with self.ui_test.load_file(get_url_for_data_file("tdf167956.docx")):
            xWriterDoc = self.xUITest.getTopFocusWindow()
            xStylesView = xWriterDoc.getChild("stylesview")

            # Without the fix in place, this test would have crashed here
            self.wait_until_styles_are_displayed(xStylesView)
            self.assertLessEqual("10", get_state_as_dict(xStylesView)["Children"])
            self.assertEqual("Intense Quote", get_state_as_dict(xStylesView)["SelectEntryText"])

            # Reset to the default toolbar
            self.xUITest.executeCommand(".uno:ToolbarMode?Mode:string=Default")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
