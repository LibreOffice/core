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
from libreoffice.uno.propertyvalue import mkPropertyValues


#Bug 139464 - change label of OK button to Move or Copy according to action selected

class okButtonLabel(UITestCase):
    def test_tdf139464_ok_button_label(self):

        with self.ui_test.create_doc_in_start_center("calc"):

            with self.ui_test.execute_dialog_through_command(".uno:Move") as xDialog:
                xOkButton = xDialog.getChild("ok")
                xCopyButton = xDialog.getChild("copy")

                self.assertEqual(get_state_as_dict(xCopyButton)['Text'], get_state_as_dict(xOkButton)['Text'])

            with self.ui_test.execute_dialog_through_command(".uno:Move") as xDialog:
                xOkButton = xDialog.getChild("ok")
                xCopyButton = xDialog.getChild("copy")
                xMoveButton = xDialog.getChild("move")

                self.assertEqual(get_state_as_dict(xMoveButton)['Text'], get_state_as_dict(xOkButton)['Text'])

                xCopyButton.executeAction("CLICK", tuple())

                self.assertEqual(get_state_as_dict(xCopyButton)['Text'], get_state_as_dict(xOkButton)['Text'])

                xMoveButton.executeAction("CLICK", tuple())

                self.assertEqual(get_state_as_dict(xMoveButton)['Text'], get_state_as_dict(xOkButton)['Text'])



# vim: set shiftwidth=4 softtabstop=4 expandtab:
