# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import change_measurement_unit

class tdf99711(UITestCase):
    def test_tdf99711(self):

        writer_doc = self.ui_test.load_file(get_url_for_data_file("shape.odt"))

        #set measurement to millimeters
        change_measurement_unit(self, "Millimeter")

        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")

        self.xUITest.executeCommand(".uno:JumpToNextFrame")

        self.xUITest.executeCommand(".uno:Sidebar")
        xWriterEdit.executeAction("SIDEBAR", mkPropertyValues({"PANEL": "TextPropertyPanel"}))

        #wait until the sidebar is available
        xChild = self.ui_test.wait_until_child_is_available('selectwidth')
        self.assertEqual(get_state_as_dict(xChild)['Text'], '10.00 mm')

        xChild = self.ui_test.wait_until_child_is_available('selectheight')
        self.assertEqual(get_state_as_dict(xChild)['Text'], '10.00 mm')

        self.xUITest.executeCommand(".uno:Sidebar")

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:

