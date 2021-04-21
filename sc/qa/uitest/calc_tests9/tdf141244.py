# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.calc import enter_text_to_cell

class tdf141244(UITestCase):

    def test_tdf141244(self):

        self.ui_test.create_doc_in_start_center("calc")

        calcDoc = self.xUITest.getTopFocusWindow()
        gridwin = calcDoc.getChild("grid_window")

        enter_text_to_cell(gridwin, "A1", '=DDE("soffice";"data1.ods";"sheet1.A1")')

        self.ui_test.execute_dialog_through_command(".uno:EditLinks")
        xDialog = self.xUITest.getTopFocusWindow()

        xLinks = xDialog.getChild("TB_LINKS")
        self.assertEqual(1, len(xLinks.getChildren()))

        def handle_modify_dlg(dialog):
            self.assertEqual("soffice", get_state_as_dict(dialog.getChild("app"))['Text'])
            self.assertEqual("data1.ods", get_state_as_dict(dialog.getChild("file"))['Text'])
            self.assertEqual("sheet1.A1", get_state_as_dict(dialog.getChild("category"))['Text'])

            # tdf#141770: Without the fix in place, the cancel button wouldn't have worked here
            xCancelBtn = dialog.getChild("cancel")
            self.ui_test.close_dialog_through_button(xCancelBtn)

        xChangeBtn = xDialog.getChild("CHANGE_SOURCE")

        self.ui_test.execute_blocking_action(xChangeBtn.executeAction, args=('CLICK', ()),
                dialog_handler=handle_modify_dlg)

        xClose = xDialog.getChild("close")
        self.ui_test.close_dialog_through_button(xClose)

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
