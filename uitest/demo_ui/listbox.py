# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase

class ListBoxTest(UITestCase):

    def test_select_entry_pos(self):

        with self.ui_test.create_doc_in_start_center("calc"):

            with self.ui_test.execute_dialog_through_command(".uno:FormatCellDialog"):
                pass


    def test_select_entry_text(self):

        with self.ui_test.create_doc_in_start_center("calc"):

            with self.ui_test.execute_dialog_through_command(".uno:FormatCellDialog"):
                pass


# vim: set shiftwidth=4 softtabstop=4 expandtab:
