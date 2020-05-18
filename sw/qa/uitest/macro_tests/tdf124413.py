# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict

class tdf124413(UITestCase):

    def test_copy_paste_overwriting_undo_redo_in_macro_editor_tdf124413(self):

        self.ui_test.create_doc_in_start_center("writer")
        #Start LibreOffice. Go to Tools > Macros > Organize Macros > Basic
        self.ui_test.execute_dialog_through_command(".uno:MacroDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        #Open Editor
        xEditBtn = xDialog.getChild("edit")
        xEditBtn.executeAction("CLICK", tuple())

        resultText = "REM  *****  BASIC  *****\n\nSub Main\n\nEnd Sub\n"

        xMacroWin = self.xUITest.getTopFocusWindow()
        xEditWin = xMacroWin.getChild('EditorWindow')

        self.assertEqual(get_state_as_dict(xEditWin)['Text'], resultText)

        self.xUITest.executeCommand(".uno:SelectAll")
        self.xUITest.executeCommand(".uno:Copy")
        self.xUITest.executeCommand(".uno:SelectAll")
        self.xUITest.executeCommand(".uno:Paste")
        self.xUITest.executeCommand(".uno:Undo")
        self.xUITest.executeCommand(".uno:Redo")
        self.xUITest.executeCommand(".uno:Undo")
        self.xUITest.executeCommand(".uno:Redo")
        self.xUITest.executeCommand(".uno:Undo")
        self.xUITest.executeCommand(".uno:Redo")

        self.assertEqual(get_state_as_dict(xEditWin)['Text'], resultText)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
