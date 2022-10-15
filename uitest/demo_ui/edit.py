# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from libreoffice.uno.propertyvalue import mkPropertyValues

from uitest.framework import UITestCase
from uitest.uihelper.common import type_text, get_state_as_dict, select_text

class EditTest(UITestCase):

    def test_type_text(self):

        with self.ui_test.create_doc_in_start_center("calc"):

            with self.ui_test.execute_modeless_dialog_through_command(".uno:AddName", close_button="cancel") as xAddNameDlg:

                xEdit = xAddNameDlg.getChild("edit")

                type_text(xEdit, "simpleRangeName")



    def test_select_text(self):

        with self.ui_test.create_doc_in_start_center("calc"):

            with self.ui_test.execute_modeless_dialog_through_command(".uno:AddName", close_button="cancel") as xAddNameDlg:

                xEdit = xAddNameDlg.getChild("edit")

                type_text(xEdit, "simpleRangeName")
                xEdit.executeAction("SELECT", mkPropertyValues({"FROM": "2", "TO": "9"}))
                type_text(xEdit, "otherChars")
                self.assertEqual("siotherCharsgeName", get_state_as_dict(xEdit)["Text"])

                select_text(xEdit, from_pos="2", to="12")
                self.assertEqual("otherChars", get_state_as_dict(xEdit)["SelectedText"])



# vim: set shiftwidth=4 softtabstop=4 expandtab:
