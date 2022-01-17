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
from libreoffice.uno.propertyvalue import mkPropertyValues

class tdf145321(UITestCase):

    # This test has been implemented as a UItest because it doesn't crash as CppUnittest
    def test_tdf145321(self):

        with self.ui_test.load_file(get_url_for_data_file("tdf145321.odt")) as writer_doc:
            xWriterDoc = self.xUITest.getTopFocusWindow()
            xWriterEdit = xWriterDoc.getChild("writer_edit")

            for j in range(5):
                for i in range(1,6):
                    xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "PAGEDOWN"}))


                # Without the fix in place, this test would have crashed here
                for i in range(6, 0, -1):
                    xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "PAGEUP"}))

# vim: set shiftwidth=4 softtabstop=4 expandtab:
