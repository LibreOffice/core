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

# Bug 156900 - no row deletion when there is a hidden last column

class tdf156900(UITestCase):
    def test_tdf156900(self):
        with self.ui_test.load_file(get_url_for_data_file("TC-table-del-add.docx")) as self.document:

            # accept all tracked changes
            self.xUITest.executeCommand(".uno:AcceptAllTrackedChanges")

            # delete last column
            self.xUITest.executeCommand(".uno:GoRight")
            self.xUITest.executeCommand(".uno:GoRight")
            self.xUITest.executeCommand(".uno:GoRight")
            self.xUITest.executeCommand(".uno:GoRight")
            self.xUITest.executeCommand(".uno:DeleteColumns")

            # hide changes
            self.xUITest.executeCommand(".uno:ShowTrackedChanges")

            # delete first row
            self.xUITest.executeCommand(".uno:DeleteRows")

            # accept all changes
            self.xUITest.executeCommand(".uno:AcceptAllTrackedChanges")

            tables = self.document.getTextTables()
            # This was 3 (not deleted row)
            self.assertEqual(len(tables[0].getRows()), 2)

            # show changes
            self.xUITest.executeCommand(".uno:ShowTrackedChanges")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
