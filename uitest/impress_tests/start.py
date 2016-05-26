# -*- Mode: python; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest_helper import UITest, get_state_as_dict

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

def start_impress(xContext):
    xUITest = xContext.ServiceManager.createInstanceWithContext(
            "org.libreoffice.uitest.UITest", xContext)

    ui_test = UITest(xUITest, xContext)

    ui_test.create_doc_in_start_center("impress")

    xImpressDoc = xUITest.getTopFocusWindow()
    print(get_state_as_dict(xImpressDoc))
    print(xImpressDoc.getChildren())

    xEditWin = xImpressDoc.getChild("impress_win")
    xEditWin.executeAction("SET", mkPropertyValues({"ZOOM": "200"}))

    print(xEditWin.getChildren())

    time.sleep(5)

    ui_test.close_doc()

def select_page(xContext):
    xUITest = xContext.ServiceManager.createInstanceWithContext(
            "org.libreoffice.uitest.UITest", xContext)

    ui_test = UITest(xUITest, xContext)

    ui_test.create_doc_in_start_center("impress")

    xImpressDoc = xUITest.getTopFocusWindow()
    print(get_state_as_dict(xImpressDoc))
    print(xImpressDoc.getChildren())

    xEditWin = xImpressDoc.getChild("impress_win")

    time.sleep(10)
    print(get_state_as_dict(xEditWin))

    xEditWin.executeAction("GOTO", mkPropertyValues({"PAGE": "2"}))

    time.sleep(5)

    print(get_state_as_dict(xEditWin))

    ui_test.close_doc()

def select_text(xContext):
    xUITest = xContext.ServiceManager.createInstanceWithContext(
            "org.libreoffice.uitest.UITest", xContext)

    ui_test = UITest(xUITest, xContext)

    ui_test.create_doc_in_start_center("impress")

    xImpressDoc = xUITest.getTopFocusWindow()
    print(get_state_as_dict(xImpressDoc))
    print(xImpressDoc.getChildren())

    xEditWin = xImpressDoc.getChild("impress_win")

    time.sleep(10)

    print(get_state_as_dict(xEditWin))

    ui_test.close_doc()

def select_object(xContext):
    xUITest = xContext.ServiceManager.createInstanceWithContext(
            "org.libreoffice.uitest.UITest", xContext)

    ui_test = UITest(xUITest, xContext)

    ui_test.create_doc_in_start_center("impress")

    xImpressDoc = xUITest.getTopFocusWindow()
    print(get_state_as_dict(xImpressDoc))
    print(xImpressDoc.getChildren())

    xEditWin = xImpressDoc.getChild("impress_win")

    xEditWin.executeAction("SELECT", mkPropertyValues({"OBJECT":"Unnamed Drawinglayer object 1"}))
    time.sleep(2)
    xEditWin.executeAction("DESELECT", tuple())

    time.sleep(4)

    print(get_state_as_dict(xEditWin))

    ui_test.close_doc()

# vim:set shiftwidth=4 softtabstop=4 expandtab: */
