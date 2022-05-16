# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.calc import enter_text_to_cell
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file
from uitest.uihelper.common import select_by_text
from libreoffice.uno.propertyvalue import mkPropertyValues
from org.libreoffice.unotest import systemPathToFileUrl
from tempfile import TemporaryDirectory
import os.path

from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file
from libreoffice.uno.propertyvalue import mkPropertyValues
from libreoffice.calc.document import get_cell_by_position


class tdf123877(UITestCase):

    def test_tdf123877(self):

        xFilePath = get_url_for_data_file("tdf123877.xlsx")

        with self.ui_test.load_file(xFilePath) as document:

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

                    with self.ui_test.execute_dialog_through_action(xCancel, "CLICK", close_button="ok"):
                    # Write error dialog is displayed
                        pass

# vim: set shiftwidth=4 softtabstop=4 expandtab:
