# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict
from libreoffice.uno.propertyvalue import mkPropertyValues

class tdf133189(UITestCase):
    def test_tdf133189(self):
        writer_doc = self.ui_test.create_doc_in_start_center("writer")

        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")

        self.xUITest.executeCommand(".uno:Sidebar")
        xWriterEdit.executeAction("SIDEBAR", mkPropertyValues({"PANEL": "PageStylesPanel"}))

        xPaperSize = xWriterEdit.getChild('papersize')
        xPaperWidth = xWriterEdit.getChild('paperwidth')
        xPaperHeight = xWriterEdit.getChild('paperheight')
        xPaperOrient = xWriterEdit.getChild('paperorientation')
        xPaperMargin = xWriterEdit.getChild('marginLB')

        #Check default page values
        self.assertEqual(get_state_as_dict(xPaperWidth)['Text'], "8.27″")
        self.assertEqual(get_state_as_dict(xPaperHeight)['Text'], "11.69″")
        self.assertEqual(get_state_as_dict(xPaperOrient)['SelectEntryText'], "Portrait")
        self.assertEqual(get_state_as_dict(xPaperSize)['SelectEntryText'], "A4")
        self.assertEqual(get_state_as_dict(xPaperMargin)['SelectEntryText'], "Normal (0.75″)")

        #set measurement to centimeters
        self.ui_test.execute_dialog_through_command(".uno:OptionsTreeDialog")
        xDialogOpt = self.xUITest.getTopFocusWindow()
        xPages = xDialogOpt.getChild("pages")
        xWriterEntry = xPages.getChild('3')
        xWriterEntry.executeAction("EXPAND", tuple())
        xWriterGeneralEntry = xWriterEntry.getChild('0')
        xWriterGeneralEntry.executeAction("SELECT", tuple())
        xMetric = xDialogOpt.getChild("metric")
        props = {"TEXT": "Centimeter"}
        actionProps = mkPropertyValues(props)
        xMetric.executeAction("SELECT", actionProps)
        xOKBtn = xDialogOpt.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        #wait until the sidebar is updated
        self.ui_test.wait_until_property_is_updated(xPaperWidth, "Text", "21.01 cm")
        self.assertEqual(get_state_as_dict(xPaperWidth)['Text'], "21.01 cm")
        self.assertEqual(get_state_as_dict(xPaperHeight)['Text'], "29.69 cm")
        self.assertEqual(get_state_as_dict(xPaperOrient)['SelectEntryText'], "Portrait")
        self.assertEqual(get_state_as_dict(xPaperSize)['SelectEntryText'], "A4")

        # tdf#129267
        self.assertEqual(get_state_as_dict(xPaperMargin)['SelectEntryText'], "Normal (1.90 cm)")

        self.xUITest.executeCommand(".uno:Sidebar")

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:

