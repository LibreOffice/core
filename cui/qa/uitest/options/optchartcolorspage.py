#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict

# Test for cui/source/options/optchart.cxx
class Test(UITestCase):

    def hex_to_rgb(self, hex):
        return str(tuple(int(hex[i:i+2], 16) for i in (0, 2, 4))).replace(" ", "")

    def test_options_charts_defaultcolors_chartcolors_colortable_color_match(self):

        with self.ui_test.execute_dialog_through_command(".uno:OptionsTreeDialog") as xDialog:
            pages = xDialog.getChild("pages")
            chartsEntry = pages.getChild('4')
            chartsEntry.executeAction("EXPAND", tuple())
            chartsEntryDefaultColorsEntry = chartsEntry.getChild('0')
            chartsEntryDefaultColorsEntry.executeAction("SELECT", tuple())

            chartsColors = xDialog.getChild("colors")
            colorTable = xDialog.getChild("table")

            # Chart Data Series default colors
            dataSeriesDefaultColors = ["004586",
                                       "ff420e",
                                       "ffd320",
                                       "579d1c",
                                       "7e0021",
                                       "83caff",
                                       "314004",
                                       "aecf00",
                                       "4b1f6f",
                                       "ff950e",
                                       "c5000b",
                                       "0084d1"]

            for i in range(len(dataSeriesDefaultColors)):
                chartsColorsDataSeriesEntry = chartsColors.getChild(i)
                chartsColorsDataSeriesEntry.executeAction("SELECT", tuple())
                self.assertEqual(get_state_as_dict(colorTable)["CurrColorPos"], str(i))
                self.assertEqual(get_state_as_dict(colorTable)["RGB"], self.hex_to_rgb(dataSeriesDefaultColors[i]))

# vim: set shiftwidth=4 softtabstop=4 expandtab:
