# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import select_pos
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict

#Bug 119462 - Find with formatting adds 0pt text size

class tdf119462(UITestCase):
    def test_tdf119462_find_format(self):
        writer_doc = self.ui_test.create_doc_in_start_center("writer")
        document = self.ui_test.get_component()
        xWriterDoc = self.xUITest.getTopFocusWindow()
        #1. Choose Edit > Find and Replace
        self.ui_test.execute_modeless_dialog_through_command(".uno:SearchDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        #2. Choose Format
        format = xDialog.getChild("format")
        def handle_format_dlg(dialog):

            #3. Choose Bold
            xTabs = dialog.getChild("tabcontrol")
            select_pos(xTabs, "0")
            xweststylelbcjk = dialog.getChild("weststylelb-cjk")
            xweststylelbcjk.executeAction("TYPE", mkPropertyValues({"TEXT":"Bold"}))
            xOkBtn = dialog.getChild("ok")
            self.ui_test.close_dialog_through_button(xOkBtn)

        self.ui_test.execute_blocking_action(format.executeAction, args=('CLICK', ()),
                dialog_handler=handle_format_dlg)
        #verify label searchdesc
        searchdesc = xDialog.getChild("searchdesc")
        self.assertEqual(get_state_as_dict(searchdesc)["Text"], "bold")
        noformat = xDialog.getChild("noformat")
        noformat.executeAction("CLICK", tuple())   #click No format button
        self.assertEqual(get_state_as_dict(searchdesc)["Text"], "")
        xcloseBtn = xDialog.getChild("close")
        self.ui_test.close_dialog_through_button(xcloseBtn)

        self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab:
