# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

from uitest.framework import UITestCase
from uitest.uihelper.common import select_pos, select_by_text, get_state_as_dict

class tdf141957(UITestCase):

   def test_tdf_141957(self):
        with self.ui_test.create_doc_in_start_center("writer") as document:
            with self.ui_test.execute_dialog_through_command(".uno:PageDialog") as xDialog:
                tabcontrol = xDialog.getChild("tabcontrol")
                select_pos(tabcontrol, "1")

                xLandscape = xDialog.getChild('radiobuttonLandscape')
                self.assertEqual("false", get_state_as_dict(xLandscape)['Checked'])
                xLandscape.executeAction("CLICK", tuple())
                self.assertEqual("true", get_state_as_dict(xLandscape)['Checked'])

                xTextDirectionList = xDialog.getChild("comboTextFlowBox")
                self.assertEqual("Left-to-right (horizontal)", get_state_as_dict(xTextDirectionList)['SelectEntryText'])
                select_by_text(xTextDirectionList, "Right-to-left (vertical)")
                self.assertEqual("Right-to-left (vertical)", get_state_as_dict(xTextDirectionList)['SelectEntryText'])

                select_pos(tabcontrol, "9") #text grid
                xCharsPerLine = xDialog.getChild("spinNF_CHARSPERLINE")
                xLinesPerLine = xDialog.getChild("spinNF_LINESPERPAGE")
                self.assertEqual("24", get_state_as_dict(xCharsPerLine)['Text'])
                self.assertEqual("20", get_state_as_dict(xLinesPerLine)['Text'])

                select_pos(tabcontrol, "4") #header

                xHeaderOn = xDialog.getChild("checkHeaderOn")
                self.assertEqual(get_state_as_dict(xHeaderOn)["Selected"], "false")
                xHeaderOn.executeAction("CLICK", tuple())
                self.assertEqual(get_state_as_dict(xHeaderOn)["Selected"], "true")

                select_pos(tabcontrol, "9") #text grid
                xCharsPerLine = xDialog.getChild("spinNF_CHARSPERLINE")
                xLinesPerLine = xDialog.getChild("spinNF_LINESPERPAGE")

                # Without the fix in place, this test would have failed with
                # AssertionError: '22' != '24'
                self.assertEqual("22", get_state_as_dict(xCharsPerLine)['Text'])
                self.assertEqual("20", get_state_as_dict(xLinesPerLine)['Text'])

# vim: set shiftwidth=4 softtabstop=4 expandtab:
