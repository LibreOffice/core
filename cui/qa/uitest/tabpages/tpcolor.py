#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.framework import UITestCase
from uitest.uihelper.common import select_pos
from uitest.uihelper.common import select_by_text
from uitest.uihelper.common import get_state_as_dict


# Test for cui/source/tabpages/tpcolor.cxx.
class Test(UITestCase):

    def testSvxColorTabPageTheme(self):
        # Given an Impress document with a theme:
        with self.ui_test.create_doc_in_start_center("impress") as component:
            template = self.xUITest.getTopFocusWindow()
            self.ui_test.close_dialog_through_button(template.getChild("close"))
            doc = self.xUITest.getTopFocusWindow()
            editWin = doc.getChild("impress_win")
            # Set theme colors.
            drawPage = component.getDrawPages()[0]
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
            master.ThemeUnoRepresentation = theme
            # Select the title shape.
            editWin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "TAB"}))

            # When using right click -> Area to refer to a theme for shape fill:
            with self.ui_test.execute_dialog_through_command(".uno:FormatArea") as xDialog:
                tabControl = xDialog.getChild("tabcontrol")
                # Area
                select_pos(tabControl, "0")
                # Color
                btnColor = xDialog.getChild("btncolor")
                btnColor.executeAction("CLICK", tuple())
                paletteSelector = xDialog.getChild("paletteselector")
                select_by_text(paletteSelector, "Theme colors")
                colorSelector = xDialog.getChild("iconview_colors")
                color_element = colorSelector.getChild("4")
                color_element.executeAction("SELECT", mkPropertyValues({}))
                self.assertEqual(get_state_as_dict(colorSelector)["SelectEntryText"], "Accent 1")

            # Then make sure the doc model is updated accordingly:
            shape = drawPage[0]
            # Without the accompanying fix in place, this test would have failed with:
            # AssertionError: -1 != 3
            # i.e. the theme metadata of the selected fill color was lost.
            self.assertEqual(shape.FillColorTheme, 3)


# vim: set shiftwidth=4 softtabstop=4 expandtab:
