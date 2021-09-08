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

class Td114878(UITestCase):

    def test_tdf114878(self):

        # Load file from Open dialog
        with self.ui_test.execute_dialog_through_command(".uno:Open", close_button="open") as xOpenDialog:

            xFileName = xOpenDialog.getChild("file_name")
            xFileName.executeAction("TYPE", mkPropertyValues({"TEXT": get_url_for_data_file("tdf114878.csv")}))

        xDialog = self.ui_test.wait_for_top_focus_window('TextImportCsvDialog')

        # First import the file with 'Evaluate Formulas' unchecked

        # Without the fix in place, this test would have failed with
        # Could not find child with id: evaluateformulas
        xEvalutateFormulas = xDialog.getChild("evaluateformulas")
        self.assertEqual('false', get_state_as_dict(xEvalutateFormulas)['Selected'])

        xOK = xDialog.getChild('ok')
        with self.ui_test.wait_until_component_loaded():
            self.ui_test.close_dialog_through_button(xOK)

        document = self.ui_test.get_component()

        self.assertEqual("=-3+5", get_cell_by_position(document, 0, 0, 0).getString())
        self.assertEqual('=TRUE()', get_cell_by_position(document, 0, 0, 1).getString())
        self.assertEqual('=4*10', get_cell_by_position(document, 0, 0, 2).getString())
        self.assertEqual('=SUM(A1:A3)', get_cell_by_position(document, 0, 0, 3).getString())

        self.ui_test.close_doc()

        # Load the same file again
        with self.ui_test.execute_dialog_through_command(".uno:Open", close_button="open") as xOpenDialog:

            xFileName = xOpenDialog.getChild("file_name")
            xFileName.executeAction("TYPE", mkPropertyValues({"TEXT": get_url_for_data_file("tdf114878.csv")}))

        xDialog = self.ui_test.wait_for_top_focus_window('TextImportCsvDialog')

        # Now import the file with 'Evaluate Formulas' checked
        xEvalutateFormulas = xDialog.getChild("evaluateformulas")
        xEvalutateFormulas.executeAction("CLICK", tuple())
        self.assertEqual('true', get_state_as_dict(xEvalutateFormulas)['Selected'])

        xOK = xDialog.getChild('ok')
        with self.ui_test.wait_until_component_loaded():
            self.ui_test.close_dialog_through_button(xOK)

        document = self.ui_test.get_component()

        self.assertEqual("2", get_cell_by_position(document, 0, 0, 0).getString())
        self.assertEqual('1', get_cell_by_position(document, 0, 0, 1).getString())
        self.assertEqual('40', get_cell_by_position(document, 0, 0, 2).getString())
        self.assertEqual('43', get_cell_by_position(document, 0, 0, 3).getString())

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
