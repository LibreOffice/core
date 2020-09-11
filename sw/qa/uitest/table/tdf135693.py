# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-

from uitest.framework import UITestCase
from uitest.path import get_srcdir_url
from uitest.uihelper.common import select_pos
from uitest.uihelper.common import get_state_as_dict

def get_url_for_data_file(file_name):
    return get_srcdir_url() + "/sw/qa/uitest/writer_tests/data/" + file_name

class tdf135693(UITestCase):

    def test_tdf135693(self):
        writer_doc = self.ui_test.load_file(get_url_for_data_file("tdf135693.odt"))

        self.xUITest.getTopFocusWindow()

        self.xUITest.executeCommand(".uno:GoRight")
        self.xUITest.executeCommand(".uno:GoDown")

        # Without the fix in place, this would have crashed here
        self.ui_test.execute_dialog_through_command(".uno:TableDialog")

        xTableDlg = self.xUITest.getTopFocusWindow()

        xTabs = xTableDlg.getChild("tabcontrol")
        select_pos(xTabs, "0")

        # Check we are in the right table
        self.assertEqual("Table1", get_state_as_dict(xTabs.getChild('name'))['Text'])

        xok = xTableDlg.getChild("ok")
        self.ui_test.close_dialog_through_button(xok)

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
