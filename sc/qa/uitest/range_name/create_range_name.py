# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from libreoffice.uno.propertyvalue import mkPropertyValues

from uitest.framework import UITestCase
from uitest.uihelper.common import type_text, select_pos
from uitest.uihelper.common import get_state_as_dict

class CreateRangeNameTest(UITestCase):

    def test_create_range_name(self):

        self.ui_test.create_doc_in_start_center("calc")

        calcDoc = self.xUITest.getTopFocusWindow()
        xPosWindow = calcDoc.getChild('pos_window')
        self.assertEqual('A1', get_state_as_dict(xPosWindow)['Text'])

        self.ui_test.execute_modeless_dialog_through_command(".uno:AddName")

        xAddNameDlg = self.xUITest.getTopFocusWindow()

        xEdit = xAddNameDlg.getChild("edit")
        type_text(xEdit, "globalRangeName")

        xAddBtn = xAddNameDlg.getChild("add")
        self.ui_test.close_dialog_through_button(xAddBtn)

        self.assertEqual('globalRangeName', get_state_as_dict(xPosWindow)['Text'])

        self.ui_test.close_doc()

    def test_create_local_range_name(self):

        self.ui_test.create_doc_in_start_center("calc")

        calcDoc = self.xUITest.getTopFocusWindow()
        xPosWindow = calcDoc.getChild('pos_window')
        self.assertEqual('A1', get_state_as_dict(xPosWindow)['Text'])

        self.ui_test.execute_modeless_dialog_through_command(".uno:AddName")

        xAddNameDlg = self.xUITest.getTopFocusWindow()

        xEdit = xAddNameDlg.getChild("edit")
        type_text(xEdit, "localRangeName")

        xScope = xAddNameDlg.getChild("scope")
        select_pos(xScope, "1")

        xAddBtn = xAddNameDlg.getChild("add")
        self.ui_test.close_dialog_through_button(xAddBtn)

        # tdf#67007: Without the fix in place, this test would have failed with
        # AssertionError: 'localRangeName' != 'A1'
        self.assertEqual('localRangeName', get_state_as_dict(xPosWindow)['Text'])

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
