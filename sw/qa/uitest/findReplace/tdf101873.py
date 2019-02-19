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
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict, type_text
import org.libreoffice.unotest
import pathlib
from uitest.path import get_srcdir_url
def get_url_for_data_file(file_name):
    return pathlib.Path(org.libreoffice.unotest.makeCopyFromTDOC(file_name)).as_uri()

#Bug 101873 - Cursor jumps to the begin of the document after searching for text not found

class tdf101873(UITestCase):
    def test_tdf101873_find_not_found_jump(self):
        writer_doc = self.ui_test.load_file(get_url_for_data_file("tdf101873.odt"))
        document = self.ui_test.get_component()
        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")
        #go to page 2
        self.ui_test.execute_dialog_through_command(".uno:GotoPage")
        xDialog = self.xUITest.getTopFocusWindow()
        xPageText = xDialog.getChild("page")
        xPageText.executeAction("TYPE", mkPropertyValues({"TEXT":"2"}))
        xOkBtn = xDialog.getChild("ok")
        xOkBtn.executeAction("CLICK", tuple())
        self.assertEqual(get_state_as_dict(xWriterEdit)["CurrentPage"], "2") #I'm on the 2nd page

        self.ui_test.execute_modeless_dialog_through_command(".uno:SearchDialog")
        xDialog = self.xUITest.getTopFocusWindow()

        searchterm = xDialog.getChild("searchterm")
        searchterm.executeAction("TYPE", mkPropertyValues({"TEXT":"notFound"}))
        xsearch = xDialog.getChild("search")
        xsearch.executeAction("CLICK", tuple())
        xsearch.executeAction("CLICK", tuple())
        #verify
        self.assertEqual(get_state_as_dict(xWriterEdit)["CurrentPage"], "2")

        xcloseBtn = xDialog.getChild("close")
        self.ui_test.close_dialog_through_button(xcloseBtn)

        self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab:
