#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict
import importlib

class tdf123841(UITestCase):

    def test_tdf123841(self):

        MainDoc = self.ui_test.create_doc_in_start_center("draw")
        MainWindow = self.xUITest.getTopFocusWindow()

        self.xUITest.executeCommand(".uno:BasicShapes.rectangle")
        SVX_HID_POPUP_COLOR_CTRL = MainWindow.getChild("SVX_HID_POPUP_COLOR_CTRL")
        colorset = SVX_HID_POPUP_COLOR_CTRL.getChild("colorset")
        colorset.executeAction("CHOOSE", mkPropertyValues({"POS": "12"}))
        self.xUITest.executeCommandWithParameters(".uno:FillColor", mkPropertyValues({"FillColor" : 16777215 }) )

        document = self.ui_test.get_component()
        # Without the patch in place, this test would have failed with
        # AssertionError: '7512015' != '16777215'
        self.assertEqual(
          hex(document.DrawPages.getByIndex(0).Background.FillGradient.StartColor), '16777215')
        # Without the accompanying fix in place, it would crash at this point
        self.xUITest.executeCommand(".uno:Rect_Unfilled")

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab: