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

#Bug 120731 - Crash cuilo!makeAutoCorrEdit when open character dialog with large amount of text selected

class tdf120731(UITestCase):
   def test_tdf120731_crash_open_char_dialog(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf120731.odt")) as writer_doc:
            xWriterDoc = self.xUITest.getTopFocusWindow()

            self.xUITest.executeCommand(".uno:SelectAll")
            with self.ui_test.execute_dialog_through_command(".uno:FontDialog"):
                pass
            self.assertEqual(writer_doc.Text.String[0:5], "Lorem")
# vim: set shiftwidth=4 softtabstop=4 expandtab:
