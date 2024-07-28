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
from libreoffice.uno.propertyvalue import mkPropertyValues
import platform

class dateFormFieldDialog(UITestCase):

    def test_setting_date_format(self):
        # open a file with a date form field
        with self.ui_test.load_file(get_url_for_data_file("date_form_field.odt")):

            # open the dialog (cursor is at the field)
            with self.ui_test.execute_dialog_through_command(".uno:ControlProperties") as xDialog:

                itemsList = xDialog.getChild("date_formats_treeview")

                # check whether we have the right format selected
                self.assertEqual(get_state_as_dict(itemsList)["Children"], "20")
                self.assertEqual(get_state_as_dict(itemsList)["SelectEntryText"], "12/01/99")

                # select a new format
                itemsList.getChild("11").executeAction("SELECT", tuple())
                self.assertEqual(get_state_as_dict(itemsList)["SelectEntryText"], "12-01")


            # open the dialog again
            with self.ui_test.execute_dialog_through_command(".uno:ControlProperties") as xDialog:
                itemsList = xDialog.getChild("date_formats_treeview")
                self.assertEqual(get_state_as_dict(itemsList)["SelectEntryText"], "12-01")


    def test_date_field_with_custom_format(self):
        # open a file with a date form field
        with self.ui_test.load_file(get_url_for_data_file("date_form_field_custom_format.odt")):

            # open the dialog (cursor is at the field)
            with self.ui_test.execute_dialog_through_command(".uno:ControlProperties") as xDialog:

                itemsList = xDialog.getChild("date_formats_treeview")

                # check whether we have the right format selected
                # This is awkward though because checking for a fixed number of
                # entries if the selected default format happens to equal a
                # standard system format the entry gets duplicated with
                # "[System]" appended. So this may be either 20 or 21 ... and
                # in that case it is the selected format and the
                # SelectEntryText doesn't match the sample string, so all this
                # is rather fragile depending on actual locale data.
                self.assertEqual(get_state_as_dict(itemsList)["Children"], "21")
                self.assertEqual(get_state_as_dict(itemsList)["SelectEntryText"], "1999. december 1., szerda[System]")


    def test_date_reformat(self):
        # open a file with a date form field
        with self.ui_test.load_file(get_url_for_data_file("date_form_field.odt")) as writer_doc:
            self.assertEqual(writer_doc.getText().getString(), "07/17/19")

            # open the dialog (cursor is at the field)
            with self.ui_test.execute_dialog_through_command(".uno:ControlProperties") as xDialog:

                itemsList = xDialog.getChild("date_formats_treeview")

                # check whether we have the right format selected
                self.assertEqual(get_state_as_dict(itemsList)["Children"], "20")
                self.assertEqual(get_state_as_dict(itemsList)["SelectEntryText"], "12/01/99")

                # select a new format
                itemsList.getChild("11").executeAction("SELECT", tuple())
                self.assertEqual(get_state_as_dict(itemsList)["SelectEntryText"], "12-01")


            # after applying the new format, the field content should be updated
            self.assertEqual(writer_doc.getText().getString(), "07-17")

    def test_date_field_with_placeholder(self):
        # open a file with a date form field
        with self.ui_test.load_file(get_url_for_data_file("date_form_field_with_placeholder.odt")) as writer_doc:
            self.assertEqual(writer_doc.getText().getString(), "[select date]")

            # open the dialog (cursor is at the field)
            with self.ui_test.execute_dialog_through_command(".uno:ControlProperties") as xDialog:

                itemsList = xDialog.getChild("date_formats_treeview")

                # check whether we have the right format selected
                self.assertEqual(get_state_as_dict(itemsList)["Children"], "20")
                self.assertEqual(get_state_as_dict(itemsList)["SelectEntryText"], "Wed 01/Dec 99")

                # select a new format
                itemsList.getChild("11").executeAction("SELECT", tuple())
                self.assertEqual(get_state_as_dict(itemsList)["SelectEntryText"], "12-01")


            # a placeholder text is not changed by format change
            self.assertEqual(writer_doc.getText().getString(), "[select date]")

    def test_date_field_without_current_date(self):
        # current date means the current date fieldmark parameter which contains the current date in YYYY-MM-DD format
        # when this parameter is missing LO tries to parse the content string to find out the set date

        # open a file with a date form field
        with self.ui_test.load_file(get_url_for_data_file("date_form_field_without_current_date.odt")) as writer_doc:
            self.assertEqual(writer_doc.getText().getString(), "07/17/19")

            # open the dialog (cursor is at the field)
            with self.ui_test.execute_dialog_through_command(".uno:ControlProperties") as xDialog:

                itemsList = xDialog.getChild("date_formats_treeview")

                # check whether we have the right format selected
                self.assertEqual(get_state_as_dict(itemsList)["Children"], "20")
                self.assertEqual(get_state_as_dict(itemsList)["SelectEntryText"], "12/01/99")

                # select a new format
                itemsList.getChild("3").executeAction("SELECT", tuple())
                self.assertEqual(get_state_as_dict(itemsList)["SelectEntryText"], "Dec 1, 1999")


            # a placeholder text is not changed by format change
            self.assertEqual(writer_doc.getText().getString(), "Jul 17, 2019")

    def test_date_picker_drop_down(self):
        with self.ui_test.load_file(get_url_for_data_file("date_picker.docx")) as writer_doc:
            xWriterDoc = self.xUITest.getTopFocusWindow()
            xWriterEdit = xWriterDoc.getChild("writer_edit")

            xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "RIGHT"}))
            xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "RIGHT"}))

            # open the dialog (cursor is at the field)
            xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "ALT+DOWN"}))
            xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "ESC"}))
            xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "UP"}))
            if platform.system() == "Windows":
                self.assertEqual(writer_doc.getText().getString(), "\r\nClick to choose a date")
            else:
                self.assertEqual(writer_doc.getText().getString(), "\nClick to choose a date")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
