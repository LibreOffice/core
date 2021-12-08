# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.calc import enter_text_to_cell
from libreoffice.uno.propertyvalue import mkPropertyValues
from libreoffice.calc.document import get_cell_by_position
from libreoffice.calc.document import is_row_hidden

class tdf126306(UITestCase):
    def check_values(self, document, results):
        for i, value in enumerate(results, start=1):
            self.assertEqual(get_cell_by_position(document, 0, 0, i).getValue(), value)

    def check_row_hidden(self, document, results = [True] * 14):
        for i, value in enumerate(results, start=1):
            bVisible = not is_row_hidden(document, i)
            self.assertEqual(bVisible, value)

    def test_run(self):
        with self.ui_test.create_doc_in_start_center("calc") as document:
            calcDoc = self.xUITest.getTopFocusWindow()
            xGridWin = calcDoc.getChild("grid_window")

            default_values = [25, 1023, 17, 9, 19, 0, 107, 89, 8, 453, 33, 3, 25, 204]

            for i, value in enumerate(default_values, start=2):
                enter_text_to_cell(xGridWin, "A" + str(i), str(value))

            xGridWin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:A15"}))

            with self.ui_test.execute_dialog_through_command(".uno:DataFilterAutoFilter", close_button="yes"):
                pass

            self.assertEqual(document.getPropertyValue("UnnamedDatabaseRanges").getByTable(0).AutoFilter, True)

            self.check_values(document, default_values)
            self.check_row_hidden(document)

            # Sort ascending button
            xGridWin.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": "0", "ROW": "0"}))
            xFloatWindow = self.xUITest.getFloatWindow()
            xMenu = xFloatWindow.getChild("menu")
            xMenu.executeAction("TYPE", mkPropertyValues({"KEYCODE":"SPACE"}))
            xMenu.executeAction("TYPE", mkPropertyValues({"KEYCODE":"RETURN"}))

            sort_asc_values = [0, 3, 8, 9, 17, 19, 25, 25, 33, 89, 107, 204, 453, 1023]
            self.check_values(document, sort_asc_values)
            self.check_row_hidden(document)

            #Undo
            self.xUITest.executeCommand(".uno:Undo")
            self.check_values(document, default_values)
            self.check_row_hidden(document)

            # Sort descending button
            xGridWin.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": "0", "ROW": "0"}))
            xFloatWindow = self.xUITest.getFloatWindow()
            xMenu = xFloatWindow.getChild("menu")
            xMenu.executeAction("TYPE", mkPropertyValues({"KEYCODE":"DOWN"}))
            xMenu.executeAction("TYPE", mkPropertyValues({"KEYCODE":"RETURN"}))

            sort_des_values = [1023, 453, 204, 107, 89, 33, 25, 25, 19, 17, 9, 8, 3, 0]
            self.check_values(document, sort_des_values)
            self.check_row_hidden(document)

            #Undo
            self.xUITest.executeCommand(".uno:Undo")
            self.check_values(document, default_values)
            self.check_row_hidden(document)

            # Top 10 button
            xGridWin.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": "0", "ROW": "0"}))
            xFloatWindow = self.xUITest.getFloatWindow()
            xMenu = xFloatWindow.getChild("menu")
            xMenu.executeAction("TYPE", mkPropertyValues({"KEYCODE":"DOWN"}))
            xMenu.executeAction("TYPE", mkPropertyValues({"KEYCODE":"DOWN"}))
            xMenu.executeAction("TYPE", mkPropertyValues({"KEYCODE":"DOWN"}))
            xMenu.executeAction("TYPE", mkPropertyValues({"KEYCODE":"RETURN"}))
            xSubFloatWindow = self.xUITest.getFloatWindow()
            xSubMenu = xSubFloatWindow.getChild("menu")
            xSubMenu.executeAction("TYPE", mkPropertyValues({"KEYCODE":"RETURN"}))

            top10_hidden_values = [True, True, True, False, True, False, True,
                    True, False, True, True, False, True, True]

            #Values are the same
            self.check_values(document, default_values)
            self.check_row_hidden(document, top10_hidden_values)

            #Undo
            self.xUITest.executeCommand(".uno:Undo")
            self.check_values(document, default_values)
            self.check_row_hidden(document)

            # Empty button
            xGridWin.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": "0", "ROW": "0"}))
            xFloatWindow = self.xUITest.getFloatWindow()
            xMenu = xFloatWindow.getChild("menu")
            xMenu.executeAction("TYPE", mkPropertyValues({"KEYCODE":"DOWN"}))
            xMenu.executeAction("TYPE", mkPropertyValues({"KEYCODE":"DOWN"}))
            xMenu.executeAction("TYPE", mkPropertyValues({"KEYCODE":"DOWN"}))
            xMenu.executeAction("TYPE", mkPropertyValues({"KEYCODE":"RETURN"}))
            xSubFloatWindow = self.xUITest.getFloatWindow()
            xSubMenu = xSubFloatWindow.getChild("menu")
            xSubMenu.executeAction("TYPE", mkPropertyValues({"KEYCODE":"DOWN"}))
            xSubMenu.executeAction("TYPE", mkPropertyValues({"KEYCODE":"RETURN"}))

            empty_values = [False] * 14
            #Values are the same
            self.check_values(document, default_values)
            self.check_row_hidden(document, empty_values)

            #Undo
            self.xUITest.executeCommand(".uno:Undo")
            self.check_values(document, default_values)
            self.check_row_hidden(document)

            # Not Empty button
            xGridWin.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": "0", "ROW": "0"}))
            xFloatWindow = self.xUITest.getFloatWindow()
            xMenu = xFloatWindow.getChild("menu")
            xMenu.executeAction("TYPE", mkPropertyValues({"KEYCODE":"DOWN"}))
            xMenu.executeAction("TYPE", mkPropertyValues({"KEYCODE":"DOWN"}))
            xMenu.executeAction("TYPE", mkPropertyValues({"KEYCODE":"DOWN"}))
            xMenu.executeAction("TYPE", mkPropertyValues({"KEYCODE":"RETURN"}))
            xSubFloatWindow = self.xUITest.getFloatWindow()
            xSubMenu = xSubFloatWindow.getChild("menu")
            xSubMenu.executeAction("TYPE", mkPropertyValues({"KEYCODE":"DOWN"}))
            xSubMenu.executeAction("TYPE", mkPropertyValues({"KEYCODE":"DOWN"}))
            xSubMenu.executeAction("TYPE", mkPropertyValues({"KEYCODE":"RETURN"}))

            #Nothing should change
            self.check_values(document, default_values)
            self.check_row_hidden(document)

            #Undo
            self.xUITest.executeCommand(".uno:Undo")
            self.check_values(document, default_values)
            self.check_row_hidden(document)

            # finish

# vim: set shiftwidth=4 softtabstop=4 expandtab:
