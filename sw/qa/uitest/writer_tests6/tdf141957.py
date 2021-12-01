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

class tdf141957(UITestCase):

    def test_tdf_141957(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf141957.odt")):
            with self.ui_test.execute_dialog_through_command(".uno:PageDialog") as xDialog:
                tabcontrol = xDialog.getChild("tabcontrol")

                xLandscape = xDialog.getChild('radiobuttonLandscape')
                self.assertEqual("true", get_state_as_dict(xLandscape)['Checked'])

                xTextDirectionList = xDialog.getChild("comboTextFlowBox")
                self.assertEqual("Right-to-left (vertical)", get_state_as_dict(xTextDirectionList)['SelectEntryText'])

                xHeaderOn = xDialog.getChild("checkHeaderOn")
                self.assertEqual(get_state_as_dict(xHeaderOn)["Selected"], "true")

                xCharsPerLine = xDialog.getChild("spinNF_CHARSPERLINE")
                xLinesPerLine = xDialog.getChild("spinNF_LINESPERPAGE")

                # Without the fix in place, this test would have failed with
                # AssertionError: '21' != '24'
                self.assertEqual("21", get_state_as_dict(xCharsPerLine)['Text'])
                self.assertEqual("20", get_state_as_dict(xLinesPerLine)['Text'])

# vim: set shiftwidth=4 softtabstop=4 expandtab:
