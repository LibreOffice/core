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

class tdf122045(UITestCase):

    def test_tdf122045(self):

        with self.ui_test.create_doc_in_start_center("writer") as document:
            xWriterDoc = self.xUITest.getTopFocusWindow()

            with self.ui_test.execute_dialog_through_command(".uno:PageDialog", close_button="cancel") as xDialog:

                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "2")

                btncolor = xDialog.getChild("btncolor")
                btncolor.executeAction("CLICK", tuple())

                xApplyBtn = xDialog.getChild("apply")
                xApplyBtn.executeAction("CLICK", tuple())

                self.assertTrue(document.isModified())
                self.assertEqual("0x729fcf", hex(document.StyleFamilies.PageStyles.Standard.BackColor))


            self.assertTrue(document.isModified())
            self.assertEqual("0x729fcf", hex(document.StyleFamilies.PageStyles.Standard.BackColor))


# vim: set shiftwidth=4 softtabstop=4 expandtab:
