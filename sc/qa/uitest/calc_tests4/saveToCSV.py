# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

import os.path
from tempfile import TemporaryDirectory

from uitest.framework import UITestCase
from uitest.uihelper.calc import enter_text_to_cell
from uitest.uihelper.common import get_state_as_dict
from uitest.uihelper.common import select_by_text
from libreoffice.uno.propertyvalue import mkPropertyValues


class saveToCSV(UITestCase):

    def test_saveToCSVDialog(self):

        with TemporaryDirectory() as tempdir:
            xFilePath = os.path.join(tempdir, 'exportToCSV-tmp.csv')

            with self.ui_test.create_doc_in_start_center("calc"):

                calcDoc = self.xUITest.getTopFocusWindow()
                gridwin = calcDoc.getChild("grid_window")

                enter_text_to_cell(gridwin, "A1", "1")
                enter_text_to_cell(gridwin, "A2", "2")
                enter_text_to_cell(gridwin, "A3", "3")
                enter_text_to_cell(gridwin, "A4", "=SUM(A1:A3)")

                # Save the document
                with self.ui_test.execute_dialog_through_command(".uno:Save", close_button="") as xSaveDialog:
                    xFileName = xSaveDialog.getChild("file_name")
                    xFileName.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
                    xFileName.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
                    xFileName.executeAction("TYPE", mkPropertyValues({"TEXT": xFilePath}))
                    xFileTypeCombo = xSaveDialog.getChild("file_type")
                    select_by_text(xFileTypeCombo, "Text CSV (.csv)")
                    xOpen = xSaveDialog.getChild("open")

                    with self.ui_test.execute_dialog_through_action(xOpen, "CLICK", close_button="") as xWarnDialog:
                        # CSV confirmation dialog is displayed
                        xSave = xWarnDialog.getChild("btnYes")

                        with self.ui_test.execute_dialog_through_action(xSave, "CLICK") as xCsvDialog:
                            xFormulas = xCsvDialog.getChild("formulas")
                            xAsShown = xCsvDialog.getChild("asshown")
                            xFixedWidth = xCsvDialog.getChild("fixedwidth")
                            xQuoteAll = xCsvDialog.getChild("quoteall")
                            self.assertEqual("false", get_state_as_dict(xFormulas)['Selected'])
                            self.assertEqual("false", get_state_as_dict(xQuoteAll)['Selected'])
                            self.assertEqual("false", get_state_as_dict(xFixedWidth)['Selected'])
                            self.assertEqual("true", get_state_as_dict(xAsShown)['Selected'])

                            xFormulas.executeAction("CLICK", tuple())

            with open(xFilePath, "r") as f:
                lines = f.readlines()
                self.assertEqual("1", lines[0].strip())
                self.assertEqual("2", lines[1].strip())
                self.assertEqual("3", lines[2].strip())
                self.assertEqual("=SUM(A1:A3)", lines[3].strip())

# vim: set shiftwidth=4 softtabstop=4 expandtab:
