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
from .DBMetaData import DBMetaData

class CommandMetaData(DBMetaData):

    def __init__(self, xMSF, aLocale=None, oNumberFormatter=None):
        super(CommandMetaData, self).__init__(xMSF, aLocale, oNumberFormatter)      

    '''def initializeFieldColumns(self, _bgetDefaultValue, _CommandName, _FieldNames):
        """ generated source for method initializeFieldColumns """
        self.setCommandName(_CommandName)
        self.FieldColumns = []
        i = 0
        while len(_FieldNames):
            self.FieldColumns[i] = FieldColumn(self, _FieldNames[i], self.getCommandName(), False)
            i += 1

    @initializeFieldColumns.register(object, str, XNameAccess)
    def initializeFieldColumns_0(self, _FieldNames, _xColumns):
        """ generated source for method initializeFieldColumns_0 """
        self.FieldColumns = []
        i = 0
        while len(_FieldNames):
            self.FieldColumns[i] = FieldColumn(self, _xColumns, _FieldNames[i])
            i += 1

    @initializeFieldColumns.register(object, str, str)
    def initializeFieldColumns_1(self, _FieldNames, _CommandName):
        """ generated source for method initializeFieldColumns_1 """
        self.setCommandName(_CommandName)
        self.FieldColumns = [None]*
        i = 0
        while len(_FieldNames):
            self.FieldColumns[i] = FieldColumn(self, _FieldNames[i], _CommandName, False)
            if self.FieldTitleSet != None and self.FieldTitleSet.containsKey(_FieldNames[i]):
                self.FieldColumns[i].setFieldTitle(self.FieldTitleSet.get(_FieldNames[i]))
                if self.FieldColumns[i].getFieldTitle() == None:
                    self.FieldColumns[i].setFieldTitle(_FieldNames[i])
                    self.FieldTitleSet.put(_FieldNames[i], _FieldNames[i])
            i += 1'''

    def getFieldTitleSet(self):
        """ generated source for method getFieldTitleSet """
        return self.FieldTitleSet

    def getColumnObjectByFieldName(self, _FieldName, _bgetByDisplayName):
        """ generated source for method getColumnObjectByFieldName """
        try:
            if _bgetByDisplayName:
                CurFieldColumn = self.getFieldColumnByDisplayName(_FieldName)
            else:
                CurFieldColumn = self.getFieldColumnByFieldName(_FieldName)
            return UnoRuntime.queryInterface(XPropertySet.__class__, oColumn)
        except Exception as exception:
            exception.printStackTrace(System.err)
            return None

    def prependSortFieldNames(self, _fieldnames):
        """ generated source for method prependSortFieldNames """
        aSortFields = ArrayList()
        i = 0
        while len(_fieldnames):
            sSortFieldName[0] = _fieldnames[i]
            if index > -1:
                sSortFieldName[1] = self.SortFieldNames[index][1]
            else:
                sSortFieldName[1] = PropertyNames.ASC
            aSortFields.add(sSortFieldName)
            i += 1
        i = 0
        while len(SortFieldNames):
            if JavaTools.FieldInList(_fieldnames, self.SortFieldNames[i][0]) == -1:
                aSortFields.add(self.SortFieldNames[i])
            i += 1
        self.SortFieldNames = [None]*len(aSortFields)
        aSortFields.toArray(self.SortFieldNames)

    def getSortFieldNames(self):
        """ generated source for method getSortFieldNames """
        return self.SortFieldNames

    def setSortFieldNames(self, aNewListList):
        """ generated source for method setSortFieldNames """
        self.SortFieldNames = aNewListList

    def getFieldColumn(self, _FieldName, _CommandName):
        """ generated source for method getFieldColumn """
        i = 0
        while len(FieldColumns):
            if self.FieldColumns[i].getFieldName() == _FieldName and self.FieldColumns[i].getCommandName() == _CommandName:
                return self.FieldColumns[i]
            i += 1
        return None

    def getFieldColumnByFieldName(self, _FieldName):
        """ generated source for method getFieldColumnByFieldName """
        i = 0
        while len(FieldColumns):
            if sFieldName == _FieldName:
                return self.FieldColumns[i]
            if _FieldName.indexOf('.') == -1:
                if sFieldName == sCompound:
                    return self.FieldColumns[i]
            i += 1
        raise com.sun.star.uno.RuntimeException()

    def getFieldColumnByDisplayName(self, _DisplayName):
        """ generated source for method getFieldColumnByDisplayName """
        identifierQuote = getIdentifierQuote()
        i = 0
        while len(FieldColumns):
            if sDisplayName == _DisplayName:
                return self.FieldColumns[i]
            if _DisplayName.indexOf('.') == -1:
                if sDisplayName == sCompound:
                    return self.FieldColumns[i]
            if quotedName == _DisplayName:
                return self.FieldColumns[i]
            i += 1
        raise com.sun.star.uno.RuntimeException()

    def getFieldColumnByTitle(self, _FieldTitle):
        """ generated source for method getFieldColumnByTitle """
        i = 0
        while len(FieldColumns):
            if self.FieldColumns[i].getFieldTitle() == _FieldTitle:
                return self.FieldColumns[i]
            i += 1
        i = 0
        while len(FieldColumns):
            if self.FieldColumns[i].getFieldName() == _FieldTitle:
                return self.FieldColumns[i]
            i += 1
        raise com.sun.star.uno.RuntimeException()

    def getFieldNamesOfCommand(self, _commandname, _commandtype, _bAppendMode):
        """ generated source for method getFieldNamesOfCommand """
        try:
            FieldNames = oCommand.getColumns().getElementNames()
            if len(FieldNames):
                while len(FieldNames):
                    if JavaTools.FieldInIntTable(WidthList, iType) >= 0:
                        ResultFieldNames.add(sFieldName)
                    elif JavaTools.FieldInIntTable(BinaryTypes, iType) >= 0:
                        ResultFieldNames.add(sFieldName)
                    n += 1
                self.m_aAllFieldNames = [None]*len(ResultFieldNames)
                self.m_aAllFieldNames = ResultFieldNames.toArray(self.m_aAllFieldNames)
                return True
        except Exception as exception:
            exception.printStackTrace(System.err)
        oResource = Resource(xMSF, "Database", "dbw")
        sMsgNoFieldsFromCommand = oResource.getResText(RID_DB_COMMON + 45)
        sMsgNoFieldsFromCommand = JavaTools.replaceSubString(sMsgNoFieldsFromCommand, _commandname, "%NAME")
        showMessageBox("ErrorBox", VclWindowPeerAttribute.OK, sMsgNoFieldsFromCommand)
        return False

    def getOrderableColumns(self, _fieldnames):
        """ generated source for method getOrderableColumns """
        aOrderableColumns = ArrayList()
        i = 0
        while len(_fieldnames):
            if getDBDataTypeInspector().isColumnOrderable(ofieldcolumn.getXColumnPropertySet()):
                aOrderableColumns.add(_fieldnames[i])
            i += 1
        sretfieldnames = [None]*len(aOrderableColumns)
        return aOrderableColumns.toArray(sretfieldnames)

    def getCommandName(self):
        """ generated source for method getCommandName """
        return self.Command

    def setCommandName(self, _command):
        """ generated source for method setCommandName """
        self.Command = _command

    def getCommandType(self):
        """ generated source for method getCommandType """
        return self.CommandType

    def setCommandType(self, _commandType):
        """ generated source for method setCommandType """
        self.CommandType = _commandType

    def isnumeric(self, _oFieldColumn):
        """ generated source for method isnumeric """
        try:
            if (len(NumericTypes)) and (ifound > 0):
                return (NumericTypes[ifound] == iType)
            else:
                return False
        except Exception as exception:
            exception.printStackTrace(System.err)
            return False

    def setNumericFields(self):
        """ generated source for method setNumericFields """
        try:
            while len(FieldColumns):
                if self.isnumeric(self.FieldColumns[i]):
                    numericfieldsvector.add(self.FieldColumns[i].getDisplayFieldName())
                i += 1
            self.NumericFieldNames = [None]*len(numericfieldsvector)
            numericfieldsvector.toArray(self.NumericFieldNames)
            return self.NumericFieldNames
        except Exception as exception:
            exception.printStackTrace(System.err)
            return []

    '''
    @overloaded
    def getFieldNames(self, _sDisplayFieldNames, _sCommandName):
        """ generated source for method getFieldNames """
        sFieldNamesVector = java.util.ArrayList()
        i = 0
        while len(FieldColumns):
            if _sCommandName == self.FieldColumns[i].getCommandName() and JavaTools.FieldInList(_sDisplayFieldNames, self.FieldColumns[i].getDisplayFieldName()) > -1:
                sFieldNamesVector.add(self.FieldColumns[i].getFieldName())
            i += 1
        sFieldNames = [None]*len(sFieldNamesVector)
        sFieldNamesVector.toArray(sFieldNames)
        return sFieldNames

    @getFieldNames.register(object)
    def getFieldNames_0(self):
        """ generated source for method getFieldNames_0 """
        sFieldNames = []
        i = 0
        while len(FieldColumns):
            sFieldNames[i] = self.FieldColumns[i].getFieldName()
            i += 1
        return sFieldNames

    def getDisplayFieldNames(self):
        """ generated source for method getDisplayFieldNames """
        sDisplayFieldNames = []
        i = 0
        while len(FieldColumns):
            sDisplayFieldNames[i] = self.FieldColumns[i].getDisplayFieldName()
            i += 1
        return sDisplayFieldNames

    '''
    def setNonAggregateFieldNames(self):
        """ generated source for method setNonAggregateFieldNames """
        try:
            while len(FieldColumns):
                if JavaTools.FieldInTable(self.AggregateFieldNames, self.FieldColumns[i].getDisplayFieldName()) == -1:
                    nonaggregatefieldsvector.add(self.FieldColumns[i].getDisplayFieldName())
                i += 1
            self.NonAggregateFieldNames = [None]*len(nonaggregatefieldsvector)
            nonaggregatefieldsvector.toArray(self.NonAggregateFieldNames)
            return self.NonAggregateFieldNames
        except Exception as exception:
            exception.printStackTrace(System.err)
            return []

    def hasNumericalFields(self, _DisplayFieldNames):
        """ generated source for method hasNumericalFields """
        if _DisplayFieldNames != None and len(_DisplayFieldNames):
            while len(_DisplayFieldNames):
                if self.isnumeric(self.getFieldColumnByDisplayName(_DisplayFieldNames[i])):
                    return True
                i += 1
        return False

    def getFieldTitle(self, FieldName):
        """ generated source for method getFieldTitle """
        FieldTitle = FieldName
        if self.FieldTitleSet != None:
            FieldTitle = self.FieldTitleSet.get(FieldName)
            if FieldTitle == None:
                return FieldName
        return FieldTitle

    def setFieldTitles(self, sFieldTitles):
        """ generated source for method setFieldTitles """
        nFieldColLength = int()
        i = 0
        while len(sFieldTitles):
            if i < nFieldColLength:
                self.FieldColumns[i].setFieldTitle(sFieldTitles[i])
            i += 1

    def getFieldTitles(self):
        """ generated source for method getFieldTitles """
        sFieldTitles = []
        i = 0
        while len(FieldColumns):
            sFieldTitles[i] = self.FieldColumns[i].getFieldTitle()
            i += 1
        return sFieldTitles

    def setGroupFieldNames(self, GroupFieldNames):
        """ generated source for method setGroupFieldNames """
        self.GroupFieldNames = GroupFieldNames

    def getGroupFieldNames(self):
        """ generated source for method getGroupFieldNames """
        return self.GroupFieldNames

    def createRecordFieldNames(self):
        """ generated source for method createRecordFieldNames """
        CurFieldName = str()
        GroupFieldCount = int()
        TotFieldCount = int()
        GroupFieldCount = JavaTools.getArraylength(self.GroupFieldNames)
        self.RecordFieldNames = [None]*TotFieldCount - GroupFieldCount
        a = 0
        i = 0
        while i < TotFieldCount:
            CurFieldName = self.FieldColumns[i].getFieldName()
            if JavaTools.FieldInList(self.GroupFieldNames, CurFieldName) < 0:
                self.RecordFieldNames[a] = CurFieldName
                a += 1
            i += 1

    def setRecordFieldNames(self, _aNewList):
        """ generated source for method setRecordFieldNames """
        self.RecordFieldNames = _aNewList

    def getRecordFieldNames(self):
        """ generated source for method getRecordFieldNames """
        return self.RecordFieldNames

    def getRecordFieldName(self, i):
        """ generated source for method getRecordFieldName """
        return self.RecordFieldNames[i]

    def getReferencedTables(self, _stablename, _ncommandtype):
        """ generated source for method getReferencedTables """
        sTotReferencedTables = []
        try:
            if _ncommandtype == com.sun.star.sdb.CommandType.TABLE and xDBMetaData.supportsIntegrityEnhancementFacility():
                self.xIndexKeys = xKeysSupplier.getKeys()
                while i < self.xIndexKeys.getCount():
                    if curtype == KeyType.FOREIGN:
                        if getTableNamesAsNameAccess().hasByName(sreftablename):
                            TableVector.add(sreftablename)
                    i += 1
                if len(TableVector) > 0:
                    sTotReferencedTables = [None]*len(TableVector)
                    TableVector.toArray(sTotReferencedTables)
        except Exception as e:
            e.printStackTrace(System.err)
        return sTotReferencedTables

    def getKeyColumns(self, _sreferencedtablename):
        """ generated source for method getKeyColumns """
        skeycolumnnames = None
        try:
            while i < self.xIndexKeys.getCount():
                if curtype == KeyType.FOREIGN:
                    if getTableNamesAsNameAccess().hasByName(scurreftablename):
                        if scurreftablename == _sreferencedtablename:
                            skeycolumnnames = [None]*2
                            skeycolumnnames[0] = smastercolnames
                            skeycolumnnames[1] = []
                            while len(smastercolnames):
                                skeycolumnnames[1][n] = AnyConverter.toString(xcolPropertySet.getPropertyValue("RelatedColumn"))
                                n += 1
                            return skeycolumnnames
                i += 1
        except Exception as e:
            e.printStackTrace()
        return skeycolumnnames

    def openFormDocument(self, _bReadOnly):
        """ generated source for method openFormDocument """

    def setCommandComposingAttributes(self):
        """ generated source for method setCommandComposingAttributes """
        try:
            self.sCatalogSep = xDBMetaData.getCatalogSeparator()
            self.sIdentifierQuote = xDBMetaData.getIdentifierQuoteString()
            self.bCommandComposerAttributesalreadyRetrieved = True
        except SQLException as e:
            e.printStackTrace(System.err)

    def isCatalogAtStart(self):
        """ generated source for method isCatalogAtStart """
        if not self.bCommandComposerAttributesalreadyRetrieved:
            self.setCommandComposingAttributes()
        return self.bCatalogAtStart

    def getCatalogSeparator(self):
        """ generated source for method getCatalogSeparator """
        if not self.bCommandComposerAttributesalreadyRetrieved:
            self.setCommandComposingAttributes()
        return self.sCatalogSep

    def getIdentifierQuote(self):
        """ generated source for method getIdentifierQuote """
        if not self.bCommandComposerAttributesalreadyRetrieved:
            self.setCommandComposingAttributes()
        return self.sIdentifierQuote

