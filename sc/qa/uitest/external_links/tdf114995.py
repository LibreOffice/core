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

from libreoffice.uno.propertyvalue import mkPropertyValues
from libreoffice.calc.document import get_cell_by_position


class tdf114995(UITestCase):

    def test_tdf114995(self):

        with self.ui_test.create_doc_in_start_center("calc") as document:

            with self.ui_test.execute_dialog_through_command(".uno:InsertExternalDataSource", close_button="") as xDialog:
                xUrl = xDialog.getChild("url")

                # Reuse existing file
                xUrl.executeAction("TYPE", mkPropertyValues({"TEXT": get_url_for_data_file("tdf60468.csv")}))

                with self.ui_test.execute_blocking_action(
                        xUrl.executeAction, args=("TYPE", mkPropertyValues({"KEYCODE": "RETURN"}))):
                    pass

                xRanges = xDialog.getChild("ranges")

                # Without the fix in place, this test would have failed with
                # AssertionError: '1' != '0'
                self.assertEqual("1", get_state_as_dict(xRanges)["Children"])

                self.assertEqual("CSV_all", get_state_as_dict(xRanges.getChild("0"))["Text"])

                # FIXME: close_dialog_through_button fails here
                xOkBtn = xDialog.getChild("ok")
                xOkBtn.executeAction("CLICK", tuple())

            for i in range(3):
                self.assertEqual("head" + str(i + 1), get_cell_by_position(document, 0, i, 0).getString())

            self.assertEqual("value1.1\nvalue1.2", get_cell_by_position(document, 0, 0, 1).getString())

            self.assertEqual("value2.1\n\tvalue2.2\nvalue2.3", get_cell_by_position(document, 0, 1, 1).getString())
            self.assertEqual("value3", get_cell_by_position(document, 0, 2, 1).getString())

# vim: set shiftwidth=4 softtabstop=4 expandtab:
