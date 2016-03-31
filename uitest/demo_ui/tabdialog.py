# -*- Mode: python; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest_helper import UITest

from helper import mkPropertyValues

import time

def select_tab_page_pos(xContext):
    xUITest = xContext.ServiceManager.createInstanceWithContext(
            "org.libreoffice.uitest.UITest", xContext)

    ui_test = UITest(xUITest, xContext)

    ui_test.create_doc_in_start_center("calc")

    ui_test.execute_dialog_through_command(".uno:FormatCellDialog")
    xCellsDlg = xUITest.getTopFocusWindow()

    props = {"POS": "1"}
    propsUNO = mkPropertyValues(props)
    xCellsDlg.executeAction("SELECT", propsUNO)

    time.sleep(20)

def select_tab_page_name(xContext):
    xUITest = xContext.ServiceManager.createInstanceWithContext(
            "org.libreoffice.uitest.UITest", xContext)

    ui_test = UITest(xUITest, xContext)

    ui_test.create_doc_in_start_center("calc")

    ui_test.execute_dialog_through_command(".uno:FormatCellDialog")
    xCellsDlg = xUITest.getTopFocusWindow()

    props = {"NAME": "Borders"}
    propsUNO = mkPropertyValues(props)
    xCellsDlg.executeAction("SELECT", propsUNO)

    time.sleep(20)


# vim:set shiftwidth=4 softtabstop=4 expandtab: */
