# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import type_text, get_state_as_dict

class tdf124088(UITestCase):

    def test_tdf124088_propose_autotext(self):
        with self.ui_test.create_doc_in_start_center("writer"):
            # Insert a random text greater than 25 characters and select it
            xWriterDoc = self.xUITest.getTopFocusWindow()
            xWriterEdit = xWriterDoc.getChild("writer_edit")
            type_text(xWriterEdit, "This is a sample paragraph")
            self.xUITest.executeCommand(".uno:SelectAll")

            # Open auto text dialog and check proposed text
            with self.ui_test.execute_dialog_through_command(".uno:EditGlossary") as xEditGlossaryDlg:
                xName = xEditGlossaryDlg.getChild("name")
                self.assertEqual(get_state_as_dict(xName)["Text"], "This is a sample")

                xShortName = xEditGlossaryDlg.getChild("shortname")
                self.assertEqual("Tias", get_state_as_dict(xShortName)["Text"])

                # Click on a group entry, proposed auto text should remain
                xCategory = xEditGlossaryDlg.getChild("category")
                xMyAutoText = xCategory.getChild("2")
                xMyAutoText.executeAction("SELECT", tuple())
                self.assertEqual(get_state_as_dict(xName)["Text"], "This is a sample")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
