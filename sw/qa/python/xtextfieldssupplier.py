#! /usr/bin/env python
# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
import unittest
import unohelper
from org.libreoffice.unotest import UnoInProcess
from com.sun.star.container import NoSuchElementException
import uno


class TestXTextFieldsSupplier(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        cls._uno = UnoInProcess()
        cls._uno.setUp()

    @classmethod
    def tearDownClass(cls):
        cls._uno.tearDown()

    def test_getTextFields(self):
        xDoc = self.__class__._uno.openDocFromTDOC("xtextfieldssupplier.odt")
        self.assertIsNotNone(xDoc, "document was not loaded")
        xTextFieldsSupplier = xDoc

        # Get text fields collection
        xTextFields = xTextFieldsSupplier.getTextFields()
        self.assertIsNotNone(xTextFields, "getTextFields fails")

        # Iterate through collection and ensure that we receive expected fields
        fieldTypesList = [
            "com.sun.star.text.textfield.PageNumber",
            "com.sun.star.text.textfield.Annotation",
            "com.sun.star.text.textfield.docinfo.CreateDateTime"
        ]
        xFieldEnum = xTextFields.createEnumeration()
        for fieldType, xField in zip(fieldTypesList, xFieldEnum):
            self.assertTrue(xField.supportsService(fieldType),
                            "field " + xField.getPresentation(True) +
                            " does not support " + fieldType + " service!")

        xDoc.close(True)

    def test_getTextFieldMasters(self):
        xDoc = self.__class__._uno.openDocFromTDOC("xtextfieldssupplier.odt")
        self.assertIsNotNone(xDoc, "document was not loaded")
        xTextFieldsSupplier = xDoc

        # Get text fields master
        xFieldMasters = xTextFieldsSupplier.getTextFieldMasters()
        self.assertIsNotNone(xFieldMasters, "getTextFieldMasters fails")
        self.assertTrue(xFieldMasters.hasElements(), "TextFieldMaster has no elements")

        # Check elements in TextFieldsMaster collection
        masterNames = [
            "com.sun.star.text.fieldmaster.SetExpression.Illustration",
            "com.sun.star.text.fieldmaster.SetExpression.Table",
            "com.sun.star.text.fieldmaster.SetExpression.Text",
            "com.sun.star.text.fieldmaster.SetExpression.Drawing",
            "com.sun.star.text.fieldmaster.SetExpression.Figure",
        ]
        for masterName in masterNames:
            self.assertTrue(xFieldMasters.hasByName(masterName),
                            "TextFieldMaster has no element " + masterName)
            master = xFieldMasters.getByName(masterName)
            self.assertIsNotNone(master,
                                 "can't get " + masterName + " from TextFieldMaster")
            self.assertIsNotNone(master.getPropertyValue("Name"),
                                 "can't get Name property from TextFieldMaster " + masterName)

        # Ensure that invalid elements are not accessible
        invalidMasterName = "com.sun.star.text.fieldmaster.SetExpression.NoSuchMaster"
        self.assertFalse(xFieldMasters.hasByName(invalidMasterName),
                         "TextFieldMaster has element " + invalidMasterName)

        with self.assertRaises(NoSuchElementException):
            xFieldMasters.getByName(invalidMasterName)

        xDoc.close(True)


if __name__ == '__main__':
    unittest.main()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
