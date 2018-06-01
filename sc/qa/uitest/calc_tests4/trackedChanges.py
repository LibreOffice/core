# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
# tests for tracked changes ; tdf912270

from uitest.framework import UITestCase
from uitest.debug import sleep
from libreoffice.calc.document import get_cell_by_position
import time
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.calc import enter_text_to_cell

class CalcTrackedChanges(UITestCase):

    def test_tdf66263_Protect_Records(self):
        calc_doc = self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        self.xUITest.executeCommand(".uno:TraceChangeMode")
        #protect dialog
        self.ui_test.execute_dialog_through_command(".uno:ProtectTraceChangeMode")
        xDialog = self.xUITest.getTopFocusWindow()
        xpass = xDialog.getChild("pass1ed")
        xpass.executeAction("TYPE", mkPropertyValues({"TEXT":"a"}))
        xconfirm = xDialog.getChild("confirm1ed")
        xconfirm.executeAction("TYPE", mkPropertyValues({"TEXT":"a"}))
        xOkBtn = xDialog.getChild("ok")
        xOkBtn.executeAction("CLICK", tuple())
        #verify password
        self.ui_test.execute_dialog_through_command(".uno:ProtectTraceChangeMode")
        xDialog = self.xUITest.getTopFocusWindow()
        xpass = xDialog.getChild("pass1ed")
        xpass.executeAction("TYPE", mkPropertyValues({"TEXT":"a"}))
        xOkBtn = xDialog.getChild("ok")
        xOkBtn.executeAction("CLICK", tuple())

        self.ui_test.close_doc()

    def test_tracked_changes_accept(self):

        calc_doc = self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        #track changes;  enter text to cell
        self.xUITest.executeCommand(".uno:TraceChangeMode")
        enter_text_to_cell(gridwin, "A1", "Test LibreOffice")
        #accept tracked changes
        self.ui_test.execute_modeless_dialog_through_command(".uno:AcceptChanges")
        xTrackDlg = self.xUITest.getTopFocusWindow()
        xAccBtn = xTrackDlg.getChild("accept")
        xAccBtn.executeAction("CLICK", tuple())
        xCancBtn = xTrackDlg.getChild("close")
        xCancBtn.executeAction("CLICK", tuple())

        self.assertEqual(get_cell_by_position(document, 0, 0, 0).getString(), "Test LibreOffice")
        self.ui_test.close_doc()

    def test_tracked_changes_acceptall(self):

        calc_doc = self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        #track changes;  enter text to cell
        self.xUITest.executeCommand(".uno:TraceChangeMode")
        enter_text_to_cell(gridwin, "A1", "Test LibreOffice")
        #accept All tracked changes
        self.ui_test.execute_modeless_dialog_through_command(".uno:AcceptChanges")
        xTrackDlg = self.xUITest.getTopFocusWindow()

        xAccBtn = xTrackDlg.getChild("acceptall")
        xAccBtn.executeAction("CLICK", tuple())
        xCancBtn = xTrackDlg.getChild("close")
        xCancBtn.executeAction("CLICK", tuple())

        self.assertEqual(get_cell_by_position(document, 0, 0, 0).getString(), "Test LibreOffice")
        self.ui_test.close_doc()

    def test_tracked_changes_reject(self):

        calc_doc = self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        #track changes;  enter text to cell
        self.xUITest.executeCommand(".uno:TraceChangeMode")
        enter_text_to_cell(gridwin, "A1", "Test LibreOffice")
        #accept tracked changes
        self.ui_test.execute_modeless_dialog_through_command(".uno:AcceptChanges")
        xTrackDlg = self.xUITest.getTopFocusWindow()

        xRejBtn = xTrackDlg.getChild("reject")
        xRejBtn.executeAction("CLICK", tuple())
        xCancBtn = xTrackDlg.getChild("close")
        xCancBtn.executeAction("CLICK", tuple())

        self.assertEqual(get_cell_by_position(document, 0, 0, 0).getString(), "")
        self.ui_test.close_doc()

    def test_tracked_changes_rejectall(self):

        # FIXME flaky test, passed once, but broke multiple times.
        return
        calc_doc = self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        #track changes;  enter text to cell
        self.xUITest.executeCommand(".uno:TraceChangeMode")
        enter_text_to_cell(gridwin, "A1", "Test LibreOffice")
        #accept tracked changes
        self.ui_test.execute_modeless_dialog_through_command(".uno:AcceptChanges")
        xTrackDlg = self.xUITest.getTopFocusWindow()

        xAccBtn = xTrackDlg.getChild("rejectall")
        xAccBtn.executeAction("CLICK", tuple())
        xCancBtn = xTrackDlg.getChild("close")
        xCancBtn.executeAction("CLICK", tuple())

        self.assertEqual(get_cell_by_position(document, 0, 0, 0).getString(), "")
        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab: