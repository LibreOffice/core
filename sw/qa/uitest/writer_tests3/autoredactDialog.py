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
from uitest.uihelper.common import type_text
from uitest.uihelper.common import select_pos
import re

class AutoRedactDialog(UITestCase):

    add_target_counter = 0

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

    def test_add_target(self):
        with self.ui_test.create_doc_in_start_center("writer"):
            with self.ui_test.execute_dialog_through_command(".uno:AutoRedactDoc", close_button="cancel") as xDialog:
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

                for i in range(0, len(targets_list)):
                    self.add_target_counter = i
                    with self.ui_test.execute_blocking_action(xAddBtn.executeAction, args=('CLICK', ()), close_button="close") as dialog:
                        xNewNameTxt=dialog.getChild("name")
                        xNewContentTxt=dialog.getChild("content")
                        xTypeList = dialog.getChild("type") #0: Text, 1: Regex, 2: Predefined

                        select_pos(xTypeList, "0") #Text
                        self.assertEqual(int(get_state_as_dict(xTypeList)["SelectEntryPos"]), 0)

                        type_text(xNewNameTxt, targets_list[self.add_target_counter][0])
                        type_text(xNewContentTxt, targets_list[self.add_target_counter][1])

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

            # Now let's make sure the dialog remembers last state
            with self.ui_test.execute_dialog_through_command(".uno:AutoRedactDoc", close_button="cancel") as xDialog:
                xTargetsListbox = xDialog.getChild("targets")
                targets_box_state_dict = get_state_as_dict(xTargetsListbox)
                self.assertEqual(int(targets_box_state_dict["Children"]), len(targets_list))

                # Make sure targets are remembered with correct names and contents
                for i in range(0, len(targets_list)):
                    child = xTargetsListbox.getChild(i)
                    child_text = self.parseTargetContent(child)
                    self.assertEqual(child_text[0], targets_list[i][0]) #name
                    self.assertEqual(child_text[2], targets_list[i][1]) #content



    def test_edit_target(self):
        with self.ui_test.create_doc_in_start_center("writer"):
            with self.ui_test.execute_dialog_through_command(".uno:AutoRedactDoc", close_button="cancel") as xDialog:
                xAddBtn = xDialog.getChild("add")
                xEditBtn = xDialog.getChild("edit")

                # Make sure we are starting with an empty targets list
                self.clearTargetsbox(xDialog)

                # We first need to add a target so that we can edit it
                with self.ui_test.execute_blocking_action(xAddBtn.executeAction, args=('CLICK', ()), close_button="close") as dialog:
                    xNewNameTxt=dialog.getChild("name")
                    xNewContentTxt=dialog.getChild("content")
                    xTypeList = dialog.getChild("type") #0: Text, 1: Regex, 2: Predefined

                    select_pos(xTypeList, "0") #Text
                    self.assertEqual(int(get_state_as_dict(xTypeList)["SelectEntryPos"]), 0)

                    type_text(xNewNameTxt, "TestTarget")
                    type_text(xNewContentTxt, "TestContent")

                # Make sure target is added successfully
                xTargetsListbox = xDialog.getChild("targets")
                targets_box_state_dict = get_state_as_dict(xTargetsListbox)
                self.assertEqual(int(targets_box_state_dict["Children"]), 1)

                # Select the added target
                target_entry = xTargetsListbox.getChild(0)
                target_entry.executeAction("SELECT", tuple())

                # Now edit the target
                with self.ui_test.execute_blocking_action(xEditBtn.executeAction, args=('CLICK', ()), close_button="close") as dialog:
                    xNameTxt=dialog.getChild("name")
                    xContentTxt=dialog.getChild("content")

                    xNameTxt.executeAction("CLEAR", tuple())
                    xContentTxt.executeAction("CLEAR", tuple())

                    type_text(xNameTxt, "TestTargetEdited")
                    type_text(xContentTxt, "TestContentEdited")

                # Make sure target is still there
                xTargetsListbox = xDialog.getChild("targets")
                targets_box_state_dict = get_state_as_dict(xTargetsListbox)
                self.assertEqual(int(targets_box_state_dict["Children"]), 1)

                # Make sure target has the new values
                target_entry = xTargetsListbox.getChild(0)
                target_text = self.parseTargetContent(target_entry)
                self.assertEqual(target_text[0], "TestTargetEdited") #name
                self.assertEqual(target_text[2], "TestContentEdited") #content



# vim: set shiftwidth=4 softtabstop=4 expandtab:
