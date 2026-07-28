# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import type_text, mkPropertyValues

import tempfile
import contextlib


class ReadOnlyTest(UITestCase):
    def save_to_file(self, command, filename):
        with contextlib.ExitStack() as chooser_stack:
            xFileChooser = chooser_stack.enter_context(
                self.ui_test.execute_dialog_through_command(command, close_button="open"))
            xFilename = xFileChooser.getChild("file_name")
            xFilename.executeAction("TYPE", mkPropertyValues({"KEYCODE": "CTRL+A"}))
            type_text(xFilename, filename)
            # Choosing a filename will open an overwrite confirmation dialog
            with self.ui_test.execute_blocking_action(chooser_stack.close,
                                                      close_button="yes"):
                pass

    # Checks that saving a read-only file with SaveAs removes the read-only mode. See tdf#172199
    def test_save_as_becomes_editable(self):
        with contextlib.ExitStack() as stack:
            # Create a new writer document and save it to a temporary file
            xComponent = stack.enter_context(self.ui_test.create_doc_in_start_center("writer"))
            read_only_file = stack.enter_context(tempfile.NamedTemporaryFile(suffix=".odt"))
            self.save_to_file(".uno:Save", read_only_file.name)

            # Enable read-only mode
            self.xUITest.executeCommand(".uno:ReadOnlyDoc")

            # Make sure we can’t edit the document
            xWriterDoc = self.xUITest.getTopFocusWindow()
            xWriterEdit = xWriterDoc.getChild("writer_edit")
            type_text(xWriterEdit, "This text shouldn’t be inserted")
            self.assertEqual(xComponent.getText().getString(), "")

            # Save the document to another file
            writable_file = stack.enter_context(tempfile.NamedTemporaryFile(suffix=".odt"))
            self.save_to_file(".uno:SaveAs", writable_file.name)

            # Make sure we can now edit the document
            type_text(xWriterEdit, "This text should be inserted")
            self.assertEqual(xComponent.getText().getString(), "This text should be inserted")


# vim: set shiftwidth=4 softtabstop=4 expandtab:
