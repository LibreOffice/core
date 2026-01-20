# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import select_pos, select_by_text
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict
from com.sun.star.awt import KeyEvent
from com.sun.star.awt import Key
from com.sun.star.container import NoSuchElementException

def select_key(xShortcuts, key_name):
    for child in xShortcuts.getChildren():
        entry = xShortcuts.getChild(child)
        name = get_state_as_dict(entry)["Text"]

        if name.startswith(key_name):
            entry.executeAction("SELECT", tuple())
            break
    else:
        raise Exception(f"Couldn’t find key {key_name}")

def select_keyboard_tab(xDialog):
    xTabs = xDialog.getChild("tabcontrol")

    n_tabs = int(get_state_as_dict(xTabs)["PageCount"])

    for i in range(n_tabs):
        select_pos(xTabs, str(i))

        if get_state_as_dict(xTabs)["CurrPageTitle"] == "Keyboard":
            break
    else:
        raise Exception("Couldn’t find keyboard tab")

class Test(UITestCase):
    def assign_key(self, xDialog, scope, key_name, action):
        xAcceleratorPage = xDialog.getChild("AccelConfigPage")

        select_keyboard_tab(xDialog)

        if scope == "office":
            xAcceleratorPage.getChild("office").executeAction("CLICK", tuple())
        elif scope == "module":
            xAcceleratorPage.getChild("module").executeAction("CLICK", tuple())
        else:
            xAcceleratorPage.getChild("document").executeAction("CLICK", tuple())
            xScope = xAcceleratorPage.getChild("savein")
            select_by_text(xScope, scope)

        xShortcuts = xAcceleratorPage.getChild("shortcuts")
        select_key(xShortcuts, key_name)

        xSearchEntry = xAcceleratorPage.getChild("searchEntry")
        xSearchEntry.executeAction("FOCUS", tuple())
        xSearchEntry.executeAction("SET", mkPropertyValues({"TEXT": action}))

        # Focus something else so that the search will be applied
        # immediately without having to wait for the timer
        xShortcuts.executeAction("FOCUS", tuple())

        xChange = xAcceleratorPage.getChild("change")
        xChange.executeAction("CLICK", tuple())

    def test_scope(self):
        with self.ui_test.create_doc_in_start_center("writer") as xComponent:
            # Set a shortcut on the global scope
            with self.ui_test.execute_dialog_through_command(".uno:ConfigureDialog") as xDialog:
                self.assign_key(xDialog, "office", "F7", ".uno:Credits")

            # Check that the key made it into the global config
            xGlobalAccelCfg = self.xContext.ServiceManager.createInstance(
                'com.sun.star.ui.GlobalAcceleratorConfiguration')
            xKeyEvent = KeyEvent()
            xKeyEvent.KeyCode = Key.F7
            self.assertEqual(xGlobalAccelCfg.getCommandByKeyEvent(xKeyEvent), ".uno:Credits")

            # Set a shortcut on the module scope
            with self.ui_test.execute_dialog_through_command(".uno:ConfigureDialog") as xDialog:
                self.assign_key(xDialog, "module", "F8", ".uno:EditBookmark")

            # Check that the key made it into the module config
            xModuleAccelCfg = self.xContext.ServiceManager.createInstanceWithArguments(
                'com.sun.star.ui.ModuleAcceleratorConfiguration',
                ('com.sun.star.text.TextDocument',))
            xKeyEvent = KeyEvent()
            xKeyEvent.KeyCode = Key.F8
            self.assertEqual(xModuleAccelCfg.getCommandByKeyEvent(xKeyEvent), ".uno:EditBookmark")

            # Set a shortcut on the document scope
            with self.ui_test.execute_dialog_through_command(".uno:ConfigureDialog") as xDialog:
                self.assign_key(xDialog, "Untitled 1", "F9", ".uno:OptionsSecurityDialog")

            # Check that the key made it into the document config
            xDocAccelCfg = xComponent.getUIConfigurationManager().getShortCutManager()
            xKeyEvent = KeyEvent()
            xKeyEvent.KeyCode = Key.F9
            self.assertEqual(xDocAccelCfg.getCommandByKeyEvent(xKeyEvent),
                             ".uno:OptionsSecurityDialog")

    def test_scope_multi_doc(self):
        # Create two writer documents and a calc document. Only the
        # writer documents should appear in the scope combobox
        with self.ui_test.create_doc_in_start_center("writer"), \
             self.ui_test.load_empty_file("calc"), \
             self.ui_test.load_empty_file("writer"), \
             self.ui_test.execute_dialog_through_command(".uno:ConfigureDialog") as xDialog:
            xAcceleratorPage = xDialog.getChild("AccelConfigPage")

            select_keyboard_tab(xDialog)
            xAcceleratorPage.getChild("document").executeAction("CLICK", tuple())

            xScope = xAcceleratorPage.getChild("savein")

            self.assertEqual(get_state_as_dict(xScope)["EntryCount"], "2")

            # The current document should be listed first
            select_pos(xScope, "0")
            self.assertEqual(get_state_as_dict(xScope)["SelectEntryText"], "Untitled 3")

            select_pos(xScope, "1")
            self.assertEqual(get_state_as_dict(xScope)["SelectEntryText"], "Untitled 1")

    def test_deleted_doc(self):
        # Create two writer docs then try to assign a key in the first
        # one but close the document before pressing ok.
        with self.ui_test.create_doc_in_start_center("writer") as xDoc1, \
             self.ui_test.load_empty_file("writer"), \
             self.ui_test.execute_dialog_through_command(".uno:ConfigureDialog") as xDialog:
            self.assign_key(xDialog, "Untitled 1", "F7", ".uno:Credits")
            xFrame1 = xDoc1.getCurrentController().getFrame()
            self.xUITest.executeCommandForProvider(".uno:CloseDoc", xFrame1)

            xAcceleratorPage = xDialog.getChild("AccelConfigPage")
            xScope = xAcceleratorPage.getChild("savein")
            self.assertEqual(get_state_as_dict(xScope)["SelectEntryText"], "Untitled 2")

    def test_reset_doc(self):
        with self.ui_test.create_doc_in_start_center("writer") as xComponent:
            with self.ui_test.execute_dialog_through_command(".uno:ConfigureDialog") as xDialog:
                # Assign a key
                self.assign_key(xDialog, "Untitled 1", "F7", ".uno:Credits")
                # … but then press reset before pressing OK
                xReset = xDialog.getChild("AccelConfigPage").getChild("reset")
                xReset.executeAction("CLICK", tuple())

            # Make sure the key isn’t assigned
            xDocAccelCfg = xComponent.getUIConfigurationManager().getShortCutManager()
            xKeyEvent = KeyEvent()
            xKeyEvent.KeyCode = Key.F7
            with self.assertRaises(NoSuchElementException):
                xDocAccelCfg.getCommandByKeyEvent(xKeyEvent)

    def test_savein_enabled(self):
        with self.ui_test.create_doc_in_start_center("writer") as xComponent:
            with self.ui_test.execute_dialog_through_command(".uno:ConfigureDialog") as xDialog:
                select_keyboard_tab(xDialog)

                xAcceleratorPage = xDialog.getChild("AccelConfigPage")
                xScope = xAcceleratorPage.getChild("savein")

                # The combobox should be initially disabled
                self.assertEqual(get_state_as_dict(xScope)["Enabled"], "false")

                # Check that selecting any of the other radio buttons makes the save-in combobox be
                # disabled
                xAcceleratorPage.getChild("office").executeAction("CLICK", tuple())
                self.assertEqual(get_state_as_dict(xScope)["Enabled"], "false")
                xAcceleratorPage.getChild("module").executeAction("CLICK", tuple())
                self.assertEqual(get_state_as_dict(xScope)["Enabled"], "false")

                # It should be enabled when the document radio button is selected
                xAcceleratorPage.getChild("document").executeAction("CLICK", tuple())
                self.assertEqual(get_state_as_dict(xScope)["Enabled"], "true")

                # … and disabled again if something else is selected
                xAcceleratorPage.getChild("office").executeAction("CLICK", tuple())
                self.assertEqual(get_state_as_dict(xScope)["Enabled"], "false")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
