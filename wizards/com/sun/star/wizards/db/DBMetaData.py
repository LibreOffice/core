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

class DBMetaData(object):

    TableNames = []
    QueryNames = []
    WidthList = []

    #  DataType.OTHER, 
    iMaxColumnsInSelect = int()
    iMaxColumnsInGroupBy = int()
    iMaxColumnNameLength = -1
    iMaxTableNameLength = -1
    bPasswordIsRequired = bool()
    NOLIMIT = 9999999
    RID_DB_COMMON = 1000
    INVALID = 9999999
    oNumberFormatter = None
    bdisposeConnection = False

    def getDataSourcePropertySet(self):
        """ generated source for method getDataSourcePropertySet """
        return self.xDataSourcePropertySet

    def __init__(self, xMSF, aLocale=None, oNumberFormatter=None):
        if not aLocale:
            self.aLocale = aLocale
        if not oNumberFormatter:
            self.oNumberFormatter = oNumberFormatter

        self.getInterfaces(xMSF)
        #[TODO-XISCO] self.InitializeWidthList()


    def getNumberFormatter(self):
        """ generated source for method getNumberFormatter """
        if self.oNumberFormatter == None:
            try:
                # TODO get the locale from the datasource
                self.aLocale = Configuration.getLocale(self.xMSF)
                self.oNumberFormatter = NumberFormatter(self.xMSF, xNumberFormatsSupplier, self.aLocale)
                self.lDateCorrection = self.oNumberFormatter.getNullDateCorrection()
            except Exception as e:
                Logger.getLogger(DBMetaData.__class__.__name__).log(Level.SEVERE, None, e)
        return self.oNumberFormatter

    def getNullDateCorrection(self):
        if self.lDateCorrection == self.INVALID:
            if self.oNumberFormatter == None:
                self.oNumberFormatter = self.getNumberFormatter()
            self.lDateCorrection = self.oNumberFormatter.getNullDateCorrection()
        return self.lDateCorrection

    def getInterfaces(self, xMSF):
        try:
            self.xMSF = xMSF
            self.xDatabaseContext = xMSF.createInstance("com.sun.star.sdb.DatabaseContext")
            self.DataSourceNames = self.xDatabaseContext.getElementNames()
        except Exception:
            traceback.print_exc()

    def setCommandTypes(self):
        TableCount = int()
        QueryCount = int()
        CommandCount = int()
        i = int()
        a = int()
        TableCount = JavaTools.getArraylength(self.TableNames)
        QueryCount = JavaTools.getArraylength(self.QueryNames)
        CommandCount = TableCount + QueryCount
        self.CommandTypes = []
        if TableCount > 0:
            while i < TableCount:
                self.CommandTypes[i] = com.sun.star.sdb.CommandType.TABLE
                i += 1
            a = i
            while i < QueryCount:
                self.CommandTypes[a] = com.sun.star.sdb.CommandType.QUERY
                a += 1
                i += 1

    def hasTableByName(self, _stablename):
        """ generated source for method hasTableByName """
        return getTableNamesAsNameAccess().hasByName(_stablename)

    def setTableByName(self, _tableName):
        """ generated source for method setTableByName """
        oTableObject = CommandObject(_tableName, com.sun.star.sdb.CommandType.TABLE)
        self.CommandObjects.add(oTableObject)

    def getTableByName(self, _tablename):
        """ generated source for method getTableByName """
        return getCommandByName(_tablename, com.sun.star.sdb.CommandType.TABLE)

    def getQueryByName(self, _queryname):
        """ generated source for method getQueryByName """
        return getCommandByName(_queryname, com.sun.star.sdb.CommandType.QUERY)

    def getCommandByName(self, _commandname, _commandtype):
        """ generated source for method getCommandByName """
        oCommand = None
        i = 0
        while i < len(self.CommandObjects):
            oCommand = self.CommandObjects.get(i)
            if (oCommand.Name == _commandname) and (oCommand.CommandType == _commandtype):
                return oCommand
            i += 1
        if oCommand == None:
            oCommand = CommandObject(_commandname, _commandtype)
            self.CommandObjects.add(oCommand)
        return oCommand

    def setQueryByName(self, _QueryName):
        """ generated source for method setQueryByName """
        oQueryObject = CommandObject(_QueryName, com.sun.star.sdb.CommandType.QUERY)
        self.CommandObjects.add(oQueryObject)

    class CommandObject(object):

        def __init__(self, _CommandName, _CommandType):
            try:
                self.Name = _CommandName
                self.CommandType = _CommandType
                if self.CommandType == com.sun.star.sdb.CommandType.TABLE:
                    oCommand = getTableNamesAsNameAccess().getByName(self.Name)
                else:
                    oCommand = getQueryNamesAsNameAccess().getByName(self.Name)
                self.xPropertySet = UnoRuntime.queryInterface(XPropertySet.__class__, oCommand)
                #  TODO: Performance leak getColumns() take very long.
                self.xColumns = UnoRuntime.queryInterface(XNameAccess.__class__, xCommandCols.getColumns())
            except Exception as e:
                Logger.getLogger(DBMetaData.__class__.__name__).log(Level.SEVERE, None, e)

    def hasEscapeProcessing(self, _xQueryPropertySet):
        """ generated source for method hasEscapeProcessing """
        bHasEscapeProcessing = False
        try:
            if _xQueryPropertySet.getPropertySetInfo().hasPropertyByName("EscapeProcessing"):
                bHasEscapeProcessing = AnyConverter.toBoolean(_xQueryPropertySet.getPropertyValue("EscapeProcessing"))
        except Exception as e:
            Logger.getLogger(DBMetaData.__class__.__name__).log(Level.SEVERE, None, e)
        return bHasEscapeProcessing

    def getQueryNamesAsNameAccess(self):
        """ generated source for method getQueryNamesAsNameAccess """
        xDBQueries = UnoRuntime.queryInterface(XQueriesSupplier.__class__, self.DBConnection)
        self.xQueryNames = xDBQueries.getQueries()
        return self.xQueryNames

    def getTableNamesAsNameAccess(self):
        """ generated source for method getTableNamesAsNameAccess """
        xDBTables = UnoRuntime.queryInterface(XTablesSupplier.__class__, self.DBConnection)
        return xDBTables.getTables()

    def getQueryNames(self):
        """ generated source for method getQueryNames """
        if self.QueryNames != None:
            if len(QueryNames):
                return self.QueryNames
        self.QueryNames = self.getQueryNamesAsNameAccess().getElementNames()
        return self.QueryNames

    def getTableNames(self):
        """ generated source for method getTableNames """
        if self.TableNames != None:
            if len(TableNames):
                return self.TableNames
        self.TableNames = self.getTableNamesAsNameAccess().getElementNames()
        return self.TableNames

    # [TODO-XISCO] Remove it ?
    def InitializeWidthList(self):
        """ generated source for method InitializeWidthList """
        self.WidthList = []
        self.WidthList[0][0] = DataType.BIT
        self.WidthList[1][0] = DataType.BOOLEAN
        self.WidthList[2][0] = DataType.TINYINT
        self.WidthList[3][0] = DataType.BIGINT
        self.WidthList[4][0] = DataType.LONGVARCHAR
        self.WidthList[5][0] = DataType.CHAR
        self.WidthList[6][0] = DataType.NUMERIC
        self.WidthList[7][0] = DataType.DECIMAL
        self.WidthList[8][0] = DataType.INTEGER
        self.WidthList[9][0] = DataType.SMALLINT
        self.WidthList[10][0] = DataType.FLOAT
        self.WidthList[11][0] = DataType.REAL
        self.WidthList[12][0] = DataType.DOUBLE
        self.WidthList[13][0] = DataType.VARCHAR
        self.WidthList[14][0] = DataType.DATE
        self.WidthList[15][0] = DataType.TIME
        self.WidthList[16][0] = DataType.TIMESTAMP

    def isBinaryDataType(self, _itype):
        """ generated source for method isBinaryDataType """
        if self.NumericTypes == None:
            self.InitializeWidthList()
        return (JavaTools.FieldInIntTable(self.BinaryTypes, _itype) > -1)

    def getMaxTablesInSelect(self):
        """ generated source for method getMaxTablesInSelect """
        try:
            if itablecount == 0:
                return DBMetaData.NOLIMIT
            else:
                return itablecount
        except SQLException as e:
            Logger.getLogger(DBMetaData.__class__.__name__).log(Level.SEVERE, None, e)
            return -1

    def getMaxColumnsInSelect(self):
        """ generated source for method getMaxColumnsInSelect """
        return self.iMaxColumnsInSelect

    def getMaxColumnsInGroupBy(self):
        """ generated source for method getMaxColumnsInGroupBy """
        return self.iMaxColumnsInGroupBy

    def setMaxColumnsInSelect(self):
        """ generated source for method setMaxColumnsInSelect """
        self.iMaxColumnsInSelect = self.xDBMetaData.getMaxColumnsInSelect()
        if self.iMaxColumnsInSelect == 0:
            self.iMaxColumnsInSelect = DBMetaData.NOLIMIT

    def setMaxColumnsInGroupBy(self):
        """ generated source for method setMaxColumnsInGroupBy """
        self.iMaxColumnsInGroupBy = self.xDBMetaData.getMaxColumnsInGroupBy()
        if self.iMaxColumnsInGroupBy == 0:
            self.iMaxColumnsInGroupBy = DBMetaData.NOLIMIT

    def getMaxColumnsInTable(self):
        """ generated source for method getMaxColumnsInTable """
        iMaxColumnsInTable = self.xDBMetaData.getMaxColumnsInTable()
        if iMaxColumnsInTable == 0:
            iMaxColumnsInTable = DBMetaData.NOLIMIT
        return iMaxColumnsInTable

    def getDataSourceObjects(self):
        """ generated source for method getDataSourceObjects """
        try:
            self.xDBMetaData = self.DBConnection.getMetaData()
            getDataSourceInterfaces()
            self.setMaxColumnsInGroupBy()
            self.setMaxColumnsInSelect()
        except SQLException as e:
            Logger.getLogger(DBMetaData.__class__.__name__).log(Level.SEVERE, None, e)

    def ensureDataSourceSettings(self):
        """ generated source for method ensureDataSourceSettings """
        if self.m_dataSourceSettings != None:
            return
        dataSourceProperties = UnoRuntime.queryInterface(XPropertySet.__class__, getDataSource())
        self.m_dataSourceSettings = UnoRuntime.queryInterface(XPropertySet.__class__, dataSourceProperties.getPropertyValue("Settings"))

    def isSQL92CheckEnabled(self):
        """ generated source for method isSQL92CheckEnabled """
        isSQL92CheckEnabled = False
        try:
            self.ensureDataSourceSettings()
            isSQL92CheckEnabled = AnyConverter.toBoolean(self.m_dataSourceSettings.getPropertyValue("EnableSQL92Check"))
        except Exception as e:
            Logger.getLogger(DBMetaData.__class__.__name__).log(Level.SEVERE, None, e)
        return isSQL92CheckEnabled

    def verifyName(self, _sname, _maxlen):
        """ generated source for method verifyName """
        if _maxlen > len(_sname):
            return _sname.substring(0, _maxlen)
        if self.isSQL92CheckEnabled():
            return Desktop.removeSpecialCharacters(self.xMSF, Configuration.getLocale(self.xMSF), _sname)
        return _sname

    def getDataSource(self):
        """ generated source for method getDataSource """
        if self.m_dataSource == None:
            try:
                self.m_dataSource = UnoRuntime.queryInterface(XDataSource.__class__, oDataSource)
            except com.sun.star.container.NoSuchElementException as e:
                pass
            except com.sun.star.lang.WrappedTargetException as e:
                pass
        return self.m_dataSource

    def setDataSourceByName(self, _DataSourceName):
        """ generated source for method setDataSourceByName """
        try:
            self.DataSourceName = _DataSourceName
            getDataSourceInterfaces()
            if xDocu != None:
                self.xModel = xDocu.getDatabaseDocument()
        except Exception as e:
            Logger.getLogger(DBMetaData.__class__.__name__).log(Level.SEVERE, None, e)

    def getDataSourceInterfaces(self):
        """ generated source for method getDataSourceInterfaces """
        self.xDataSourcePropertySet = UnoRuntime.queryInterface(XPropertySet.__class__, self.getDataSource())
        self.bPasswordIsRequired = (bool(self.xDataSourcePropertySet.getPropertyValue("IsPasswordRequired"))).booleanValue()

    def getConnection(self, curproperties):
        try:
            if Properties.hasPropertyValue(curproperties, PropertyNames.ACTIVE_CONNECTION):
                xConnection = UnoRuntime.queryInterface(XConnection.__class__, Properties.getPropertyValue(curproperties, PropertyNames.ACTIVE_CONNECTION))
                if xConnection != None:
                    self.m_dataSource = UnoRuntime.queryInterface(XDataSource.__class__, child.getParent())
                    if xDocu != None:
                        self.xModel = xDocu.getDatabaseDocument()
                    if xPSet != None:
                        self.DataSourceName = AnyConverter.toString(xPSet.getPropertyValue(PropertyNames.PROPERTY_NAME))
                    return self.getConnection(xConnection)
                else:
                    self.bdisposeConnection = True
            else:
                self.bdisposeConnection = True
            if Properties.hasPropertyValue(curproperties, "DataSourceName"):
                return self.getConnection(sDataSourceName)
            elif Properties.hasPropertyValue(curproperties, "DataSource"):
                self.m_dataSource = UnoRuntime.queryInterface(XDataSource.__class__, Properties.getPropertyValue(curproperties, "DataSource"))
                if xDocu != None:
                    self.xModel = xDocu.getDatabaseDocument()
                return self.getConnection(self.m_dataSource)
            if Properties.hasPropertyValue(curproperties, "DatabaseLocation"):
                return self.getConnection(sDataSourceName)
        except IllegalArgumentException as e:
            Logger.getLogger(DBMetaData.__class__.__name__).log(Level.SEVERE, None, e)
        except UnknownPropertyException as e:
            Logger.getLogger(DBMetaData.__class__.__name__).log(Level.SEVERE, None, e)
        except WrappedTargetException as e:
            Logger.getLogger(DBMetaData.__class__.__name__).log(Level.SEVERE, None, e)
        return False

    #TODO-XISCO
    '''
    def getConnection_0(self, _DataSourceName):
        """ generated source for method getConnection_0 """
        self.setDataSourceByName(_DataSourceName)
        return self.getConnection(self.getDataSource())

    def getConnection_1(self, _DBConnection):
        """ generated source for method getConnection_1 """
        try:
            self.DBConnection = _DBConnection
            self.m_connectionTools = UnoRuntime.queryInterface(XConnectionTools.__class__, self.DBConnection)
            self.getDataSourceObjects()
            return True
        except Exception as e:
            Logger.getLogger(DBMetaData.__class__.__name__).log(Level.SEVERE, None, e)
            return False

    @getConnection.register(object, XDataSource)
    def getConnection_2(self, _dataSource):
        """ generated source for method getConnection_2 """
        oResource = Resource(self.xMSF, "Database", "dbw")
        try:
            if self.DBConnection != None:
                self.xConnectionComponent.dispose()
            self.getDataSourceInterfaces()
            if not self.bPasswordIsRequired:
                self.DBConnection = _dataSource.getConnection(PropertyNames.EMPTY_STRING, PropertyNames.EMPTY_STRING)
                bgetConnection = True
            else:
                while True:
                    try:
                        self.DBConnection = xCompleted2.connectWithCompletion(xInteractionHandler)
                        bgetConnection = self.DBConnection != None
                        if not bgetConnection:
                            bExitLoop = True
                    except Exception as exception:
                        iMsg = showMessageBox("QueryBox", VclWindowPeerAttribute.RETRY_CANCEL, sMsgNoConnection)
                        bExitLoop = iMsg == 0
                        bgetConnection = False
                    if not ((not bExitLoop)):
                        break
            if not bgetConnection:
                showMessageBox("ErrorBox", VclWindowPeerAttribute.OK, sMsgConnectionImpossible)
            else:
                self.xConnectionComponent = UnoRuntime.queryInterface(XComponent.__class__, self.DBConnection)
                self.m_connectionTools = UnoRuntime.queryInterface(XConnectionTools.__class__, self.DBConnection)
                self.getDataSourceObjects()
            return bgetConnection
        except Exception as e:
            showMessageBox("ErrorBox", VclWindowPeerAttribute.OK, sMsgConnectionImpossible)
            Logger.getLogger(DBMetaData.__class__.__name__).log(Level.SEVERE, None, e)
            return False'''

    def getMaxColumnNameLength(self):
        """ generated source for method getMaxColumnNameLength """
        try:
            if self.iMaxColumnNameLength <= 0:
                self.iMaxColumnNameLength = self.xDBMetaData.getMaxColumnNameLength()
            return self.iMaxColumnNameLength
        except SQLException as e:
            Logger.getLogger(DBMetaData.__class__.__name__).log(Level.SEVERE, None, e)
            return 0

    def getMaxTableNameLength(self):
        """ generated source for method getMaxTableNameLength """
        try:
            if self.iMaxTableNameLength <= 0:
                self.iMaxTableNameLength = self.xDBMetaData.getMaxTableNameLength()
            return self.iMaxTableNameLength
        except SQLException as e:
            Logger.getLogger(DBMetaData.__class__.__name__).log(Level.SEVERE, None, e)
            return 0

    def supportsPrimaryKeys(self):
        """ generated source for method supportsPrimaryKeys """
        supportsPrimaryKeys = False
        try:
            self.ensureDataSourceSettings()
            if AnyConverter.isVoid(primaryKeySupport):
                supportsPrimaryKeys = supportsCoreSQLGrammar()
            else:
                supportsPrimaryKeys = AnyConverter.toBoolean(primaryKeySupport)
        except Exception as ex:
            Logger.getLogger(DBMetaData.__class__.__name__).log(Level.SEVERE, None, ex)
        return supportsPrimaryKeys

    def supportsCoreSQLGrammar(self):
        """ generated source for method supportsCoreSQLGrammar """
        try:
            return self.xDBMetaData.supportsCoreSQLGrammar()
        except SQLException as e:
            Logger.getLogger(DBMetaData.__class__.__name__).log(Level.SEVERE, None, e)
            return False

    def supportsAutoIncrementation(self):
        """ generated source for method supportsAutoIncrementation """
        return False

    def supportsQueriesInFrom(self):
        """ generated source for method supportsQueriesInFrom """
        return self.m_connectionTools.getDataSourceMetaData().supportsQueriesInFrom()

    def suggestName(self, i_objectType, i_baseName):
        """ generated source for method suggestName """
        return self.m_connectionTools.getObjectNames().suggestName(i_objectType, i_baseName)

    def createQuery(self, _oSQLQueryComposer, _QueryName):
        """ generated source for method createQuery """
        try:
            xPSet.setPropertyValue(PropertyNames.COMMAND, s)
            self.m_connectionTools.getObjectNames().checkNameForCreate(com.sun.star.sdb.CommandType.QUERY, _QueryName)
            xNameCont.insertByName(_QueryName, oQuery)
            return True
        except WrappedTargetException as exception:
            try:
                sqlError = exception.TargetException
            except ClassCastException as castError:
                pass
            if sqlError != None:
                callSQLErrorMessageDialog(sqlError, None)
                return False
            Logger.getLogger(DBMetaData.__class__.__name__).log(Level.SEVERE, None, exception)
        except SQLException as e:
            callSQLErrorMessageDialog(e, None)
            return False
        except Exception as e:
            Logger.getLogger(DBMetaData.__class__.__name__).log(Level.SEVERE, None, e)
        return False

    def dispose(self):
        """ generated source for method dispose """
        if (self.DBConnection != None) and (self.bdisposeConnection):
            self.xConnectionComponent.dispose()

    def getReportDocuments(self):
        """ generated source for method getReportDocuments """
        xReportDocumentSuppl = UnoRuntime.queryInterface(XReportDocumentsSupplier.__class__, self.xModel)
        xReportDocumentSuppl.getReportDocuments()
        return UnoRuntime.queryInterface(XHierarchicalNameAccess.__class__, xReportDocumentSuppl.getReportDocuments())

    def getFormDocuments(self):
        """ generated source for method getFormDocuments """
        xFormDocumentSuppl = UnoRuntime.queryInterface(XFormDocumentsSupplier.__class__, self.xModel)
        return UnoRuntime.queryInterface(XHierarchicalNameAccess.__class__, xFormDocumentSuppl.getFormDocuments())

    def hasFormDocumentByName(self, _sFormName):
        """ generated source for method hasFormDocumentByName """
        xFormDocumentSuppl = UnoRuntime.queryInterface(XFormDocumentsSupplier.__class__, self.xModel)
        xFormNameAccess = UnoRuntime.queryInterface(XNameAccess.__class__, xFormDocumentSuppl.getFormDocuments())
        return xFormNameAccess.hasByName(_sFormName)

    def addFormDocument(self, _xComponent):
        """ generated source for method addFormDocument """
        _xFormDocNameAccess = self.getFormDocuments()
        addDatabaseDocument(_xComponent, _xFormDocNameAccess, False)

    def addReportDocument(self, _xComponent, _bcreatedynamicreport):
        """ generated source for method addReportDocument """
        xReportDocNameAccess = self.getReportDocuments()
        addDatabaseDocument(_xComponent, xReportDocNameAccess, _bcreatedynamicreport)

    def addDatabaseDocument(self, _xComponent, _xDocNameAccess, i_createTemplate):
        """ generated source for method addDatabaseDocument """
        try:
            xCloseable.close(False)
            creationArgs.put(PropertyNames.PROPERTY_NAME, basename)
            creationArgs.put(PropertyNames.URL, documentURL)
            creationArgs.put("AsTemplate", i_createTemplate)
            xHier.insertByHierarchicalName(sdocname, oDBDocument)
            xSimpleFileAccess.kill(documentURL)
        except Exception as e:
            Logger.getLogger(DBMetaData.__class__.__name__).log(Level.SEVERE, None, e)

    def createTypeInspector(self):
        """ generated source for method createTypeInspector """
        self.oTypeInspector = TypeInspector(self.xDBMetaData.getTypeInfo())

    def getDBDataTypeInspector(self):
        """ generated source for method getDBDataTypeInspector """
        return self.oTypeInspector

    def StringsFromResultSet(self, _xResultSet, _icol):
        """ generated source for method StringsFromResultSet """
        sColValues = None
        if _xResultSet == None:
            return sColValues
        try:
            while _xResultSet.next():
                aColVector.add(xRow.getString(_icol))
            sColValues = []
            aColVector.toArray(sColValues)
        except SQLException as e:
            Logger.getLogger(DBMetaData.__class__.__name__).log(Level.SEVERE, None, e)
        return sColValues

    def getCatalogNames(self):
        """ generated source for method getCatalogNames """
        try:
            return self.StringsFromResultSet(xResultSet, 1)
        except SQLException as e:
            Logger.getLogger(DBMetaData.__class__.__name__).log(Level.SEVERE, None, e)
            return None

    def getSchemaNames(self):
        """ generated source for method getSchemaNames """
        try:
            return self.StringsFromResultSet(xResultSet, 1)
        except SQLException as e:
            Logger.getLogger(DBMetaData.__class__.__name__).log(Level.SEVERE, None, e)
            return None

    def storeDatabaseDocumentToTempPath(self, _xcomponent, _storename):
        """ generated source for method storeDatabaseDocumentToTempPath """
        try:
            oStoreProperties[0] = Properties.createProperty("FilterName", "writer8")
            storepath += ".odt"
            xStoreable.storeAsURL(storepath, oStoreProperties)
            return True
        except Exception as e:
            Logger.getLogger(DBMetaData.__class__.__name__).log(Level.SEVERE, None, e)
            return False

    def showMessageBox(self, windowServiceName, windowAttribute, MessageText):
        """ generated source for method showMessageBox """
        if getWindowPeer() != None:
            return SystemDialog.showMessageBox(self.xMSF, self.xWindowPeer, windowServiceName, windowAttribute, MessageText)
        else:
            return SystemDialog.showMessageBox(self.xMSF, windowServiceName, windowAttribute, MessageText)

    def getWindowPeer(self):
        """ generated source for method getWindowPeer """
        return self.xWindowPeer

    def setWindowPeer(self, windowPeer):
        """ generated source for method setWindowPeer """
        self.xWindowPeer = windowPeer

    def callSQLErrorMessageDialog(self, oSQLException, _xWindow):
        """ generated source for method callSQLErrorMessageDialog """
        try:
            aPropertyValue[0] = Properties.createProperty("SQLException", oSQLException)
            aPropertyValue[1] = Properties.createProperty("ParentWindow", _xWindow)
            xInitialization.initialize(aPropertyValue)
            xExecutableDialog.execute()
        except com.sun.star.uno.Exception as ex:
            Logger.getLogger(getClass().__name__).log(Level.SEVERE, "error calling the error dialog", ex)

    def finish(self):
        """ generated source for method finish """
        self.xQueryNames = None
        self.xNameAccess = None
        self.xDatabaseContext = None
        self.xDBMetaData = None
        self.m_dataSource = None
        self.xModel = None
        self.xDataSourcePropertySet = None
        self.xWindowPeer = None
        self.DBConnection = None
        self.m_connectionTools = None
        self.xMSF = None
        self.xConnectionComponent = None
        self.CommandObjects = None

