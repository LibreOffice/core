# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import change_measurement_unit
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file
from libreoffice.uno.propertyvalue import mkPropertyValues

class tdf137446(UITestCase):

    def test_tdf137446(self):

        calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf137446.ods"))
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")

        change_measurement_unit(self, "Centimeter")

        gridwin.executeAction("SELECT", mkPropertyValues({"OBJECT": "lineObject"}))

        self.ui_test.execute_dialog_through_command(".uno:TransformDialog")

        xDialog = self.xUITest.getTopFocusWindow()

        self.assertEqual('10', get_state_as_dict(xDialog.getChild('MTR_FLD_WIDTH'))['Value'])
        self.assertEqual('0', get_state_as_dict(xDialog.getChild('MTR_FLD_HEIGHT'))['Value'])
        self.assertEqual('5.51', get_state_as_dict(xDialog.getChild('MTR_FLD_POS_X'))['Value'])
        self.assertEqual('2.55', get_state_as_dict(xDialog.getChild('MTR_FLD_POS_Y'))['Value'])
        self.assertEqual('0', get_state_as_dict(xDialog.getChild('NF_ANGLE'))['Value'])


        xDialog.getChild('MTR_FLD_WIDTH').executeAction("UP", tuple())

        xOkBtn = xDialog.getChild("ok")
        xOkBtn.executeAction("CLICK", tuple())

        self.ui_test.execute_dialog_through_command(".uno:TransformDialog")

        xDialog = self.xUITest.getTopFocusWindow()

        self.assertEqual('10.1', get_state_as_dict(xDialog.getChild('MTR_FLD_WIDTH'))['Value'])
        self.assertEqual('0', get_state_as_dict(xDialog.getChild('MTR_FLD_HEIGHT'))['Value'])
        self.assertEqual('5.51', get_state_as_dict(xDialog.getChild('MTR_FLD_POS_X'))['Value'])
        self.assertEqual('2.55', get_state_as_dict(xDialog.getChild('MTR_FLD_POS_Y'))['Value'])

        # Without the fix in place, this test would have failed with AssertionError: '0' != '180'
        self.assertEqual('0', get_state_as_dict(xDialog.getChild('NF_ANGLE'))['Value'])

        xOkBtn = xDialog.getChild("ok")
        xOkBtn.executeAction("CLICK", tuple())

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
