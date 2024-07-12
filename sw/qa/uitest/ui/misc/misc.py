# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

"""Covers sw/source/ui/misc/ fixes."""

from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict
from uitest.uihelper.common import type_text


class TestTmpdlg(UITestCase):
    def test_content_control_dialog(self):
        with self.ui_test.create_doc_in_start_center("writer") as xComponent:
            # Insert a dropdown content control, verify that a placeholder item is provided.
            self.xUITest.executeCommand(".uno:InsertDropdownContentControl")
            paragraphs = xComponent.Text.createEnumeration()
            paragraph = paragraphs.nextElement()
            portions = paragraph.createEnumeration()
            portion = portions.nextElement()
            contentControl = portion.ContentControl
            contentControl.Alias = "my alias"
            contentControl.Tag = "my tag"
            contentControl.TabIndex = "1"
            listItems = contentControl.ListItems
            self.assertEqual(len(listItems), 1)
            self.assertEqual(listItems[0][0].Name, "DisplayText")
            self.assertEqual(listItems[0][0].Value, "")
            self.assertEqual(listItems[0][1].Name, "Value")
            self.assertEqual(listItems[0][1].Value, "Choose an item")

            # Append a new list item.
            with self.ui_test.execute_dialog_through_command(".uno:ContentControlProperties") as xDialog:
                xAlias = xDialog.getChild("aliasentry")
                self.assertEqual(get_state_as_dict(xAlias)['Text'], "my alias")
                type_text(xAlias, "new alias ")
                xTag = xDialog.getChild("tagentry")
                self.assertEqual(get_state_as_dict(xTag)['Text'], "my tag")
                type_text(xTag, "new tag ")
                xAdd = xDialog.getChild("add")

                # Id is a random number now, not 0.
                # xId = xDialog.getChild("idspinbutton")
                # self.assertEqual(get_state_as_dict(xId)['Text'], "0")
                # type_text(xId, "429496729") # added in front, making it 4294967290

                xTabIndex = xDialog.getChild("tabindexspinbutton")
                self.assertEqual(get_state_as_dict(xTabIndex)['Text'], "1")
                type_text(xTabIndex, "-") # add a minus in front, making it -1

                with self.ui_test.execute_blocking_action(xAdd.executeAction, args=('CLICK', ())) as xSubDialog:
                    xDisplayName = xSubDialog.getChild("displayname")
                    type_text(xDisplayName, "Foo Bar")
                    xValue = xSubDialog.getChild("value")
                    type_text(xValue, "foo-bar")

            # Verify that the UI appended the list item.
            listItems = contentControl.ListItems
            self.assertEqual(len(listItems), 2)
            self.assertEqual(listItems[1][0].Name, "DisplayText")
            self.assertEqual(listItems[1][0].Value, "Foo Bar")
            self.assertEqual(listItems[1][1].Name, "Value")
            self.assertEqual(listItems[1][1].Value, "foo-bar")
            self.assertEqual(contentControl.Alias, "new alias my alias")
            self.assertEqual(contentControl.Tag, "new tag my tag")
            # self.assertEqual(contentControl.Id, -6) # stored as signed, displays as unsigned
            self.assertEqual(contentControl.TabIndex, 4294967295) # stored as unsigned, displays as signed


# vim: set shiftwidth=4 softtabstop=4 expandtab:
