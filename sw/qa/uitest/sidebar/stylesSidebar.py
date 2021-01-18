# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file
from libreoffice.uno.propertyvalue import mkPropertyValues

class StylesSidebar(UITestCase):

   def test_load_styles_from_template(self):
        self.ui_test.create_doc_in_start_center("writer")

        self.ui_test.execute_dialog_through_command(".uno:LoadStyles")

        xDialog = self.xUITest.getTopFocusWindow()

        xText = xDialog.getChild("text")
        xNumbering = xDialog.getChild("numbering")
        xFrame = xDialog.getChild("frame")
        xPages = xDialog.getChild("pages")

        self.assertEqual('true', get_state_as_dict(xText)['Selected'])
        self.assertEqual('false', get_state_as_dict(xNumbering)['Selected'])
        self.assertEqual('false', get_state_as_dict(xFrame)['Selected'])
        self.assertEqual('false', get_state_as_dict(xPages)['Selected'])

        xNumbering.executeAction("CLICK", tuple())
        xFrame.executeAction("CLICK", tuple())
        xPages.executeAction("CLICK", tuple())

        self.assertEqual('true', get_state_as_dict(xText)['Selected'])
        self.assertEqual('true', get_state_as_dict(xNumbering)['Selected'])
        self.assertEqual('true', get_state_as_dict(xFrame)['Selected'])
        self.assertEqual('true', get_state_as_dict(xPages)['Selected'])

        xFileName = xDialog.getChild("fromfile")
        xFileName.executeAction("CLICK", tuple())

        xOpenDialog = self.xUITest.getTopFocusWindow()
        xFileName = xOpenDialog.getChild("file_name")
        xFileName.executeAction("TYPE", mkPropertyValues({"TEXT": get_url_for_data_file("customStyles.odt")}))

        xOpenBtn = xOpenDialog.getChild("open")
        xOpenBtn.executeAction("CLICK", tuple())

        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")

        self.xUITest.executeCommand(".uno:Sidebar")
        xWriterEdit.executeAction("SIDEBAR", mkPropertyValues({"PANEL": "StyleListPanel"}))

        xFilter = xWriterEdit.getChild('filter')
        xFilter.executeAction("SELECT", mkPropertyValues({"TEXT": "Custom Styles"}))

        expectedResults = ["customParagraphStyle", "customCharacterStyle", "customFrameStyle",
                "customPageStyle", "customNumberingStyle"]

        for i in range(5):
            xLeft = xWriterEdit.getChild('left')

            #change to another style type
            xLeft.executeAction("CLICK", mkPropertyValues({"POS": str( i + 1 )}))

            xFlatView = xWriterEdit.getChild("flatview")

            self.assertEqual(1, len(xFlatView.getChildren()))

            xFlatView.getChild('0').executeAction("SELECT", tuple())
            self.assertEqual(expectedResults[i], get_state_as_dict(xFlatView)['SelectEntryText'])

        self.xUITest.executeCommand(".uno:Sidebar")

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
