# -*- Mode: python; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest_helper import UITest

from helper import mkPropertyValues
from uitest_helper import get_state_as_dict

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

def up(xContext):
    xUITest = xContext.ServiceManager.createInstanceWithContext(
            "org.libreoffice.uitest.UITest", xContext)

    ui_test = UITest(xUITest, xContext)

    ui_test.create_doc_in_start_center("calc")

    ui_test.execute_dialog_through_command(".uno:FormatCellDialog")
    xCellsDlg = xUITest.getTopFocusWindow()

    # select the numbers tab page
    xCellsDlg.executeAction("SELECT", mkPropertyValues({"POS": "0"}))
    
    xDecimalPlaces = xCellsDlg.getChild("leadzerosed")
    xDecimalPlaces.executeAction("UP", tuple())

    decimal_places_state = get_state_as_dict(xDecimalPlaces)
    assert(decimal_places_state["Text"] == "2")

    okBtn = xCellsDlg.getChild("ok")
    okBtn.executeAction("CLICK", tuple())

    ui_test.close_doc()

def down(xContext):
    xUITest = xContext.ServiceManager.createInstanceWithContext(
            "org.libreoffice.uitest.UITest", xContext)

    ui_test = UITest(xUITest, xContext)

    ui_test.create_doc_in_start_center("calc")

    ui_test.execute_dialog_through_command(".uno:FormatCellDialog")
    xCellsDlg = xUITest.getTopFocusWindow()

    # select the numbers tab page
    xCellsDlg.executeAction("SELECT", mkPropertyValues({"POS": "0"}))
    
    xDecimalPlaces = xCellsDlg.getChild("leadzerosed")
    xDecimalPlaces.executeAction("UP", tuple())
    xDecimalPlaces.executeAction("UP", tuple())

    decimal_places_state = get_state_as_dict(xDecimalPlaces)
    assert(decimal_places_state["Text"] == "3")
    
    xDecimalPlaces.executeAction("DOWN", tuple())

    decimal_places_state = get_state_as_dict(xDecimalPlaces)
    assert(decimal_places_state["Text"] == "2")

    okBtn = xCellsDlg.getChild("ok")
    okBtn.executeAction("CLICK", tuple())

    ui_test.close_doc()

def text(xContext):
    xUITest = xContext.ServiceManager.createInstanceWithContext(
            "org.libreoffice.uitest.UITest", xContext)

    ui_test = UITest(xUITest, xContext)

    ui_test.create_doc_in_start_center("calc")

    ui_test.execute_dialog_through_command(".uno:FormatCellDialog")
    xCellsDlg = xUITest.getTopFocusWindow()
    
    xDecimalPlaces = xCellsDlg.getChild("leadzerosed")
    xDecimalPlaces.executeAction("TYPE", mkPropertyValues({"TEXT": "4"}))

    decimal_places_state = get_state_as_dict(xDecimalPlaces)
    assert(decimal_places_state["Text"] == "41")

    okBtn = xCellsDlg.getChild("ok")
    okBtn.executeAction("CLICK", tuple())

    ui_test.close_doc()

# vim:set shiftwidth=4 softtabstop=4 expandtab: */
