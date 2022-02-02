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
from uitest.uihelper.common import select_pos
from libreoffice.uno.propertyvalue import mkPropertyValues

class tdf105301(UITestCase):

    def test_tdf105301(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf105301.ods")) as calc_doc:
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")

            gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:R9"}))

            with self.ui_test.execute_dialog_through_command(".uno:DataSort") as xDialog:
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "0")
                self.assertEqual("B", get_state_as_dict(xDialog.getChild("sortlb"))['DisplayText'])
                self.assertEqual("C", get_state_as_dict(xDialog.getChild("sortlb2"))['DisplayText'])
                self.assertEqual("D", get_state_as_dict(xDialog.getChild("sortlb3"))['DisplayText'])

                # Without the fix in place, this test would have failed with
                # AssertionError: 'E' != '- undefined -'
                self.assertEqual("E", get_state_as_dict(xDialog.getChild("sortlb4"))['DisplayText'])
                self.assertEqual("F", get_state_as_dict(xDialog.getChild("sortlb5"))['DisplayText'])
                self.assertEqual("G", get_state_as_dict(xDialog.getChild("sortlb6"))['DisplayText'])

                # tdf#51828: Without the fix in place, this test would have failed here
                self.assertEqual("- undefined -", get_state_as_dict(xDialog.getChild("sortlb7"))['DisplayText'])


# vim: set shiftwidth=4 softtabstop=4 expandtab:
