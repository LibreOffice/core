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
from libreoffice.uno.propertyvalue import mkPropertyValues

class dimensionLinesTabPage(UITestCase):

    def test_tdf171746_number_decimal_places_limit(self):
        with self.ui_test.create_doc_in_start_center("draw"):
            # Insert a dimension line using CTRL key
            xArgs = mkPropertyValues({"KeyModifier": 8192})
            self.xUITest.executeCommandWithParameters(".uno:MeasureLine", xArgs)

            # Open the dimension lines tab page
            with self.ui_test.execute_dialog_through_command(".uno:measureAttributes") as xDialog:
                # Increase decimal places 5 times (starting from default = 2)
                xDecimalPlaces = xDialog.getChild("MTR_FLD_DECIMALPLACES")
                for _ in range(5):
                    xDecimalPlaces.executeAction("UP", tuple())
                # Without the fix in place, this test would have failed with
                # AssertionError: '5' != '7'
                self.assertEqual(get_state_as_dict(xDecimalPlaces)["Text"], "5")

# vim: set shiftwidth=4 softtabstop=4 expandtab: