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


# Bug 124829 - CRASH: cutting and undoing a sheet with external links
class tdf124829(UITestCase):
    def test_tdf124829_Crash_cutting_undo_sheet_external_links(self):
        #numberingformatpage.ui
        with self.ui_test.load_file(get_url_for_data_file("tdf124829.ods")) as calc_doc:
            self.xUITest.executeCommand(".uno:SelectAll")
            self.xUITest.executeCommand(".uno:Cut")
            for i in range(40):
                self.xUITest.executeCommand(".uno:Undo")

            #verify; no crashes
            self.assertEqual(calc_doc.Sheets.getCount(), 6)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
