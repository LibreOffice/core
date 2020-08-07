# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
#Bug 126168 - Crash in: rtl_uString_acquire: frame style undo redo

class tdf126168(UITestCase):

   def test_tdf126168_frame_undo_redo_crash(self):
        self.ui_test.create_doc_in_start_center("writer")
        document = self.ui_test.get_component()
        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")

        #2) Menu > Insert > Frame > Frame
        #3) Press OK in Frame dialog
        self.ui_test.execute_dialog_through_command(".uno:InsertFrame")   #  insert frame
        xDialog = self.xUITest.getTopFocusWindow()
        xokbtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xokbtn)
        self.assertEqual(document.TextFrames.getCount(), 1)
        #New Style from Selection  [uno:StyleNewByExample]
        self.ui_test.execute_dialog_through_command(".uno:StyleNewByExample")
        #5) Enter a name in the Create Style dialog and press OK
        xDialog = self.xUITest.getTopFocusWindow()
        stylename = xDialog.getChild("stylename")
        stylename.executeAction("TYPE", mkPropertyValues({"TEXT":"a"}))
        xokbtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xokbtn)
        #6) ctrl+z 3 times
        self.xUITest.executeCommand(".uno:Undo")
        self.xUITest.executeCommand(".uno:Undo")
        self.xUITest.executeCommand(".uno:Undo")
        self.assertEqual(document.TextFrames.getCount(), 0)
        #7) shift+ctrl+z 3 times
        self.xUITest.executeCommand(".uno:Redo")
        self.xUITest.executeCommand(".uno:Redo")
        self.xUITest.executeCommand(".uno:Redo")

        #Results: crash
        self.assertEqual(document.CurrentController.PageCount, 1)

        self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab:
