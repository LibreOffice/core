# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_url_for_data_file

from libreoffice.uno.propertyvalue import mkPropertyValues

# Bug 160343 - Standard filter don't works with numbers with decimals
class tdf160343(UITestCase):
    def standard_filter(self, sTable, sField, sCondition, sValue, nExpectedRows):
        with self.ui_test.load_file(get_url_for_data_file("tdf160343.odb")):
            xToolkit = self.xContext.ServiceManager.createInstance('com.sun.star.awt.Toolkit')
            # connects to the database
            self.xUITest.executeCommand(".uno:DBViewTables")
            xToolkit.waitUntilAllIdlesDispatched()
            xDbController = self.ui_test.get_desktop().getActiveFrame().getController()
            xTableController = xDbController.loadComponent(0, sTable, False)  # DatabaseObject.TABLE
            xToolkit.waitUntilAllIdlesDispatched()
            xForm = xTableController.CurrentControl.getModel().getParent()

            with self.ui_test.execute_blocking_action(
                    self.xUITest.executeCommandForProvider,
                    args=(".uno:FilterCrit", xTableController.getFrame())) as xDialog:
                xDialog.getChild("field1").executeAction("SELECT", mkPropertyValues({"TEXT": sField}))
                xDialog.getChild("cond1").executeAction("SELECT", mkPropertyValues({"TEXT": sCondition}))
                xDialog.getChild("value1").executeAction("SET", mkPropertyValues({"TEXT": sValue}))
            xToolkit.waitUntilAllIdlesDispatched()

            # Without the fix in place, with a locale using a decimal comma the value was lost:
            # the filter was not set and all rows were shown
            self.assertNotEqual("", xForm.Filter)
            self.assertEqual(nExpectedRows, xForm.RowCount)

    def test_decimal_german(self):
        with self.ui_test.set_config('/org.openoffice.Setup/L10N/ooSetupSystemLocale', 'de-DE'):
            self.standard_filter("tbl_Decimal", "Decimal_2", ">", "4,2", 6)

    def test_decimal_english(self):
        with self.ui_test.set_config('/org.openoffice.Setup/L10N/ooSetupSystemLocale', 'en-US'):
            self.standard_filter("tbl_Decimal", "Decimal_2", ">", "4.2", 6)

    def test_integer_german(self):
        with self.ui_test.set_config('/org.openoffice.Setup/L10N/ooSetupSystemLocale', 'de-DE'):
            self.standard_filter("tbl_Decimal", "ID", ">", "3", 7)

    def test_date_german(self):
        with self.ui_test.set_config('/org.openoffice.Setup/L10N/ooSetupSystemLocale', 'de-DE'):
            self.standard_filter("tbl_DateTime", "DateValue", ">", "07.02.2026", 1)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
