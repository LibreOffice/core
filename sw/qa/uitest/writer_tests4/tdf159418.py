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

class tdf159418(UITestCase):

    def test_tdf159418(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf159418.odt")):

            with self.ui_test.execute_dialog_through_command(".uno:IndexEntryDialog", close_button="close") as xDialog:

                xEntry = xDialog.getChild("entryed")
                xNext = xDialog.getChild("next")
                xPrevious = xDialog.getChild("previous")

                # Without the fix in place, this test would have failed with
                # AssertionError: 'true' != 'false'
                self.assertEqual("true", get_state_as_dict(xNext)["Visible"])
                self.assertEqual("true", get_state_as_dict(xNext)["Enabled"])
                self.assertEqual("true", get_state_as_dict(xPrevious)["Visible"])
                self.assertEqual("false", get_state_as_dict(xPrevious)["Enabled"])
                self.assertEqual("Function", get_state_as_dict(xEntry)["Text"])

                xNext.executeAction("CLICK", tuple())

                self.assertEqual("true", get_state_as_dict(xNext)["Visible"])
                self.assertEqual("true", get_state_as_dict(xPrevious)["Visible"])
                self.assertEqual("true", get_state_as_dict(xPrevious)["Visible"])
                self.assertEqual("true", get_state_as_dict(xPrevious)["Enabled"])
                self.assertEqual("vector", get_state_as_dict(xEntry)["Text"])

                xPrevious.executeAction("CLICK", tuple())

                self.assertEqual("true", get_state_as_dict(xNext)["Visible"])
                self.assertEqual("true", get_state_as_dict(xNext)["Enabled"])
                self.assertEqual("true", get_state_as_dict(xPrevious)["Visible"])
                self.assertEqual("false", get_state_as_dict(xPrevious)["Enabled"])
                self.assertEqual("Function", get_state_as_dict(xEntry)["Text"])

# vim: set shiftwidth=4 softtabstop=4 expandtab:
