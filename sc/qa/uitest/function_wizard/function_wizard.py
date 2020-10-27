# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from libreoffice.uno.propertyvalue import mkPropertyValues

from uitest.framework import UITestCase

class FunctionWizardTest(UITestCase):
    # tdf#98427
    def test_open_function_wizard(self):
        self.ui_test.create_doc_in_start_center("calc")

        self.ui_test.execute_modeless_dialog_through_command(".uno:FunctionDialog")

        xFunctionDlg = self.xUITest.getTopFocusWindow()

        xArrayChkBox = xFunctionDlg.getChild("array")
        xArrayChkBox.executeAction("CLICK", tuple())

        xCancelBtn = xFunctionDlg.getChild("cancel")
        self.ui_test.close_dialog_through_button(xCancelBtn)

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
