# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_url_for_data_file, get_state_as_dict, select_by_text

import sys

#Bug 99619 - query design segfault on redoing an undone table join creation
class tdf99619(UITestCase):
    def test_tdf99619(self):
        # The sample file is an HSQLDB database with the two tables “person” and “object”. They both
        # have a text field and a primary key. The object table additionally has an person_id field
        # which is meant to reference an owner in the person table. There is also one query called
        # “object_owners” which contains the two tables but no join.
        with self.ui_test.load_file(get_url_for_data_file("tdf99619.odb")) as document:
            xDbWindow = self.xUITest.getTopFocusWindow()

            self.xUITest.executeCommand(".uno:DBViewQueries")
            # The above command is run asynchronously via an event on the main thread so we need to
            # wait until it is dispatched before executing the next command
            xToolkit = self.xContext.ServiceManager.createInstance('com.sun.star.awt.Toolkit')
            xToolkit.waitUntilAllIdlesDispatched()

            # We just want to select the single query in the list of queries but it seems difficult
            # to get access to the treeview window for it because there are multiple treeviews in a
            # hierarchy of windows with no name. However AppController handles SelectAll and that
            # does the trick
            self.xUITest.executeCommand(".uno:SelectAll")

            xDbFrame = self.ui_test.get_desktop().getCurrentFrame()

            with self.ui_test.open_subcomponent_through_command(".uno:DBQueryEdit") as xQueryFrame:
                xQueryController = xQueryFrame.getController()

                # Add a relation via the dialog
                with self.ui_test.execute_blocking_action(
                        self.xUITest.executeCommandForProvider,
                        args=(".uno:DBAddRelation", xQueryController)) as xDialog:

                    # Choose the two tables
                    select_by_text(xDialog.getChild("table1"), "object")
                    select_by_text(xDialog.getChild("table2"), "person")

                    # Set the join type
                    select_by_text(xDialog.getChild("type"), "Inner join")

                    # Use a natural join because it’s too difficult to manipulate the grid to select
                    # fields
                    xDialog.getChild("natural").executeAction("CLICK", tuple())

                # Undo the join
                self.xUITest.executeCommandForProvider(".uno:Undo", xQueryFrame)
                # Redo the join. This is where it crashes without any fixes to the bug
                self.xUITest.executeCommandForProvider(".uno:Redo", xQueryFrame)

                # Save the query. This only saves the query in memory
                # and doesn’t change the database file on disk
                self.xUITest.executeCommandForProvider(".uno:Save", xQueryFrame)

                # Switch to SQL mode
                self.xUITest.executeCommandForProvider(".uno:DBChangeDesignMode",
                                                       xQueryController)

                # Get the SQL source for the query
                xSql = self.xUITest.getTopFocusWindow().getChild("sql")
                query = get_state_as_dict(xSql)["Text"]

                # Make sure that the join is in the query
                if "NATURAL INNER JOIN" not in query:
                    print(f"Join missing from query: {query}", file=sys.stderr)
                self.assertTrue("NATURAL INNER JOIN" in query)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
