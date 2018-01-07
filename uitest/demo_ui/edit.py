# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from libreoffice.uno.propertyvalue import mkPropertyValues

from uitest.framework import UITestCase
from uitest.uihelper.common import type_text, get_state_as_dict

import time

class EditTest(UITestCase):

    def test_type_text(self):

        self.ui_test.create_doc_in_start_center("calc")

        self.ui_test.execute_modeless_dialog_through_command(".uno:AddName")
        xAddNameDlg = self.xUITest.getTopFocusWindow()

        xEdit = xAddNameDlg.getChild("edit")

        type_text(xEdit, "simpleRangeName")

        xAddBtn = xAddNameDlg.getChild("cancel")
        self.ui_test.close_dialog_through_button(xAddBtn)

        self.ui_test.close_doc()

    def test_select_text(self):

        self.ui_test.create_doc_in_start_center("calc")

        self.ui_test.execute_modeless_dialog_through_command(".uno:AddName")
        xAddNameDlg = self.xUITest.getTopFocusWindow()

        xEdit = xAddNameDlg.getChild("edit")

        type_text(xEdit, "simpleRangeName")
        xEdit.executeAction("SELECT", mkPropertyValues({"FROM": "2", "TO": "9"}))
        type_text(xEdit, "otherChars")
        self.assertEqual("siotherCharsgeName", get_state_as_dict(xEdit)["Text"])

        xEdit.executeAction("SELECT", mkPropertyValues({"FROM": "2", "TO": "12"}))
        self.assertEqual("otherChars", get_state_as_dict(xEdit)["SelectedText"])

        xAddBtn = xAddNameDlg.getChild("cancel")
        self.ui_test.close_dialog_through_button(xAddBtn)

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
