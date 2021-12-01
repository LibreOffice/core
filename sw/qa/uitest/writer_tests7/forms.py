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
from uitest.uihelper.common import change_measurement_unit
from uitest.uihelper.common import select_by_text
from libreoffice.uno.propertyvalue import mkPropertyValues

class Forms(UITestCase):

    def test_tdf140486(self):

        with self.ui_test.load_file(get_url_for_data_file("tdf140486.odt")):

            self.xUITest.executeCommand(".uno:JumpToNextFrame")

            with self.ui_test.execute_modeless_dialog_through_command(".uno:ControlProperties", close_button=""):
                xChild = self.ui_test.wait_until_child_is_available('listbox-Empty string is NULL')

                # Without the fix in place, this test would have failed with
                # AssertionError: 'Yes' != 'No'
                self.assertEqual("Yes", get_state_as_dict(xChild)['SelectEntryText'])

    def test_tdf140198(self):

        with self.ui_test.load_file(get_url_for_data_file("tdf140198.odt")):

            self.xUITest.executeCommand(".uno:JumpToNextFrame")

            with self.ui_test.execute_modeless_dialog_through_command(".uno:ControlProperties", close_button=""):
                xChild = self.ui_test.wait_until_child_is_available('listbox-Text type')

                # Without the fix in place, this test would have failed with
                # AssertionError: 'Multi-line' != 'Single-line'
                self.assertEqual("Multi-line", get_state_as_dict(xChild)['SelectEntryText'])

    def test_tdf141084(self):

        # Reuse document from tdf#140239
        with self.ui_test.load_file(get_url_for_data_file("tdf140239.odt")):

            with self.ui_test.execute_modeless_dialog_through_command(".uno:FormProperties", close_button=""):
                xURL = self.ui_test.wait_until_child_is_available('urlcontrol-URL')

                xURL.executeAction("TYPE", mkPropertyValues({"TEXT": "1"}))
                xURL.executeAction("TYPE", mkPropertyValues({"TEXT": "2"}))
                xURL.executeAction("TYPE", mkPropertyValues({"TEXT": "3"}))
                xURL.executeAction("TYPE", mkPropertyValues({"TEXT": "4"}))
                xURL.executeAction("TYPE", mkPropertyValues({"TEXT": "5"}))

                # Without the fix in place, this test would have failed with
                # AssertionError: '12345' != 'file:///tmp/tmp/5file:///tmp/tmp/4file://[40 chars]mp/1'
                self.assertEqual("12345", get_state_as_dict(xURL)['Text'])

    def test_tdf140239(self):

        with self.ui_test.load_file(get_url_for_data_file("tdf140239.odt")):

            self.xUITest.executeCommand(".uno:JumpToNextFrame")

            with self.ui_test.execute_modeless_dialog_through_command(".uno:ControlProperties", close_button=""):
                xAction = self.ui_test.wait_until_child_is_available('listbox-Action')
                xURL = self.ui_test.wait_until_child_is_available('urlcontrol-URL')
                xEntry = self.ui_test.wait_until_child_is_available('entry')

                self.assertEqual("None", get_state_as_dict(xAction)['SelectEntryText'])
                self.assertEqual("false", get_state_as_dict(xURL)['Enabled'])

                select_by_text(xAction, "Open document/web page")

                self.assertEqual("Open document/web page", get_state_as_dict(xAction)['SelectEntryText'])

                self.ui_test.wait_until_property_is_updated(xURL, "Enabled", "true")
                self.assertEqual("true", get_state_as_dict(xURL)['Enabled'])

                xURL.executeAction("TYPE", mkPropertyValues({"TEXT": "1"}))
                xURL.executeAction("TYPE", mkPropertyValues({"TEXT": "2"}))
                xURL.executeAction("TYPE", mkPropertyValues({"TEXT": "3"}))
                xURL.executeAction("TYPE", mkPropertyValues({"TEXT": "4"}))
                xURL.executeAction("TYPE", mkPropertyValues({"TEXT": "5"}))

                # Without the fix in place, this test would have failed with
                # AssertionError: '12345' != '54321'
                self.assertEqual("12345", get_state_as_dict(xURL)['Text'])

                xEntry.executeAction("FOCUS", tuple())
                self.assertEqual("Push Button", get_state_as_dict(xEntry)['Text'])

                xEntry.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
                xEntry.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
                xEntry.executeAction("TYPE", mkPropertyValues({"TEXT": "Push"}))

                # Move the focus to another element so the changes done before will take effect
                xAction.executeAction("FOCUS", tuple())

                # tdf#131522: Without the fix in place, this test would have failed with
                # AssertionError: 'Push' != 'Push Button'
                self.assertEqual("Push", get_state_as_dict(xEntry)['Text'])

    def test_tdf138701(self):

        # Reuse file from another test
        with self.ui_test.load_file(get_url_for_data_file("tdf140198.odt")):

            self.xUITest.executeCommand(".uno:JumpToNextFrame")

            with self.ui_test.execute_modeless_dialog_through_command(".uno:ControlProperties", close_button=""):
                xChild = self.ui_test.wait_until_child_is_available('combobox-Data field')

                xChild.executeAction("TYPE", mkPropertyValues({"TEXT": "1"}))
                xChild.executeAction("TYPE", mkPropertyValues({"TEXT": "2"}))
                xChild.executeAction("TYPE", mkPropertyValues({"TEXT": "3"}))
                xChild.executeAction("TYPE", mkPropertyValues({"TEXT": "4"}))
                xChild.executeAction("TYPE", mkPropertyValues({"TEXT": "5"}))

                # Without the fix in place, this test would have failed with
                # AssertionError: '12345' != '54321'
                self.assertEqual("12345", get_state_as_dict(xChild)['Text'])

    def test_tdf139486(self):

        # Reuse file from another test
        with self.ui_test.load_file(get_url_for_data_file("tdf140198.odt")) as document:

            change_measurement_unit(self, "Centimeter")

            self.xUITest.executeCommand(".uno:JumpToNextFrame")

            drawPage = document.getDrawPages().getByIndex(0)
            shape = drawPage.getByIndex(0)
            self.assertEqual(13996, shape.getSize().Width)
            self.assertEqual(2408, shape.getSize().Height)

            with self.ui_test.execute_modeless_dialog_through_command(".uno:ControlProperties", close_button=""):
                xWidth = self.ui_test.wait_until_child_is_available('numericfield-Width')
                xHeight = self.ui_test.wait_until_child_is_available('numericfield-Height')

                self.assertEqual("14.00 cm", get_state_as_dict(xWidth)['Text'])
                self.assertEqual("2.41 cm", get_state_as_dict(xHeight)['Text'])

                xWidth.executeAction("FOCUS", tuple())
                xWidth.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
                xWidth.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
                xWidth.executeAction("TYPE", mkPropertyValues({"TEXT":"20 cm"}))

                self.assertEqual("20 cm", get_state_as_dict(xWidth)['Text'])

                xHeight.executeAction("FOCUS", tuple())
                xHeight.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
                xHeight.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
                xHeight.executeAction("TYPE", mkPropertyValues({"TEXT":"5 cm"}))

                self.assertEqual("5 cm", get_state_as_dict(xHeight)['Text'])

                # Move the focus to another element so the changes done before take effect on the document
                xDialog = self.xUITest.getTopFocusWindow()
                xDialog.getChild('numericfield-PositionY').executeAction("FOCUS", tuple())

                xToolkit = self.xContext.ServiceManager.createInstance('com.sun.star.awt.Toolkit')
                xToolkit.processEventsToIdle()

                # Without the fix in place, the size of the form wouldn't have changed
                self.assertEqual(20001, shape.getSize().Width)
                self.assertEqual(5001, shape.getSize().Height)

    def test_tdf138271(self):

        with self.ui_test.load_file(get_url_for_data_file("tdf138271.odt")):

            self.xUITest.executeCommand(".uno:JumpToNextFrame")

            aOldValue = ["-1000000.00", "1000000.00"]
            aNewValue = ["-100.00", "100.00"]

            for i, name in enumerate(['formattedcontrol-Value min.', 'formattedcontrol-Value max.']):

                with self.ui_test.execute_modeless_dialog_through_command(".uno:ControlProperties", close_button=""):
                    xChild = self.ui_test.wait_until_child_is_available(name)

                    self.assertEqual(aOldValue[i], get_state_as_dict(xChild)['Text'])

                    xChild.executeAction("FOCUS", tuple())
                    xChild.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
                    xChild.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
                    xChild.executeAction("TYPE", mkPropertyValues({"TEXT": aNewValue[i]}))

                    #Close the dialog and open it again
                    self.xUITest.executeCommand(".uno:ControlProperties")

                with self.ui_test.execute_modeless_dialog_through_command(".uno:ControlProperties", close_button=""):
                    xChild = self.ui_test.wait_until_child_is_available(name)

                    # Without the fix in place, this test would have failed here because
                    # the values wouldn't have changed
                    self.assertEqual(aNewValue[i], get_state_as_dict(xChild)['Text'])

                    self.xUITest.executeCommand(".uno:ControlProperties")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
