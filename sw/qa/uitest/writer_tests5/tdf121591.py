# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
import time
from uitest.uihelper.common import get_state_as_dict, type_text
from uitest.debug import sleep
from uitest.uihelper.common import select_pos
#Bug 121591 - CRASH: Print Preview fails if cursor inside Floating frame
class tdf121591(UITestCase):

   def test_tdf121591_frame_print_preview(self):
        self.ui_test.create_doc_in_start_center("writer")
        document = self.ui_test.get_component()
        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")
        self.ui_test.execute_dialog_through_command(".uno:InsertFrame")   #  insert frame
        xDialogFr = self.xUITest.getTopFocusWindow()

        xOkBtn=xDialogFr.getChild("ok")
        xOkBtn.executeAction("CLICK", tuple())

        type_text(xWriterEdit, "Text in the frame")
        #open and close print preview
        self.xUITest.executeCommand(".uno:PrintPreview")  #open print preview
        self.xUITest.executeCommand(".uno:ClosePreview")  # close print preview
        #verify nr. of frames
        self.assertEqual(document.TextFrames.getCount(), 1)

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
