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
from uitest.uihelper.common import select_pos

class WriterInsertBreakDialog(UITestCase):

    def getPages(self, total):
        document = self.ui_test.get_component()

        self.assertEqual(document.CurrentController.PageCount, total)

    def test_insert_line_break(self):

        with self.ui_test.create_doc_in_start_center("writer"):

            with self.ui_test.execute_dialog_through_command(".uno:InsertBreak") as xDialog:
                xOption = xDialog.getChild("linerb")
                xOption.executeAction("CLICK", tuple())

            self.getPages(1)


    def test_insert_column_break(self):

        with self.ui_test.create_doc_in_start_center("writer"):

            with self.ui_test.execute_dialog_through_command(".uno:InsertBreak") as xDialog:
                xOption = xDialog.getChild("columnrb")
                xOption.executeAction("CLICK", tuple())

            self.getPages(1)


    def test_insert_page_break(self):

        with self.ui_test.create_doc_in_start_center("writer"):

            for i in range(9):
                with self.subTest(i=i):
                    with self.ui_test.execute_dialog_through_command(".uno:InsertBreak") as xDialog:

                        xOption = xDialog.getChild("pagerb")
                        xOption.executeAction("CLICK", tuple())

                        xStyleList = xDialog.getChild("stylelb")
                        select_pos(xStyleList, str(i))

                    self.getPages(i + 2)


    def test_cancel_button_insert_line_break_dialog(self):

        with self.ui_test.create_doc_in_start_center("writer"):

            with self.ui_test.execute_dialog_through_command(".uno:InsertBreak", close_button="cancel"):
                pass

            self.getPages(1)


# vim: set shiftwidth=4 softtabstop=4 expandtab:
