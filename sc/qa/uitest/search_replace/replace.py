# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file

class ReplaceTest(UITestCase):

    def test_tdf106194(self):
        doc = self.ui_test.load_file(get_url_for_data_file("tdf106194.ods"))

        xGridWin = self.xUITest.getTopFocusWindow().getChild("grid_window")

        self.ui_test.execute_modeless_dialog_through_command(".uno:SearchDialog")

        xSearchDlg = self.xUITest.getTopFocusWindow()

        xSearchTerm = xSearchDlg.getChild("searchterm")
        xSearchTerm.executeAction("TYPE", mkPropertyValues({"TEXT": "1"}))
        xReplaceTerm = xSearchDlg.getChild("replaceterm")
        xReplaceTerm.executeAction("TYPE", mkPropertyValues({"TEXT": "2"}))

        xSearchBtn = xSearchDlg.getChild("search")
        xSearchBtn.executeAction("CLICK", tuple())

        self.assertEqual(get_state_as_dict(xGridWin)["CurrentRow"], "1")
        lastTopVisibleRow = int(get_state_as_dict(xGridWin)["TopVisibleRow"])

        # start replacing
        xReplaceBtn = xSearchDlg.getChild("replace")
        xReplaceBtn.executeAction("CLICK", tuple())

        # check position and visible range
        self.assertEqual(get_state_as_dict(xGridWin)["CurrentRow"], "199")
        currentTopVisibleRow = int(get_state_as_dict(xGridWin)["TopVisibleRow"])
        self.assertGreater(currentTopVisibleRow, lastTopVisibleRow)

        lastTopVisibleRow = currentTopVisibleRow

        # replace again
        xReplaceBtn.executeAction("CLICK", tuple())

        # check position and visible range
        self.assertEqual(get_state_as_dict(xGridWin)["CurrentRow"], "499")
        currentTopVisibleRow = int(get_state_as_dict(xGridWin)["TopVisibleRow"])
        self.assertGreater(currentTopVisibleRow, lastTopVisibleRow)

        xReplaceBtn.executeAction("CLICK", tuple())

        xCloseBtn = xSearchDlg.getChild("close")

        self.ui_test.close_dialog_through_button(xCloseBtn)

        self.ui_test.close_doc()
