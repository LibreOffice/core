# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
#Bug 85360 - LibreOffice crashes and crashes desktop too on inserting 4th slide


class insertSlide(UITestCase):

   def test_insert_slide(self):
        with self.ui_test.create_doc_in_start_center("impress") as document:
            xTemplateDlg = self.xUITest.getTopFocusWindow()
            xCancelBtn = xTemplateDlg.getChild("close")
            self.ui_test.close_dialog_through_button(xCancelBtn)
            self.assertEqual(document.DrawPages.getCount(), 1)  #nr. of pages

            self.xUITest.executeCommand(".uno:InsertPage")

            #verify
            self.assertEqual(document.DrawPages.getCount(), 2)  #nr. of pages
            #undo
            self.xUITest.executeCommand(".uno:Undo")
            self.assertEqual(document.DrawPages.getCount(), 1)  #nr. of pages

   def test_tdf85360_insert_4th_slide(self):
        with self.ui_test.create_doc_in_start_center("impress") as document:
            xTemplateDlg = self.xUITest.getTopFocusWindow()
            xCancelBtn = xTemplateDlg.getChild("close")
            self.ui_test.close_dialog_through_button(xCancelBtn)
            self.assertEqual(document.DrawPages.getCount(), 1)  #nr. of pages

            self.xUITest.executeCommand(".uno:InsertPage")
            self.xUITest.executeCommand(".uno:InsertPage")
            self.xUITest.executeCommand(".uno:InsertPage")
            self.xUITest.executeCommand(".uno:InsertPage")
            #verify
            self.assertEqual(document.DrawPages.getCount(), 5)  #nr. of pages
            #undo
            self.xUITest.executeCommand(".uno:Undo")
            self.xUITest.executeCommand(".uno:Undo")
            self.xUITest.executeCommand(".uno:Undo")
            self.xUITest.executeCommand(".uno:Undo")
            self.assertEqual(document.DrawPages.getCount(), 1)  #nr. of pages
# vim: set shiftwidth=4 softtabstop=4 expandtab:
