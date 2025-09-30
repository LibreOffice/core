# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file
from libreoffice.uno.propertyvalue import mkPropertyValues

class DataProvider(UITestCase):

    def test_brokenCSV(self):

        with self.ui_test.create_doc_in_start_center("calc"):

            # tdf#168497: without the fix in place, it would have crashed here
            with self.ui_test.execute_dialog_through_command(".uno:DataProvider") as xDialog:
                xBrowse = xDialog.getChild("browse")
                xApply = xDialog.getChild("apply")
                xProvider = xDialog.getChild("provider_lst")
                xURL = xDialog.getChild("ed_url")

                with self.ui_test.execute_blocking_action(
                        xBrowse.executeAction, args=('CLICK', ()), close_button="") as dialog:
                    xFileName = dialog.getChild("file_name")
                    xFileName.executeAction("TYPE", mkPropertyValues({"TEXT": get_url_for_data_file("tdf165658.csv")}))
                    xOpen = dialog.getChild("open")
                    xOpen.executeAction("CLICK", tuple())

                self.assertEqual(get_url_for_data_file("tdf165658.csv"), get_state_as_dict(xURL)['Text'])
                self.assertEqual("CSV", get_state_as_dict(xProvider)['DisplayText'])

                # tdf#165658: without the fix in place, it would have crashed here
                with self.ui_test.execute_blocking_action(
                        xApply.executeAction, args=('CLICK', ()), close_button="close") as dialog:
                    pass

# vim: set shiftwidth=4 softtabstop=4 expandtab:
