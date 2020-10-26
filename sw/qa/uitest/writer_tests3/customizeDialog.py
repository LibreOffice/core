# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

import time

from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict
from uitest.uihelper.common import select_pos

class ConfigureDialog(UITestCase):

    def test_open_ConfigureDialog_writer(self):

        self.ui_test.create_doc_in_start_center("writer")
        self.ui_test.execute_dialog_through_command(".uno:ConfigureDialog")
        xDialog = self.xUITest.getTopFocusWindow()

        xcancBtn = xDialog.getChild("cancel")
        xcancBtn.executeAction("CLICK", tuple())

        self.ui_test.close_doc()

    def test_search_filter(self):
        self.ui_test.create_doc_in_start_center("writer")
        self.ui_test.execute_dialog_through_command(".uno:ConfigureDialog")
        xDialog = self.xUITest.getTopFocusWindow()

        xfunc = xDialog.getChild("functions")
        xSearch = xDialog.getChild("searchEntry")

        initialEntryCount = get_state_as_dict(xfunc)["Children"]
        self.assertTrue(initialEntryCount != 0)

        xSearch.executeAction("SET", mkPropertyValues({"TEXT":"format"}))

        # Wait for the search/filter op to be completed
        timeout = time.time() + 1
        while time.time() < timeout:
            filteredEntryCount = get_state_as_dict(xfunc)["Children"]
            if filteredEntryCount != initialEntryCount:
                break
            time.sleep(0.1)

        self.assertTrue(filteredEntryCount < initialEntryCount)

        xSearch.executeAction("CLEAR", tuple())

        # Wait for the search/filter op to be completed
        timeout = time.time() + 1
        while time.time() < timeout:
            finalEntryCount = get_state_as_dict(xfunc)["Children"]
            if finalEntryCount != filteredEntryCount:
                break
            time.sleep(0.1)

        self.assertEqual(initialEntryCount, finalEntryCount)


        xcancBtn = xDialog.getChild("cancel")  #button Cancel
        xcancBtn.executeAction("CLICK", tuple())  #click the button

        self.ui_test.close_doc()

    def test_category_listbox(self):
        self.ui_test.create_doc_in_start_center("writer")
        self.ui_test.execute_dialog_through_command(".uno:ConfigureDialog")
        xDialog = self.xUITest.getTopFocusWindow()

        xFunc = xDialog.getChild("functions")
        xCategory = xDialog.getChild("commandcategorylist")

        initialEntryCount = get_state_as_dict(xFunc)["Children"]
        self.assertTrue(initialEntryCount != 0)

        select_pos(xCategory, "1")
        filteredEntryCount = get_state_as_dict(xFunc)["Children"]
        self.assertTrue(filteredEntryCount < initialEntryCount)

        select_pos(xCategory, "0")
        finalEntryCount = get_state_as_dict(xFunc)["Children"]
        self.assertEqual(initialEntryCount, finalEntryCount)

        xcancBtn = xDialog.getChild("cancel")  #button Cancel
        xcancBtn.executeAction("CLICK", tuple())  #click the button

        self.ui_test.close_doc()

    def test_tdf133862(self):
        self.ui_test.create_doc_in_start_center("writer")

        self.xUITest.executeCommand(".uno:InsertObjectStarMath")

        # Without the fix in place, calling customize dialog after inserting
        # a formula object would crash
        self.ui_test.execute_dialog_through_command(".uno:ConfigureDialog")
        xDialog = self.xUITest.getTopFocusWindow()

        xcancBtn = xDialog.getChild("cancel")
        xcancBtn.executeAction("CLICK", tuple())

        self.ui_test.close_doc()

    def test_gear_button_menu(self):
        self.ui_test.create_doc_in_start_center("writer")

        self.ui_test.execute_dialog_through_command(".uno:ConfigureDialog")
        def close_dialog(dlg):
            CancelBtn = dlg.getChild("cancel")
            self.ui_test.close_dialog_through_button(CancelBtn)

        # Open the New Menu Dialog with id = 0
        xDialog = self.xUITest.getTopFocusWindow()
        xmenugearbtn=xDialog.getChild("menugearbtn")
        def show_dialog0():
            xmenugearbtn.executeAction("OPENFROMLIST", mkPropertyValues({"POS": "0" }))
        self.ui_test.execute_blocking_action( action=show_dialog0, dialog_handler=close_dialog)

        # Open the Rename Menu Dialog with id = 2
        xDialog = self.xUITest.getTopFocusWindow()
        xmenugearbtn=xDialog.getChild("menugearbtn")
        def show_dialog2():
            xmenugearbtn.executeAction("OPENFROMLIST", mkPropertyValues({"POS": "2"}))
        self.ui_test.execute_blocking_action( action=show_dialog2, dialog_handler=close_dialog)

        xDialog = self.xUITest.getTopFocusWindow()
        xcancBtn = xDialog.getChild("cancel")
        self.ui_test.close_dialog_through_button(xcancBtn)

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
