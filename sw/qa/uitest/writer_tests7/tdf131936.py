# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.config import DEFAULT_SLEEP
import time

class tdf131936(UITestCase):

    def test_tdf131936_saveas_docx_version(self):
        self.ui_test.load_file(get_url_for_data_file("tdf131936.docx"))

        self.ui_test.execute_dialog_through_command(".uno:SaveAs")
        time.sleep(DEFAULT_SLEEP)
        xDialog = self.xUITest.getTopFocusWindow()
        xFileTypeCombo = xDialog.getChild("file_type")
        state = get_state_as_dict(xFileTypeCombo)
        self.assertEqual(state["SelectEntryText"], "Office Open XML Text (Transitional) (.docx)")

        xCancel = xDialog.getChild("cancel")
        self.ui_test.close_dialog_through_button(xCancel)

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
