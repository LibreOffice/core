# -*- Mode: python; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest_helper import UITest

from helper import mkPropertyValues

def select_entry_pos(xContext):
    xUITest = xContext.ServiceManager.createInstanceWithContext(
            "org.libreoffice.uitest.UITest", xContext)

    ui_test = UITest(xUITest, xContext)

    ui_test.create_doc_in_start_center("calc")

    ui_test.execute_modeless_dialog_through_command(".uno:AddName")
    xAddNameDlg = xUITest.getTopFocusWindow()

    scopeCB = xAddNameDlg.getChild("scope")
    props = {"POS": "1"}
    actionProps = mkPropertyValues(props)
    scopeCB.executeAction("SELECT", actionProps)

# vim:set shiftwidth=4 softtabstop=4 expandtab: */
