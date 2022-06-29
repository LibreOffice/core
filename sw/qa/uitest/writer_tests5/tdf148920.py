# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict

class tdf148920(UITestCase):

   def test_tdf148920(self):

        with self.ui_test.create_doc_in_start_center("writer"):

            self.xUITest.executeCommand(".uno:StyleApply?Style:string=Text%20body&FamilyName:string=ParagraphStyles")

            with self.ui_test.execute_dialog_through_command(".uno:EditStyle", close_button="cancel") as xDialog:
                xFilter = xDialog.getChild("filter")

                # Without the fix in place, this test would have failed with
                # AssertionError: 'true' != 'false'
                self.assertEqual("true", get_state_as_dict(xFilter)["Enabled"])

                # tdf#91035: Without the fix in place, this test would have failed with
                # AssertionError: 'All Styles' != 'Hidden Styles'
                self.assertEqual("All Styles", get_state_as_dict(xFilter)["SelectEntryText"])


# vim: set shiftwidth=4 softtabstop=4 expandtab:
