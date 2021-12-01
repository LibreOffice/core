# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.calc import enter_text_to_cell
from uitest.uihelper.common import select_pos
from uitest.uihelper.common import select_by_text

class tdf126248(UITestCase):

    def assertFontName(self, gridwin, fontName):

        #Open the sidebar
        self.xUITest.executeCommand(".uno:Sidebar")
        gridwin.executeAction("SIDEBAR", mkPropertyValues({"PANEL": "TextPropertyPanel"}))

        xCalcDoc = self.xUITest.getTopFocusWindow()

        xFontName = xCalcDoc.getChild("fontnamecombobox")
        self.ui_test.wait_until_property_is_updated(xFontName, "Text", fontName)
        self.assertEqual(fontName, get_state_as_dict(xFontName)['Text'])

        #Close the sidebar
        self.xUITest.executeCommand(".uno:Sidebar")

    def changeLocalSetting(self, language):
        with self.ui_test.execute_dialog_through_command(".uno:OptionsTreeDialog") as xDialog:
            xPages = xDialog.getChild("pages")
            xLanguageEntry = xPages.getChild('2')
            xLanguageEntry.executeAction("EXPAND", tuple())
            xxLanguageEntryGeneralEntry = xLanguageEntry.getChild('0')
            xxLanguageEntryGeneralEntry.executeAction("SELECT", tuple())

            # Check asian support is enabled
            asianlanguage = xDialog.getChild("asiansupport")
            self.assertEqual("true", get_state_as_dict(asianlanguage)['Selected'])

            localeSetting = xDialog.getChild("localesetting")
            select_by_text(localeSetting, language)
            self.ui_test.wait_until_property_is_updated(localeSetting, 'SelectEntryText', language)
            self.assertEqual(language, get_state_as_dict(localeSetting)['SelectEntryText'])

    def test_tdf126248(self):

        with self.ui_test.create_doc_in_start_center("calc"):

            self.changeLocalSetting("Chinese (traditional)")

            with self.ui_test.execute_dialog_through_command(".uno:FormatCellDialog") as xDialog:
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "1")

                # Get current font names from the Format Cell dialog
                westFontName = get_state_as_dict(xDialog.getChild("westfontnamelb-cjk"))['Text']
                eastFontName = get_state_as_dict(xDialog.getChild("eastfontnamelb"))['Text']

            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")

            enter_text_to_cell(gridwin, "A1", "Test")

            # Without the fix in place, this test would have failed here
            self.assertFontName(gridwin, westFontName)

            enter_text_to_cell(gridwin, "B1", "測試")

            self.assertFontName(gridwin, eastFontName)

            self.changeLocalSetting("English (USA)")

            enter_text_to_cell(gridwin, "C1", "Test")

            self.assertFontName(gridwin, westFontName)

            enter_text_to_cell(gridwin, "D1", "測試")

            self.assertFontName(gridwin, eastFontName)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
