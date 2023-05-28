# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import get_url_for_data_file
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues

class tdf137726(UITestCase):

    def test_tdf137726(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf131638.ods")) as calc_doc:
            MainWindow = self.xUITest.getTopFocusWindow()
            grid_window = MainWindow.getChild("grid_window")
            grid_window.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
            grid_window.executeAction("TYPE", mkPropertyValues({"TEXT": "-/2"}))
            grid_window.executeAction("TYPE", mkPropertyValues({"KEYCODE": "RETURN"}))
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 0).getString(), "-/2")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
