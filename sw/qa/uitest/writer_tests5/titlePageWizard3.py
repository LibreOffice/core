# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_url_for_data_file
#import time

# This tests the Format->Title Page wizard, specifically inserting pages before a TOC or table,
# which are edge cases. (The TOC is harder to test because it works but gets lost on a TOC update.)
class tdf114343(UITestCase):
    def test_tdf114343(self):  # table test
        with self.ui_test.load_file(get_url_for_data_file("tdf114343_titlePageDialog.odt")) as document:

            # Confirm the starting state.
            self.assertEqual(document.CurrentController.PageCount, 1)
            #time.sleep(2)

            #dialog Title Page
            with self.ui_test.execute_dialog_through_command(".uno:TitlePageDialog") as xDialog:
                #Insert three title/index pages at the end of the document (plus a content page).
                newPages = xDialog.getChild("RB_INSERT_NEW_PAGES")
                newPages.executeAction("CLICK", tuple())
                #time.sleep(4)

            # an extra paragraph was added before TOC/Table so splitNode could work.
            self.assertEqual(document.CurrentController.PageCount, 2)
            #time.sleep(5)

    def test2_tdf114343(self):  # TOC test
        with self.ui_test.load_file(get_url_for_data_file("tdf114343_titlePageDialogB.odt")) as document:

            # Confirm the starting state.
            self.assertEqual(document.CurrentController.PageCount, 1)

            #dialog Title Page
            with self.ui_test.execute_dialog_through_command(".uno:TitlePageDialog") as xDialog:
                #Insert three title/index pages at the end of the document (plus a content page).
                newPages = xDialog.getChild("RB_INSERT_NEW_PAGES")
                newPages.executeAction("CLICK", tuple())
                #time.sleep(4)

            # update the TOC
            self.xUITest.executeCommand(".uno:UpdateAllIndexes")
            # an extra paragraph was added before TOC/Table so splitNode could work.
            self.assertEqual(document.CurrentController.PageCount, 2)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
