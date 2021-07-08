# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

from uitest.framework import UITestCase
from uitest.uihelper.common import get_url_for_data_file

class tdf126541(UITestCase):

    def test_tdf126541_GridVisibilityImportXlsx(self):
        # Import an ods file with 'Hide' global grid visibility setting.
        self.ui_test.load_file(get_url_for_data_file("tdf126541_GridOffGlobally.ods"))
        document = self.ui_test.get_component()
        controller = document.getCurrentController()
        grid = controller.ShowGrid
        self.assertEqual(grid, False)
        self.ui_test.close_doc()

        # Importing xlsx file should set the global grid visibility setting to 'Show'
        # Sheet based grid line visibility setting should not overwrite the global setting.
        self.ui_test.load_file(get_url_for_data_file("tdf126541_GridOff.xlsx"))
        document = self.ui_test.get_component()
        controller = document.getCurrentController()
        grid = controller.ShowGrid
        self.assertEqual(grid, True)
        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
