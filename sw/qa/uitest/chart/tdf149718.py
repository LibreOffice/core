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

class tdf149718( UITestCase ):

    def test_chart_from_table_with_merged_cells( self ):
        with self.ui_test.create_doc_in_start_center("writer"):
            xWriterDoc = self.xUITest.getTopFocusWindow()
            xWriterEdit = xWriterDoc.getChild("writer_edit")
            with self.ui_test.execute_dialog_through_command(".uno:InsertTable") as xDialog:
                formatlbinstable = xDialog.getChild("formatlbinstable")
                entry = formatlbinstable.getChild("1")
                entry.executeAction("SELECT", tuple())
            xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE":"RETURN"}))

            self.xUITest.executeCommand(".uno:GoDown")
            self.xUITest.executeCommand(".uno:CharRightSel")
            self.xUITest.executeCommand(".uno:MergeCells")

            self.xUITest.executeCommand(".uno:GoDown")
            xWriterEdit.executeAction("TYPE", mkPropertyValues({"TEXT": "1"}))
            self.xUITest.executeCommand(".uno:GoLeft")
            self.xUITest.executeCommand(".uno:GoLeft")
            xWriterEdit.executeAction("TYPE", mkPropertyValues({"TEXT": "1"}))
            self.xUITest.executeCommand(".uno:CharRightSel")
            with self.ui_test.execute_dialog_through_command(".uno:InsertObjectChart", close_button="finish") as xDialog:
                 xDialog.getChild('Wizard')

# vim: set shiftwidth=4 softtabstop=4 expandtab:
