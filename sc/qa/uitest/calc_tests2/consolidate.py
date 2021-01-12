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
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues

class consolidate(UITestCase):
    def test_consolidate(self):
        calc_doc = self.ui_test.load_file(get_url_for_data_file("consolidate.ods"))
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()

        self.ui_test.execute_modeless_dialog_through_command(".uno:DataConsolidate")
        xDialog = self.xUITest.getTopFocusWindow()
        xfunc = xDialog.getChild("func")
        xlbdataarea = xDialog.getChild("lbdataarea")
        xadd = xDialog.getChild("add")
        xbyrow = xDialog.getChild("byrow")
        xbycol = xDialog.getChild("bycol")
        xeddestarea = xDialog.getChild("eddestarea")

        props = {"TEXT": "Sum"}
        actionProps = mkPropertyValues(props)
        xfunc.executeAction("SELECT", actionProps)

        propsA = {"TEXT": "range1"}
        actionPropsA = mkPropertyValues(propsA)
        xlbdataarea.executeAction("SELECT", actionPropsA)
        xadd.executeAction("CLICK", tuple())
        propsB = {"TEXT": "range2"}
        actionPropsB = mkPropertyValues(propsB)
        xlbdataarea.executeAction("SELECT", actionPropsB)
        xadd.executeAction("CLICK", tuple())
        propsC = {"TEXT": "range3"}
        actionPropsC = mkPropertyValues(propsC)
        xlbdataarea.executeAction("SELECT", actionPropsC)
        xadd.executeAction("CLICK", tuple())
        xbyrow.executeAction("CLICK", tuple())
        xbycol.executeAction("CLICK", tuple())
        xeddestarea.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
        xeddestarea.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
        xeddestarea.executeAction("TYPE", mkPropertyValues({"TEXT":"$Total.$A$2"}))
        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)
        #verify
        self.assertEqual(get_cell_by_position(document, 0, 1, 2).getValue(), 300)
        self.assertEqual(get_cell_by_position(document, 0, 1, 3).getValue(), 303)
        self.assertEqual(get_cell_by_position(document, 0, 1, 4).getValue(), 306)
        self.assertEqual(get_cell_by_position(document, 0, 1, 5).getValue(), 309)

        self.assertEqual(get_cell_by_position(document, 0, 2, 2).getValue(), 303)
        self.assertEqual(get_cell_by_position(document, 0, 2, 3).getValue(), 306)
        self.assertEqual(get_cell_by_position(document, 0, 2, 4).getValue(), 309)
        self.assertEqual(get_cell_by_position(document, 0, 2, 5).getValue(), 312)

        self.assertEqual(get_cell_by_position(document, 0, 3, 2).getValue(), 306)
        self.assertEqual(get_cell_by_position(document, 0, 3, 3).getValue(), 309)
        self.assertEqual(get_cell_by_position(document, 0, 3, 4).getValue(), 312)
        self.assertEqual(get_cell_by_position(document, 0, 3, 5).getValue(), 315)

        self.assertEqual(get_cell_by_position(document, 0, 4, 2).getValue(), 309)
        self.assertEqual(get_cell_by_position(document, 0, 4, 3).getValue(), 312)
        self.assertEqual(get_cell_by_position(document, 0, 4, 4).getValue(), 315)
        self.assertEqual(get_cell_by_position(document, 0, 4, 5).getValue(), 318)
        #verify dialog
        self.ui_test.execute_modeless_dialog_through_command(".uno:DataConsolidate")
        xDialog = self.xUITest.getTopFocusWindow()
        xfunc = xDialog.getChild("func")
        xlbdataarea = xDialog.getChild("lbdataarea")
        xdelete = xDialog.getChild("delete")
        xbyrow = xDialog.getChild("byrow")
        xbycol = xDialog.getChild("bycol")
        xeddestarea = xDialog.getChild("eddestarea")
        xconsareas = xDialog.getChild("consareas")
        self.assertEqual(get_state_as_dict(xfunc)["SelectEntryText"], "Sum")
        self.assertEqual(get_state_as_dict(xconsareas)["Children"], "3")
        self.assertEqual(get_state_as_dict(xeddestarea)["Text"], "$Total.$A$2")
        self.assertEqual(get_state_as_dict(xbyrow)["Selected"], "true")
        self.assertEqual(get_state_as_dict(xbycol)["Selected"], "true")
        #delete first range
        xFirstEntry = xconsareas.getChild("0")
        xFirstEntry.executeAction("SELECT", tuple())
        xdelete.executeAction("CLICK", tuple())
        self.assertEqual(get_state_as_dict(xconsareas)["Children"], "2")
        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        self.assertEqual(get_cell_by_position(document, 0, 1, 2).getValue(), 200)
        self.assertEqual(get_cell_by_position(document, 0, 1, 3).getValue(), 202)
        self.assertEqual(get_cell_by_position(document, 0, 1, 4).getValue(), 204)
        self.assertEqual(get_cell_by_position(document, 0, 1, 5).getValue(), 206)

        self.assertEqual(get_cell_by_position(document, 0, 2, 2).getValue(), 202)
        self.assertEqual(get_cell_by_position(document, 0, 2, 3).getValue(), 204)
        self.assertEqual(get_cell_by_position(document, 0, 2, 4).getValue(), 206)
        self.assertEqual(get_cell_by_position(document, 0, 2, 5).getValue(), 208)

        self.assertEqual(get_cell_by_position(document, 0, 3, 2).getValue(), 204)
        self.assertEqual(get_cell_by_position(document, 0, 3, 3).getValue(), 206)
        self.assertEqual(get_cell_by_position(document, 0, 3, 4).getValue(), 208)
        self.assertEqual(get_cell_by_position(document, 0, 3, 5).getValue(), 210)

        self.assertEqual(get_cell_by_position(document, 0, 4, 2).getValue(), 206)
        self.assertEqual(get_cell_by_position(document, 0, 4, 3).getValue(), 208)
        self.assertEqual(get_cell_by_position(document, 0, 4, 4).getValue(), 210)
        self.assertEqual(get_cell_by_position(document, 0, 4, 5).getValue(), 212)

        # test cancel button
        self.ui_test.execute_modeless_dialog_through_command(".uno:DataConsolidate")
        xDialog = self.xUITest.getTopFocusWindow()
        xCancelBtn = xDialog.getChild("cancel")
        self.ui_test.close_dialog_through_button(xCancelBtn)

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
