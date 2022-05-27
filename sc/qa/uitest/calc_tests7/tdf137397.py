# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import change_measurement_unit

class tdf137397(UITestCase):

    def test_tdf137397(self):

        with self.ui_test.load_file(get_url_for_data_file("tdf137397.ods")):

            change_measurement_unit(self, 'Centimeter')

            xCalcDoc = self.xUITest.getTopFocusWindow()
            xGridWin = xCalcDoc.getChild("grid_window")

            xGridWin.executeAction("SELECT", mkPropertyValues({"OBJECT": "Shape"}))

            self.xUITest.executeCommand(".uno:Sidebar")
            xGridWin.executeAction("SIDEBAR", mkPropertyValues({"PANEL": "TextPropertyPanel"}))

            xChild = self.ui_test.wait_until_child_is_available('selectwidth')
            self.assertEqual(get_state_as_dict(xChild)['Text'], '6.00 cm')

            # Without the fix in place, this test would have failed with
            # AssertionError: '-14,585,309.84 cm' != '2.00 cm'
            xChild = self.ui_test.wait_until_child_is_available('selectheight')
            self.assertEqual(get_state_as_dict(xChild)['Text'], '2.00 cm')


            self.xUITest.executeCommand(".uno:Sidebar")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
