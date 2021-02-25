# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file
from libreoffice.uno.propertyvalue import mkPropertyValues

class Forms(UITestCase):

    def test_tdf131000(self):

        self.ui_test.load_file(get_url_for_data_file("tdf131000.ods"))
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")

        gridwin.executeAction("SELECT", mkPropertyValues({"OBJECT": "form"}))

        # Without the fix in place, this test would have hung here
        self.ui_test.execute_modeless_dialog_through_command(".uno:ControlProperties")

        xCellRange = self.ui_test.wait_until_child_is_available('textfield-Source cell range')

        self.assertEqual("A1:A7000", get_state_as_dict(xCellRange)['Text'])

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
