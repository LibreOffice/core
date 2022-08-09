# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

import time

from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict
from uitest.uihelper.common import select_pos

class ConfigureDialog(UITestCase):

    def test_search_filter(self):
        with self.ui_test.create_doc_in_start_center("writer"):
            with self.ui_test.execute_dialog_through_command(".uno:ConfigureDialog", close_button="cancel") as xDialog:

                xfunc = xDialog.getChild("functions")
                xSearch = xDialog.getChild("searchEntry")

                initialEntryCount = get_state_as_dict(xfunc)["Children"]
                self.assertTrue(initialEntryCount != 0)

                xSearch.executeAction("TYPE", mkPropertyValues({"TEXT":"format"}))

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

    def test_category_listbox(self):
        with self.ui_test.create_doc_in_start_center("writer"):
            with self.ui_test.execute_dialog_through_command(".uno:ConfigureDialog", close_button="cancel") as xDialog:

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

    def test_tdf133862(self):
        with self.ui_test.create_doc_in_start_center("writer"):

            self.xUITest.executeCommand(".uno:InsertObjectStarMath")

            # Without the fix in place, calling customize dialog after inserting
            # a formula object would crash
            with self.ui_test.execute_dialog_through_command(".uno:ConfigureDialog", close_button="cancel"):
                pass

    def test_gear_button_menu(self):
        with self.ui_test.create_doc_in_start_center("writer"):

            with self.ui_test.execute_dialog_through_command(".uno:ConfigureDialog", close_button="cancel") as xDialog:

                # Open the New Menu Dialog with id = 0
                xmenugearbtn=xDialog.getChild("menugearbtn")
                with self.ui_test.execute_blocking_action(
                        xmenugearbtn.executeAction, args=("OPENFROMLIST", mkPropertyValues({"POS": "0"})), close_button="cancel"):
                    pass

                # Open the Rename Menu Dialog with id = 2
                with self.ui_test.execute_blocking_action(
                        xmenugearbtn.executeAction, args=("OPENFROMLIST", mkPropertyValues({"POS": "2"})), close_button="cancel"):
                    pass

# vim: set shiftwidth=4 softtabstop=4 expandtab:
