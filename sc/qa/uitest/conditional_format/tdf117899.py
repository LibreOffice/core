# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file
from libreoffice.calc.document import get_cell_by_position
from org.libreoffice.unotest import systemPathToFileUrl
from tempfile import TemporaryDirectory
import os.path

class Tdf117899(UITestCase):

  def execute_conditional_format_manager_dialog(self):

    self.ui_test.execute_dialog_through_command(".uno:ConditionalFormatManagerDialog")
    xCondFormatMgr = self.xUITest.getTopFocusWindow()

    aExpectedResult = 'Formula is $E3="нет"'
    xList = xCondFormatMgr.getChild("CONTAINER")
    self.assertEqual(1, len(xList.getChildren()))
    self.assertTrue(get_state_as_dict(xList.getChild('0'))['Text'].endswith(aExpectedResult))

    xOKBtn = xCondFormatMgr.getChild("ok")
    self.ui_test.close_dialog_through_button(xOKBtn)

  def test_tdf117899(self):
    with self.ui_test.load_file(get_url_for_data_file("tdf117899.ods")):

        self.execute_conditional_format_manager_dialog()

        self.xUITest.executeCommand(".uno:SelectAll")

        self.xUITest.executeCommand(".uno:Copy")

        with TemporaryDirectory() as tempdir:
            xFilePath = os.path.join(tempdir, "tdf117899-temp.ods")

            with self.ui_test.load_empty_file("writer"):

                self.xUITest.getTopFocusWindow()

                # Paste as an OLE spreadsheet
                formatProperty = mkPropertyValues({"SelectedFormat": 85})
                self.xUITest.executeCommandWithParameters(".uno:ClipboardFormatItems", formatProperty)

                # Save Copy as
                self.ui_test.execute_dialog_through_command(".uno:ObjectMenue?VerbID:short=-8")
                xDialog = self.xUITest.getTopFocusWindow()

                xFileName = xDialog.getChild("file_name")
                xFileName.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
                xFileName.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
                xFileName.executeAction("TYPE", mkPropertyValues({"TEXT": xFilePath}))

                xOpenBtn = xDialog.getChild("open")
                self.ui_test.close_dialog_through_button(xOpenBtn)

            with self.ui_test.load_file(systemPathToFileUrl(xFilePath)):

                xCalcDoc = self.xUITest.getTopFocusWindow()
                gridwin = xCalcDoc.getChild("grid_window")

                # Without the fix in place, this test would have failed here
                self.execute_conditional_format_manager_dialog()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
