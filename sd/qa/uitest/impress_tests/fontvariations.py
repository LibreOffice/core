# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

import time

from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict

# Bundled by external/more_fonts: Reem Kufi has a single wght axis, 400..700.
VARIABLE_FONT = "Reem Kufi"
# A value that is no named instance, so it stays an explicit setting.
OFF_INSTANCE = '"wght" 650'


# Tests that a drawing text object takes a font style and font variations,
# i.e. that both slots reach the shells outside Writer too.
class FontVariations(UITestCase):
    def hasFont(self, sName):
        xToolkit = self.xContext.ServiceManager.createInstanceWithContext(
            "com.sun.star.awt.Toolkit", self.xContext)
        xDevice = xToolkit.createScreenCompatibleDevice(100, 100)
        return any(aDescriptor.Name == sName for aDescriptor in xDevice.FontDescriptors)

    def setUp(self):
        super().setUp()
        if not self.hasFont(VARIABLE_FONT):
            self.skipTest("bundled fonts are not available")

    def test_variations_are_applied_to_drawing_text(self):
        with self.ui_test.create_doc_in_start_center("impress") as component:
            xTemplateDlg = self.xUITest.getTopFocusWindow()
            self.ui_test.close_dialog_through_button(xTemplateDlg.getChild("close"))

            xDoc = self.xUITest.getTopFocusWindow()
            xEditWin = xDoc.getChild("impress_win")

            # Enter the title placeholder and give it some text.
            xEditWin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "TAB"}))
            xEditWin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "RETURN"}))
            xEditWin.executeAction("TYPE", mkPropertyValues({"TEXT": "Hello"}))
            self.xUITest.executeCommand(".uno:SelectAll")

            self.xUITest.executeCommandWithParameters(
                ".uno:CharFontName",
                mkPropertyValues({"CharFontName.FamilyName": VARIABLE_FONT}))
            self.xUITest.executeCommandWithParameters(
                ".uno:FontVariations", mkPropertyValues({"FontVariations": OFF_INSTANCE}))

            xEditWin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "ESC"}))
            xEditWin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "ESC"}))

            xShape = component.DrawPages[0][0]
            self.assertEqual(VARIABLE_FONT, xShape.CharFontName)
            self.assertEqual(OFF_INSTANCE, xShape.CharFontVariations)

    def test_style_box_names_the_font(self):
        with self.ui_test.create_doc_in_start_center("impress"):
            xTemplateDlg = self.xUITest.getTopFocusWindow()
            self.ui_test.close_dialog_through_button(xTemplateDlg.getChild("close"))

            xDoc = self.xUITest.getTopFocusWindow()
            xEditWin = xDoc.getChild("impress_win")
            xEditWin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "TAB"}))
            xEditWin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "RETURN"}))
            xEditWin.executeAction("TYPE", mkPropertyValues({"TEXT": "Hello"}))
            self.xUITest.executeCommand(".uno:SelectAll")

            xStyle = xDoc.getChild("fontstylecombobox")
            self.assertEqual("true", get_state_as_dict(xStyle)["Enabled"])

            # The bindings refresh the box asynchronously.
            self.xUITest.executeCommand(".uno:Bold")
            for _ in range(50):
                if get_state_as_dict(xStyle)["Text"] == "Bold":
                    break
                time.sleep(0.1)
            self.assertEqual("Bold", get_state_as_dict(xStyle)["Text"])


# vim: set shiftwidth=4 softtabstop=4 expandtab:
