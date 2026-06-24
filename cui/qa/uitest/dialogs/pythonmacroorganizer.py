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
from libreoffice.uno.propertyvalue import mkPropertyValues
from com.sun.star.beans import PropertyValue
from com.sun.star.script.provider import ScriptURIHelper
from com.sun.star.ucb import SimpleFileAccess
import uno

import tempfile
import os


def get_user_script_directory(context):
    uriHelper = ScriptURIHelper.create(context, "Python", "user")
    uri = uriHelper.getRootStorageURI()
    return uno.fileUrlToSystemPath(uri)


class PythonMacroOrganizerTest(UITestCase):
    def test_enable_create_edit_buttons(self):
        script_dir = get_user_script_directory(self.xContext)
        os.makedirs(script_dir, exist_ok=True)

        # Create a temporary directory in the script directory to contain our test script
        with tempfile.TemporaryDirectory(dir=script_dir) as temp_dir:
            # Create a script with a known name
            with open(os.path.join(temp_dir, "MyScript.py"), "w", encoding="utf-8") as f:
                print("def MyMacro():\n    pass", file=f)

            with self.ui_test.execute_dialog_through_command(
                    ".uno:ScriptOrganizer?ScriptOrganizer.Language:string=Python",
                    close_button="close") as xDialog:
                xEditButton = xDialog.getChild("edit")
                xCreateButton = xDialog.getChild("create")
                xRunButton = xDialog.getChild("ok")
                xScripts = xDialog.getChild("scripts")

                xMyMacros = xScripts.getChild(0)
                self.assertEqual(get_state_as_dict(xMyMacros)["Text"], "My Macros")
                xMyMacros.executeAction("SELECT", tuple())

                self.assertEqual(get_state_as_dict(xEditButton)["Enabled"], "false")
                self.assertEqual(get_state_as_dict(xCreateButton)["Enabled"], "true")
                self.assertEqual(get_state_as_dict(xRunButton)["Enabled"], "false")

                xMyMacros.executeAction("EXPAND", tuple())

                # Find the library for the temporary directory
                xTestLibrary = \
                    next(x for x in map(lambda i: xMyMacros.getChild(i),
                                        range(int(get_state_as_dict(xMyMacros)["Children"])))
                         if get_state_as_dict(x)["Text"] == os.path.basename(temp_dir))

                xTestLibrary.executeAction("SELECT", tuple())

                # A folder is selected, so we should be able to create but not edit
                self.assertEqual(get_state_as_dict(xEditButton)["Enabled"], "false")
                self.assertEqual(get_state_as_dict(xCreateButton)["Enabled"], "true")
                self.assertEqual(get_state_as_dict(xRunButton)["Enabled"], "false")

                xTestLibrary.executeAction("EXPAND", tuple())

                # The library should contain the single script file that we created
                self.assertEqual(get_state_as_dict(xTestLibrary)["Children"], "1")
                xTestScript = xTestLibrary.getChild(0)
                self.assertEqual(get_state_as_dict(xTestScript)["Text"], "MyScript")

                xTestScript.executeAction("SELECT", tuple())

                # A script is selected, so we should be able to edit but not run or create
                self.assertEqual(get_state_as_dict(xEditButton)["Enabled"], "true")
                self.assertEqual(get_state_as_dict(xCreateButton)["Enabled"], "false")
                self.assertEqual(get_state_as_dict(xRunButton)["Enabled"], "false")

                xTestScript.executeAction("EXPAND", tuple())

                # The script should contain the single macro from the file
                self.assertEqual(get_state_as_dict(xTestScript)["Children"], "1")
                xTestMacro = xTestScript.getChild(0)
                self.assertEqual(get_state_as_dict(xTestMacro)["Text"], "MyMacro")

                xTestMacro.executeAction("SELECT", tuple())

                # A macro is selected, so we should be able to edit and run but not create
                self.assertEqual(get_state_as_dict(xEditButton)["Enabled"], "true")
                self.assertEqual(get_state_as_dict(xCreateButton)["Enabled"], "false")
                self.assertEqual(get_state_as_dict(xRunButton)["Enabled"], "true")

    def test_create(self):
        script_dir = get_user_script_directory(self.xContext)
        os.makedirs(script_dir, exist_ok=True)

        # Create a temporary directory in the script directory to contain our test script
        with tempfile.TemporaryDirectory(dir=script_dir) as temp_dir:
            with self.ui_test.create_doc_in_start_center("writer") as xDoc:
                # Open the Python macro organizer dialog. The close button is set to “ok” so that
                # the final action will be to run the macro
                with self.ui_test.execute_dialog_through_command(
                        ".uno:ScriptOrganizer?ScriptOrganizer.Language:string=Python",
                        close_button="ok") as xDialog:
                    xScripts = xDialog.getChild("scripts")

                    xMyMacros = xScripts.getChild(0)
                    xMyMacros.executeAction("EXPAND", tuple())

                    # Find the library for the temporary directory
                    xTestLibrary = \
                        next(x for x in map(lambda i: xMyMacros.getChild(i),
                                            range(int(get_state_as_dict(xMyMacros)["Children"])))
                             if get_state_as_dict(x)["Text"] == os.path.basename(temp_dir))

                    xTestLibrary.executeAction("SELECT", tuple())

                    xCreateButton = xDialog.getChild("create")

                    with self.ui_test.execute_blocking_action(
                            xCreateButton.executeAction, args=("CLICK", tuple())) \
                            as xNameDialog:
                        xEntry = xNameDialog.getChild("entry")
                        xEntry.executeAction(
                            "SET", mkPropertyValues({"TEXT": "TestScript"}))

                    # The new script should be immediately selected
                    self.assertEqual(get_state_as_dict(xScripts)["SelectEntryText"], "TestScript")

                    # tdf#171586 The button enabledness should be updated immediately without having
                    # to change the selection
                    xEditButton = xDialog.getChild("edit")
                    xCreateButton = xDialog.getChild("create")
                    xRunButton = xDialog.getChild("ok")
                    self.assertEqual(get_state_as_dict(xEditButton)["Enabled"], "true")
                    self.assertEqual(get_state_as_dict(xCreateButton)["Enabled"], "false")
                    self.assertEqual(get_state_as_dict(xRunButton)["Enabled"], "false")

                    # The library should have one child which is the script we just created
                    self.assertEqual(get_state_as_dict(xTestLibrary)["Children"], "1")
                    xScript = xTestLibrary.getChild(0)
                    self.assertEqual(get_state_as_dict(xScript)["Text"], "TestScript")

                    xScript.executeAction("EXPAND", tuple())

                    # The script should have the single example macro as a child
                    self.assertEqual(get_state_as_dict(xScript)["Children"], "1")
                    xMacro = xScript.getChild(0)
                    self.assertEqual(get_state_as_dict(xMacro)["Text"], "exampleMacro")

                    xMacro.executeAction("SELECT", tuple())

                # Running the macro should have added the example text to the document
                self.assertEqual(xDoc.getText().getString().strip(), "example text")

    # tdf#171671 Test a filename with spaces
    def test_script_name_with_spaces(self):
        script_dir = get_user_script_directory(self.xContext)
        os.makedirs(script_dir, exist_ok=True)

        # Create a temporary directory in the script directory to contain our test script
        with tempfile.TemporaryDirectory(dir=script_dir) as temp_dir:
            # Create a script with a filename that contains spaces
            with open(os.path.join(temp_dir, "My Script.py"), "w", encoding="utf-8") as f:
                print("def MyMacro():\n    pass", file=f)
            # Open the Python macro organizer dialog
            with self.ui_test.execute_dialog_through_command(
                    ".uno:ScriptOrganizer?ScriptOrganizer.Language:string=Python",
                    close_button="close") as xDialog:
                xScripts = xDialog.getChild("scripts")

                xMyMacros = xScripts.getChild(0)
                xMyMacros.executeAction("EXPAND", tuple())

                # Find the library for the temporary directory
                xTestLibrary = \
                    next(x for x in map(lambda i: xMyMacros.getChild(i),
                                        range(int(get_state_as_dict(xMyMacros)["Children"])))
                         if get_state_as_dict(x)["Text"] == os.path.basename(temp_dir))

                xTestLibrary.executeAction("EXPAND", tuple())

                # The library should have one child which is the script we created with the filename
                # containing spaces
                self.assertEqual(get_state_as_dict(xTestLibrary)["Children"], "1")
                xScript = xTestLibrary.getChild(0)
                self.assertEqual(get_state_as_dict(xScript)["Text"], "My Script")

    def test_macros_disabled(self):
        # Create a new document with a temporary filename and add a test script to it
        with tempfile.NamedTemporaryFile(suffix=".odt") as doc_file:
            with self.ui_test.create_doc_in_start_center("writer") as xComponent:
                xFileAccess = SimpleFileAccess.create(self.xContext)
                xFileAccess.createFolder("vnd.sun.star.tdoc:/1/Scripts/python")

                with tempfile.NamedTemporaryFile(suffix=".py", mode="w+",
                                                 encoding="utf-8") as script_file:
                    print("def addHello():\n"
                          "    XSCRIPTCONTEXT.getDocument().getText().setString(\"hello\")",
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

            # Helper function to select the macro
            def select_macro(xDialog):
                xScripts = xDialog.getChild("scripts")
                # Find the library for the temporary document
                xDocLibrary = \
                    next(x for x in map(lambda i: xScripts.getChild(i),
                                        range(int(get_state_as_dict(xScripts)["Children"])))
                         if get_state_as_dict(x)["Text"] == os.path.basename(doc_file.name))
                xDocLibrary.executeAction("EXPAND", tuple())
                xScript = xDocLibrary.getChild(0)
                xScript.executeAction("EXPAND", tuple())
                xMacro = xScript.getChild(0)
                xMacro.executeAction("SELECT", tuple())

            # Open the document we just created
            with self.ui_test.load_file(uno.systemPathToFileUrl(doc_file.name)) as xComponent:
                # We shouldn’t be able to run the embedded script because the security policy
                # disallows it
                with self.ui_test.execute_dialog_through_command(
                        ".uno:ScriptOrganizer?ScriptOrganizer.Language:string=Python",
                        close_button="close") as xDialog:
                    select_macro(xDialog)

                    # Clicking run should pop up a nested modal dialog reporting the problem
                    xRun = xDialog.getChild("ok")
                    with self.ui_test.execute_blocking_action(xRun.executeAction,
                                                              args=("CLICK", tuple())):
                        pass

                # The macro shouldn’t have run so the document text should still be empty
                self.assertEqual(xComponent.getText().getString(), "")

            # Temporarily disable macro security
            with self.ui_test.set_config("/org.openoffice.Office.Common/Security/Scripting/"
                                         "MacroSecurityLevel", 0):
                with self.ui_test.load_file(uno.systemPathToFileUrl(doc_file.name)) as xComponent:
                    # Now we should be able to run the embedded script
                    with self.ui_test.execute_dialog_through_command(
                            ".uno:ScriptOrganizer?ScriptOrganizer.Language:string=Python",
                            close_button="ok") as xDialog:
                        select_macro(xDialog)

                    # The script should have run so the document should contain the text that it
                    # added
                    self.assertEqual(xComponent.getText().getString(), "hello")


# vim: set shiftwidth=4 softtabstop=4 expandtab:
