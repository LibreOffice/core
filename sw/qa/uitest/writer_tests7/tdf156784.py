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

# Bug 156784 - crash fix: Select All in section starting table with hidden first column

class tdf156784(UITestCase):
    def test_tdf156784(self):
        with self.ui_test.load_file(get_url_for_data_file("TC-table-del-add.docx")) as self.document:

            # accept all tracked changes
            self.xUITest.executeCommand(".uno:AcceptAllTrackedChanges")

            # delete first column
            self.xUITest.executeCommand(".uno:DeleteColumns")

            # hide changes
            self.xUITest.executeCommand(".uno:ShowTrackedChanges")

            # select cell content
            self.xUITest.executeCommand(".uno:SelectAll")

            # This resulted crashing (select section starting table with hidden first column)
            self.xUITest.executeCommand(".uno:SelectAll")

            # show changes
            self.xUITest.executeCommand(".uno:ShowTrackedChanges")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
