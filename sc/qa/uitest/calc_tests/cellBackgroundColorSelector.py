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
from uitest.uihelper.common import select_pos
from uitest.uihelper.common import select_by_text
from libreoffice.uno.propertyvalue import mkPropertyValues

class CalcCellBackgroundColorSelector(UITestCase):

    def test_color_selector(self):

        #This is to test color selection
        with self.ui_test.create_doc_in_start_center("calc"):
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")
            #select cell A1
            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
            #format - cell
            with self.ui_test.execute_dialog_through_command(".uno:FormatCellDialog") as xDialog:
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "6")  #tab Numbers
                # click on color btn
                xbtncolor = xDialog.getChild("btncolor")
                xbtncolor.executeAction("CLICK",tuple())
                xpaletteselector = xDialog.getChild("paletteselector")

                # Now we have the ColorPage that we can get the color selector from it
                xColorpage = xDialog.getChild("ColorPage")
                color_selector = xColorpage.getChild("iconview_colors")

                # For chart-palettes colors
                select_by_text(xpaletteselector, "Chart Palettes")
                # Select Color with id 1
                color_element2 = color_selector.getChild("1")
                color_element2.executeAction("SELECT", mkPropertyValues({}))
                self.assertEqual(get_state_as_dict(color_selector)["SelectedItemId"], "1")
                self.assertEqual(get_state_as_dict(color_selector)["SelectedItemPos"], "1")
                self.assertEqual(get_state_as_dict(color_selector)["VisibleCount"], "12")
                self.assertEqual(get_state_as_dict(color_selector)["SelectEntryText"], "Chart 2")

                # Select Color with id 4
                color_element5 = color_selector.getChild("4")
                color_element5.executeAction("SELECT", mkPropertyValues({}))
                self.assertEqual(get_state_as_dict(color_selector)["SelectedItemId"], "4")
                self.assertEqual(get_state_as_dict(color_selector)["SelectedItemPos"], "4")
                self.assertEqual(get_state_as_dict(color_selector)["SelectEntryText"], "Chart 5")

                # For libreoffice colors
                select_by_text(xpaletteselector, "LibreOffice")
                # Select Color with id 5
                color_element6 = color_selector.getChild("5")
                color_element6.executeAction("SELECT", mkPropertyValues({}))
                self.assertEqual(get_state_as_dict(color_selector)["SelectedItemId"], "5")
                self.assertEqual(get_state_as_dict(color_selector)["SelectedItemPos"], "5")
                self.assertEqual(get_state_as_dict(color_selector)["VisibleCount"], "32")
                self.assertEqual(get_state_as_dict(color_selector)["SelectEntryText"], "Green Accent")

                # Select Color with id 29
                color_element30 = color_selector.getChild("29")
                color_element30.executeAction("SELECT", mkPropertyValues({}))
                self.assertEqual(get_state_as_dict(color_selector)["SelectedItemId"], "29")
                self.assertEqual(get_state_as_dict(color_selector)["SelectedItemPos"], "29")
                self.assertEqual(get_state_as_dict(color_selector)["SelectEntryText"], "Yellow Accent")

                # For html colors
                select_by_text(xpaletteselector, "HTML")
                # Select Color with id 0
                color_element1 = color_selector.getChild("0")
                color_element1.executeAction("SELECT", mkPropertyValues({}))
                self.assertEqual(get_state_as_dict(color_selector)["SelectedItemId"], "0")
                self.assertEqual(get_state_as_dict(color_selector)["SelectedItemPos"], "0")
                self.assertEqual(get_state_as_dict(color_selector)["VisibleCount"], "139")
                self.assertEqual(get_state_as_dict(color_selector)["SelectEntryText"], "White")
                # Select Color with id 119
                color_element120 = color_selector.getChild("119")
                color_element120.executeAction("SELECT", mkPropertyValues({}))
                self.assertEqual(get_state_as_dict(color_selector)["SelectedItemId"], "119")
                self.assertEqual(get_state_as_dict(color_selector)["SelectedItemPos"], "119")
                self.assertEqual(get_state_as_dict(color_selector)["SelectEntryText"], "Navy")

                # For freecolour-hlc colors
                select_by_text(xpaletteselector, "Freecolour HLC")
                # Select Color with id 987
                color_element988 = color_selector.getChild("987")
                color_element988.executeAction("SELECT", mkPropertyValues({}))
                self.assertEqual(get_state_as_dict(color_selector)["SelectedItemId"], "987")
                self.assertEqual(get_state_as_dict(color_selector)["SelectedItemPos"], "987")
                self.assertEqual(get_state_as_dict(color_selector)["VisibleCount"], "1032")
                self.assertEqual(get_state_as_dict(color_selector)["SelectEntryText"], "HLC 350 60 10")
                # Select Color with id 574
                color_element575 = color_selector.getChild("574")
                color_element575.executeAction("SELECT", mkPropertyValues({}))
                self.assertEqual(get_state_as_dict(color_selector)["SelectedItemId"], "574")
                self.assertEqual(get_state_as_dict(color_selector)["SelectedItemPos"], "574")
                self.assertEqual(get_state_as_dict(color_selector)["SelectEntryText"], "HLC 190 50 20")

                # For tonal colors
                select_by_text(xpaletteselector, "Tonal")
                # Select Color with id 16
                color_element17 = color_selector.getChild("16")
                color_element17.executeAction("SELECT", mkPropertyValues({}))
                self.assertEqual(get_state_as_dict(color_selector)["SelectedItemId"], "16")
                self.assertEqual(get_state_as_dict(color_selector)["SelectedItemPos"], "16")
                self.assertEqual(get_state_as_dict(color_selector)["VisibleCount"], "120")
                self.assertEqual(get_state_as_dict(color_selector)["SelectEntryText"], "Cyan 82%")
                # Select Color with id 12
                color_element13 = color_selector.getChild("12")
                color_element13.executeAction("SELECT", mkPropertyValues({}))
                self.assertEqual(get_state_as_dict(color_selector)["SelectedItemId"], "12")
                self.assertEqual(get_state_as_dict(color_selector)["SelectedItemPos"], "12")
                self.assertEqual(get_state_as_dict(color_selector)["SelectEntryText"], "Magenta 82%")

                # For material colors
                select_by_text(xpaletteselector, "Material")
                # Select Color with id 8
                color_element9 = color_selector.getChild("8")
                color_element9.executeAction("SELECT", mkPropertyValues({}))
                self.assertEqual(get_state_as_dict(color_selector)["SelectedItemId"], "8")
                self.assertEqual(get_state_as_dict(color_selector)["SelectedItemPos"], "8")
                self.assertEqual(get_state_as_dict(color_selector)["VisibleCount"], "228")
                self.assertEqual(get_state_as_dict(color_selector)["SelectEntryText"], "Gray 800")

                # For standard colors
                select_by_text(xpaletteselector, "Standard")
                # Select Color with id 2
                color_element3 = color_selector.getChild("2")
                color_element3.executeAction("SELECT", mkPropertyValues({}))
                self.assertEqual(get_state_as_dict(color_selector)["SelectedItemId"], "2")
                self.assertEqual(get_state_as_dict(color_selector)["SelectedItemPos"], "2")
                self.assertEqual(get_state_as_dict(color_selector)["VisibleCount"], "120")
                self.assertEqual(get_state_as_dict(color_selector)["SelectEntryText"], "Dark Gray 3")



    def test_recent_color_selector(self):

        #This is to test recent color selection
        with self.ui_test.create_doc_in_start_center("calc"):
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")
            #select cell A5
            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A5"}))
            #format - cell
            with self.ui_test.execute_dialog_through_command(".uno:FormatCellDialog") as xDialog:
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "6")  #tab Numbers
                # click on color btn
                xbtncolor = xDialog.getChild("btncolor")
                xbtncolor.executeAction("CLICK",tuple())

                # we will select color for cell A5 to be able to predict the latest color in
                # recent color selector
                xpaletteselector = xDialog.getChild("paletteselector")
                xColorpage = xDialog.getChild("ColorPage")
                color_selector = xColorpage.getChild("iconview_colors")

                # For chart-palettes colors
                select_by_text(xpaletteselector, "Chart Palettes")
                # Select Color with id 1
                color_element2 = color_selector.getChild("1")
                color_element2.executeAction("SELECT", mkPropertyValues({}))
                print("Available properties:", list(get_state_as_dict(color_selector).keys()))
                recent_state = get_state_as_dict(color_selector)
                print("Parth see the following:")
                for key, value in recent_state.items():
                    print(f"{key}: {value}")
                self.assertEqual(get_state_as_dict(color_selector)["SelectedItemId"], "1")
                self.assertEqual(get_state_as_dict(color_selector)["SelectedItemPos"], "1")
                self.assertEqual(get_state_as_dict(color_selector)["VisibleCount"], "12")
                self.assertEqual(get_state_as_dict(color_selector)["SelectEntryText"], "Chart 2")

                # close the dialog after selection of the color

            #select cell D3
            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "D3"}))
            #format - cell
            with self.ui_test.execute_dialog_through_command(".uno:FormatCellDialog") as xDialog:
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "6")  #tab Numbers
                # click on color btn
                xbtncolor = xDialog.getChild("btncolor")
                xbtncolor.executeAction("CLICK",tuple())

                xColorpage = xDialog.getChild("ColorPage")
                recent_color_selector = xColorpage.getChild("iconview_recent_colors")

                # Select Color with id 0
                recent_color_element1 = recent_color_selector.getChild("0")
                recent_color_element1.executeAction("SELECT", mkPropertyValues({}))
                self.assertEqual(get_state_as_dict(recent_color_selector)["SelectedItemId"], "0")
                self.assertEqual(get_state_as_dict(recent_color_selector)["SelectedItemPos"], "0")
                self.assertEqual(get_state_as_dict(recent_color_selector)["SelectEntryText"], "Chart 2")



# vim: set shiftwidth=4 softtabstop=4 expandtab:
