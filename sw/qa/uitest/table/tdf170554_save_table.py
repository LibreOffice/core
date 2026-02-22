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
from org.libreoffice.unotest import systemPathToFileUrl
from tempfile import TemporaryDirectory
import os.path

class tdf170554(UITestCase):

    def test_tdf170554_save_table(self):

        with TemporaryDirectory() as tempdir:
            xFilePath = os.path.join(tempdir, 'tdf170554-tmp.odt')

            with self.ui_test.create_doc_in_start_center("writer") as document:

                with self.ui_test.execute_dialog_through_command(".uno:InsertTable") as xDialog:

                    xColSpin = xDialog.getChild("colspin")
                    xColSpin.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
                    xColSpin.executeAction("TYPE", mkPropertyValues({"TEXT": "2"}))

                    xRowSpin = xDialog.getChild("rowspin")
                    xRowSpin.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
                    xRowSpin.executeAction("TYPE", mkPropertyValues({"TEXT": "2"}))

                    self.assertEqual("2", get_state_as_dict(xColSpin)["Text"])
                    self.assertEqual("2", get_state_as_dict(xRowSpin)["Text"])

                tables = document.getTextTables()
                self.assertEqual(2, len(tables[0].getRows()))
                self.assertEqual(2, len(tables[0].getColumns()))

                # Without the fix in place, this test would have failed to save the document
                with self.ui_test.execute_dialog_through_command('.uno:SaveAs', close_button="open") as xDialog:
                    xFileName = xDialog.getChild('file_name')
                    xFileName.executeAction('TYPE', mkPropertyValues({'KEYCODE':'CTRL+A'}))
                    xFileName.executeAction('TYPE', mkPropertyValues({'KEYCODE':'BACKSPACE'}))
                    xFileName.executeAction('TYPE', mkPropertyValues({'TEXT': xFilePath}))

            with self.ui_test.load_file(systemPathToFileUrl(xFilePath)) as document2:

                tables = document2.getTextTables()
                self.assertEqual(2, len(tables[0].getRows()))
                self.assertEqual(2, len(tables[0].getColumns()))

# vim: set shiftwidth=4 softtabstop=4 expandtab:
