# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file
from uitest.uihelper.common import select_pos
from uitest.uihelper.calc import enter_text_to_cell
from libreoffice.calc.document import get_sheet_from_doc
from libreoffice.calc.conditional_format import get_conditional_format_from_sheet
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues

#text to column testcase

class CalcTextToColumns(UITestCase):

    def test_text_to_columns_dot(self):
        #_Dot_as_Separator
        calc_doc = self.ui_test.load_file(get_url_for_data_file("text_to_columns_dot.ods"))
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        #Make sure that tools-options-StarOffice Calc-General-Input settings-Show overwrite warning when pasting data is tagged.
        self.ui_test.execute_dialog_through_command(".uno:OptionsTreeDialog")  #optionsdialog
        xDialogOpt = self.xUITest.getTopFocusWindow()

        xPages = xDialogOpt.getChild("pages")
        xWriterEntry = xPages.getChild('3')                 # Calc
        xWriterEntry.executeAction("EXPAND", tuple())
        xWriterGeneralEntry = xWriterEntry.getChild('0')
        xWriterGeneralEntry.executeAction("SELECT", tuple())          #General / replwarncb
        xreplwarncb = xDialogOpt.getChild("replwarncb")
        print(get_state_as_dict(xreplwarncb)["Selected"])
        if (get_state_as_dict(xreplwarncb)["Selected"]) == "false":
            xreplwarncb.executeAction("CLICK", tuple())
        print(get_state_as_dict(xreplwarncb)["Selected"])
        xOKBtn = xDialogOpt.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        #Select A1:A5 on Sheet 'Dot_as_Separator'
        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:A5"}))
        #Data - Text to Columns
        self.ui_test.execute_dialog_through_command(".uno:TextToColumns")
        xDialog = self.xUITest.getTopFocusWindow()
        #Untag Tab as separator and tag other. Put a dot into the input field next to the other checkbox
        xtab = xDialog.getChild("tab")
        xcomma = xDialog.getChild("comma")
        xsemicolon = xDialog.getChild("semicolon")
        xspace = xDialog.getChild("space")
        xother = xDialog.getChild("other")
        xinputother = xDialog.getChild("inputother")

        if (get_state_as_dict(xother)["Selected"]) == "false":
            xother.executeAction("CLICK", tuple())
        xinputother.executeAction("TYPE", mkPropertyValues({"TEXT":"."}))
        #Click Ok
        #Does an overwrite warning come up? If not file an Issue.
        #Tag the 'Do not show warning again' checkbox and press Ok.
        xOK = xDialog.getChild("ok")
        def handle_warn_dlg(dialog):
            xyesBtn = dialog.getChild("yes")
            xask = dialog.getChild("ask")
            xask.executeAction("CLICK", tuple())
            self.ui_test.close_dialog_through_button(xyesBtn)

        self.ui_test.execute_blocking_action(xOK.executeAction, args=('CLICK', ()),
                dialog_handler=handle_warn_dlg)
        #Verify
        self.assertEqual(get_cell_by_position(document, 0, 0, 0).getValue(), 1)
        self.assertEqual(get_cell_by_position(document, 0, 0, 1).getValue(), 2)
        self.assertEqual(get_cell_by_position(document, 0, 0, 2).getValue(), 3)
        self.assertEqual(get_cell_by_position(document, 0, 0, 3).getValue(), 4)
        self.assertEqual(get_cell_by_position(document, 0, 0, 4).getValue(), 5)
        self.assertEqual(get_cell_by_position(document, 0, 1, 0).getValue(), 2)
        self.assertEqual(get_cell_by_position(document, 0, 1, 1).getValue(), 2)
        self.assertEqual(get_cell_by_position(document, 0, 1, 2).getValue(), 2)
        self.assertEqual(get_cell_by_position(document, 0, 1, 3).getValue(), 2)
        self.assertEqual(get_cell_by_position(document, 0, 1, 4).getValue(), 2)
        self.assertEqual(get_cell_by_position(document, 0, 2, 0).getValue(), 3)
        self.assertEqual(get_cell_by_position(document, 0, 2, 1).getValue(), 3)
        self.assertEqual(get_cell_by_position(document, 0, 2, 2).getValue(), 3)
        self.assertEqual(get_cell_by_position(document, 0, 2, 3).getValue(), 3)
        self.assertEqual(get_cell_by_position(document, 0, 2, 4).getValue(), 3)
        self.assertEqual(get_cell_by_position(document, 0, 3, 0).getValue(), 4)
        self.assertEqual(get_cell_by_position(document, 0, 3, 1).getValue(), 4)
        self.assertEqual(get_cell_by_position(document, 0, 3, 2).getValue(), 4)
        self.assertEqual(get_cell_by_position(document, 0, 3, 3).getValue(), 4)
        self.assertEqual(get_cell_by_position(document, 0, 3, 4).getValue(), 4)
        self.assertEqual(get_cell_by_position(document, 0, 4, 0).getValue(), 5)
        self.assertEqual(get_cell_by_position(document, 0, 4, 1).getValue(), 5)
        self.assertEqual(get_cell_by_position(document, 0, 4, 2).getValue(), 5)
        self.assertEqual(get_cell_by_position(document, 0, 4, 3).getValue(), 5)
        self.assertEqual(get_cell_by_position(document, 0, 4, 4).getValue(), 5)
        self.assertEqual(get_cell_by_position(document, 0, 5, 0).getValue(), 6)
        self.assertEqual(get_cell_by_position(document, 0, 5, 1).getValue(), 6)
        self.assertEqual(get_cell_by_position(document, 0, 5, 2).getValue(), 6)
        self.assertEqual(get_cell_by_position(document, 0, 5, 3).getValue(), 6)
        self.assertEqual(get_cell_by_position(document, 0, 5, 4).getValue(), 6)
        self.assertEqual(get_cell_by_position(document, 0, 6, 0).getString(), "random content")
        self.assertEqual(get_cell_by_position(document, 0, 6, 1).getValue(), 0)
        self.assertEqual(get_cell_by_position(document, 0, 6, 2).getValue(), 0)
        self.assertEqual(get_cell_by_position(document, 0, 6, 3).getValue(), 0)
        self.assertEqual(get_cell_by_position(document, 0, 6, 4).getString(), "random content")
        #verify setting in options
        self.ui_test.execute_dialog_through_command(".uno:OptionsTreeDialog")  #optionsdialog
        xDialogOpt = self.xUITest.getTopFocusWindow()

        xPages = xDialogOpt.getChild("pages")
        xWriterEntry = xPages.getChild('3')                 # Calc
        xWriterEntry.executeAction("EXPAND", tuple())
        xWriterGeneralEntry = xWriterEntry.getChild('0')
        xWriterGeneralEntry.executeAction("SELECT", tuple())          #General / replwarncb
        xreplwarncb = xDialogOpt.getChild("replwarncb")
        self.assertEqual(get_state_as_dict(xreplwarncb)["Selected"], "false")
        xOKBtn = xDialogOpt.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        self.ui_test.close_doc()

    def test_text_to_columns_comma(self):
        #Comma as Separator
        calc_doc = self.ui_test.load_file(get_url_for_data_file("text_to_columns_comma.ods"))
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        #Make sure that tools-options-StarOffice Calc-General-Input settings-Show overwrite warning when pasting data is tagged.
        self.ui_test.execute_dialog_through_command(".uno:OptionsTreeDialog")  #optionsdialog
        xDialogOpt = self.xUITest.getTopFocusWindow()

        xPages = xDialogOpt.getChild("pages")
        xWriterEntry = xPages.getChild('3')                 # Calc
        xWriterEntry.executeAction("EXPAND", tuple())
        xWriterGeneralEntry = xWriterEntry.getChild('0')
        xWriterGeneralEntry.executeAction("SELECT", tuple())          #General / replwarncb
        xreplwarncb = xDialogOpt.getChild("replwarncb")
        print(get_state_as_dict(xreplwarncb)["Selected"])
        if (get_state_as_dict(xreplwarncb)["Selected"]) == "false":
            xreplwarncb.executeAction("CLICK", tuple())
        print(get_state_as_dict(xreplwarncb)["Selected"])
        xOKBtn = xDialogOpt.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        # Select A1:A5
        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:A5"}))
        # Data - Text to Columns
        self.ui_test.execute_dialog_through_command(".uno:TextToColumns")
        xDialog = self.xUITest.getTopFocusWindow()
        # Untag Tab as separator and tag comma.
        xComma = xDialog.getChild("comma")
        if (get_state_as_dict(xComma)["Selected"]) == "false":
            xComma.executeAction("CLICK", tuple())
        # Click Ok
        xOK = xDialog.getChild("ok")
        def handle_warn_dlg(dialog):
            xyesBtn = dialog.getChild("yes")
            self.ui_test.close_dialog_through_button(xyesBtn)

        self.ui_test.execute_blocking_action(xOK.executeAction, args=('CLICK', ()),
                dialog_handler=handle_warn_dlg)
        #Verify
        self.assertEqual(get_cell_by_position(document, 0, 0, 0).getValue(), 1)
        self.assertEqual(get_cell_by_position(document, 0, 0, 1).getValue(), 2)
        self.assertEqual(get_cell_by_position(document, 0, 0, 2).getValue(), 3)
        self.assertEqual(get_cell_by_position(document, 0, 0, 3).getValue(), 4)
        self.assertEqual(get_cell_by_position(document, 0, 0, 4).getValue(), 5)
        self.assertEqual(get_cell_by_position(document, 0, 1, 0).getValue(), 2)
        self.assertEqual(get_cell_by_position(document, 0, 1, 1).getValue(), 2)
        self.assertEqual(get_cell_by_position(document, 0, 1, 2).getValue(), 2)
        self.assertEqual(get_cell_by_position(document, 0, 1, 3).getValue(), 2)
        self.assertEqual(get_cell_by_position(document, 0, 1, 4).getValue(), 2)
        self.assertEqual(get_cell_by_position(document, 0, 2, 0).getValue(), 3)
        self.assertEqual(get_cell_by_position(document, 0, 2, 1).getValue(), 3)
        self.assertEqual(get_cell_by_position(document, 0, 2, 2).getValue(), 3)
        self.assertEqual(get_cell_by_position(document, 0, 2, 3).getValue(), 3)
        self.assertEqual(get_cell_by_position(document, 0, 2, 4).getValue(), 3)
        self.assertEqual(get_cell_by_position(document, 0, 3, 0).getValue(), 4)
        self.assertEqual(get_cell_by_position(document, 0, 3, 1).getValue(), 4)
        self.assertEqual(get_cell_by_position(document, 0, 3, 2).getValue(), 4)
        self.assertEqual(get_cell_by_position(document, 0, 3, 3).getValue(), 4)
        self.assertEqual(get_cell_by_position(document, 0, 3, 4).getValue(), 4)
        self.assertEqual(get_cell_by_position(document, 0, 4, 0).getValue(), 5)
        self.assertEqual(get_cell_by_position(document, 0, 4, 1).getValue(), 5)
        self.assertEqual(get_cell_by_position(document, 0, 4, 2).getValue(), 5)
        self.assertEqual(get_cell_by_position(document, 0, 4, 3).getValue(), 5)
        self.assertEqual(get_cell_by_position(document, 0, 4, 4).getValue(), 5)
        self.assertEqual(get_cell_by_position(document, 0, 5, 0).getValue(), 6)
        self.assertEqual(get_cell_by_position(document, 0, 5, 1).getValue(), 6)
        self.assertEqual(get_cell_by_position(document, 0, 5, 2).getValue(), 6)
        self.assertEqual(get_cell_by_position(document, 0, 5, 3).getValue(), 6)
        self.assertEqual(get_cell_by_position(document, 0, 5, 4).getValue(), 6)
        self.assertEqual(get_cell_by_position(document, 0, 6, 0).getString(), "random content")
        self.assertEqual(get_cell_by_position(document, 0, 6, 1).getValue(), 0)
        self.assertEqual(get_cell_by_position(document, 0, 6, 2).getValue(), 0)
        self.assertEqual(get_cell_by_position(document, 0, 6, 3).getValue(), 0)
        self.assertEqual(get_cell_by_position(document, 0, 6, 4).getString(), "random content")

        self.ui_test.close_doc()
    def test_text_to_columns_semicolon(self):
        #Semicolon  as Separator
        calc_doc = self.ui_test.load_file(get_url_for_data_file("text_to_columns_semicolon.ods"))
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        #Make sure that tools-options-StarOffice Calc-General-Input settings-Show overwrite warning when pasting data is tagged.
        self.ui_test.execute_dialog_through_command(".uno:OptionsTreeDialog")  #optionsdialog
        xDialogOpt = self.xUITest.getTopFocusWindow()

        xPages = xDialogOpt.getChild("pages")
        xWriterEntry = xPages.getChild('3')                 # Calc
        xWriterEntry.executeAction("EXPAND", tuple())
        xWriterGeneralEntry = xWriterEntry.getChild('0')
        xWriterGeneralEntry.executeAction("SELECT", tuple())          #General / replwarncb
        xreplwarncb = xDialogOpt.getChild("replwarncb")
        if (get_state_as_dict(xreplwarncb)["Selected"]) == "false":
            xreplwarncb.executeAction("CLICK", tuple())
        xOKBtn = xDialogOpt.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        # Select A1:A5 on Sheet
        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:A5"}))
        # Data - Text to Columns
        self.ui_test.execute_dialog_through_command(".uno:TextToColumns")
        xDialog = self.xUITest.getTopFocusWindow()
        # Untag comma as separator and tag Semicolon
        xSemicolon = xDialog.getChild("semicolon")
        if (get_state_as_dict(xSemicolon)["Selected"]) == "false":
            xSemicolon.executeAction("CLICK", tuple())
        # Click Ok
        xOK = xDialog.getChild("ok")
        def handle_warn_dlg(dialog):
            xyesBtn = dialog.getChild("yes")
            self.ui_test.close_dialog_through_button(xyesBtn)

        self.ui_test.execute_blocking_action(xOK.executeAction, args=('CLICK', ()),
                dialog_handler=handle_warn_dlg)
        #Verify
        self.assertEqual(get_cell_by_position(document, 0, 0, 0).getValue(), 1)
        self.assertEqual(get_cell_by_position(document, 0, 0, 1).getValue(), 2)
        self.assertEqual(get_cell_by_position(document, 0, 0, 2).getValue(), 3)
        self.assertEqual(get_cell_by_position(document, 0, 0, 3).getValue(), 4)
        self.assertEqual(get_cell_by_position(document, 0, 0, 4).getValue(), 5)
        self.assertEqual(get_cell_by_position(document, 0, 1, 0).getValue(), 2)
        self.assertEqual(get_cell_by_position(document, 0, 1, 1).getValue(), 2)
        self.assertEqual(get_cell_by_position(document, 0, 1, 2).getValue(), 2)
        self.assertEqual(get_cell_by_position(document, 0, 1, 3).getValue(), 2)
        self.assertEqual(get_cell_by_position(document, 0, 1, 4).getValue(), 2)
        self.assertEqual(get_cell_by_position(document, 0, 2, 0).getValue(), 3)
        self.assertEqual(get_cell_by_position(document, 0, 2, 1).getValue(), 3)
        self.assertEqual(get_cell_by_position(document, 0, 2, 2).getValue(), 3)
        self.assertEqual(get_cell_by_position(document, 0, 2, 3).getValue(), 3)
        self.assertEqual(get_cell_by_position(document, 0, 2, 4).getValue(), 3)
        self.assertEqual(get_cell_by_position(document, 0, 3, 0).getValue(), 4)
        self.assertEqual(get_cell_by_position(document, 0, 3, 1).getValue(), 4)
        self.assertEqual(get_cell_by_position(document, 0, 3, 2).getValue(), 4)
        self.assertEqual(get_cell_by_position(document, 0, 3, 3).getValue(), 4)
        self.assertEqual(get_cell_by_position(document, 0, 3, 4).getValue(), 4)
        self.assertEqual(get_cell_by_position(document, 0, 4, 0).getValue(), 5)
        self.assertEqual(get_cell_by_position(document, 0, 4, 1).getValue(), 5)
        self.assertEqual(get_cell_by_position(document, 0, 4, 2).getValue(), 5)
        self.assertEqual(get_cell_by_position(document, 0, 4, 3).getValue(), 5)
        self.assertEqual(get_cell_by_position(document, 0, 4, 4).getValue(), 5)
        self.assertEqual(get_cell_by_position(document, 0, 5, 0).getValue(), 6)
        self.assertEqual(get_cell_by_position(document, 0, 5, 1).getValue(), 6)
        self.assertEqual(get_cell_by_position(document, 0, 5, 2).getValue(), 6)
        self.assertEqual(get_cell_by_position(document, 0, 5, 3).getValue(), 6)
        self.assertEqual(get_cell_by_position(document, 0, 5, 4).getValue(), 6)
        self.assertEqual(get_cell_by_position(document, 0, 6, 0).getString(), "random content")
        self.assertEqual(get_cell_by_position(document, 0, 6, 1).getValue(), 0)
        self.assertEqual(get_cell_by_position(document, 0, 6, 2).getValue(), 0)
        self.assertEqual(get_cell_by_position(document, 0, 6, 3).getValue(), 0)
        self.assertEqual(get_cell_by_position(document, 0, 6, 4).getString(), "random content")
        self.ui_test.close_doc()

    def test_text_to_columns_space(self):
        #Space as Separator
        calc_doc = self.ui_test.load_file(get_url_for_data_file("text_to_columns_space.ods"))
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        #Make sure that tools-options-StarOffice Calc-General-Input settings-Show overwrite warning when pasting data is tagged.
        self.ui_test.execute_dialog_through_command(".uno:OptionsTreeDialog")  #optionsdialog
        xDialogOpt = self.xUITest.getTopFocusWindow()

        xPages = xDialogOpt.getChild("pages")
        xWriterEntry = xPages.getChild('3')                 # Calc
        xWriterEntry.executeAction("EXPAND", tuple())
        xWriterGeneralEntry = xWriterEntry.getChild('0')
        xWriterGeneralEntry.executeAction("SELECT", tuple())          #General / replwarncb
        xreplwarncb = xDialogOpt.getChild("replwarncb")
        if (get_state_as_dict(xreplwarncb)["Selected"]) == "false":
            xreplwarncb.executeAction("CLICK", tuple())
        xOKBtn = xDialogOpt.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        # Select A1:A5 on Sheet
        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:A5"}))
        # Data - Text to Columns
        self.ui_test.execute_dialog_through_command(".uno:TextToColumns")
        xDialog = self.xUITest.getTopFocusWindow()
        # Untag comma as separator and tag Semicolon
        xSpace = xDialog.getChild("space")
        if (get_state_as_dict(xSpace)["Selected"]) == "false":
            xSpace.executeAction("CLICK", tuple())
        # xspace.executeAction("CLICK", tuple())
        # Click Ok
        xOK = xDialog.getChild("ok")
        def handle_warn_dlg(dialog):
            xyesBtn = dialog.getChild("yes")
            self.ui_test.close_dialog_through_button(xyesBtn)

        self.ui_test.execute_blocking_action(xOK.executeAction, args=('CLICK', ()),
                dialog_handler=handle_warn_dlg)
        #Verify
        self.assertEqual(get_cell_by_position(document, 0, 0, 0).getValue(), 1)
        self.assertEqual(get_cell_by_position(document, 0, 0, 1).getValue(), 2)
        self.assertEqual(get_cell_by_position(document, 0, 0, 2).getValue(), 3)
        self.assertEqual(get_cell_by_position(document, 0, 0, 3).getValue(), 4)
        self.assertEqual(get_cell_by_position(document, 0, 0, 4).getValue(), 5)
        self.assertEqual(get_cell_by_position(document, 0, 1, 0).getValue(), 2)
        self.assertEqual(get_cell_by_position(document, 0, 1, 1).getValue(), 2)
        self.assertEqual(get_cell_by_position(document, 0, 1, 2).getValue(), 2)
        self.assertEqual(get_cell_by_position(document, 0, 1, 3).getValue(), 2)
        self.assertEqual(get_cell_by_position(document, 0, 1, 4).getValue(), 2)
        self.assertEqual(get_cell_by_position(document, 0, 2, 0).getValue(), 3)
        self.assertEqual(get_cell_by_position(document, 0, 2, 1).getValue(), 3)
        self.assertEqual(get_cell_by_position(document, 0, 2, 2).getValue(), 3)
        self.assertEqual(get_cell_by_position(document, 0, 2, 3).getValue(), 3)
        self.assertEqual(get_cell_by_position(document, 0, 2, 4).getValue(), 3)
        self.assertEqual(get_cell_by_position(document, 0, 3, 0).getValue(), 4)
        self.assertEqual(get_cell_by_position(document, 0, 3, 1).getValue(), 4)
        self.assertEqual(get_cell_by_position(document, 0, 3, 2).getValue(), 4)
        self.assertEqual(get_cell_by_position(document, 0, 3, 3).getValue(), 4)
        self.assertEqual(get_cell_by_position(document, 0, 3, 4).getValue(), 4)
        self.assertEqual(get_cell_by_position(document, 0, 4, 0).getValue(), 5)
        self.assertEqual(get_cell_by_position(document, 0, 4, 1).getValue(), 5)
        self.assertEqual(get_cell_by_position(document, 0, 4, 2).getValue(), 5)
        self.assertEqual(get_cell_by_position(document, 0, 4, 3).getValue(), 5)
        self.assertEqual(get_cell_by_position(document, 0, 4, 4).getValue(), 5)
        self.assertEqual(get_cell_by_position(document, 0, 5, 0).getValue(), 6)
        self.assertEqual(get_cell_by_position(document, 0, 5, 1).getValue(), 6)
        self.assertEqual(get_cell_by_position(document, 0, 5, 2).getValue(), 6)
        self.assertEqual(get_cell_by_position(document, 0, 5, 3).getValue(), 6)
        self.assertEqual(get_cell_by_position(document, 0, 5, 4).getValue(), 6)
        self.assertEqual(get_cell_by_position(document, 0, 6, 0).getString(), "random content")
        self.assertEqual(get_cell_by_position(document, 0, 6, 1).getValue(), 0)
        self.assertEqual(get_cell_by_position(document, 0, 6, 2).getValue(), 0)
        self.assertEqual(get_cell_by_position(document, 0, 6, 3).getValue(), 0)
        self.assertEqual(get_cell_by_position(document, 0, 6, 4).getString(), "random content")
        self.ui_test.close_doc()

    def test_text_to_columns_pipe(self):
        #Space as Separator
        calc_doc = self.ui_test.load_file(get_url_for_data_file("text_to_columns_pipe.ods"))
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        #Make sure that tools-options-StarOffice Calc-General-Input settings-Show overwrite warning when pasting data is tagged.
        self.ui_test.execute_dialog_through_command(".uno:OptionsTreeDialog")  #optionsdialog
        xDialogOpt = self.xUITest.getTopFocusWindow()

        xPages = xDialogOpt.getChild("pages")
        xWriterEntry = xPages.getChild('3')                 # Calc
        xWriterEntry.executeAction("EXPAND", tuple())
        xWriterGeneralEntry = xWriterEntry.getChild('0')
        xWriterGeneralEntry.executeAction("SELECT", tuple())          #General / replwarncb
        xreplwarncb = xDialogOpt.getChild("replwarncb")
        if (get_state_as_dict(xreplwarncb)["Selected"]) == "false":
            xreplwarncb.executeAction("CLICK", tuple())
        xOKBtn = xDialogOpt.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        # Select A1:A5 on Sheet
        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:A5"}))
        # Data - Text to Columns
        self.ui_test.execute_dialog_through_command(".uno:TextToColumns")
        xDialog = self.xUITest.getTopFocusWindow()
        # Untag comma as separator and tag Semicolon
        xtab = xDialog.getChild("tab")
        xcomma = xDialog.getChild("comma")
        xsemicolon = xDialog.getChild("semicolon")
        xspace = xDialog.getChild("space")
        xother = xDialog.getChild("other")
        xinputother = xDialog.getChild("inputother")
        xSpace = xDialog.getChild("space")
        if (get_state_as_dict(xother)["Selected"]) == "false":
            xother.executeAction("CLICK", tuple())
        xinputother.executeAction("TYPE", mkPropertyValues({"TEXT":"|"}))
        # Click Ok
        xOK = xDialog.getChild("ok")
        def handle_warn_dlg(dialog):
            xyesBtn = dialog.getChild("yes")
            self.ui_test.close_dialog_through_button(xyesBtn)

        self.ui_test.execute_blocking_action(xOK.executeAction, args=('CLICK', ()),
                dialog_handler=handle_warn_dlg)
        #Verify
        self.assertEqual(get_cell_by_position(document, 0, 0, 0).getValue(), 1)
        self.assertEqual(get_cell_by_position(document, 0, 0, 1).getValue(), 2)
        self.assertEqual(get_cell_by_position(document, 0, 0, 2).getValue(), 3)
        self.assertEqual(get_cell_by_position(document, 0, 0, 3).getValue(), 4)
        self.assertEqual(get_cell_by_position(document, 0, 0, 4).getValue(), 5)
        self.assertEqual(get_cell_by_position(document, 0, 1, 0).getValue(), 2)
        self.assertEqual(get_cell_by_position(document, 0, 1, 1).getValue(), 2)
        self.assertEqual(get_cell_by_position(document, 0, 1, 2).getValue(), 2)
        self.assertEqual(get_cell_by_position(document, 0, 1, 3).getValue(), 2)
        self.assertEqual(get_cell_by_position(document, 0, 1, 4).getValue(), 2)
        self.assertEqual(get_cell_by_position(document, 0, 2, 0).getValue(), 3)
        self.assertEqual(get_cell_by_position(document, 0, 2, 1).getValue(), 3)
        self.assertEqual(get_cell_by_position(document, 0, 2, 2).getValue(), 3)
        self.assertEqual(get_cell_by_position(document, 0, 2, 3).getValue(), 3)
        self.assertEqual(get_cell_by_position(document, 0, 2, 4).getValue(), 3)
        self.assertEqual(get_cell_by_position(document, 0, 3, 0).getValue(), 4)
        self.assertEqual(get_cell_by_position(document, 0, 3, 1).getValue(), 4)
        self.assertEqual(get_cell_by_position(document, 0, 3, 2).getValue(), 4)
        self.assertEqual(get_cell_by_position(document, 0, 3, 3).getValue(), 4)
        self.assertEqual(get_cell_by_position(document, 0, 3, 4).getValue(), 4)
        self.assertEqual(get_cell_by_position(document, 0, 4, 0).getValue(), 5)
        self.assertEqual(get_cell_by_position(document, 0, 4, 1).getValue(), 5)
        self.assertEqual(get_cell_by_position(document, 0, 4, 2).getValue(), 5)
        self.assertEqual(get_cell_by_position(document, 0, 4, 3).getValue(), 5)
        self.assertEqual(get_cell_by_position(document, 0, 4, 4).getValue(), 5)
        self.assertEqual(get_cell_by_position(document, 0, 5, 0).getValue(), 6)
        self.assertEqual(get_cell_by_position(document, 0, 5, 1).getValue(), 6)
        self.assertEqual(get_cell_by_position(document, 0, 5, 2).getValue(), 6)
        self.assertEqual(get_cell_by_position(document, 0, 5, 3).getValue(), 6)
        self.assertEqual(get_cell_by_position(document, 0, 5, 4).getValue(), 6)
        self.assertEqual(get_cell_by_position(document, 0, 6, 0).getString(), "random content")
        self.assertEqual(get_cell_by_position(document, 0, 6, 1).getValue(), 0)
        self.assertEqual(get_cell_by_position(document, 0, 6, 2).getValue(), 0)
        self.assertEqual(get_cell_by_position(document, 0, 6, 3).getValue(), 0)
        self.assertEqual(get_cell_by_position(document, 0, 6, 4).getString(), "random content")
        self.ui_test.close_doc()

    def test_text_to_columns_pipespace(self):
        #Space as Separator
        calc_doc = self.ui_test.load_file(get_url_for_data_file("text_to_columns_pipe_space.ods"))
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        #Make sure that tools-options-StarOffice Calc-General-Input settings-Show overwrite warning when pasting data is tagged.
        self.ui_test.execute_dialog_through_command(".uno:OptionsTreeDialog")  #optionsdialog
        xDialogOpt = self.xUITest.getTopFocusWindow()

        xPages = xDialogOpt.getChild("pages")
        xWriterEntry = xPages.getChild('3')                 # Calc
        xWriterEntry.executeAction("EXPAND", tuple())
        xWriterGeneralEntry = xWriterEntry.getChild('0')
        xWriterGeneralEntry.executeAction("SELECT", tuple())          #General / replwarncb
        xreplwarncb = xDialogOpt.getChild("replwarncb")
        if (get_state_as_dict(xreplwarncb)["Selected"]) == "false":
            xreplwarncb.executeAction("CLICK", tuple())
        xOKBtn = xDialogOpt.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        # Select A1:A5 on Sheet
        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:A5"}))
        # Data - Text to Columns
        self.ui_test.execute_dialog_through_command(".uno:TextToColumns")
        xDialog = self.xUITest.getTopFocusWindow()
        #
        xtab = xDialog.getChild("tab")
        xcomma = xDialog.getChild("comma")
        xsemicolon = xDialog.getChild("semicolon")
        xspace = xDialog.getChild("space")
        xother = xDialog.getChild("other")
        xinputother = xDialog.getChild("inputother")
        if (get_state_as_dict(xspace)["Selected"]) == "false":
            xspace.executeAction("CLICK", tuple())
        if (get_state_as_dict(xother)["Selected"]) == "false":
            xother.executeAction("CLICK", tuple())
        xinputother.executeAction("TYPE", mkPropertyValues({"TEXT":"|"}))
        # Click Ok
        xOK = xDialog.getChild("ok")
        def handle_warn_dlg(dialog):
            xyesBtn = dialog.getChild("yes")
            self.ui_test.close_dialog_through_button(xyesBtn)

        self.ui_test.execute_blocking_action(xOK.executeAction, args=('CLICK', ()),
                dialog_handler=handle_warn_dlg)
        #Verify
        self.assertEqual(get_cell_by_position(document, 0, 0, 0).getValue(), 1)
        self.assertEqual(get_cell_by_position(document, 0, 0, 1).getValue(), 2)
        self.assertEqual(get_cell_by_position(document, 0, 0, 2).getValue(), 3)
        self.assertEqual(get_cell_by_position(document, 0, 0, 3).getValue(), 4)
        self.assertEqual(get_cell_by_position(document, 0, 0, 4).getValue(), 5)
        self.assertEqual(get_cell_by_position(document, 0, 1, 0).getValue(), 2)
        self.assertEqual(get_cell_by_position(document, 0, 1, 1).getValue(), 2)
        self.assertEqual(get_cell_by_position(document, 0, 1, 2).getValue(), 2)
        self.assertEqual(get_cell_by_position(document, 0, 1, 3).getValue(), 2)
        self.assertEqual(get_cell_by_position(document, 0, 1, 4).getValue(), 2)
        self.assertEqual(get_cell_by_position(document, 0, 2, 0).getValue(), 3)
        self.assertEqual(get_cell_by_position(document, 0, 2, 1).getValue(), 3)
        self.assertEqual(get_cell_by_position(document, 0, 2, 2).getValue(), 3)
        self.assertEqual(get_cell_by_position(document, 0, 2, 3).getValue(), 3)
        self.assertEqual(get_cell_by_position(document, 0, 2, 4).getValue(), 3)
        self.assertEqual(get_cell_by_position(document, 0, 3, 0).getValue(), 4)
        self.assertEqual(get_cell_by_position(document, 0, 3, 1).getValue(), 4)
        self.assertEqual(get_cell_by_position(document, 0, 3, 2).getValue(), 4)
        self.assertEqual(get_cell_by_position(document, 0, 3, 3).getValue(), 4)
        self.assertEqual(get_cell_by_position(document, 0, 3, 4).getValue(), 4)
        self.assertEqual(get_cell_by_position(document, 0, 4, 0).getValue(), 5)
        self.assertEqual(get_cell_by_position(document, 0, 4, 1).getValue(), 5)
        self.assertEqual(get_cell_by_position(document, 0, 4, 2).getValue(), 5)
        self.assertEqual(get_cell_by_position(document, 0, 4, 3).getValue(), 5)
        self.assertEqual(get_cell_by_position(document, 0, 4, 4).getValue(), 5)
        self.assertEqual(get_cell_by_position(document, 0, 5, 0).getValue(), 6)
        self.assertEqual(get_cell_by_position(document, 0, 5, 1).getValue(), 6)
        self.assertEqual(get_cell_by_position(document, 0, 5, 2).getValue(), 6)
        self.assertEqual(get_cell_by_position(document, 0, 5, 3).getValue(), 6)
        self.assertEqual(get_cell_by_position(document, 0, 5, 4).getValue(), 6)
        self.assertEqual(get_cell_by_position(document, 0, 6, 0).getString(), "random content")
        self.assertEqual(get_cell_by_position(document, 0, 6, 1).getValue(), 0)
        self.assertEqual(get_cell_by_position(document, 0, 6, 2).getValue(), 0)
        self.assertEqual(get_cell_by_position(document, 0, 6, 3).getValue(), 0)
        self.assertEqual(get_cell_by_position(document, 0, 6, 4).getString(), "random content")
        self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab: