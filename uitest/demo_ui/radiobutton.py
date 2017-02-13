# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from libreoffice.uno.propertyvalue import mkPropertyValues

from uitest.framework import UITestCase

import time

try:
    import pyuno
    import uno
    import unohelper
except ImportError:
    print("pyuno not found: try to set PYTHONPATH and URE_BOOTSTRAP variables")
    print("PYTHONPATH=/installation/opt/program")
    print("URE_BOOTSTRAP=file:///installation/opt/program/fundamentalrc")
    raise

class RadioButtonTest(UITestCase):

    def test_toggle_radiobutton(self):

        self.ui_test.create_doc_in_start_center("calc")

        self.ui_test.execute_dialog_through_command(".uno:FormatCellDialog")
        xCellsDlg = self.xUITest.getTopFocusWindow()
        xNegativeNumRedCB = xCellsDlg.getChild("negnumred")
        xNegativeNumRedCB.executeAction("CLICK",tuple())

        okBtn = xCellsDlg.getChild("ok")
        self.ui_test.close_dialog_through_button(okBtn)

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
