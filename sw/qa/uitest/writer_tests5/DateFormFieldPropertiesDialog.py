# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file

class dateFormFieldDialog(UITestCase):

    def test_setting_date_format(self):
        # open a file with a date form field
        writer_doc = self.ui_test.load_file(get_url_for_data_file("date_form_field.odt"))
        document = self.ui_test.get_component()
        xWriterDoc = self.xUITest.getTopFocusWindow()

        # open the dialog (cursor is at the field)
        self.ui_test.execute_dialog_through_command(".uno:ControlProperties")
        xDialog = self.xUITest.getTopFocusWindow()

        itemsList = xDialog.getChild("date_formats_treeview")

        # check whether we have the right format selected
        self.assertEqual(get_state_as_dict(itemsList)["Children"], "20")
        self.assertEqual(get_state_as_dict(itemsList)["SelectEntryText"], "12/31/99")

        # select a new format
        itemsList.getChild("11").executeAction("SELECT", tuple());
        self.assertEqual(get_state_as_dict(itemsList)["SelectEntryText"], "12-31")

        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        # open the dialog again
        self.ui_test.execute_dialog_through_command(".uno:ControlProperties")
        xDialog = self.xUITest.getTopFocusWindow()
        itemsList = xDialog.getChild("date_formats_treeview")
        self.assertEqual(get_state_as_dict(itemsList)["SelectEntryText"], "12-31")

        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        self.ui_test.close_doc()

    def test_date_field_with_custom_format(self):
        # open a file with a date form field
        writer_doc = self.ui_test.load_file(get_url_for_data_file("date_form_field_custom_format.odt"))
        document = self.ui_test.get_component()
        xWriterDoc = self.xUITest.getTopFocusWindow()

        # open the dialog (cursor is at the field)
        self.ui_test.execute_dialog_through_command(".uno:ControlProperties")
        xDialog = self.xUITest.getTopFocusWindow()

        itemsList = xDialog.getChild("date_formats_treeview")

        # check whether we have the right format selected
        self.assertEqual(get_state_as_dict(itemsList)["Children"], "20")
        self.assertEqual(get_state_as_dict(itemsList)["SelectEntryText"], "1999. december 31., péntek")

        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        self.ui_test.close_doc()

    def test_date_reformat(self):
        # open a file with a date form field
        writer_doc = self.ui_test.load_file(get_url_for_data_file("date_form_field.odt"))
        document = self.ui_test.get_component()
        xWriterDoc = self.xUITest.getTopFocusWindow()
        self.assertEqual(writer_doc.getText().getString(), "07/17/19")

        # open the dialog (cursor is at the field)
        self.ui_test.execute_dialog_through_command(".uno:ControlProperties")
        xDialog = self.xUITest.getTopFocusWindow()

        itemsList = xDialog.getChild("date_formats_treeview")

        # check whether we have the right format selected
        self.assertEqual(get_state_as_dict(itemsList)["Children"], "20")
        self.assertEqual(get_state_as_dict(itemsList)["SelectEntryText"], "12/31/99")

        # select a new format
        itemsList.getChild("11").executeAction("SELECT", tuple());
        self.assertEqual(get_state_as_dict(itemsList)["SelectEntryText"], "12-31")

        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        # after applying the new format, the field content should be updated
        self.assertEqual(writer_doc.getText().getString(), "07-17")

        self.ui_test.close_doc()

    def test_date_field_with_placeholder(self):
        # open a file with a date form field
        writer_doc = self.ui_test.load_file(get_url_for_data_file("date_form_field_with_placeholder.odt"))
        document = self.ui_test.get_component()
        xWriterDoc = self.xUITest.getTopFocusWindow()
        self.assertEqual(writer_doc.getText().getString(), "[select date]")

        # open the dialog (cursor is at the field)
        self.ui_test.execute_dialog_through_command(".uno:ControlProperties")
        xDialog = self.xUITest.getTopFocusWindow()

        itemsList = xDialog.getChild("date_formats_treeview")

        # check whether we have the right format selected
        self.assertEqual(get_state_as_dict(itemsList)["Children"], "20")
        self.assertEqual(get_state_as_dict(itemsList)["SelectEntryText"], "Fri 31/Dec 99")

        # select a new format
        itemsList.getChild("11").executeAction("SELECT", tuple());
        self.assertEqual(get_state_as_dict(itemsList)["SelectEntryText"], "12-31")

        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        # a placeholder text is not changed by format change
        self.assertEqual(writer_doc.getText().getString(), "[select date]")

        self.ui_test.close_doc()

    def test_date_field_without_current_date(self):
        # current date means the current date fieldmark parameter which contains the current date in YYYY-MM-DD format
        # when this parameter is missing LO tries to parse the content string to find out the set date

        # open a file with a date form field
        writer_doc = self.ui_test.load_file(get_url_for_data_file("date_form_field_without_current_date.odt"))
        document = self.ui_test.get_component()
        xWriterDoc = self.xUITest.getTopFocusWindow()
        self.assertEqual(writer_doc.getText().getString(), "07/17/19")

        # open the dialog (cursor is at the field)
        self.ui_test.execute_dialog_through_command(".uno:ControlProperties")
        xDialog = self.xUITest.getTopFocusWindow()

        itemsList = xDialog.getChild("date_formats_treeview")

        # check whether we have the right format selected
        self.assertEqual(get_state_as_dict(itemsList)["Children"], "20")
        self.assertEqual(get_state_as_dict(itemsList)["SelectEntryText"], "12/31/99")

        # select a new format
        itemsList.getChild("3").executeAction("SELECT", tuple());
        self.assertEqual(get_state_as_dict(itemsList)["SelectEntryText"], "Dec 31, 1999")

        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        # a placeholder text is not changed by format change
        self.assertEqual(writer_doc.getText().getString(), "Jul 17, 2019")

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
