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
# import org.libreoffice.unotest
# import pathlib
from uitest.path import get_srcdir_url
def get_url_for_data_file(file_name):
#    return pathlib.Path(org.libreoffice.unotest.makeCopyFromTDOC(file_name)).as_uri()
    return get_srcdir_url() + "/sc/qa/uitest/calc_tests/data/" + file_name

#Bug 93328 - Editing circular reference causes #VALUE! error
#In the attached spreadsheet, when either the cell g15 or g27 is edited (say include a "+1"), a #VALUE! error is generated throughout the circular reference loop.
#Editing of other cells in the loop do not (seem to consistently) generate this error.
class tdf93328(UITestCase):
    def test_tdf93328(self):
        calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf93328.ods"))
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        enter_text_to_cell(gridwin, "G27", "=SUM(G24:G26)+1")

        #Verify
        self.assertEqual(round(get_cell_by_position(document, 0, 6, 26).getValue() ,2), 6427.89)

        #undo
        self.xUITest.executeCommand(".uno:Undo")
        self.assertEqual(round(get_cell_by_position(document, 0, 6, 26).getValue() ,2), 6426.95)

        self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab:
