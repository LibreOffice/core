# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.uihelper.common import get_state_as_dict
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import change_measurement_unit
from uitest.framework import UITestCase

class Tdf82616(UITestCase):

    def test_tdf82616(self):
        self.ui_test.create_doc_in_start_center("impress")

        xTemplateDlg = self.xUITest.getTopFocusWindow()
        xCancelBtn = xTemplateDlg.getChild("close")
        self.ui_test.close_dialog_through_button(xCancelBtn)

        change_measurement_unit(self, 'Centimeter')

        xImpressDoc = self.xUITest.getTopFocusWindow()

        document = self.ui_test.get_component()
        self.assertEqual(1400, document.DrawPages[0].getByIndex(0).Position.X)
        self.assertEqual(628, document.DrawPages[0].getByIndex(0).Position.Y)
        self.assertEqual(1400, document.DrawPages[0].getByIndex(1).Position.X)
        self.assertEqual(3685, document.DrawPages[0].getByIndex(1).Position.Y)

        self.assertIsNone(document.CurrentSelection)

        xEditWin = xImpressDoc.getChild("impress_win")
        xEditWin.executeAction("SELECT", mkPropertyValues({"OBJECT":"Unnamed Drawinglayer object 1"}))
        self.assertEqual("com.sun.star.drawing.SvxShapeCollection", document.CurrentSelection.getImplementationName())

        xEditWin.executeAction("SIDEBAR", mkPropertyValues({"PANEL": "PosSizePropertyPanel"}))

        xHorizontalPos = xImpressDoc.getChild("horizontalpos")
        self.ui_test.wait_until_property_is_updated(xHorizontalPos, "Value", "1.4")
        self.assertEqual("1.4", get_state_as_dict(xHorizontalPos)['Value'])

        xVerticalPos = xImpressDoc.getChild("verticalpos")
        self.ui_test.wait_until_property_is_updated(xVerticalPos, "Value", "3.69")
        self.assertEqual("3.69", get_state_as_dict(xVerticalPos)['Value'])

        xDrawinglayerObject = xEditWin.getChild("Unnamed Drawinglayer object 1")
        xDrawinglayerObject.executeAction("MOVE", mkPropertyValues({"X": "-5000", "Y":"-10000"}))

        self.assertEqual(1400, document.DrawPages[0].getByIndex(0).Position.X)
        self.assertEqual(628, document.DrawPages[0].getByIndex(0).Position.Y)
        self.assertEqual(-3600, document.DrawPages[0].getByIndex(1).Position.X)
        self.assertEqual(-6315, document.DrawPages[0].getByIndex(1).Position.Y)

        # Without the fix in place, this test would have failed with
        # AssertionError: '-3.6' != '0'
        self.ui_test.wait_until_property_is_updated(xHorizontalPos, "Value", "-3.6")
        self.assertEqual("-3.6", get_state_as_dict(xHorizontalPos)['Value'])

        self.ui_test.wait_until_property_is_updated(xVerticalPos, "Value", "-6.32")
        self.assertEqual("-6.32", get_state_as_dict(xVerticalPos)['Value'])

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
