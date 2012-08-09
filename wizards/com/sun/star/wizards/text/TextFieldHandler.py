#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#
import traceback
import time
import unicodedata
from wizards.common.PropertyNames import PropertyNames

from com.sun.star.util import DateTime

class TextFieldHandler(object):
    '''
    Creates a new instance of TextFieldHandler
    @param xMSF
    @param xTextDocument
    '''

    xTextFieldsSupplierAux = None
    arrayTextFields = None
    dictTextFields = None

    def __init__(self, xMSF, xTextDocument):
        self.xMSFDoc = xMSF
        self.xTextFieldsSupplier = xTextDocument
        if TextFieldHandler.xTextFieldsSupplierAux is not \
                self.xTextFieldsSupplier:
            self.__getTextFields()
            TextFieldHandler.xTextFieldsSupplierAux = self.xTextFieldsSupplier

    def refreshTextFields(self):
        xUp = self.xTextFieldsSupplier.TextFields
        xUp.refresh()

    def getUserFieldContent(self, xTextCursor):
        try:
            xTextRange = xTextCursor.getEnd()
            oTextField = Helper.getUnoPropertyValue(xTextRange, "TextField")
            if com.sun.star.uno.AnyConverter.isVoid(oTextField):
                return ""
            else:
                xMaster = oTextField.getTextFieldMaster()
                UserFieldContent = xMaster.getPropertyValue("Content")
                return UserFieldContent

        except Exception, exception:
            traceback.print_exc()

        return ""

    def insertUserField(self, xTextCursor, FieldName, FieldTitle):
        try:
            xField = self.xMSFDoc.createInstance(
                "com.sun.star.text.TextField.User")

            if self.xTextFieldsSupplier.TextFieldMasters.hasByName(
                "com.sun.star.text.FieldMaster.User." + FieldName):
                oMaster = self.xTextFieldsSupplier.TextFieldMasters.getByName( \
                    "com.sun.star.text.FieldMaster.User." + FieldName)
                oMaster.dispose()

            xPSet = createUserField(FieldName, FieldTitle)
            xField.attachTextFieldMaster(xPSet)
            xTextCursor.getText().insertTextContent(
                xTextCursor, xField, False)
        except com.sun.star.uno.Exception, exception:
            traceback.print_exc()

    def createUserField(self, FieldName, FieldTitle):
        xPSet = self.xMSFDoc.createInstance(
            "com.sun.star.text.FieldMaster.User")
        xPSet.setPropertyValue(PropertyNames.PROPERTY_NAME, FieldName)
        xPSet.setPropertyValue("Content", FieldTitle)
        return xPSet

    def __getTextFields(self):
        try:
            if self.xTextFieldsSupplier.TextFields.hasElements():
                TextFieldHandler.dictTextFields = {}
                TextFieldHandler.arrayTextFields = []
                xEnum = \
                    self.xTextFieldsSupplier.TextFields.createEnumeration()
                while xEnum.hasMoreElements():
                    oTextField = xEnum.nextElement()
                    TextFieldHandler.arrayTextFields.append(oTextField)
                    xPropertySet = oTextField.TextFieldMaster
                    if xPropertySet.Name:
                        TextFieldHandler.dictTextFields[xPropertySet.Name] = \
                            oTextField
        except Exception, e:
            #TODO Auto-generated catch block
            traceback.print_exc()

    def __getTextFieldsByProperty(
            self, _PropertyName, _aPropertyValue):
        try:
            xProperty = TextFieldHandler.dictTextFields[_aPropertyValue]
            xPropertySet = xProperty.TextFieldMaster
            if xPropertySet.PropertySetInfo.hasPropertyByName(
                    _PropertyName):
                oValue = xPropertySet.getPropertyValue(_PropertyName)
                sValue = unicodedata.normalize(
                    'NFKD', oValue).encode('ascii','ignore')
                if sValue == _aPropertyValue:
                    return xProperty
            return None
        except KeyError, e:
            return None

    def changeUserFieldContent(self, _FieldName, _FieldContent):
        DependentTextFields = self.__getTextFieldsByProperty(
                PropertyNames.PROPERTY_NAME, _FieldName)
        if DependentTextFields is not None:
            DependentTextFields.TextFieldMaster.setPropertyValue("Content", _FieldContent)
            self.refreshTextFields()

    def updateDocInfoFields(self):
        try:
            for i in TextFieldHandler.arrayTextFields:
                if i.supportsService(
                    "com.sun.star.text.TextField.ExtendedUser"):
                    i.update()

                if i.supportsService(
                    "com.sun.star.text.TextField.User"):
                    i.update()

        except Exception, e:
            traceback.print_exc()

    def updateDateFields(self):
        try:
            now = time.localtime(time.time())
            dt = DateTime()
            dt.Day = time.strftime("%d", now)
            dt.Year = time.strftime("%Y", now)
            dt.Month = time.strftime("%m", now)
            dt.Month += 1
            for i in TextFieldHandler.arrayTextFields:
                if i.supportsService(
                    "com.sun.star.text.TextField.DateTime"):
                    i.setPropertyValue("IsFixed", False)
                    i.setPropertyValue("DateTimeValue", dt)

        except Exception, e:
            traceback.print_exc()

    def fixDateFields(self, _bSetFixed):
        try:
            for i in TextFieldHandler.arrayTextFields:
                if i.supportsService(
                    "com.sun.star.text.TextField.DateTime"):
                    i.setPropertyValue("IsFixed", _bSetFixed)

        except Exception, e:
            traceback.print_exc()

    def removeUserFieldByContent(self, _FieldContent):
        try:
            xDependentTextFields = self.__getTextFieldsByProperty(
                "Content", _FieldContent)
            if xDependentTextFields != None:
                i = 0
                while i < xDependentTextFields.length:
                    xDependentTextFields[i].dispose()
                    i += 1

        except Exception, e:
            traceback.print_exc()
