# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import type_text
from uitest.uihelper.common import select_pos
import time

class tdf106899(UITestCase):

    def test_tdf106899_alphabetical_index_utf8(self):
        with self.ui_test.create_doc_in_start_center("writer") as document:
            xWriterDoc = self.xUITest.getTopFocusWindow()
            xWriterEdit = xWriterDoc.getChild("writer_edit")
            type_text(xWriterEdit, "Cựu Kim Sơn")
            self.xUITest.executeCommand(".uno:SelectAll")

            # Add an index for the utf-8 text
            with self.ui_test.execute_dialog_through_command(".uno:InsertIndexesEntry") as xDialog:
                xInsertBtn = xDialog.getChild("insert")
                xInsertBtn.executeAction("CLICK", tuple())
                xCloseBtn = xDialog.getChild("close")
                self.ui_test.close_dialog_through_button(xCloseBtn)
                time.sleep(2)

            # Add an alphabetical index to the document
            with self.ui_test.execute_dialog_through_command(".uno:InsertMultiIndex") as xDialog:
                 xIndexType = xDialog.getChild("type")
                 select_pos(xIndexType, "1")
                 xOkBtn = xDialog.getChild("ok")
                 self.ui_test.close_dialog_through_button(xOkBtn)
                 time.sleep(2)

            # Check if the inserted alphabetical index is correct
            xDocumentIndexes = xTextDoc.getDocumentIndexes()
            self.assertEqual(xDocumentIndexes.getCount(), 1)
            self.assertEqual(xDocumentIndexes.hasByName("Alphabetical Index1"), True)
            xIndexAnchor = xDocumentIndexes.getByName("Alphabetical Index1").getAnchor()
            self.assertEqual("Cựu Kim Sơn" in xIndexAnchor.getString(), True)
            time.sleep(2)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
