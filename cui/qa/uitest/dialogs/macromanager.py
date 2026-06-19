# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from libreoffice.uno.eventlistener import EventListener
from uitest.framework import UITestCase
from uitest.test import DEFAULT_SLEEP
from uitest.uihelper.common import get_state_as_dict, type_text, mkPropertyValues

import time


class MacroManagerTest(UITestCase):
    def select_tree_node(self, xNode, *parts):
        for i, part in enumerate(parts):
            if i > 0:
                xNode.executeAction("EXPAND", tuple())

            # Find the node in the tree with this part name
            try:
                xNode = next(x for x in map(lambda name: xNode.getChild(name),
                                            xNode.getChildren())
                             if get_state_as_dict(x)["Text"] == part)
            except StopIteration:
                self.fail(f"Couldn’t find tree child {part}")

            xNode.executeAction("SELECT", tuple())

        return xNode

    def test_delete(self):
        # Create a new BASIC library using the macro manager
        with EventListener(self.xContext, "OnNew") as event:
            with self.ui_test.execute_dialog_through_command(".uno:MacroManager",
                                                             close_button=
                                                             "librarymoduledialogedit") \
                                                             as xDialog:
                xContainers = xDialog.getChild("scriptcontainers")
                xStandard = self.select_tree_node(xContainers, "My Macros", "Basic", "Standard")

                # Delete all other modules in the library. If we don’t do this then the BASIC editor
                # will create an EditorWindow for every module and we won’t be able get access to
                # the one we want.
                xDelete = xDialog.getChild("librarymoduledialogdelete")
                xStandard.executeAction("EXPAND", tuple())
                while get_state_as_dict(xStandard)["Children"] != '0':
                    xStandard.getChild('0').executeAction("SELECT", tuple())
                    with self.ui_test.execute_blocking_action(xDelete.executeAction,
                                                              args=("CLICK", tuple()),
                                                              close_button="yes") as xConfirm:
                        pass

                xNewModule = xDialog.getChild("newmodule")
                with self.ui_test.execute_blocking_action(xNewModule.executeAction,
                                                          args=("CLICK", tuple()),
                                                          close_button="ok") as xConfirm:
                    xNameEntry = xConfirm.getChild("name_entry")
                    type_text(xNameEntry, "TestDelete")

                self.select_tree_node(xContainers, "My Macros", "Basic", "Standard", "TestDelete")

            while not event.executed:
                time.sleep(DEFAULT_SLEEP)

        self.xUITest.executeCommand(".uno:SelectAll")

        xEditorWindow = self.xUITest.getTopFocusWindow().getChild("EditorWindow")

        code = "\n\n".join(map(lambda x: f"Sub {x}\nEnd Sub", ["first", "middle", "last"]))

        for line in code.splitlines():
            type_text(xEditorWindow, line)
            xEditorWindow.executeAction("TYPE", mkPropertyValues({"KEYCODE": "RETURN"}))

        text = get_state_as_dict(xEditorWindow)["Text"]
        self.assertFalse(text.find("Sub middle") == -1)

        # Open the macro manager
        with self.ui_test.execute_dialog_through_command(".uno:MacroManager",
                                                         close_button="close") as xDialog:
            xContainers = xDialog.getChild("scriptcontainers")
            self.select_tree_node(xContainers, "My Macros", "Basic", "Standard", "TestDelete")

            xScripts = xDialog.getChild("scripts")
            self.select_tree_node(xScripts, "middle")

            # Delete the middle macro
            xDelete = xDialog.getChild("macrodelete")
            with self.ui_test.execute_blocking_action(xDelete.executeAction, ("CLICK", tuple()),
                                                      close_button="yes"):
                pass

            # Make sure the macro isn’t in the list any more
            xScripts = xDialog.getChild("scripts")
            self.assertEqual(get_state_as_dict(xScripts)["Children"], '2')
            self.assertEqual(get_state_as_dict(xScripts.getChild(0))["Text"], "first")
            self.assertEqual(get_state_as_dict(xScripts.getChild(1))["Text"], "last")

        # Make sure the macro has disappeared from the macro editor source code
        text = get_state_as_dict(xEditorWindow)["Text"]
        self.assertEqual(text.find("Sub middle"), -1)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
