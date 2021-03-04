#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file

class navigator(UITestCase):

    def test_rename_sheet(self):
        self.ui_test.load_file(get_url_for_data_file("navigator.ods"))
        xCalcDoc = self.xUITest.getTopFocusWindow()
        xGridWin = xCalcDoc.getChild("grid_window")

        self.xUITest.executeCommand(".uno:Sidebar")
        xGridWin.executeAction("SIDEBAR", mkPropertyValues({"PANEL": "ScNavigatorPanel"}))

        xCalcDoc = self.xUITest.getTopFocusWindow()
        xNavigatorPanel = xCalcDoc.getChild("NavigatorPanel")
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
        xNavigatorPanel = xCalcDoc.getChild("NavigatorPanel")
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

        xCalcDoc = self.xUITest.getTopFocusWindow()
        xNavigatorPanel = xCalcDoc.getChild("NavigatorPanel")
        xContentBox = xNavigatorPanel.getChild('contentbox')
        xSheets = xContentBox.getChild("0")
        self.assertEqual(get_state_as_dict(xSheets)['Text'], 'Sheets')
        self.assertEqual(len(xSheets.getChildren()), 2)
        self.assertEqual(get_state_as_dict(xSheets.getChild('0'))['Text'], 'Sheet1')
        self.assertEqual(get_state_as_dict(xSheets.getChild('1'))['Text'], 'S2')
        xRangeNames = xContentBox.getChild("1")
        self.assertEqual(get_state_as_dict(xRangeNames)['Text'], 'Range names')
        self.assertEqual(len(xRangeNames.getChildren()), 2)
        #FIXME: tdf#133082
        #self.assertEqual(get_state_as_dict(xRangeNames.getChild('0'))['Text'], 'first (Sheet1)')
        #self.assertEqual(get_state_as_dict(xRangeNames.getChild('1'))['Text'], 'second (Sheet1)')

        self.xUITest.executeCommand(".uno:Sidebar")
        self.ui_test.close_doc()

    def test_tdf98493(self):

        self.ui_test.load_file(get_url_for_data_file("tdf98493.ods"))
        xCalcDoc = self.xUITest.getTopFocusWindow()
        xGridWin = xCalcDoc.getChild("grid_window")

        self.assertEqual(get_state_as_dict(xGridWin)["SelectedTable"], "2")

        self.xUITest.executeCommand(".uno:Sidebar")
        xGridWin.executeAction("SIDEBAR", mkPropertyValues({"PANEL": "ScNavigatorPanel"}))

        xCalcDoc = self.xUITest.getTopFocusWindow()
        xNavigatorPanel = xCalcDoc.getChild("NavigatorPanel")
        xToolBar = xNavigatorPanel.getChild("toolbox2")
        xToolBar.executeAction("CLICK", mkPropertyValues({"POS": "0"})) # 'toggle' button

        xContentBox = xNavigatorPanel.getChild('contentbox')

        # tdf#133079, without the fix in place, it would be 8
        self.assertEqual(len(xContentBox.getChildren()), 1)

        xSheets = xContentBox.getChild('0')
        self.assertEqual(len(xSheets.getChildren()), 12)

        #key=item position, value=sheet ( there are hidden sheets )
        results = { '1': '0', '5': '4', '6': '4', '11': '10'}

        for k, v in results.items():
            xChild = xSheets.getChild(k)
            xChild.executeAction("DOUBLECLICK", tuple())

            self.assertEqual(get_state_as_dict(xGridWin)["SelectedTable"], v)

        self.xUITest.executeCommand(".uno:Sidebar")
        self.ui_test.close_doc()


    def test_tdf134390(self):
        calc_doc = self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        xGridWin = xCalcDoc.getChild("grid_window")

        self.xUITest.executeCommand(".uno:Sidebar")
        xGridWin.executeAction("SIDEBAR", mkPropertyValues({"PANEL": "ScNavigatorPanel"}))

        xCalcDoc = self.xUITest.getTopFocusWindow()
        xNavigatorPanel = xCalcDoc.getChild("NavigatorPanel")
        xToolBar = xNavigatorPanel.getChild("toolbox2")
        xToolBar.executeAction("CLICK", mkPropertyValues({"POS": "0"})) # 'toggle' button

        xRow = xNavigatorPanel.getChild('row')
        xColumn = xNavigatorPanel.getChild('column')
        self.assertEqual(get_state_as_dict(xColumn)['Value'], '1')
        self.assertEqual(get_state_as_dict(xRow)['Value'], '1')
        self.assertEqual(get_state_as_dict(xGridWin)["CurrentRow"], "0")
        self.assertEqual(get_state_as_dict(xGridWin)["CurrentColumn"], "0")

        xRow.executeAction("UP", tuple())
        xColumn.executeAction("UP", tuple())

        # Use return to update the current cell
        xColumn.executeAction("TYPE", mkPropertyValues({"KEYCODE":"RETURN"}))

        self.assertEqual(get_state_as_dict(xColumn)['Value'], '2')
        self.assertEqual(get_state_as_dict(xRow)['Value'], '2')

        self.assertEqual(get_state_as_dict(xGridWin)["CurrentRow"], "1")
        self.assertEqual(get_state_as_dict(xGridWin)["CurrentColumn"], "1")

        self.xUITest.executeCommand(".uno:Sidebar")

        self.ui_test.close_doc()
