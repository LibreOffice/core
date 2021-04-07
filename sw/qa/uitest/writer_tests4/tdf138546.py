# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import change_measurement_unit
from uitest.uihelper.common import get_state_as_dict
from uitest.uihelper.common import select_pos
from uitest.uihelper.common import get_url_for_data_file

class tdf138546(UITestCase):
    def test_tdf138546(self):
        self.ui_test.load_file(get_url_for_data_file("tdf138546.odt"))
        xWriterDoc = self.xUITest.getTopFocusWindow()

        change_measurement_unit(self, "Centimeter")

        #dialog Columns
        self.ui_test.execute_dialog_through_command(".uno:FormatColumns")
        xDialog = self.xUITest.getTopFocusWindow()

        colsnf = xDialog.getChild("colsnf")
        width1mf = xDialog.getChild("width1mf")
        self.assertEqual(get_state_as_dict(colsnf)["Text"], "2")
        self.assertEqual((get_state_as_dict(width1mf)["Text"])[0:3], "2.0") #2.00 cm
        xOKBtn = xDialog.getChild("cancel")
        self.ui_test.close_dialog_through_button(xOKBtn)

        self.ui_test.execute_dialog_through_command(".uno:PageDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        tabcontrol = xDialog.getChild("tabcontrol")
        select_pos(tabcontrol, "7") #Columns
        colsnf = xDialog.getChild("colsnf")
        width1mf = xDialog.getChild("width1mf")
        self.assertEqual(get_state_as_dict(colsnf)["Text"], "2")
        self.assertEqual((get_state_as_dict(width1mf)["Text"])[0:3], "2.0") #2.00 cm
        xOKBtn = xDialog.getChild("cancel")
        self.ui_test.close_dialog_through_button(xOKBtn)

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
