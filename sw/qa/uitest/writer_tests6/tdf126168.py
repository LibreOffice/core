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
#Bug 126168 - Crash in: rtl_uString_acquire: frame style undo redo

class tdf126168(UITestCase):

   def test_tdf126168_frame_undo_redo_crash(self):
        with self.ui_test.create_doc_in_start_center("writer") as document:
            xWriterDoc = self.xUITest.getTopFocusWindow()
            xWriterEdit = xWriterDoc.getChild("writer_edit")

            #2) Menu > Insert > Frame > Frame
            #3) Press OK in Frame dialog
            with self.ui_test.execute_dialog_through_command(".uno:InsertFrame"):
                pass
            self.assertEqual(document.TextFrames.getCount(), 1)
            #New Style from Selection  [uno:StyleNewByExample]
            with self.ui_test.execute_dialog_through_command(".uno:StyleNewByExample") as xDialog:
                #5) Enter a name in the Create Style dialog and press OK
                stylename = xDialog.getChild("stylename")
                stylename.executeAction("TYPE", mkPropertyValues({"TEXT":"a"}))
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

# vim: set shiftwidth=4 softtabstop=4 expandtab:
