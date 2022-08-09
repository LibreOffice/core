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
from uitest.uihelper.common import change_measurement_unit
from uitest.uihelper.common import get_url_for_data_file

class tdf119246(UITestCase):

    def test_tdf119246(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf119246.odp")) as document:

            change_measurement_unit(self, "Centimeter")

            xImpressDoc = self.xUITest.getTopFocusWindow()

            self.assertIsNone(document.CurrentSelection)

            xEditWin = xImpressDoc.getChild("impress_win")
            xEditWin.executeAction("SELECT", mkPropertyValues({"OBJECT":"DimensionShape"}))
            self.assertEqual("com.sun.star.drawing.SvxShapeCollection", document.CurrentSelection.getImplementationName())

            with self.ui_test.execute_dialog_through_command(".uno:MeasureAttributes") as xDialog:
                xLineDist = xDialog.getChild('MTR_LINE_DIST')
                xGuideOvergang = xDialog.getChild('MTR_FLD_HELPLINE_OVERHANG')
                xGuideDist = xDialog.getChild('MTR_FLD_HELPLINE_DIST')
                xLeftDist = xDialog.getChild('MTR_FLD_HELPLINE1_LEN')
                xRightDist = xDialog.getChild('MTR_FLD_HELPLINE2_LEN')
                xDecimalPlace = xDialog.getChild('MTR_FLD_DECIMALPLACES')

                # Without the fix in place, this test would have failed with
                # AssertionError: '2.80 cm' != '1.00 cm'
                self.assertEqual("2.80 cm", get_state_as_dict(xLineDist)['Text'])
                self.assertEqual("0.20 cm", get_state_as_dict(xGuideOvergang)['Text'])
                self.assertEqual("0.50 cm", get_state_as_dict(xGuideDist)['Text'])
                self.assertEqual("4.00 cm", get_state_as_dict(xLeftDist)['Text'])
                self.assertEqual("-1.40 cm", get_state_as_dict(xRightDist)['Text'])
                self.assertEqual("2", get_state_as_dict(xDecimalPlace)['Text'])

# vim: set shiftwidth=4 softtabstop=4 expandtab:
