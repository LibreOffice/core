# -*- Mode: python; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest_helper import UITest

from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest_helper import get_state_as_dict

import time

from uitest.framework import UITestCase

try:
    import pyuno
    import uno
    import unohelper
except ImportError:
    print("pyuno not found: try to set PYTHONPATH and URE_BOOTSTRAP variables")
    print("PYTHONPATH=/installation/opt/program")
    print("URE_BOOTSTRAP=file:///installation/opt/program/fundamentalrc")
    raise

class SpinFieldTest(UITestCase):

    def test_up(self):

        self.ui_test.create_doc_in_start_center("calc")

        self.ui_test.execute_dialog_through_command(".uno:FormatCellDialog")
        xCellsDlg = self.xUITest.getTopFocusWindow()

        # select the numbers tab page
        xCellsDlg.executeAction("SELECT", mkPropertyValues({"POS": "0"}))
        
        xDecimalPlaces = xCellsDlg.getChild("leadzerosed")
        xDecimalPlaces.executeAction("UP", tuple())

        decimal_places_state = get_state_as_dict(xDecimalPlaces)
        assert(decimal_places_state["Text"] == "2")

        okBtn = xCellsDlg.getChild("ok")
        okBtn.executeAction("CLICK", tuple())

        self.ui_test.close_doc()

    def test_down(self):

        self.ui_test.create_doc_in_start_center("calc")

        self.ui_test.execute_dialog_through_command(".uno:FormatCellDialog")
        xCellsDlg = self.xUITest.getTopFocusWindow()

        # select the numbers tab page
        xCellsDlg.executeAction("SELECT", mkPropertyValues({"POS": "0"}))
        
        xDecimalPlaces = xCellsDlg.getChild("leadzerosed")
        xDecimalPlaces.executeAction("UP", tuple())
        xDecimalPlaces.executeAction("UP", tuple())

        decimal_places_state = get_state_as_dict(xDecimalPlaces)
        assert(decimal_places_state["Text"] == "3")
        
        xDecimalPlaces.executeAction("DOWN", tuple())

        decimal_places_state = get_state_as_dict(xDecimalPlaces)
        assert(decimal_places_state["Text"] == "2")

        okBtn = xCellsDlg.getChild("ok")
        okBtn.executeAction("CLICK", tuple())

        self.ui_test.close_doc()

    def test_text(self):

        self.ui_test.create_doc_in_start_center("calc")

        self.ui_test.execute_dialog_through_command(".uno:FormatCellDialog")
        xCellsDlg = self.xUITest.getTopFocusWindow()
        
        xDecimalPlaces = xCellsDlg.getChild("leadzerosed")
        xDecimalPlaces.executeAction("TYPE", mkPropertyValues({"TEXT": "4"}))

        decimal_places_state = get_state_as_dict(xDecimalPlaces)
        assert(decimal_places_state["Text"] == "41")

        okBtn = xCellsDlg.getChild("ok")
        okBtn.executeAction("CLICK", tuple())

        self.ui_test.close_doc()

# vim:set shiftwidth=4 softtabstop=4 expandtab: */
