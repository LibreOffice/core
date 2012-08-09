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

from com.sun.star.lang import Locale
from com.sun.star.util.NumberFormat import DATE, LOGICAL, DATETIME, TEXT, NUMBER

class NumberFormatter(object):

    def __init__(self, _xNumberFormatsSupplier, _aLocale, _xMSF=None):
        self.iDateFormatKey = -1
        self.iDateTimeFormatKey = -1
        self.iNumberFormatKey = -1
        self.iTextFormatKey = -1
        self.iTimeFormatKey = -1
        self.iLogicalFormatKey = -1
        self.bNullDateCorrectionIsDefined = False
        self.aLocale = _aLocale
        if _xMSF is not None:
            self.xNumberFormatter = _xMSF.createInstance(
                "com.sun.star.util.NumberFormatter")
        self.xNumberFormats = _xNumberFormatsSupplier.NumberFormats
        self.xNumberFormatSettings = \
            _xNumberFormatsSupplier.NumberFormatSettings
        self.xNumberFormatter.attachNumberFormatsSupplier(
            _xNumberFormatsSupplier)

    '''
    @param _xMSF
    @param _xNumberFormatsSupplier
    @return
    @throws Exception
    @deprecated
    '''

    @classmethod
    def createNumberFormatter(self, _xMSF, _xNumberFormatsSupplier):
        oNumberFormatter = _xMSF.createInstance(
            "com.sun.star.util.NumberFormatter")
        oNumberFormatter.attachNumberFormatsSupplier(_xNumberFormatsSupplier)
        return oNumberFormatter

    '''
    gives a key to pass to a NumberFormat object. <br/>
    example: <br/>
    <pre>
    XNumberFormatsSupplier nsf =
        (XNumberFormatsSupplier)UnoRuntime.queryInterface(...,document)
    int key = Desktop.getNumberFormatterKey(
        nsf, ...star.i18n.NumberFormatIndex.DATE...)
    XNumberFormatter nf = Desktop.createNumberFormatter(xmsf, nsf);
    nf.convertNumberToString( key, 1972 );
    </pre>
    @param numberFormatsSupplier
    @param type - a constant out of i18n.NumberFormatIndex enumeration.
    @return a key to use with a util.NumberFormat instance.
    '''

    @classmethod
    def getNumberFormatterKey(self, numberFormatsSupplier, Type):
        return numberFormatsSupplier.NumberFormats.getFormatIndex(
            Type, Locale())

    def convertNumberToString(self, _nkey, _dblValue, _xNumberFormatter=None):
        if _xNumberFormatter is None:
            return self.xNumberFormatter.convertNumberToString(
                _nkey, _dblValue)
        else:
            return _xNumberFormatter.convertNumberToString(_nkey, _dblValue)

    def convertStringToNumber(self, _nkey, _sString):
        return self.xNumberFormatter.convertStringToNumber(_nkey, _sString)

    '''
    @param dateCorrection The lDateCorrection to set.
    '''

    def setNullDateCorrection(self, dateCorrection):
        self.lDateCorrection = dateCorrection

    def defineNumberFormat(self, _FormatString):
        try:
            NewFormatKey = self.xNumberFormats.queryKey(
                _FormatString, self.aLocale, True)
            if NewFormatKey is -1:
                NewFormatKey = self.xNumberFormats.addNew(
                    _FormatString, self.aLocale)

            return NewFormatKey
        except Exception, e:
            traceback.print_exc()
            return -1

    '''
    returns a numberformat for a FormatString.
    @param _FormatString
    @param _aLocale
    @return
    '''

    def defineNumberFormat(self, _FormatString, _aLocale):
        try:
            NewFormatKey = self.xNumberFormats.queryKey(
                _FormatString, _aLocale, True)
            if NewFormatKey == -1:
                NewFormatKey = self.xNumberFormats.addNew(
                    _FormatString, _aLocale)

            return NewFormatKey
        except Exception, e:
            traceback.print_exc()
            return -1

    def setNumberFormat(self, _xFormatObject, _FormatKey, _oNumberFormatter):
        try:
            xNumberFormat = _oNumberFormatter.xNumberFormats.getByKey(
                _FormatKey)
            FormatString = str(Helper.getUnoPropertyValue(
                xNumberFormat, "FormatString"))
            oLocale = Helper.getUnoPropertyValue(xNumberFormat, "Locale")
            NewFormatKey = defineNumberFormat(FormatString, oLocale)
            _xFormatObject.setPropertyValue(
                "FormatsSupplier",
                _oNumberFormatter.xNumberFormatter.getNumberFormatsSupplier())
            if _xFormatObject.getPropertySetInfo().hasPropertyByName(
                    "NumberFormat"):
                _xFormatObject.setPropertyValue("NumberFormat", NewFormatKey)
            elif _xFormatObject.getPropertySetInfo().hasPropertyByName(
                    "FormatKey"):
                _xFormatObject.setPropertyValue("FormatKey", NewFormatKey)
            else:
                # TODO: throws a exception in a try catch environment, very helpful?
                raise Exception

        except Exception, exception:
            traceback.print_exc()

    def getNullDateCorrection(self):
        if not self.bNullDateCorrectionIsDefined:
            dNullDate = Helper.getUnoStructValue(
                self.xNumberFormatSettings, "NullDate")
            lNullDate = Helper.convertUnoDatetoInteger(dNullDate)
            oCal = java.util.Calendar.getInstance()
            oCal.set(1900, 1, 1)
            dTime = oCal.getTime()
            lTime = dTime.getTime()
            lDBNullDate = lTime / (3600 * 24000)
            self.lDateCorrection = lDBNullDate - lNullDate
            return self.lDateCorrection
        else:
            return self.lDateCorrection

    def setBooleanReportDisplayNumberFormat(self):
        FormatString = "[=1]" + str(9745) + ";[=0]" + str(58480) + ";0"
        self.iLogicalFormatKey = self.xNumberFormats.queryKey(
            FormatString, self.aLocale, True)
        try:
            if self.iLogicalFormatKey == -1:
                self.iLogicalFormatKey = self.xNumberFormats.addNew(
                    FormatString, self.aLocale)

        except Exception, e:
            #MalformedNumberFormat
            traceback.print_exc()
            self.iLogicalFormatKey = self.xNumberFormats.getStandardFormat(
                NumberFormat.LOGICAL, self.aLocale)

        return self.iLogicalFormatKey

    '''
    @return Returns the iDateFormatKey.
    '''

    def getDateFormatKey(self):
        if self.iDateFormatKey == -1:
            self.iDateFormatKey = self.xNumberFormats.getStandardFormat(
                NumberFormat.DATE, self.aLocale)

        return self.iDateFormatKey

    '''
    @return Returns the iDateTimeFormatKey.
    '''

    def getDateTimeFormatKey(self):
        if self.iDateTimeFormatKey == -1:
            self.iDateTimeFormatKey = self.xNumberFormats.getStandardFormat(
                NumberFormat.DATETIME, self.aLocale)

        return self.iDateTimeFormatKey

    '''
    @return Returns the iLogicalFormatKey.
    '''

    def getLogicalFormatKey(self):
        if self.iLogicalFormatKey == -1:
            self.iLogicalFormatKey = self.xNumberFormats.getStandardFormat(
                NumberFormat.LOGICAL, self.aLocale)

        return self.iLogicalFormatKey

    '''
    @return Returns the iNumberFormatKey.
    '''

    def getNumberFormatKey(self):
        if self.iNumberFormatKey == -1:
            self.iNumberFormatKey = self.xNumberFormats.getStandardFormat(
                NumberFormat.NUMBER, self.aLocale)

        return self.iNumberFormatKey

    '''
    @return Returns the iTextFormatKey.
    '''

    def getTextFormatKey(self):
        if self.iTextFormatKey == -1:
            self.iTextFormatKey = self.xNumberFormats.getStandardFormat(
                NumberFormat.TEXT, self.aLocale)

        return self.iTextFormatKey

    '''
    @return Returns the iTimeFormatKey.
    '''

    def getTimeFormatKey(self):
        if self.iTimeFormatKey == -1:
            self.iTimeFormatKey = self.xNumberFormats.getStandardFormat(
                NumberFormat.TIME, self.aLocale)

        return self.iTimeFormatKey
