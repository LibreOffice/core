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

# Bug 152964 - Undo of tracked deletion of an empty table row crashed Writer


class tdf152964(UITestCase):
    def test_tdf152964(self):
        with self.ui_test.create_doc_in_start_center("writer"):

            # redlining should be on
            self.xUITest.executeCommand(".uno:TrackChanges")
            # hide changes
            self.xUITest.executeCommand(".uno:ShowTrackedChanges")

            # insert a table
            xWriterDoc = self.xUITest.getTopFocusWindow()
            xWriterEdit = xWriterDoc.getChild("writer_edit")
            xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "RETURN"}))
            with self.ui_test.execute_dialog_through_command(".uno:InsertTable") as xDialog:
                formatlbinstable = xDialog.getChild("formatlbinstable")
                entry = formatlbinstable.getChild("1")
                entry.executeAction("SELECT", tuple())

            # delete its second and first rows
            self.xUITest.executeCommand(".uno:GoDown")
            self.xUITest.executeCommand(".uno:DeleteRows")
            self.xUITest.executeCommand(".uno:DeleteRows")

            # This crashed Writer
            self.xUITest.executeCommand(".uno:Undo")

            # test other Undos and Redos
            self.xUITest.executeCommand(".uno:Undo")
            self.xUITest.executeCommand(".uno:Undo")
            self.xUITest.executeCommand(".uno:Redo")
            self.xUITest.executeCommand(".uno:Redo")
            self.xUITest.executeCommand(".uno:Redo")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
