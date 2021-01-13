# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file
from libreoffice.uno.propertyvalue import mkPropertyValues

class tdf132783(UITestCase):

   def test_tdf132783(self):
        calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf132783.ods"))

        for i in range(5):
            self.ui_test.execute_modeless_dialog_through_command(".uno:SearchDialog")
            xDialog = self.xUITest.getTopFocusWindow()
            xSearchTerm = xDialog.getChild("searchterm")

            xSearchTerm.executeAction("TYPE", mkPropertyValues({"TEXT":"1"}))

            xSearch = xDialog.getChild("search")
            xSearch.executeAction("CLICK", tuple())

            xcloseBtn = xDialog.getChild("close")
            self.ui_test.close_dialog_through_button(xcloseBtn)

            xGridWin = self.xUITest.getTopFocusWindow().getChild("grid_window")
            self.assertEqual(get_state_as_dict(xGridWin)["CurrentRow"], "1")
            self.assertEqual(get_state_as_dict(xGridWin)["CurrentColumn"], str( 96 + i))

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
