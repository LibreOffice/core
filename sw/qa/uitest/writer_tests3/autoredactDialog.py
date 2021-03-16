# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict
from uitest.uihelper.common import type_text
from uitest.uihelper.common import select_pos
import re

class AutoRedactDialog(UITestCase):

    add_target_counter = 0

    # Open the Auto Redact Dialog
    def launch_and_get_autoredact_dialog(self):
        self.ui_test.execute_dialog_through_command(".uno:AutoRedactDoc")
        xDialog = self.xUITest.getTopFocusWindow()
        self.assertTrue(xDialog is not None)
        return xDialog

    def getText(self, xObj):
        return get_state_as_dict(xObj)["Text"]

    def parseTargetContent(self, xObj):
        return re.split(r'\t+', self.getText(xObj))

    def clearTargetsbox(self, xDialog):
        xTargetsListbox = xDialog.getChild("targets")
        xDeleteBtn = xDialog.getChild("delete")

        child_count = len(xTargetsListbox.getChildren())

        if child_count < 1:
            return

        for i in range(0, child_count):
            child = xTargetsListbox.getChild(0)
            child.executeAction("SELECT", tuple())
            xDeleteBtn.executeAction("CLICK", tuple())

        # Verify
        self.assertEqual(len(xTargetsListbox.getChildren()), 0)


    def test_open_AutoRedactDialog_writer(self):
        self.ui_test.create_doc_in_start_center("writer")
        xDialog = self.launch_and_get_autoredact_dialog()
        xcancBtn = xDialog.getChild("cancel")
        self.ui_test.close_dialog_through_button(xcancBtn)
        self.ui_test.close_doc()

    def test_add_target(self):
        self.ui_test.create_doc_in_start_center("writer")
        xDialog = self.launch_and_get_autoredact_dialog()
        xAddBtn = xDialog.getChild("add")

        # Make sure we are starting with an empty targets list
        self.clearTargetsbox(xDialog)

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
            xTypeList = dialog.getChild("type") #0: Text, 1: Regex, 2: Predefined

            select_pos(xTypeList, 0) #Text
            self.assertEqual(int(get_state_as_dict(xTypeList)["SelectEntryPos"]), 0)

            type_text(xNewNameTxt, targets_list[self.add_target_counter][0])
            type_text(xNewContentTxt, targets_list[self.add_target_counter][1])

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

        # Now let's make sure the dialog remembers last state
        xDialog = self.launch_and_get_autoredact_dialog()
        xTargetsListbox = xDialog.getChild("targets")
        targets_box_state_dict = get_state_as_dict(xTargetsListbox)
        self.assertEqual(int(targets_box_state_dict["Children"]), len(targets_list))

        # Make sure targets are remembered with correct names and contents
        for i in range(0, len(targets_list)):
            child = xTargetsListbox.getChild(i)
            child_text = self.parseTargetContent(child)
            self.assertEqual(child_text[0], targets_list[i][0]) #name
            self.assertEqual(child_text[2], targets_list[i][1]) #content

        xcancBtn = xDialog.getChild("cancel")
        self.ui_test.close_dialog_through_button(xcancBtn)

        self.ui_test.close_doc()


    def test_edit_target(self):
        self.ui_test.create_doc_in_start_center("writer")
        xDialog = self.launch_and_get_autoredact_dialog()
        xAddBtn = xDialog.getChild("add")
        xEditBtn = xDialog.getChild("edit")

        # Make sure we are starting with an empty targets list
        self.clearTargetsbox(xDialog)

        # We first need to add a target so that we can edit it
        def handle_add_dlg(dialog):                     #handle add target dialog - need special handling
            xNewNameTxt=dialog.getChild("name")
            xNewContentTxt=dialog.getChild("content")
            xOKBtn = dialog.getChild("close")
            xTypeList = dialog.getChild("type") #0: Text, 1: Regex, 2: Predefined

            select_pos(xTypeList, 0) #Text
            self.assertEqual(int(get_state_as_dict(xTypeList)["SelectEntryPos"]), 0)

            type_text(xNewNameTxt, "TestTarget")
            type_text(xNewContentTxt, "TestContent")

            self.ui_test.close_dialog_through_button(xOKBtn)

        self.ui_test.execute_blocking_action(xAddBtn.executeAction, args=('CLICK', ()),
                dialog_handler=handle_add_dlg)             #close add target dialog with OK button

        # Make sure target is added successfully
        xTargetsListbox = xDialog.getChild("targets")
        targets_box_state_dict = get_state_as_dict(xTargetsListbox)
        self.assertEqual(int(targets_box_state_dict["Children"]), 1)

        # Select the added target
        target_entry = xTargetsListbox.getChild(0)
        target_entry.executeAction("SELECT", tuple())

        # Now edit the target
        def handle_edit_dlg(dialog):                     #handle add target dialog - need special handling
            xNameTxt=dialog.getChild("name")
            xContentTxt=dialog.getChild("content")
            xOKBtn = dialog.getChild("close")

            xNameTxt.executeAction("CLEAR", tuple())
            xContentTxt.executeAction("CLEAR", tuple())

            type_text(xNameTxt, "TestTargetEdited")
            type_text(xContentTxt, "TestContentEdited")

            self.ui_test.close_dialog_through_button(xOKBtn)

        self.ui_test.execute_blocking_action(xEditBtn.executeAction, args=('CLICK', ()),
                dialog_handler=handle_edit_dlg)             #close add target dialog with OK button

        # Make sure target is still there
        xTargetsListbox = xDialog.getChild("targets")
        targets_box_state_dict = get_state_as_dict(xTargetsListbox)
        self.assertEqual(int(targets_box_state_dict["Children"]), 1)

        # Make sure target has the new values
        target_entry = xTargetsListbox.getChild(0)
        target_text = self.parseTargetContent(target_entry)
        self.assertEqual(target_text[0], "TestTargetEdited") #name
        self.assertEqual(target_text[2], "TestContentEdited") #content

        xcancBtn = xDialog.getChild("cancel")
        self.ui_test.close_dialog_through_button(xcancBtn)

        self.ui_test.close_doc()


# vim: set shiftwidth=4 softtabstop=4 expandtab:
