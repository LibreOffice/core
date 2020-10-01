# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import type_text

def enter_text_to_cell(gridwin, cell, text):
    gridwin.executeAction("SELECT", mkPropertyValues({"CELL": cell}))
    type_text(gridwin, text)
    gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "RETURN"}))

def change_measurement_units(UITestCase, unit):
    UITestCase.ui_test.execute_dialog_through_command(".uno:OptionsTreeDialog")
    xDialogOpt = UITestCase.xUITest.getTopFocusWindow()

    xPages = xDialogOpt.getChild("pages")
    xEntry = xPages.getChild('3')
    xEntry.executeAction("EXPAND", tuple())
    xGeneralEntry = xEntry.getChild('0')
    xGeneralEntry.executeAction("SELECT", tuple())
    xUnit = xDialogOpt.getChild("unitlb")
    props = {"TEXT": unit}
    actionProps = mkPropertyValues(props)
    xUnit.executeAction("SELECT", actionProps)

    xOKBtn = xDialogOpt.getChild("ok")
    UITestCase.ui_test.close_dialog_through_button(xOKBtn)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
