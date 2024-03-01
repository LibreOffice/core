# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict
from tempfile import TemporaryDirectory
import os.path

class PasswordPolicy(UITestCase):
   def test_save_with_password_policy(self):
       # with a Password Policy configuration that enforces one lowercase, one uppercase and one number and at least 8 chars.
       with self.ui_test.set_config('/org.openoffice.Office.Common/Security/Scripting/PasswordPolicy', r"^(?=.*[a-z])(?=.*[A-Z])(?=.*\d)[a-zA-Z\d]{8,}$"):
           with TemporaryDirectory() as tempdir:
               xFilePath = os.path.join(tempdir, "password_save_with_policy_test.odt")

               with self.ui_test.create_doc_in_start_center("writer"):
                   # Save the document
                   with self.ui_test.execute_dialog_through_command(".uno:Save", close_button="") as xSaveDialog:
                       xFileName = xSaveDialog.getChild("file_name")
                       xFileName.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
                       xFileName.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
                       xFileName.executeAction("TYPE", mkPropertyValues({"TEXT": xFilePath}))
                       xPasswordCheckButton = xSaveDialog.getChild("password")
                       xPasswordCheckButton.executeAction("CLICK", tuple())
                       xOpen = xSaveDialog.getChild("open")

                       hasExitedEarly = True
                       with self.ui_test.execute_dialog_through_action(xOpen, "CLICK") as xPasswordDialog:
                           xNewPassword = xPasswordDialog.getChild("newpassEntry")
                           xNewPassword.executeAction("TYPE", mkPropertyValues({"TEXT": "password"}))
                           xConfirmPassword = xPasswordDialog.getChild("confirmpassEntry")
                           xConfirmPassword.executeAction("TYPE", mkPropertyValues({"TEXT": "password"}))

                           # check if the current state of the password entry reflects the rejection of the password.
                           xIndicator = xPasswordDialog.getChild("newpassIndicator")
                           IndicatorIsVisible = get_state_as_dict(xIndicator)["Visible"] == "true"
                           self.assertTrue(IndicatorIsVisible)

                           # the dialog shouldn't exit on this OK click.
                           xOk = xPasswordDialog.getChild("ok")
                           xOk.executeAction("CLICK", tuple())

                           # enter a password that will fit the password policy
                           xNewPassword.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
                           xNewPassword.executeAction("TYPE", mkPropertyValues({"TEXT": "Zxcvbnm123"}))
                           xConfirmPassword.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
                           xConfirmPassword.executeAction("TYPE", mkPropertyValues({"TEXT": "Zxcvbnm123"}))

                           # check the state is as expected now
                           IndicatorIsVisible = get_state_as_dict(xIndicator)["Visible"] == "true"
                           self.assertFalse(IndicatorIsVisible)
                           hasExitedEarly = False

                       # test if the dialog accepted the password that didn't fit the policy
                       self.assertFalse(hasExitedEarly)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
