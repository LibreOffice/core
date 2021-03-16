# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict

#envaddresspage.ui

class WriterInsertEnvelope(UITestCase):

    def test_insert_envelope(self):
        self.ui_test.create_doc_in_start_center("writer")
        document = self.ui_test.get_component()
        self.ui_test.execute_dialog_through_command(".uno:InsertEnvelope")

        xDialog = self.xUITest.getTopFocusWindow()

        xAddrTxt= xDialog.getChild("addredit")
        xSenderTxt = xDialog.getChild("senderedit")
        xSenderCheckBox = xDialog.getChild("sender")

        xAddrTxt.executeAction("SELECT", mkPropertyValues({"FROM": "1", "TO": "200"}))
        xAddrTxt.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
        xAddrTxt.executeAction("TYPE", mkPropertyValues({"TEXT":"Address"}))

        xSenderTxt.executeAction("SELECT", mkPropertyValues({"FROM": "1", "TO": "200"}))
        xSenderTxt.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
        xSenderTxt.executeAction("TYPE", mkPropertyValues({"TEXT":"Sender"}))

        xInsBtn = xDialog.getChild("user")
        xInsBtn.executeAction("CLICK", tuple())

        self.ui_test.execute_dialog_through_command(".uno:InsertEnvelope")
        xDialog = self.xUITest.getTopFocusWindow()
        xAddrTxt= xDialog.getChild("addredit")
        xSenderTxt = xDialog.getChild("senderedit")
        self.assertEqual(get_state_as_dict(xAddrTxt)["Text"], "Address")
        self.assertEqual(get_state_as_dict(xSenderTxt)["Text"], "Sender")

        xcancelBtn = xDialog.getChild("cancel")
        self.ui_test.close_dialog_through_button(xcancelBtn)

        self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab:
