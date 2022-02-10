# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file
from libreoffice.uno.propertyvalue import mkPropertyValues
import datetime

class compareDocuments(UITestCase):

    def test_tdf130960(self):

        with self.ui_test.load_file(get_url_for_data_file("tdf130960.odt")) as writer_doc:

            xWriterDoc = self.xUITest.getTopFocusWindow()

            with self.ui_test.execute_dialog_through_command(".uno:CompareDocuments", close_button="") as xOpenDialog:
                xFileName = xOpenDialog.getChild("file_name")
                xFileName.executeAction("TYPE", mkPropertyValues({"TEXT": get_url_for_data_file("tdf130960_2.odt")}))
                xOpenBtn = xOpenDialog.getChild("open")

                # Close the dialog and open it again so the list of changes is updated
                with self.ui_test.execute_dialog_through_action(xOpenBtn, 'CLICK', close_button="close"):
                    pass

            with self.ui_test.execute_modeless_dialog_through_command(".uno:AcceptTrackedChanges", close_button="close") as xTrackDlg:
                changesList = xTrackDlg.getChild("writerchanges")

                text = "Unknown Author\t" + datetime.datetime.now().strftime("%m/%d/%Y")
                self.assertEqual(2, len(changesList.getChildren()))
                self.assertTrue(get_state_as_dict(changesList.getChild('0'))["Text"].startswith(text))
                self.assertTrue(get_state_as_dict(changesList.getChild('1'))["Text"].startswith(text))

# vim: set shiftwidth=4 softtabstop=4 expandtab:
