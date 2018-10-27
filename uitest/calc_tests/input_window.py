# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, type_text
from libreoffice.uno.propertyvalue import mkPropertyValues

from libreoffice.calc.document import get_cell_by_position

import time

class InputWindowTest(UITestCase):

    def test_input_window(self):

        self.ui_test.create_doc_in_start_center("calc")

        xCalcDoc = self.xUITest.getTopFocusWindow()
        document = self.ui_test.get_component()

        xInputWin = xCalcDoc.getChild("sc_input_window")

        type_text(xInputWin, "test")
        xInputWin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "RETURN"}))
        self.assertEqual(get_cell_by_position(document, 0, 0, 0).getString(), "test")

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
