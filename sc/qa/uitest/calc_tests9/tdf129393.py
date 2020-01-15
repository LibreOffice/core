# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict
from libreoffice.uno.propertyvalue import mkPropertyValues
from libreoffice.calc.document import get_cell_by_position
import pathlib
import org.libreoffice.unotest
import time

def get_url_for_data_file(file_name):
    return pathlib.Path(org.libreoffice.unotest.makeCopyFromTDOC(file_name)).as_uri()

class tdf129393(UITestCase):

    def test_tdf_129393_change_background_unprotected_cell(self):
        calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf129393.ods"))
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A2"}))
        self.xUITest.executeCommand(".uno:SelectRow")

        #Wait for the error message to be displayed ( without the accompanying fix in place )
        #otherwise the background is changed before the error is prompted and it doens't fail
        time.sleep(1)

        colorProperty = mkPropertyValues({"BackgroundColor": 16776960})
        self.xUITest.executeCommandWithParameters(".uno:BackgroundColor", colorProperty)

        self.assertEqual(get_cell_by_position(document, 0, 0, 1).CellBackColor, 16776960)

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
