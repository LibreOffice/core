# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, type_text
from uitest.uihelper.common import select_pos

#Bug 122722 - Hiding characters will crash. Crash in: SwAttrIter::CtorInitAttrIter(SwTextNode &,SwScriptInfo &,SwTextFrame const *)

class tdf122722(UITestCase):
   def test_tdf122722_format_character_hidden(self):
        with self.ui_test.create_doc_in_start_center("writer") as document:
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
            with self.ui_test.execute_dialog_through_command(".uno:FontDialog") as xDialog:
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "1")

                xHidden = xDialog.getChild("hiddencb")

                xHidden.executeAction("CLICK", tuple())

            #un-hidden
            with self.ui_test.execute_dialog_through_command(".uno:FontDialog") as xDialog:
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "1")

                xHidden = xDialog.getChild("hiddencb")

                self.assertEqual(get_state_as_dict(xHidden)["Selected"], "true")
                xHidden.executeAction("CLICK", tuple())


            self.assertEqual(document.Text.String[0:11], "LibreOffice")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
