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

#Bug 122977 - CRASH: "Tools"->"Options" "LibreOfficeDev"->"Charts"->"Default Colors"

class chartDefaultColors(UITestCase):
    def test_tdf122977_crash_chart_default_colors_options(self):
        calc_doc = self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        #Go to Tools -> Options -> Charts -> Default Colors
        self.ui_test.execute_dialog_through_command(".uno:OptionsTreeDialog")  #optionsdialog
        xDialogOpt = self.xUITest.getTopFocusWindow()

        xPages = xDialogOpt.getChild("pages")
        xChartEntry = xPages.getChild('5')                 # Charts
        xChartEntry.executeAction("EXPAND", tuple())
        xChartGeneralEntry = xChartEntry.getChild('0')
        xChartGeneralEntry.executeAction("SELECT", tuple())          #Default Colors
        xColors = xDialogOpt.getChild("colors")
        xAdd = xDialogOpt.getChild("add")
        xDelete = xDialogOpt.getChild("delete")
        xDefault = xDialogOpt.getChild("default")
        #click Default - reset
        xDefault.executeAction("CLICK", tuple())
        nrDefaultColors = get_state_as_dict(xColors)["EntryCount"]
        nrDefaultColors1 = int(nrDefaultColors) + 1
        xAdd.executeAction("CLICK", tuple())    #add new color
        self.assertEqual(get_state_as_dict(xColors)["EntryCount"], str(nrDefaultColors1))
        #delete new color
        def handle_delete_dlg(dialog):
            xyesBtn = dialog.getChild("yes")
            self.ui_test.close_dialog_through_button(xyesBtn)

        self.ui_test.execute_blocking_action(xDelete.executeAction, args=('CLICK', ()),
                dialog_handler=handle_delete_dlg)
        self.assertEqual(get_state_as_dict(xColors)["EntryCount"], nrDefaultColors)

        xAdd.executeAction("CLICK", tuple())    #add new color
        self.assertEqual(get_state_as_dict(xColors)["EntryCount"], str(nrDefaultColors1))
        #click Default
        xDefault.executeAction("CLICK", tuple())
        self.assertEqual(get_state_as_dict(xColors)["EntryCount"], nrDefaultColors)

        xCancelBtn = xDialogOpt.getChild("cancel")
        self.ui_test.close_dialog_through_button(xCancelBtn)

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
