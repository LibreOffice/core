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

   def test_tdf156243_Autocorrect_dialog(self):
        with self.ui_test.create_doc_in_start_center("writer") as document:
            xWriterDoc = self.xUITest.getTopFocusWindow()
            xWriterEdit = xWriterDoc.getChild("writer_edit")
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

# vim: set shiftwidth=4 softtabstop=4 expandtab:
