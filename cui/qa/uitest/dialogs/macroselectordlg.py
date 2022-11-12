#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict

class tdf145978(UITestCase):
    def test_tdf145978(self):
        with self.ui_test.execute_dialog_through_command(".uno:RunMacro", close_button="") as xDialog:
            xCategoriesTree = xDialog.getChild("categories")
            xCategoriesTreeEntry = xCategoriesTree.getChild('1') #Application Macros
            xCategoriesTreeEntry.executeAction("EXPAND", tuple())
            xCategoriesTreeEntry = xCategoriesTreeEntry.getChild('8') #HelloWorld
            xCategoriesTreeEntry.executeAction("SELECT", tuple())

            aCategoriesTreeEntryText = get_state_as_dict(xCategoriesTreeEntry)["Text"]

            xCommandsTree = xDialog.getChild("commands")
            xCommandsTreeEntry = xCommandsTree.getChild('2') #HelloWorld Python
            xCommandsTreeEntry.executeAction("SELECT", tuple())

            aCommandsTreeEntryText = get_state_as_dict(xCommandsTreeEntry)["Text"]

            xOKBtn = xDialog.getChild("ok")
            self.ui_test.close_dialog_through_button(xOKBtn)

        #Verify the dialog reloads with previous run macro selected
        with self.ui_test.execute_dialog_through_command(".uno:RunMacro") as xDialog:
            xTree = xDialog.getChild("categories")
            self.assertEqual(get_state_as_dict(xTree)["SelectEntryText"], aCategoriesTreeEntryText)
            xTree = xDialog.getChild("commands")
            self.assertEqual(get_state_as_dict(xTree)["SelectEntryText"], aCommandsTreeEntryText)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
