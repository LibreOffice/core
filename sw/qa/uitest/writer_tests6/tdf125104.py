# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import select_pos, get_state_as_dict
from uitest.uihelper.common import select_by_text
from libreoffice.uno.propertyvalue import mkPropertyValues

class tdf125104(UITestCase):

    def set_combo_layout_format(self, dialog, format):
        tabcontrol = dialog.getChild("tabcontrol")
        select_pos(tabcontrol, "1")
        comboLayoutFormat = dialog.getChild("comboLayoutFormat")
        select_by_text(comboLayoutFormat, format)

    def test_tdf125104_pageFormat_numbering(self):
        with self.ui_test.create_doc_in_start_center("writer") as document:

            # insert page numbers on multiple pages
            self.xUITest.executeCommand(".uno:InsertPageNumberField")
            self.xUITest.executeCommand(".uno:InsertPagebreak")
            self.xUITest.executeCommand(".uno:InsertPageNumberField")
            text = document.Text.String.replace('\r\n', '\n')
            self.assertEqual(text[0:1], "1")
            self.assertEqual(text[2:3], "2")

            # Bug 125104 - Changing page numbering to "1st, 2nd, 3rd,..." causes crashes when trying to change Page settings later
            with self.ui_test.execute_dialog_through_command(".uno:PageDialog") as xDialog:
                self.set_combo_layout_format(xDialog, "1st, 2nd, 3rd, ...")

            text = document.Text.String.replace('\r\n', '\n')
            self.assertEqual(text[0:3], "1st")
            self.assertEqual(text[4:7], "2nd")

            with self.ui_test.execute_dialog_through_command(".uno:PageDialog", close_button="cancel") as xDialog:
                comboLayoutFormat = xDialog.getChild("comboLayoutFormat")
                self.assertEqual(get_state_as_dict(comboLayoutFormat)["SelectEntryText"], "1st, 2nd, 3rd, ...")

            # change to devanagari alphabet format
            with self.ui_test.execute_dialog_through_command(".uno:PageDialog") as xDialog:
                self.set_combo_layout_format(xDialog, "क, ख, ग, ...")

            text = document.Text.String.replace('\r\n', '\n')
            self.assertEqual(text[0:1], "क")
            self.assertEqual(text[2:3], "ख")

            # change to devanagari number format
            with self.ui_test.execute_dialog_through_command(".uno:PageDialog") as xDialog:
                self.set_combo_layout_format(xDialog, "१, २, ३, ...")

            text = document.Text.String.replace('\r\n', '\n')
            self.assertEqual(text[0:1], "१")
            self.assertEqual(text[2:3], "२")


# vim: set shiftwidth=4 softtabstop=4 expandtab:
