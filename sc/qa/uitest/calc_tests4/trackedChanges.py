# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# tests for tracked changes ; tdf912270

from uitest.framework import UITestCase
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.calc import enter_text_to_cell
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file
import datetime

class CalcTrackedChanges(UITestCase):

    def test_tdf131907(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf131907.ods")):
            with self.ui_test.execute_modeless_dialog_through_command(".uno:AcceptChanges", close_button="close") as xTrackDlg:

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


    def test_tdf66263_Protect_Records(self):
        with self.ui_test.create_doc_in_start_center("calc"):
            self.ui_test.wait_until_child_is_available("grid_window")
            self.xUITest.executeCommand(".uno:TraceChangeMode")
            #protect dialog
            with self.ui_test.execute_dialog_through_command(".uno:ProtectTraceChangeMode") as xDialog:
                xpass = xDialog.getChild("pass1ed")
                xpass.executeAction("TYPE", mkPropertyValues({"TEXT":"a"}))
                xconfirm = xDialog.getChild("confirm1ed")
                xconfirm.executeAction("TYPE", mkPropertyValues({"TEXT":"a"}))
            #verify password
            with self.ui_test.execute_dialog_through_command(".uno:ProtectTraceChangeMode") as xDialog:
                xpass = xDialog.getChild("pass1ed")
                xpass.executeAction("TYPE", mkPropertyValues({"TEXT":"a"}))


    def test_tracked_changes_accept(self):

        with self.ui_test.create_doc_in_start_center("calc") as document:
            self.ui_test.wait_until_child_is_available("grid_window")
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")
            #track changes;  enter text to cell
            self.xUITest.executeCommand(".uno:TraceChangeMode")
            enter_text_to_cell(gridwin, "A1", "Test LibreOffice")
            enter_text_to_cell(gridwin, "A2", "Test LibreOffice")
            #accept tracked changes
            with self.ui_test.execute_modeless_dialog_through_command(".uno:AcceptChanges", close_button="close") as xTrackDlg:

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


            self.assertEqual(get_cell_by_position(document, 0, 0, 0).getString(), "Test LibreOffice")

    def test_tracked_changes_acceptall(self):

        with self.ui_test.create_doc_in_start_center("calc") as document:
            self.ui_test.wait_until_child_is_available("grid_window")
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")
            #track changes;  enter text to cell
            self.xUITest.executeCommand(".uno:TraceChangeMode")
            enter_text_to_cell(gridwin, "A1", "Test LibreOffice")
            enter_text_to_cell(gridwin, "A2", "Test LibreOffice")
            #accept All tracked changes
            with self.ui_test.execute_modeless_dialog_through_command(".uno:AcceptChanges", close_button="close") as xTrackDlg:

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


            self.assertEqual(get_cell_by_position(document, 0, 0, 0).getString(), "Test LibreOffice")

    def test_tracked_changes_reject(self):

        with self.ui_test.create_doc_in_start_center("calc") as document:
            self.ui_test.wait_until_child_is_available("grid_window")
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")
            #track changes;  enter text to cell
            self.xUITest.executeCommand(".uno:TraceChangeMode")
            enter_text_to_cell(gridwin, "A1", "Test LibreOffice")
            enter_text_to_cell(gridwin, "A2", "Test LibreOffice")
            #accept tracked changes
            with self.ui_test.execute_modeless_dialog_through_command(".uno:AcceptChanges", close_button="close") as xTrackDlg:

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


            self.assertEqual(get_cell_by_position(document, 0, 0, 0).getString(), "")

    def test_tracked_changes_rejectall(self):

        with self.ui_test.create_doc_in_start_center("calc") as document:
            self.ui_test.wait_until_child_is_available("grid_window")
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")
            #track changes;  enter text to cell
            self.xUITest.executeCommand(".uno:TraceChangeMode")
            enter_text_to_cell(gridwin, "A1", "Test LibreOffice")
            enter_text_to_cell(gridwin, "A2", "Test LibreOffice")
            #accept tracked changes
            with self.ui_test.execute_modeless_dialog_through_command(".uno:AcceptChanges", close_button="close") as xTrackDlg:

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


            self.assertEqual(get_cell_by_position(document, 0, 0, 0).getString(), "")

    def test_tdf136062(self):

        with self.ui_test.load_file(get_url_for_data_file("tdf136062.ods")):

            self.xUITest.getTopFocusWindow()

            with self.ui_test.execute_modeless_dialog_through_command(".uno:AcceptChanges", close_button="close") as xTrackDlg:

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

    def test_tdf85353(self):
        with self.ui_test.create_doc_in_start_center("calc"):
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")

            enter_text_to_cell(gridwin, "A1", "15")
            enter_text_to_cell(gridwin, "D1", "0")
            enter_text_to_cell(gridwin, "E1", "0")

            with self.ui_test.execute_dialog_through_command(".uno:CompareDocuments", close_button="") as xOpenDialog:
                xFileName = xOpenDialog.getChild("file_name")
                xFileName.executeAction("TYPE", mkPropertyValues({"TEXT": get_url_for_data_file("tdf85353.ods")}))
                xOpenBtn = xOpenDialog.getChild("open")

                with self.ui_test.execute_dialog_through_action(xOpenBtn, 'CLICK', close_button="close") as xTrackDlg:
                    changesList = xTrackDlg.getChild("calcchanges")

                    # Without the fix in place, this test would have failed with
                    # AssertionError: 1 != 0
                    self.assertEqual(1, len(changesList.getChildren()))
                    self.assertTrue(get_state_as_dict(changesList.getChild('0'))['Text'].startswith("Changed contents\tSheet1.E1"))
                    self.assertTrue(get_state_as_dict(changesList.getChild('0'))['Text'].endswith("(Cell E1 changed from '5' to '0')"))

# vim: set shiftwidth=4 softtabstop=4 expandtab:
