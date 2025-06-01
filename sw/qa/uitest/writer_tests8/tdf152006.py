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

class tdf152006(UITestCase):

    def test_tdf_152006(self):
        with self.ui_test.load_file(get_url_for_data_file('tdf152006.fodt')):
            with self.ui_test.execute_dialog_through_command('.uno:PageDialog') as xDialog:
                xApply = xDialog.getChild('apply')

                xLinesGridRB = xDialog.getChild('radioRB_LINESGRID')
                xDisplayCB = xDialog.getChild('checkCB_DISPLAY')

                # Document starts in state with no grid, not displayed
                self.assertEqual('false', get_state_as_dict(xDisplayCB)['Selected'])

                xLinesGridRB.executeAction('CLICK', tuple())
                xDisplayCB.executeAction('CLICK', tuple())

                self.assertEqual('true', get_state_as_dict(xDisplayCB)['Selected'])

                # First apply: enable grid and display
                xApply.executeAction('CLICK', tuple())

            with self.ui_test.execute_dialog_through_command('.uno:PageDialog') as xDialog:
                xApply = xDialog.getChild('apply')

                xNoGridRB = xDialog.getChild('radioRB_NOGRID')
                xLinesGridRB = xDialog.getChild('radioRB_LINESGRID')
                xDisplayCB = xDialog.getChild('checkCB_DISPLAY')

                self.assertEqual('true', get_state_as_dict(xDisplayCB)['Selected']);

                xNoGridRB.executeAction('CLICK', tuple())

                # Second apply: disable grid only
                xApply.executeAction('CLICK', tuple())

                xLinesGridRB.executeAction('CLICK', tuple())

                self.assertEqual('true', get_state_as_dict(xDisplayCB)['Selected'])

                xDisplayCB.executeAction('CLICK', tuple())

                self.assertEqual('false', get_state_as_dict(xDisplayCB)['Selected'])

                # Third apply: enable grid and disable display
                xApply.executeAction('CLICK', tuple())

                xDisplayCB.executeAction('CLICK', tuple())

                # Fourth apply: enable display
                xApply.executeAction('CLICK', tuple())

            with self.ui_test.execute_dialog_through_command('.uno:PageDialog') as xDialog:
                xLinesGridRB = xDialog.getChild('radioRB_LINESGRID')
                xDisplayCB = xDialog.getChild('checkCB_DISPLAY')

                # The above steps should have enabled the grid and enabled display.
                # Without the fix, the display will still be disabled.
                self.assertEqual('true', get_state_as_dict(xDisplayCB)['Selected']);

# vim: set shiftwidth=4 softtabstop=4 expandtab:
