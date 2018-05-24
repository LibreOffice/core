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
import org.libreoffice.unotest
import pathlib
#Bug 70925 - Advanced filter: case sensitive criterium produces no result

def get_url_for_data_file(file_name):
    return pathlib.Path(org.libreoffice.unotest.makeCopyFromTDOC(file_name)).as_uri()

class tdf70925(UITestCase):
    def test_td70925_advanced_filter(self):
        calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf70925.ods"))
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:F484"}))

        self.ui_test.execute_modeless_dialog_through_command(".uno:DataFilterSpecialFilter")
        xDialog = self.xUITest.getTopFocusWindow()
        xcase  = xDialog.getChild("case")
        xedfilterarea = xDialog.getChild("edfilterarea")
        xedfilterarea.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
        xedfilterarea.executeAction("TYPE", mkPropertyValues({"KEYCODE": "BACKSPACE"}))
        xedfilterarea.executeAction("TYPE", mkPropertyValues({"TEXT":"$Feuille2.A1:F2"}))
        if (get_state_as_dict(xcase)["Selected"]) == "false":
            xcase.executeAction("CLICK", tuple())
        xcopyresult  = xDialog.getChild("copyresult")
        xedcopyarea  = xDialog.getChild("edcopyarea")
        if (get_state_as_dict(xcopyresult)["Selected"]) == "false":
            xcopyresult.executeAction("CLICK", tuple())
        xedcopyarea.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
        xedcopyarea.executeAction("TYPE", mkPropertyValues({"KEYCODE": "BACKSPACE"}))
        xedcopyarea.executeAction("TYPE", mkPropertyValues({"TEXT":"$Feuille3.A1"}))
        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)
        #verify
        self.assertEqual(get_cell_by_position(document, 2, 0, 0).getString(), "Titre")
        self.assertEqual(get_cell_by_position(document, 2, 5, 0).getString(), "Genre")
        self.assertEqual(get_cell_by_position(document, 2, 0, 1).getString(), "Une femme neuve")
        self.assertEqual(get_cell_by_position(document, 2, 0, 183).getString(), "Beignets de tomates vertes")

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab: