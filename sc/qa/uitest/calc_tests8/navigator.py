#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict
import time

class tdf133028(UITestCase):

    def test_tdf133028(self):
        self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        xGridWin = xCalcDoc.getChild("grid_window")
        print(xCalcDoc.getChildren())

        self.xUITest.executeCommand(".uno:Sidebar")
        xGridWin.executeAction("SIDEBAR", mkPropertyValues({"PANEL": "ScNavigatorPanel"}))

        xNavigatorPanel = xGridWin.getChild("NavigatorPanelParent")
        print(xNavigatorPanel)
        xNavigatorPanel.executeAction("ROOT", tuple())

        xGridWin.executeAction("FOCUS", tuple())
        time.sleep(2)

        self.xUITest.executeCommand(".uno:Sidebar")
        self.ui_test.close_doc()
