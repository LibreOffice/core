# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import get_url_for_data_file
from uitest.uihelper.common import select_by_text
from libreoffice.uno.propertyvalue import mkPropertyValues
from tempfile import TemporaryDirectory
import os.path

class hashIncompatible(UITestCase):

    def test_hashIncompatible(self):

        with TemporaryDirectory() as tempdir:
            xFilePath = os.path.join(tempdir, "hashIncompatible-temp.ods")

            with self.ui_test.load_file(get_url_for_data_file("hashIncompatible.xlsx")) as document:

                # Save the XLSX document as ODS with a sheet protected with an unsupported hash format
                with self.ui_test.execute_dialog_through_command(".uno:SaveAs", close_button="") as xSaveDialog:
                    xFileName = xSaveDialog.getChild("file_name")
                    xFileName.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
                    xFileName.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
                    xFileName.executeAction("TYPE", mkPropertyValues({"TEXT": xFilePath}))
                    xFileTypeCombo = xSaveDialog.getChild("file_type")
                    select_by_text(xFileTypeCombo, "ODF Spreadsheet (.ods)")

                    xOpen = xSaveDialog.getChild("open")

                    with self.ui_test.execute_dialog_through_action(xOpen, "CLICK", close_button="") as xRetypePasswordDialog:
                        # hash error dialog is still displayed (only disabled for the recovery file)
                        xCancel = xRetypePasswordDialog.getChild("cancel")

                        with self.ui_test.execute_dialog_through_action(xCancel, "CLICK"):
                            # Write error dialog is displayed
                            pass

                # Check the document is not created
                self.assertFalse(os.path.isfile(xFilePath))

                # Now check it can be saved to ODS using the correct password
                with self.ui_test.execute_dialog_through_command(".uno:SaveAs", close_button="") as xSaveDialog:
                    xFileName = xSaveDialog.getChild("file_name")
                    xFileName.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
                    xFileName.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
                    xFileName.executeAction("TYPE", mkPropertyValues({"TEXT": xFilePath}))
                    xFileTypeCombo = xSaveDialog.getChild("file_type")
                    select_by_text(xFileTypeCombo, "ODF Spreadsheet (.ods)")

                    xOpen = xSaveDialog.getChild("open")

                    with self.ui_test.execute_dialog_through_action(xOpen, "CLICK") as xRetypePasswordDialog:
                        xRetypeBtn = xRetypePasswordDialog.getChild("button")

                        with self.ui_test.execute_blocking_action(
                                xRetypeBtn.executeAction, args=('CLICK', ())) as xPasswordDialog:
                            xNewPassEntry = xPasswordDialog.getChild("newpassEntry")
                            xConfirmPassEntry = xPasswordDialog.getChild("confirmpassEntry")
                            xNewPassEntry.executeAction("TYPE", mkPropertyValues({"TEXT": "Hello"}))
                            xConfirmPassEntry.executeAction("TYPE", mkPropertyValues({"TEXT": "Hello"}))

            self.assertTrue(os.path.isfile(xFilePath))

# vim: set shiftwidth=4 softtabstop=4 expandtab:
