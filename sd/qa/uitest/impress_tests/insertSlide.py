# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict
import time
from uitest.debug import sleep
from uitest.uihelper.common import select_pos
#Bug 85360 - LibreOffice crashes and crashes desktop too on inserting 4th slide

class insertSlide(UITestCase):

   def test_insert_slide(self):
        self.ui_test.create_doc_in_start_center("impress")
        xTemplateDlg = self.xUITest.getTopFocusWindow()
        xCancelBtn = xTemplateDlg.getChild("close")
        self.ui_test.close_dialog_through_button(xCancelBtn)
        document = self.ui_test.get_component()
        self.assertEqual(document.DrawPages.getCount(), 1)  #nr. of pages

        self.xUITest.executeCommand(".uno:InsertPage")

        #verify
        self.assertEqual(document.DrawPages.getCount(), 2)  #nr. of pages
        #undo
        self.xUITest.executeCommand(".uno:Undo")
        self.assertEqual(document.DrawPages.getCount(), 1)  #nr. of pages
        self.ui_test.close_doc()

   def test_tdf85360_insert_4th_slide(self):
        self.ui_test.create_doc_in_start_center("impress")
        xTemplateDlg = self.xUITest.getTopFocusWindow()
        xCancelBtn = xTemplateDlg.getChild("close")
        self.ui_test.close_dialog_through_button(xCancelBtn)
        document = self.ui_test.get_component()
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
        self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab:
