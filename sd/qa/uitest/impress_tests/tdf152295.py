# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, select_pos
from uitest.uihelper.common import change_measurement_unit

class tdf152295(UITestCase):
    def test_tdf152295(self):
        with self.ui_test.create_doc_in_start_center("impress"):
            TemplateDialog = self.xUITest.getTopFocusWindow()
            close = TemplateDialog.getChild("close")
            self.ui_test.close_dialog_through_button(close)

            with change_measurement_unit(self, "Centimeter"):
                with self.ui_test.execute_dialog_through_command(".uno:PageSetup") as DrawPageDialog:
                    xTabs = DrawPageDialog.getChild("tabcontrol")
                    select_pos(xTabs, "1")
                    btnbitmap = DrawPageDialog.getChild("btnbitmap")
                    btnbitmap.executeAction("CLICK",tuple())
                    width = DrawPageDialog.getChild("width")
                    for _ in range(50):
                        width.executeAction("UP",tuple())
                    height = DrawPageDialog.getChild("height")
                    for _ in range(50):
                        height.executeAction("UP",tuple())

                with self.ui_test.execute_dialog_through_command(".uno:PageSetup") as DrawPageDialog:
                    xTabs = DrawPageDialog.getChild("tabcontrol")
                    select_pos(xTabs, "1")
                    btnbitmap = DrawPageDialog.getChild("btnbitmap")
                    btnbitmap.executeAction("CLICK",tuple())

                with self.ui_test.execute_dialog_through_command(".uno:PageSetup") as DrawPageDialog:
                    xTabs = DrawPageDialog.getChild("tabcontrol")
                    select_pos(xTabs, "1")
                    btnbitmap = DrawPageDialog.getChild("btnbitmap")
                    btnbitmap.executeAction("CLICK",tuple())
                    width = DrawPageDialog.getChild("width")
                    height = DrawPageDialog.getChild("height")

                    # Without the fix in place, this test would have failed with
                    # AssertionError: '6.00 cm' != '13.55 cm'
                    # AssertionError: '6.00 cm' != '13.55 cm'
                    self.assertEqual("6.00 cm", get_state_as_dict(width)['Text'])
                    self.assertEqual("6.00 cm", get_state_as_dict(height)['Text'])

# vim: set shiftwidth=4 softtabstop=4 expandtab:
