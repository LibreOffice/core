# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict, select_pos

#https://bugs.documentfoundation.org/show_bug.cgi?id=128593
#Bug 128593 - Writer table cell's background color is always black

class tdf128593(UITestCase):
    def test_tdf128593_table_background_color(self):
        with self.ui_test.create_doc_in_start_center("writer"):
            MainWindow = self.xUITest.getTopFocusWindow()

            with self.ui_test.execute_dialog_through_command(".uno:InsertTable"):
                pass

            self.xUITest.executeCommandWithParameters(".uno:TableCellBackgroundColor", mkPropertyValues({"TableCellBackgroundColor" : 16776960 }) )
            with self.ui_test.execute_dialog_through_command(".uno:TableDialog") as TablePropertiesDialog:
                writer_edit = MainWindow.getChild("writer_edit")
                writer_edit.executeAction("SELECT", mkPropertyValues({"END_POS": "0", "START_POS": "0"}))
                tabcontrol = TablePropertiesDialog.getChild("tabcontrol")
                select_pos(tabcontrol, "4")
                Rcustom = TablePropertiesDialog.getChild("R_custom")  #255
                self.assertEqual(get_state_as_dict(Rcustom)["Text"], "255")
                Gcustom = TablePropertiesDialog.getChild("G_custom")  #255
                self.assertEqual(get_state_as_dict(Gcustom)["Text"], "255")
                Bcustom = TablePropertiesDialog.getChild("B_custom")  #0
                self.assertEqual(get_state_as_dict(Bcustom)["Text"], "0")


# vim: set shiftwidth=4 softtabstop=4 expandtab:
