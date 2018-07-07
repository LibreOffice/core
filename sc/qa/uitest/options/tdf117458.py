# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict
from uitest.uihelper.common import select_pos
from uitest.uihelper.calc import enter_text_to_cell
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues

#Bug 117458 - Selection doesn't move left nor right by pressing ENTER

class tdf117458(UITestCase):
    def test_tdf117458_selection_move_by_enter(self):
        calc_doc = self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        #Go to Tools -> Options -> LibreOffice Calc -> General
        self.ui_test.execute_dialog_through_command(".uno:OptionsTreeDialog")  #optionsdialog
        xDialogOpt = self.xUITest.getTopFocusWindow()

        xPages = xDialogOpt.getChild("pages")
        xCalcEntry = xPages.getChild('3')                 # Calc
        xCalcEntry.executeAction("EXPAND", tuple())
        xCalcGeneralEntry = xCalcEntry.getChild('0')
        xCalcGeneralEntry.executeAction("SELECT", tuple())          #General
        xaligncb = xDialogOpt.getChild("aligncb")
        xalignlb = xDialogOpt.getChild("alignlb")
        if (get_state_as_dict(xaligncb)["Selected"]) == "false":
            xaligncb.executeAction("CLICK", tuple())
        #Down
        props = {"TEXT": "Down"}
        actionProps = mkPropertyValues(props)
        xalignlb.executeAction("SELECT", actionProps)

        xOKBtn = xDialogOpt.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)
        #Select cell A1
        #and down up to "deselect" range
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
        self.xUITest.executeCommand(".uno:GoDown")
        self.xUITest.executeCommand(".uno:GoUp")
        #press Enter
        gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "RETURN"}))
        #Verify
        self.assertEqual(get_state_as_dict(gridwin)["CurrentRow"], "1")

        #Go to Tools -> Options -> LibreOffice Calc -> General
        self.ui_test.execute_dialog_through_command(".uno:OptionsTreeDialog")  #optionsdialog
        xDialogOpt = self.xUITest.getTopFocusWindow()

        xPages = xDialogOpt.getChild("pages")
        xCalcEntry = xPages.getChild('3')                 # Calc
        xCalcEntry.executeAction("EXPAND", tuple())
        xCalcGeneralEntry = xCalcEntry.getChild('0')
        xCalcGeneralEntry.executeAction("SELECT", tuple())          #General
        xaligncb = xDialogOpt.getChild("aligncb")
        xalignlb = xDialogOpt.getChild("alignlb")
        if (get_state_as_dict(xaligncb)["Selected"]) == "false":
            xaligncb.executeAction("CLICK", tuple())
        #Up
        props = {"TEXT": "Up"}
        actionProps = mkPropertyValues(props)
        xalignlb.executeAction("SELECT", actionProps)

        xOKBtn = xDialogOpt.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)
        #Select cell A2
        #and down up to "deselect" range
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A2"}))
        self.xUITest.executeCommand(".uno:GoDown")
        self.xUITest.executeCommand(".uno:GoUp")
        #press Enter
        gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "RETURN"}))
        #Verify
        self.assertEqual(get_state_as_dict(gridwin)["CurrentRow"], "0")

        #Go to Tools -> Options -> LibreOffice Calc -> General
        self.ui_test.execute_dialog_through_command(".uno:OptionsTreeDialog")  #optionsdialog
        xDialogOpt = self.xUITest.getTopFocusWindow()

        xPages = xDialogOpt.getChild("pages")
        xCalcEntry = xPages.getChild('3')                 # Calc
        xCalcEntry.executeAction("EXPAND", tuple())
        xCalcGeneralEntry = xCalcEntry.getChild('0')
        xCalcGeneralEntry.executeAction("SELECT", tuple())          #General
        xaligncb = xDialogOpt.getChild("aligncb")
        xalignlb = xDialogOpt.getChild("alignlb")
        if (get_state_as_dict(xaligncb)["Selected"]) == "false":
            xaligncb.executeAction("CLICK", tuple())
        #Right
        props = {"TEXT": "Right"}
        actionProps = mkPropertyValues(props)
        xalignlb.executeAction("SELECT", actionProps)

        xOKBtn = xDialogOpt.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)
        #Select cell A1
        #and down up to "deselect" range
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
        self.xUITest.executeCommand(".uno:GoDown")
        self.xUITest.executeCommand(".uno:GoUp")
        #press Enter
        gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "RETURN"}))
        #Verify
        self.assertEqual(get_state_as_dict(gridwin)["CurrentColumn"], "1")

        #Go to Tools -> Options -> LibreOffice Calc -> General
        self.ui_test.execute_dialog_through_command(".uno:OptionsTreeDialog")  #optionsdialog
        xDialogOpt = self.xUITest.getTopFocusWindow()

        xPages = xDialogOpt.getChild("pages")
        xCalcEntry = xPages.getChild('3')                 # Calc
        xCalcEntry.executeAction("EXPAND", tuple())
        xCalcGeneralEntry = xCalcEntry.getChild('0')
        xCalcGeneralEntry.executeAction("SELECT", tuple())          #General
        xaligncb = xDialogOpt.getChild("aligncb")
        xalignlb = xDialogOpt.getChild("alignlb")
        if (get_state_as_dict(xaligncb)["Selected"]) == "false":
            xaligncb.executeAction("CLICK", tuple())
        #Left
        props = {"TEXT": "Left"}
        actionProps = mkPropertyValues(props)
        xalignlb.executeAction("SELECT", actionProps)

        xOKBtn = xDialogOpt.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)
        #Select cell B1
        #and down up to "deselect" range
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "B1"}))
        self.xUITest.executeCommand(".uno:GoDown")
        self.xUITest.executeCommand(".uno:GoUp")
        #press Enter
        gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "RETURN"}))
        #Verify
        self.assertEqual(get_state_as_dict(gridwin)["CurrentColumn"], "0")

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab: