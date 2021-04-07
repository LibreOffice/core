# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
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
        calc_doc = self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        #select cell A1
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
        #format - cell
        self.ui_test.execute_dialog_through_command(".uno:FormatCellDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        xTabs = xDialog.getChild("tabcontrol")
        select_pos(xTabs, "6")  #tab Numbers
        # click on color btn
        xbtncolor = xDialog.getChild("btncolor")
        xbtncolor.executeAction("CLICK",tuple())
        xpaletteselector = xDialog.getChild("paletteselector")

        # Now we have the ColorPage that we can get the color selector from it
        xColorpage = xDialog.getChild("ColorPage")
        color_selector = xColorpage.getChild("colorset")

        # For chart-palettes colors
        select_by_text(xpaletteselector, "chart-palettes")
        # Select Color with id 2
        color_selector.executeAction("CHOOSE", mkPropertyValues({"POS": "2"}))
        self.assertEqual(get_state_as_dict(color_selector)["CurrColorId"], "2")
        self.assertEqual(get_state_as_dict(color_selector)["CurrColorPos"], "1")
        self.assertEqual(get_state_as_dict(color_selector)["ColorsCount"], "12")
        self.assertEqual(get_state_as_dict(color_selector)["ColCount"], "12")
        self.assertEqual(get_state_as_dict(color_selector)["ColorText"], "Chart 2")
        self.assertEqual(get_state_as_dict(color_selector)["RGB"], "(255,66,14)")

        # Select Color with id 5
        color_selector.executeAction("CHOOSE", mkPropertyValues({"POS": "5"}))
        self.assertEqual(get_state_as_dict(color_selector)["CurrColorId"], "5")
        self.assertEqual(get_state_as_dict(color_selector)["CurrColorPos"], "4")
        self.assertEqual(get_state_as_dict(color_selector)["ColorText"], "Chart 5")
        self.assertEqual(get_state_as_dict(color_selector)["RGB"], "(126,0,33)")

        # For libreoffice colors
        select_by_text(xpaletteselector, "libreoffice")
        # Select Color with id 6
        color_selector.executeAction("CHOOSE", mkPropertyValues({"POS": "6"}))
        self.assertEqual(get_state_as_dict(color_selector)["CurrColorId"], "6")
        self.assertEqual(get_state_as_dict(color_selector)["CurrColorPos"], "5")
        self.assertEqual(get_state_as_dict(color_selector)["ColorsCount"], "32")
        self.assertEqual(get_state_as_dict(color_selector)["ColCount"], "12")
        self.assertEqual(get_state_as_dict(color_selector)["ColorText"], "Green Accent")
        self.assertEqual(get_state_as_dict(color_selector)["RGB"], "(44,238,14)")

        # Select Color with id 30
        color_selector.executeAction("CHOOSE", mkPropertyValues({"POS": "30"}))
        self.assertEqual(get_state_as_dict(color_selector)["CurrColorId"], "30")
        self.assertEqual(get_state_as_dict(color_selector)["CurrColorPos"], "29")
        self.assertEqual(get_state_as_dict(color_selector)["ColorText"], "Yellow Accent")
        self.assertEqual(get_state_as_dict(color_selector)["RGB"], "(255,215,76)")

        # For html colors
        select_by_text(xpaletteselector, "html")
        # Select Color with id 1
        color_selector.executeAction("CHOOSE", mkPropertyValues({"POS": "1"}))
        self.assertEqual(get_state_as_dict(color_selector)["CurrColorId"], "1")
        self.assertEqual(get_state_as_dict(color_selector)["CurrColorPos"], "0")
        self.assertEqual(get_state_as_dict(color_selector)["ColorsCount"], "139")
        self.assertEqual(get_state_as_dict(color_selector)["ColCount"], "12")
        self.assertEqual(get_state_as_dict(color_selector)["ColorText"], "White")
        # Select Color with id 120
        color_selector.executeAction("CHOOSE", mkPropertyValues({"POS": "120"}))
        self.assertEqual(get_state_as_dict(color_selector)["CurrColorId"], "120")
        self.assertEqual(get_state_as_dict(color_selector)["CurrColorPos"], "119")
        self.assertEqual(get_state_as_dict(color_selector)["ColorText"], "Navy")

        # For freecolour-hlc colors
        select_by_text(xpaletteselector, "freecolour-hlc")
        # Select Color with id 988
        color_selector.executeAction("CHOOSE", mkPropertyValues({"POS": "988"}))
        self.assertEqual(get_state_as_dict(color_selector)["CurrColorId"], "988")
        self.assertEqual(get_state_as_dict(color_selector)["CurrColorPos"], "987")
        self.assertEqual(get_state_as_dict(color_selector)["ColorsCount"], "1032")
        self.assertEqual(get_state_as_dict(color_selector)["ColCount"], "12")
        self.assertEqual(get_state_as_dict(color_selector)["ColorText"], "HLC 350 60 10")
        # Select Color with id 575
        color_selector.executeAction("CHOOSE", mkPropertyValues({"POS": "575"}))
        self.assertEqual(get_state_as_dict(color_selector)["CurrColorId"], "575")
        self.assertEqual(get_state_as_dict(color_selector)["CurrColorPos"], "574")
        self.assertEqual(get_state_as_dict(color_selector)["ColorText"], "HLC 190 50 20")

        # For tonal colors
        select_by_text(xpaletteselector, "tonal")
        # Select Color with id 17
        color_selector.executeAction("CHOOSE", mkPropertyValues({"POS": "17"}))
        self.assertEqual(get_state_as_dict(color_selector)["CurrColorId"], "17")
        self.assertEqual(get_state_as_dict(color_selector)["CurrColorPos"], "16")
        self.assertEqual(get_state_as_dict(color_selector)["ColorsCount"], "120")
        self.assertEqual(get_state_as_dict(color_selector)["ColCount"], "12")
        self.assertEqual(get_state_as_dict(color_selector)["ColorText"], "Cyan 82%")
        # Select Color with id 13
        color_selector.executeAction("CHOOSE", mkPropertyValues({"POS": "13"}))
        self.assertEqual(get_state_as_dict(color_selector)["CurrColorId"], "13")
        self.assertEqual(get_state_as_dict(color_selector)["CurrColorPos"], "12")
        self.assertEqual(get_state_as_dict(color_selector)["ColorText"], "Magenta 82%")

        # For material colors
        select_by_text(xpaletteselector, "material")
        # Select Color with id 9
        color_selector.executeAction("CHOOSE", mkPropertyValues({"POS": "9"}))
        self.assertEqual(get_state_as_dict(color_selector)["CurrColorId"], "9")
        self.assertEqual(get_state_as_dict(color_selector)["CurrColorPos"], "8")
        self.assertEqual(get_state_as_dict(color_selector)["ColorsCount"], "228")
        self.assertEqual(get_state_as_dict(color_selector)["ColCount"], "12")
        self.assertEqual(get_state_as_dict(color_selector)["ColorText"], "Gray 800")

        # For standard colors
        select_by_text(xpaletteselector, "standard")
        # Select Color with id 3
        color_selector.executeAction("CHOOSE", mkPropertyValues({"POS": "3"}))
        self.assertEqual(get_state_as_dict(color_selector)["CurrColorId"], "3")
        self.assertEqual(get_state_as_dict(color_selector)["CurrColorPos"], "2")
        self.assertEqual(get_state_as_dict(color_selector)["ColorsCount"], "120")
        self.assertEqual(get_state_as_dict(color_selector)["ColCount"], "12")
        self.assertEqual(get_state_as_dict(color_selector)["ColorText"], "Dark Gray 3")

        xOk = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOk)

        self.ui_test.close_doc()

    def test_recent_color_selector(self):

        #This is to test recent color selection
        calc_doc = self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        #select cell A5
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A5"}))
        #format - cell
        self.ui_test.execute_dialog_through_command(".uno:FormatCellDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        xTabs = xDialog.getChild("tabcontrol")
        select_pos(xTabs, "6")  #tab Numbers
        # click on color btn
        xbtncolor = xDialog.getChild("btncolor")
        xbtncolor.executeAction("CLICK",tuple())

        # we will select color for cell A5 to be able to predict the latest color in
        # recent color selector
        xpaletteselector = xDialog.getChild("paletteselector")
        xColorpage = xDialog.getChild("ColorPage")
        color_selector = xColorpage.getChild("colorset")

        # For chart-palettes colors
        select_by_text(xpaletteselector, "chart-palettes")
        # Select Color with id 2
        color_selector.executeAction("CHOOSE", mkPropertyValues({"POS": "2"}))
        self.assertEqual(get_state_as_dict(color_selector)["CurrColorId"], "2")
        self.assertEqual(get_state_as_dict(color_selector)["CurrColorPos"], "1")
        self.assertEqual(get_state_as_dict(color_selector)["ColorsCount"], "12")
        self.assertEqual(get_state_as_dict(color_selector)["ColCount"], "12")
        self.assertEqual(get_state_as_dict(color_selector)["ColorText"], "Chart 2")
        xrgb = get_state_as_dict(color_selector)["RGB"]

        # close the dialog after selection of the color
        xOk = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOk)

        #select cell D3
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "D3"}))
        #format - cell
        self.ui_test.execute_dialog_through_command(".uno:FormatCellDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        xTabs = xDialog.getChild("tabcontrol")
        select_pos(xTabs, "6")  #tab Numbers
        # click on color btn
        xbtncolor = xDialog.getChild("btncolor")
        xbtncolor.executeAction("CLICK",tuple())

        xColorpage = xDialog.getChild("ColorPage")
        recent_color_selector = xColorpage.getChild("recentcolorset")

        # Select Color with id 1
        recent_color_selector.executeAction("CHOOSE", mkPropertyValues({"POS": "1"}))
        self.assertEqual(get_state_as_dict(recent_color_selector)["CurrColorId"], "1")
        self.assertEqual(get_state_as_dict(recent_color_selector)["CurrColorPos"], "0")
        self.assertEqual(get_state_as_dict(recent_color_selector)["ColorText"], "Chart 2")
        self.assertEqual(get_state_as_dict(recent_color_selector)["RGB"], xrgb)

        xOk = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOk)

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
