# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
import unittest
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.debug import sleep

shortcuts = [
    {"command": "F1"},
    {"command": "F2"},
    {"command": "F5"},
    {"command": "F11"},
    {"command": "CTRL+k"},
    {"command": "CTRL+h"},
    {"command": "CTRL+f"},
    {"command": "CTRL+SHIFT+o", "skipSecondTrigger": True},
    {"command": "CTRL+F8"},
    {"command": "CTRL+`"},
    {"command": "CTRL+F5"},
    {"command": "CTRL+SHIFT+F4"},
    {"command": "CTRL+SHIFT+J"},
    {"command": "CTRL+ALT+c", "skipSecondTrigger": True},
    {"command": "CTRL+b"},
    {"command": "CTRL+i"},
    {"command": "CTRL+l"},
    {"command": "CTRL+e"},
    {"command": "CTRL+r"},
    {"command": "CTRL+j"},
    {"command": "CTRL+m"},
    {"command": "CTRL+1"},
]


def load_tests(loader, tests, pattern):
    return unittest.TestSuite(hitShortcut(shortcut)
                              for shortcut in shortcuts)

# Test to hit all listed shortcuts one by one
# the test only checks if calc crashes by using the shortcut, see e.g. tdf#128562
class hitShortcut(UITestCase):
    def check(self, dialog):
        calc_doc = self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()

        gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": dialog["command"]}))
        if 'skipSecondTrigger' not in dialog or dialog['skipSecondTrigger'] != True:
            gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": dialog["command"]}))

        self.ui_test.close_doc()

shortcutCount = 0
for shortcut in shortcuts:
    shortcutCount = shortcutCount + 1


    def ch(shortcut):
        return lambda self: self.check(shortcut)


    setattr(hitShortcut, "test_%02d_%s" % (shortcutCount, shortcut["command"]), ch(shortcut))
# vim: set shiftwidth=4 softtabstop=4 expandtab:
