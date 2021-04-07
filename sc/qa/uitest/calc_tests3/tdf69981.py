# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues

class tdf69981(UITestCase):

    def test_tdf69981_text_to_columns(self):

        calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf69981.ods"))
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

        #Select A2:A7
        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A2:A7"}))
        #Data - Text to Columns
        self.ui_test.execute_dialog_through_command(".uno:TextToColumns")
        xDialog = self.xUITest.getTopFocusWindow()
        xtab = xDialog.getChild("tab")
        xcomma = xDialog.getChild("comma")
        xtab.executeAction("CLICK", tuple())
        xcomma.executeAction("CLICK", tuple())
        #Click Ok
        #overwrite warning come up
        #press Ok.
        xOK = xDialog.getChild("ok")
        def handle_warn_dlg(dialog):
            xyesBtn = dialog.getChild("yes")
            self.ui_test.close_dialog_through_button(xyesBtn)

        self.ui_test.execute_blocking_action(xOK.executeAction, args=('CLICK', ()),
                dialog_handler=handle_warn_dlg)
        #Verify
        self.assertEqual(get_cell_by_position(document, 0, 0, 0).getString(), "Original")
        self.assertEqual(get_cell_by_position(document, 0, 0, 1).getString(), "a")
        self.assertEqual(get_cell_by_position(document, 0, 0, 2).getString(), "")
        self.assertEqual(get_cell_by_position(document, 0, 0, 3).getString(), "")
        self.assertEqual(get_cell_by_position(document, 0, 0, 4).getString(), "a")
        self.assertEqual(get_cell_by_position(document, 0, 0, 5).getString(), "a")
        self.assertEqual(get_cell_by_position(document, 0, 0, 6).getString(), "a")

        self.assertEqual(get_cell_by_position(document, 0, 1, 0).getString(), "Copy")
        self.assertEqual(get_cell_by_position(document, 0, 1, 1).getString(), "b")
        self.assertEqual(get_cell_by_position(document, 0, 1, 2).getString(), "b")
        self.assertEqual(get_cell_by_position(document, 0, 1, 3).getString(), "")
        self.assertEqual(get_cell_by_position(document, 0, 1, 4).getString(), "")
        self.assertEqual(get_cell_by_position(document, 0, 1, 5).getString(), "b")
        self.assertEqual(get_cell_by_position(document, 0, 1, 6).getString(), "")

        self.assertEqual(get_cell_by_position(document, 0, 2, 0).getString(), "")
        self.assertEqual(get_cell_by_position(document, 0, 2, 1).getString(), "c")
        self.assertEqual(get_cell_by_position(document, 0, 2, 2).getString(), "c")
        self.assertEqual(get_cell_by_position(document, 0, 2, 3).getString(), "c")
        self.assertEqual(get_cell_by_position(document, 0, 2, 4).getString(), "")
        self.assertEqual(get_cell_by_position(document, 0, 2, 5).getString(), "")
        self.assertEqual(get_cell_by_position(document, 0, 2, 6).getString(), "c")

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
