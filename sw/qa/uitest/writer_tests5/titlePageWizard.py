# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file
from libreoffice.uno.propertyvalue import mkPropertyValues

# This tests the Format->Title Page wizard, specifically the reset page number portion,
# replacing some pages with title pages,
# inserting pages in the middle of the document,
# and inserting at the very end of the document.
class tdf138907(UITestCase):
    def test_tdf138907(self):
        self.ui_test.load_file(get_url_for_data_file("tdf138907_titlePageDialog.odt"))
        document = self.ui_test.get_component()

        # Confirm the starting state. Just a page break, without a valid restart page number on page 2
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
        #set restart page number to 2. With this doc, it defaults to resetting to 1.
        xRestartNumbering = xDialog.getChild("NF_RESTART_NUMBERING")
        xRestartNumbering.executeAction("UP", tuple()) # restart numbering at 2
        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        Paragraphs = document.Text.createEnumeration()
        Para1 = Paragraphs.nextElement()
        self.assertEqual(Para1.String, "6")
        self.assertEqual(Para1.PageDescName, "First Page")
        self.assertEqual(Para2.String, "2")
        Para2 = Paragraphs.nextElement()
        # Without this fix, there was no PageDescName specified, just Landscape as default.
        self.assertEqual(Para2.PageDescName, "Landscape")

        #re-run dialog Title Page
        self.ui_test.execute_dialog_through_command(".uno:TitlePageDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        #accept defaults and OK without making any changes.
        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        # Without this fix, re-running the wizard was failing with the title page restarting at page 2.
        Paragraphs = document.Text.createEnumeration()
        Para1 = Paragraphs.nextElement()
        self.assertEqual(Para1.String, "6")
        self.assertEqual(Para1.PageDescName, "First Page")
        Para2 = Paragraphs.nextElement()
        self.assertEqual(Para2.String, "2")
        self.assertEqual(Para2.PageDescName, "Landscape")

        #Note: 6 virtual pages, including blank, even page seen in book view
        self.assertEqual(document.CurrentController.PageCount, 6)

        #Now test replacing several pages with title and index styles

        #dialog Title Page
        self.ui_test.execute_dialog_through_command(".uno:TitlePageDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        print(xDialog.getChildren())
        #Convert three pages to title/index pages starting at page two.
        xPageCount = xDialog.getChild("NF_PAGE_COUNT")
        for _ in range(0,2):
            xPageCount.executeAction("UP", tuple())
        xUseStartingPage = xDialog.getChild("RB_PAGE_START")
        xUseStartingPage.executeAction("CLICK", tuple())
        xStartingPage = xDialog.getChild("NF_PAGE_START")
        xStartingPage.executeAction("UP", tuple()) #Start at page 2.

        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        Paragraphs = document.Text.createEnumeration()
        Para1 = Paragraphs.nextElement()
        self.assertEqual(Para1.String, "6")
        self.assertEqual(Para1.PageDescName, "First Page")
        # Without the fix, the following results are all off by one.
        Para2 = Paragraphs.nextElement()
        self.assertEqual(Para2.String, "6")
        self.assertEqual(Para2.PageDescName, "First Page")
        Para3 = Paragraphs.nextElement()
        self.assertEqual(Para3.String, "7")
        self.assertEqual(Para3.PageDescName, "Index")
        Para4 = Paragraphs.nextElement()
        self.assertEqual(Para4.String, "8")
        self.assertEqual(Para4.PageDescName, "Index")
        Para5 = Paragraphs.nextElement()
        self.assertEqual(Para5.String, "2")
        self.assertEqual(Para5.PageDescName, "Landscape")

        #Now test inserting at the end of the document

        #dialog Title Page
        self.ui_test.execute_dialog_through_command(".uno:TitlePageDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        print(xDialog.getChildren())
        #Insert three title/index pages at the end of the document (plus a content page).
        newPages = xDialog.getChild("RB_INSERT_NEW_PAGES")
        newPages.executeAction("CLICK", tuple())
        xPageCount = xDialog.getChild("NF_PAGE_COUNT")
        for _ in range(0,2):
            xPageCount.executeAction("UP", tuple())
        xUseStartingPage = xDialog.getChild("RB_PAGE_START")
        xUseStartingPage.executeAction("CLICK", tuple())
        xStartingPage = xDialog.getChild("NF_PAGE_START")
        for _ in range(0,18):
            xStartingPage.executeAction("UP", tuple()) #Start at mythical page 20.

        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        # Without the fix, the pages were being inserted before the last page.
        text = document.Text.String.replace('\r\n', '\n')
        self.assertEqual(text[0:1], "6")
        self.assertEqual(text[2:3], "6")
        self.assertEqual(text[4:5], "7")
        self.assertEqual(text[6:7], "8")
        # Without the fix, the new pages were inserted before the last page.
        self.assertFalse("\n" in text[8:9])
        #Note: 13 total virtual pages, including four blanks, as seen in book view
        self.assertEqual(document.CurrentController.PageCount, 13)

        #Now test inserting in the middle of the document

        #dialog Title Page
        self.ui_test.execute_dialog_through_command(".uno:TitlePageDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        print(xDialog.getChildren())
        #Insert three title/index pages starting at page 2.
        newPages = xDialog.getChild("RB_INSERT_NEW_PAGES")
        newPages.executeAction("CLICK", tuple())
        xPageCount = xDialog.getChild("NF_PAGE_COUNT")
        for _ in range(0,2):
            xPageCount.executeAction("UP", tuple())
        xUseStartingPage = xDialog.getChild("RB_PAGE_START")
        xUseStartingPage.executeAction("CLICK", tuple())
        xStartingPage = xDialog.getChild("NF_PAGE_START")
        for _ in range(0,10):
            xStartingPage.executeAction("DOWN", tuple()) #Reset to page 1
        xStartingPage.executeAction("UP", tuple()) #Start at page 2.

        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        # Without first re-calculating the layout, the styles were applied to the wrong pages.
        Paragraphs = document.Text.createEnumeration()
        Para1 = Paragraphs.nextElement()
        self.assertEqual(Para1.String, "6")
        self.assertEqual(Para1.PageDescName, "First Page")
        # The next three pages are the ones that were just inserted.
        Para2 = Paragraphs.nextElement()
        self.assertEqual(Para2.String, "")
        self.assertEqual(Para2.PageDescName, "First Page")
        Para3 = Paragraphs.nextElement()
        self.assertEqual(Para3.String, "")
        self.assertEqual(Para3.PageDescName, "Index")
        Para4 = Paragraphs.nextElement()
        self.assertEqual(Para4.String, "")
        self.assertEqual(Para4.PageDescName, "Index")
        # A bit of a quirk is that the style of the first page after the
        # title page is still First Page - so that is used as the Normal page style.
        # OK - this is a bit of a strange workflow, so just accept that.
        Para5 = Paragraphs.nextElement()
        self.assertEqual(Para5.String, "6")
        self.assertEqual(Para5.PageDescName, "First Page")
        Para6 = Paragraphs.nextElement()
        self.assertEqual(Para6.String, "7")
        self.assertEqual(Para6.PageDescName, "Index")
        Para7 = Paragraphs.nextElement()
        self.assertEqual(Para7.String, "8")
        self.assertEqual(Para7.PageDescName, "Index")
        Para8 = Paragraphs.nextElement()
        self.assertEqual(Para8.String, "2")
        self.assertEqual(Para8.PageDescName, "Landscape")
        Para9 = Paragraphs.nextElement()
        self.assertEqual(Para9.String, "")
        self.assertEqual(Para9.PageDescName, "First Page")
        Para10 = Paragraphs.nextElement()
        self.assertEqual(Para10.String, "")
        self.assertEqual(Para10.PageDescName, "Index")
        Para11 = Paragraphs.nextElement()
        self.assertEqual(Para11.String, "")
        self.assertEqual(Para11.PageDescName, "Index")
        # The quirk resets this extra content page to the "style after First page == First Page"
        Para12 = Paragraphs.nextElement()
        self.assertEqual(Para12.String, "")
        self.assertEqual(Para12.PageDescName, "First Page")
        #Note: 17 total virtual pages, including five blanks, as seen in book view
        self.assertEqual(document.CurrentController.PageCount, 17)

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
