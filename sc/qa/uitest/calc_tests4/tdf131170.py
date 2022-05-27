# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file

class tdf131170(UITestCase):
    def test_DefineLabelRange(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf131170.ods")):

            with self.ui_test.execute_dialog_through_command(".uno:DefineLabelRange") as xDialog:

                xRange = xDialog.getChild("range")
                self.assertEqual(4, len(xRange.getChildren()))
                self.assertEqual(get_state_as_dict(xRange.getChild('0'))["Text"].strip(), "--- Column ---")
                self.assertEqual(get_state_as_dict(xRange.getChild('1'))["Text"].strip(), "$Sheet1.$I$6:$K$6 [AA, BB, CC]")
                self.assertEqual(get_state_as_dict(xRange.getChild('2'))["Text"].strip(), "--- Row ---")
                self.assertEqual(get_state_as_dict(xRange.getChild('3'))["Text"].strip(), "$Sheet1.$H$7:$H$9 [X, Y, Z]")

                xDeleteBtn = xDialog.getChild("delete")

                xRange.getChild('1').executeAction("SELECT", tuple())
                with self.ui_test.execute_blocking_action(xDeleteBtn.executeAction, args=('CLICK', ()), close_button="yes"):
                    pass

                self.assertEqual(3, len(xRange.getChildren()))
                self.assertEqual(get_state_as_dict(xRange.getChild('0'))["Text"].strip(), "--- Column ---")
                self.assertEqual(get_state_as_dict(xRange.getChild('1'))["Text"].strip(), "--- Row ---")
                self.assertEqual(get_state_as_dict(xRange.getChild('2'))["Text"].strip(), "$Sheet1.$H$7:$H$9 [X, Y, Z]")

                xRange.getChild('2').executeAction("SELECT", tuple())
                with self.ui_test.execute_blocking_action(xDeleteBtn.executeAction, args=('CLICK', ()), close_button="yes"):
                    pass

                self.assertEqual(2, len(xRange.getChildren()))
                self.assertEqual(get_state_as_dict(xRange.getChild('0'))["Text"].strip(), "--- Column ---")
                self.assertEqual(get_state_as_dict(xRange.getChild('1'))["Text"].strip(), "--- Row ---")


# vim: set shiftwidth=4 softtabstop=4 expandtab:
