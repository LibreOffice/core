# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_url_for_data_file
from libreoffice.uno.propertyvalue import mkPropertyValues
from com.sun.star.awt import MouseButton
from com.sun.star.awt import MouseEvent

class tdf132714(UITestCase):
    def test_tdf132714(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf132714.odt")):

            # delete second row (first data row) in the associated text table of the chart
            self.xUITest.executeCommand(".uno:GoDown")
            self.xUITest.executeCommand(".uno:GoDown")
            self.xUITest.executeCommand(".uno:GoDown")
            # Without the fix in place, at this point crash occurs.
            self.xUITest.executeCommand(".uno:DeleteRows")

    def test_delete_table(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf132714.odt")) as document:

            # delete second row (first data row) in the associated text table of the chart
            self.xUITest.executeCommand(".uno:GoDown")
            self.xUITest.executeCommand(".uno:GoDown")
            # Without the fix in place, at this point crash occurs.
            self.xUITest.executeCommand(".uno:DeleteTable")

            # select embedded chart
            self.assertEqual(1, len(document.EmbeddedObjects))
            document.CurrentController.select(document.getEmbeddedObjects()[0])
            self.assertEqual("SwXTextEmbeddedObject", document.CurrentSelection.getImplementationName())

            xChartMainTop = self.xUITest.getTopFocusWindow()
            xWriterEdit = xChartMainTop.getChild("writer_edit")
            # edit object by pressing Enter
            xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE":"RETURN"}))

            # create mouse event in the chart area
            xFrame = document.getCurrentController().getFrame()
            self.assertIsNotNone(xFrame)
            xWindow = xFrame.getContainerWindow()
            self.assertIsNotNone(xWindow)

            xMouseEvent = MouseEvent()
            xMouseEvent.Modifiers = 0
            xMouseEvent.Buttons = MouseButton.LEFT
            xMouseEvent.X = 1000
            xMouseEvent.Y = 400
            xMouseEvent.ClickCount = 1
            xMouseEvent.PopupTrigger = False
            xMouseEvent.Source = xWindow

            # send mouse event
            xToolkitRobot = xWindow.getToolkit()
            self.assertIsNotNone(xToolkitRobot)

            # Click in the chart area

            # Without the fix in place, this test would have crashed here
            xToolkitRobot.mouseMove(xMouseEvent)

    def test_data_ranges(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf132714.odt")) as document:

            # delete second row (first data row) in the associated text table of the chart
            self.xUITest.executeCommand(".uno:GoDown")
            self.xUITest.executeCommand(".uno:GoDown")
            # Without the fix in place, at this point crash occurs.
            self.xUITest.executeCommand(".uno:DeleteTable")

            # select embedded chart
            self.assertEqual(1, len(document.EmbeddedObjects))
            document.CurrentController.select(document.EmbeddedObjects[0])
            self.assertEqual("SwXTextEmbeddedObject", document.CurrentSelection.getImplementationName())

            xChartMainTop = self.xUITest.getTopFocusWindow()
            xWriterEdit = xChartMainTop.getChild("writer_edit")
            # edit object by pressing Enter
            xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE":"RETURN"}))

            # open DataRanges dialog window
            xChartMain = xChartMainTop.getChild("chart_window")
            xSeriesObj =  xChartMain.getChild("CID/Page=")

            # Without the fix in place, this test would have crashed here
            with self.ui_test.execute_dialog_through_action(xSeriesObj, "COMMAND", mkPropertyValues({"COMMAND": "DataRanges"})):
                pass


# vim: set shiftwidth=4 softtabstop=4 expandtab:
