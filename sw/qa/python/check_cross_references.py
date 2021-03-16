'''
  This file is part of the LibreOffice project.

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  This file incorporates work covered by the following license notice:

    Licensed to the Apache Software Foundation (ASF) under one or more
    contributor license agreements. See the NOTICE file distributed
    with this work for additional information regarding copyright
    ownership. The ASF licenses this file to you under the Apache
    License, Version 2.0 (the "License"); you may not use this file
    except in compliance with the License. You may obtain a copy of
    the License at http://www.apache.org/licenses/LICENSE-2.0 .
'''
import unittest
from com.sun.star.text.ReferenceFieldPart import (NUMBER, NUMBER_NO_CONTEXT, NUMBER_FULL_CONTEXT, TEXT)
from com.sun.star.text.ReferenceFieldSource import BOOKMARK
from org.libreoffice.unotest import UnoInProcess


class CheckCrossReferences(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        cls._uno = UnoInProcess()
        cls._uno.setUp()
        cls.document = cls._uno.openDocFromTDOC("CheckCrossReferences.odt")
        cls.xParaEnum = None
        cls.xPortionEnum = None
        cls.xFieldsRefresh = None

    @classmethod
    def tearDownClass(cls):
        cls._uno.tearDown()
        # HACK in case cls.document holds a UNO proxy to an SwXTextDocument (whose dtor calls
        # Application::GetSolarMutex via sw::UnoImplPtrDeleter), which would potentially only be
        # garbage-collected after VCL has already been deinitialized:
        cls.document = None

    def getNextField(self):
        while True:
            while self.xPortionEnum is None:
                if (not(self.xParaEnum.hasMoreElements())):
                    self.fail("Cannot retrieve next field.")

                aPara = self.xParaEnum.nextElement()
                self.xPortionEnum = aPara.createEnumeration()

            if (self.xPortionEnum is None):
                break

            for xPortionProps in self.xPortionEnum:
                sPortionType = str(xPortionProps.getPropertyValue("TextPortionType"))
                if (sPortionType == "TextField"):
                    xField = xPortionProps.getPropertyValue("TextField")
                    self.assertTrue(xField, "Cannot retrieve next field")
                    return xField

            self.xPortionEnum = None
        return None  # unreachable

    def getFieldProps(self, xField):
        xProps = xField
        self.assertTrue(xProps, "Cannot retrieve field properties.")
        return xProps

    def checkField(self, xField, xProps, nFormat, aExpectedFieldResult):
        # set requested format
        xProps.setPropertyValue("ReferenceFieldPart", int(nFormat))

        # refresh fields in order to get new format applied
        self.xFieldsRefresh.refresh()
        aFieldResult = xField.getPresentation(False)
        self.assertEqual(aExpectedFieldResult, aFieldResult, "set reference field format doesn't result in correct field result")

    def test_checkCrossReferences(self):
        xParaEnumAccess = self.document.getText()
        self.xParaEnum = xParaEnumAccess.createEnumeration()

        # get field refresher
        xFieldSupp = self.__class__.document
        self.xFieldsRefresh = xFieldSupp.getTextFields()

        # check first reference field
        # strings for checking
        FieldResult1 = "*i*"
        FieldResult2 = "+b+*i*"
        FieldResult3 = "-1-+b+*i*"
        FieldResult4 = "1"
        FieldResult5 = "1"
        FieldResult6 = "A.1"
        FieldResult7 = "2(a)"
        FieldResult8 = "2(b)"
        FieldResult9 = "2"
        FieldResult10 = "1(a)"
        FieldResult11 = "(b)"
        FieldResult12 = "(a)"

        # variables for current field
        xField = self.getNextField()
        xProps = self.getFieldProps(xField)
        self.checkField(xField, xProps, NUMBER, FieldResult2)
        self.checkField(xField, xProps, NUMBER_NO_CONTEXT, FieldResult1)
        self.checkField(xField, xProps, NUMBER_FULL_CONTEXT, FieldResult3)

        xField = self.getNextField()
        xProps = self.getFieldProps(xField)
        self.checkField(xField, xProps, NUMBER, FieldResult1)
        self.checkField(xField, xProps, NUMBER_NO_CONTEXT, FieldResult1)
        self.checkField(xField, xProps, NUMBER_FULL_CONTEXT, FieldResult3)

        xField = self.getNextField()
        xProps = self.getFieldProps(xField)
        self.checkField(xField, xProps, NUMBER, FieldResult3)
        self.checkField(xField, xProps, NUMBER_NO_CONTEXT, FieldResult1)
        self.checkField(xField, xProps, NUMBER_FULL_CONTEXT, FieldResult3)

        xField = self.getNextField()
        xProps = self.getFieldProps(xField)
        self.checkField(xField, xProps, NUMBER, FieldResult5)
        self.checkField(xField, xProps, NUMBER_NO_CONTEXT, FieldResult4)
        self.checkField(xField, xProps, NUMBER_FULL_CONTEXT, FieldResult6)

        xField = self.getNextField()
        xProps = self.getFieldProps(xField)
        self.checkField(xField, xProps, NUMBER, FieldResult4)
        self.checkField(xField, xProps, NUMBER_NO_CONTEXT, FieldResult4)
        self.checkField(xField, xProps, NUMBER_FULL_CONTEXT, FieldResult6)

        xField = self.getNextField()
        xProps = self.getFieldProps(xField)
        self.checkField(xField, xProps, NUMBER, FieldResult6)
        self.checkField(xField, xProps, NUMBER_NO_CONTEXT, FieldResult4)
        self.checkField(xField, xProps, NUMBER_FULL_CONTEXT, FieldResult6)

        xField = self.getNextField()
        xProps = self.getFieldProps(xField)
        self.checkField(xField, xProps, NUMBER, FieldResult7)
        self.checkField(xField, xProps, NUMBER_NO_CONTEXT, FieldResult12)
        self.checkField(xField, xProps, NUMBER_FULL_CONTEXT, FieldResult7)
        xField = self.getNextField()
        xProps = self.getFieldProps(xField)
        self.checkField(xField, xProps, NUMBER, FieldResult8)
        self.checkField(xField, xProps, NUMBER_NO_CONTEXT, FieldResult11)
        self.checkField(xField, xProps, NUMBER_FULL_CONTEXT, FieldResult8)

        xField = self.getNextField()
        xProps = self.getFieldProps(xField)
        self.checkField(xField, xProps, NUMBER, FieldResult9)
        self.checkField(xField, xProps, NUMBER_NO_CONTEXT, FieldResult9)
        self.checkField(xField, xProps, NUMBER_FULL_CONTEXT, FieldResult9)

        xField = self.getNextField()
        xProps = self.getFieldProps(xField)
        self.checkField(xField, xProps, NUMBER, FieldResult4)
        self.checkField(xField, xProps, NUMBER_NO_CONTEXT, FieldResult4)
        self.checkField(xField, xProps, NUMBER_FULL_CONTEXT, FieldResult4)

        xField = self.getNextField()
        xProps = self.getFieldProps(xField)
        self.checkField(xField, xProps, NUMBER, FieldResult10)
        self.checkField(xField, xProps, NUMBER_NO_CONTEXT, FieldResult12)
        self.checkField(xField, xProps, NUMBER_FULL_CONTEXT, FieldResult10)

        xField = self.getNextField()
        xProps = self.getFieldProps(xField)
        self.checkField(xField, xProps, NUMBER, FieldResult12)
        self.checkField(xField, xProps, NUMBER_NO_CONTEXT, FieldResult12)
        self.checkField(xField, xProps, NUMBER_FULL_CONTEXT, FieldResult7)

        # insert a certain cross-reference bookmark and a reference field to this bookmark
        # restart paragraph enumeration
        xParaEnumAccess = self.__class__.document.getText()
        self.xParaEnum = xParaEnumAccess.createEnumeration()

        # iterate on the paragraphs to find certain paragraph to insert the bookmark
        for xParaTextRange in self.xParaEnum:

            if xParaTextRange.getString() == "J":
                break
            else:
                xParaTextRange = None

        self.assertTrue(xParaTextRange, "Cannot find paragraph to insert cross-reference bookmark")

        # insert bookmark
        xFac = self.__class__.document
        cBookmarkName = "__RefNumPara__47114711"
        xBookmark = xFac.createInstance("com.sun.star.text.Bookmark")

        if xBookmark is not None:
            xName = xBookmark
            xName.setName(cBookmarkName)
            xBookmark.attach(xParaTextRange.getStart())

        # insert reference field, which references the inserted bookmark
        xNewField = xFac.createInstance("com.sun.star.text.TextField.GetReference")

        if xNewField is not None:
            xFieldProps = xNewField
            xFieldProps.setPropertyValue("ReferenceFieldPart", int(TEXT))
            xFieldProps.setPropertyValue("ReferenceFieldSource", int(BOOKMARK))
            xFieldProps.setPropertyValue("SourceName", cBookmarkName)
            xFieldTextRange = self.xParaEnum.nextElement()
            xNewField.attach(xFieldTextRange.getEnd())
            self.xFieldsRefresh.refresh()

        # check inserted reference field
        xField = xNewField
        self.assertEqual("J", xField.getPresentation(False), "inserted reference field doesn't has correct field result")

        xParaTextRange.getStart().setString("Hallo new bookmark: ")
        self.xFieldsRefresh.refresh()
        self.assertEqual("Hallo new bookmark: J", xField.getPresentation(False), "inserted reference field doesn't has correct field result")


if __name__ == "__main__":
    unittest.main()
