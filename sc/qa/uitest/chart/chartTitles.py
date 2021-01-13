# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import select_pos
from uitest.uihelper.common import change_measurement_unit
from uitest.uihelper.calc import enter_text_to_cell
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file, type_text

#Chart Display Titles dialog

class chartTitles(UITestCase):
   def test_chart_display_titles_dialog(self):
    calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf98390.ods"))
    xCalcDoc = self.xUITest.getTopFocusWindow()
    gridwin = xCalcDoc.getChild("grid_window")
    document = self.ui_test.get_component()

    xCS = document.Sheets[0].Charts[0].getEmbeddedObject().FirstDiagram.CoordinateSystems[0]

    self.assertFalse(document.Sheets[0].Charts[0].getEmbeddedObject().HasMainTitle)
    self.assertFalse(document.Sheets[0].Charts[0].getEmbeddedObject().HasSubTitle)
    self.assertIsNone(xCS.getAxisByDimension(0, 0).TitleObject)
    self.assertIsNone(xCS.getAxisByDimension(1, 0).TitleObject)

    gridwin.executeAction("SELECT", mkPropertyValues({"OBJECT": "Object 1"}))
    gridwin.executeAction("ACTIVATE", tuple())
    xChartMainTop = self.xUITest.getTopFocusWindow()
    xChartMain = xChartMainTop.getChild("chart_window")
    xSeriesObj =  xChartMain.getChild("CID/D=0:CS=0:CT=0:Series=0")
    self.ui_test.execute_dialog_through_action(xSeriesObj, "COMMAND", mkPropertyValues({"COMMAND": "InsertMenuTitles"}))
    xDialog = self.xUITest.getTopFocusWindow()

    maintitle = xDialog.getChild("maintitle")
    subtitle = xDialog.getChild("subtitle")
    primaryXaxis = xDialog.getChild("primaryXaxis")
    primaryYaxis = xDialog.getChild("primaryYaxis")
    secondaryXaxis = xDialog.getChild("secondaryXaxis")
    secondaryYaxis = xDialog.getChild("secondaryYaxis")

    maintitle.executeAction("TYPE", mkPropertyValues({"TEXT":"A"}))
    subtitle.executeAction("TYPE", mkPropertyValues({"TEXT":"B"}))
    primaryXaxis.executeAction("TYPE", mkPropertyValues({"TEXT":"C"}))
    primaryYaxis.executeAction("TYPE", mkPropertyValues({"TEXT":"D"}))
    secondaryXaxis.executeAction("TYPE", mkPropertyValues({"TEXT":"E"}))
    secondaryYaxis.executeAction("TYPE", mkPropertyValues({"TEXT":"F"}))

    xOKBtn = xDialog.getChild("ok")
    self.ui_test.close_dialog_through_button(xOKBtn)

    self.assertTrue(document.Sheets[0].Charts[0].getEmbeddedObject().HasMainTitle)
    self.assertTrue(document.Sheets[0].Charts[0].getEmbeddedObject().HasSubTitle)
    self.assertEqual("A", document.Sheets[0].Charts[0].getEmbeddedObject().Title.String)
    self.assertEqual("B", document.Sheets[0].Charts[0].getEmbeddedObject().SubTitle.String)
    self.assertEqual("C", xCS.getAxisByDimension(0, 0).TitleObject.Text[0].String)
    self.assertEqual("D", xCS.getAxisByDimension(1, 0).TitleObject.Text[0].String)
    self.assertEqual("E", xCS.getAxisByDimension(0, 1).TitleObject.Text[0].String)
    self.assertEqual("F", xCS.getAxisByDimension(1, 1).TitleObject.Text[0].String)

    #reopen and verify InsertMenuTitles dialog
    gridwin.executeAction("SELECT", mkPropertyValues({"OBJECT": "Object 1"}))
    gridwin.executeAction("ACTIVATE", tuple())
    xChartMainTop = self.xUITest.getTopFocusWindow()
    xChartMain = xChartMainTop.getChild("chart_window")
    xSeriesObj =  xChartMain.getChild("CID/D=0:CS=0:CT=0:Series=0")
    self.ui_test.execute_dialog_through_action(xSeriesObj, "COMMAND", mkPropertyValues({"COMMAND": "InsertMenuTitles"}))
    xDialog = self.xUITest.getTopFocusWindow()

    maintitle = xDialog.getChild("maintitle")
    subtitle = xDialog.getChild("subtitle")
    primaryXaxis = xDialog.getChild("primaryXaxis")
    primaryYaxis = xDialog.getChild("primaryYaxis")
    secondaryXaxis = xDialog.getChild("secondaryXaxis")
    secondaryYaxis = xDialog.getChild("secondaryYaxis")

    self.assertEqual(get_state_as_dict(maintitle)["Text"], "A")
    self.assertEqual(get_state_as_dict(subtitle)["Text"], "B")
    self.assertEqual(get_state_as_dict(primaryXaxis)["Text"], "C")
    self.assertEqual(get_state_as_dict(primaryYaxis)["Text"], "D")
    self.assertEqual(get_state_as_dict(secondaryXaxis)["Text"], "E")
    self.assertEqual(get_state_as_dict(secondaryYaxis)["Text"], "F")

    xOKBtn = xDialog.getChild("ok")
    self.ui_test.close_dialog_through_button(xOKBtn)

    self.ui_test.close_doc()

   def test_title_move_with_arrows_keys(self):

    calc_doc = self.ui_test.load_file(get_url_for_data_file("chartArea.ods"))
    xCalcDoc = self.xUITest.getTopFocusWindow()
    gridwin = xCalcDoc.getChild("grid_window")

    change_measurement_unit(self, "Centimeter")

    gridwin.executeAction("SELECT", mkPropertyValues({"OBJECT": "Object 1"}))
    gridwin.executeAction("ACTIVATE", tuple())
    xChartMainTop = self.xUITest.getTopFocusWindow()
    xChartMain = xChartMainTop.getChild("chart_window")

    # Select the title
    xTitle = xChartMain.getChild("CID/Title=")
    xTitle.executeAction("SELECT", tuple())

    self.ui_test.execute_dialog_through_action(xTitle, "COMMAND", mkPropertyValues({"COMMAND": "TransformDialog"}))

    xDialog = self.xUITest.getTopFocusWindow()
    self.assertEqual("3.52", get_state_as_dict(xDialog.getChild("MTR_FLD_POS_X"))['Value'])
    self.assertEqual("0.3", get_state_as_dict(xDialog.getChild("MTR_FLD_POS_Y"))['Value'])

    xOkBtn = xDialog.getChild("ok")
    xOkBtn.executeAction("CLICK", tuple())

    xChartMain.executeAction("TYPE", mkPropertyValues({"KEYCODE": "UP"}))
    xChartMain.executeAction("TYPE", mkPropertyValues({"KEYCODE": "LEFT"}))

    self.ui_test.execute_dialog_through_action(xTitle, "COMMAND", mkPropertyValues({"COMMAND": "TransformDialog"}))

    # Check the position has changed after moving the title using the arrows keys
    xDialog = self.xUITest.getTopFocusWindow()
    self.assertEqual("3.42", get_state_as_dict(xDialog.getChild("MTR_FLD_POS_X"))['Value'])
    self.assertEqual("0.2", get_state_as_dict(xDialog.getChild("MTR_FLD_POS_Y"))['Value'])

    xOkBtn = xDialog.getChild("ok")
    xOkBtn.executeAction("CLICK", tuple())

    self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab:
