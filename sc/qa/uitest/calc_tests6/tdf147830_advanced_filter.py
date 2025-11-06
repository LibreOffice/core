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

class tdf147830(UITestCase):

    def test_tdf147830(self):

        with self.ui_test.load_file(get_url_for_data_file("tdf147830.ods")) as calc_doc:

            with self.ui_test.execute_modeless_dialog_through_command(".uno:DataFilterSpecialFilter") as xDialog:
                xFilterArea = xDialog.getChild("lbfilterarea")
                self.assertEqual("- undefined -", get_state_as_dict(xFilterArea)["SelectEntryText"])
                self.assertEqual("5", get_state_as_dict(xFilterArea)["EntryCount"])

                select_by_text(xFilterArea, "Irgendwo")

                xEditFilterArea = xDialog.getChild("edfilterarea")
                self.assertEqual("$Filterkriterien.$A$40:$AMF$60", get_state_as_dict(xEditFilterArea)["Text"])

            # Without the fix in place, this test would have crashed here

            self.assertFalse(is_row_hidden(calc_doc, 0))
            for i in range(1, 17):
                self.assertTrue(is_row_hidden(calc_doc, i))

            self.assertFalse(is_row_hidden(calc_doc, 17))
            self.assertTrue(is_row_hidden(calc_doc, 18))
            self.assertTrue(is_row_hidden(calc_doc, 19))
            self.assertTrue(is_row_hidden(calc_doc, 20))

            for i in range(21, 26):
                self.assertFalse(is_row_hidden(calc_doc, i))

            self.assertTrue(is_row_hidden(calc_doc, 27))
            self.assertFalse(is_row_hidden(calc_doc, 28))
            self.assertFalse(is_row_hidden(calc_doc, 29))

            self.xUITest.executeCommand(".uno:Undo")

            for i in range(30):
                self.assertFalse(is_row_hidden(calc_doc, i))

# vim: set shiftwidth=4 softtabstop=4 expandtab:
