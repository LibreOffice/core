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

# Bundled by external/more_fonts: Reem Kufi has a single wght axis, 400..700,
# with named instances at 400/500/600/700.
VARIABLE_FONT = "Reem Kufi"
# A value that is no named instance, so it stays an explicit setting.
OFF_INSTANCE = '"wght" 650'


# Tests for the .uno:FontVariations slot the font variations popover applies
# through, svx/source/sidebar/text/FontVariationsToolBoxControl.cxx.
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

    def typeText(self, xDoc):
        xDoc.getChild("writer_edit").executeAction(
            "TYPE", mkPropertyValues({"TEXT": "Hello"}))
        self.xUITest.executeCommand(".uno:SelectAll")

    def setFont(self, sName):
        self.xUITest.executeCommandWithParameters(
            ".uno:CharFontName", mkPropertyValues({"CharFontName.FamilyName": sName}))

    def setVariations(self, sVariations):
        self.xUITest.executeCommandWithParameters(
            ".uno:FontVariations", mkPropertyValues({"FontVariations": sVariations}))

    def getStyle(self, xDoc, sExpected):
        # The bindings refresh the box asynchronously.
        xStyle = xDoc.getChild("fontstylecombobox")
        sStyle = ""
        for _ in range(50):
            sStyle = get_state_as_dict(xStyle)["Text"]
            if sStyle == sExpected:
                break
            time.sleep(0.1)
        return sStyle

    def test_settings_are_applied(self):
        with self.ui_test.create_doc_in_start_center("writer") as component:
            xDoc = self.xUITest.getTopFocusWindow()
            self.typeText(xDoc)

            self.setFont(VARIABLE_FONT)
            self.setVariations(OFF_INSTANCE)

            self.assertEqual(OFF_INSTANCE, component.CurrentSelection[0].CharFontVariations)

    def test_style_names_no_font_off_instance(self):
        with self.ui_test.create_doc_in_start_center("writer"):
            xDoc = self.xUITest.getTopFocusWindow()
            self.typeText(xDoc)

            self.setFont(VARIABLE_FONT)
            self.assertEqual("Regular", self.getStyle(xDoc, "Regular"))

            # Settings that describe no named instance leave the style empty,
            # keeping the one they replaced would mislead.
            self.setVariations(OFF_INSTANCE)
            self.assertEqual("", self.getStyle(xDoc, ""))

            self.setVariations("")
            self.assertEqual("Regular", self.getStyle(xDoc, "Regular"))


# vim: set shiftwidth=4 softtabstop=4 expandtab:
