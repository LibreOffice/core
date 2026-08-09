# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

import time

from com.sun.star.awt.FontSlant import ITALIC
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict


# Tests for the font style box in the formatting toolbar,
# SvxFontStyleToolBoxControl in svx/source/tbxctrls/tbcontrl.cxx.
class FontStyleBox(UITestCase):
    def typeText(self, xDoc):
        xDoc.getChild("writer_edit").executeAction(
            "TYPE", mkPropertyValues({"TEXT": "Hello"}))
        self.xUITest.executeCommand(".uno:SelectAll")

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

    def test_box_shows_current_style(self):
        with self.ui_test.create_doc_in_start_center("writer"):
            xDoc = self.xUITest.getTopFocusWindow()
            self.typeText(xDoc)

            self.assertEqual("Regular", self.getStyle(xDoc, "Regular"))

            self.xUITest.executeCommand(".uno:Bold")
            self.assertEqual("Bold", self.getStyle(xDoc, "Bold"))

    def test_box_sizing_follows_font(self):
        with self.ui_test.create_doc_in_start_center("writer"):
            xDoc = self.xUITest.getTopFocusWindow()
            self.typeText(xDoc)
            xStyle = xDoc.getChild("fontstylecombobox")

            # Wide enough for the style names, not a fixed handful of pixels.
            self.assertGreater(int(get_state_as_dict(xStyle)["Size"].split("x")[0]), 100)

    def test_box_applies_style(self):
        with self.ui_test.create_doc_in_start_center("writer") as component:
            xDoc = self.xUITest.getTopFocusWindow()
            self.typeText(xDoc)

            xStyle = xDoc.getChild("fontstylecombobox")
            xStyle.executeAction("TYPE", mkPropertyValues({"KEYCODE": "CTRL+A"}))
            xStyle.executeAction("TYPE", mkPropertyValues({"TEXT": "Bold Italic"}))
            xStyle.executeAction("TYPE", mkPropertyValues({"KEYCODE": "RETURN"}))

            self.assertEqual(150.0, component.CurrentSelection[0].CharWeight)
            self.assertEqual(ITALIC, component.CurrentSelection[0].CharPosture)


# vim: set shiftwidth=4 softtabstop=4 expandtab:
