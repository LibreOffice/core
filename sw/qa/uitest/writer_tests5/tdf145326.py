# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file
from libreoffice.uno.propertyvalue import mkPropertyValues
from org.libreoffice.unotest import systemPathToFileUrl
from tempfile import TemporaryDirectory
import os.path

class Tdf145326(UITestCase):

    def test_tdf145326(self):

        with TemporaryDirectory() as tempdir:
            xFilePath = os.path.join(tempdir, "tdf145326-temp.odt")

            with self.ui_test.execute_dialog_through_command(".uno:Open", close_button="") as xOpenDialog:

                xFileName = xOpenDialog.getChild("file_name")
                xFileName.executeAction("TYPE", mkPropertyValues({"TEXT": get_url_for_data_file("tdf145326.odt")}))

                xOpenBtn = xOpenDialog.getChild("open")
                # Update all links dialog
                with self.ui_test.wait_until_component_loaded():
                    with self.ui_test.execute_blocking_action(xOpenBtn.executeAction, args=('CLICK', ()), close_button="yes"):
                        pass

            with self.ui_test.execute_dialog_through_command(".uno:LinkDialog", close_button="close") as xDialog:

                sLinks = "TB_LINKS"
                xLinks = xDialog.getChild(sLinks)
                self.assertEqual(1, len(xLinks.getChildren()))

                sFileName = "FULL_FILE_NAME"
                xFileName = xDialog.getChild(sFileName)
                self.assertTrue(get_state_as_dict(xFileName)["Text"].endswith("SAmple odp.ods"))

                sBreakLink = "BREAK_LINK"
                xBreakLink = xDialog.getChild(sBreakLink)

                with self.ui_test.execute_blocking_action(xBreakLink.executeAction,
                        args=("CLICK", tuple()), close_button="yes"):
                    pass

            # Save Copy as
            with self.ui_test.execute_dialog_through_command('.uno:SaveAs', close_button="open") as xDialog:
                xFileName = xDialog.getChild('file_name')
                xFileName.executeAction('TYPE', mkPropertyValues({'KEYCODE':'CTRL+A'}))
                xFileName.executeAction('TYPE', mkPropertyValues({'KEYCODE':'BACKSPACE'}))
                xFileName.executeAction('TYPE', mkPropertyValues({'TEXT': xFilePath}))

            self.ui_test.close_doc()

            with self.ui_test.load_file(systemPathToFileUrl(xFilePath)) as doc2:

                self.xUITest.executeCommand(".uno:LinkDialog")

                # Since the image is no longer linked, the link dialog is not open.
                # Without the fix in place, this dialog would have been opened
                xMainWin = self.xUITest.getTopFocusWindow()
                self.assertTrue(sLinks not in xMainWin.getChildren())
                self.assertTrue(sFileName not in xMainWin.getChildren())
                self.assertTrue(sBreakLink not in xMainWin.getChildren())
                self.assertTrue("writer_edit" in xMainWin.getChildren())

                self.assertEqual(doc2.TextTables.getCount(), 1)
                table = doc2.getTextTables()[0]

                self.assertEqual(len(table.getRows()), 7)
                self.assertEqual("10,000", table.getCellByName("D2").getString())
                self.assertEqual("20,000", table.getCellByName("D3").getString())
                self.assertEqual("5,000", table.getCellByName("D4").getString())
                self.assertEqual("7,000", table.getCellByName("D5").getString())
                self.assertEqual("5,000", table.getCellByName("D6").getString())
                self.assertEqual("7,000", table.getCellByName("D7").getString())

# vim: set shiftwidth=4 softtabstop=4 expandtab:
