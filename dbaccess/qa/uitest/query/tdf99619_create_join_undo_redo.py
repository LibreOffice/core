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
from uitest.test import DEFAULT_SLEEP

from com.sun.star.util import URL

import sys
import time

#Bug 99619 - query design segfault on redoing an undone table join creation
class tdf99619(UITestCase):
    def execute_for_provider(self, xProvider, command):
        url = URL()
        url.Complete = command
        xUrlTransformer = self.xContext.ServiceManager.createInstanceWithContext(
            "com.sun.star.util.URLTransformer", self.xContext)
        _, url = xUrlTransformer.parseStrict(url)

        xDispatch = xProvider.queryDispatch(url, "", 0)
        xDispatch.dispatch(url, [])

    def test_tdf99619(self):
        # The sample file is an HSQLDB database with the two tables “person” and “object”. They both
        # have a text field and a primary key. The object table additionally has an person_id field
        # which is meant to reference an owner in the person table. There is also one query called
        # “object_owners” which contains the two tables but no join.
        with self.ui_test.load_file(get_url_for_data_file("tdf99619.odb")) as document:
            xDbWindow = self.xUITest.getTopFocusWindow()

            self.xUITest.executeCommand(".uno:DBViewQueries")

            # We just want to select the single query in the list of queries but it seems difficult
            # to get access to the treeview window for it because there are multiple treeviews in a
            # hierarchy of windows with no name. However AppController handles SelectAll and that
            # does the trick
            self.xUITest.executeCommand(".uno:SelectAll")

            xDbFrame = self.ui_test.get_desktop().getCurrentFrame()

            self.xUITest.executeCommand(".uno:DBQueryEdit")

            while True:
                xQueryFrame = self.ui_test.get_desktop().getCurrentFrame()

                if xQueryFrame != xDbFrame:
                    break
                time.sleep(DEFAULT_SLEEP)

            xQueryController = xQueryFrame.getController()

            # Add a relation via the dialog
            with self.ui_test.execute_blocking_action(
                    self.execute_for_provider,
                    args=(xQueryController, ".uno:DBAddRelation")) as xDialog:

                # Choose the two tables
                select_by_text(xDialog.getChild("table1"), "object")
                select_by_text(xDialog.getChild("table2"), "person")

                # Set the join type
                select_by_text(xDialog.getChild("type"), "Inner join")

                # Use a natural join because it’s too difficult to manipulate the grid to select
                # fields
                xDialog.getChild("natural").executeAction("CLICK", tuple())

            # Undo the join
            self.execute_for_provider(xQueryFrame, ".uno:Undo")
            # Redo the join. This is where it crashes without any fixes to the bug
            self.execute_for_provider(xQueryFrame, ".uno:Redo")

            # Save the query. This only saves the query in memory and doesn’t change the database
            # file on disk
            self.execute_for_provider(xQueryFrame, ".uno:Save")

            # Switch to SQL mode
            self.execute_for_provider(xQueryController, ".uno:DBChangeDesignMode")

            # Get the SQL source for the query
            xSql = self.xUITest.getTopFocusWindow().getChild("sql")
            query = get_state_as_dict(xSql)["Text"]

            # Make sure that the join is in the query
            if "NATURAL INNER JOIN" not in query:
                print(f"Join missing from query: {query}", file=sys.stderr)
            self.assertTrue("NATURAL INNER JOIN" in query)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
