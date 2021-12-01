# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
#uitest sw / Title Page dialog

class titlePage(UITestCase):
    def test_title_page(self):
        with self.ui_test.create_doc_in_start_center("writer") as document:
            xWriterDoc = self.xUITest.getTopFocusWindow()
            #dialog Title Page
            with self.ui_test.execute_dialog_through_command(".uno:TitlePageDialog") as xDialog:
                #select new Pages; nr of pages =2 (click UP), save; verify pageCount = 3
                newPages = xDialog.getChild("RB_INSERT_NEW_PAGES")
                newPages.executeAction("CLICK", tuple())
                xpageCount = xDialog.getChild("NF_PAGE_COUNT")
                xpageCount.executeAction("UP", tuple())
            self.assertEqual(document.CurrentController.PageCount, 3)

            # check cancel button
            with self.ui_test.execute_dialog_through_command(".uno:TitlePageDialog", close_button="cancel"):
                pass
            self.assertEqual(document.CurrentController.PageCount, 3)


# vim: set shiftwidth=4 softtabstop=4 expandtab:
