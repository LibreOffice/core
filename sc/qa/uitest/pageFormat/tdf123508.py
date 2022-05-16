# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import select_by_text, select_pos

from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file


# Bug 123508 - "Fit print range(s) to width/height" value falls back to default when window closes
class tdf123508(UITestCase):
   def test_tdf123508_format_page_scale(self):
    with self.ui_test.load_file(get_url_for_data_file("tdf123508.ods")) as calc_doc:
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        #open "Format > Page - Sheet > Scale
        with self.ui_test.execute_dialog_through_command(".uno:PageFormatDialog") as xDialog:
            xTabs = xDialog.getChild("tabcontrol")
            select_pos(xTabs, "6")  #tab Scale
            scalingMode = xDialog.getChild("comboLB_SCALEMODE")
            spinEDSCALEPAGEWIDTH = xDialog.getChild("spinED_SCALEPAGEWIDTH")
            spinEDSCALEPAGEHEIGHT = xDialog.getChild("spinED_SCALEPAGEHEIGHT")
            #select "Fit print range(s) to width/height"  from the scale mode drop-down list
            select_by_text(scalingMode, "Fit print range(s) to width/height")
            #define a value for the page, e.g.: width   2; height  2
            spinEDSCALEPAGEWIDTH.executeAction("UP", tuple())
            spinEDSCALEPAGEHEIGHT.executeAction("UP", tuple())


        #verify
        with self.ui_test.execute_dialog_through_command(".uno:PageFormatDialog", close_button="cancel") as xDialog:
            xTabs = xDialog.getChild("tabcontrol")
            select_pos(xTabs, "6")  #tab Scale
            scalingMode = xDialog.getChild("comboLB_SCALEMODE")
            spinEDSCALEPAGEWIDTH = xDialog.getChild("spinED_SCALEPAGEWIDTH")
            spinEDSCALEPAGEHEIGHT = xDialog.getChild("spinED_SCALEPAGEHEIGHT")

            self.assertEqual(get_state_as_dict(scalingMode)["SelectEntryText"], "Fit print range(s) to width/height")
            self.assertEqual(get_state_as_dict(spinEDSCALEPAGEWIDTH)["Text"], "2")
            self.assertEqual(get_state_as_dict(spinEDSCALEPAGEHEIGHT)["Text"], "2")


# vim: set shiftwidth=4 softtabstop=4 expandtab:
