# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file
from libreoffice.uno.propertyvalue import mkPropertyValues
from libreoffice.calc.document import get_cell_by_position

class Tdf60468(UITestCase):

    def test_tdf60468(self):

        # Load file from Open dialog
        self.ui_test.execute_dialog_through_command(".uno:Open")

        xOpenDialog = self.xUITest.getTopFocusWindow()
        xFileName = xOpenDialog.getChild("file_name")
        xFileName.executeAction("TYPE", mkPropertyValues({"TEXT": get_url_for_data_file("tdf60468.csv")}))

        xOpenBtn = xOpenDialog.getChild("open")
        xOpenBtn.executeAction("CLICK", tuple())

        xDialog = self.xUITest.getTopFocusWindow()
        self.assertEqual('true', get_state_as_dict(xDialog.getChild("tab"))['Selected'])
        self.assertEqual('true', get_state_as_dict(xDialog.getChild("comma"))['Selected'])
        self.assertEqual('true', get_state_as_dict(xDialog.getChild("semicolon"))['Selected'])
        self.assertEqual('1', get_state_as_dict(xDialog.getChild("fromrow"))['Text'])

        xOK = xDialog.getChild('ok')
        self.ui_test.close_dialog_through_button(xOK)

        document = self.ui_test.get_component()

        # tdf#142040: Without the fix in place, this test would have failed with
        # AssertionError: 'head1' != ''
        for i in range(3):
            self.assertEqual("head" + str(i + 1), get_cell_by_position(document, 0, i, 0).getString())

        self.assertEqual("value1.1\nvalue1.2", get_cell_by_position(document, 0, 0, 1).getString())

        # Without the fix in place, this test would have failed with
        # AssertionError: 'value2.1\n\tvalue2.2\nvalue2.3' != 'value2.1\n\tvalue2.2'
        self.assertEqual("value2.1\n\tvalue2.2\nvalue2.3", get_cell_by_position(document, 0, 1, 1).getString())
        self.assertEqual("value3", get_cell_by_position(document, 0, 2, 1).getString())

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
