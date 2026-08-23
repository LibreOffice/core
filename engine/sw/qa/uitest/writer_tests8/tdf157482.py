# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file

class tdf157482(UITestCase):

    def set_security_warnings(self, enabled):
        with self.ui_test.execute_dialog_through_command('.uno:OptionsSecurityDialog', close_button="") as dialog:
            for name in ('whenpdf', 'whenprinting', 'savesenddocs', 'whensigning'):
                xBox = dialog.getChild(name)
                if get_state_as_dict(xBox)['Selected'] != enabled:
                    xBox.executeAction('CLICK', tuple())
                self.assertEqual(get_state_as_dict(xBox)["Selected"], enabled)

            xOkBtn = dialog.getChild('ok')
            xOkBtn.executeAction('CLICK', tuple())

    def test_tdf157482(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf157482.odt")) as document:
            try:
                self.set_security_warnings("true")
                self.xUITest.executeCommand('.uno:Save')
                controller = document.getCurrentController()
                self.assertTrue(controller.hasInfobar("securitywarn"))
                controller.removeInfobar("securitywarn")
                self.assertFalse(controller.hasInfobar("securitywarn"))

                with self.ui_test.execute_dialog_through_command('.uno:ExportToPDF', close_button="cancel") as xDialog:
                    self.assertTrue(controller.hasInfobar("securitywarn"))
                    controller.removeInfobar("securitywarn")
                    self.assertFalse(controller.hasInfobar("securitywarn"))

                with self.ui_test.execute_dialog_through_command(".uno:Signature", close_button="close"):
                    self.assertTrue(controller.hasInfobar("securitywarn"))
                    controller.removeInfobar("securitywarn")
                    self.assertFalse(controller.hasInfobar("securitywarn"))

            finally:
                self.set_security_warnings("false")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
