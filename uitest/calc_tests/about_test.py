# -*- Mode: python; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

import time

from uitest_helper import UITest

def test_about_dlg(xContext):
    xUITest = xContext.ServiceManager.createInstanceWithContext(
            "org.libreoffice.uitest.UITest", xContext)

    ui_test = UITest(xUITest, xContext)

    ui_test.create_doc_in_start_center("calc")

    ui_test.execute_dialog_through_command(".uno:About")

    xAboutDlg = xUITest.getTopFocusWindow()

    xCloseBtn = xAboutDlg.getChild("close")
    xCloseBtn.executeAction("CLICK", tuple())

# vim:set shiftwidth=4 softtabstop=4 expandtab: */
