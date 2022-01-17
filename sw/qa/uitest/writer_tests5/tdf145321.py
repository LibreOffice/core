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

            for j in range(5):
                self.assertEqual("1", get_state_as_dict(xWriterEdit)["CurrentPage"])

                self.xUITest.executeCommand(".uno:GoToEndOfDoc")

                self.assertEqual("6", get_state_as_dict(xWriterEdit)["CurrentPage"])

                # Without the fix in place, this test would have crashed here
                self.xUITest.executeCommand(".uno:GoToStartOfDoc")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
