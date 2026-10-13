# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, select_pos, select_by_text


class tdf173028(UITestCase):
    def test_remember_attributes_after_closing_document(self):
        for position in ("0", "1", "2"):
            with self.ui_test.load_empty_file("writer"):
                with self.ui_test.execute_modeless_dialog_through_command(
                        ".uno:SearchDialog", close_button="close") as dialog:
                    dialog.getChild("noformat").executeAction("CLICK", tuple())
                    with self.ui_test.execute_blocking_action(
                            dialog.getChild("attributes").executeAction,
                            args=("CLICK", tuple())) as attributes:
                        listing = attributes.getChild("treeview")
                        entry = listing.getChild(position)
                        entry.executeAction("CLICK", tuple())
                        self.assertEqual("true", get_state_as_dict(entry)["IsChecked"])
                    description = get_state_as_dict(dialog.getChild("searchdesc"))["Text"]
                    self.assertNotEqual("", description)
                    dialog.getChild("search").executeAction("CLICK", tuple())

            # The remembered attribute holder must survive destruction of the
            # first document's item pool, while retaining the search criterion.
            with self.ui_test.load_empty_file("writer"):
                with self.ui_test.execute_modeless_dialog_through_command(
                        ".uno:SearchDialog", close_button="close") as dialog:
                    self.assertEqual(description,
                                     get_state_as_dict(dialog.getChild("searchdesc"))["Text"])

    def test_remember_format_after_closing_document(self):
        with self.ui_test.load_empty_file("writer"):
            with self.ui_test.execute_modeless_dialog_through_command(
                    ".uno:SearchDialog", close_button="close") as dialog:
                dialog.getChild("noformat").executeAction("CLICK", tuple())
                with self.ui_test.execute_blocking_action(
                        dialog.getChild("format").executeAction,
                        args=("CLICK", tuple())) as formatting:
                    select_pos(formatting.getChild("tabcontrol"), "1")
                    select_by_text(formatting.getChild("underlinelb"), "Single")
                self.assertEqual("Single underline",
                                 get_state_as_dict(dialog.getChild("searchdesc"))["Text"])
                dialog.getChild("search").executeAction("CLICK", tuple())

        with self.ui_test.load_empty_file("writer"):
            with self.ui_test.execute_modeless_dialog_through_command(
                    ".uno:SearchDialog", close_button="close") as dialog:
                self.assertEqual("Single underline",
                                 get_state_as_dict(dialog.getChild("searchdesc"))["Text"])

# vim: set shiftwidth=4 softtabstop=4 expandtab:
