# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
from libreoffice.calc.document import get_cell_by_position

class tdf135761(UITestCase):
    def test_tdf135761_ctrl_backspace(self):
        with self.ui_test.create_doc_in_start_center("calc") as document:
            xCalcDoc = self.xUITest.getTopFocusWindow()
            xGridWindow = xCalcDoc.getChild("grid_window")

            # Enter text including two whitespaces in the text, and delete the left part of the text
            xGridWindow.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
            xGridWindow.executeAction("TYPE", mkPropertyValues({"TEXT": "a  b"}))
            xGridWindow.executeAction("TYPE", mkPropertyValues({"KEYCODE":"F2"}))
            xGridWindow.executeAction("TYPE", mkPropertyValues({"KEYCODE":"LEFT"}))
            xGridWindow.executeAction("TYPE", mkPropertyValues({"KEYCODE":"LEFT"}))
            xGridWindow.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+BACKSPACE"}))
            xGridWindow.executeAction("TYPE", mkPropertyValues({"KEYCODE":"RETURN"}))

            # Without the fix in place, this test would have failed with
            # AssertionError: ' b' != 'a b'

            self.assertEqual(get_cell_by_position(document, 0, 0, 0).getString(), " b")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
