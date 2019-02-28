# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
import time
from uitest.uihelper.common import get_state_as_dict, type_text
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.debug import sleep

class exchangeDB(UITestCase):

    def test_exchange_database(self):

        self.ui_test.create_doc_in_start_center("writer")

        self.ui_test.execute_dialog_through_command(".uno:ChangeDatabaseField")
        xExDBDlg = self.xUITest.getTopFocusWindow()
        xCloseBtn = xExDBDlg.getChild("close")
        self.ui_test.close_dialog_through_button(xCloseBtn)

        self.ui_test.close_doc()

    def test_exchange_database2(self):

        self.ui_test.create_doc_in_start_center("writer")

        self.ui_test.execute_dialog_through_command(".uno:ChangeDatabaseField")
        xExDBDlg = self.xUITest.getTopFocusWindow()
        xTreelist = xExDBDlg.getChild("availablelb")

        xLabeldb = xExDBDlg.getChild("dbnameft")
        self.assertEqual(get_state_as_dict(xLabeldb)["Text"], "[None]")

        xTreeEntry = xTreelist.getChild('0')                 #Available Databases
        xTreeEntry.executeAction("EXPAND", tuple())          #Click on the Bibliography
        xTreeEntry.executeAction("COLLAPSE", tuple())
        xTreeEntry.executeAction("EXPAND", tuple())
        xTreeEntry2 = xTreeEntry.getChild('0')                 #Available Databases
        xTreeEntry2.executeAction("SELECT", tuple())          #Click on the biblio

        xDefineBtn = xExDBDlg.getChild("ok")
        xDefineBtn.executeAction("CLICK", tuple())

        self.ui_test.execute_dialog_through_command(".uno:ChangeDatabaseField")
        xExDBDlg = self.xUITest.getTopFocusWindow()
        xLabeldb = xExDBDlg.getChild("dbnameft")
        self.assertEqual(get_state_as_dict(xLabeldb)["Text"], "Bibliography.biblio")

        xCloseBtn = xExDBDlg.getChild("close")
        self.ui_test.close_dialog_through_button(xCloseBtn)

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
