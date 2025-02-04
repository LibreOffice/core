# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_url_for_data_file, get_state_as_dict, select_by_text
from libreoffice.calc.document import is_row_hidden

class tdf149907(UITestCase):

    def test_tdf149907(self):

        with self.ui_test.load_file(get_url_for_data_file("tdf149907.ods")) as doc:

            with self.ui_test.execute_modeless_dialog_through_command(".uno:DataFilterStandardFilter") as xDialog:
                xField1 = xDialog.getChild("field1")
                xVal1 = xDialog.getChild("val1")
                xCond1 = xDialog.getChild("cond1")
                self.assertEqual('Test', get_state_as_dict(xField1)['SelectEntryText'])
                self.assertEqual('=', get_state_as_dict(xCond1)['SelectEntryText'])
                self.assertEqual('1', get_state_as_dict(xVal1)['Text'])
                select_by_text(xCond1, ">")

            self.assertFalse(is_row_hidden(doc, 0))
            # Without the fix in place, it would have failed here with AssertionError: False is not true
            self.assertTrue(is_row_hidden(doc, 1))
            self.assertFalse(is_row_hidden(doc, 2))
            self.assertFalse(is_row_hidden(doc, 3))
            self.assertFalse(is_row_hidden(doc, 4))
            self.assertFalse(is_row_hidden(doc, 5))
            self.assertFalse(is_row_hidden(doc, 6))

# vim: set shiftwidth=4 softtabstop=4 expandtab:
