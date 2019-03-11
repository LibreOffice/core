# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict
from libreoffice.uno.propertyvalue import mkPropertyValues
import org.libreoffice.unotest
import pathlib
import time

def get_url_for_data_file(file_name):
    return pathlib.Path(org.libreoffice.unotest.makeCopyFromTDOC(file_name)).as_uri()

class dropDownFormFieldDialog(UITestCase):

    def test_add_new_items(self):

        # open a file with an empty form field
        writer_doc = self.ui_test.load_file(get_url_for_data_file("empty_drop_down_form_field.odt"))
        document = self.ui_test.get_component()
        xWriterDoc = self.xUITest.getTopFocusWindow()

        # open the dialog (cursor is at the field)
        self.ui_test.execute_dialog_through_command(".uno:ControlProperties")
        xDialog = self.xUITest.getTopFocusWindow()

        itemEntry = xDialog.getChild("item_entry")
        addButton = xDialog.getChild("add_button")
        itemsList = xDialog.getChild("items_treeview")

        # initial state
        self.assertEqual(get_state_as_dict(itemEntry)["Text"], "")
        self.assertEqual(get_state_as_dict(addButton)["Enabled"], "false")
        self.assertEqual(get_state_as_dict(itemsList)["Children"], "0")

        # add some new items
        itemEntry.executeAction("TYPE", mkPropertyValues({"TEXT":"1000"}))
        self.assertEqual(get_state_as_dict(addButton)["Enabled"], "true")
        addButton.executeAction("CLICK", tuple())
        self.assertEqual(get_state_as_dict(addButton)["Enabled"], "false")
        itemEntry.executeAction("TYPE", mkPropertyValues({"TEXT":"2000"}))
        addButton.executeAction("CLICK", tuple())
        itemEntry.executeAction("TYPE", mkPropertyValues({"TEXT":"3000"}))
        addButton.executeAction("CLICK", tuple())
        itemEntry.executeAction("TYPE", mkPropertyValues({"TEXT":"4000"}))
        addButton.executeAction("CLICK", tuple())

        # check whether the items are there in the list
        self.assertEqual(get_state_as_dict(itemsList)["Children"], "4")
        self.assertEqual(get_state_as_dict(itemsList.getChild("0"))["Text"], "1000")
        self.assertEqual(get_state_as_dict(itemsList.getChild("1"))["Text"], "2000")
        self.assertEqual(get_state_as_dict(itemsList.getChild("2"))["Text"], "3000")
        self.assertEqual(get_state_as_dict(itemsList.getChild("3"))["Text"], "4000")

        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        # check whether items are the same after reopening
        self.ui_test.execute_dialog_through_command(".uno:ControlProperties")
        xDialog = self.xUITest.getTopFocusWindow()

        itemsList = xDialog.getChild("items_treeview")
        self.assertEqual(get_state_as_dict(itemsList)["Children"], "4")
        self.assertEqual(get_state_as_dict(itemsList.getChild("0"))["Text"], "1000")
        self.assertEqual(get_state_as_dict(itemsList.getChild("1"))["Text"], "2000")
        self.assertEqual(get_state_as_dict(itemsList.getChild("2"))["Text"], "3000")
        self.assertEqual(get_state_as_dict(itemsList.getChild("3"))["Text"], "4000")

        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        self.ui_test.close_doc()

    def test_remove_items(self):

        # open a file with an empty form field
        writer_doc = self.ui_test.load_file(get_url_for_data_file("empty_drop_down_form_field.odt"))
        document = self.ui_test.get_component()
        xWriterDoc = self.xUITest.getTopFocusWindow()

        # open the dialog (cursor is at the field)
        self.ui_test.execute_dialog_through_command(".uno:ControlProperties")
        xDialog = self.xUITest.getTopFocusWindow()

        itemEntry = xDialog.getChild("item_entry")
        addButton = xDialog.getChild("add_button")
        itemsList = xDialog.getChild("items_treeview")
        removeButton = xDialog.getChild("remove_button")

        # initial state
        self.assertEqual(get_state_as_dict(itemEntry)["Text"], "")
        self.assertEqual(get_state_as_dict(addButton)["Enabled"], "false")
        self.assertEqual(get_state_as_dict(itemsList)["Children"], "0")
        self.assertEqual(get_state_as_dict(removeButton)["Enabled"], "false")

        # add some new items
        itemEntry.executeAction("TYPE", mkPropertyValues({"TEXT":"1000"}))
        self.assertEqual(get_state_as_dict(addButton)["Enabled"], "true")
        addButton.executeAction("CLICK", tuple())
        self.assertEqual(get_state_as_dict(removeButton)["Enabled"], "true")
        itemEntry.executeAction("TYPE", mkPropertyValues({"TEXT":"2000"}))
        addButton.executeAction("CLICK", tuple())
        itemEntry.executeAction("TYPE", mkPropertyValues({"TEXT":"3000"}))
        addButton.executeAction("CLICK", tuple())
        itemEntry.executeAction("TYPE", mkPropertyValues({"TEXT":"4000"}))
        addButton.executeAction("CLICK", tuple())

        # check whether the items are there in the list
        self.assertEqual(get_state_as_dict(itemsList)["Children"], "4")
        self.assertEqual(get_state_as_dict(itemsList.getChild("0"))["Text"], "1000")
        self.assertEqual(get_state_as_dict(itemsList.getChild("1"))["Text"], "2000")
        self.assertEqual(get_state_as_dict(itemsList.getChild("2"))["Text"], "3000")
        self.assertEqual(get_state_as_dict(itemsList.getChild("3"))["Text"], "4000")

        # select an item from the list and remove it
        itemsList.getChild("1").executeAction("SELECT", tuple());
        removeButton.executeAction("CLICK", tuple())

        # check whether the right item was removed
        self.assertEqual(get_state_as_dict(itemsList)["Children"], "3")
        self.assertEqual(get_state_as_dict(itemsList.getChild("0"))["Text"], "1000")
        self.assertEqual(get_state_as_dict(itemsList.getChild("1"))["Text"], "3000")
        self.assertEqual(get_state_as_dict(itemsList.getChild("2"))["Text"], "4000")

        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        # check whether items are the same after reopening
        self.ui_test.execute_dialog_through_command(".uno:ControlProperties")
        xDialog = self.xUITest.getTopFocusWindow()

        itemsList = xDialog.getChild("items_treeview")
        removeButton = xDialog.getChild("remove_button")
        self.assertEqual(get_state_as_dict(itemsList)["Children"], "3")
        self.assertEqual(get_state_as_dict(itemsList.getChild("0"))["Text"], "1000")
        self.assertEqual(get_state_as_dict(itemsList.getChild("1"))["Text"], "3000")
        self.assertEqual(get_state_as_dict(itemsList.getChild("2"))["Text"], "4000")

        # remove all items
        itemsList.getChild("1").executeAction("SELECT", tuple());
        removeButton.executeAction("CLICK", tuple())
        removeButton.executeAction("CLICK", tuple())
        removeButton.executeAction("CLICK", tuple())

        self.assertEqual(get_state_as_dict(removeButton)["Enabled"], "false")
        self.assertEqual(get_state_as_dict(itemsList)["Children"], "0")

        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        self.ui_test.close_doc()

    def test_move_items(self):

        # open a file with an empty form field
        writer_doc = self.ui_test.load_file(get_url_for_data_file("empty_drop_down_form_field.odt"))
        document = self.ui_test.get_component()
        xWriterDoc = self.xUITest.getTopFocusWindow()

        # open the dialog (cursor is at the field)
        self.ui_test.execute_dialog_through_command(".uno:ControlProperties")
        xDialog = self.xUITest.getTopFocusWindow()

        itemEntry = xDialog.getChild("item_entry")
        addButton = xDialog.getChild("add_button")
        itemsList = xDialog.getChild("items_treeview")
        upButton = xDialog.getChild("up_button")
        downButton = xDialog.getChild("down_button")

        # initial state
        self.assertEqual(get_state_as_dict(itemEntry)["Text"], "")
        self.assertEqual(get_state_as_dict(addButton)["Enabled"], "false")
        self.assertEqual(get_state_as_dict(itemsList)["Children"], "0")
        self.assertEqual(get_state_as_dict(upButton)["Enabled"], "false")
        self.assertEqual(get_state_as_dict(downButton)["Enabled"], "false")

        # add some new items
        itemEntry.executeAction("TYPE", mkPropertyValues({"TEXT":"1000"}))
        self.assertEqual(get_state_as_dict(addButton)["Enabled"], "true")
        addButton.executeAction("CLICK", tuple())
        itemEntry.executeAction("TYPE", mkPropertyValues({"TEXT":"2000"}))
        addButton.executeAction("CLICK", tuple())
        itemEntry.executeAction("TYPE", mkPropertyValues({"TEXT":"3000"}))
        addButton.executeAction("CLICK", tuple())
        itemEntry.executeAction("TYPE", mkPropertyValues({"TEXT":"4000"}))
        addButton.executeAction("CLICK", tuple())

        # check whether the items are there in the list
        self.assertEqual(get_state_as_dict(itemsList)["Children"], "4")
        self.assertEqual(get_state_as_dict(itemsList.getChild("0"))["Text"], "1000")
        self.assertEqual(get_state_as_dict(itemsList.getChild("1"))["Text"], "2000")
        self.assertEqual(get_state_as_dict(itemsList.getChild("2"))["Text"], "3000")
        self.assertEqual(get_state_as_dict(itemsList.getChild("3"))["Text"], "4000")

        # select an item from the list and move it up
        itemsList.getChild("1").executeAction("SELECT", tuple())
        self.assertEqual(get_state_as_dict(upButton)["Enabled"], "true")
        self.assertEqual(get_state_as_dict(downButton)["Enabled"], "true")
        upButton.executeAction("CLICK", tuple())
        self.assertEqual(get_state_as_dict(upButton)["Enabled"], "false")
        self.assertEqual(get_state_as_dict(downButton)["Enabled"], "true")

        # check whether the item was correctly moved
        self.assertEqual(get_state_as_dict(itemsList)["Children"], "4")
        self.assertEqual(get_state_as_dict(itemsList.getChild("0"))["Text"], "2000")
        self.assertEqual(get_state_as_dict(itemsList.getChild("1"))["Text"], "1000")
        self.assertEqual(get_state_as_dict(itemsList.getChild("2"))["Text"], "3000")
        self.assertEqual(get_state_as_dict(itemsList.getChild("3"))["Text"], "4000")

        # move down the selected item
        downButton.executeAction("CLICK", tuple())
        downButton.executeAction("CLICK", tuple())
        downButton.executeAction("CLICK", tuple())
        self.assertEqual(get_state_as_dict(upButton)["Enabled"], "true")
        self.assertEqual(get_state_as_dict(downButton)["Enabled"], "false")

        # check whether the item was correctly moved
        self.assertEqual(get_state_as_dict(itemsList)["Children"], "4")
        self.assertEqual(get_state_as_dict(itemsList.getChild("0"))["Text"], "1000")
        self.assertEqual(get_state_as_dict(itemsList.getChild("1"))["Text"], "3000")
        self.assertEqual(get_state_as_dict(itemsList.getChild("2"))["Text"], "4000")
        self.assertEqual(get_state_as_dict(itemsList.getChild("3"))["Text"], "2000")

        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        # check whether items are the same after reopening
        self.ui_test.execute_dialog_through_command(".uno:ControlProperties")
        xDialog = self.xUITest.getTopFocusWindow()

        itemsList = xDialog.getChild("items_treeview")
        self.assertEqual(get_state_as_dict(itemsList)["Children"], "4")
        self.assertEqual(get_state_as_dict(itemsList.getChild("0"))["Text"], "1000")
        self.assertEqual(get_state_as_dict(itemsList.getChild("1"))["Text"], "3000")
        self.assertEqual(get_state_as_dict(itemsList.getChild("2"))["Text"], "4000")
        self.assertEqual(get_state_as_dict(itemsList.getChild("3"))["Text"], "2000")

        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        self.ui_test.close_doc()

    def test_drop_down_after_import(self):

        files = ["drop_down_form_field.odt", "drop_down_form_field.doc", "drop_down_form_field.docx"]
        for file in files:
            # open a file with a drop-down for field with items and selection
            writer_doc = self.ui_test.load_file(get_url_for_data_file(file))
            document = self.ui_test.get_component()
            xWriterDoc = self.xUITest.getTopFocusWindow()

            # open the dialog (cursor is at the field)
            self.ui_test.execute_dialog_through_command(".uno:ControlProperties")
            xDialog = self.xUITest.getTopFocusWindow()

            itemsList = xDialog.getChild("items_treeview")

            # check whether the items are there in the list
            self.assertEqual(get_state_as_dict(itemsList)["Children"], "4")
            self.assertEqual(get_state_as_dict(itemsList.getChild("0"))["Text"], "1000")
            self.assertEqual(get_state_as_dict(itemsList.getChild("1"))["Text"], "2000")
            self.assertEqual(get_state_as_dict(itemsList.getChild("2"))["Text"], "3000")
            self.assertEqual(get_state_as_dict(itemsList.getChild("3"))["Text"], "4000")

            self.assertEqual(get_state_as_dict(itemsList)["SelectEntryText"], "3000")

            xOKBtn = xDialog.getChild("ok")
            self.ui_test.close_dialog_through_button(xOKBtn)

            self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
