# -*- Mode: python; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

import time

from uitest_helper import UITest

from helper import mkPropertyValues
from uihelper.calc import enter_text_to_cell

from uitest_helper import get_state_as_dict

try:
    import pyuno
    import uno
    import unohelper
except ImportError:
    print("pyuno not found: try to set PYTHONPATH and URE_BOOTSTRAP variables")
    print("PYTHONPATH=/installation/opt/program")
    print("URE_BOOTSTRAP=file:///installation/opt/program/fundamentalrc")
    raise

def expand(xContext):
    xUITest = xContext.ServiceManager.createInstanceWithContext(
            "org.libreoffice.uitest.UITest", xContext)

    ui_test = UITest(xUITest, xContext)

    ui_test.create_doc_in_start_center("calc")

    xCalcDoc = xUITest.getTopFocusWindow()
    xGridWindow = xCalcDoc.getChild("grid_window")
    enter_text_to_cell(xGridWindow, "B2", "=2+3+4")
    xGridWindow.executeAction("SELECT", mkPropertyValues({"CELL": "B2"}))

    ui_test.execute_modeless_dialog_through_command(".uno:FunctionDialog")

    xFunctionDlg = xUITest.getTopFocusWindow()

    xTabs = xFunctionDlg.getChild("tabs")
    xTabs.executeAction("SELECT", mkPropertyValues({"POS":"1"}))

    xTreelist = xTabs.getChild("struct")

    xTreeEntry = xTreelist.getChild('0')

    xTreeEntry.executeAction("COLLAPSE", tuple())

    time.sleep(1)

    xTreeEntry.executeAction("EXPAND", tuple())

    time.sleep(1)

    xCancelBtn = xFunctionDlg.getChild("cancel")
    xCancelBtn.executeAction("CLICK", tuple())

    ui_test.close_doc()

# vim:set shiftwidth=4 softtabstop=4 expandtab: */
