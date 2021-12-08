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
from uitest.uihelper.common import select_by_text, get_state_as_dict
from libreoffice.uno.propertyvalue import mkPropertyValues
from libreoffice.calc.document import get_row

#Bug 46184 - [Calc] [AutoFilter] Option "Copy results to ..." remains activated in AutoFilter

class tdf46184(UITestCase):
    def test_tdf46184_copy_results_to(self):
        with self.ui_test.create_doc_in_start_center("calc") as document:
            calcDoc = self.xUITest.getTopFocusWindow()
            gridwin = calcDoc.getChild("grid_window")

            enter_text_to_cell(gridwin, "A1", "A")
            enter_text_to_cell(gridwin, "A2", "1")
            enter_text_to_cell(gridwin, "A3", "2")
            enter_text_to_cell(gridwin, "A4", "3")

            gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:A4"}))

            self.xUITest.executeCommand(".uno:DataFilterAutoFilter")

            gridwin.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": "0", "ROW": "0"}))
            xFloatWindow = self.xUITest.getFloatWindow()
            #Choose Standard Filter... button
            xMenu = xFloatWindow.getChild("menu")
            xMenu.executeAction("TYPE", mkPropertyValues({"KEYCODE":"DOWN"}))
            xMenu.executeAction("TYPE", mkPropertyValues({"KEYCODE":"DOWN"}))
            xMenu.executeAction("TYPE", mkPropertyValues({"KEYCODE":"DOWN"}))
            xMenu.executeAction("TYPE", mkPropertyValues({"KEYCODE":"RETURN"}))
            xSubFloatWindow = self.xUITest.getFloatWindow()
            xSubMenu = xSubFloatWindow.getChild("menu")
            xSubMenu.executeAction("TYPE", mkPropertyValues({"KEYCODE":"DOWN"}))
            xSubMenu.executeAction("TYPE", mkPropertyValues({"KEYCODE":"DOWN"}))
            xSubMenu.executeAction("TYPE", mkPropertyValues({"KEYCODE":"DOWN"}))
            self.assertEqual("Standard Filter...", get_state_as_dict(xSubMenu)['SelectEntryText'])
            xSubMenu.executeAction("TYPE", mkPropertyValues({"KEYCODE":"RETURN"}))

            xDialog = self.xUITest.getTopFocusWindow()
            xfield1 = xDialog.getChild("field1")
            xcond1 = xDialog.getChild("cond1")
            xval1 = xDialog.getChild("val1")
            xcopyresult = xDialog.getChild("copyresult")
            xedcopyarea = xDialog.getChild("edcopyarea")
            xdestpers = xDialog.getChild("destpers")

            select_by_text(xfield1, "A")
            select_by_text(xcond1, ">")
            xval1.executeAction("TYPE", mkPropertyValues({"TEXT":"1"}))
            xcopyresult.executeAction("CLICK", tuple())
            xedcopyarea.executeAction("TYPE", mkPropertyValues({"TEXT":"A6"}))
            if get_state_as_dict(xdestpers)['Selected'] == 'false':
                xdestpers.executeAction("CLICK", tuple())
            self.assertEqual('true', get_state_as_dict(xcopyresult)['Selected'])
            xOKBtn = xDialog.getChild("ok")
            self.ui_test.close_dialog_through_button(xOKBtn)


            row1 = get_row(document, 1)
            row2 = get_row(document, 2)
            row3 = get_row(document, 3)
            self.assertTrue(row1.getPropertyValue("IsVisible"))
            self.assertTrue(row2.getPropertyValue("IsVisible"))
            self.assertTrue(row3.getPropertyValue("IsVisible"))

            gridwin.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": "0", "ROW": "0"}))
            xFloatWindow = self.xUITest.getFloatWindow()
            xCheckListMenu = xFloatWindow.getChild("FilterDropDown")
            xTreeList = xCheckListMenu.getChild("check_list_box")
            xEntry = xTreeList.getChild("1")
            xEntry.executeAction("CLICK", tuple())
            xOkBtn = xFloatWindow.getChild("ok")
            xOkBtn.executeAction("CLICK", tuple())

            self.assertTrue(row1.getPropertyValue("IsVisible"))
            self.assertFalse(row2.getPropertyValue("IsVisible"))
            self.assertTrue(row3.getPropertyValue("IsVisible"))

# vim: set shiftwidth=4 softtabstop=4 expandtab:
