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
from .CommandMetaData import CommandMetaData

class TableDescriptor(CommandMetaData):
    
    def __init__(self, xMSF, _sColumnAlreadyExistsMessage):
        super(TableDescriptor, self).__init__(xMSF)
        self.columncontainer = []
        self.keycolumncontainer = []
        self.sColumnAlreadyExistsMessage = _sColumnAlreadyExistsMessage

'''
    class ColumnDescriptor(object):
        """ generated source for class ColumnDescriptor """
        Name = str()
        xColPropertySet = XPropertySet()

        def __init__(self, _xColPropertySet, _Name):
            """ generated source for method __init__ """
            self.Name = _Name
            self.xColPropertySet = _xColPropertySet

    def getConnection(self, _curPropertyValue):
        """ generated source for method getConnection """
        if super(TableDescriptor, self).getConnection(_curPropertyValue):
            #  XTablesSupplier xDBTables = (XTablesSupplier) UnoRuntime.queryInterface(XTablesSupplier.class, DBConnection);
            #  xTableNames = xDBTables.getTables();
            self.xTableAppend = UnoRuntime.queryInterface(XAppend.__class__, getTableNamesAsNameAccess())
            self.xTableDrop = UnoRuntime.queryInterface(XDrop.__class__, getTableNamesAsNameAccess())
            self.xTableDataDescriptorFactory = UnoRuntime.queryInterface(XDataDescriptorFactory.__class__, getTableNamesAsNameAccess())
            self.xPropTableDataDescriptor = self.xTableDataDescriptorFactory.createDataDescriptor()
            self.xNameAccessColumns = xColumnsSupplier.getColumns()
            self.xColumnDataDescriptorFactory = UnoRuntime.queryInterface(XDataDescriptorFactory.__class__, self.xNameAccessColumns)
            try:
                createTypeInspector()
                self.sTableFilters = str(AnyConverter.toArray(getDataSourcePropertySet().getPropertyValue("TableFilter")))
            except Exception as e:
                e.printStackTrace(System.err)
            return True
        else:
            return False

    def removePrimaryKeys(self):
        """ generated source for method removePrimaryKeys """
        if len(self.keycolumncontainer) > 0:
            while i >= 0:
                self.keycolumncontainer.remove(i)
                i -= 1

    def createPrimaryKeys(self, _fieldnames, _bAutoincrementation):
        """ generated source for method createPrimaryKeys """
        try:
            self.xIndexAccessKeys = xKeySupplier.getKeys()
            self.xKeyDrop = UnoRuntime.queryInterface(XDrop.__class__, self.xIndexAccessKeys)
            self.xKeyAppend = UnoRuntime.queryInterface(XAppend.__class__, xKeyFac)
            self.xKey = xKeyFac.createDataDescriptor()
            self.xKey.setPropertyValue("Type", int(KeyType.PRIMARY))
            self.xKeyColumnSupplier = UnoRuntime.queryInterface(XColumnsSupplier.__class__, self.xKey)
            self.xKeyColAppend = UnoRuntime.queryInterface(XAppend.__class__, xKeyColFac)
            self.removePrimaryKeys()
            while len(_fieldnames):
                xKeyColPropertySet.setPropertyValue(PropertyNames.PROPERTY_NAME, _fieldnames[i])
                self.keycolumncontainer.add(xKeyColPropertySet)
                if hasByName(_fieldnames[i]):
                    xColPropertySet = getByName(_fieldnames[i])
                else:
                    xColPropertySet = addPrimaryKeyColumn(_fieldnames[i])
                xColPropertySet.setPropertyValue("IsNullable", int(com.sun.star.sdbc.ColumnValue.NO_NULLS))
                if _bAutoincrementation:
                    if nDataType != oTypeInspector.INVALID:
                        if xColPropertySet.getPropertySetInfo().hasPropertyByName("IsAutoIncrement"):
                            xColPropertySet.setPropertyValue("Type", int(nDataType))
                            xColPropertySet.setPropertyValue("IsAutoIncrement", Boolean.valueOf(_bAutoincrementation))
                modifyColumn(_fieldnames[i], xColPropertySet)
                i += 1
            return True
        except UnknownPropertyException as e:
            e.printStackTrace(System.err)
        except PropertyVetoException as e:
            e.printStackTrace(System.err)
        except IllegalArgumentException as e:
            e.printStackTrace(System.err)
        except WrappedTargetException as e:
            e.printStackTrace(System.err)
        return False

    def isColunnNameDuplicate(self, _xColumns, _xToBeAppendedPropertySet):
        """ generated source for method isColunnNameDuplicate """
        try:
            if _xColumns.hasByName(sColumnName):
                showMessageBox("ErrorBox", VclWindowPeerAttribute.OK, sMessage)
                return True
            return False
        except Exception as ex:
            ex.printStackTrace(System.err)
            return False

    # 
    #      * creates the table under the passed name
    #      * @param _catalogname
    #      * @param _schemaname
    #      * @param _tablename is made unique if necessary
    #      * @param _fieldnames
    #      * @return true or false to indicate successful creation or not
    #      
    @overloaded
    def createTable(self, _catalogname, _schemaname, _tablename, _fieldnames):
        """ generated source for method createTable """
        breturn = True
        try:
            while i < len(self.columncontainer):
                if not self.isColunnNameDuplicate(self.xNameAccessColumns, xColPropertySet):
                    xAppendColumns.appendByDescriptor(xColPropertySet)
                    # xColPropertySet.setPropertyValue("Type", 32423)
                else:
                    breturn = False
                i += 1
            if breturn:
                assignTableProperty(PropertyNames.PROPERTY_NAME, _tablename)
                assignTableProperty("CatalogName", _catalogname)
                assignTableProperty("SchemaName", _schemaname)
                self.xTableContainer = UnoRuntime.queryInterface(XContainer.__class__, getTableNamesAsNameAccess())
                self.xTableContainer.addContainerListener(self)
                if len(self.keycolumncontainer) > 0:
                    while i < len(self.keycolumncontainer):
                        if not self.isColunnNameDuplicate(self.xKeyColumnSupplier.getColumns(), xKeyColPropertySet):
                            self.xKeyColAppend.appendByDescriptor(xKeyColPropertySet)
                        else:
                            breturn = False
                        i += 1
                    if breturn:
                        self.xKeyAppend.appendByDescriptor(self.xKey)
                if breturn:
                    #  TODO: LLA: describe what is he doing here.
                    self.xTableAppend.appendByDescriptor(self.xPropTableDataDescriptor)
        except SQLException as oSQLException:
            super(TableDescriptor, self).callSQLErrorMessageDialog(oSQLException, self.xWindow)
            breturn = False
        except Exception as e:
            e.printStackTrace(System.err)
            breturn = False
        if not breturn:
            removeAllColumnsFromDescriptor(_tablename)
            self.removePrimaryKeys()
        return breturn

    def removeAllColumnsFromDescriptor(self, _tablename):
        """ generated source for method removeAllColumnsFromDescriptor """
        try:
            self.xPropTableDataDescriptor.setPropertyValue(PropertyNames.PROPERTY_NAME, PropertyNames.EMPTY_STRING)
            if (self.xKeyDrop != None) and (self.xIndexAccessKeys != None):
                if icount > 0:
                    while i >= 0:
                        self.xKeyDrop.dropByIndex(i)
                        i -= 1
            while i >= 0:
                xColumnDrop.dropByIndex(i)
                i -= 1
            if self.xTableDrop != None:
                if getTableNamesAsNameAccess().hasByName(_tablename):
                    self.xTableDrop.dropByName(_tablename)
            if self.bIDFieldisInserted:
                self.dropColumnbyName(self.IDFieldName)
                self.bIDFieldisInserted = False
            return False
        except SQLException as oSQLException:
            super(TableDescriptor, self).callSQLErrorMessageDialog(oSQLException, self.xWindow)
        except Exception as e1:
            e1.printStackTrace(System.err)
        return False

    @createTable.register(object, str, str, str, str, bool, str)
    def createTable_0(self, _catalogname, _schemaname, _tablename, _keycolumnnames, _bAutoincrementation, _fieldnames):
        """ generated source for method createTable_0 """
        if self.createPrimaryKeys(_keycolumnnames, _bAutoincrementation):
            return self.createTable(_catalogname, _schemaname, _tablename, _fieldnames)
        return False

    def assignTableProperty(self, _spropname, _svalue):
        """ generated source for method assignTableProperty """
        if _svalue != None:
            if not _svalue == PropertyNames.EMPTY_STRING:
                try:
                    self.xPropTableDataDescriptor.setPropertyValue(_spropname, _svalue)
                except Exception as e:
                    e.printStackTrace(System.err)

    def modifyColumnName(self, _soldname, _snewname):
        """ generated source for method modifyColumnName """
        try:
            return modifyColumn(_soldname, PropertyNames.PROPERTY_NAME, _snewname)
        except Exception as e:
            e.printStackTrace(System.err)
            showMessageBox("ErrorBox", VclWindowPeerAttribute.OK, e.getMessage())
            return False

    @overloaded
    def modifyColumn(self, _sname, _spropname, _oValue):
        """ generated source for method modifyColumn """
        try:
            if len(self.columncontainer) > 0:
                while i < len(self.columncontainer):
                    if oColumnDescriptor.Name == _sname:
                        oColumnDescriptor.xColPropertySet.setPropertyValue(_spropname, _oValue)
                        if _spropname == PropertyNames.PROPERTY_NAME:
                            oColumnDescriptor.Name = str(_oValue)
                        self.columncontainer.remove(i)
                        self.columncontainer.add(i, oColumnDescriptor)
                        return True
                    i += 1
        except UnknownPropertyException as e:
            e.printStackTrace(System.err)
        except PropertyVetoException as e:
            e.printStackTrace(System.err)
        except IllegalArgumentException as e:
            e.printStackTrace(System.err)
        except WrappedTargetException as e:
            e.printStackTrace(System.err)
        return False

    @modifyColumn.register(object, str, XPropertySet)
    def modifyColumn_0(self, _sname, _xColPropertySet):
        """ generated source for method modifyColumn_0 """
        try:
            if len(self.columncontainer) > 0:
                while i < len(self.columncontainer):
                    if oColumnDescriptor.Name == _sname:
                        oColumnDescriptor.xColPropertySet = _xColPropertySet
                        oColumnDescriptor.Name = str(_xColPropertySet.getPropertyValue(PropertyNames.PROPERTY_NAME))
                        self.columncontainer.remove(i)
                        self.columncontainer.add(i, oColumnDescriptor)
                        return True
                    i += 1
        except Exception as e:
            e.printStackTrace(System.err)
        return False

    def dropColumnbyName(self, _sname):
        """ generated source for method dropColumnbyName """
        try:
            if len(self.columncontainer) > 0:
                while i < len(self.columncontainer):
                    if oColumnDescriptor != None:
                        if oColumnDescriptor.Name == _sname:
                            self.columncontainer.remove(i)
                    i += 1
        except Exception as e:
            e.printStackTrace(System.err)

    def getColumnNames(self):
        """ generated source for method getColumnNames """
        if len(self.columncontainer) > 0:
            try:
                while i < len(self.columncontainer):
                    fieldnames[i] = oColumnDescriptor.Name
                    i += 1
                return fieldnames
            except RuntimeException as e:
                e.printStackTrace(System.err)
        return []

    def hasByName(self, _fieldname):
        """ generated source for method hasByName """
        try:
            if len(self.columncontainer) > 0:
                while i < len(self.columncontainer):
                    if oColumnDescriptor.Name == _fieldname:
                        return True
                    i += 1
        except RuntimeException as e:
            e.printStackTrace(System.err)
        return False

    def getColumnDescriptorByName(self, _fieldname):
        """ generated source for method getColumnDescriptorByName """
        try:
            if len(self.columncontainer) > 0:
                while i < len(self.columncontainer):
                    if oColumnDescriptor.Name == _fieldname:
                        return oColumnDescriptor
                    i += 1
        except RuntimeException as e:
            e.printStackTrace(System.err)
        return None

    def getByName(self, _fieldname):
        """ generated source for method getByName """
        oColumnDescriptor = self.getColumnDescriptorByName(_fieldname)
        if oColumnDescriptor != None:
            return oColumnDescriptor.xColPropertySet
        else:
            return None

    def getByIndex(self, _index):
        """ generated source for method getByIndex """
        try:
            if len(self.columncontainer) > _index:
                return oColumnDescriptor.xColPropertySet
        except RuntimeException as e:
            e.printStackTrace(System.err)
        return None

    def clonePropertySet(self, _snewname, _xnewPropertySet):
        """ generated source for method clonePropertySet """
        xRetPropertySet = self.xColumnDataDescriptorFactory.createDataDescriptor()
        try:
            if self.hasByName(_snewname):
                while len(aColProperties):
                    xRetPropertySet.setPropertyValue(sPropName, oColValue)
                    i += 1
        except Exception as e:
            e.printStackTrace(System.err)
        return xRetPropertySet

    @overloaded
    def addColumn(self, _aNewPropertyValues):
        """ generated source for method addColumn """
        try:
            if not self.hasByName(sname):
                oPropertySet.assignPropertyValues(_aNewPropertyValues, True)
                self.columncontainer.add(oColumnDescriptor)
                return True
        except Exception as e:
            e.printStackTrace(System.err)
        return False

    def moveColumn(self, _nOldIndex, _nNewIndex):
        """ generated source for method moveColumn """
        try:
            self.columncontainer.remove(_nOldIndex)
            self.columncontainer.add(_nNewIndex, oColumnDescriptor)
            return True
        except Exception as e:
            e.printStackTrace(System.err)
            return False

    @addColumn.register(object, str, XPropertySet)
    def addColumn_0(self, _columnname, _xNewColPropertySet):
        """ generated source for method addColumn_0 """
        try:
            if not self.hasByName(_columnname):
                if _columnname == PropertyNames.EMPTY_STRING:
                    return False
                else:
                    oPropertySet.assignNewPropertySet(_columnname, _xNewColPropertySet)
                    self.columncontainer.add(oColumnDescriptor)
                    return True
        except Exception as e:
            e.printStackTrace(System.err)
        return False

    def addPrimaryKeyColumn(self, _columnname):
        """ generated source for method addPrimaryKeyColumn """
        try:
            if not self.hasByName(_columnname):
                try:
                    self.IDFieldName = Desktop.getUniqueName(self.getColumnNames(), _columnname, PropertyNames.EMPTY_STRING)
                    xColPropertySet.setPropertyValue(PropertyNames.PROPERTY_NAME, self.IDFieldName)
                    xColPropertySet.setPropertyValue("Type", int(nDataType))
                    xColPropertySet.setPropertyValue("TypeName", oTypeInspector.getDefaultTypeName(nDataType, None))
                    self.columncontainer.add(0, oColumnDescriptor)
                    self.bIDFieldisInserted = True
                    return xColPropertySet
                except RuntimeException as e1:
                    e1.printStackTrace(System.err)
        except Exception as e:
            e.printStackTrace(System.err)
        return None

    def getNonBinaryFieldNames(self):
        """ generated source for method getNonBinaryFieldNames """
        NonBinaryFieldNameVector = ArrayList()
        try:
            while i < len(self.columncontainer):
                try:
                    itype = AnyConverter.toInt(xColPropertySet.getPropertyValue("Type"))
                    if not isBinaryDataType(itype):
                        NonBinaryFieldNameVector.add(oColumnDescriptor.Name)
                except Exception as e:
                    e.printStackTrace(System.err)
                i += 1
        except RuntimeException as e:
            e.printStackTrace(System.err)
        sbinaryfieldnames = [None]*len(NonBinaryFieldNameVector)
        NonBinaryFieldNameVector.toArray(sbinaryfieldnames)
        return sbinaryfieldnames

    @overloaded
    def getComposedTableName(self, _scatalogname, _sschemaname, _stablename):
        """ generated source for method getComposedTableName """
        self.ComposedTableName = CommandName(self, _scatalogname, _sschemaname, _stablename, False)
        return self.ComposedTableName.getComposedName()

    @getComposedTableName.register(object)
    def getComposedTableName_0(self):
        """ generated source for method getComposedTableName_0 """
        if self.ComposedTableName != None:
            return self.ComposedTableName.getComposedName()
        else:
            return None

    #  (non-Javadoc)
    #      * @see com.sun.star.container.XContainerListener#elementInserted(com.sun.star.container.ContainerEvent)
    #      
    def elementInserted(self, arg0):
        """ generated source for method elementInserted """
        try:
            self.ComposedTableName = CommandName(self, scatalogname, sschemaname, stablename, False)
            appendTableNameToFilter(self.ComposedTableName.getComposedName())
        except Exception as e:
            e.printStackTrace(System.err)

    #  (non-Javadoc)
    #      * @see com.sun.star.container.XContainerListener#elementRemoved(com.sun.star.container.ContainerEvent)
    #      
    def elementRemoved(self, arg0):
        """ generated source for method elementRemoved """

    #  (non-Javadoc)
    #      * @see com.sun.star.container.XContainerListener#elementReplaced(com.sun.star.container.ContainerEvent)
    #      
    def elementReplaced(self, arg0):
        """ generated source for method elementReplaced """

    #  (non-Javadoc)
    #      * @see com.sun.star.lang.XEventListener#disposing(com.sun.star.lang.EventObject)
    #      
    def disposing(self, arg0):
        """ generated source for method disposing """

    # 
    #      * @param _scomposedtablename
    #      * @return
    #      
    def appendTableNameToFilter(self, _scomposedtablename):
        """ generated source for method appendTableNameToFilter """
        bhastoinsert = True
        i = 0
        while len(sTableFilters):
            if self.sTableFilters[i].compareTo("%") > -1:
                if self.sTableFilters[i].endsWith("." + _scomposedtablename):
                    bhastoinsert = False
                elif 1 == len(length):
                    bhastoinsert = False
            elif self.sTableFilters[i] == _scomposedtablename:
                bhastoinsert = False
            if not bhastoinsert:
                break
            i += 1
        if bhastoinsert:
            System.arraycopy(self.sTableFilters, 0, sNewTableFilters, 0, )
            sNewTableFilters[len(sTableFilters)] = _scomposedtablename
            self.sTableFilters = sNewTableFilters
            try:
                getDataSourcePropertySet().setPropertyValue("TableFilter", self.sTableFilters)
            except Exception as e:
                e.printStackTrace(System.err)
                bhastoinsert = False
        return bhastoinsert
'''
