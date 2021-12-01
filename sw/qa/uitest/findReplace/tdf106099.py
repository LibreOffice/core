# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
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
        with self.ui_test.create_doc_in_start_center("writer"):
            xWriterDoc = self.xUITest.getTopFocusWindow()
            #Find and replace
            with self.ui_test.execute_modeless_dialog_through_command(".uno:SearchDialog", close_button="close") as xDialog:
                #2. Choose Format
                format = xDialog.getChild("format")
                with self.ui_test.execute_blocking_action(format.executeAction, args=('CLICK', ())) as dialog:
                    #Font effects - Underline Single. OK.
                    xTabs = dialog.getChild("tabcontrol")
                    select_pos(xTabs, "1")          #tab font effects
                    underlinelb = dialog.getChild("underlinelb")
                    select_by_text(underlinelb, "Single")

                #verify label searchdesc
                searchdesc = xDialog.getChild("searchdesc")
                self.assertEqual(get_state_as_dict(searchdesc)["Text"], "Single underline")
                noformat = xDialog.getChild("noformat")
                noformat.executeAction("CLICK", tuple())   #click No format button
                self.assertEqual(get_state_as_dict(searchdesc)["Text"], "")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
