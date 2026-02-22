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
from libreoffice.uno.propertyvalue import mkPropertyValues

class tdf170701(UITestCase):

    def test_tdf170701_buttons_wrongly_enabled_for_inner_changes(self):

        with self.ui_test.load_file(get_url_for_data_file("tdf170701.docx")):

            with self.ui_test.execute_modeless_dialog_through_command(".uno:AcceptTrackedChanges", close_button="close") as xTrackDlg:
                changesList = xTrackDlg.getChild("writerchanges")

                self.assertEqual(3, len(changesList.getChildren()))
                self.assertEqual('true', get_state_as_dict(changesList.getChild('0'))["IsSelected"])
                self.assertEqual('false', get_state_as_dict(changesList.getChild('1'))["IsSelected"])
                self.assertEqual('false', get_state_as_dict(changesList.getChild('2'))["IsSelected"])

                self.assertEqual('0', get_state_as_dict(changesList.getChild('0'))["Children"])
                self.assertEqual('1', get_state_as_dict(changesList.getChild('1'))["Children"])
                self.assertEqual('0', get_state_as_dict(changesList.getChild('0'))["Children"])

                self.assertEqual('true', get_state_as_dict(xTrackDlg.getChild("accept"))["Enabled"])
                self.assertEqual('true', get_state_as_dict(xTrackDlg.getChild("acceptall"))["Enabled"])
                self.assertEqual('true', get_state_as_dict(xTrackDlg.getChild("reject"))["Enabled"])
                self.assertEqual('true', get_state_as_dict(xTrackDlg.getChild("rejectall"))["Enabled"])

                changesList.executeAction("TYPE", mkPropertyValues({"KEYCODE": "DOWN"}))

                self.assertEqual('false', get_state_as_dict(changesList.getChild('0'))["IsSelected"])
                self.assertEqual('true', get_state_as_dict(changesList.getChild('1'))["IsSelected"])
                self.assertEqual('false', get_state_as_dict(changesList.getChild('2'))["IsSelected"])

                changesList.executeAction("TYPE", mkPropertyValues({"KEYCODE": "RIGHT"}))
                changesList.executeAction("TYPE", mkPropertyValues({"KEYCODE": "DOWN"}))

                self.assertEqual('false', get_state_as_dict(changesList.getChild('0'))["IsSelected"])
                self.assertEqual('false', get_state_as_dict(changesList.getChild('1'))["IsSelected"])
                self.assertEqual('false', get_state_as_dict(changesList.getChild('2'))["IsSelected"])
                self.assertEqual('true', get_state_as_dict(changesList.getChild('1').getChild('0'))["IsSelected"])

                self.ui_test.wait_until_property_is_updated(xTrackDlg.getChild("accept"), "Enabled", "false")
                # Without the fix in place, this test would have failed here with
                # AssertionError: 'true' != 'false'
                self.assertEqual('false', get_state_as_dict(xTrackDlg.getChild("accept"))["Enabled"])
                self.assertEqual('false', get_state_as_dict(xTrackDlg.getChild("reject"))["Enabled"])
                self.assertEqual('true', get_state_as_dict(xTrackDlg.getChild("acceptall"))["Enabled"])
                self.assertEqual('true', get_state_as_dict(xTrackDlg.getChild("rejectall"))["Enabled"])

# vim: set shiftwidth=4 softtabstop=4 expandtab:
