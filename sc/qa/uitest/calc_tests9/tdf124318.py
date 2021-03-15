# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file
from libreoffice.uno.propertyvalue import mkPropertyValues

class tdf124318(UITestCase):

    def test_tdf124318(self):

        calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf124318.xls"))

        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")

        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "B4"}))

        self.ui_test.execute_dialog_through_action(gridwin, "TYPE", mkPropertyValues({"KEYCODE": "CTRL+k"}))
        xDialog = self.xUITest.getTopFocusWindow()

        # Without the fix in place, this test would have failed with
        # AssertionError: 'mobile/identification/authentification' !=
        # 'mobile/identification/authentification%00塹\uf481ᴻ䡿ⲯ嶂藄挧%00%00ꮥ%00'
        self.assertEqual("mobile/identification/authentification",
                get_state_as_dict(xDialog.getChild("target"))['Text'].split(".fr/")[1])
        self.assertEqual("mobile/identification/authentification",
                get_state_as_dict(xDialog.getChild("indication"))['Text'].split(".fr/")[1])

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
