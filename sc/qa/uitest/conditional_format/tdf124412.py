# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import get_url_for_data_file, get_state_as_dict
from libreoffice.uno.propertyvalue import mkPropertyValues

class tdf124412(UITestCase):

    def test_tdf124412(self):

        with self.ui_test.load_file(get_url_for_data_file("tdf124412.ods")) as calc_doc:

            xTopWindow = self.xUITest.getTopFocusWindow()
            gridwin = xTopWindow.getChild("grid_window")

            gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A2:A13"}))

            with self.ui_test.execute_modeless_dialog_through_command(".uno:ConditionalFormatDialog") as xCondFormatDlg:
                xCondition1 = xCondFormatDlg.getChild("Condition 1")
                xCondition2 = xCondFormatDlg.getChild("Condition 2")

                # This is empty because the entry is selected
                self.assertEqual("", get_state_as_dict(xCondition1)["DisplayText"])

                # Without the fix in place, this test would have failed with
                # AssertionError: 'Date is last month' != 'Date is today'
                self.assertEqual("Date is last month", get_state_as_dict(xCondition2)["DisplayText"])

# vim: set shiftwidth=4 softtabstop=4 expandtab:
