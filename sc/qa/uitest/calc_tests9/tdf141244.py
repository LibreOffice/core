# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.calc import enter_text_to_cell
from uitest.uihelper.common import get_state_as_dict


class tdf141244(UITestCase):

    def test_tdf141244(self):

        with self.ui_test.create_doc_in_start_center("calc"):

            calcDoc = self.xUITest.getTopFocusWindow()
            gridwin = calcDoc.getChild("grid_window")

            enter_text_to_cell(gridwin, "A1", '=DDE("soffice";"data1.ods";"sheet1.A1")')

            with self.ui_test.execute_dialog_through_command(".uno:EditLinks", close_button="close") as xDialog:

                xLinks = xDialog.getChild("TB_LINKS")
                self.assertEqual(1, len(xLinks.getChildren()))

                xChangeBtn = xDialog.getChild("CHANGE_SOURCE")

                with self.ui_test.execute_blocking_action(xChangeBtn.executeAction, args=('CLICK', ()), close_button="cancel") as dialog:
                    self.assertEqual("soffice", get_state_as_dict(dialog.getChild("app"))['Text'])
                    self.assertEqual("data1.ods", get_state_as_dict(dialog.getChild("file"))['Text'])
                    self.assertEqual("sheet1.A1", get_state_as_dict(dialog.getChild("category"))['Text'])

                    # tdf#141770: Without the fix in place, the cancel button wouldn't have worked here



# vim: set shiftwidth=4 softtabstop=4 expandtab:
