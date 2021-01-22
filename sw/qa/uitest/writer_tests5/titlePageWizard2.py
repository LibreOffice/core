# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file
from libreoffice.uno.propertyvalue import mkPropertyValues

# This tests both an edge cases, and some more realistic situations.
class tdf138907(UITestCase):
    def test_tdf138907(self):
        self.ui_test.load_file(get_url_for_data_file("tdf138907_titlePageDialog.odt"))
        document = self.ui_test.get_component()

        # Test an undefined situation - try to modify pages beyond the end of the document.

        #dialog Title Page
        self.ui_test.execute_dialog_through_command(".uno:TitlePageDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        #set restart page number to 2. With this doc, it defaults to resetting to 1.
        xRestartNumbering = xDialog.getChild("NF_RESTART_NUMBERING")
        xRestartNumbering.executeAction("UP", tuple()) # restart numbering at 2

        #Convert three pages to title/index pages starting at non-existing page twenty.
        xPageCount = xDialog.getChild("NF_PAGE_COUNT")
        for _ in range(0,2):
            xPageCount.executeAction("UP", tuple())
        xUseStartingPage = xDialog.getChild("RB_PAGE_START")
        xUseStartingPage.executeAction("CLICK", tuple())
        xStartingPage = xDialog.getChild("NF_PAGE_START")
        for _ in range(0,19):
            xStartingPage.executeAction("UP", tuple()) #Start at mythical page 20.

        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        # Nothing should happen when modifying pages that don't exist.
        # Just a page break, without a valid restart page number on page 2
        self.assertEqual(document.CurrentController.PageCount, 5)
        Paragraphs = document.Text.createEnumeration()
        Para1 = Paragraphs.nextElement()
        self.assertEqual(Para1.String, "6")
        self.assertEqual(Para1.PageDescName, "First Page")
        Para2 = Paragraphs.nextElement()
        self.assertEqual(Para2.String, "7")
        self.assertEqual(Para2.PageDescName, None)
        Para3 = Paragraphs.nextElement()
        self.assertEqual(Para3.String, "8")
        self.assertEqual(Para3.PageDescName, None)
        Para4 = Paragraphs.nextElement()
        self.assertEqual(Para4.String, "9")
        self.assertEqual(Para4.PageDescName, None)
        Para5 = Paragraphs.nextElement()
        self.assertEqual(Para5.String, "10")
        self.assertEqual(Para5.PageDescName, None)


        #dialog Title Page
        self.ui_test.execute_dialog_through_command(".uno:TitlePageDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        #set restart page number to 1 - which is the default.
        #set restart title page to 1 - the current value for this document is 6.
        xRestartNumbering = xDialog.getChild("NF_SET_PAGE_NUMBER")
        print(xDialog.getChildren())
        for _ in range(0,5):
            xRestartNumbering.executeAction("DOWN", tuple()) # restart title numbering at 1
        #Insert two title/index pages at beginning of the document.
        newPages = xDialog.getChild("RB_INSERT_NEW_PAGES")
        newPages.executeAction("CLICK", tuple())
        xPageCount = xDialog.getChild("NF_PAGE_COUNT")
        for _ in range(0,1):
            xPageCount.executeAction("UP", tuple())

        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        Paragraphs = document.Text.createEnumeration()
        Para1 = Paragraphs.nextElement()
        self.assertEqual(Para1.String, "")
        self.assertEqual(Para1.PageDescName, "First Page")
        Para2 = Paragraphs.nextElement()
        self.assertEqual(Para2.String, "")
        self.assertEqual(Para2.PageDescName, "Index")
        Para3 = Paragraphs.nextElement()
        self.assertEqual(Para3.String, "1")
        self.assertEqual(Para3.PageDescName, "Landscape")
        Para4 = Paragraphs.nextElement()
        self.assertEqual(Para4.String, "2")
        Para5 = Paragraphs.nextElement()
        self.assertEqual(Para5.String, "3")
        Para6 = Paragraphs.nextElement()
        self.assertEqual(Para6.String, "4")
        Para7 = Paragraphs.nextElement()
        self.assertEqual(Para7.String, "5")

        #Now test replacing several pages with title and index styles

        #dialog Title Page
        self.ui_test.execute_dialog_through_command(".uno:TitlePageDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        print(xDialog.getChildren())
        #Convert four pages to title/index pages starting at page one.
        xPageCount = xDialog.getChild("NF_PAGE_COUNT")
        for _ in range(0,3):
            xPageCount.executeAction("DOWN", tuple())  #reset to 1 first
        for _ in range(0,3):
            xPageCount.executeAction("UP", tuple())

        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        Paragraphs = document.Text.createEnumeration()
        Para1 = Paragraphs.nextElement()
        self.assertEqual(Para1.String, "")
        self.assertEqual(Para1.PageDescName, "First Page")
        Para2 = Paragraphs.nextElement()
        self.assertEqual(Para2.String, "")
        self.assertEqual(Para2.PageDescName, "Index")
        Para3 = Paragraphs.nextElement()
        self.assertEqual(Para3.String, "3")
        self.assertEqual(Para3.PageDescName, "Index")
        Para4 = Paragraphs.nextElement()
        self.assertEqual(Para4.String, "4")
        self.assertEqual(Para4.PageDescName, "Index")
        Para5 = Paragraphs.nextElement()
        self.assertEqual(Para5.String, "1")
        self.assertEqual(Para5.PageDescName, "Landscape")
        Para6 = Paragraphs.nextElement()
        self.assertEqual(Para6.String, "2")
        Para7 = Paragraphs.nextElement()
        self.assertEqual(Para7.String, "3")

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
