# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, select_by_text
from uitest.test import DEFAULT_SLEEP
from libreoffice.uno.propertyvalue import mkPropertyValues
from libreoffice.uno.eventlistener import EventListener

import time

class InsertRelation(UITestCase):
    # Previously the Insert Relation dialog wouldn’t let you select a table in one box if the other
    # box already has it selected. This test checks the new behaviour where you can select any of
    # the three tables and the other table changes itself so that it’s not the same.
    def test_choose_between_three_tables(self):
        with self.ui_test.create_db_in_start_center() as xDocument:
            # Create three tables using the design view
            for table_num in range(3):
                with self.ui_test.open_subcomponent_through_command(
                        ".uno:DBNewTable", close_win=False) as xTableFrame:

                    # Press TAB in the table editor and then type in a field name
                    xTableWindow = self.xUITest.getWindow(xTableFrame.getContainerWindow())
                    xTableEditor = xTableWindow.getChild("DBTableEditor")
                    xTableEditor.executeAction("TYPE", mkPropertyValues({"KEYCODE": "TAB"}))
                    self.xUITest.getFocusWindow().executeAction(
                        "SET", mkPropertyValues({"TEXT": f"field{table_num + 1}"}))

                    # Close the window. This will open a dialog asking if we want to save
                    with self.ui_test.execute_blocking_action(
                            self.xUITest.executeCommandForProvider,
                            args=(".uno:CloseWin", xTableFrame),
                            close_button=None) as xSaveDialog:
                        # Choose yes. This will open another dialog asking us to name the table
                        with self.ui_test.execute_blocking_action(
                                xSaveDialog.getChild("yes").executeAction,
                                args=("CLICK", tuple()),
                                close_button=None) as xNameDialog:
                            xNameDialog.getChild("title").executeAction(
                                "SET", mkPropertyValues({"TEXT": f"table{table_num + 1}"}))
                            # Clicking OK will open a third dialog
                            # asking if we want to add a primary key
                            with self.ui_test.execute_blocking_action(
                                    xNameDialog.getChild("ok").executeAction,
                                    args=("CLICK", tuple()),
                                    close_button="yes"):
                                pass

            # Create a new query with the design view. This will open a new frame for the query as
            # well as a modeless dialog to add the tables
            with EventListener(self.xContext, "DialogExecute") as event:
                with self.ui_test.open_subcomponent_through_command(
                        ".uno:DBNewQuery") as xQueryFrame:
                    while not event.executed:
                        time.sleep(DEFAULT_SLEEP)
                    try:
                        xAddTablesDialog = self.xUITest.getTopFocusWindow()
                        xTableList = xAddTablesDialog.getChild("tablelist")
                        xAdd = xAddTablesDialog.getChild("add")
                        # Select and add all of the tables in turn
                        for i in range(3):
                            xTableList.getChild(str(i)).executeAction("SELECT", tuple())
                            xAdd.executeAction("CLICK", tuple())
                    finally:
                        self.ui_test.close_dialog_through_button(
                            xAddTablesDialog.getChild("close"), xAddTablesDialog)

                    xQueryController = xQueryFrame.getController()

                    with self.ui_test.execute_blocking_action(
                            self.xUITest.executeCommandForProvider,
                            args=(".uno:DBAddRelation", xQueryController),
                            close_button="cancel") as xDialog:
                        xTable1 = xDialog.getChild("table1")
                        xTable2 = xDialog.getChild("table2")

                        table2_value = get_state_as_dict(xTable2)["SelectEntryText"]

                        # Try setting table1 to the same value as table2
                        select_by_text(xTable1, table2_value)
                        # Make sure that it worked
                        self.assertEqual(get_state_as_dict(xTable1)["SelectEntryText"],
                                         table2_value)
                        self.assertTrue(get_state_as_dict(xTable2)["SelectEntryText"] !=
                                        table2_value)

                        # Try choosing all 3 tables for table1
                        for i in range(3):
                            table_name = f"table{i + 1}"
                            select_by_text(xTable1, table_name)
                            self.assertEqual(get_state_as_dict(xTable1)["SelectEntryText"],
                                             table_name)
                            self.assertTrue(get_state_as_dict(xTable2)["SelectEntryText"] !=
                                            table_name)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
