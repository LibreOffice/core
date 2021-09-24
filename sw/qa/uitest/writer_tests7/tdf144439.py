# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import type_text, select_by_text, select_pos

class tdf144439(UITestCase):

    def test_tdf144439_list(self):
        with self.ui_test.create_doc_in_start_center("writer") as document:
            xWriterDoc = self.xUITest.getTopFocusWindow()
            xWriterEdit = xWriterDoc.getChild("writer_edit")
            type_text(xWriterEdit, "List item")

            with self.ui_test.execute_dialog_through_command(".uno:BulletsAndNumberingDialog") as xDialog:
                # Select custom tab
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "5")

                # Select level
                xLevels = xDialog.getChild("levellb")
                # TODO: does not work either of following 3 variants
                #xLevels.executeAction("DOWN", tuple())
                #select_by_text(xLevels, "2")
                #select_pos(xLevels, 2)

                # Select numbering
                xNumFmt = xDialog.getChild("numfmtlb")
                select_by_text(xNumFmt, "1, 2, 3, ...")

                # Increase number of sublevels to show
                xSubLevels = xDialog.getChild("sublevels")
                xSubLevels.executeAction("UP", tuple())

            # TODO: increase numbering level 1->2

            # TODO: check actual numbering

# vim: set shiftwidth=4 softtabstop=4 expandtab:
