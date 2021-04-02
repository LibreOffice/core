# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from libreoffice.uno.propertyvalue import mkPropertyValues

from uitest.framework import UITestCase
from uitest.uihelper.common import type_text, get_state_as_dict
from uitest.uihelper.keyboard import select_all

class InvalidNames(UITestCase):

    def test_invalid_names(self):

        self.ui_test.create_doc_in_start_center("calc")

        self.ui_test.execute_modeless_dialog_through_command(".uno:AddName")

        xAddNameDlg = self.xUITest.getTopFocusWindow()

        invalid_names = ["A1", "12", "1.2", "A1:B2", "test.a", \
                "test+", "test-", "test*", "test!abc", "test#", \
                "test^", "test°", "test$", "test§", "test%", \
                "test&", "test/", "test(", "test)", "test[", "test]", \
                "test\\", "test`", "test´", "test'", "test~", "test<", \
                "tst>", "test|", "test:t", "test;z"]

        xLabel = xAddNameDlg.getChild("label")
        xAddBtn = xAddNameDlg.getChild("add")
        xEdit = xAddNameDlg.getChild("edit")

        success_text = get_state_as_dict(xLabel)["Text"]

        for name in invalid_names:
            with self.subTest(name = name):
                select_all(xEdit)
                type_text(xEdit, name)

                # tdf#132869 - Without the fix in place, this test would have failed with
                # - Expected: "Invalid name. Start with a letter, use only letters, numbers and underscore."
                # - Actual  : ""
                self.assertNotEqual(success_text, get_state_as_dict(xEdit)["QuickHelpText"])
                self.assertEqual(get_state_as_dict(xAddBtn)["Enabled"], "false")


        select_all(xEdit)
        type_text(xEdit, "valid_name")

        self.assertEqual(success_text, get_state_as_dict(xLabel)["Text"])
        self.assertEqual(success_text, get_state_as_dict(xEdit)["QuickHelpText"])
        self.assertEqual(get_state_as_dict(xAddBtn)["Enabled"], "true")

        self.ui_test.close_dialog_through_button(xAddBtn)

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
