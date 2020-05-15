#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict
from uitest.path import get_srcdir_url

def get_url_for_data_file(file_name):
    return get_srcdir_url() + "/sc/qa/uitest/calc_tests/data/" + file_name

class navigator(UITestCase):

    def test_rename_sheet(self):
        self.ui_test.load_file(get_url_for_data_file("navigator.ods"))
        xCalcDoc = self.xUITest.getTopFocusWindow()
        xGridWin = xCalcDoc.getChild("grid_window")

        self.xUITest.executeCommand(".uno:Sidebar")
        xGridWin.executeAction("SIDEBAR", mkPropertyValues({"PANEL": "ScNavigatorPanel"}))

        xCalcDoc = self.xUITest.getTopFocusWindow()
        xNavigatorPanel = xCalcDoc.getChild("NavigatorPanelParent")
        xContentBox = xNavigatorPanel.getChild('contentbox')
        xSheets = xContentBox.getChild("0")
        self.assertEqual(get_state_as_dict(xSheets)['Text'], 'Sheets')
        self.assertEqual(len(xSheets.getChildren()), 2)
        self.assertEqual(get_state_as_dict(xSheets.getChild('0'))['Text'], 'Sheet1')
        self.assertEqual(get_state_as_dict(xSheets.getChild('1'))['Text'], 'S2')
        xRangeNames = xContentBox.getChild("1")
        self.assertEqual(get_state_as_dict(xRangeNames)['Text'], 'Range names')
        self.assertEqual(len(xRangeNames.getChildren()), 2)
        self.assertEqual(get_state_as_dict(xRangeNames.getChild('0'))['Text'], 'first (Sheet1)')
        self.assertEqual(get_state_as_dict(xRangeNames.getChild('1'))['Text'], 'second (Sheet1)')

        #The document has one comment too
        xComments = xContentBox.getChild("6")
        self.assertEqual(get_state_as_dict(xComments)['Text'], 'Comments')
        self.assertEqual(len(xComments.getChildren()), 1)
        self.assertEqual(get_state_as_dict(xComments.getChild('0'))['Text'], 'Test Comment')

        self.ui_test.execute_dialog_through_command(".uno:RenameTable")
        xDialog = self.xUITest.getTopFocusWindow()
        xname_entry = xDialog.getChild("name_entry")
        xname_entry.executeAction("TYPE", mkPropertyValues({"TEXT": "S1"}))
        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        xCalcDoc = self.xUITest.getTopFocusWindow()
        xNavigatorPanel = xCalcDoc.getChild("NavigatorPanelParent")
        xContentBox = xNavigatorPanel.getChild('contentbox')
        xSheets = xContentBox.getChild("0")
        self.assertEqual(get_state_as_dict(xSheets)['Text'], 'Sheets')
        self.assertEqual(len(xSheets.getChildren()), 2)
        self.assertEqual(get_state_as_dict(xSheets.getChild('0'))['Text'], 'S1')
        self.assertEqual(get_state_as_dict(xSheets.getChild('1'))['Text'], 'S2')
        xRangeNames = xContentBox.getChild("1")
        self.assertEqual(get_state_as_dict(xRangeNames)['Text'], 'Range names')
        self.assertEqual(len(xRangeNames.getChildren()), 2)
        self.assertEqual(get_state_as_dict(xRangeNames.getChild('0'))['Text'], 'first (S1)')
        self.assertEqual(get_state_as_dict(xRangeNames.getChild('1'))['Text'], 'second (S1)')

        self.xUITest.executeCommand(".uno:Undo")

        self.assertEqual(get_state_as_dict(xSheets.getChild('0'))['Text'], 'Sheet1')
        self.assertEqual(get_state_as_dict(xSheets.getChild('1'))['Text'], 'S2')
        #FIXME: tdf#133082
        #self.assertEqual(get_state_as_dict(xRangeNames.getChild('0'))['Text'], 'first (Sheet1)')
        #self.assertEqual(get_state_as_dict(xRangeNames.getChild('1'))['Text'], 'second (Sheet1)')

        self.xUITest.executeCommand(".uno:Sidebar")
        self.ui_test.close_doc()
