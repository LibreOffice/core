# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
from com.sun.star.beans import PropertyValue
from org.libreoffice.unotest import systemPathToFileUrl
from uitest.uihelper.common import select_by_text
from tempfile import TemporaryDirectory
import os.path

#Bug 147086 - Password in .xlsx creates readonly sheet

class tdf147086(UITestCase):

    def test_tdf147086(self):

        with TemporaryDirectory() as tempdir:
            xFilePath = os.path.join(tempdir, "tdf147086_tmp.xlsx")

            with self.ui_test.create_doc_in_start_center("calc"):
                # Save the document
                with self.ui_test.execute_dialog_through_command(".uno:Save", close_button="") as xSaveDialog:
                    xFileName = xSaveDialog.getChild("file_name")
                    xFileName.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
                    xFileName.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
                    xFileName.executeAction("TYPE", mkPropertyValues({"TEXT": xFilePath}))
                    xFileTypeCombo = xSaveDialog.getChild("file_type")
                    select_by_text(xFileTypeCombo, "Excel 2007–365 (.xlsx)")
                    xPasswordCheckButton = xSaveDialog.getChild("password")
                    xPasswordCheckButton.executeAction("CLICK", tuple())
                    xOpen = xSaveDialog.getChild("open")

                    with self.ui_test.execute_dialog_through_action(xOpen, "CLICK", close_button="") as xPasswordDialog:
                        xReadonly = xPasswordDialog.getChild("readonly")
                        xReadonly.executeAction("CLICK", tuple())

                        xOk = xPasswordDialog.getChild("ok")
                        # XLSX confirmation dialog is displayed
                        with self.ui_test.execute_dialog_through_action(xOk, "CLICK", close_button="btnYes"):
                            pass

            with self.ui_test.load_file(systemPathToFileUrl(xFilePath), [PropertyValue(Name="Silent", Value=True)]) as document:

                self.assertTrue(document.isReadonly())

                self.xUITest.executeCommand(".uno:EditDoc")

                self.assertFalse(document.isReadonly())

# vim: set shiftwidth=4 softtabstop=4 expandtab:
