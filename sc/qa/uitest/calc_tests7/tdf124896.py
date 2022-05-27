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

# Bug 124896 - CRASH: deleting sheet

class tdf124896(UITestCase):
   def test_tdf124896_crash_delete_sheet(self):
    with self.ui_test.load_file(get_url_for_data_file("tdf124896.ods")) as calc_doc:
        # Remove X Bar R Chart (2) sheet
        with self.ui_test.execute_dialog_through_command(".uno:Remove", close_button="yes"):
            pass

        #verify; no crashes
        self.assertEqual(calc_doc.Sheets.getCount(), 2)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
