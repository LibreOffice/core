# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file
from libreoffice.uno.propertyvalue import mkPropertyValues

class tdf114710(UITestCase):

    def test_tdf114710(self):
        calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf114710.ods"))
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()

        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:O7"}))
        self.xUITest.executeCommand(".uno:Copy")

        self.xUITest.executeCommand(".uno:CloseDoc")

        self.ui_test.create_doc_in_start_center("writer")

        self.xUITest.getTopFocusWindow()

        self.ui_test.execute_dialog_through_command(".uno:PasteSpecial")
        xDialog = self.xUITest.getTopFocusWindow()

        xList = xDialog.getChild('list')

        for childName in xList.getChildren():
            xChild = xList.getChild(childName)
            if get_state_as_dict(xChild)['Text'] == "Graphics Device Interface metafile (GDI)":
                break

        xChild.executeAction("SELECT", tuple())
        self.assertEqual(
                get_state_as_dict(xList)['SelectEntryText'], "Graphics Device Interface metafile (GDI)")

        xOkBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOkBtn)

        # Without the fix in place, this test would have crashed here
        document = self.ui_test.get_component()
        self.assertEqual(1, document.GraphicObjects.getCount())

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:

