# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
from org.libreoffice.unotest import systemPathToFileUrl
from uitest.uihelper.common import get_state_as_dict
from uitest.uihelper.common import select_by_text
from uitest.uihelper.common import select_pos
from uitest.uihelper.common import type_text
from tempfile import TemporaryDirectory
import os.path

#Bug 97899 - FILEOPEN: Cannot format some numbering of DOCX unless with the text

class tdf97899(UITestCase):

    def test_tdf97899(self):

        with TemporaryDirectory() as tempdir:
            xFilePath = os.path.join(tempdir, "tdf97899-tmp.docx")

            with self.ui_test.create_doc_in_start_center("writer"):
                xWriterDoc = self.xUITest.getTopFocusWindow()
                xWriterEdit = xWriterDoc.getChild("writer_edit")

                # Create an Ordered List
                type_text(xWriterEdit, "a")
                xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "RETURN"}))
                type_text(xWriterEdit, "b")
                xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "RETURN"}))
                type_text(xWriterEdit, "c")

                self.xUITest.executeCommand(".uno:SelectAll")
                self.xUITest.executeCommand(".uno:DefaultNumbering")

                # Save the document
                with self.ui_test.execute_dialog_through_command(".uno:Save", close_button="open") as xSaveDialog:
                    xFileName = xSaveDialog.getChild("file_name")
                    xFileName.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
                    xFileName.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
                    xFileName.executeAction("TYPE", mkPropertyValues({"TEXT": xFilePath}))
                    xFileTypeCombo = xSaveDialog.getChild("file_type")
                    select_by_text(xFileTypeCombo, "Office Open XML Text (Transitional) (.docx)")

                # DOCX confirmation dialog is displayed
                xWarnDialog = self.xUITest.getTopFocusWindow()
                xSave = xWarnDialog.getChild("save")
                self.ui_test.close_dialog_through_button(xSave)

            with self.ui_test.load_file(systemPathToFileUrl(xFilePath)):
                xWriterDoc = self.xUITest.getTopFocusWindow()
                xWriterEdit = xWriterDoc.getChild("writer_edit")

                xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "LEFT"}))

                self.xUITest.executeCommand(".uno:Bold")

                # Check the font style of the numbering
                with self.ui_test.execute_dialog_through_command(".uno:FontDialog", close_button="cancel") as xDialog:
                    xTabs = xDialog.getChild("tabcontrol")
                    select_pos(xTabs, "0")

                    xWestFontStyle = xDialog.getChild("weststylelb-cjk")
                    self.assertEqual(get_state_as_dict(xWestFontStyle)["Text"], "Bold")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
