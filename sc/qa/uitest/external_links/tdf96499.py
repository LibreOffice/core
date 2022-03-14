# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict
from uitest.uihelper.common import select_pos
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_url_for_data_file
from libreoffice.calc.document import get_cell_by_position

class tdf96499(UITestCase):

    def test_tdf96499(self):

        with self.ui_test.create_doc_in_start_center("calc") as document:

            with self.ui_test.execute_dialog_through_command(".uno:InsertExternalDataSource", close_button="") as xDialog:
                xUrl = xDialog.getChild("url")

                xUrl.executeAction("TYPE", mkPropertyValues({"TEXT": get_url_for_data_file("tdf96499.html")}))

                with self.ui_test.execute_blocking_action(
                        xUrl.executeAction, args=("TYPE", mkPropertyValues({"KEYCODE": "RETURN"}))):
                    pass

                xRanges = xDialog.getChild("ranges")

                self.assertEqual("3", get_state_as_dict(xRanges)["Children"])

                # FIXME: close_dialog_through_button fails here
                xOkBtn = xDialog.getChild("ok")
                xOkBtn.executeAction("CLICK", tuple())

            self.assertEqual("银行卡号", get_cell_by_position(document, 0, 0, 0).getString())
            self.assertEqual("身份证号", get_cell_by_position(document, 0, 1, 0).getString())

            for i in range (1, 10):
                # Without the fix in place, this test would have failed with
                # AssertionError: '4100025601074122197' != '4,10002560107412E+018'
                self.assertEqual("4100025601074122197", get_cell_by_position(document, 0, 0, i).getString())

# vim: set shiftwidth=4 softtabstop=4 expandtab:
