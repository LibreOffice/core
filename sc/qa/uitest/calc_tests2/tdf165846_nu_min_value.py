# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import select_by_text
from uitest.uihelper.common import get_state_as_dict
from libreoffice.uno.propertyvalue import mkPropertyValues

# Bug 165846 - Ensure degrees of freedom (nu value) is greater than 0
class tdf165846(UITestCase):
    def test_tdf165846_nu_min_value(self):
        with self.ui_test.create_doc_in_start_center("calc"):
            with self.ui_test.execute_modeless_dialog_through_command(".uno:RandomNumberGeneratorDialog") as xDialog:
                # Select chi squared distribution
                xDistributionCombo = xDialog.getChild("distribution-combo")
                select_by_text(xDistributionCombo, "Chi Squared")

                # Select an incorrect degrees of freedom (nu) value
                xParameter1Spin = xDialog.getChild("parameter1-spin")
                xParameter1Spin.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
                xParameter1Spin.executeAction("DOWN", tuple())

                # Without the fix in place, this test would have failed with
                # AssertionError: '0.0001' != '-1.0000'
                self.assertEqual("0.0001", get_state_as_dict(xParameter1Spin)["Text"])

# vim: set shiftwidth=4 softtabstop=4 expandtab:
