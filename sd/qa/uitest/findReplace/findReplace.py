# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import select_pos
from uitest.uihelper.calc import enter_text_to_cell
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file, type_text

class findReplace(UITestCase):
    def test_find_impress(self):
        impress_doc = self.ui_test.load_file(get_url_for_data_file("findReplace.odp"))
        document = self.ui_test.get_component()
        # print(dir(document))
        # xPages= document.CurrentController.getCurrentPage().Number
        # print(xPages)

        # check current slide is 1
        self.assertEqual(document.CurrentController.getCurrentPage().Number, 1)

        # search for string "second"
        self.ui_test.execute_modeless_dialog_through_command(".uno:SearchDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        searchterm = xDialog.getChild("searchterm")
        searchterm.executeAction("TYPE", mkPropertyValues({"TEXT":"second"}))  #2nd slide
        xsearch = xDialog.getChild("search")
        xsearch.executeAction("CLICK", tuple())

        # verify we moved to slide 2
        self.assertEqual(document.CurrentController.getCurrentPage().Number, 2)

        # search for string "third"
        searchterm.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
        searchterm.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
        searchterm.executeAction("TYPE", mkPropertyValues({"TEXT":"third"}))
        xsearch.executeAction("CLICK", tuple())

        #verify we moved to slide 3
        self.assertEqual(document.CurrentController.getCurrentPage().Number, 3)  #3rd slide

        # close the dialog
        xcloseBtn = xDialog.getChild("close")
        self.ui_test.close_dialog_through_button(xcloseBtn)

        # now open dialog and verify find="third" (remember last value);
        # replace value with "First" (click match case) with word "Replace"
        # click twice the Replace button, check "Replace first first"

        # open the dialog again
        self.ui_test.execute_modeless_dialog_through_command(".uno:SearchDialog")
        xDialog = self.xUITest.getTopFocusWindow()

        # verify search string is still "third" from previous search
        searchterm = xDialog.getChild("searchterm")
        self.assertEqual(get_state_as_dict(searchterm)["Text"], "third")

        # replace it with "First"
        searchterm.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
        searchterm.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
        searchterm.executeAction("TYPE", mkPropertyValues({"TEXT":"First"}))

        # click "match case"
        matchcase = xDialog.getChild("matchcase")
        matchcase.executeAction("CLICK", tuple())  #click match case

        # set the replace string to "Replace"
        replaceterm = xDialog.getChild("replaceterm")
        replaceterm.executeAction("TYPE", mkPropertyValues({"TEXT":"Replace"})) #replace textbox

        # hit replace button 2 times
        replace = xDialog.getChild("replace")
        replace.executeAction("CLICK", tuple())
        replace.executeAction("CLICK", tuple())   #click twice Replace button (one selects, second replaces)

        # close and reopen the dialog, because of bug 122788
        xcloseBtn = xDialog.getChild("close")
        self.ui_test.close_dialog_through_button(xcloseBtn)
        self.ui_test.execute_modeless_dialog_through_command(".uno:SearchDialog")
        xDialog = self.xUITest.getTopFocusWindow()

        # now replace first (uncheck match case) with word "aaa" - click once Replace All button, check "Replace aaa aaa"
        matchcase = xDialog.getChild("matchcase")
        matchcase.executeAction("CLICK", tuple())  # uncheck match case

        replaceterm = xDialog.getChild("replaceterm")
        replaceterm.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
        replaceterm.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
        replaceterm.executeAction("TYPE", mkPropertyValues({"TEXT":"aaa"}))
        replaceall = xDialog.getChild("replaceall")
        replaceall.executeAction("CLICK", tuple()) # click on replace all button
        xcloseBtn = xDialog.getChild("close")
        self.ui_test.close_dialog_through_button(xcloseBtn) #close the dialog

        # go to second page
        self.ui_test.execute_modeless_dialog_through_command(".uno:SearchDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        searchterm = xDialog.getChild("searchterm")
        searchterm.executeAction("TYPE", mkPropertyValues({"TEXT":"second"}))  #2nd slide
        xsearch = xDialog.getChild("search")
        xsearch.executeAction("CLICK", tuple())
        xcloseBtn = xDialog.getChild("close")
        self.ui_test.close_dialog_through_button(xcloseBtn)
        self.assertEqual(document.CurrentController.getCurrentPage().Number, 2)
        #now check if text "Replace aaa aaa" is on first slide
        self.ui_test.execute_modeless_dialog_through_command(".uno:SearchDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        searchterm = xDialog.getChild("searchterm")
        backsearch = xDialog.getChild("backsearch")
        searchterm.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
        searchterm.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
        searchterm.executeAction("TYPE", mkPropertyValues({"TEXT":"Replace aaa aaa"}))
        backsearch.executeAction("CLICK", tuple())
        #verify
        self.assertEqual(document.CurrentController.getCurrentPage().Number, 1)  #1st slide

        xcloseBtn = xDialog.getChild("close")
        self.ui_test.close_dialog_through_button(xcloseBtn)

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
