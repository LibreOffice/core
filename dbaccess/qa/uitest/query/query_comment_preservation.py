# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# Regression test for tdf#42713 - SQL comments were lost when switching
# between Design view and SQL view in the Base query editor.

from uitest.framework import UITestCase
from uitest.uihelper.common import get_url_for_data_file, get_state_as_dict

from com.sun.star.awt import Toolkit

class QueryCommentPreservation(UITestCase):
    """Test that SQL comments in a query are preserved across Design/SQL view toggles."""

    def _open_comment_query(self):
        """Navigate to the Queries section and open q_with_comment for editing.

        Returns xToolkit. The frame is already in Design view because that is the
        default open mode.
        """
        xToolkit = Toolkit.create(self.xContext)

        self.xUITest.executeCommand(".uno:DBViewQueries")
        xToolkit.waitUntilAllIdlesDispatched()

        # The file contains a single query ("q_with_comment"), so SelectAll
        # selects exactly that one entry and DBQueryEdit opens it.
        self.xUITest.executeCommand(".uno:SelectAll")

        return xToolkit

    def _get_sql_text(self, xQueryController, xToolkit):
        """Switch to SQL view, return the text, then switch back to Design view.

        The caller is responsible for the final view state if needed.
        """
        self.xUITest.executeCommandForProvider(".uno:DBChangeDesignMode", xQueryController)
        xToolkit.waitUntilAllIdlesDispatched()
        xSql = self.xUITest.getTopFocusWindow().getChild("sql")
        return get_state_as_dict(xSql)["Text"]

    def test_comment_preserved_design_to_sql(self):
        """A query that starts in Design view keeps its SQL comment when toggled to SQL view."""
        with self.ui_test.load_file(get_url_for_data_file("tdf42713.odb")) as document:
            xToolkit = self._open_comment_query()

            with self.ui_test.open_subcomponent_through_command(".uno:DBQueryEdit") as xQueryFrame:
                xQueryController = xQueryFrame.getController()

                # Toggle Design -> SQL; the comment must survive the round-trip
                # through the graphical query builder.
                sql_text = self._get_sql_text(xQueryController, xToolkit)
                self.assertIn("-- my comment", sql_text,
                              "SQL comment must be visible after Design->SQL toggle; got: "
                              + sql_text)

    def test_comment_preserved_after_round_trip(self):
        """A SQL comment survives a full Design->SQL->Design->SQL round-trip."""
        with self.ui_test.load_file(get_url_for_data_file("tdf42713.odb")) as document:
            xToolkit = self._open_comment_query()

            with self.ui_test.open_subcomponent_through_command(".uno:DBQueryEdit") as xQueryFrame:
                xQueryController = xQueryFrame.getController()

                # 1st toggle: Design -> SQL
                after_first = self._get_sql_text(xQueryController, xToolkit)
                self.assertIn("-- my comment", after_first,
                              "Comment missing after first Design->SQL toggle; got: "
                              + after_first)

                # 2nd toggle: SQL -> Design (switch back)
                self.xUITest.executeCommandForProvider(".uno:DBChangeDesignMode", xQueryController)
                xToolkit.waitUntilAllIdlesDispatched()

                # 3rd toggle: Design -> SQL (comment must still be present)
                after_round_trip = self._get_sql_text(xQueryController, xToolkit)
                self.assertIn("-- my comment", after_round_trip,
                              "Comment lost after Design->SQL->Design->SQL round-trip; got: "
                              + after_round_trip)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
