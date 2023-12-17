# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict
from uitest.uihelper.common import select_pos
from uitest.uihelper.common import type_text
from libreoffice.uno.propertyvalue import mkPropertyValues

class tdf156243(UITestCase):

    def change_autocorrect_option(self, enabled):
        with self.ui_test.execute_dialog_through_command(".uno:AutoCorrectDlg") as xDialog:
            xTabs = xDialog.getChild("tabcontrol")
            select_pos(xTabs, "2")
            xList = xDialog.getChild('list')
            xCheckbox = xList.getChild("3")
            self.assertEqual("Automatic *bold*, /italic/, -strikeout- and _underline_", get_state_as_dict(xCheckbox)["Text"])

            xCheckbox.executeAction("CLICK", tuple())
            self.assertEqual(enabled, get_state_as_dict(xCheckbox)["IsChecked"])

    def test_tdf156243_Autocorrect_dialog(self):
        with self.ui_test.create_doc_in_start_center("writer"):
            xWriterDoc = self.xUITest.getTopFocusWindow()
            xWriterEdit = xWriterDoc.getChild("writer_edit")
            try:
                self.change_autocorrect_option("true")

                type_text(xWriterEdit, "*ab*")

                with self.ui_test.execute_dialog_through_command(".uno:AutoFormatRedlineApply", close_button="close") as xAutoFmt:
                    xAcceptAll = xAutoFmt.getChild("acceptall")
                    xAcceptAll.executeAction("CLICK", tuple())
                xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE":"SHIFT+LEFT"}))
                with self.ui_test.execute_dialog_through_command(".uno:FontDialog", close_button="cancel") as xDialog:
                    xTabs = xDialog.getChild("tabcontrol")
                    select_pos(xTabs, "0")
                    xweststylelbcjk = xDialog.getChild("cbWestStyle")
                    self.assertEqual(get_state_as_dict(xweststylelbcjk)["Text"], "Bold")
                self.xUITest.executeCommand(".uno:GoLeft")
                with self.ui_test.execute_dialog_through_command(".uno:FontDialog", close_button="cancel") as xDialog:
                    xTabs = xDialog.getChild("tabcontrol")
                    select_pos(xTabs, "0")
                    xweststylelbcjk = xDialog.getChild("cbWestStyle")
                    self.assertEqual(get_state_as_dict(xweststylelbcjk)["Text"], "Bold")

            finally:
                # reset to default
                self.change_autocorrect_option("false")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
