# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file
from uitest.uihelper.common import select_pos
from libreoffice.uno.propertyvalue import mkPropertyValues
from time import sleep

class tdf156165(UITestCase):

    def change_autocorrect_option(self, enabled):
        with self.ui_test.execute_dialog_through_command(".uno:AutoCorrectDlg") as xDialog:
            xTabs = xDialog.getChild("tabcontrol")
            select_pos(xTabs, "2")
            xList = xDialog.getChild('list')
            xCheckbox = xList.getChild("17")
            self.assertEqual("Replace Custom Styles", get_state_as_dict(xCheckbox)["Text"])

            xCheckbox.executeAction("CLICK", tuple())
            self.assertEqual(enabled, get_state_as_dict(xCheckbox)["IsChecked"])

    def test_tdf156165(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf156165.odt")):
            xMainWindow = self.xUITest.getTopFocusWindow()
            writer_edit = xMainWindow.getChild("writer_edit")
            style=xMainWindow.getChild('applystyle')

            # Replace Custom Styles when applying manually with it disabled, should not change style
            writer_edit.executeAction("SELECT", mkPropertyValues({"END_POS": "0", "START_POS": "12"}))
            self.xUITest.executeCommand(".uno:AutoFormatApply")
            sleep(1)
            self.assertEqual(get_state_as_dict(style)["Text"], "eSelah")

            # Replace Custom Styles when typing with it disabled, should not change style
            writer_edit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "DOWN"}))
            writer_edit.executeAction("SELECT", mkPropertyValues({"END_POS": "12", "START_POS": "12"}))
            writer_edit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "RETURN"}))
            sleep(1)
            self.assertEqual(get_state_as_dict(style)["Text"], "eSelah") # new line
            writer_edit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "UP"}))
            writer_edit.executeAction("SELECT", mkPropertyValues({"END_POS": "0", "START_POS": "12"}))
            self.assertEqual(get_state_as_dict(style)["Text"], "eSelah") # original line

            try:
                self.change_autocorrect_option("true")

                # Replace Custom Styles when applying manually with it enabled, should change style
                writer_edit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "DOWN"}))
                writer_edit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "DOWN"}))
                writer_edit.executeAction("SELECT", mkPropertyValues({"END_POS": "0", "START_POS": "12"}))
                self.xUITest.executeCommand(".uno:AutoFormatApply")
                sleep(1)
                self.assertEqual(get_state_as_dict(style)["Text"], "Body Text")

                # Replace Custom Styles when typing with it enabled, should not change style
                writer_edit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "DOWN"}))
                writer_edit.executeAction("SELECT", mkPropertyValues({"END_POS": "12", "START_POS": "12"}))
                writer_edit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "RETURN"}))
                sleep(1)
                self.assertEqual(get_state_as_dict(style)["Text"], "eSelah") # new line
                writer_edit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "UP"}))
                writer_edit.executeAction("SELECT", mkPropertyValues({"END_POS": "0", "START_POS": "12"}))
                self.assertEqual(get_state_as_dict(style)["Text"], "eSelah") # original line

            finally:
                # reset to default
                self.change_autocorrect_option("false")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
