# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import get_url_for_data_file
from libreoffice.uno.propertyvalue import mkPropertyValues
from libreoffice.calc.document import get_cell_by_position
from uitest.uihelper.calc import enter_text_to_cell
from uitest.uihelper.common import get_state_as_dict, select_pos

class tdf156611(UITestCase):
    def test_tdf156611_insert_hyperlink_like_excel(self):
        # The hyperlink interoperability setting, that this test is testing,
        # works only if MS document type is opened.
        # but it does not need any data from the file, any xlsx or xls file can be opened for this test
        with self.ui_test.load_file(get_url_for_data_file("tdf126541_GridOff.xlsx")) as document:

            # data that we will check against when hyperlink is inserted
            urls =[["",""],["https://www.documentfoundation.org/",""]]
            texts =[["aaa bbb","bbb"],["cccc ddd","ddd"],["eeee","aaa cccc eeee"]]

            # 1. run, we want hyperlink insertion work like in MS excel (only 1 hyperlink/cell is allowed)
            # 2. run, we want hyperlink insertion work as it did in calc (more hyperlinks can be in 1 cell)
            for i in range(2):
                xCalcDoc = self.xUITest.getTopFocusWindow()
                xGridWindow = xCalcDoc.getChild("grid_window")

                #Change hyperlink interoperability setting
                #Go to Tools -> Options -> LibreofficeDev Calc -> Compatibility
                with self.ui_test.execute_dialog_through_command(".uno:OptionsTreeDialog", close_button="cancel") as xDialogOpt:

                    xPages = xDialogOpt.getChild("pages")
                    xChartEntry = xPages.getChild('3')                 # LibreofficeDev Calc
                    xChartEntry.executeAction("EXPAND", tuple())
                    xChartGeneralEntry = xChartEntry.getChild('7')
                    xChartGeneralEntry.executeAction("SELECT", tuple())          #Compatibility

                    xLinks = xDialogOpt.getChild("cellLinkCB")
                    xLinks.executeAction("CLICK", tuple())
                    xApply = xDialogOpt.getChild("apply")
                    xApply.executeAction("CLICK", tuple())

                enter_text_to_cell(xGridWindow, "A1", "aaa bbb")

                # Select last word of the cell text: "bbb"
                xGridWindow.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
                xGridWindow.executeAction("TYPE", mkPropertyValues({"KEYCODE":"F2"}))
                xGridWindow.executeAction("TYPE", mkPropertyValues({"KEYCODE":"SHIFT+CTRL+LEFT"}))

                # Insert hyperlink
                with self.ui_test.execute_dialog_through_command(".uno:HyperlinkDialog") as xDialog:
                    xTab = xDialog.getChild("tabcontrol")
                    select_pos(xTab, "0")

                    xTarget = xDialog.getChild("target")
                    self.assertEqual(get_state_as_dict(xTarget)["Text"], "")
                    xIndication = xDialog.getChild("indication")
                    self.assertEqual(get_state_as_dict(xIndication)["Text"], texts[0][i])
                    # 1. run "aaa bbb" The whole cell text
                    # 2. run "bbb" Only the selected text

                    # Insert a sample hyperlink, and change text
                    xTarget.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
                    xTarget.executeAction("TYPE", mkPropertyValues({"TEXT": "https://www.documentfoundation.org/"}))
                    xIndication.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
                    xIndication.executeAction("TYPE", mkPropertyValues({"TEXT": "cccc"}))

                # Edit cell text: insert " ddd" in the end
                xGridWindow.executeAction("SELECT", mkPropertyValues({"CELL": "A2"}))
                xGridWindow.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
                xGridWindow.executeAction("TYPE", mkPropertyValues({"KEYCODE":"F2"}))
                xGridWindow.executeAction("TYPE", mkPropertyValues({"TEXT": " ddd"}))
                xGridWindow.executeAction("TYPE", mkPropertyValues({"KEYCODE":"RETURN"}))

                # Select the last word of cell text: "ddd"
                xGridWindow.executeAction("SELECT", mkPropertyValues({"CELL": "A2"}))
                xGridWindow.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
                xGridWindow.executeAction("TYPE", mkPropertyValues({"KEYCODE":"F2"}))
                xGridWindow.executeAction("TYPE", mkPropertyValues({"KEYCODE":"SHIFT+CTRL+LEFT"}))

                # Insert hyperlink
                with self.ui_test.execute_dialog_through_command(".uno:HyperlinkDialog") as xDialog2:
                    xTab = xDialog2.getChild("tabcontrol")
                    select_pos(xTab, "0")

                    xTarget = xDialog2.getChild("target")
                    self.assertEqual(get_state_as_dict(xTarget)["Text"], urls[1][i])
                    # 1. run: "https://www.documentfoundation.org/" the cell already have this url.
                    # 2. run: "" The selected text is not a hyperlink yet.
                    xIndication = xDialog2.getChild("indication")
                    self.assertEqual(get_state_as_dict(xIndication)["Text"], texts[1][i])
                    # 1. run: "cccc ddd" The whole cell text
                    # 2. run: "ddd" Only the selected text

                    # Insert a sample hyperlink, and change text
                    xTarget.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
                    xTarget.executeAction("TYPE", mkPropertyValues({"TEXT": "https://aWrongLink/"}))
                    xIndication.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
                    xIndication.executeAction("TYPE", mkPropertyValues({"TEXT": "eeee"}))

                # Move focus to ensure cell is not in edit mode
                xGridWindow.executeAction("SELECT", mkPropertyValues({"CELL": "A2"}))
                xGridWindow.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))

                # Check contents of the cell
                xCell = get_cell_by_position(document, 0, 0, 0)
                self.assertEqual(xCell.getString(), texts[2][i])
                # 1. run: "eeee" last hyperlink insertion overwritten the whole cell text with "eeee"
                # 2. run: "aaa cccc eeee" as every hyperlink insertion only overwritten the actually selected text
                xTextFields = xCell.getTextFields()
                self.assertEqual(len(xTextFields), i+1)
                self.assertEqual(xTextFields[i].URL, "https://aWrongLink/")
                if (i==1):
                    self.assertEqual(xTextFields[0].URL, "https://www.documentfoundation.org/")
                # 1. run: only the last inserted hyperlink will remain: "https://aWrongLink/"
                # 2. run: both links will be in the cell

# vim: set shiftwidth=4 softtabstop=4 expandtab:
