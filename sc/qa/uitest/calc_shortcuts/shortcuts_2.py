# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
import unittest
from libreoffice.uno.propertyvalue import mkPropertyValues

# Test to hit all listed shortcuts one by one
# the test only checks if calc crashes by using the shortcut, see e.g. tdf#128562
class Shortcut(UITestCase):
    def test_shortcut(self):
        shortcuts = [
            {"command": "F1"},
            {"command": "F2"},
            {"command": "F5"},
            {"command": "F7", "closeButton": "ok"},
            {"command": "F9"},
            {"command": "F11"},
            {"command": "CTRL+k"},
            {"command": "CTRL+h"},
            {"command": "CTRL+f"},
            {"command": "CTRL+b"},
            {"command": "CTRL+i"},
            {"command": "CTRL+l"},
            {"command": "CTRL+e"},
            {"command": "CTRL+r"},
            {"command": "CTRL+j"},
            {"command": "CTRL+m"},
            {"command": "CTRL+F5"},
            {"command": "CTRL+F8"},
            {"command": "CTRL+`"},
            {"command": "SHIFT+F5"},
            {"command": "SHIFT+F9"},
            {"command": "ALT+F12", "closeButton": "cancel", "skipSecondTrigger": True},
            {"command": "CTRL+SHIFT+F4"},
            {"command": "CTRL+SHIFT+J"},
            {"command": "F12", "skipSecondTrigger": True, "closeButton": "ok"},
            {"command": "CTRL+F3", "skipSecondTrigger": True, "closeButton": "cancel"},
            {"command": "CTRL+F2", "skipSecondTrigger": True, "closeButton": "cancel"},
            {"command": "CTRL+F5"},
        ]


        calc_doc = self.ui_test.create_doc_in_start_center("calc")

        for shortcut in shortcuts:
            print(shortcut["command"])
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")
            document = self.ui_test.get_component()
            gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": shortcut["command"]}))

            xDialog = self.xUITest.getTopFocusWindow()

            if 'closeButton' in shortcut:
                xCloseBtn = xDialog.getChild(shortcut['closeButton'])
                self.ui_test.close_dialog_through_button(xCloseBtn)

            if 'skipSecondTrigger' not in shortcut or shortcut['skipSecondTrigger'] is not True:
                gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": shortcut["command"]}))

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
