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

# This tests the Format->Title Page wizard, specifically inserting extra pages.
class tdf138907(UITestCase):
    def test_tdf138907(self):
        self.ui_test.load_file(get_url_for_data_file("tdf138907_titlePageDialog.odt"))
        document = self.ui_test.get_component()

        # Confirm the starting state. Just a page break, without a valid restart page number on page 2
        self.assertEqual(document.CurrentController.PageCount, 2)
        text = document.Text.String.replace('\r\n', '\n')
        self.assertEqual(text[0:1], "5")
        self.assertEqual(text[2:3], "6")
        Paragraphs = document.Text.createEnumeration()
        Para1 = Paragraphs.nextElement()
        self.assertEqual(Para1.PageDescName, "First Page")
        Para2 = Paragraphs.nextElement()
        self.assertEqual(Para2.PageDescName, None)

        #dialog Title Page
        self.ui_test.execute_dialog_through_command(".uno:TitlePageDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        print(xDialog.getChildren())
        #Insert title/index pages at the end of the document.
        newPages = xDialog.getChild("RB_INSERT_NEW_PAGES")
        newPages.executeAction("CLICK", tuple())
        xPageCount = xDialog.getChild("NF_PAGE_COUNT")
        for _ in range(0,2):
            xPageCount.executeAction("UP", tuple())
        xUseStartingPage = xDialog.getChild("RB_PAGE_START")
        xUseStartingPage.executeAction("CLICK", tuple())
        xStartingPage = xDialog.getChild("NF_PAGE_START")
        for _ in range(0,8):
            xStartingPage.executeAction("UP", tuple()) #Start at page 10.

        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        # Without the fix, the pages were being inserted before the last page.
        # NOTE: there are still LOTS of problems here that still need fixing.
        text = document.Text.String.replace('\r\n', '\n')
        self.assertEqual(text[0:1], "5")
        self.assertFalse("\n" in text[2:3])

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
