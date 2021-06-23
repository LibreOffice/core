# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues

class CellDropDownItems(UITestCase):

    def test_dropdownitems(self):

        #This is to test Dropdown items in grid window
        calc_doc = self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()

        #select cell C10
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "C10"}))

        #Open Validation Dialog
        self.ui_test.execute_dialog_through_command(".uno:Validation")
        xDialog = self.xUITest.getTopFocusWindow()

        #Select List option
        xallow = xDialog.getChild("allow")
        xallow.executeAction("SELECT", mkPropertyValues({"POS": "6"}))

        #Add items to the List
        xminlist = xDialog.getChild("minlist")
        xminlist.executeAction("TYPE", mkPropertyValues({"TEXT": "Item1"}))
        xminlist.executeAction("TYPE", mkPropertyValues({"KEYCODE": "RETURN"}))
        xminlist.executeAction("TYPE", mkPropertyValues({"TEXT": "Item2"}))
        xminlist.executeAction("TYPE", mkPropertyValues({"KEYCODE": "RETURN"}))
        xminlist.executeAction("TYPE", mkPropertyValues({"TEXT": "Item3"}))
        xminlist.executeAction("TYPE", mkPropertyValues({"KEYCODE": "RETURN"}))
        xminlist.executeAction("TYPE", mkPropertyValues({"TEXT": "Item4"}))
        xminlist.executeAction("TYPE", mkPropertyValues({"KEYCODE": "RETURN"}))
        xminlist.executeAction("TYPE", mkPropertyValues({"TEXT": "Item5"}))

        #Close the dialog
        xOk = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOk)

        #Launch the Select Menu to view the list ans select first item in the list
        gridwin = xCalcDoc.getChild("grid_window")
        gridwin.executeAction("LAUNCH", mkPropertyValues({"SELECTMENU": "", "COL": "2", "ROW": "9"}))

        #Select the TreeList UI Object
        xWin = self.xUITest.getTopFocusWindow()
        xlist = xWin.getChild("list")

        xListItem = xlist.getChild('0')
        xListItem.executeAction("DOUBLECLICK" , mkPropertyValues({}) )
        self.assertEqual(get_cell_by_position(document, 0, 2, 9).getString(), "Item1")

        #Launch the Select Menu to view the list ans select Third item in the list
        gridwin = xCalcDoc.getChild("grid_window")
        gridwin.executeAction("LAUNCH", mkPropertyValues({"SELECTMENU": "", "COL": "2", "ROW": "9"}))

        #Select the TreeList UI Object
        xWin = self.xUITest.getTopFocusWindow()
        xlist = xWin.getChild("list")

        xListItem = xlist.getChild('2')
        xListItem.executeAction("DOUBLECLICK" , mkPropertyValues({}) )
        self.assertEqual(get_cell_by_position(document, 0, 2, 9).getString(), "Item3")

        #Launch the Select Menu to view the list ans select Fifth item in the list
        gridwin = xCalcDoc.getChild("grid_window")
        gridwin.executeAction("LAUNCH", mkPropertyValues({"SELECTMENU": "", "COL": "2", "ROW": "9"}))

        #Select the TreeList UI Object
        xWin = self.xUITest.getTopFocusWindow()
        xlist = xWin.getChild("list")

        xListItem = xlist.getChild('4')
        xListItem.executeAction("DOUBLECLICK" , mkPropertyValues({}) )
        self.assertEqual(get_cell_by_position(document, 0, 2, 9).getString(), "Item5")

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
