# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict

# opens the dialogs, closes it with the given close button
# and if there is an "OK" button open the dialog again and close it by using the OK button
# the test only checks if LibreOffice crashes by opening the dialog
def testDialog(UITestCase, app, dialog):
    doc = UITestCase.ui_test.create_doc_in_start_center(app)
    UITestCase.ui_test.execute_dialog_through_command(dialog['command'])
    xDialog = UITestCase.xUITest.getTopFocusWindow()

    xCloseBtn = xDialog.getChild(dialog['closeButton'])
    if 'skipTestOK' in dialog and dialog['skipTestOK'] == True:
        xOKBtn = None
    else:
        try:
            xOKBtn = xDialog.getChild("ok")
            if (get_state_as_dict(xOKBtn)["Enabled"] != "true"):
                xOKBtn = None
        except:
            xOKBtn = None

    UITestCase.ui_test.close_dialog_through_button(xCloseBtn)
    if (xOKBtn != None):
        print("check also OK button")
        UITestCase.ui_test.execute_dialog_through_command(dialog['command'])
        xDialog = UITestCase.xUITest.getTopFocusWindow()
        xOKBtn = xDialog.getChild("ok")
        UITestCase.ui_test.close_dialog_through_button(xOKBtn)
    UITestCase.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
