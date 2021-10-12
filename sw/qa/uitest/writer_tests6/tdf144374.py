# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
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

#Bug 144374 - Writer: FILESAVE to DOCX as read-only with additional password protection for editing not working

class tdf144374(UITestCase):

   def test_tdf144374_DOCX(self):
        with self.ui_test.create_doc_in_start_center("writer"):
            with TemporaryDirectory() as tempdir:
                xFilePath = os.path.join(tempdir, "tdf144374-tmp.docx")

                # Save the document
                with self.ui_test.execute_dialog_through_command(".uno:Save", close_button="") as xSaveDialog:
                    xFileName = xSaveDialog.getChild("file_name")
                    xFileName.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
                    xFileName.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
                    xFileName.executeAction("TYPE", mkPropertyValues({"TEXT": xFilePath}))
                    xFileTypeCombo = xSaveDialog.getChild("file_type")
                    select_by_text(xFileTypeCombo, "Office Open XML Text (Transitional) (.docx)")
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

                # DOCX confirmation dialog is displayed
                xWarnDialog = self.xUITest.getTopFocusWindow()
                xSave = xWarnDialog.getChild("save")
                self.ui_test.close_dialog_through_button(xSave)

                self.ui_test.close_doc()

                with self.ui_test.load_file(systemPathToFileUrl(xFilePath)):
                    xWriterEdit = self.xUITest.getTopFocusWindow().getChild("writer_edit")
                    document = self.ui_test.get_component()

                    self.assertTrue(document.isReadonly())

                    #Without the fix in place, this dialog wouldn't have been displayed
                    with self.ui_test.execute_dialog_through_action(xWriterEdit, "TYPE", mkPropertyValues({"KEYCODE": "CTRL+SHIFT+M"})) as xDialog:
                        xPassword = xDialog.getChild("newpassEntry")
                        xPassword.executeAction("TYPE", mkPropertyValues({"TEXT": "password"}))

                    self.assertFalse(document.isReadonly())

# vim: set shiftwidth=4 softtabstop=4 expandtab:
