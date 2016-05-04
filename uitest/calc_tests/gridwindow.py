# -*- Mode: python; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest_helper import UITest

from helper import mkPropertyValues

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

def select_cell(xContext):
    xUITest = xContext.ServiceManager.createInstanceWithContext(
            "org.libreoffice.uitest.UITest", xContext)

    ui_test = UITest(xUITest, xContext)

    ui_test.create_doc_in_start_center("calc")
    xTopWindow = xUITest.getTopFocusWindow()

    xGridWindow = xTopWindow.getChild("grid_window")

    selectProps = mkPropertyValues({"CELL": "B10"})
    xGridWindow.executeAction("SELECT", selectProps)

    time.sleep(20)

def select_range(xContext):
    xUITest = xContext.ServiceManager.createInstanceWithContext(
            "org.libreoffice.uitest.UITest", xContext)

    ui_test = UITest(xUITest, xContext)

    ui_test.create_doc_in_start_center("calc")
    xTopWindow = xUITest.getTopFocusWindow()

    xGridWindow = xTopWindow.getChild("grid_window")

    time.sleep(1)

    selectProps = mkPropertyValues({"RANGE": "B10:C20"})
    xGridWindow.executeAction("SELECT", selectProps)

    time.sleep(20)

# vim:set shiftwidth=4 softtabstop=4 expandtab: */
