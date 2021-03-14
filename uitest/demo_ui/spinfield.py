# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from libreoffice.uno.propertyvalue import mkPropertyValues

from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, type_text, select_pos

class SpinFieldTest(UITestCase):

    def test_up(self):

        self.ui_test.create_doc_in_start_center("calc")

        self.ui_test.execute_dialog_through_command(".uno:FormatCellDialog")
        xCellsDlg = self.xUITest.getTopFocusWindow()

        xDecimalPlaces = xCellsDlg.getChild("leadzerosed")
        xDecimalPlaces.executeAction("UP", tuple())

        decimal_places_state = get_state_as_dict(xDecimalPlaces)
        assert(decimal_places_state["Text"] == "2")

        okBtn = xCellsDlg.getChild("ok")
        self.ui_test.close_dialog_through_button(okBtn)

        self.ui_test.close_doc()

    def test_down(self):

        self.ui_test.create_doc_in_start_center("calc")

        self.ui_test.execute_dialog_through_command(".uno:FormatCellDialog")
        xCellsDlg = self.xUITest.getTopFocusWindow()

        xDecimalPlaces = xCellsDlg.getChild("leadzerosed")
        xDecimalPlaces.executeAction("UP", tuple())
        xDecimalPlaces.executeAction("UP", tuple())

        decimal_places_state = get_state_as_dict(xDecimalPlaces)
        assert(decimal_places_state["Text"] == "3")
        
        xDecimalPlaces.executeAction("DOWN", tuple())

        decimal_places_state = get_state_as_dict(xDecimalPlaces)
        assert(decimal_places_state["Text"] == "2")

        okBtn = xCellsDlg.getChild("ok")
        self.ui_test.close_dialog_through_button(okBtn)

        self.ui_test.close_doc()

    def test_text(self):

        self.ui_test.create_doc_in_start_center("calc")

        self.ui_test.execute_dialog_through_command(".uno:FormatCellDialog")
        xCellsDlg = self.xUITest.getTopFocusWindow()
        
        xDecimalPlaces = xCellsDlg.getChild("leadzerosed")
        type_text(xDecimalPlaces, "4")

        decimal_places_state = get_state_as_dict(xDecimalPlaces)
        assert(decimal_places_state["Text"] == "41")

        okBtn = xCellsDlg.getChild("ok")
        self.ui_test.close_dialog_through_button(okBtn)

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
