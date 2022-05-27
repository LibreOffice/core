# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues

#text to column testcase

class CalcTextToColumns(UITestCase):

    def test_text_to_columns_dot(self):
        #_Dot_as_Separator
        with self.ui_test.load_file(get_url_for_data_file("text_to_columns_dot.ods")) as calc_doc:
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")
            #Make sure that tools-options-StarOffice Calc-General-Input settings-Show overwrite warning when pasting data is tagged.
            with self.ui_test.execute_dialog_through_command(".uno:OptionsTreeDialog") as xDialogOpt:

                xPages = xDialogOpt.getChild("pages")
                xWriterEntry = xPages.getChild('3')                 # Calc
                xWriterEntry.executeAction("EXPAND", tuple())
                xWriterGeneralEntry = xWriterEntry.getChild('0')
                xWriterGeneralEntry.executeAction("SELECT", tuple())          #General / replwarncb
                xreplwarncb = xDialogOpt.getChild("replwarncb")
                if (get_state_as_dict(xreplwarncb)["Selected"]) == "false":
                    xreplwarncb.executeAction("CLICK", tuple())

            #Select A1:A5 on Sheet 'Dot_as_Separator'
            gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:A5"}))
            #Data - Text to Columns
            with self.ui_test.execute_dialog_through_command(".uno:TextToColumns", close_button="") as xDialog:
                #Untag Tab as separator and tag other. Put a dot into the input field next to the other checkbox
                xother = xDialog.getChild("other")
                xinputother = xDialog.getChild("inputother")

                if (get_state_as_dict(xother)["Selected"]) == "false":
                    xother.executeAction("CLICK", tuple())
                xinputother.executeAction("TYPE", mkPropertyValues({"TEXT":"."}))
                #Click Ok
                #Does an overwrite warning come up? If not file an Issue.
                #Tag the 'Do not show warning again' checkbox and press Ok.
                xOK = xDialog.getChild("ok")
                with self.ui_test.execute_blocking_action(xOK.executeAction, args=('CLICK', ()), close_button="yes") as dialog:
                    xask = dialog.getChild("ask")
                    xask.executeAction("CLICK", tuple())

            #Verify
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 0).getValue(), 1)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 1).getValue(), 2)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 2).getValue(), 3)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 3).getValue(), 4)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 4).getValue(), 5)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 1, 0).getValue(), 2)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 1, 1).getValue(), 2)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 1, 2).getValue(), 2)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 1, 3).getValue(), 2)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 1, 4).getValue(), 2)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 2, 0).getValue(), 3)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 2, 1).getValue(), 3)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 2, 2).getValue(), 3)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 2, 3).getValue(), 3)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 2, 4).getValue(), 3)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 3, 0).getValue(), 4)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 3, 1).getValue(), 4)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 3, 2).getValue(), 4)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 3, 3).getValue(), 4)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 3, 4).getValue(), 4)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 4, 0).getValue(), 5)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 4, 1).getValue(), 5)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 4, 2).getValue(), 5)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 4, 3).getValue(), 5)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 4, 4).getValue(), 5)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 5, 0).getValue(), 6)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 5, 1).getValue(), 6)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 5, 2).getValue(), 6)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 5, 3).getValue(), 6)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 5, 4).getValue(), 6)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 6, 0).getString(), "random content")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 6, 1).getValue(), 0)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 6, 2).getValue(), 0)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 6, 3).getValue(), 0)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 6, 4).getString(), "random content")

            #verify setting in options
            with self.ui_test.execute_dialog_through_command(".uno:OptionsTreeDialog") as xDialogOpt:

                xPages = xDialogOpt.getChild("pages")
                xWriterEntry = xPages.getChild('3')                 # Calc
                xWriterEntry.executeAction("EXPAND", tuple())
                xWriterGeneralEntry = xWriterEntry.getChild('0')
                xWriterGeneralEntry.executeAction("SELECT", tuple())          #General / replwarncb
                xreplwarncb = xDialogOpt.getChild("replwarncb")
                self.assertEqual(get_state_as_dict(xreplwarncb)["Selected"], "false")

    def test_text_to_columns_comma(self):
        #Comma as Separator
        with self.ui_test.load_file(get_url_for_data_file("text_to_columns_comma.ods")) as calc_doc:
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")
            #Make sure that tools-options-StarOffice Calc-General-Input settings-Show overwrite warning when pasting data is tagged.
            with self.ui_test.execute_dialog_through_command(".uno:OptionsTreeDialog") as xDialogOpt:

                xPages = xDialogOpt.getChild("pages")
                xWriterEntry = xPages.getChild('3')                 # Calc
                xWriterEntry.executeAction("EXPAND", tuple())
                xWriterGeneralEntry = xWriterEntry.getChild('0')
                xWriterGeneralEntry.executeAction("SELECT", tuple())          #General / replwarncb
                xreplwarncb = xDialogOpt.getChild("replwarncb")
                if (get_state_as_dict(xreplwarncb)["Selected"]) == "false":
                    xreplwarncb.executeAction("CLICK", tuple())

            # Select A1:A5
            gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:A5"}))
            # Data - Text to Columns
            with self.ui_test.execute_dialog_through_command(".uno:TextToColumns", close_button="") as xDialog:
                # Untag Tab as separator and tag comma.
                xComma = xDialog.getChild("comma")
                if (get_state_as_dict(xComma)["Selected"]) == "false":
                    xComma.executeAction("CLICK", tuple())
                # Click Ok

                xOK = xDialog.getChild("ok")
                with self.ui_test.execute_blocking_action(xOK.executeAction, args=('CLICK', ()), close_button="yes"):
                    pass

            #Verify
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 0).getValue(), 1)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 1).getValue(), 2)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 2).getValue(), 3)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 3).getValue(), 4)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 4).getValue(), 5)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 1, 0).getValue(), 2)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 1, 1).getValue(), 2)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 1, 2).getValue(), 2)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 1, 3).getValue(), 2)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 1, 4).getValue(), 2)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 2, 0).getValue(), 3)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 2, 1).getValue(), 3)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 2, 2).getValue(), 3)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 2, 3).getValue(), 3)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 2, 4).getValue(), 3)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 3, 0).getValue(), 4)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 3, 1).getValue(), 4)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 3, 2).getValue(), 4)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 3, 3).getValue(), 4)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 3, 4).getValue(), 4)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 4, 0).getValue(), 5)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 4, 1).getValue(), 5)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 4, 2).getValue(), 5)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 4, 3).getValue(), 5)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 4, 4).getValue(), 5)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 5, 0).getValue(), 6)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 5, 1).getValue(), 6)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 5, 2).getValue(), 6)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 5, 3).getValue(), 6)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 5, 4).getValue(), 6)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 6, 0).getString(), "random content")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 6, 1).getValue(), 0)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 6, 2).getValue(), 0)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 6, 3).getValue(), 0)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 6, 4).getString(), "random content")

        def test_text_to_columns_semicolon(self):
            #Semicolon  as Separator
            with self.ui_test.load_file(get_url_for_data_file("text_to_columns_semicolon.ods")) as calc_doc:
                gridwin = xCalcDoc.getChild("grid_window")
                #Make sure that tools-options-StarOffice Calc-General-Input settings-Show overwrite warning when pasting data is tagged.
            with self.ui_test.execute_dialog_through_command(".uno:OptionsTreeDialog") as xDialogOpt:

                xPages = xDialogOpt.getChild("pages")
                xWriterEntry = xPages.getChild('3')                 # Calc
                xWriterEntry.executeAction("EXPAND", tuple())
                xWriterGeneralEntry = xWriterEntry.getChild('0')
                xWriterGeneralEntry.executeAction("SELECT", tuple())          #General / replwarncb
                xreplwarncb = xDialogOpt.getChild("replwarncb")
                if (get_state_as_dict(xreplwarncb)["Selected"]) == "false":
                    xreplwarncb.executeAction("CLICK", tuple())

            # Select A1:A5 on Sheet
            gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:A5"}))
            # Data - Text to Columns
            with self.ui_test.execute_dialog_through_command(".uno:TextToColumns", close_button="") as xDialog:
                # Untag comma as separator and tag Semicolon
                xSemicolon = xDialog.getChild("semicolon")
                if (get_state_as_dict(xSemicolon)["Selected"]) == "false":
                    xSemicolon.executeAction("CLICK", tuple())
                # Click Ok

                xOK = xDialog.getChild("ok")
                with self.ui_test.execute_blocking_action(xOK.executeAction, args=('CLICK', ()), close_button="yes"):
                    pass

            #Verify
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 0).getValue(), 1)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 1).getValue(), 2)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 2).getValue(), 3)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 3).getValue(), 4)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 4).getValue(), 5)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 1, 0).getValue(), 2)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 1, 1).getValue(), 2)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 1, 2).getValue(), 2)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 1, 3).getValue(), 2)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 1, 4).getValue(), 2)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 2, 0).getValue(), 3)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 2, 1).getValue(), 3)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 2, 2).getValue(), 3)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 2, 3).getValue(), 3)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 2, 4).getValue(), 3)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 3, 0).getValue(), 4)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 3, 1).getValue(), 4)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 3, 2).getValue(), 4)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 3, 3).getValue(), 4)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 3, 4).getValue(), 4)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 4, 0).getValue(), 5)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 4, 1).getValue(), 5)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 4, 2).getValue(), 5)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 4, 3).getValue(), 5)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 4, 4).getValue(), 5)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 5, 0).getValue(), 6)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 5, 1).getValue(), 6)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 5, 2).getValue(), 6)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 5, 3).getValue(), 6)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 5, 4).getValue(), 6)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 6, 0).getString(), "random content")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 6, 1).getValue(), 0)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 6, 2).getValue(), 0)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 6, 3).getValue(), 0)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 6, 4).getString(), "random content")

        def test_text_to_columns_space(self):
            #Space as Separator
            with self.ui_test.load_file(get_url_for_data_file("text_to_columns_space.ods")) as calc_doc:
                gridwin = xCalcDoc.getChild("grid_window")
                #Make sure that tools-options-StarOffice Calc-General-Input settings-Show overwrite warning when pasting data is tagged.
            with self.ui_test.execute_dialog_through_command(".uno:OptionsTreeDialog") as xDialogOpt:

                xPages = xDialogOpt.getChild("pages")
                xWriterEntry = xPages.getChild('3')                 # Calc
                xWriterEntry.executeAction("EXPAND", tuple())
                xWriterGeneralEntry = xWriterEntry.getChild('0')
                xWriterGeneralEntry.executeAction("SELECT", tuple())          #General / replwarncb
                xreplwarncb = xDialogOpt.getChild("replwarncb")
                if (get_state_as_dict(xreplwarncb)["Selected"]) == "false":
                    xreplwarncb.executeAction("CLICK", tuple())

            # Select A1:A5 on Sheet
            gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:A5"}))
            # Data - Text to Columns
            with self.ui_test.execute_dialog_through_command(".uno:TextToColumns", close_button="") as xDialog:
                # Untag comma as separator and tag Semicolon
                xSpace = xDialog.getChild("space")
                if (get_state_as_dict(xSpace)["Selected"]) == "false":
                    xSpace.executeAction("CLICK", tuple())
                # xspace.executeAction("CLICK", tuple())
                # Click Ok

                xOK = xDialog.getChild("ok")
                with self.ui_test.execute_blocking_action(xOK.executeAction, args=('CLICK', ()), close_button="yes"):
                    pass

            #Verify
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 0).getValue(), 1)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 1).getValue(), 2)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 2).getValue(), 3)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 3).getValue(), 4)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 4).getValue(), 5)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 1, 0).getValue(), 2)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 1, 1).getValue(), 2)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 1, 2).getValue(), 2)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 1, 3).getValue(), 2)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 1, 4).getValue(), 2)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 2, 0).getValue(), 3)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 2, 1).getValue(), 3)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 2, 2).getValue(), 3)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 2, 3).getValue(), 3)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 2, 4).getValue(), 3)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 3, 0).getValue(), 4)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 3, 1).getValue(), 4)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 3, 2).getValue(), 4)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 3, 3).getValue(), 4)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 3, 4).getValue(), 4)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 4, 0).getValue(), 5)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 4, 1).getValue(), 5)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 4, 2).getValue(), 5)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 4, 3).getValue(), 5)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 4, 4).getValue(), 5)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 5, 0).getValue(), 6)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 5, 1).getValue(), 6)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 5, 2).getValue(), 6)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 5, 3).getValue(), 6)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 5, 4).getValue(), 6)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 6, 0).getString(), "random content")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 6, 1).getValue(), 0)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 6, 2).getValue(), 0)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 6, 3).getValue(), 0)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 6, 4).getString(), "random content")

        def test_text_to_columns_pipe(self):
            #Space as Separator
            with self.ui_test.load_file(get_url_for_data_file("text_to_columns_pipe.ods")) as calc_doc:
                gridwin = xCalcDoc.getChild("grid_window")
                #Make sure that tools-options-StarOffice Calc-General-Input settings-Show overwrite warning when pasting data is tagged.
            with self.ui_test.execute_dialog_through_command(".uno:OptionsTreeDialog") as xDialogOpt:

                xPages = xDialogOpt.getChild("pages")
                xWriterEntry = xPages.getChild('3')                 # Calc
                xWriterEntry.executeAction("EXPAND", tuple())
                xWriterGeneralEntry = xWriterEntry.getChild('0')
                xWriterGeneralEntry.executeAction("SELECT", tuple())          #General / replwarncb
                xreplwarncb = xDialogOpt.getChild("replwarncb")
                if (get_state_as_dict(xreplwarncb)["Selected"]) == "false":
                    xreplwarncb.executeAction("CLICK", tuple())

            # Select A1:A5 on Sheet
            gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:A5"}))
            # Data - Text to Columns
            with self.ui_test.execute_dialog_through_command(".uno:TextToColumns", close_button="") as xDialog:
                # Untag comma as separator and tag Semicolon
                xother = xDialog.getChild("other")
                xinputother = xDialog.getChild("inputother")
                if (get_state_as_dict(xother)["Selected"]) == "false":
                    xother.executeAction("CLICK", tuple())
                xinputother.executeAction("TYPE", mkPropertyValues({"TEXT":"|"}))
                # Click Ok

                xOK = xDialog.getChild("ok")
                with self.ui_test.execute_blocking_action(xOK.executeAction, args=('CLICK', ()), close_button="yes"):
                    pass

            #Verify
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 0).getValue(), 1)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 1).getValue(), 2)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 2).getValue(), 3)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 3).getValue(), 4)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 4).getValue(), 5)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 1, 0).getValue(), 2)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 1, 1).getValue(), 2)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 1, 2).getValue(), 2)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 1, 3).getValue(), 2)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 1, 4).getValue(), 2)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 2, 0).getValue(), 3)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 2, 1).getValue(), 3)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 2, 2).getValue(), 3)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 2, 3).getValue(), 3)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 2, 4).getValue(), 3)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 3, 0).getValue(), 4)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 3, 1).getValue(), 4)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 3, 2).getValue(), 4)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 3, 3).getValue(), 4)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 3, 4).getValue(), 4)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 4, 0).getValue(), 5)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 4, 1).getValue(), 5)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 4, 2).getValue(), 5)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 4, 3).getValue(), 5)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 4, 4).getValue(), 5)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 5, 0).getValue(), 6)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 5, 1).getValue(), 6)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 5, 2).getValue(), 6)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 5, 3).getValue(), 6)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 5, 4).getValue(), 6)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 6, 0).getString(), "random content")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 6, 1).getValue(), 0)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 6, 2).getValue(), 0)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 6, 3).getValue(), 0)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 6, 4).getString(), "random content")

        def test_text_to_columns_pipespace(self):
            #Space as Separator
            with self.ui_test.load_file(get_url_for_data_file("text_to_columns_pipe_space.ods")) as calc_doc:
                gridwin = xCalcDoc.getChild("grid_window")
                #Make sure that tools-options-StarOffice Calc-General-Input settings-Show overwrite warning when pasting data is tagged.
            with self.ui_test.execute_dialog_through_command(".uno:OptionsTreeDialog") as xDialogOpt:

                xPages = xDialogOpt.getChild("pages")
                xWriterEntry = xPages.getChild('3')                 # Calc
                xWriterEntry.executeAction("EXPAND", tuple())
                xWriterGeneralEntry = xWriterEntry.getChild('0')
                xWriterGeneralEntry.executeAction("SELECT", tuple())          #General / replwarncb
                xreplwarncb = xDialogOpt.getChild("replwarncb")
                if (get_state_as_dict(xreplwarncb)["Selected"]) == "false":
                    xreplwarncb.executeAction("CLICK", tuple())

            # Select A1:A5 on Sheet
            gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:A5"}))
            # Data - Text to Columns
            with self.ui_test.execute_dialog_through_command(".uno:TextToColumns", close_button="") as xDialog:
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
                with self.ui_test.execute_blocking_action(xOK.executeAction, args=('CLICK', ()), close_button="yes"):
                    pass

            #Verify
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 0).getValue(), 1)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 1).getValue(), 2)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 2).getValue(), 3)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 3).getValue(), 4)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 4).getValue(), 5)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 1, 0).getValue(), 2)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 1, 1).getValue(), 2)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 1, 2).getValue(), 2)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 1, 3).getValue(), 2)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 1, 4).getValue(), 2)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 2, 0).getValue(), 3)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 2, 1).getValue(), 3)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 2, 2).getValue(), 3)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 2, 3).getValue(), 3)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 2, 4).getValue(), 3)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 3, 0).getValue(), 4)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 3, 1).getValue(), 4)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 3, 2).getValue(), 4)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 3, 3).getValue(), 4)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 3, 4).getValue(), 4)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 4, 0).getValue(), 5)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 4, 1).getValue(), 5)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 4, 2).getValue(), 5)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 4, 3).getValue(), 5)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 4, 4).getValue(), 5)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 5, 0).getValue(), 6)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 5, 1).getValue(), 6)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 5, 2).getValue(), 6)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 5, 3).getValue(), 6)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 5, 4).getValue(), 6)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 6, 0).getString(), "random content")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 6, 1).getValue(), 0)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 6, 2).getValue(), 0)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 6, 3).getValue(), 0)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 6, 4).getString(), "random content")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
