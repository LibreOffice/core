# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict
from libreoffice.uno.propertyvalue import mkPropertyValues
#uitest sc - move/copy sheet dialog

class moveCopySheet(UITestCase):
    def test_copy_move_sheet(self):
        with self.ui_test.create_doc_in_start_center("calc") as document:
            #default - 1 sheet; select the sheet (is selected), dialog move/copy sheet
            with self.ui_test.execute_dialog_through_command(".uno:Move") as xDialog:
                #new name = newName
                newName = xDialog.getChild("newName")
                newName.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
                newName.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
                newName.executeAction("TYPE", mkPropertyValues({"TEXT":"newName"}))
            #verify, the file has 2 sheets; first one "newName" is selected
            self.assertEqual(document.Sheets.getCount(), 2)
            # dialog move/copy sheet ; Move is selected; select -move to end position - ; New Name = moveName
            with self.ui_test.execute_dialog_through_command(".uno:Move") as xDialog:
                insertBefore = xDialog.getChild("insertBefore")

                xTreeEntry = insertBefore.getChild('2')
                xTreeEntry.executeAction("SELECT", tuple())

                newName = xDialog.getChild("newName")
                self.assertEqual(get_state_as_dict(newName)["Text"], "newName")
                newName.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
                newName.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
                newName.executeAction("TYPE", mkPropertyValues({"TEXT":"moveName"}))
            # Verify, the file has 2 sheets; first one is "Sheet1" ; second one is "moveName"
            self.assertEqual(document.Sheets.getCount(), 2)

            self.assertEqual(document.Sheets[0].Name, "Sheet1")
            self.assertEqual(document.Sheets[1].Name, "moveName")

            # Verify that the cancel button does not do anything
            with self.ui_test.execute_dialog_through_command(".uno:Move", close_button="cancel"):
                pass

            self.assertEqual(document.Sheets.getCount(), 2)
            self.assertEqual(document.Sheets[0].Name, "Sheet1")
            self.assertEqual(document.Sheets[1].Name, "moveName")

            #Check copy option
            with self.ui_test.execute_dialog_through_command(".uno:Move") as xDialog:
                xCopy = xDialog.getChild("copy")
                xCopy.executeAction("CLICK", tuple())
                sheetName = "moveName_2"
                newName = xDialog.getChild("newName")
                self.assertEqual(get_state_as_dict(newName)["Text"], sheetName)


            self.assertEqual(document.Sheets.getCount(), 3)
            self.assertEqual(document.Sheets[0].Name, sheetName)
            self.assertEqual(document.Sheets[1].Name, "Sheet1")
            self.assertEqual(document.Sheets[2].Name, "moveName")

    #tdf#139464 Set OK button label to selected action: Move or Copy
    def test_tdf139464_move_sheet(self):
        with self.ui_test.create_doc_in_start_center("calc"):
            with self.ui_test.execute_dialog_through_command(".uno:Move") as xDialog:
                xOkButton = xDialog.getChild("ok")
                xCopyButton = xDialog.getChild("copy")
                self.assertEqual(get_state_as_dict(xCopyButton)['Text'], get_state_as_dict(xOkButton)['Text'])
            with self.ui_test.execute_dialog_through_command(".uno:Move") as xDialog:
                xOkButton = xDialog.getChild("ok")
                xCopyButton = xDialog.getChild("copy")
                xMoveButton = xDialog.getChild("move")
                self.assertEqual(get_state_as_dict(xMoveButton)['Text'], get_state_as_dict(xOkButton)['Text'])
                xCopyButton.executeAction("CLICK", tuple())
                self.assertEqual(get_state_as_dict(xCopyButton)['Text'], get_state_as_dict(xOkButton)['Text'])
                xMoveButton.executeAction("CLICK", tuple())
                self.assertEqual(get_state_as_dict(xMoveButton)['Text'], get_state_as_dict(xOkButton)['Text'])


# vim: set shiftwidth=4 softtabstop=4 expandtab:
