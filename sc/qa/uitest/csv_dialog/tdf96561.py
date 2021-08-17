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

class Tdf96561(UITestCase):

    def test_tdf96561(self):

        # Load file from Open dialog
        with self.ui_test.execute_dialog_through_command(".uno:Open", close_button="open") as xOpenDialog:

            xFileName = xOpenDialog.getChild("file_name")
            xFileName.executeAction("TYPE", mkPropertyValues({"TEXT": get_url_for_data_file("tdf96561.csv")}))

        xDialog = self.ui_test.wait_for_top_focus_window('TextImportCsvDialog')

        # Set text delimiter in case it's changed by another test
        xSeparatedBy = xDialog.getChild("toseparatedby")
        xSeparatedBy.executeAction("CLICK", tuple())

        xTextDelimiter = xDialog.getChild("textdelimiter")
        xTextDelimiter.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
        xTextDelimiter.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
        xTextDelimiter.executeAction("TYPE", mkPropertyValues({"TEXT": "\""}))

        xOK = xDialog.getChild('ok')
        with self.ui_test.wait_until_component_loaded():
            self.ui_test.close_dialog_through_button(xOK)

        document = self.ui_test.get_component()

        self.assertEqual('Campo con ";" en medio', get_cell_by_position(document, 0, 0, 0).getString())

        # Without the fix in place, this test would have failed with
        # AssertionError: '""; esta al """""principio del Campo' != 'Campo la tiene al final;"'
        self.assertEqual('""; esta al """""principio del Campo', get_cell_by_position(document, 0, 0, 1).getString())
        self.assertEqual('Campo la tiene al final;"', get_cell_by_position(document, 0, 0, 2).getString())

        for i in range(1,5):
            for j in range(0,3):
                self.assertEqual("1", get_cell_by_position(document, 0, i, j).getString())


# vim: set shiftwidth=4 softtabstop=4 expandtab:
