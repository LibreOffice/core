# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import get_url_for_data_file
from uitest.uihelper.common import select_pos

#Bug 123547 - Systematic crash after opening Tools > Chapter Numbering

class tdf123547(UITestCase):
   def test_tdf123547_numbering_dialog_crash(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf123547.docx")) as writer_doc:
            xWriterDoc = self.xUITest.getTopFocusWindow()

            with self.ui_test.execute_dialog_through_command(".uno:ChapterNumberingDialog") as xDialog:
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "1")
            #verify we didn't crash
            self.assertEqual(writer_doc.CurrentController.PageCount, 1)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
