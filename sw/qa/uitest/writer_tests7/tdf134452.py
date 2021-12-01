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
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import select_pos

# Bug 134452 - Applying a table style caused the loss of break/pagedesc props of the table


class tdf134452(UITestCase):
    def test_tdf134452(self):
        with self.ui_test.create_doc_in_start_center("writer"):
            xWriterDoc = self.xUITest.getTopFocusWindow()
            xWriterEdit = xWriterDoc.getChild("writer_edit")
            xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "RETURN"}))
            with self.ui_test.execute_dialog_through_command(".uno:InsertTable") as xDialog:
                formatlbinstable = xDialog.getChild("formatlbinstable")
                entry = formatlbinstable.getChild("1")
                entry.executeAction("SELECT", tuple())
            #setting the break and pageDesc properties
            with self.ui_test.execute_dialog_through_command(".uno:TableDialog") as xDialog:
                tabcontrol = xDialog.getChild("tabcontrol")
                select_pos(tabcontrol, "1")
                xbreak = xDialog.getChild("break")
                xbreak.executeAction("CLICK", tuple())
                self.assertEqual("true", get_state_as_dict(xbreak)["Selected"])
                xpagestyle = xDialog.getChild("pagestyle")
                xpagestyle.executeAction("CLICK", tuple())
                self.assertEqual("true", get_state_as_dict(xpagestyle)["Selected"])

            #applying table style on the table
            #without the fix, break and pageDesc properties would be overridden and lost
            document = self.ui_test.get_component()
            tables = document.getTextTables()
            tables[0].setPropertyValue("TableTemplateName", "Box List Red")

            with self.ui_test.execute_dialog_through_command(".uno:TableDialog") as xDialog:
                tabcontrol = xDialog.getChild("tabcontrol")
                select_pos(tabcontrol, "1")
                xbreak = xDialog.getChild("break")
                self.assertEqual("true", get_state_as_dict(xbreak)["Selected"])
                xpagestyle = xDialog.getChild("pagestyle")
                self.assertEqual("true", get_state_as_dict(xpagestyle)["Selected"])
# vim: set shiftwidth=4 softtabstop=4 expandtab:
