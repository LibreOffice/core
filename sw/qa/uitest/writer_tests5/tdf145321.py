# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import get_url_for_data_file, get_state_as_dict

class tdf145321(UITestCase):

    # This test has been implemented as a UItest because it doesn't crash as CppUnittest
    def test_tdf145321(self):

        with self.ui_test.load_file(get_url_for_data_file("tdf145321.odt")) as writer_doc:
            xWriterDoc = self.xUITest.getTopFocusWindow()
            xWriterEdit = xWriterDoc.getChild("writer_edit")

            for i in range(1,6):
                with self.ui_test.execute_dialog_through_command(".uno:StatePageNumber") as xDialog:
                    self.assertEqual(str(i), get_state_as_dict(xDialog.getChild("page"))["Text"])

                self.xUITest.executeCommand(".uno:PageDown")

            # Without the fix in place, this test would have crashed here
            for i in range(6, 0, -1):
                with self.ui_test.execute_dialog_through_command(".uno:StatePageNumber") as xDialog:
                    self.assertEqual(str(i), get_state_as_dict(xDialog.getChild("page"))["Text"])

                self.xUITest.executeCommand(".uno:PageUp")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
