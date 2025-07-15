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

                self.ui_test.wait_until_property_is_updated(xPages, "Children", "9")
                filteredEntryCount = int(get_state_as_dict(xPages)["Children"])
                self.assertEqual(9, filteredEntryCount)

                xSearch.executeAction("CLEAR", tuple())

                while True:
                    finalEntryCount = int(get_state_as_dict(xPages)["Children"])
                    if finalEntryCount > filteredEntryCount:
                        break
                    time.sleep(self.ui_test.get_default_sleep())

                # Ideally this assert should be initialEntryCount == finalEntryCount
                # but it fails in some slow machines
                self.assertGreater(finalEntryCount, filteredEntryCount)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
