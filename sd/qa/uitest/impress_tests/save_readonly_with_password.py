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
from org.libreoffice.unotest import systemPathToFileUrl
from uitest.uihelper.common import select_by_text
from tempfile import TemporaryDirectory
import os.path

class save_readonly_with_password(UITestCase):

    #Bug 145511 - FILESAVE to PPTX as read-only with additional password protection for editing not working
   def test_save_to_pptx(self):

        with TemporaryDirectory() as tempdir:
            xFilePath = os.path.join(tempdir, "tdf144374-tmp.pptx")

            with self.ui_test.create_doc_in_start_center("impress"):
                xTemplateDlg = self.xUITest.getTopFocusWindow()
                xCancelBtn = xTemplateDlg.getChild("close")
                self.ui_test.close_dialog_through_button(xCancelBtn)

                # Save the document
                with self.ui_test.execute_dialog_through_command(".uno:Save", close_button="") as xSaveDialog:
                    xFileName = xSaveDialog.getChild("file_name")
                    xFileName.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
                    xFileName.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
                    xFileName.executeAction("TYPE", mkPropertyValues({"TEXT": xFilePath}))
                    xFileTypeCombo = xSaveDialog.getChild("file_type")
                    select_by_text(xFileTypeCombo, "Office Open XML Presentation (.pptx)")
                    xPasswordCheckButton = xSaveDialog.getChild("password")
                    xPasswordCheckButton.executeAction("CLICK", tuple())
                    xOpen = xSaveDialog.getChild("open")

                    with self.ui_test.execute_dialog_through_action(xOpen, "CLICK", close_button="") as xPasswordDialog:
                        xReadonly = xPasswordDialog.getChild("readonly")
                        xReadonly.executeAction("CLICK", tuple())
                        xNewPassword = xPasswordDialog.getChild("newpassroEntry")
                        xNewPassword.executeAction("TYPE", mkPropertyValues({"TEXT": "password"}))
                        xConfirmPassword = xPasswordDialog.getChild("confirmropassEntry")
                        xConfirmPassword.executeAction("TYPE", mkPropertyValues({"TEXT": "password"}))

                        xOk = xPasswordDialog.getChild("ok")
                        # PPTX confirmation dialog is displayed
                        with self.ui_test.execute_dialog_through_action(xOk, "CLICK", close_button="save"):
                            pass

            self.ui_test.wait_until_file_is_available(xFilePath)

            with self.ui_test.load_file(systemPathToFileUrl(xFilePath)) as document:

                self.assertTrue(document.isReadonly())

                # Without the fix in place, this dialog wouldn't have been displayed
                with self.ui_test.execute_dialog_through_command(".uno:EditDoc") as xDialog:
                    xPassword = xDialog.getChild("newpassEntry")
                    xPassword.executeAction("TYPE", mkPropertyValues({"TEXT": "password"}))

                self.assertFalse(document.isReadonly())

   def test_save_to_odp(self):

        with TemporaryDirectory() as tempdir:
            xFilePath = os.path.join(tempdir, "readonly_with_password_tmp.odp")

            with self.ui_test.create_doc_in_start_center("impress"):
                xTemplateDlg = self.xUITest.getTopFocusWindow()
                xCancelBtn = xTemplateDlg.getChild("close")
                self.ui_test.close_dialog_through_button(xCancelBtn)

                # Save the document
                with self.ui_test.execute_dialog_through_command(".uno:Save", close_button="") as xSaveDialog:
                    xFileName = xSaveDialog.getChild("file_name")
                    xFileName.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
                    xFileName.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
                    xFileName.executeAction("TYPE", mkPropertyValues({"TEXT": xFilePath}))
                    xPasswordCheckButton = xSaveDialog.getChild("password")
                    xPasswordCheckButton.executeAction("CLICK", tuple())
                    xOpen = xSaveDialog.getChild("open")

                    with self.ui_test.execute_dialog_through_action(xOpen, "CLICK") as xPasswordDialog:
                        xReadonly = xPasswordDialog.getChild("readonly")
                        xReadonly.executeAction("CLICK", tuple())
                        xNewPassword = xPasswordDialog.getChild("newpassroEntry")
                        xNewPassword.executeAction("TYPE", mkPropertyValues({"TEXT": "password"}))
                        xConfirmPassword = xPasswordDialog.getChild("confirmropassEntry")
                        xConfirmPassword.executeAction("TYPE", mkPropertyValues({"TEXT": "password"}))

            self.ui_test.wait_until_file_is_available(xFilePath)

            with self.ui_test.load_file(systemPathToFileUrl(xFilePath)) as document:

                self.assertTrue(document.isReadonly())

                with self.ui_test.execute_dialog_through_command(".uno:EditDoc") as xDialog:
                    xPassword = xDialog.getChild("newpassEntry")
                    xPassword.executeAction("TYPE", mkPropertyValues({"TEXT": "password"}))

                self.assertFalse(document.isReadonly())

# vim: set shiftwidth=4 softtabstop=4 expandtab:
