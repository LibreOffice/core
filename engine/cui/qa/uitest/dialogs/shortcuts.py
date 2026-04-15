# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import select_pos
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict

class Test(UITestCase):

    def test_tab_navigation(self):
        with self.ui_test.create_doc_in_start_center("writer"),\
                self.ui_test.execute_dialog_through_command(".uno:EditStyle") as xDialog:
            xTabs = xDialog.getChild("tabcontrol")
            select_pos(xTabs, "0")

            for i in range(16):
                self.assertEqual(get_state_as_dict(xTabs)["CurrPagePos"], str(i))
                xTabs.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+PAGEDOWN"}))

            for i in reversed(range(16)):
                self.assertEqual(get_state_as_dict(xTabs)["CurrPagePos"], str(i))
                xTabs.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+PAGEUP"}))



# vim: set shiftwidth=4 softtabstop=4 expandtab:
