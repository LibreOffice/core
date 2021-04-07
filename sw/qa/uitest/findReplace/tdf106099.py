# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import select_pos
from uitest.uihelper.common import select_by_text
from uitest.uihelper.common import get_state_as_dict

#Bug 106099 - Find and replace by format, font color added by force

class tdf106099(UITestCase):
    def test_tdf106099_find_format_underline(self):
        writer_doc = self.ui_test.create_doc_in_start_center("writer")
        document = self.ui_test.get_component()
        xWriterDoc = self.xUITest.getTopFocusWindow()
        #Find and replace
        self.ui_test.execute_modeless_dialog_through_command(".uno:SearchDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        #2. Choose Format
        format = xDialog.getChild("format")
        def handle_format_dlg(dialog):
            #Font effects - Underline Single. OK.
            xTabs = dialog.getChild("tabcontrol")
            select_pos(xTabs, "1")          #tab font effects
            underlinelb = dialog.getChild("underlinelb")
            select_by_text(underlinelb, "Single")
            xOkBtn = dialog.getChild("ok")
            self.ui_test.close_dialog_through_button(xOkBtn)

        self.ui_test.execute_blocking_action(format.executeAction, args=('CLICK', ()),
                dialog_handler=handle_format_dlg)
        #verify label searchdesc
        searchdesc = xDialog.getChild("searchdesc")
        print(get_state_as_dict(searchdesc))
        self.assertEqual(get_state_as_dict(searchdesc)["Text"], "Single underline")
        noformat = xDialog.getChild("noformat")
        noformat.executeAction("CLICK", tuple())   #click No format button
        self.assertEqual(get_state_as_dict(searchdesc)["Text"], "")
        xcloseBtn = xDialog.getChild("close")
        self.ui_test.close_dialog_through_button(xcloseBtn)

        self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab:
