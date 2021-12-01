# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict
from uitest.uihelper.common import type_text

class tdf138822(UITestCase):

    def test_tdf138822(self):
        with self.ui_test.create_doc_in_start_center("calc"):

            calcDoc = self.xUITest.getTopFocusWindow()
            xPosWindow = calcDoc.getChild('pos_window')
            self.assertEqual('A1', get_state_as_dict(xPosWindow)['Text'])

            with self.ui_test.execute_modeless_dialog_through_command(".uno:DefineName", close_button="add"):
                pass

            xDefineNamesDialog = self.xUITest.getTopFocusWindow()

            xAddBtn = xDefineNamesDialog.getChild("add")
            self.assertEqual("false", get_state_as_dict(xAddBtn)['Enabled'])

            xEdit = xDefineNamesDialog.getChild("edit")
            type_text(xEdit, "rangeName")

            self.assertEqual("true", get_state_as_dict(xAddBtn)['Enabled'])

            self.ui_test.close_dialog_through_button(xAddBtn)

            xManageNamesDialog = self.xUITest.getTopFocusWindow()

            xNamesList = xManageNamesDialog.getChild('names')
            self.assertEqual(1, len(xNamesList.getChildren()))
            self.assertEqual(get_state_as_dict(xNamesList.getChild('0'))["Text"], "rangeName\t$Sheet1.$A$1\tDocument (Global)")

            xCancelBtn = xManageNamesDialog.getChild("cancel")
            self.ui_test.close_dialog_through_button(xCancelBtn)

            # Open the dialog again
            with self.ui_test.execute_modeless_dialog_through_command(".uno:DefineName") as xManageNamesDialog:

                xNamesList = xManageNamesDialog.getChild('names')

                # Without the fix in place, this test would have failed with
                # AssertionError: 0 != 1
                self.assertEqual(0, len(xNamesList.getChildren()))

# vim: set shiftwidth=4 softtabstop=4 expandtab:
