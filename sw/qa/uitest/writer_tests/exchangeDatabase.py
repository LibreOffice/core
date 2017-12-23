#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
import time
from uitest.uihelper.common import get_state_as_dict, type_text
from libreoffice.uno.propertyvalue import mkPropertyValues

class exchangeDB(UITestCase):

    def test_exchange_database(self):

        self.ui_test.create_doc_in_start_center("writer")

        self.ui_test.execute_dialog_through_command(".uno:ChangeDatabaseField")
        xExDBDlg = self.xUITest.getTopFocusWindow()
        xExDBBtn = xExDBDlg.getChild("close")
        xExDBBtn.executeAction("CLICK", tuple())

    def test_exchange_database2(self):

        self.ui_test.create_doc_in_start_center("writer")

        self.ui_test.execute_dialog_through_command(".uno:ChangeDatabaseField")
        xExDBDlg = self.xUITest.getTopFocusWindow()
        xTreelist = xExDBDlg.getChild("availablelb")

        xTreeEntry = xTreelist.getChild('0')                 #Available Databases
        xTreeEntry.executeAction("EXPAND", tuple())          #Click on the Bibliography

        xTreeEntry.executeAction("COLLAPSE", tuple())
        xTreeEntry.executeAction("EXPAND", tuple())

        xExDBBtn = xExDBDlg.getChild("close")
        xExDBBtn.executeAction("CLICK", tuple())



        self.ui_test.close_doc()
