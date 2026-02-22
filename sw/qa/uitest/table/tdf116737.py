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
from uitest.uihelper.common import select_pos

#Bug 116737 - INSERT TABLE: Can't select table style (gen/gtk)
class tdf116737(UITestCase):
    def test_tdf116737_select_table_style(self):

        with self.ui_test.create_doc_in_start_center("writer"):

            #Insert => Insert Table / It's not possible to select a table style
            with self.ui_test.execute_dialog_through_command(".uno:InsertTable") as xDialog:

                formatlbinstable = xDialog.getChild("formatlbinstable")
                entry = formatlbinstable.getChild("10") #Simple List Shaded
                entry.executeAction("SELECT", tuple())


            #verify .uno:TableDialog
            with self.ui_test.execute_dialog_through_command(".uno:TableDialog") as xDialog:
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "4")   #tab Background

                btncolor = xDialog.getChild("btncolor")
                btncolor.executeAction("CLICK", tuple())

                R_custom = xDialog.getChild("R_custom")
                G_custom = xDialog.getChild("G_custom")
                B_custom = xDialog.getChild("B_custom")
                #"Simple List Shaded" -> header should be black
                self.assertEqual(get_state_as_dict(R_custom)["Text"], "0")
                self.assertEqual(get_state_as_dict(G_custom)["Text"], "0")
                self.assertEqual(get_state_as_dict(B_custom)["Text"], "0")


# vim: set shiftwidth=4 softtabstop=4 expandtab:
