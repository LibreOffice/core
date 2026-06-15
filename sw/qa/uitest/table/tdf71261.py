# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, select_pos
from libreoffice.uno.propertyvalue import mkPropertyValues


class tdf71261(UITestCase):

    def test_insert_table_name_with_space(self):
        with self.ui_test.create_doc_in_start_center("writer") as document:
            with self.ui_test.execute_dialog_through_command(".uno:InsertTable") as xDialog:
                xNameEdit = xDialog.getChild("nameedit")
                xNameEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "CTRL+A"}))
                xNameEdit.executeAction("TYPE", mkPropertyValues({"TEXT": "Table 1"}))
                self.assertEqual(get_state_as_dict(xNameEdit)["Text"], "Table 1")

            tables = document.getTextTables()
            self.assertEqual(tables[0].getName(), "Table 1")

    def test_uno_set_table_name_with_space(self):
        with self.ui_test.create_doc_in_start_center("writer") as document:
            with self.ui_test.execute_dialog_through_command(".uno:InsertTable") as xDialog:
                pass  # create a table with the default name

            tables = document.getTextTables()
            tables[0].setName("My Table")
            self.assertEqual(tables[0].getName(), "My Table")

    def test_table_properties_name_with_space(self):
        with self.ui_test.create_doc_in_start_center("writer") as document:
            with self.ui_test.execute_dialog_through_command(".uno:InsertTable") as xDialog:
                pass  # create a table with the default name

            with self.ui_test.execute_dialog_through_command(".uno:TableDialog") as xDialog:
                tabcontrol = xDialog.getChild("tabcontrol")
                select_pos(tabcontrol, "0")
                name = xDialog.getChild("name")
                name.executeAction("TYPE", mkPropertyValues({"KEYCODE": "CTRL+A"}))
                name.executeAction("TYPE", mkPropertyValues({"TEXT": "My Table"}))

            with self.ui_test.execute_dialog_through_command(".uno:TableDialog") as xDialog:
                tabcontrol = xDialog.getChild("tabcontrol")
                select_pos(tabcontrol, "0")
                name = xDialog.getChild("name")
                self.assertEqual(get_state_as_dict(name)["Text"], "My Table")

            tables = document.getTextTables()
            self.assertEqual(tables[0].getName(), "My Table")


# vim: set shiftwidth=4 softtabstop=4 expandtab:
