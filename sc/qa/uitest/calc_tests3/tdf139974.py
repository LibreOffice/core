# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_url_for_data_file


class tdf139974(UITestCase):

    def test_tdf139974(self):

        with self.ui_test.load_file(get_url_for_data_file("tdf139974.ods")) as document:

            self.assertEqual(1930864179960.0, get_cell_by_position(document, 0, 0, 0).getValue())
            self.assertEqual(17204.0, get_cell_by_position(document, 0, 1, 0).getValue())

            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")
            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))

            gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE":"F2"}))

            self.xUITest.executeCommand(".uno:SelectAll")
            self.xUITest.executeCommand(".uno:Copy")

            gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "ESC"}))

            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A2"}))

            gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE":"F2"}))

            self.xUITest.executeCommand(".uno:Paste")

            gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE":"RETURN"}))

            # Without the fix in place, this test would have failed with
            # AssertionError: 17205 != 17204
            self.assertEqual(17205.0, get_cell_by_position(document, 0, 1, 1).getValue())

            self.assertEqual(1930864179960.0, get_cell_by_position(document, 0, 0, 1).getValue())

# vim: set shiftwidth=4 softtabstop=4 expandtab:
