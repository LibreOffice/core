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
from uitest.uihelper.common import select_by_text
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues

class consolidate(UITestCase):
    def test_consolidate(self):
        with self.ui_test.load_file(get_url_for_data_file("consolidate.ods")) as calc_doc:
            with self.ui_test.execute_modeless_dialog_through_command(".uno:DataConsolidate") as xDialog:
                xfunc = xDialog.getChild("func")
                xlbdataarea = xDialog.getChild("lbdataarea")
                xadd = xDialog.getChild("add")
                xbyrow = xDialog.getChild("byrow")
                xbycol = xDialog.getChild("bycol")
                xeddestarea = xDialog.getChild("eddestarea")

                select_by_text(xfunc, "Sum")

                select_by_text(xlbdataarea, "range1")
                xadd.executeAction("CLICK", tuple())
                select_by_text(xlbdataarea, "range2")
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

            #verify
            self.assertEqual(get_cell_by_position(calc_doc, 0, 1, 2).getValue(), 300)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 1, 3).getValue(), 303)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 1, 4).getValue(), 306)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 1, 5).getValue(), 309)

            self.assertEqual(get_cell_by_position(calc_doc, 0, 2, 2).getValue(), 303)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 2, 3).getValue(), 306)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 2, 4).getValue(), 309)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 2, 5).getValue(), 312)

            self.assertEqual(get_cell_by_position(calc_doc, 0, 3, 2).getValue(), 306)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 3, 3).getValue(), 309)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 3, 4).getValue(), 312)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 3, 5).getValue(), 315)

            self.assertEqual(get_cell_by_position(calc_doc, 0, 4, 2).getValue(), 309)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 4, 3).getValue(), 312)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 4, 4).getValue(), 315)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 4, 5).getValue(), 318)
            #verify dialog
            with self.ui_test.execute_modeless_dialog_through_command(".uno:DataConsolidate") as xDialog:
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

            self.assertEqual(get_cell_by_position(calc_doc, 0, 1, 2).getValue(), 200)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 1, 3).getValue(), 202)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 1, 4).getValue(), 204)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 1, 5).getValue(), 206)

            self.assertEqual(get_cell_by_position(calc_doc, 0, 2, 2).getValue(), 202)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 2, 3).getValue(), 204)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 2, 4).getValue(), 206)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 2, 5).getValue(), 208)

            self.assertEqual(get_cell_by_position(calc_doc, 0, 3, 2).getValue(), 204)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 3, 3).getValue(), 206)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 3, 4).getValue(), 208)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 3, 5).getValue(), 210)

            self.assertEqual(get_cell_by_position(calc_doc, 0, 4, 2).getValue(), 206)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 4, 3).getValue(), 208)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 4, 4).getValue(), 210)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 4, 5).getValue(), 212)

            # test cancel button
            with self.ui_test.execute_modeless_dialog_through_command(".uno:DataConsolidate", close_button="cancel"):
                pass

# vim: set shiftwidth=4 softtabstop=4 expandtab:
