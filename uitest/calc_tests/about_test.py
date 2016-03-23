# -*- Mode: python; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

import time

from uitest_helper import UITest
from helper import Screenshot

def get_bounding_box(props):
    size_str = None
    pos_str = None
    for prop in props:
        if prop.Name == "AbsPosition":
            pos_str = prop.Value
        elif prop.Name == "Size":
            size_str = prop.Value

    x1, y1 = pos_str.split("x")
    dx, dy = size_str.split("x")
    return x1, y1, str(int(x1) + int(dx)), str(int(y1) + int(dy))

def test_about_dlg_with_screenshot(xContext):
    xUITest = xContext.ServiceManager.createInstanceWithContext(
            "org.libreoffice.uitest.UITest", xContext)

    ui_test = UITest(xUITest, xContext)

    ui_test.create_doc_in_start_center("calc")

    ui_test.execute_dialog_through_command(".uno:About")

    xAboutDlg = xUITest.getTopFocusWindow()

    # take the screenshot
    time.sleep(1)
    aboutDlgState = xAboutDlg.getState()
    x1, y1, x2, y2 = get_bounding_box(aboutDlgState)
    screenshot = Screenshot()
    screenshot.take_screenshot(x1, y1, x2, y2)

    xCloseBtn = xAboutDlg.getChild("close")
    xCloseBtn.executeAction("CLICK", tuple())

# vim:set shiftwidth=4 softtabstop=4 expandtab: */
