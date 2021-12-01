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
#Bug 107847 - CRASH Opening macro tab of properties dialog (images, frames) causes crash

class tdf107847(UITestCase):

   def test_tdf_107847_macro_tab_crash(self):
        with self.ui_test.create_doc_in_start_center("writer") as document:
            xWriterDoc = self.xUITest.getTopFocusWindow()

            with self.ui_test.execute_dialog_through_command(".uno:InsertFrame") as xDialog:
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "1")
                select_pos(xTabs, "2")
                select_pos(xTabs, "3")
                select_pos(xTabs, "4")
                select_pos(xTabs, "5")
                select_pos(xTabs, "6")
                select_pos(xTabs, "7")
                select_pos(xTabs, "8")   #tab Macro

            self.assertEqual(document.TextFrames.getCount(), 1)
            self.xUITest.executeCommand(".uno:Undo")
            self.assertEqual(document.TextFrames.getCount(), 0)
# vim: set shiftwidth=4 softtabstop=4 expandtab:
