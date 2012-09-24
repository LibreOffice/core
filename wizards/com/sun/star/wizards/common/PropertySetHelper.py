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
from .DebugHelper import DebugHelper

class PropertySetHelper(object):

    @classmethod
    def __init__(self, _aObj):
        self.m_xPropertySet = _aObj
        self.m_aHashMap = {}

    '''
    set a property, don't throw any exceptions,
    they will only write down as a hint in the helper debug output
    @param _sName name of the property to set
    @param _aValue property value as object
    '''

    def setPropertyValueDontThrow(self, _sName, _aValue):
        try:
            setPropertyValue(_sName, _aValue)
        except Exception, e:
            DebugHelper.writeInfo(
                "Don't throw the exception with property name(" \
                + _sName + " ) : " + e.getMessage())

    '''
    set a property,
    @param _sName name of the property to set
    @param _aValue property value as object
    @throws java.lang.Exception
    '''

    def setPropertyValue(self, _sName, _aValue):
        if self.m_xPropertySet != None:
            try:
                self.m_xPropertySet.setPropertyValue(_sName, _aValue)
            except com.sun.star.beans.UnknownPropertyException, e:
                DebugHelper.writeInfo(e.getMessage())
                DebugHelper.exception(e)
            except com.sun.star.beans.PropertyVetoException, e:
                DebugHelper.writeInfo(e.getMessage())
                DebugHelper.exception(e)
            except ValueError, e:
                DebugHelper.writeInfo(e.getMessage())
                DebugHelper.exception(e)
            except com.sun.star.lang.WrappedTargetException, e:
                DebugHelper.writeInfo(e.getMessage())
                DebugHelper.exception(e)

        else:
            self.m_aHashMap[_sName] = _aValue

    '''
    get a property and convert it to a int value
    @param _sName the string name of the property
    @param _nDefault if an error occur, return this value
    @return the int value of the property
    '''

    def getPropertyValueAsInteger(self, _sName, _nDefault):
        aObject = None
        nValue = _nDefault
        if self.m_xPropertySet != None:
            try:
                aObject = self.m_xPropertySet.getPropertyValue(_sName)
            except com.sun.star.beans.UnknownPropertyException, e:
                DebugHelper.writeInfo(e.getMessage())
            except com.sun.star.lang.WrappedTargetException, e:
                DebugHelper.writeInfo(e.getMessage())

        if aObject != None:
            try:
                nValue = NumericalHelper.toInt(aObject)
            except ValueError, e:
                DebugHelper.writeInfo(
                    "can't convert a object to integer.")

        return nValue

    '''
    get a property and convert it to a short value
    @param _sName the string name of the property
    @param _nDefault if an error occur, return this value
    @return the int value of the property
    '''

    def getPropertyValueAsShort(self, _sName, _nDefault):
        aObject = None
        nValue = _nDefault
        if self.m_xPropertySet != None:
            try:
                aObject = self.m_xPropertySet.getPropertyValue(_sName)
            except com.sun.star.beans.UnknownPropertyException, e:
                DebugHelper.writeInfo(e.getMessage())
            except com.sun.star.lang.WrappedTargetException, e:
                DebugHelper.writeInfo(e.getMessage())

        if aObject != None:
            try:
                nValue = NumericalHelper.toShort(aObject)
            except ValueError, e:
                DebugHelper.writeInfo("can't convert a object to short.")

        return nValue

    '''
    get a property and convert it to a double value
    @param _sName the string name of the property
    @param _nDefault if an error occur, return this value
    @return the int value of the property
    '''

    def getPropertyValueAsDouble(self, _sName, _nDefault):
        aObject = None
        nValue = _nDefault
        if self.m_xPropertySet != None:
            try:
                aObject = self.m_xPropertySet.getPropertyValue(_sName)
            except com.sun.star.beans.UnknownPropertyException, e:
                DebugHelper.writeInfo(e.getMessage())
            except com.sun.star.lang.WrappedTargetException, e:
                DebugHelper.writeInfo(e.getMessage())

        # TODO: I wonder why the same thing is not done in the rest of the
        # getPropertyValueAs* functions...
        if aObject == None:
            if _sName in self.m_aHashMap:
                aObject = self.m_aHashMap[_sName]

        if aObject != None:
            try:
                nValue = NumericalHelper.toDouble(aObject)
            except ValueError, e:
                DebugHelper.writeInfo("can't convert a object to integer.")

        return nValue

    '''
    get a property and convert it to a boolean value
    @param _sName the string name of the property
    @param _bDefault if an error occur, return this value
    @return the boolean value of the property
    '''

    def getPropertyValueAsBoolean(self, _sName, _bDefault):
        aObject = None
        bValue = _bDefault
        if self.m_xPropertySet != None:
            try:
                aObject = self.m_xPropertySet.getPropertyValue(_sName)
            except com.sun.star.beans.UnknownPropertyException, e:
                DebugHelper.writeInfo(e.getMessage())
                DebugHelper.writeInfo(
                    "UnknownPropertyException caught: Name:=" + _sName)
            except com.sun.star.lang.WrappedTargetException, e:
                DebugHelper.writeInfo(e.getMessage())

        if aObject != None:
            try:
                bValue = NumericalHelper.toBoolean(aObject)
            except ValueError, e:
                DebugHelper.writeInfo("can't convert a object to boolean.")

        return bValue

    '''
    get a property and convert it to a string value
    @param _sName the string name of the property
    @param _sDefault if an error occur, return this value
    @return the string value of the property
    '''

    def getPropertyValueAsString(self, _sName, _sDefault):
        aObject = None
        sValue = _sDefault
        if self.m_xPropertySet != None:
            try:
                aObject = self.m_xPropertySet.getPropertyValue(_sName)
            except com.sun.star.beans.UnknownPropertyException, e:
                DebugHelper.writeInfo(e.getMessage())
            except com.sun.star.lang.WrappedTargetException, e:
                DebugHelper.writeInfo(e.getMessage())

        if aObject != None:
            try:
                sValue = AnyConverter.toString(aObject)
            except ValueError, e:
                DebugHelper.writeInfo("can't convert a object to string.")

        return sValue

    '''
    get a property and don't convert it
    @param _sName the string name of the property
    @return the object value of the property without any conversion
    '''

    def getPropertyValueAsObject(self, _sName):
        aObject = None
        if self.m_xPropertySet != None:
            try:
                aObject = self.m_xPropertySet.getPropertyValue(_sName)
            except com.sun.star.beans.UnknownPropertyException, e:
                DebugHelper.writeInfo(e.getMessage())
            except com.sun.star.lang.WrappedTargetException, e:
                DebugHelper.writeInfo(e.getMessage())

        return aObject

    '''
    Debug helper, to show all properties
    which are available in the given object.
    @param _xObj the object of which the properties should shown
    '''

    @classmethod
    def showProperties(self, _xObj):
        aHelper = PropertySetHelper.PropertySetHelper_unknown(_xObj)
        aHelper.showProperties()

    '''
    Debug helper, to show all properties which are available
    in the current object.
    '''

    def showProperties(self):
        sName = ""
        if self.m_xPropertySet != None:
            sName = self.m_xPropertySet.getImplementationName()

            xInfo = self.m_xPropertySet.getPropertySetInfo()
            aAllProperties = xInfo.getProperties()
            DebugHelper.writeInfo(
                "Show all properties of Implementation of :'" + sName + "'")
            i = 0
            while i < aAllProperties.length:
                DebugHelper.writeInfo(" - " + aAllProperties[i].Name)
                i += 1
        else:
            DebugHelper.writeInfo(
                "The given object don't support XPropertySet interface.")

