# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file

class tdf167128(UITestCase):

    def test_tdf167128(self):

        with self.ui_test.load_file(get_url_for_data_file("tdf167128.fodt")):

            with self.ui_test.execute_modeless_dialog_through_command(".uno:AcceptTrackedChanges", close_button="close") as xTrackDlg:
                changesList = xTrackDlg.getChild("writerchanges")

                self.assertEqual(3, len(changesList.getChildren()))
                self.assertEqual('true', get_state_as_dict(changesList.getChild('0'))["IsSelected"])
                self.assertEqual('false', get_state_as_dict(changesList.getChild('1'))["IsSelected"])
                self.assertEqual('false', get_state_as_dict(changesList.getChild('2'))["IsSelected"])

                # Without the fix in place, this test would have failed here with
                # AssertionError: 'true' != 'false'
                self.assertEqual('true', get_state_as_dict(xTrackDlg.getChild("accept"))["Enabled"])
                self.assertEqual('true', get_state_as_dict(xTrackDlg.getChild("acceptall"))["Enabled"])
                self.assertEqual('true', get_state_as_dict(xTrackDlg.getChild("reject"))["Enabled"])
                self.assertEqual('true', get_state_as_dict(xTrackDlg.getChild("rejectall"))["Enabled"])


# vim: set shiftwidth=4 softtabstop=4 expandtab:
