# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-

from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
import importlib
from uitest.debug import sleep
from uitest.uihelper.common import select_pos
from uitest.uihelper.common import get_state_as_dict, type_text

class customSlideShowDialog(UITestCase):
    def test_customSlideShowDialog(self):
        self.ui_test.create_doc_in_start_center("impress")
        MainWindow = self.xUITest.getTopFocusWindow()
        TemplateDialog = self.xUITest.getTopFocusWindow()
        cancel = TemplateDialog.getChild("close")
        self.ui_test.close_dialog_through_button(cancel)

        self.ui_test.execute_dialog_through_command(".uno:CustomShowDialog")
        xDialog = self.xUITest.getTopFocusWindow()

        # Without the fix in place, this test would have crashed here
        self.ui_test.close_dialog_through_button(xDialog.getChild("ok"))

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
