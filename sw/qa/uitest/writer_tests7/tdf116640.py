# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict
from uitest.uihelper.common import select_pos

class tdf116640(UITestCase):

    def test_tdf116640(self):

        with self.ui_test.create_doc_in_start_center("writer") as document:

            with self.ui_test.execute_dialog_through_command(".uno:InsertSection") as xDialog:
                xColumns = xDialog.getChild("colsnf")
                xAutoWith = xDialog.getChild("autowidth")
                xBalance = xDialog.getChild("balance")
                xLineStyle = xDialog.getChild("linestylelb")
                xSpacing = xDialog.getChild("spacing1mf")

                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "1")

                self.assertEqual("1", get_state_as_dict(xColumns)['Text'])
                self.assertEqual("false", get_state_as_dict(xAutoWith)['Enabled'])
                self.assertEqual("false", get_state_as_dict(xBalance)['Enabled'])
                self.assertEqual("false", get_state_as_dict(xLineStyle)['Enabled'])
                self.assertEqual("false", get_state_as_dict(xSpacing)['Enabled'])

                xColumns.executeAction("UP", tuple())

                self.assertEqual("2", get_state_as_dict(xColumns)['Text'])
                self.assertEqual("true", get_state_as_dict(xAutoWith)['Enabled'])
                self.assertEqual("true", get_state_as_dict(xBalance)['Enabled'])
                self.assertEqual("true", get_state_as_dict(xLineStyle)['Enabled'])
                self.assertEqual("true", get_state_as_dict(xSpacing)['Enabled'])

            xWriterDoc = self.xUITest.getTopFocusWindow()
            xWriterEdit = xWriterDoc.getChild("writer_edit")

            self.assertEqual(1, len(document.TextSections))
            self.assertEqual(2, len(document.TextSections.Section1.TextColumns.Columns))

            self.xUITest.executeCommand(".uno:Undo")

            self.assertEqual(0, len(document.TextSections))

            self.xUITest.executeCommand(".uno:Redo")

            self.assertEqual(1, len(document.TextSections))
            self.assertEqual(2, len(document.TextSections.Section1.TextColumns.Columns))

            # Without the fix in place, this test would have crashed here
            self.xUITest.executeCommand(".uno:Undo")

            self.assertEqual(0, len(document.TextSections))

# vim: set shiftwidth=4 softtabstop=4 expandtab:
