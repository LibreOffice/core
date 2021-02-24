# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file
from libreoffice.uno.propertyvalue import mkPropertyValues

class Forms(UITestCase):

    def test_tdf140486(self):

        self.ui_test.load_file(get_url_for_data_file("tdf140486.odt"))

        self.xUITest.executeCommand(".uno:JumpToNextFrame")

        self.ui_test.execute_modeless_dialog_through_command(".uno:ControlProperties")
        xChild = self.ui_test.wait_until_child_is_available('listbox-Empty string is NULL')

        # Without the fix in place, this test would have failed with
        # AssertionError: 'Yes' != 'No'
        self.assertEqual("Yes", get_state_as_dict(xChild)['SelectEntryText'])

        self.ui_test.close_doc()

    def test_tdf140198(self):

        self.ui_test.load_file(get_url_for_data_file("tdf140198.odt"))

        self.xUITest.executeCommand(".uno:JumpToNextFrame")

        self.ui_test.execute_modeless_dialog_through_command(".uno:ControlProperties")
        xChild = self.ui_test.wait_until_child_is_available('listbox-Text type')

        # Without the fix in place, this test would have failed with
        # AssertionError: 'Multi-line' != 'Single-line'
        self.assertEqual("Multi-line", get_state_as_dict(xChild)['SelectEntryText'])

        self.ui_test.close_doc()

    def test_tdf140239(self):

        self.ui_test.load_file(get_url_for_data_file("tdf140239.odt"))

        self.xUITest.executeCommand(".uno:JumpToNextFrame")

        self.ui_test.execute_modeless_dialog_through_command(".uno:ControlProperties")
        xAction = self.ui_test.wait_until_child_is_available('listbox-Action')
        xURL = self.ui_test.wait_until_child_is_available('urlcontrol-URL')

        self.assertEqual("None", get_state_as_dict(xAction)['SelectEntryText'])
        self.assertEqual("false", get_state_as_dict(xURL)['Enabled'])

        xAction.executeAction("SELECT", mkPropertyValues({"TEXT": "Open document/web page"}))

        self.assertEqual("Open document/web page", get_state_as_dict(xAction)['SelectEntryText'])
        self.assertEqual("true", get_state_as_dict(xURL)['Enabled'])

        xURL.executeAction("TYPE", mkPropertyValues({"TEXT": "1"}))
        xURL.executeAction("TYPE", mkPropertyValues({"TEXT": "2"}))
        xURL.executeAction("TYPE", mkPropertyValues({"TEXT": "3"}))
        xURL.executeAction("TYPE", mkPropertyValues({"TEXT": "4"}))
        xURL.executeAction("TYPE", mkPropertyValues({"TEXT": "5"}))

        # Without the fix in place, this test would have failed with
        # AssertionError: '12345' != '54321'
        self.assertEqual("12345", get_state_as_dict(xURL)['Text'])

        self.ui_test.close_doc()

    def test_tdf138701(self):

        # Reuse file from another test
        self.ui_test.load_file(get_url_for_data_file("tdf140198.odt"))

        self.xUITest.executeCommand(".uno:JumpToNextFrame")

        self.ui_test.execute_modeless_dialog_through_command(".uno:ControlProperties")
        xChild = self.ui_test.wait_until_child_is_available('combobox-Data field')

        xChild.executeAction("TYPE", mkPropertyValues({"TEXT": "1"}))
        xChild.executeAction("TYPE", mkPropertyValues({"TEXT": "2"}))
        xChild.executeAction("TYPE", mkPropertyValues({"TEXT": "3"}))
        xChild.executeAction("TYPE", mkPropertyValues({"TEXT": "4"}))
        xChild.executeAction("TYPE", mkPropertyValues({"TEXT": "5"}))

        # Without the fix in place, this test would have failed with
        # AssertionError: '12345' != '54321'
        self.assertEqual("12345", get_state_as_dict(xChild)['Text'])

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
