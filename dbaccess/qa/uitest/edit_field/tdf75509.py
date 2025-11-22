# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_url_for_data_file, get_state_as_dict

from libreoffice.uno.propertyvalue import mkPropertyValues

#Bug 75509 - Incorrect display of digits in default Numeric formatting
#            of a field when language isn't English
class tdf75509(UITestCase):
    def test_tdf75509(self):
        # The sample file is an HSQLDB database with a single table. Aside from the primary key, the
        # table has two numeric type fields each with 2 decimal places. One of them is called
        # “FrenchField” and is set to a French locale (ie, the decimal separator is “,”) and the
        # other is called “EnglishField” and is set to a US English locale.
        with self.ui_test.load_file(get_url_for_data_file("tdf75509.odb")) as document:
            xDbWindow = self.xUITest.getTopFocusWindow()

            self.xUITest.executeCommand(".uno:DBViewTables")
            # The above command is run asynchronously via an event on the main thread so we need to
            # wait until it is dispatched before executing the next command
            xToolkit = self.xContext.ServiceManager.createInstance('com.sun.star.awt.Toolkit')
            xToolkit.waitUntilAllIdlesDispatched()

            # We just want to select the single table in the list of tables but it seems difficult
            # to get access to the treeview window for it because there are multiple treeviews in a
            # hierarchy of windows with no name. However AppController handles SelectAll and that
            # does the trick
            self.xUITest.executeCommand(".uno:SelectAll")

            with self.ui_test.open_subcomponent_through_command(".uno:DBTableEdit") as xTableFrame:
                xTableWindow = self.xUITest.getWindow(xTableFrame.getContainerWindow())

                xTableEditor = xTableWindow.getChild("DBTableEditor")

                # Select the “FrenchField” row
                xTableEditor.executeAction("TYPE", mkPropertyValues({"KEYCODE": "DOWN"}))

                # Type a value with the comma decimal separator into the default value field
                xDefaultValue = xTableWindow.getChild("DefaultValue")

                xDefaultValue.executeAction("FOCUS", tuple())
                xDefaultValue.executeAction("SET", mkPropertyValues({"TEXT": "3,14"}))

                # Focus something else so that the example text will get updated
                xTableEditor.executeAction("FOCUS", tuple())

                # The example format should be updated to reflect the default value
                xFormatText = xTableWindow.getChild("FormatText")
                self.assertEqual(get_state_as_dict(xFormatText)["Text"], "3,14")

                # Select the “EnglishField"
                xTableEditor.executeAction("TYPE", mkPropertyValues({"KEYCODE": "DOWN"}))

                # Perform the same tests with the “.” decimal separator
                xDefaultValue.executeAction("FOCUS", tuple())
                xDefaultValue.executeAction("SET", mkPropertyValues({"TEXT": "2.72"}))
                xTableEditor.executeAction("FOCUS", tuple())
                self.assertEqual(get_state_as_dict(xFormatText)["Text"], "2.72")

                # Save the table (ie, just the table, not the actual database file to disk)
                self.xUITest.executeCommandForProvider(".uno:Save", xTableFrame)

            with self.ui_test.open_subcomponent_through_command(".uno:DBTableOpen") as xTableFrame:
                xTableWindow = self.xUITest.getWindow(xTableFrame.getContainerWindow())

                # Focus the “FrenchField” input in the table
                xGrid = xTableWindow.getChild("DBGrid")
                xGrid.executeAction("FOCUS", tuple())
                xGrid.executeAction("TYPE", mkPropertyValues({"KEYCODE": "TAB"}))

                # It should have the default value with a comma separator
                xEdit = self.xUITest.getFocusWindow()
                self.assertEqual(get_state_as_dict(xEdit)["Text"], "3,14")

                # Focus the “EnglishField” input in the table
                xGrid.executeAction("TYPE", mkPropertyValues({"KEYCODE": "TAB"}))

                # It should have the default value with a dot separator
                xEdit = self.xUITest.getFocusWindow()
                self.assertEqual(get_state_as_dict(xEdit)["Text"], "2.72")

                # Modify the field so that the we can save the record
                xEdit.executeAction("SET", mkPropertyValues({"TEXT": "2.71"}))

                self.xUITest.executeCommandForProvider(".uno:RecSave", xTableFrame)

            # Check that the default values actually entered the database
            xDbController = self.ui_test.get_desktop().getActiveFrame().getController()
            xConnection = xDbController.ActiveConnection
            xStatement = xConnection.createStatement()
            xResultSet = xStatement.executeQuery("SELECT \"FrenchField\", \"EnglishField\" "
                                                 "FROM \"Table1\"")
            self.assertTrue(xResultSet.next())
            self.assertEqual(xResultSet.getString(1), "3.14")
            self.assertEqual(xResultSet.getString(2), "2.71")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
