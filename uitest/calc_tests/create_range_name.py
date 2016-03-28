# -*- Mode: python; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest_helper import UITest

from helper import mkPropertyValues

try:
    import pyuno
    import uno
    import unohelper
except ImportError:
    print("pyuno not found: try to set PYTHONPATH and URE_BOOTSTRAP variables")
    print("PYTHONPATH=/installation/opt/program")
    print("URE_BOOTSTRAP=file:///installation/opt/program/fundamentalrc")
    raise

def create_range_name(xContext):
    xUITest = xContext.ServiceManager.createInstanceWithContext(
            "org.libreoffice.uitest.UITest", xContext)

    ui_test = UITest(xUITest, xContext)

    ui_test.create_doc_in_start_center("calc")

    ui_test.execute_modeless_dialog_through_command(".uno:AddName")

    xAddNameDlg = xUITest.getTopFocusWindow()

    props = {"TEXT": "simpleRangeName"}
    actionProps = mkPropertyValues(props)

    xEdit = xAddNameDlg.getChild("edit")
    xEdit.executeAction("TYPE", actionProps)
    xAddBtn = xAddNameDlg.getChild("add")
    xAddBtn.executeAction("CLICK", tuple())

def create_local_range_name(xContext):
    xUITest = xContext.ServiceManager.createInstanceWithContext(
            "org.libreoffice.uitest.UITest", xContext)

    ui_test = UITest(xUITest, xContext)

    ui_test.create_doc_in_start_center("calc")

    ui_test.execute_modeless_dialog_through_command(".uno:AddName")

    xAddNameDlg = xUITest.getTopFocusWindow()

    props = {"TEXT": "simpleRangeName"}
    actionProps = mkPropertyValues(props)

    xEdit = xAddNameDlg.getChild("edit")
    xEdit.executeAction("TYPE", actionProps)

    xScope = xAddNameDlg.getChild("scope")
    props = {"POS": "1"}
    scopeProps = mkPropertyValues(props)
    xScope.executeAction("SELECT", scopeProps)

    xAddBtn = xAddNameDlg.getChild("add")
    xAddBtn.executeAction("CLICK", tuple())

# vim:set shiftwidth=4 softtabstop=4 expandtab: */
