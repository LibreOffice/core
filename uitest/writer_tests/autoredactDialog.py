# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict
import time
import re
from uitest.debug import sleep

class AutoRedactDialog(UITestCase):

    add_target_counter = 0

    # Craete a new Writer doc, and open the Auto Redact Dialog
    def launch_and_get_autoredact_dialog(self):
        self.ui_test.create_doc_in_start_center("writer")
        self.ui_test.execute_dialog_through_command(".uno:AutoRedactDoc")
        xDialog = self.xUITest.getTopFocusWindow()
        self.assertTrue(xDialog is not None)
        return xDialog

    def getText(self, xObj):
        return get_state_as_dict(xObj)["Text"]

    def parseTargetContent(self, xObj):
        return re.split(r'\t+', self.getText(xObj))

    def test_open_AutoRedactDialog_writer(self):
        xDialog = self.launch_and_get_autoredact_dialog()
        xcancBtn = xDialog.getChild("cancel")
        self.ui_test.close_dialog_through_button(xcancBtn)
        self.ui_test.close_doc()

    def test_add_target(self):
        xDialog = self.launch_and_get_autoredact_dialog()
        xAddBtn = xDialog.getChild("add")

        # Names need to be distinct
        # ["target name", "target content"],
        targets_list = [
        ["target1", "content1"],
        ["target2", "content2"],
        ["target3", "content3"],
        ]

        def handle_add_dlg(dialog):                     #handle add target dialog - need special handling
            xNewNameTxt=dialog.getChild("name")
            xNewContentTxt=dialog.getChild("content")
            xOKBtn = dialog.getChild("close")

            xNewNameTxt.executeAction("TYPE", mkPropertyValues({"TEXT":targets_list[self.add_target_counter][0]}))
            xNewContentTxt.executeAction("TYPE", mkPropertyValues({"TEXT":targets_list[self.add_target_counter][1]}))

            self.ui_test.close_dialog_through_button(xOKBtn)

        for i in range(0, len(targets_list)):
            self.add_target_counter = i
            self.ui_test.execute_blocking_action(xAddBtn.executeAction, args=('CLICK', ()),
                    dialog_handler=handle_add_dlg)             #close add target dialog with OK button

        # Make sure targets are added successfully
        xTargetsListbox = xDialog.getChild("targets")
        targets_box_state_dict = get_state_as_dict(xTargetsListbox)
        self.assertEqual(int(targets_box_state_dict["Children"]), len(targets_list))

        # Make sure targets are added with correct names and contents
        for i in range(0, len(targets_list)):
            child = xTargetsListbox.getChild(i)
            child_text = self.parseTargetContent(child)
            self.assertEqual(child_text[0], targets_list[i][0]) #name
            self.assertEqual(child_text[2], targets_list[i][1]) #content

        xcancBtn = xDialog.getChild("cancel")
        self.ui_test.close_dialog_through_button(xcancBtn)

        self.ui_test.close_doc()



# vim: set shiftwidth=4 softtabstop=4 expandtab:
