#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from libreoffice.uno.propertyvalue import convert_property_values_to_dict
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict
from uitest.uihelper.common import select_pos
from uitest.uihelper.common import select_by_text

# Test for cui/source/tabpages/themepage.cxx.
class Test(UITestCase):

    def testThemePage(self):
        # Given an Impress document with a master page that has a theme:
        with self.ui_test.create_doc_in_start_center("impress") as component:
            template = self.xUITest.getTopFocusWindow()
            self.ui_test.close_dialog_through_button(template.getChild("close"))
            doc = self.xUITest.getTopFocusWindow()
            editWin = doc.getChild("impress_win")
            drawPage = component.getDrawPages().getByIndex(0)
            master = drawPage.MasterPage
            theme = mkPropertyValues({
                "Name": "nameA",
                "ColorSchemeName": "colorSetA",
                "ColorScheme": tuple([
                    0x000000,  # dk1
                    0x000000,  # lt1
                    0x000000,  # dk2
                    0x000000,  # lt2
                    0x0000ff,  # accent1
                    0x000000,  # accent2
                    0x000000,  # accent3
                    0x000000,  # accent4
                    0x000000,  # accent5
                    0x000000,  # accent6
                    0x000000,  # hlink
                    0x000000,  # folHlink
                ])
            })
            master.Theme = theme

            # When changing the name of the theme:
            self.xUITest.executeCommand(".uno:SlideMasterPage")
            with self.ui_test.execute_dialog_through_command(".uno:PageSetup") as xDialog:
                xTabs = xDialog.getChild("tabcontrol")
                # Select RID_SVXPAGE_THEME.
                select_pos(xTabs, "3")
                themeName = xDialog.getChild("themeName")
                themeName.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
                themeName.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
                themeName.executeAction("TYPE", mkPropertyValues({"TEXT": "nameB"}))
                colorSetName = xDialog.getChild("colorSetName")
                colorSetName.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
                colorSetName.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
                colorSetName.executeAction("TYPE", mkPropertyValues({"TEXT": "colorSetB"}))

                # Select a custom accent1 color.
                accent1 = xDialog.getChild("btnAccent1")
                accent1.executeAction("OPENLIST", tuple())
                floatWindow = self.xUITest.getFloatWindow()
                paletteSelector = floatWindow.getChild("palette_listbox")
                select_by_text(paletteSelector, "chart-palettes")
                colorSet = floatWindow.getChild("colorset")
                colorSet.executeAction("CHOOSE", mkPropertyValues({"POS": "2"}))

            # Then make sure the doc model is updated accordingly:
            # Without the accompanying fix in place, this test would have failed with:
            # AssertionError: 'nameA' != 'nameB'
            # i.e. the UI didn't update the theme name.
            theme = convert_property_values_to_dict(master.Theme)
            self.assertEqual(theme["Name"], "nameB")
            # Without the accompanying fix in place, this test would have failed with:
            # AssertionError: 'colorSetA' != 'colorSetB'
            # i.e. the UI didn't update the color scheme name.
            self.assertEqual(theme["ColorSchemeName"], "colorSetB")
            colorSet = theme["ColorScheme"]
            # Without the accompanying fix in place, this test would have failed with:
            # AssertionError: 0 != 16728590 (#ff420e)
            # i.e. the UI didn't update the accent1 color from black to a custom value.
            self.assertEqual(colorSet[4], 0xff420e)


# vim: set shiftwidth=4 softtabstop=4 expandtab:
