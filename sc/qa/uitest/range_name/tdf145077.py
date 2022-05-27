# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file
from uitest.uihelper.common import select_by_text

class tdf145077(UITestCase):

    def test_tdf145077(self):

        with self.ui_test.load_file(get_url_for_data_file("tdf145077.ods")):

            calcDoc = self.xUITest.getTopFocusWindow()
            gridwin = calcDoc.getChild("grid_window")

            xPosWindow = calcDoc.getChild('pos_window')

            rangeList = ["Players (Team1)", "Points (Team1)", "Winners (Team1)"]
            expectedSelection = ["Team1.D1:Team1.R1", "Team1.D10:Team1.R10", "Team1.B2:Team1.B9"]

            # Without the fix in place, this test would have failed with
            # AssertionError: 'Team1.D10:Team1.R10' != 'Team1.D1:Team1.R1'
            for index, rangeName in enumerate(rangeList):
                gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "B10"}))

                self.assertEqual("Team1.B10:Team1.B10", get_state_as_dict(gridwin)["MarkedArea"])

                select_by_text(xPosWindow, rangeName)

                # Check selection
                self.assertEqual(expectedSelection[index], get_state_as_dict(gridwin)["MarkedArea"])

# vim: set shiftwidth=4 softtabstop=4 expandtab:
