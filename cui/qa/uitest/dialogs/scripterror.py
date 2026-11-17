# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file

import contextlib


class ScriptErrorTest(UITestCase):
    def test_missing_script(self):
        with contextlib.ExitStack() as stack:
            # Temporarily disable macro security
            stack.enter_context(self.ui_test.set_config(
                "/org.openoffice.Office.Common/Security/Scripting/"
                "MacroSecurityLevel", 0))
            # Open a document containing a button with an event pointing to a non-existant macro.
            # This should cause the error dialog to display when clicked
            stack.enter_context(self.ui_test.load_file(get_url_for_data_file(
                "button-with-missing-macro.fodt")))

            # Find the button
            writer_window = self.xUITest.getTopFocusWindow()
            writer_edit = writer_window.getChild("writer_edit")
            # The button should be the only child of writer_edit. It has no name
            button = writer_edit.getChild("")

            with self.ui_test.execute_dialog_through_action(button, "CLICK") as dialog:
                label = dialog.getChild("PrimaryMessage")
                self.assertEqual(get_state_as_dict(label)["Text"],
                                 "The Basic macro Standard.Module1.NotFound could not be "
                                 "found. The macro is expected to be located at:\n"
                                 "\n"
                                 "Location: application\n"
                                 "Library: Standard\n"
                                 "Module: Module1\n"
                                 "Macro name: NotFound")


# vim: set shiftwidth=4 softtabstop=4 expandtab:
