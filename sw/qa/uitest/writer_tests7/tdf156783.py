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
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import select_pos
from uitest.uihelper.common import get_url_for_data_file

# Bug 156783 - crash fix: setting table border on a table without correct table cursor

class tdf156783(UITestCase):
    def test_tdf156783(self):
        with self.ui_test.load_file(get_url_for_data_file("TC-table-del-add.docx")) as self.document:

            # accept all tracked changes
            self.xUITest.executeCommand(".uno:AcceptAllTrackedChanges")

            # select all tables
            self.xUITest.executeCommand(".uno:SelectAll")
            self.xUITest.executeCommand(".uno:SelectAll")
            self.xUITest.executeCommand(".uno:SelectAll")

            # dialog Table Properties - Borders
            with self.ui_test.execute_dialog_through_command(".uno:TableDialog", close_button="ok") as xDialog:
                tabcontrol = xDialog.getChild("tabcontrol")
                select_pos(tabcontrol, "3")

                sync = xDialog.getChild("sync")
                rightmf = xDialog.getChild("rightmf")
                leftmf = xDialog.getChild("leftmf")
                topmf = xDialog.getChild("topmf")
                bottommf = xDialog.getChild("bottommf")
                sync.executeAction("CLICK", tuple())

                rightmf.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
                rightmf.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
                rightmf.executeAction("TYPE", mkPropertyValues({"TEXT":"72pt"}))
                leftmf.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
                leftmf.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
                leftmf.executeAction("TYPE", mkPropertyValues({"TEXT":"72pt"}))
                topmf.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
                topmf.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
                topmf.executeAction("TYPE", mkPropertyValues({"TEXT":"72pt"}))
                bottommf.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
                bottommf.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
                bottommf.executeAction("TYPE", mkPropertyValues({"TEXT":"72pt"}))

                self.assertEqual(get_state_as_dict(rightmf)["Text"], u"72pt")
                self.assertEqual(get_state_as_dict(leftmf)["Text"], u"72pt")
                self.assertEqual(get_state_as_dict(topmf)["Text"], u"72pt")
                self.assertEqual(get_state_as_dict(bottommf)["Text"], u"72pt")

            # Without the fix in place, this test would have crashed here

# vim: set shiftwidth=4 softtabstop=4 expandtab:
