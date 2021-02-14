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
        self.xUITest.executeCommandWithParameters(".uno:Rect_Unfilled", mkPropertyValues({"KeyModifier" : 0 }) )
        self.xUITest.executeCommandWithParameters(".uno:Rect", mkPropertyValues({"KeyModifier" : 0 }) )

        document = self.ui_test.get_component()
        # # AssertionError: '.uno:Rect_Unfilled' != '.uno:Rect'
        self.xUITest.executeCommandWithParameters(".uno:Rect_Unfilled", mkPropertyValues({"KeyModifier" : 0 }) )

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab: