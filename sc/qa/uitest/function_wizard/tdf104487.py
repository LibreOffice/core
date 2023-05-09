# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, select_pos

class tdf104487(UITestCase):
    def test_tdf104487_remember_function_category(self):
        with self.ui_test.create_doc_in_start_center("calc"):
            # Open function dialog and select a function category
            with self.ui_test.execute_modeless_dialog_through_command(".uno:FunctionDialog") as xDialog:
                xCategory = xDialog.getChild("category")
                select_pos(xCategory, "3")

            # Open function dialog again and check whether function category was remembered
            with self.ui_test.execute_modeless_dialog_through_command(".uno:FunctionDialog") as xDialog:
                xCategory = xDialog.getChild("category")
                # Without the fix in place, this test would have failed with
                # AssertionError: '3' != '1'
                # i.e. the last used function category in the function wizard was not remembered
                self.assertEqual("3", get_state_as_dict(xCategory)["SelectEntryPos"])

# vim: set shiftwidth=4 softtabstop=4 expandtab:
