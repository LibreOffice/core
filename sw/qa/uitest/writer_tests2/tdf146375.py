# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, select_by_text
from libreoffice.uno.propertyvalue import mkPropertyValues
import random
import string

class Tdf146375(UITestCase):

   def test_tdf146375(self):
        with self.ui_test.create_doc_in_start_center("writer"):

            count = 0
            # Use a random name
            categoryName = ''.join(random.choice(string.ascii_lowercase) for i in range(15))
            renamedCategory = categoryName + "-renamed"

            with self.ui_test.execute_dialog_through_command(".uno:NewDoc", close_button="close") as xDialog:
                xFilterFolder = xDialog.getChild("filter_folder")
                self.assertEqual("All Categories", get_state_as_dict(xFilterFolder)["SelectEntryText"])
                count = int(get_state_as_dict(xFilterFolder)["EntryCount"])

                xActionMenu = xDialog.getChild("action_menu")

                # Create a new category
                with self.ui_test.execute_blocking_action(
                        xActionMenu.executeAction, args=('OPENFROMLIST', mkPropertyValues({"POS": "0"}))) as xNameDialog:
                    xEntry = xNameDialog.getChild("entry")
                    xEntry.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
                    xEntry.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
                    xEntry.executeAction("TYPE", mkPropertyValues({"TEXT": categoryName}))

                self.assertEqual(count + 1, int(get_state_as_dict(xFilterFolder)["EntryCount"]))

                select_by_text(xFilterFolder, categoryName)
                self.assertEqual(categoryName, get_state_as_dict(xFilterFolder)["SelectEntryText"])

                # Rename the category
                with self.ui_test.execute_blocking_action(
                        xActionMenu.executeAction, args=('OPENFROMLIST', mkPropertyValues({"POS": "1"}))) as xNameDialog:
                    xEntry = xNameDialog.getChild("entry")
                    xEntry.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
                    xEntry.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
                    xEntry.executeAction("TYPE", mkPropertyValues({"TEXT": renamedCategory}))

                self.assertEqual(count + 1, int(get_state_as_dict(xFilterFolder)["EntryCount"]))
                self.assertEqual(renamedCategory, get_state_as_dict(xFilterFolder)["SelectEntryText"])

            with self.ui_test.execute_dialog_through_command(".uno:NewDoc", close_button="close") as xDialog:
                xFilterFolder = xDialog.getChild("filter_folder")
                self.assertEqual(count + 1, int(get_state_as_dict(xFilterFolder)["EntryCount"]))

                select_by_text(xFilterFolder, renamedCategory)

                # Without the fix in place, this test would have failed with
                # AssertionError: 'zwpyzgwuwleanap-renamed' != 'All Categories'
                self.assertEqual(renamedCategory, get_state_as_dict(xFilterFolder)["SelectEntryText"])

                xActionMenu = xDialog.getChild("action_menu")

                # Delete the category
                with self.ui_test.execute_blocking_action(
                        xActionMenu.executeAction, args=('OPENFROMLIST', mkPropertyValues({"POS": "2"})), close_button="yes") as xNameDialog:
                    pass

                self.assertEqual(count, int(get_state_as_dict(xFilterFolder)["EntryCount"]))

# vim: set shiftwidth=4 softtabstop=4 expandtab:
