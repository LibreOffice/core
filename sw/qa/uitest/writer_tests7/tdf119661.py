# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
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

class tdf119661(UITestCase):

    def test_tdf119661(self):

        self.ui_test.create_doc_in_start_center("writer")

        self.ui_test.execute_dialog_through_command(".uno:InsertGraphic")

        xOpenDialog = self.xUITest.getTopFocusWindow()

        xFileName = xOpenDialog.getChild("file_name")
        xFileName.executeAction("TYPE", mkPropertyValues({"TEXT": get_url_for_data_file("LibreOffice.jpg")}))

        xLink = xOpenDialog.getChild("link")
        self.assertEqual("false", get_state_as_dict(xLink)['Selected'])

        xLink.executeAction("CLICK", tuple())

        xOpenBtn = xOpenDialog.getChild("open")
        xOpenBtn.executeAction("CLICK", tuple())

        #Confirmation dialog is displayed
        xWarnDialog = self.xUITest.getTopFocusWindow()
        xOK = xWarnDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOK)

        self.ui_test.execute_dialog_through_command(".uno:LinkDialog")

        xDialog = self.xUITest.getTopFocusWindow()

        sLinks = "TB_LINKS"
        xLinks = xDialog.getChild(sLinks)
        self.assertEqual(1, len(xLinks.getChildren()))

        sFileName = "FULL_FILE_NAME"
        xFileName = xDialog.getChild(sFileName)
        self.assertTrue(get_state_as_dict(xFileName)["Text"].endswith("/LibreOffice.jpg"))

        sBreakLink = "BREAK_LINK"
        xBreakLink = xDialog.getChild(sBreakLink)

        self.ui_test.execute_blocking_action(xBreakLink.executeAction,
            args=("CLICK", tuple()), dialog_element="yes")

        xClose = xDialog.getChild("close")
        self.ui_test.close_dialog_through_button(xClose)

        with TemporaryDirectory() as tempdir:
            xFilePath = os.path.join(tempdir, "tdf119661-tmp.odt")

            # Save Copy as
            self.ui_test.execute_dialog_through_command(".uno:SaveAs")
            xDialog = self.xUITest.getTopFocusWindow()

            xFileName = xDialog.getChild("file_name")
            xFileName.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
            xFileName.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
            xFileName.executeAction("TYPE", mkPropertyValues({"TEXT": xFilePath}))

            xOpenBtn = xDialog.getChild("open")
            self.ui_test.close_dialog_through_button(xOpenBtn)

            # Close the Writer document
            self.ui_test.close_doc()

            self.ui_test.load_file(systemPathToFileUrl(xFilePath))

            self.xUITest.executeCommand(".uno:LinkDialog")

            # Since the image is no longer linked, the link dialog is not open.
            # Without the fix in place, this dialog would have been opened
            xMainWin = self.xUITest.getTopFocusWindow()
            self.assertTrue(sLinks not in xMainWin.getChildren())
            self.assertTrue(sFileName not in xMainWin.getChildren())
            self.assertTrue(sBreakLink not in xMainWin.getChildren())
            self.assertTrue("writer_edit" in xMainWin.getChildren())

            self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
