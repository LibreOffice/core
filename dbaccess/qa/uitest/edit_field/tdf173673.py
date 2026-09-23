# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_url_for_data_file, select_pos

from libreoffice.uno.propertyvalue import mkPropertyValues

# Bug 173673 - Tablefilter: Resetting impossible when "Apply Filter" is unchecked
class tdf173673(UITestCase):
    def test_tdf173673(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf75509.odb")) as document:
            xConnection = document.DataSource.getConnection("", "")
            xStatement = xConnection.createStatement()
            xStatement.executeUpdate('INSERT INTO "Table1" ("FrenchField", "EnglishField") VALUES (1, 1)')
            xStatement.executeUpdate('INSERT INTO "Table1" ("FrenchField", "EnglishField") VALUES (2, 2)')
            xConnection.close()

            self.xUITest.executeCommand(".uno:DBViewTables")
            xToolkit = self.xContext.ServiceManager.createInstance('com.sun.star.awt.Toolkit')
            xToolkit.waitUntilAllIdlesDispatched()
            self.xUITest.executeCommand(".uno:SelectAll")

            with self.ui_test.open_subcomponent_through_command(".uno:DBTableOpen") as xTableFrame:
                xTableWindow = self.xUITest.getWindow(xTableFrame.getContainerWindow())
                xGrid = xTableWindow.getChild("DBGrid")
                xForm = xTableFrame.getController().CurrentControl.getModel().getParent()

                self.assertEqual(2, xForm.RowCount)

                # AutoFilter on the first row's FrenchField cell (value 1)
                xGrid.executeAction("FOCUS", tuple())
                xGrid.executeAction("TYPE", mkPropertyValues({"KEYCODE": "TAB"}))
                self.xUITest.executeCommandForProvider(".uno:AutoFilter", xTableFrame)
                xToolkit.waitUntilAllIdlesDispatched()
                self.assertNotEqual("", xForm.Filter)
                self.assertTrue(xForm.ApplyFilter)
                self.assertEqual(1, xForm.RowCount)

                # Uncheck "Apply Filter": the filter is kept, but no longer applied
                self.xUITest.executeCommandForProvider(".uno:FormFiltered", xTableFrame)
                xToolkit.waitUntilAllIdlesDispatched()
                self.assertNotEqual("", xForm.Filter)
                self.assertFalse(xForm.ApplyFilter)
                self.assertEqual(2, xForm.RowCount)

                # Clear the condition in the Standard Filter dialog
                with self.ui_test.execute_blocking_action(
                        self.xUITest.executeCommandForProvider,
                        args=(".uno:FilterCrit", xTableFrame)) as xDialog:
                    xDialog.getChild("field1").executeAction("SELECT", mkPropertyValues({"TEXT": "- none -"}))
                xToolkit.waitUntilAllIdlesDispatched()

                # Check that the old filter is removed
                self.assertEqual("", xForm.Filter)
                self.assertEqual(2, xForm.RowCount)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
