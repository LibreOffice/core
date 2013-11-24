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
from datetime import date as dateTimeObject

from ...common.FileAccess import FileAccess
from ...common.ConfigGroup import ConfigGroup
from ...common.NumberFormatter import NumberFormatter
from ...common.Properties import Properties
from ..WebConfigSet import WebConfigSet
from .CGExporter import CGExporter
from .CGLayout import CGLayout
from .CGStyle import CGStyle
from .CGIconSet import CGIconSet
from .CGImage import CGImage
from .CGFilter import CGFilter
from .CGSessionName import CGSessionName
from .CGSession import CGSession

from com.sun.star.i18n.NumberFormatIndex import DATE_SYS_DMMMYYYY
from com.sun.star.i18n.NumberFormatIndex import NUMBER_1000DEC2

class CGSettings(ConfigGroup):

    RESOURCE_PAGES_TEMPLATE = 0
    RESOURCE_SLIDES_TEMPLATE = 1
    RESOURCE_CREATED_TEMPLATE = 2
    RESOURCE_UPDATED_TEMPLATE = 3
    RESOURCE_SIZE_TEMPLATE = 4

    def __init__(self, xmsf_, resources_, document):
        self.cp_WorkDir = str()
        self.cp_Exporters = WebConfigSet(CGExporter)
        self.cp_Layouts = WebConfigSet(CGLayout)
        self.cp_Styles = WebConfigSet(CGStyle)
        self.cp_IconSets = WebConfigSet(CGIconSet)
        self.cp_BackgroundImages = WebConfigSet(CGImage)
        self.cp_SavedSessions = WebConfigSet(CGSessionName)
        self.cp_Filters = WebConfigSet(CGFilter)
        self.savedSessions = WebConfigSet(CGSessionName)
        self.cp_DefaultSession = CGSession()
        self.cp_LastSavedSession = str()
        self.fileAccess = None
        self.workPath = None
        self.xmsf = xmsf_
        try:
            self.soTemplateDir = FileAccess.getOfficePath2(self.xmsf, "Config", "", "");
            self.soGalleryDir = FileAccess.getOfficePath2(self.xmsf, "Gallery", "share", "");
            self.setRoot(self)
            self.formatter = self.Formatter(self.xmsf, document, resources_)
            self.resources = resources_
            self.exportersMap = {}
        except Exception:
            traceback.print_exc()

    def getExporters(self, mime):
        exps = self.exportersMap.get(mime)
        if exps is None:
            exps = self.createExporters(mime)
            self.exportersMap[mime] = exps
        return exps

    def createExporters(self, mime):
        exporters = self.cp_Exporters.childrenList
        v = []
        for i in exporters:
            if i.supports(mime):
                try:
                    v.append(i)
                except Exception:
                    traceback.print_exc()
        return v

    '''
    call after read.
    @param xmsf
    @param document the background document. used for date/number formatting.
    '''

    def configure(self, xmsf):
        self.workPath = FileAccess.connectURLs(self.soTemplateDir, self.cp_WorkDir)
        self.calcExportersTargetTypeNames(xmsf)

    def calcExportersTargetTypeNames(self, xmsf):
        typeDetect = xmsf.createInstance("com.sun.star.document.TypeDetection")
        for i in range(self.cp_Exporters.getSize()):
            self.calcExporterTargetTypeName(typeDetect, self.cp_Exporters.getElementAt(i))

    def calcExporterTargetTypeName(self, typeDetect, exporter):
        if (exporter is None):
            return
        if not (exporter.cp_TargetType == "" or exporter.cp_TargetType is None):
            targetTypeName = Properties.getPropertyValue(
                typeDetect.getByName(exporter.cp_TargetType), "UIName")
            exporter.cp_targetTypeName = targetTypeName

    def getFileAccess(self, xmsf = None):
        if xmsf is None:
            xmsf = self.xmsf
        if self.fileAccess is None:
            self.fileAccess = FileAccess(xmsf)
        return self.fileAccess

    class Formatter(object):
        class DateUtils(object):

            def __init__(self, xmsf, document):
                self.formatSupplier = document
                formatSettings = self.formatSupplier.getNumberFormatSettings()
                date = formatSettings.NullDate
                self.calendar = dateTimeObject(date.Year, date.Month, date.Day)
                self.formatter = NumberFormatter.createNumberFormatter(xmsf, self.formatSupplier)

            '''
            @param format a constant of the enumeration NumberFormatIndex
            @return
            '''
            def getFormat(self, f):
                return NumberFormatter.getNumberFormatterKey(self.formatSupplier, f)

            '''
            @param date a VCL date in form of 20041231
            @return a document relative date
            '''
            def format(self, formatIndex, date):
                dateTime = dateTimeObject(date.Year, date.Month, date.Day)
                difference =  dateTime - self.calendar
                return self.formatter.convertNumberToString(formatIndex,  difference.days)

            def getFormatter(self):
                return self.formatter

        def __init__(self, xmsf, document, resources):
            self.dateUtils = self.DateUtils(xmsf, document)
            self.dateFormat = self.dateUtils.getFormat(DATE_SYS_DMMMYYYY)
            self.numberFormat = self.dateUtils.getFormat(NUMBER_1000DEC2)
            self.resources = resources

        def formatCreated(self, date):
            sDate = self.dateUtils.format(self.dateFormat, date)
            return self.resources[CGSettings.RESOURCE_CREATED_TEMPLATE].replace(
                "%DATE", sDate)

        def formatUpdated(self, date):
            sDate = self.dateUtils.format(self.dateFormat, date);
            return self.resources[CGSettings.RESOURCE_UPDATED_TEMPLATE].replace(
                "%DATE", sDate)

        def formatFileSize(self, size):
            sizeInKB = size / float(1024)
            sSize = self.dateUtils.getFormatter().convertNumberToString(
                self.numberFormat, sizeInKB)
            return self.resources[CGSettings.RESOURCE_SIZE_TEMPLATE].replace(
                "%NUMBER", sSize)
