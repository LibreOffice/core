# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict
from libreoffice.uno.propertyvalue import mkPropertyValues
#uitest sw / View-Zoom

class writerZoom(UITestCase):
    def test_zoom_writer(self):
        writer_doc = self.ui_test.create_doc_in_start_center("writer")
        document = self.ui_test.get_component()
        xWriterDoc = self.xUITest.getTopFocusWindow()
        #dialog View-Zoom-Zoom
        self.ui_test.execute_dialog_through_command(".uno:Zoom")
        xDialog = self.xUITest.getTopFocusWindow()

        #select Optimal & Automatic - OK - open and verify
        optimal = xDialog.getChild("optimal")
        optimal.executeAction("CLICK", tuple())
        automatic = xDialog.getChild("automatic")
        automatic.executeAction("CLICK", tuple())
        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        self.ui_test.execute_dialog_through_command(".uno:Zoom")
        xDialog = self.xUITest.getTopFocusWindow()
        optimal = xDialog.getChild("optimal")
        automatic = xDialog.getChild("automatic")
        self.assertEqual(get_state_as_dict(optimal)["Checked"], "true")
        self.assertEqual(get_state_as_dict(automatic)["Checked"], "true")
        #select fit weight & Single page - OK - open and verify
        fitwandh = xDialog.getChild("fitwandh")
        singlepage = xDialog.getChild("singlepage")
        fitwandh.executeAction("CLICK", tuple())
        singlepage.executeAction("CLICK", tuple())
        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        self.ui_test.execute_dialog_through_command(".uno:Zoom")
        xDialog = self.xUITest.getTopFocusWindow()
        singlepage = xDialog.getChild("singlepage")
        fitwandh = xDialog.getChild("fitwandh")
        self.assertEqual(get_state_as_dict(singlepage)["Checked"], "true")
        self.assertEqual(get_state_as_dict(fitwandh)["Checked"], "true")
        #select fit width & columns  - OK - open and verify
        fitw = xDialog.getChild("fitw")
        columnssb = xDialog.getChild("columnssb")
        columns = xDialog.getChild("columns")
        fitw.executeAction("CLICK", tuple())
        columns.executeAction("CLICK", tuple())
        columnssb.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
        columnssb.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
        columnssb.executeAction("TYPE", mkPropertyValues({"TEXT":"3"}))
        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        self.ui_test.execute_dialog_through_command(".uno:Zoom")
        xDialog = self.xUITest.getTopFocusWindow()
        fitw = xDialog.getChild("fitw")
        columnssb = xDialog.getChild("columnssb")
        columns = xDialog.getChild("columns")
        self.assertEqual(get_state_as_dict(fitw)["Checked"], "true")
        self.assertEqual(get_state_as_dict(columns)["Checked"], "true")
        self.assertEqual(get_state_as_dict(columnssb)["Text"], "3")
        #select 100% & Automatic - OK - open and verify
        automatic = xDialog.getChild("automatic")
        x100pc = xDialog.getChild("100pc")
        x100pc.executeAction("CLICK", tuple())
        automatic.executeAction("CLICK", tuple())
        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        self.ui_test.execute_dialog_through_command(".uno:Zoom")
        xDialog = self.xUITest.getTopFocusWindow()
        automatic = xDialog.getChild("automatic")
        x100pc = xDialog.getChild("100pc")
        self.assertEqual(get_state_as_dict(automatic)["Checked"], "true")
        self.assertEqual(get_state_as_dict(x100pc)["Checked"], "true")
        #select variable 103% & Automatic - OK - open and verify
        variable = xDialog.getChild("variable")
        zoomsb = xDialog.getChild("zoomsb")
        variable.executeAction("CLICK", tuple())
        zoomsb.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
        zoomsb.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
        zoomsb.executeAction("TYPE", mkPropertyValues({"TEXT":"101"}))
        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        self.ui_test.execute_dialog_through_command(".uno:Zoom")
        xDialog = self.xUITest.getTopFocusWindow()
        variable = xDialog.getChild("variable")
        zoomsb = xDialog.getChild("zoomsb")
        self.assertEqual(get_state_as_dict(variable)["Checked"], "true")
        self.assertEqual(get_state_as_dict(zoomsb)["Text"], "101%")
        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
