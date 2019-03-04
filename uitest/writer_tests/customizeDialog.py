# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict
import time
from uitest.debug import sleep

class ConfigureDialog(UITestCase):

    def test_open_ConfigureDialog_writer(self):

        self.ui_test.create_doc_in_start_center("writer")
        self.ui_test.execute_dialog_through_command(".uno:ConfigureDialog")
        xDialog = self.xUITest.getTopFocusWindow()

        xcancBtn = xDialog.getChild("cancel")
        xcancBtn.executeAction("CLICK", tuple())

        self.ui_test.close_doc()

    def test_search_filter(self):
        self.ui_test.create_doc_in_start_center("writer")
        self.ui_test.execute_dialog_through_command(".uno:ConfigureDialog")
        xDialog = self.xUITest.getTopFocusWindow()

        xfunc = xDialog.getChild("functions")
        xSearch = xDialog.getChild("searchEntry")

        initialEntryCount = get_state_as_dict(xfunc)["Children"]
        self.assertTrue(initialEntryCount is not 0)

        xSearch.executeAction("SET", mkPropertyValues({"TEXT":"format"}))

        # Wait for the search/filter op to be completed
        time.sleep(1)

        filteredEntryCount = get_state_as_dict(xfunc)["Children"]
        self.assertTrue(filteredEntryCount < initialEntryCount)

        xSearch.executeAction("CLEAR", tuple())

        # Wait for the search/filter op to be completed
        time.sleep(1)

        finalEntryCount = get_state_as_dict(xfunc)["Children"]
        self.assertEqual(initialEntryCount, finalEntryCount)


        xcancBtn = xDialog.getChild("cancel")  #button Cancel
        xcancBtn.executeAction("CLICK", tuple())  #click the button

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
