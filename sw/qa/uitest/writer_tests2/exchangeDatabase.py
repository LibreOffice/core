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

class exchangeDB(UITestCase):

    def test_exchange_database(self):

        with self.ui_test.create_doc_in_start_center("writer"):

            with self.ui_test.execute_dialog_through_command(".uno:ChangeDatabaseField", close_button="close"):
                pass


    def test_exchange_database2(self):

        with self.ui_test.create_doc_in_start_center("writer"):

            with self.ui_test.execute_dialog_through_command(".uno:ChangeDatabaseField") as xExDBDlg:
                xTreelist = xExDBDlg.getChild("availablelb")

                xLabeldb = xExDBDlg.getChild("dbnameft")
                self.assertEqual(get_state_as_dict(xLabeldb)["Text"], "[None]")

                xTreeEntry = xTreelist.getChild('0')                 #Available Databases
                xTreeEntry.executeAction("EXPAND", tuple())          #Click on the Bibliography
                xTreeEntry.executeAction("COLLAPSE", tuple())
                xTreeEntry.executeAction("EXPAND", tuple())
                xTreeEntry2 = xTreeEntry.getChild('0')                 #Available Databases
                xTreeEntry2.executeAction("SELECT", tuple())          #Click on the biblio

            with self.ui_test.execute_dialog_through_command(".uno:ChangeDatabaseField", close_button="close") as xExDBDlg:
                xLabeldb = xExDBDlg.getChild("dbnameft")
                self.assertEqual(get_state_as_dict(xLabeldb)["Text"], "Bibliography.biblio")


# vim: set shiftwidth=4 softtabstop=4 expandtab:
