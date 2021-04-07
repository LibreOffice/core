# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict
#uitest sc / View-Zoom

class calcZoom(UITestCase):
    def test_zoom_calc(self):
        MainDoc = self.ui_test.create_doc_in_start_center("calc")
        MainWindow = self.xUITest.getTopFocusWindow()
        gridwin = MainWindow.getChild("grid_window")
        gridwin.executeAction("SELECT", mkPropertyValues({"TABLE": "0"}))
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))

        #Set the Zoom to be 100%
        gridwin.executeAction("SET", mkPropertyValues({"ZOOM": "100"}))
        self.assertEqual(get_state_as_dict(gridwin)["Zoom"], "100")

        #dialog View-Zoom-Zoom
        self.ui_test.execute_dialog_through_command(".uno:Zoom")
        xDialog = self.xUITest.getTopFocusWindow()

        #select fit weight & height - OK - open and verify
        fitwandh = xDialog.getChild("fitwandh")
        fitwandh.executeAction("CLICK",tuple())
        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        self.ui_test.execute_dialog_through_command(".uno:Zoom")
        xDialog = self.xUITest.getTopFocusWindow()
        #select fit weight - OK - open and verify
        fitw = xDialog.getChild("fitw")
        fitw.executeAction("CLICK",tuple())
        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        self.ui_test.execute_dialog_through_command(".uno:Zoom")
        xDialog = self.xUITest.getTopFocusWindow()
        #select 100% & Automatic - OK - open and verify
        x100pc = xDialog.getChild("100pc")
        x100pc.executeAction("CLICK", tuple())
        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)
        self.assertEqual(get_state_as_dict(gridwin)["Zoom"], "100")

        #Set the Zoom to be 103%
        gridwin.executeAction("SET", mkPropertyValues({"ZOOM": "103"}))
        self.assertEqual(get_state_as_dict(gridwin)["Zoom"], "103")

        self.ui_test.execute_dialog_through_command(".uno:Zoom")
        xDialog = self.xUITest.getTopFocusWindow()
        #select variable option and make zoom 100% again - OK - open and verify
        zoomsb = xDialog.getChild("zoomsb")
        self.assertEqual(get_state_as_dict(zoomsb)["Text"], "103%")
        zoomsb.executeAction("DOWN",tuple())
        zoomsb.executeAction("DOWN",tuple())
        zoomsb.executeAction("DOWN",tuple())
        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)
        self.assertEqual(get_state_as_dict(gridwin)["Zoom"], "100")

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
