# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import select_by_text
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file

#Bug 124586 - Crash if switch from user outline numbering to chapter numbering with same paragraph style

class tdf124586(UITestCase):
   def test_tdf124586_crash_switch_outline_numbering(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf124586.odt")) as writer_doc:
            xWriterDoc = self.xUITest.getTopFocusWindow()

            #Goto Tools > Chapter Numbering.
            with self.ui_test.execute_dialog_through_command(".uno:ChapterNumberingDialog") as xDialog:
                xstyle = xDialog.getChild("style")
                select_by_text(xstyle, "MyHeading")

            self.assertEqual(writer_doc.Text.String[0:8], "Schritte")

            with self.ui_test.execute_dialog_through_command(".uno:ChapterNumberingDialog") as xDialog:
                xstyle = xDialog.getChild("style")
                self.assertEqual(get_state_as_dict(xstyle)["SelectEntryText"], "MyHeading")
# vim: set shiftwidth=4 softtabstop=4 expandtab:
