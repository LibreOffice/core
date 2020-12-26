# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict
from libreoffice.uno.propertyvalue import mkPropertyValues
import org.libreoffice.unotest
import pathlib

def get_url_for_data_file(file_name):
    return pathlib.Path(org.libreoffice.unotest.makeCopyFromTDOC(file_name)).as_uri()

# This tests the Format->Title Page wizard, specifically the reset page number portion.
class tdf138907(UITestCase):
    def test_tdf138907(self):
        self.ui_test.load_file(get_url_for_data_file("tdf138907_titlePageDialog.odt"))
        document = self.ui_test.get_component()

        # Confirm the starting state. Just a page break, without a valid restart page number on page 2
        self.assertEqual(document.CurrentController.PageCount, 2)
        text = document.Text.String.replace('\r\n', '\n')
        self.assertEqual(text[0:1], "5")
        self.assertEqual(text[2:3], "6")

        #dialog Title Page
        self.ui_test.execute_dialog_through_command(".uno:TitlePageDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        #accept defaults (which includes a page number reset to "1" for page 2) and OK without making any changes.
        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        # This correctly reset the starting page number for page 2 as "1".
        # It wasn't persistent across round-trips though
        text = document.Text.String.replace('\r\n', '\n')
        self.assertEqual(text[0:1], "5")
        self.assertEqual(text[2:3], "1")
        #add assert here that page 2 is specified with page style break as "Landscape" to make it persistent.

        #re-run dialog Title Page
        self.ui_test.execute_dialog_through_command(".uno:TitlePageDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        #accept defaults and OK without making any changes.
        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        # Without this fix, re-running the wizard was failing with the title page restarting at page 1.
        text = document.Text.String.replace('\r\n', '\n')
        self.assertEqual(text[0:1], "5")
        self.assertEqual(text[2:3], "1")

        #Note: 3 virtual pages, including blank, even page seen in book view
        self.assertEqual(document.CurrentController.PageCount, 3)

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
