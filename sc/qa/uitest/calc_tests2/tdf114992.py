# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
import org.libreoffice.unotest
import pathlib
#Bug:  Delete a sheet in a calc document and LO crashes
def get_url_for_data_file(file_name):
    return pathlib.Path(org.libreoffice.unotest.makeCopyFromTDOC(file_name)).as_uri()

class tdf114992(UITestCase):
    def test_tdf114992_delete_sheet_crash(self):
        calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf114992.ods"))
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()

        self.ui_test.execute_dialog_through_command(".uno:Remove")  #delete sheet
        xDialog = self.xUITest.getTopFocusWindow()
        xOKButton = xDialog.getChild("yes")
        xOKButton.executeAction("CLICK", tuple())

        self.assertEqual(document.Sheets.getCount(), 1)
        self.xUITest.executeCommand(".uno:Undo")
        self.assertEqual(document.Sheets.getCount(), 2)

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab: