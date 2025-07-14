# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

import time

from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict

class tdf49895(UITestCase):

    def test_tdf49895(self):
        with self.ui_test.create_doc_in_start_center("writer"):

            with self.ui_test.execute_dialog_through_command(".uno:OptionsTreeDialog") as xDialog:

                xPages = xDialog.getChild("pages")
                initialEntryCount = int(get_state_as_dict(xPages)["Children"])
                self.assertTrue(initialEntryCount != 0)

                xSearch = xDialog.getChild("searchEntry")

                xSearch.executeAction("TYPE", mkPropertyValues({"TEXT":"View"}))

                # Wait for the search/filter op to be completed
                while True:
                    filteredEntryCount = int(get_state_as_dict(xPages)["Children"])
                    if filteredEntryCount != initialEntryCount:
                        break
                    time.sleep(self.ui_test.get_default_sleep())

                self.assertEqual(9, filteredEntryCount)

                xSearch.executeAction("CLEAR", tuple())

                # Wait for the search/filter op to be completed
                while True:
                    finalEntryCount = int(get_state_as_dict(xPages)["Children"])
                    if finalEntryCount != filteredEntryCount:
                        break
                    time.sleep(self.ui_test.get_default_sleep())

                self.assertEqual(initialEntryCount, finalEntryCount)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
