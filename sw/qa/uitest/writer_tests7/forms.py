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

    def test_tdf140486(self):

        self.ui_test.load_file(get_url_for_data_file("tdf140486.odt"))

        self.xUITest.executeCommand(".uno:JumpToNextFrame")

        self.ui_test.execute_modeless_dialog_through_command(".uno:ControlProperties")
        xChild = self.ui_test.wait_until_child_is_available('listbox-Empty string is NULL')

        # Without the fix in place, this test would have failed with
        # AssertionError: 'Yes' != 'No'
        self.assertEqual("Yes", get_state_as_dict(xChild)['SelectEntryText'])

        self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab:
