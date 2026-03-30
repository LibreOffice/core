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
from uitest.uihelper.common import select_by_text
from libreoffice.uno.propertyvalue import mkPropertyValues

class StylesSidebar(UITestCase):

    def test_load_styles_from_template(self):
        with self.ui_test.create_doc_in_start_center("writer"):

            with self.ui_test.execute_dialog_through_command(".uno:LoadStyles", close_button="") as xDialog:
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

                with self.ui_test.execute_dialog_through_action(xFileName, 'CLICK', close_button="open") as dialog:
                    xFileName = dialog.getChild("file_name")
                    xFileName.executeAction("TYPE", mkPropertyValues({"TEXT": get_url_for_data_file("customStyles.odt")}))

            xWriterDoc = self.xUITest.getTopFocusWindow()
            xWriterEdit = xWriterDoc.getChild("writer_edit")

            expectedResults = ["customParagraphStyle", "customCharacterStyle", "customFrameStyle",
                    "customPageStyle", "customNumberingStyle"]

            self.xUITest.executeCommand(".uno:Sidebar")

            xWriterEdit.executeAction("SIDEBAR", mkPropertyValues({"PANEL": "StyleListPanel"}))

            # wait until the template panel is available
            xTemplatePanel = self.ui_test.wait_until_child_is_available('TemplatePanel')

            # 'left' is the toolbar that contains the style family buttons
            xLeft = xTemplatePanel.getChild('left')

            for i in range(5):

                # change to another style type (style family)
                xLeft.executeAction("CLICK", mkPropertyValues({"POS": str( i )}))

                # open the style filters popover
                xToolBar = xTemplatePanel.getChild("stylefilterstoolbar")
                xToolBar.executeAction("CLICK", mkPropertyValues({"POS": "0"}))

                xFloatWindow = self.xUITest.getFloatWindow()
                xStyleFiltersPopover = xFloatWindow.getChild("container")
                xStyleFiltersTreeView = xStyleFiltersPopover.getChild("stylefilterstreeview")

                # click on 'All Styles' entry
                xEntry = xStyleFiltersTreeView.getChild("0")
                xEntry.executeAction("CLICK", tuple())

                # click on 'Custom Styles' entry
                xEntry = xStyleFiltersTreeView.getChild("3")
                xEntry.executeAction("CLICK", tuple())

                # close the style filters popover
                xToolBar = xTemplatePanel.getChild("stylefilterstoolbar")
                xToolBar.executeAction("CLICK", mkPropertyValues({"POS": "0"}))

                xFlatView = xTemplatePanel.getChild("flatview")

                self.assertEqual(1, len(xFlatView.getChildren()))

                xFlatView.getChild('0').executeAction("SELECT", tuple())
                self.ui_test.wait_until_property_is_updated(xFlatView, "SelectEntryText", expectedResults[i])
                self.assertEqual(expectedResults[i], get_state_as_dict(xFlatView)['SelectEntryText'])

            self.xUITest.executeCommand(".uno:Sidebar")


# vim: set shiftwidth=4 softtabstop=4 expandtab:
