# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file

from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues


class tdf95217(UITestCase):

    def test_tdf95217(self):

        with self.ui_test.create_doc_in_start_center("calc") as calc_doc:

            with self.ui_test.execute_dialog_through_command(".uno:InsertExternalDataSource", close_button="") as xDialog:
                xUrl = xDialog.getChild("url")
                xUrl.executeAction("TYPE", mkPropertyValues({"TEXT": get_url_for_data_file("tdf95217.html")}))

                with self.ui_test.execute_blocking_action(xUrl.executeAction, args=("TYPE", mkPropertyValues({"KEYCODE": "RETURN"}))):
                    pass

                xRanges = xDialog.getChild("ranges")
                self.assertEqual("3", get_state_as_dict(xRanges)["Children"])

                # tdf#142600: Without the fix in place, this test would have failed with
                # AssertionError: 'HTML_all' != 'HTML_1'
                self.assertEqual("HTML_all", get_state_as_dict(xRanges.getChild("0"))["Text"])
                self.assertEqual("HTML_tables", get_state_as_dict(xRanges.getChild("1"))["Text"])
                self.assertEqual("HTML_1", get_state_as_dict(xRanges.getChild("2"))["Text"])

                # FIXME: close_dialog_through_button fails here
                xOkBtn = xDialog.getChild("ok")
                xOkBtn.executeAction("CLICK", tuple())

            # Without the fix in place, this test would have failed with
            # AssertionError: 'وضعیت بازار' != 'ÙˆØ¶Ø¹ÛŒØª Ø¨Ø§Ø²Ø§Ø±'
            self.assertEqual("وضعیت بازار", get_cell_by_position(calc_doc, 0, 0, 0).getString())
            self.assertEqual("شاخص کل", get_cell_by_position(calc_doc, 0, 0, 1).getString())
            self.assertEqual("شاخص كل (هم وزن)", get_cell_by_position(calc_doc, 0, 0, 2).getString())
            self.assertEqual("بسته", get_cell_by_position(calc_doc, 0, 1, 0).getString())
            self.assertEqual("63175", get_cell_by_position(calc_doc, 0, 1, 1).getString())
            self.assertEqual("9,690.30 (22.80)", get_cell_by_position(calc_doc, 0, 1, 2).getString())

# vim: set shiftwidth=4 softtabstop=4 expandtab:
