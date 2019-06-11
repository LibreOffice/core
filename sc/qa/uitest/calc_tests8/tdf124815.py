# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict
from uitest.uihelper.common import select_pos
from uitest.uihelper.calc import enter_text_to_cell
from libreoffice.calc.document import get_sheet_from_doc
from libreoffice.calc.conditional_format import get_conditional_format_from_sheet
from uitest.debug import sleep
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues
import org.libreoffice.unotest
import pathlib

def get_url_for_data_file(file_name):
    return pathlib.Path(org.libreoffice.unotest.makeCopyFromTDOC(file_name)).as_uri()

#Bug 124815 - CRASH: inserting a column

class tdf124815(UITestCase):
    def test_tdf124815_crash_inserting_column(self):
        #numberingformatpage.ui
        calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf124815.ods"))
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()

        #Go to Sheet - Insert Column - Insert Column before .uno:InsertColumnsBefore
        self.xUITest.executeCommand(".uno:InsertColumnsBefore")
        #verify
        self.assertEqual(get_cell_by_position(document, 0, 3, 0).getString(), "Rakennukset")
        self.xUITest.executeCommand(".uno:Undo")
        self.assertEqual(get_cell_by_position(document, 0, 2, 0).getString(), "Rakennukset")

        self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab:
