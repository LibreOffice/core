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
from libreoffice.uno.propertyvalue import mkPropertyValues

class tdf142472(UITestCase):
    def test_tdf142472_name_box_autocomplete_case(self):
        with self.ui_test.create_doc_in_start_center("calc"):
            calcDoc = self.xUITest.getTopFocusWindow()
            xPosWindow = calcDoc.getChild('pos_window')

            # Type "MAPS" in order to partially match the prefix for "Manage Names..."
            xPosWindow.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
            xPosWindow.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
            xPosWindow.executeAction("TYPE", mkPropertyValues({"TEXT":"MAPS"}))
            xPosWindow.executeAction("TYPE", mkPropertyValues({"KEYCODE":"RETURN"}))

            # Without the fix in place, this test would have failed with
            # AssertionError: 'MAPS' != 'MaPS'
            # i.e. "MA" was autocompleted to "Manage Names..." and its case changed to "Ma"
            self.assertEqual('MAPS', get_state_as_dict(xPosWindow)['Text'])

# vim: set shiftwidth=4 softtabstop=4 expandtab:
