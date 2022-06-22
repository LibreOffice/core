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
from uitest.uihelper.common import select_pos
from uitest.uihelper.common import get_url_for_data_file

class tdf128744(UITestCase):

   def test_tdf128744(self):
        # load the sample file
        with self.ui_test.load_file(get_url_for_data_file("tdf128744.docx")):

            # first try to unprotect Record Changes with an invalid password

            with self.ui_test.execute_dialog_through_command(".uno:SetDocumentProperties") as xDialog:
                xRecordChangesCheckbox = xDialog.getChild("recordchanges")
                self.assertEqual(get_state_as_dict(xRecordChangesCheckbox)["Selected"], "true")
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "3")     #tab Security
                xProtectBtn = xDialog.getChild("protect")

                # No close_button: click on the "Ok" inside to check the "Invalid password" infobox
                with self.ui_test.execute_blocking_action(xProtectBtn.executeAction, args=('CLICK', ()), close_button="") as xPasswordDialog:
                    self.assertEqual(get_state_as_dict(xPasswordDialog)["DisplayText"], "Enter Password")
                    xPassword = xPasswordDialog.getChild("pass1ed")
                    xPassword.executeAction("TYPE", mkPropertyValues({"TEXT": "bad password"}))
                    xOkBtn = xPasswordDialog.getChild("ok")
                    with self.ui_test.execute_blocking_action(xOkBtn.executeAction, args=('CLICK', ())) as xInfoBox:
                        # "Invalid password" infobox
                        self.assertEqual(get_state_as_dict(xInfoBox)["DisplayText"], 'Information')

            # now open the dialog again and read the properties, Record Changes checkbox is still enabled
            with self.ui_test.execute_dialog_through_command(".uno:SetDocumentProperties", close_button="cancel") as xDialog:
                xRecordChangesCheckbox = xDialog.getChild("recordchanges")
                self.assertEqual(get_state_as_dict(xRecordChangesCheckbox)["Selected"], "true")
                xResetBtn = xDialog.getChild("reset")
                xResetBtn.executeAction("CLICK", tuple())

            # unprotect Record Changes with the valid password "test"

            with self.ui_test.execute_dialog_through_command(".uno:SetDocumentProperties") as xDialog:
                xRecordChangesCheckbox = xDialog.getChild("recordchanges")
                self.assertEqual(get_state_as_dict(xRecordChangesCheckbox)["Selected"], "true")
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "3")     #tab Security
                xProtectBtn = xDialog.getChild("protect")

                with self.ui_test.execute_blocking_action(xProtectBtn.executeAction, args=('CLICK', ())) as xPasswordDialog:
                    self.assertEqual(get_state_as_dict(xPasswordDialog)["DisplayText"], "Enter Password")
                    xPassword = xPasswordDialog.getChild("pass1ed")
                    # give the correct password
                    xPassword.executeAction("TYPE", mkPropertyValues({"TEXT": "test"}))

            # now open the dialog again and read the properties, Record Changes checkbox is disabled now
            with self.ui_test.execute_dialog_through_command(".uno:SetDocumentProperties", close_button="cancel") as xDialog:
                xRecordChangesCheckbox = xDialog.getChild("recordchanges")
                self.assertEqual(get_state_as_dict(xRecordChangesCheckbox)["Selected"], "false")
                xResetBtn = xDialog.getChild("reset")
                xResetBtn.executeAction("CLICK", tuple())

# vim: set shiftwidth=4 softtabstop=4 expandtab:
