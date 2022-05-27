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

#Bug:  Delete a sheet in a calc document and LO crashes

class tdf114992(UITestCase):
    def test_tdf114992_delete_sheet_crash(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf114992.ods")) as calc_doc:
            with self.ui_test.execute_dialog_through_command(".uno:Remove", close_button="yes"):
                pass

            self.assertEqual(calc_doc.Sheets.getCount(), 1)
            self.xUITest.executeCommand(".uno:Undo")
            self.assertEqual(calc_doc.Sheets.getCount(), 2)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
