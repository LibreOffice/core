# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict, type_text
import time
from uitest.debug import sleep
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import select_pos

#Bug 122722 - Hiding characters will crash. Crash in: SwAttrIter::CtorInitAttrIter(SwTextNode &,SwScriptInfo &,SwTextFrame const *)

class tdf122722(UITestCase):
   def test_tdf122722_format_character_hidden(self):
        self.ui_test.create_doc_in_start_center("writer")
        document = self.ui_test.get_component()
        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")
        #1. Start LibreOffice
        #2. Create New Writer Document
        #3. Type "LibreOffice" in Writer
        type_text(xWriterEdit, "LibreOffice")
        #4. Select "LibreOffice" with mouse, and right click
        self.xUITest.executeCommand(".uno:SelectAll")
        self.assertEqual(document.Text.String[0:11], "LibreOffice")
        #5. Appear Context Menu, Character -> Character
        #6. Opened Character, Select "Font Effect" tab
        #7. Check Hidden, and click [OK]
        #8. Crash a LibreOffice
        self.ui_test.execute_dialog_through_command(".uno:FontDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        xTabs = xDialog.getChild("tabcontrol")
        select_pos(xTabs, "1")

        xEffects = xDialog.getChild("effectslb")
        xRelief = xDialog.getChild("relieflb")
        xBlinking = xDialog.getChild("blinkingcb")
        xHidden = xDialog.getChild("hiddencb")
        xOverline = xDialog.getChild("overlinelb")
        xStrikeout = xDialog.getChild("strikeoutlb")
        xUnderline = xDialog.getChild("underlinelb")
        xEmphasis = xDialog.getChild("emphasislb")
        xPosition = xDialog.getChild("positionlb")

        xHidden.executeAction("CLICK", tuple())

        xOK = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOK)
        #un-hidden
        self.ui_test.execute_dialog_through_command(".uno:FontDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        xTabs = xDialog.getChild("tabcontrol")
        select_pos(xTabs, "1")

        xEffects = xDialog.getChild("effectslb")
        xRelief = xDialog.getChild("relieflb")
        xBlinking = xDialog.getChild("blinkingcb")
        xHidden = xDialog.getChild("hiddencb")
        xOverline = xDialog.getChild("overlinelb")
        xStrikeout = xDialog.getChild("strikeoutlb")
        xUnderline = xDialog.getChild("underlinelb")
        xEmphasis = xDialog.getChild("emphasislb")
        xPosition = xDialog.getChild("positionlb")

        self.assertEqual(get_state_as_dict(xHidden)["Selected"], "true")
        xHidden.executeAction("CLICK", tuple())

        xOK = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOK)

        self.assertEqual(document.Text.String[0:11], "LibreOffice")

        self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab:
