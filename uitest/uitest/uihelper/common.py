# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from libreoffice.uno.propertyvalue import convert_property_values_to_dict, mkPropertyValues
import org.libreoffice.unotest
import pathlib

def get_state_as_dict(ui_object):
    return convert_property_values_to_dict(ui_object.getState())

def type_text(ui_object, text):
    ui_object.executeAction("TYPE", mkPropertyValues({"TEXT": text}))

def select_pos(ui_object, pos):
    ui_object.executeAction("SELECT", mkPropertyValues({"POS": pos}))

def select_text(ui_object, from_pos, to):
    ui_object.executeAction("SELECT", mkPropertyValues({"FROM": from_pos, "TO": to}))

def get_url_for_data_file(file_name):
    return pathlib.Path(org.libreoffice.unotest.makeCopyFromTDOC(file_name)).as_uri()

def change_measurement_unit(UITestCase, unit):
    UITestCase.ui_test.execute_dialog_through_command(".uno:OptionsTreeDialog")
    xDialogOpt = UITestCase.xUITest.getTopFocusWindow()

    xPages = xDialogOpt.getChild("pages")
    xAppEntry = xPages.getChild('3')
    xAppEntry.executeAction("EXPAND", tuple())
    xGeneralEntry = xAppEntry.getChild('0')
    xGeneralEntry.executeAction("SELECT", tuple())

    # Calc
    if 'unitlb' in xDialogOpt.getChildren():
        xUnit = xDialogOpt.getChild("unitlb")

    # Writer
    elif 'metric' in xDialogOpt.getChildren():
        xUnit = xDialogOpt.getChild("metric")

    # Impress
    elif 'units' in xDialogOpt.getChildren():
        xUnit = xDialogOpt.getChild("units")

    props = {"TEXT": unit}
    actionProps = mkPropertyValues(props)
    xUnit.executeAction("SELECT", actionProps)

    # tdf#137930: Check apply button doesn't reset the value
    xApplyBtn = xDialogOpt.getChild("apply")
    xApplyBtn.executeAction("CLICK", tuple())
    UITestCase.assertEqual(unit, get_state_as_dict(xUnit)['SelectEntryText'])

    xOKBtn = xDialogOpt.getChild("ok")
    UITestCase.ui_test.close_dialog_through_button(xOKBtn)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
