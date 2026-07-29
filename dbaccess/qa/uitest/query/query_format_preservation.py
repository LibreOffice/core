# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# Regression test for tdf#42713 - SQL comments are preserved when working
# with queries containing comments in the Base query editor.
# Also covers tdf#46841 - a query's SQL formatting more generally (not just
# comments) is preserved when it was last saved from SQL View.

from uitest.framework import UITestCase
from uitest.uihelper.common import get_url_for_data_file, get_state_as_dict

from com.sun.star.awt import Toolkit


class QueryCommentPreservation(UITestCase):
    """Test that SQL comments in a query are preserved when opening."""

    def test_comment_preserved_when_opening(self):
        """A saved query with comments opens in SQL view with comments intact (tdf#42713)."""
        with self.ui_test.load_file(
            get_url_for_data_file("tdf42713.odb")
        ) as document:
            xToolkit = Toolkit.create(self.xContext)

            self.xUITest.executeCommand(".uno:DBViewQueries")
            xToolkit.waitUntilAllIdlesDispatched()

            # The file contains a single query ("q_with_comment"), so SelectAll
            # selects exactly that one entry and DBQueryEdit opens it.
            self.xUITest.executeCommand(".uno:SelectAll")

            with self.ui_test.open_subcomponent_through_command(
                ".uno:DBQueryEdit"
            ) as xQueryFrame:
                # Query opens in SQL view (forced due to comments, tdf#42713)
                xSql = self.xUITest.getTopFocusWindow().getChild("sql")
                sql_text = get_state_as_dict(xSql)["Text"]
                self.assertIn(
                    "-- my comment",
                    sql_text,
                    "SQL comment must be visible when opening query; got: "
                    + sql_text,
                )

    def test_format_preserved_when_last_edited_in_sql_view(self):
        """A query with no comments, but last saved from SQL View, opens in
        SQL View with its formatting intact instead of being 
        canonicalized by Design View (tdf#46841)."""
        with self.ui_test.load_file(
            get_url_for_data_file("tdf46841.odb")
        ) as document:
            xToolkit = Toolkit.create(self.xContext)

            self.xUITest.executeCommand(".uno:DBViewQueries")
            xToolkit.waitUntilAllIdlesDispatched()

            self.xUITest.executeCommand(".uno:SelectAll")

            with self.ui_test.open_subcomponent_through_command(
                ".uno:DBQueryEdit"
            ) as xQueryFrame:
                xSql = self.xUITest.getTopFocusWindow().getChild("sql")
                sql_text = get_state_as_dict(xSql)["Text"]
                self.assertEqual(
                    'SELECT "object"."description" \nFROM "object"',
                    sql_text,
                    "Query last saved from SQL View must open in SQL View "
                    "with its formatting intact; got: " + sql_text,
                )

# vim: set shiftwidth=4 softtabstop=4 expandtab:
