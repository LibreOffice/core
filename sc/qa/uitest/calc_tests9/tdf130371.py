# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file

class namedRanges(UITestCase):

    def test_tdf130371(self):
        calc_doc =  self.ui_test.load_file(get_url_for_data_file("tdf130371.ods"))
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()

        text1 = "value\t$Sheet2.$B$2\tSheet2"
        text2 = "value\t$Sheet3.$B$2\tSheet3"
        text3 = "value\t$Sheet4.$B$2\tSheet4"

        self.ui_test.execute_dialog_through_command(".uno:DefineName")
        xDialog = self.xUITest.getTopFocusWindow()
        namesList = xDialog.getChild('names')
        self.assertEqual(2, len(namesList.getChildren()))
        self.assertEqual(get_state_as_dict(namesList.getChild('0'))["Text"], text1)
        self.assertEqual(get_state_as_dict(namesList.getChild('1'))["Text"], text2)

        xOkBtn = xDialog.getChild("ok")
        xOkBtn.executeAction("CLICK", tuple())

        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "B3"}))
        self.xUITest.executeCommand(".uno:Copy")

        self.xUITest.executeCommand(".uno:JumpToNextTable")
        self.xUITest.executeCommand(".uno:JumpToNextTable")

        self.xUITest.executeCommand(".uno:Paste")

        self.ui_test.execute_dialog_through_command(".uno:DefineName")
        xDialog = self.xUITest.getTopFocusWindow()
        namesList = xDialog.getChild('names')
        self.assertEqual(3, len(namesList.getChildren()))
        self.assertEqual(get_state_as_dict(namesList.getChild('0'))["Text"], text1)
        self.assertEqual(get_state_as_dict(namesList.getChild('1'))["Text"], text2)
        self.assertEqual(get_state_as_dict(namesList.getChild('2'))["Text"], text3)

        xOkBtn = xDialog.getChild("ok")
        xOkBtn.executeAction("CLICK", tuple())

        self.xUITest.executeCommand(".uno:Undo")

        self.ui_test.execute_dialog_through_command(".uno:DefineName")
        xDialog = self.xUITest.getTopFocusWindow()
        namesList = xDialog.getChild('names')
        self.assertEqual(2, len(namesList.getChildren()))
        self.assertEqual(get_state_as_dict(namesList.getChild('0'))["Text"], text1)
        self.assertEqual(get_state_as_dict(namesList.getChild('1'))["Text"], text2)

        xCancelBtn = xDialog.getChild("cancel")
        xCancelBtn.executeAction("CLICK", tuple())

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
