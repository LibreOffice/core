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

import uno

LOCALE = '/org.openoffice.Setup/L10N/ooSetupSystemLocale'

# Bug 160343 - Standard filter don't works with numbers with decimals
class tdf160343(UITestCase):
    def open_table(self, sTable):
        xToolkit = self.xContext.ServiceManager.createInstance('com.sun.star.awt.Toolkit')
        # connects to the database
        self.xUITest.executeCommand(".uno:DBViewTables")
        xToolkit.waitUntilAllIdlesDispatched()
        xDbController = self.ui_test.get_desktop().getActiveFrame().getController()
        xTableController = xDbController.loadComponent(0, sTable, False)  # DatabaseObject.TABLE
        xToolkit.waitUntilAllIdlesDispatched()
        return xTableController, xTableController.CurrentControl.getModel().getParent()

    def set_standard_filter(self, xTableController, sField, sCondition, sValue):
        with self.ui_test.execute_blocking_action(
                self.xUITest.executeCommandForProvider,
                args=(".uno:FilterCrit", xTableController.getFrame())) as xDialog:
            xDialog.getChild("field1").executeAction("SELECT", mkPropertyValues({"TEXT": sField}))
            xDialog.getChild("cond1").executeAction("SELECT", mkPropertyValues({"TEXT": sCondition}))
            xDialog.getChild("value1").executeAction("SET", mkPropertyValues({"TEXT": sValue}))
        self.xContext.ServiceManager.createInstance('com.sun.star.awt.Toolkit').waitUntilAllIdlesDispatched()

    def standard_filter(self, sTable, sField, sCondition, sValue, nExpectedRows):
        with self.ui_test.load_file(get_url_for_data_file("tdf160343.odb")):
            xTableController, xForm = self.open_table(sTable)
            self.set_standard_filter(xTableController, sField, sCondition, sValue)

            # Without the fix in place, with a locale using a decimal comma the value was lost:
            # the filter was not set and all rows were shown
            self.assertNotEqual("", xForm.Filter)
            self.assertEqual(nExpectedRows, xForm.RowCount)

    def test_decimal_german(self):
        with self.ui_test.set_config(LOCALE, 'de-DE'):
            self.standard_filter("tbl_Decimal", "Decimal_2", ">", "4,2", 6)

    def test_decimal_english(self):
        with self.ui_test.set_config(LOCALE, 'en-US'):
            self.standard_filter("tbl_Decimal", "Decimal_2", ">", "4.2", 6)

    def test_integer_german(self):
        with self.ui_test.set_config(LOCALE, 'de-DE'):
            self.standard_filter("tbl_Decimal", "ID", ">", "3", 7)

    def test_date_german(self):
        with self.ui_test.set_config(LOCALE, 'de-DE'):
            self.standard_filter("tbl_DateTime", "DateValue", ">", "07.02.2026", 1)

    def test_format_changed_while_open(self):
        with self.ui_test.set_config(LOCALE, 'de-DE'):
            with self.ui_test.load_file(get_url_for_data_file("tdf160343.odb")) as document:
                xTableController, xForm = self.open_table("tbl_Decimal")

                # The field has a German format; change it to English (USA) while the table is
                # open, like Column Format does
                aLocale = uno.createUnoStruct("com.sun.star.lang.Locale")
                aLocale.Language = "en"
                aLocale.Country = "US"
                xFormats = document.DataSource.NumberFormatsSupplier.getNumberFormats()
                nFormatKey = xFormats.queryKey("0.00", aLocale, False)
                if nFormatKey == -1:
                    nFormatKey = xFormats.addNew("0.00", aLocale)
                xGridModel = xTableController.CurrentControl.getModel()
                xGridModel.getByName("Decimal_2").FormatKey = nFormatKey

                # The filter uses the format of the row set's column
                self.assertEqual(nFormatKey, xForm.Columns.getByName("Decimal_2").FormatKey)

                # Without the fix in place, the old German format was used for "5.2" until the
                # table was reopened: the value was lost and no filter was set
                self.set_standard_filter(xTableController, "Decimal_2", ">", "5.2")
                self.assertNotEqual("", xForm.Filter)
                self.assertEqual(5, xForm.RowCount)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
