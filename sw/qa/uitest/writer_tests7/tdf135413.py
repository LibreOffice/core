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

class tdf135413(UITestCase):

    def test_tdf135413(self):

        # Without the fix in place, this test would have crashed opening the document
        # It seems the issue is only reproducible when the UI is displayed,
        # thus, test it with a UItest
        with self.ui_test.load_file(get_url_for_data_file("tdf135413.docx")):
            xWriterDoc = self.xUITest.getTopFocusWindow()
            xWriterEdit = xWriterDoc.getChild("writer_edit")
            self.assertEqual("16", get_state_as_dict(xWriterEdit)["Pages"])

# vim: set shiftwidth=4 softtabstop=4 expandtab:
