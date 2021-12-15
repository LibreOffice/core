# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import select_pos

class CellDropDownItems(UITestCase):

    def test_dropdownitems(self):

        #This is to test Dropdown items in grid window
        with self.ui_test.create_doc_in_start_center("calc") as document:
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")

            #select cell C10
            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "C10"}))

            #Open Validation Dialog
            with self.ui_test.execute_dialog_through_command(".uno:Validation") as xDialog:

                #Select List option
                xallow = xDialog.getChild("allow")
                select_pos(xallow, "6")

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


# vim: set shiftwidth=4 softtabstop=4 expandtab:
