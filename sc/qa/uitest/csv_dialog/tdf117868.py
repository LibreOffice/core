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

class Td117868(UITestCase):

    def test_tdf117868(self):

        # Load file from Open dialog
        with self.ui_test.execute_dialog_through_command(".uno:Open", close_button="open") as xOpenDialog:

            xFileName = xOpenDialog.getChild("file_name")
            xFileName.executeAction("TYPE", mkPropertyValues({"TEXT": get_url_for_data_file("tdf117868.csv")}))

        xDialog = self.ui_test.wait_for_top_focus_window('TextImportCsvDialog')

        # Set text delimiter in case it's changed by another test
        xSeparatedBy = xDialog.getChild("toseparatedby")
        xSeparatedBy.executeAction("CLICK", tuple())

        # Without the fix in place, this test would haved failed with
        # AssertionError: 'true' != 'false'
        self.assertEqual('true', get_state_as_dict(xDialog.getChild("other"))['Selected'])
        self.assertEqual('false', get_state_as_dict(xDialog.getChild("tab"))['Selected'])
        self.assertEqual('false', get_state_as_dict(xDialog.getChild("comma"))['Selected'])
        self.assertEqual('false', get_state_as_dict(xDialog.getChild("semicolon"))['Selected'])

        self.assertEqual('1', get_state_as_dict(xDialog.getChild("fromrow"))['Text'])

        xInputOther = xDialog.getChild("inputother")
        self.assertEqual("|", get_state_as_dict(xInputOther)['Text'])

        xOK = xDialog.getChild('ok')
        with self.ui_test.wait_until_component_loaded():
            self.ui_test.close_dialog_through_button(xOK)

        document = self.ui_test.get_component()

        self.assertEqual("LETTER", get_cell_by_position(document, 0, 0, 1).getString())
        self.assertEqual("ANIMAL", get_cell_by_position(document, 0, 1, 1).getString())
        self.assertEqual('a', get_cell_by_position(document, 0, 0, 2).getString())
        self.assertEqual('aardvark', get_cell_by_position(document, 0, 1, 2).getString())
        self.assertEqual('b', get_cell_by_position(document, 0, 0, 3).getString())
        self.assertEqual('bear', get_cell_by_position(document, 0, 1, 3).getString())
        self.assertEqual('c', get_cell_by_position(document, 0, 0, 4).getString())
        self.assertEqual('cow', get_cell_by_position(document, 0, 1, 4).getString())

# vim: set shiftwidth=4 softtabstop=4 expandtab:
