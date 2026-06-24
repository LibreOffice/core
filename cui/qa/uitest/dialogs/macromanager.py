# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from com.sun.star.beans import PropertyValue
from com.sun.star.script.provider import ScriptURIHelper
from com.sun.star.ucb import SimpleFileAccess
from libreoffice.uno.eventlistener import EventListener
from uitest.framework import UITestCase
from uitest.test import DEFAULT_SLEEP
from uitest.uihelper.common import get_state_as_dict, type_text, mkPropertyValues
import uno

import contextlib
import tempfile
import time
import os


def get_user_script_directory(context):
    uriHelper = ScriptURIHelper.create(context, "Python", "user")
    uri = uriHelper.getRootStorageURI()
    return uno.fileUrlToSystemPath(uri)


def generate_test_script_for_directory(directory):
    # Generate a python script that creates a file in the given directory
    return f"""
import os.path

def MyMacro():
    with open(os.path.join(r'{directory}', 'script-ran'), 'x'):
        pass
    """

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

    def select_macro(self, xDialog, *parts):
        self.select_tree_node(xDialog.getChild("scriptcontainers"), *parts[0:-1])
        self.select_tree_node(xDialog.getChild("scripts"), parts[-1])

    def run_macro(self, *parts):
        with self.ui_test.execute_dialog_through_command(".uno:MacroManager",
                                                         close_button="run") as xDialog:
            self.select_macro(xDialog, *parts)

    def run_user_script(self):
        script_dir = get_user_script_directory(self.xContext)
        os.makedirs(script_dir, exist_ok=True)

        # Create a temporary directory in the script directory to contain our test script
        with tempfile.TemporaryDirectory(dir=script_dir) as temp_script_dir:
            # Create a temporary directory to create the check file in
            with tempfile.TemporaryDirectory() as check_file_dir:
                # Create a script with a known name
                with open(os.path.join(temp_script_dir, "MyScript.py"), "w", encoding="utf-8") as f:
                    print(generate_test_script_for_directory(check_file_dir), file=f)

                self.run_macro("My Macros", "Python", os.path.basename(temp_script_dir), "MyScript",
                               "MyMacro")

                # Return whether the test file was created
                return os.path.exists(os.path.join(check_file_dir, 'script-ran'))

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
            self.select_macro(xDialog, "My Macros", "Basic", "Standard", "TestDelete", "middle")

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

    def test_start_center(self):
        # Try running a script directly from the start center. See tdf#171924
        self.assertTrue(self.run_user_script())

    def test_embedded_script(self):
        # Create a temporary directory to create the check file in
        with tempfile.TemporaryDirectory() as check_file_dir:
            # Create a new document with a temporary filename and add a test script to it
            with tempfile.NamedTemporaryFile(suffix=".odt") as doc_file:
                with self.ui_test.create_doc_in_start_center("writer") as xComponent:
                    xFileAccess = SimpleFileAccess.create(self.xContext)
                    xFileAccess.createFolder("vnd.sun.star.tdoc:/1/Scripts/python")

                    with tempfile.NamedTemporaryFile(suffix=".py", mode="w+",
                                                     encoding="utf-8") as script_file:
                        print(generate_test_script_for_directory(check_file_dir),
                              file=script_file)
                        script_file.flush()

                        xFileAccess.copy(uno.systemPathToFileUrl(script_file.name),
                                         "vnd.sun.star.tdoc:/1/Scripts/python/MyScript.py")

                    # Save the document to the temporary file
                    save_opts = (
                        PropertyValue(Name="Overwrite", Value=True),
                        PropertyValue(Name="FilterName", Value="writer8"),
                    )
                    xComponent.storeAsURL(uno.systemPathToFileUrl(doc_file.name), save_opts)

                # Open the document we just created
                with self.ui_test.load_file(uno.systemPathToFileUrl(doc_file.name)):
                    # We should be able to run user macros even though we’re looking at a document
                    # with macros in it
                    self.assertTrue(self.run_user_script())

                    # We shouldn’t be able to run the embedded script because the security policy
                    # disallows it
                    with contextlib.ExitStack() as stack:
                        xDialog = stack.enter_context(
                            self.ui_test.execute_dialog_through_command(".uno:MacroManager",
                                                                        close_button="run"))
                        self.select_macro(xDialog, os.path.basename(doc_file.name), "Python",
                                          "MyScript", "MyMacro")
                        # We want to start the EventListener for the blocking action of showing the
                        # error dialog *after* creating the run macro dialog so that it doesn’t pick
                        # up the wrong event.
                        with self.ui_test.execute_blocking_action(stack.close):
                            pass

                    self.assertFalse(os.path.exists(os.path.join(check_file_dir, 'script-ran')))

                # Temporarily disable macro security
                with self.ui_test.set_config("/org.openoffice.Office.Common/Security/Scripting/"
                                             "MacroSecurityLevel", 0):
                    with self.ui_test.load_file(uno.systemPathToFileUrl(doc_file.name)):
                        # Now we should be able to run the embedded script
                        self.run_macro(os.path.basename(doc_file.name), "Python", "MyScript",
                                       "MyMacro")
                        self.assertTrue(os.path.exists(os.path.join(check_file_dir, 'script-ran')))


# vim: set shiftwidth=4 softtabstop=4 expandtab:
