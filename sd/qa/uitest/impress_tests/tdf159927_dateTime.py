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

class tdf159927(UITestCase):

   def test_tdf159927(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf159927_dateTime.odp")) as document:

            with self.ui_test.execute_dialog_through_command(".uno:HeaderAndFooter", close_button="cancel") as xDialog:
                language_list = xDialog.getChild("language_list")
                # Expected: the existing language of the date/time field: Finnish
                # Actual without the fix: the user's locale as specified in tools - options.
                self.assertEqual(get_state_as_dict(language_list)["SelectEntryText"], "Finnish")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
