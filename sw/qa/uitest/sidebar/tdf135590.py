# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, select_pos
from uitest.uihelper.common import change_measurement_unit
from uitest.uihelper.common import select_by_text
from libreoffice.uno.propertyvalue import mkPropertyValues

class tdf135590(UITestCase):
    def test_tdf135590(self):
        writer_doc = self.ui_test.create_doc_in_start_center("writer")

        #change measurement to Centimeter
        change_measurement_unit(self, 'Centimeter')

        self.ui_test.execute_dialog_through_command(".uno:InsertEnvelope")

        xDialog = self.xUITest.getTopFocusWindow()

        tabcontrol = xDialog.getChild("tabcontrol")
        select_pos(tabcontrol, "1")

        xWidth = xDialog.getChild('width')
        xHeight = xDialog.getChild('height')
        xFormat = xDialog.getChild("format")

        select_by_text(xFormat, "C6 Envelope")

        self.assertEqual("16.2", get_state_as_dict(xWidth)['Value'])
        self.assertEqual("11.4", get_state_as_dict(xHeight)['Value'])

        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        # A new document is created
        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")

        self.ui_test.execute_dialog_through_command(".uno:PageDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        tabcontrol = xDialog.getChild("tabcontrol")
        select_pos(tabcontrol, "1")

        xWidth = xDialog.getChild('spinWidth')
        xHeight = xDialog.getChild('spinHeight')
        xFormatList = xDialog.getChild("comboPageFormat")

        # Without the fix in place, this test would have failed with
        # AssertionError: '16.2' != '11.4'
        self.assertEqual("16.2", get_state_as_dict(xWidth)['Value'])
        self.assertEqual("11.4", get_state_as_dict(xHeight)['Value'])
        self.assertEqual("User", get_state_as_dict(xFormatList)['SelectEntryText'])

        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        self.xUITest.executeCommand(".uno:Sidebar")
        xWriterEdit.executeAction("SIDEBAR", mkPropertyValues({"PANEL": "PageStylesPanel"}))

        xPaperSize = xWriterEdit.getChild('papersize')
        self.ui_test.wait_until_property_is_updated(xPaperSize, "SelectEntryText", "User")
        self.assertEqual(get_state_as_dict(xPaperSize)['SelectEntryText'], "User")

        xPaperHeight = xWriterEdit.getChild('paperheight')
        self.ui_test.wait_until_property_is_updated(xPaperHeight, "Text", "11.40 cm")
        self.assertEqual(get_state_as_dict(xPaperHeight)['Text'], "11.40 cm")

        xPaperWidth = xWriterEdit.getChild('paperwidth')
        self.ui_test.wait_until_property_is_updated(xPaperWidth, "Text", "16.20 cm")
        self.assertEqual(get_state_as_dict(xPaperWidth)['Text'], "16.20 cm")

        self.xUITest.executeCommand(".uno:Sidebar")

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:

