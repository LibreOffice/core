# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_url_for_data_file, select_by_text
from libreoffice.uno.propertyvalue import mkPropertyValues
from libreoffice.calc.document import is_row_hidden

class tdf163395(UITestCase):

    def test_tdf163395(self):

        with self.ui_test.load_file(get_url_for_data_file("tdf163395.ods")) as doc:

            xGridWin = self.xUITest.getTopFocusWindow().getChild("grid_window")
            xGridWin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:B7"}))
            #Choose DATA-FILTER-STANDARDFILTER
            #Choose field name "Percent"/ Choose condition ">="/Enter value "23%"/Press OK button
            with self.ui_test.execute_modeless_dialog_through_command(".uno:DataFilterStandardFilter") as xDialog:
                xfield1 = xDialog.getChild("field1")
                xval1 = xDialog.getChild("val1")
                xcond1 = xDialog.getChild("cond1")
                select_by_text(xfield1, "Percent")
                select_by_text(xcond1, ">=")
                xval1.executeAction("TYPE", mkPropertyValues({"TEXT":"23%"}))

            self.assertFalse(is_row_hidden(doc, 0))
            self.assertFalse(is_row_hidden(doc, 1))
            self.assertFalse(is_row_hidden(doc, 2))
            self.assertFalse(is_row_hidden(doc, 3))
            self.assertFalse(is_row_hidden(doc, 4))
            self.assertTrue(is_row_hidden(doc, 5))
            self.assertFalse(is_row_hidden(doc, 6))

# vim: set shiftwidth=4 softtabstop=4 expandtab:
