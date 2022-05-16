# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase


class FunctionWizardTest(UITestCase):
    # tdf#98427
    def test_open_function_wizard(self):
        with self.ui_test.create_doc_in_start_center("calc"):

            with self.ui_test.execute_modeless_dialog_through_command(".uno:FunctionDialog", close_button="cancel") as xFunctionDlg:


                xArrayChkBox = xFunctionDlg.getChild("array")
                xArrayChkBox.executeAction("CLICK", tuple())



# vim: set shiftwidth=4 softtabstop=4 expandtab:
