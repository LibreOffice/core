# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.uihelper.common import get_state_as_dict
from uitest.uihelper.common import get_url_for_data_file
from libreoffice.uno.propertyvalue import mkPropertyValues

# opens the dialogs, closes it with the given close button
# and if there is an "OK" button open the dialog again and close it by using the OK button
# the test only checks if LibreOffice crashes by opening the dialog
def testAppDialog(UITestCase, app, dialog):
    with UITestCase.ui_test.create_doc_in_start_center(app):
        with UITestCase.ui_test.execute_dialog_through_command(
                dialog['command'], close_button=dialog['closeButton']) as xDialog:
            if 'skipTestOK' in dialog and dialog['skipTestOK'] == True:
                xOKBtn = None
            else:
                try:
                    xOKBtn = xDialog.getChild("ok")
                    if (get_state_as_dict(xOKBtn)["Enabled"] != "true"):
                        xOKBtn = None
                except:
                    xOKBtn = None

        if (xOKBtn != None):
            print("check also OK button")
            with UITestCase.ui_test.execute_dialog_through_command(dialog['command']):
                pass

# opens the dialogs, closes it with the given close button
# and if there is an "OK" button open the dialog again and close it by using the OK button
# the test only checks if LibreOffice crashes by opening the dialog
def testChartDialog(UITestCase, dialog):
    with UITestCase.ui_test.load_file(get_url_for_data_file("chart.ods")):
        xCalcDoc = UITestCase.xUITest.getTopFocusWindow()
        xGridWin = xCalcDoc.getChild("grid_window")
        xGridWin.executeAction("SELECT", mkPropertyValues({"OBJECT": "Object 1"}))
        xGridWin.executeAction("ACTIVATE", tuple())
        xChartMainTop = UITestCase.xUITest.getTopFocusWindow()
        xChartMain = xChartMainTop.getChild("chart_window")
        xObj = xChartMain.getChild("CID/D=0")
        with UITestCase.ui_test.execute_dialog_through_action(
                xObj, "COMMAND",
                mkPropertyValues({"COMMAND": dialog['command']}),
                close_button=dialog['closeButton']) as xDialog:
            if 'skipTestOK' in dialog and dialog['skipTestOK'] == True:
                xOKBtn = None
            else:
                try:
                    xOKBtn = xDialog.getChild("ok")
                    if (get_state_as_dict(xOKBtn)["Enabled"] != "true"):
                        xOKBtn = None
                except:
                    xOKBtn = None

        if (xOKBtn != None):
            print("check also OK button")
            with UITestCase.ui_test.execute_dialog_through_action(
                    xObj, "COMMAND",
                    mkPropertyValues({"COMMAND": dialog['command']})):
                pass

# vim: set shiftwidth=4 softtabstop=4 expandtab:
