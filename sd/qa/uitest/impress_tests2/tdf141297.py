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
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file
from org.libreoffice.unotest import systemPathToFileUrl
from tempfile import TemporaryDirectory
import os.path

class tdf141297(UITestCase):

    def test_tdf141297(self):
        with TemporaryDirectory() as tempdir:
            xFilePath = os.path.join(tempdir, "tdf141297-tmp.odp")

            with self.ui_test.create_doc_in_start_center("impress"):

                xTemplateDlg = self.xUITest.getTopFocusWindow()
                xCancelBtn = xTemplateDlg.getChild("close")
                self.ui_test.close_dialog_through_button(xCancelBtn)

                with self.ui_test.execute_dialog_through_command(".uno:InsertGraphic", close_button="") as xOpenDialog:

                    xFileName = xOpenDialog.getChild("file_name")
                    xFileName.executeAction("TYPE", mkPropertyValues({"TEXT": get_url_for_data_file("LibreOffice.jpg")}))

                    xLink = xOpenDialog.getChild("link")
                    self.assertEqual("false", get_state_as_dict(xLink)['Selected'])

                    xLink.executeAction("CLICK", tuple())

                    xOpen = xOpenDialog.getChild("open")
                    #Confirmation dialog is displayed
                    with self.ui_test.execute_dialog_through_action(xOpen, "CLICK"):
                        pass

                with self.ui_test.execute_dialog_through_command(".uno:ManageLinks", close_button="close") as xDialog:

                    sLinks = "TB_LINKS"
                    xLinks = xDialog.getChild(sLinks)
                    self.assertEqual(1, len(xLinks.getChildren()))

                    sFileName = "FULL_FILE_NAME"
                    xFileName = xDialog.getChild(sFileName)
                    self.assertTrue(get_state_as_dict(xFileName)["Text"].endswith("/LibreOffice.jpg"))

                    sBreakLink = "BREAK_LINK"
                    xBreakLink = xDialog.getChild(sBreakLink)

                    with self.ui_test.execute_blocking_action(xBreakLink.executeAction,
                            args=("CLICK", tuple()), close_button="yes"):
                        pass

                    # Save Copy as
                    with self.ui_test.execute_dialog_through_command(".uno:SaveAs", close_button="open") as xDialog:

                        xFileName = xDialog.getChild("file_name")
                        xFileName.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
                        xFileName.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
                        xFileName.executeAction("TYPE", mkPropertyValues({"TEXT": xFilePath}))

            self.ui_test.wait_until_file_is_available(xFilePath)

            with self.ui_test.load_file(systemPathToFileUrl(xFilePath)):

                self.xUITest.executeCommand(".uno:ManageLinks")

                # Since the image is no longer linked, the link dialog is not open.
                # Without the fix in place, this dialog would have been opened
                xMainWin = self.xUITest.getTopFocusWindow()
                self.assertTrue(sLinks not in xMainWin.getChildren())
                self.assertTrue(sFileName not in xMainWin.getChildren())
                self.assertTrue(sBreakLink not in xMainWin.getChildren())
                self.assertTrue("impress_win" in xMainWin.getChildren())

# vim: set shiftwidth=4 softtabstop=4 expandtab:
