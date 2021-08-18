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

class Tdf142395(UITestCase):

    def test_tdf142395(self):

        # Load file from Open dialog
        with self.ui_test.execute_dialog_through_command(".uno:Open", close_button="open") as xOpenDialog:

            xFileName = xOpenDialog.getChild("file_name")
            xFileName.executeAction("TYPE", mkPropertyValues({"TEXT": get_url_for_data_file("tdf142395.csv")}))


        xDialog = self.ui_test.wait_for_top_focus_window('TextImportCsvDialog')
        xSeparatedBy = xDialog.getChild("toseparatedby")
        xSeparatedBy.executeAction("CLICK", tuple())

        # Remove the text delimiter
        xTextDelimiter = xDialog.getChild("textdelimiter")
        xTextDelimiter.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
        xTextDelimiter.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))

        xOK = xDialog.getChild('ok')
        with self.ui_test.wait_until_component_loaded():
            self.ui_test.close_dialog_through_button(xOK)

        document = self.ui_test.get_component()

        self.assertEqual("a", get_cell_by_position(document, 0, 0, 0).getString())
        self.assertEqual(" b", get_cell_by_position(document, 0, 1, 0).getString())
        self.assertEqual(" c", get_cell_by_position(document, 0, 2, 0).getString())

        # Without the fix in place, this test would have failed with
        # AssertionError: ' ' != '晦餪'
        self.assertEqual(" ", get_cell_by_position(document, 0, 3, 0).getString())

        self.assertEqual("e", get_cell_by_position(document, 0, 0, 1).getString())
        self.assertEqual(" f", get_cell_by_position(document, 0, 1, 1).getString())
        self.assertEqual(" g", get_cell_by_position(document, 0, 2, 1).getString())
        self.assertEqual(" ", get_cell_by_position(document, 0, 3, 1).getString())


# vim: set shiftwidth=4 softtabstop=4 expandtab:
