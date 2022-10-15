# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, type_text

class SpinFieldTest(UITestCase):

    def test_up(self):

        with self.ui_test.create_doc_in_start_center("calc"):

            with self.ui_test.execute_dialog_through_command(".uno:FormatCellDialog") as xCellsDlg:

                xDecimalPlaces = xCellsDlg.getChild("leadzerosed")
                xDecimalPlaces.executeAction("UP", tuple())

                decimal_places_state = get_state_as_dict(xDecimalPlaces)
                assert(decimal_places_state["Text"] == "2")



    def test_down(self):

        with self.ui_test.create_doc_in_start_center("calc"):

            with self.ui_test.execute_dialog_through_command(".uno:FormatCellDialog") as xCellsDlg:

                xDecimalPlaces = xCellsDlg.getChild("leadzerosed")
                xDecimalPlaces.executeAction("UP", tuple())
                xDecimalPlaces.executeAction("UP", tuple())

                decimal_places_state = get_state_as_dict(xDecimalPlaces)
                assert(decimal_places_state["Text"] == "3")
        
                xDecimalPlaces.executeAction("DOWN", tuple())

                decimal_places_state = get_state_as_dict(xDecimalPlaces)
                assert(decimal_places_state["Text"] == "2")



    def test_text(self):

        with self.ui_test.create_doc_in_start_center("calc"):

            with self.ui_test.execute_dialog_through_command(".uno:FormatCellDialog") as xCellsDlg:
        
                xDecimalPlaces = xCellsDlg.getChild("leadzerosed")
                type_text(xDecimalPlaces, "4")

                decimal_places_state = get_state_as_dict(xDecimalPlaces)
                assert(decimal_places_state["Text"] == "41")



# vim: set shiftwidth=4 softtabstop=4 expandtab:
