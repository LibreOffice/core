# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
# tests for tracked changes ; tdf912270

from uitest.framework import UITestCase
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.calc import enter_text_to_cell
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file
import datetime

class CalcTrackedChanges(UITestCase):

    def test_tdf131907(self):
        calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf131907.ods"))
        xCalcDoc = self.xUITest.getTopFocusWindow()
        self.ui_test.execute_modeless_dialog_through_command(".uno:AcceptChanges")
        xTrackDlg = self.xUITest.getTopFocusWindow()

        xChangesList = xTrackDlg.getChild("calcchanges")
        self.assertEqual(1, len(xChangesList.getChildren()))

        textStart = "Row inserted \tSheet1.1:1\t \t"
        textEnd = "(Row 1:1 inserted)"
        self.assertTrue(get_state_as_dict(xChangesList.getChild('0'))["Text"].startswith(textStart))
        self.assertTrue(get_state_as_dict(xChangesList.getChild('0'))["Text"].endswith(textEnd))

        #it would crash here
        xRejBtn = xTrackDlg.getChild("reject")
        xRejBtn.executeAction("CLICK", tuple())

        self.assertEqual(2, len(xChangesList.getChildren()))
        self.assertEqual(get_state_as_dict(xChangesList.getChild('0'))["Text"], "Accepted")
        self.assertEqual(get_state_as_dict(xChangesList.getChild('1'))["Text"], "Rejected")

        xCancBtn = xTrackDlg.getChild("close")
        xCancBtn.executeAction("CLICK", tuple())

        self.ui_test.close_doc()

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
        enter_text_to_cell(gridwin, "A2", "Test LibreOffice")
        #accept tracked changes
        self.ui_test.execute_modeless_dialog_through_command(".uno:AcceptChanges")
        xTrackDlg = self.xUITest.getTopFocusWindow()

        xChangesList = xTrackDlg.getChild("calcchanges")
        self.assertEqual(2, len(xChangesList.getChildren()))

        textStart = "Changed contents\tSheet1.A1\t \t" + datetime.datetime.now().strftime("%m/%d/%Y")
        textEnd = "(Cell A1 changed from '<empty>' to 'Test LibreOffice')"
        textStart2 = "Changed contents\tSheet1.A2\t \t" + datetime.datetime.now().strftime("%m/%d/%Y")
        textEnd2 = "(Cell A2 changed from '<empty>' to 'Test LibreOffice')"
        self.assertTrue(get_state_as_dict(xChangesList.getChild('0'))["Text"].startswith(textStart))
        self.assertTrue(get_state_as_dict(xChangesList.getChild('0'))["Text"].endswith(textEnd))
        self.assertTrue(get_state_as_dict(xChangesList.getChild('1'))["Text"].startswith(textStart2))
        self.assertTrue(get_state_as_dict(xChangesList.getChild('1'))["Text"].endswith(textEnd2))

        xAccBtn = xTrackDlg.getChild("accept")
        xAccBtn.executeAction("CLICK", tuple())

        self.assertEqual(2, len(xChangesList.getChildren()))
        self.assertTrue(get_state_as_dict(xChangesList.getChild('0'))["Text"].startswith(textStart2))
        self.assertTrue(get_state_as_dict(xChangesList.getChild('0'))["Text"].endswith(textEnd2))
        self.assertEqual(get_state_as_dict(xChangesList.getChild('1'))["Text"], "Accepted")

        xAccBtn = xTrackDlg.getChild("accept")
        xAccBtn.executeAction("CLICK", tuple())

        self.assertEqual(1, len(xChangesList.getChildren()))
        self.assertEqual(get_state_as_dict(xChangesList.getChild('0'))["Text"], "Accepted")
        xChangesList.getChild('0').executeAction("EXPAND", tuple())

        self.assertEqual(2, len(xChangesList.getChild('0').getChildren()))

        self.assertTrue(get_state_as_dict(xChangesList.getChild('0').getChild('0'))["Text"].startswith(textStart))
        self.assertTrue(get_state_as_dict(xChangesList.getChild('0').getChild('0'))["Text"].endswith(textEnd))
        self.assertTrue(get_state_as_dict(xChangesList.getChild('0').getChild('1'))["Text"].startswith(textStart2))
        self.assertTrue(get_state_as_dict(xChangesList.getChild('0').getChild('1'))["Text"].endswith(textEnd2))

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
        enter_text_to_cell(gridwin, "A2", "Test LibreOffice")
        #accept All tracked changes
        self.ui_test.execute_modeless_dialog_through_command(".uno:AcceptChanges")
        xTrackDlg = self.xUITest.getTopFocusWindow()

        xChangesList = xTrackDlg.getChild("calcchanges")
        self.assertEqual(2, len(xChangesList.getChildren()))

        textStart = "Changed contents\tSheet1.A1\t \t" + datetime.datetime.now().strftime("%m/%d/%Y")
        textEnd = "(Cell A1 changed from '<empty>' to 'Test LibreOffice')"
        textStart2 = "Changed contents\tSheet1.A2\t \t" + datetime.datetime.now().strftime("%m/%d/%Y")
        textEnd2 = "(Cell A2 changed from '<empty>' to 'Test LibreOffice')"
        self.assertTrue(get_state_as_dict(xChangesList.getChild('0'))["Text"].startswith(textStart))
        self.assertTrue(get_state_as_dict(xChangesList.getChild('0'))["Text"].endswith(textEnd))
        self.assertTrue(get_state_as_dict(xChangesList.getChild('1'))["Text"].startswith(textStart2))
        self.assertTrue(get_state_as_dict(xChangesList.getChild('1'))["Text"].endswith(textEnd2))

        xAccBtn = xTrackDlg.getChild("acceptall")
        xAccBtn.executeAction("CLICK", tuple())

        self.assertEqual(1, len(xChangesList.getChildren()))
        self.assertEqual(get_state_as_dict(xChangesList.getChild('0'))["Text"], "Accepted")
        xChangesList.getChild('0').executeAction("EXPAND", tuple())

        self.assertEqual(2, len(xChangesList.getChild('0').getChildren()))

        self.assertTrue(get_state_as_dict(xChangesList.getChild('0').getChild('0'))["Text"].startswith(textStart))
        self.assertTrue(get_state_as_dict(xChangesList.getChild('0').getChild('0'))["Text"].endswith(textEnd))
        self.assertTrue(get_state_as_dict(xChangesList.getChild('0').getChild('1'))["Text"].startswith(textStart2))
        self.assertTrue(get_state_as_dict(xChangesList.getChild('0').getChild('1'))["Text"].endswith(textEnd2))

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
        enter_text_to_cell(gridwin, "A2", "Test LibreOffice")
        #accept tracked changes
        self.ui_test.execute_modeless_dialog_through_command(".uno:AcceptChanges")
        xTrackDlg = self.xUITest.getTopFocusWindow()

        xChangesList = xTrackDlg.getChild("calcchanges")
        self.assertEqual(2, len(xChangesList.getChildren()))

        textStart = "Changed contents\tSheet1.A1\t \t" + datetime.datetime.now().strftime("%m/%d/%Y")
        textEnd = "(Cell A1 changed from '<empty>' to 'Test LibreOffice')"
        textStart2 = "Changed contents\tSheet1.A2\t \t" + datetime.datetime.now().strftime("%m/%d/%Y")
        textEnd2 = "(Cell A2 changed from '<empty>' to 'Test LibreOffice')"
        self.assertTrue(get_state_as_dict(xChangesList.getChild('0'))["Text"].startswith(textStart))
        self.assertTrue(get_state_as_dict(xChangesList.getChild('0'))["Text"].endswith(textEnd))
        self.assertTrue(get_state_as_dict(xChangesList.getChild('1'))["Text"].startswith(textStart2))
        self.assertTrue(get_state_as_dict(xChangesList.getChild('1'))["Text"].endswith(textEnd2))

        xRejBtn = xTrackDlg.getChild("reject")
        xRejBtn.executeAction("CLICK", tuple())

        self.assertEqual(3, len(xChangesList.getChildren()))
        self.assertTrue(get_state_as_dict(xChangesList.getChild('0'))["Text"].startswith(textStart2))
        self.assertTrue(get_state_as_dict(xChangesList.getChild('0'))["Text"].endswith(textEnd2))
        self.assertEqual(get_state_as_dict(xChangesList.getChild('1'))["Text"], "Accepted")
        self.assertEqual(get_state_as_dict(xChangesList.getChild('2'))["Text"], "Rejected")

        xAccBtn = xTrackDlg.getChild("reject")
        xAccBtn.executeAction("CLICK", tuple())

        self.assertEqual(2, len(xChangesList.getChildren()))
        self.assertEqual(get_state_as_dict(xChangesList.getChild('0'))["Text"], "Accepted")
        self.assertEqual(get_state_as_dict(xChangesList.getChild('1'))["Text"], "Rejected")

        xChangesList.getChild('0').executeAction("EXPAND", tuple())
        self.assertEqual(2, len(xChangesList.getChild('0').getChildren()))

        textEnd3 = "(Cell A1 changed from 'Test LibreOffice' to '<empty>')"
        textEnd4 = "(Cell A2 changed from 'Test LibreOffice' to '<empty>')"
        self.assertTrue(get_state_as_dict(xChangesList.getChild('0').getChild('0'))["Text"].startswith(textStart))
        self.assertTrue(get_state_as_dict(xChangesList.getChild('0').getChild('0'))["Text"].endswith(textEnd3))
        self.assertTrue(get_state_as_dict(xChangesList.getChild('0').getChild('1'))["Text"].startswith(textStart2))
        self.assertTrue(get_state_as_dict(xChangesList.getChild('0').getChild('1'))["Text"].endswith(textEnd4))

        xChangesList.getChild('1').executeAction("EXPAND", tuple())
        self.assertEqual(2, len(xChangesList.getChild('1').getChildren()))

        self.assertTrue(get_state_as_dict(xChangesList.getChild('1').getChild('0'))["Text"].startswith(textStart))
        self.assertTrue(get_state_as_dict(xChangesList.getChild('1').getChild('0'))["Text"].endswith(textEnd))
        self.assertTrue(get_state_as_dict(xChangesList.getChild('1').getChild('1'))["Text"].startswith(textStart2))
        self.assertTrue(get_state_as_dict(xChangesList.getChild('1').getChild('1'))["Text"].endswith(textEnd2))

        xCancBtn = xTrackDlg.getChild("close")
        xCancBtn.executeAction("CLICK", tuple())

        self.assertEqual(get_cell_by_position(document, 0, 0, 0).getString(), "")
        self.ui_test.close_doc()

    def test_tracked_changes_rejectall(self):

        calc_doc = self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        #track changes;  enter text to cell
        self.xUITest.executeCommand(".uno:TraceChangeMode")
        enter_text_to_cell(gridwin, "A1", "Test LibreOffice")
        enter_text_to_cell(gridwin, "A2", "Test LibreOffice")
        #accept tracked changes
        self.ui_test.execute_modeless_dialog_through_command(".uno:AcceptChanges")
        xTrackDlg = self.xUITest.getTopFocusWindow()

        xChangesList = xTrackDlg.getChild("calcchanges")
        self.assertEqual(2, len(xChangesList.getChildren()))

        textStart = "Changed contents\tSheet1.A1\t \t" + datetime.datetime.now().strftime("%m/%d/%Y")
        textEnd = "(Cell A1 changed from '<empty>' to 'Test LibreOffice')"
        textStart2 = "Changed contents\tSheet1.A2\t \t" + datetime.datetime.now().strftime("%m/%d/%Y")
        textEnd2 = "(Cell A2 changed from '<empty>' to 'Test LibreOffice')"
        self.assertTrue(get_state_as_dict(xChangesList.getChild('0'))["Text"].startswith(textStart))
        self.assertTrue(get_state_as_dict(xChangesList.getChild('0'))["Text"].endswith(textEnd))
        self.assertTrue(get_state_as_dict(xChangesList.getChild('1'))["Text"].startswith(textStart2))
        self.assertTrue(get_state_as_dict(xChangesList.getChild('1'))["Text"].endswith(textEnd2))

        xAccBtn = xTrackDlg.getChild("rejectall")
        xAccBtn.executeAction("CLICK", tuple())

        self.assertEqual(2, len(xChangesList.getChildren()))
        self.assertEqual(get_state_as_dict(xChangesList.getChild('0'))["Text"], "Accepted")
        self.assertEqual(get_state_as_dict(xChangesList.getChild('1'))["Text"], "Rejected")

        xChangesList.getChild('0').executeAction("EXPAND", tuple())
        self.assertEqual(2, len(xChangesList.getChild('0').getChildren()))

        textEnd3 = "(Cell A1 changed from 'Test LibreOffice' to '<empty>')"
        textEnd4 = "(Cell A2 changed from 'Test LibreOffice' to '<empty>')"
        self.assertTrue(get_state_as_dict(xChangesList.getChild('0').getChild('0'))["Text"].startswith(textStart2))
        self.assertTrue(get_state_as_dict(xChangesList.getChild('0').getChild('0'))["Text"].endswith(textEnd4))
        self.assertTrue(get_state_as_dict(xChangesList.getChild('0').getChild('1'))["Text"].startswith(textStart))
        self.assertTrue(get_state_as_dict(xChangesList.getChild('0').getChild('1'))["Text"].endswith(textEnd3))

        xChangesList.getChild('1').executeAction("EXPAND", tuple())
        self.assertEqual(2, len(xChangesList.getChild('1').getChildren()))

        self.assertTrue(get_state_as_dict(xChangesList.getChild('1').getChild('0'))["Text"].startswith(textStart))
        self.assertTrue(get_state_as_dict(xChangesList.getChild('1').getChild('0'))["Text"].endswith(textEnd))
        self.assertTrue(get_state_as_dict(xChangesList.getChild('1').getChild('1'))["Text"].startswith(textStart2))
        self.assertTrue(get_state_as_dict(xChangesList.getChild('1').getChild('1'))["Text"].endswith(textEnd2))

        xCancBtn = xTrackDlg.getChild("close")
        xCancBtn.executeAction("CLICK", tuple())

        self.assertEqual(get_cell_by_position(document, 0, 0, 0).getString(), "")
        self.ui_test.close_doc()

    def test_tdf136062(self):

        self.ui_test.load_file(get_url_for_data_file("tdf136062.ods"))

        self.xUITest.getTopFocusWindow()

        self.ui_test.execute_modeless_dialog_through_command(".uno:AcceptChanges")
        xTrackDlg = self.xUITest.getTopFocusWindow()

        xChangesList = xTrackDlg.getChild("calcchanges")
        self.assertEqual(1, len(xChangesList.getChildren()))

        xRejectAllBtn = xTrackDlg.getChild("rejectall")
        xRejectBtn = xTrackDlg.getChild("reject")
        xAcceptAllBtn = xTrackDlg.getChild("acceptall")
        xAcceptBtn = xTrackDlg.getChild("accept")

        # Without the fix in place, it would have failed with
        # AssertionError: 'R~eject All' != 'R~eject All/Clear formatting'
        self.assertEqual('R~eject All', get_state_as_dict(xRejectAllBtn)['Text'])
        self.assertEqual('~Reject', get_state_as_dict(xRejectBtn)['Text'])
        self.assertEqual('A~ccept All', get_state_as_dict(xAcceptAllBtn)['Text'])
        self.assertEqual('~Accept', get_state_as_dict(xAcceptBtn)['Text'])

        xCancBtn = xTrackDlg.getChild("close")
        xCancBtn.executeAction("CLICK", tuple())

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
