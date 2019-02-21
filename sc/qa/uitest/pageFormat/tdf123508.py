# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict
from uitest.uihelper.common import select_pos
from uitest.uihelper.calc import enter_text_to_cell
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict, type_text
import org.libreoffice.unotest
import pathlib

def get_url_for_data_file(file_name):
    return pathlib.Path(org.libreoffice.unotest.makeCopyFromTDOC(file_name)).as_uri()

#Bug 123508 - "Fit print range(s) to width/height" value falls back to default when window closes

class tdf123508(UITestCase):
   def test_tdf123508_format_page_scale(self):
    calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf123508.ods"))
    xCalcDoc = self.xUITest.getTopFocusWindow()
    gridwin = xCalcDoc.getChild("grid_window")
    document = self.ui_test.get_component()
    #open "Format > Page - Sheet > Scale
    self.ui_test.execute_dialog_through_command(".uno:PageFormatDialog")
    xDialog = self.xUITest.getTopFocusWindow()
    xTabs = xDialog.getChild("tabcontrol")
    select_pos(xTabs, "6")  #tab Scale
    scalingMode = xDialog.getChild("comboLB_SCALEMODE")
    spinEDSCALEPAGEWIDTH = xDialog.getChild("spinED_SCALEPAGEWIDTH")
    spinEDSCALEPAGEHEIGHT = xDialog.getChild("spinED_SCALEPAGEHEIGHT")
    #select "Fit print range(s) to width/height"  from the scale mode drop-down list
    props = {"TEXT": "Fit print range(s) to width/height"}
    actionProps = mkPropertyValues(props)
    scalingMode.executeAction("SELECT", actionProps)
    #define a value for the page, e.g.: width   2; height  2
    spinEDSCALEPAGEWIDTH.executeAction("UP", tuple())
    spinEDSCALEPAGEHEIGHT.executeAction("UP", tuple())

    xOKBtn = xDialog.getChild("ok")
    self.ui_test.close_dialog_through_button(xOKBtn)

    #verify
    self.ui_test.execute_dialog_through_command(".uno:PageFormatDialog")
    xDialog = self.xUITest.getTopFocusWindow()
    xTabs = xDialog.getChild("tabcontrol")
    select_pos(xTabs, "6")  #tab Scale
    scalingMode = xDialog.getChild("comboLB_SCALEMODE")
    spinEDSCALEPAGEWIDTH = xDialog.getChild("spinED_SCALEPAGEWIDTH")
    spinEDSCALEPAGEHEIGHT = xDialog.getChild("spinED_SCALEPAGEHEIGHT")

    self.assertEqual(get_state_as_dict(scalingMode)["SelectEntryText"], "Fit print range(s) to width/height")
    self.assertEqual(get_state_as_dict(spinEDSCALEPAGEWIDTH)["Text"], "2")
    self.assertEqual(get_state_as_dict(spinEDSCALEPAGEHEIGHT)["Text"], "2")

    xOKBtn = xDialog.getChild("ok")
    self.ui_test.close_dialog_through_button(xOKBtn)

    self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab:
