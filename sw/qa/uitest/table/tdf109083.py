# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import select_pos

#Bug 109083 - Updating table style: changes didn't propagate to other tables when selection was over two columns at the moment of updating
class tdf109083(UITestCase):
    def test_tdf109083(self):
        with self.ui_test.create_doc_in_start_center("writer"):
            xWriterDoc = self.xUITest.getTopFocusWindow()
            xWriterEdit = xWriterDoc.getChild("writer_edit")
            #generate two 2x2 tables with the same autoformat table style (Default Table Style)
            #Note that this style is different than applying nothing!
            for i in range(0, 2):
                with self.ui_test.execute_dialog_through_command(".uno:InsertTable") as xDialog:
                    formatlbinstable = xDialog.getChild("formatlbinstable")
                    entry = formatlbinstable.getChild("1")
                    entry.executeAction("SELECT", tuple())
                xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE":"RETURN"}))

            #select the last row of the first table
            for i in range (0,2):
                self.xUITest.executeCommand(".uno:GoDown")
            for i in range (0,2):
                self.xUITest.executeCommand(".uno:CharRightSel")
            #set a specific color on the selected cells (last row)
            self.xUITest.executeCommandWithParameters(".uno:TableCellBackgroundColor", mkPropertyValues({"TableCellBackgroundColor" : 16776960 }))

            #Sidebar -> Table Styles -> Style Actions -> "Update Selected Style" (note: the row is still selected)
            self.xUITest.executeCommand(".uno:Sidebar")
            xWriterEdit.executeAction("SIDEBAR", mkPropertyValues({"PANEL": "StyleListPanel"}))
            xLeft = xWriterEdit.getChild('left')
            xLeft.executeAction("CLICK", mkPropertyValues({"POS": "5"}))
            xRight = xWriterEdit.getChild('right')
            xRight.executeAction("CLICK", mkPropertyValues({"POS": "3"}))
            #select the second table
            for i in range (0,2):
                self.xUITest.executeCommand(".uno:GoDown")

            #first row's cells must be yellow, second/last row's cells must be updated to yellow by now
            for i in range (0,4):
                with self.ui_test.execute_dialog_through_command(".uno:TableDialog") as xDialog:
                    xTabs = xDialog.getChild("tabcontrol")
                    select_pos(xTabs, "4")   #tab Background
                    btncolor = xDialog.getChild("btncolor")
                    btncolor.executeAction("CLICK", tuple())
                    hex_custom = xDialog.getChild("hex_custom")
                    if i >= 2:
                        self.assertEqual(get_state_as_dict(hex_custom)["Text"], "ffff00")
                    else:
                        self.assertEqual(get_state_as_dict(hex_custom)["Text"], "ffffff")
                self.xUITest.executeCommand(".uno:GoRight")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
