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

                initialEntryCount = int(get_state_as_dict(xfunc)["Children"])
                self.assertTrue(initialEntryCount != 0)

                xSearch.executeAction("TYPE", mkPropertyValues({"TEXT":"format"}))

                # Wait for the search/filter op to be completed
                while True:
                    filteredEntryCount = int(get_state_as_dict(xfunc)["Children"])
                    if filteredEntryCount != initialEntryCount:
                        break
                    time.sleep(self.ui_test.get_default_sleep())

                self.assertTrue(filteredEntryCount < initialEntryCount)

                xSearch.executeAction("CLEAR", tuple())

                # Wait for the search/filter op to be completed
                while True:
                    finalEntryCount = int(get_state_as_dict(xfunc)["Children"])
                    if finalEntryCount != filteredEntryCount:
                        break
                    time.sleep(self.ui_test.get_default_sleep())

                self.assertEqual(initialEntryCount, finalEntryCount)

    def test_category_listbox(self):
        with self.ui_test.create_doc_in_start_center("writer"):
            with self.ui_test.execute_dialog_through_command(".uno:ConfigureDialog", close_button="cancel") as xDialog:

                xFunc = xDialog.getChild("functions")
                xCategory = xDialog.getChild("commandcategorylist")

                initialEntryCount = int(get_state_as_dict(xFunc)["Children"])
                self.assertTrue(initialEntryCount != 0)

                select_pos(xCategory, "1")
                filteredEntryCount = int(get_state_as_dict(xFunc)["Children"])
                self.assertTrue(filteredEntryCount < initialEntryCount)

                select_pos(xCategory, "0")
                finalEntryCount = int(get_state_as_dict(xFunc)["Children"])
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

    def test_add_remove_items(self):
        with self.ui_test.create_doc_in_start_center("writer"):

            with self.ui_test.execute_dialog_through_command(".uno:ConfigureDialog") as xDialog:
                xTab = xDialog.getChild("tabcontrol")
                select_pos(xTab, "0")

                xFunctions = xDialog.getChild("functions")
                xMenuContents = xDialog.getChild("menucontents")
                xAddBtn = xDialog.getChild("add")
                xRemoveBtn = xDialog.getChild("remove")

                self.assertEqual("1", get_state_as_dict(xFunctions)['SelectionCount'])
                sSelectEntryText = get_state_as_dict(xFunctions)['SelectEntryText']
                nChildrenCount = int(get_state_as_dict(xMenuContents)['Children'])

                self.assertEqual('true',get_state_as_dict(xAddBtn)['Enabled'])
                self.assertEqual('false',get_state_as_dict(xRemoveBtn)['Enabled'])

                xAddBtn.executeAction("CLICK", tuple())

                self.assertEqual(nChildrenCount + 1, int(get_state_as_dict(xMenuContents)['Children']))
                self.assertEqual(sSelectEntryText, get_state_as_dict(xMenuContents)['SelectEntryText'])

                self.assertEqual('false',get_state_as_dict(xAddBtn)['Enabled'])
                self.assertEqual('true',get_state_as_dict(xRemoveBtn)['Enabled'])

                xRemoveBtn.executeAction("CLICK", tuple())

                self.assertEqual(nChildrenCount, int(get_state_as_dict(xMenuContents)['Children']))


# vim: set shiftwidth=4 softtabstop=4 expandtab:
