#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict

class tdf145978(UITestCase):
    def test_tdf145978(self):
        with self.ui_test.execute_dialog_through_command(".uno:RunMacro") as xDialog:
            xCategoriesTree = xDialog.getChild("categories")
            xCategoriesTreeEntry = xCategoriesTree.getChild('1') #Application Macros
            xCategoriesTreeEntry.executeAction("EXPAND", tuple())

            xSubCategoriesTreeEntry = None
            for i in xCategoriesTreeEntry.getChildren():
                xChild = xCategoriesTreeEntry.getChild(i)
                if get_state_as_dict(xChild)["Text"] == "HelloWorld":
                    xSubCategoriesTreeEntry = xChild
                    break

            xSubCategoriesTreeEntry.executeAction("SELECT", tuple())

            xCommandsTree = xDialog.getChild("commands")

            xCommandsTreeEntry = None
            for i in xCommandsTree.getChildren():
                xChild = xCommandsTree.getChild(i)
                if get_state_as_dict(xChild)["Text"] == "HelloWorldPython":
                    xCommandsTreeEntry = xChild
                    break

            xCommandsTreeEntry.executeAction("SELECT", tuple())

        #Verify the dialog reloads with previous run macro selected
        with self.ui_test.execute_dialog_through_command(".uno:RunMacro") as xDialog:
            xTree = xDialog.getChild("categories")
            self.assertEqual("HelloWorld", get_state_as_dict(xTree)["SelectEntryText"])
            xTree = xDialog.getChild("commands")
            self.assertEqual("HelloWorldPython", get_state_as_dict(xTree)["SelectEntryText"])

# vim: set shiftwidth=4 softtabstop=4 expandtab:
