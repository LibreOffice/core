/*************************************************************************
 *
 *  RCSfile: TableDescriptor.java,v
 *
 *  Revision: 1.0
 *
 *  last change: Author: bc  Date: Jul 11, 2004 2:44:43 PM
 *
 *  The Contents of this file are made available subject to the terms of
 *  the BSD license.
 *
 *  Copyright (c) 2003 by Sun Microsystems, Inc.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of Sun Microsystems, Inc. nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 *  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 *  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 *  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *************************************************************************/
package com.sun.star.wizards.db;

import java.util.Vector;

import com.sun.star.awt.VclWindowPeerAttribute;
import com.sun.star.beans.Property;
import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.PropertyVetoException;
import com.sun.star.beans.UnknownPropertyException;
import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XIndexAccess;
import com.sun.star.container.XNameAccess;
import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.lang.IndexOutOfBoundsException;
import com.sun.star.lang.WrappedTargetException;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sdbc.SQLException;
import com.sun.star.sdbcx.KeyType;
import com.sun.star.sdbcx.XAppend;
import com.sun.star.sdbcx.XColumnsSupplier;
import com.sun.star.sdbcx.XDataDescriptorFactory;
import com.sun.star.sdbcx.XDrop;
import com.sun.star.sdbcx.XKeysSupplier;
import com.sun.star.sdbcx.XTablesSupplier;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.util.XRefreshable;
import com.sun.star.wizards.common.Desktop;
import com.sun.star.wizards.common.Properties;
import com.sun.star.wizards.common.SystemDialog;


public class TableDescriptor extends CommandMetaData {
    XDataDescriptorFactory xTableDataDescriptorFactory;
    XPropertySet xPropTableDataDescriptor;
    XNameAccess xNameAccessColumns;
    XIndexAccess xIndexAccessKeys;
    XAppend xAppendField;
    public XDataDescriptorFactory xColumnDataDescriptorFactory;
    XAppend xTableAppend;
    XDrop   xTableDrop;
    XDrop xColumnDrop;
    XDrop xKeyDrop;
    XRefreshable xRefreshable;
    public Vector tempcolumncontainer;

        /**
         * @param xMSF
         */
        public TableDescriptor(XMultiServiceFactory xMSF) {
            super(xMSF);
            this.tempcolumncontainer = new Vector();
        }


        public boolean getConnection(PropertyValue[] _curPropertyValue){
            if (super.getConnection(_curPropertyValue)){
                XTablesSupplier xDBTables = (XTablesSupplier) UnoRuntime.queryInterface(XTablesSupplier.class, DBConnection);
                xTableNames =  xDBTables.getTables();
                xTableAppend = (XAppend) UnoRuntime.queryInterface(XAppend.class, xTableNames);
                xTableDrop = (XDrop) UnoRuntime.queryInterface(XDrop.class, xTableNames);
                xTableDataDescriptorFactory = (XDataDescriptorFactory) UnoRuntime.queryInterface(XDataDescriptorFactory.class, xTableNames);
                xPropTableDataDescriptor = xTableDataDescriptorFactory.createDataDescriptor();
                XColumnsSupplier xColumnsSupplier = (XColumnsSupplier) UnoRuntime.queryInterface(XColumnsSupplier.class, xPropTableDataDescriptor);
                xNameAccessColumns = xColumnsSupplier.getColumns();
                xColumnDrop = (XDrop) UnoRuntime.queryInterface(XDrop.class, xNameAccessColumns);
                xColumnDataDescriptorFactory = (XDataDescriptorFactory) UnoRuntime.queryInterface(XDataDescriptorFactory.class, xNameAccessColumns);
                xAppendField = (XAppend) UnoRuntime.queryInterface(XAppend.class, xNameAccessColumns);
                xRefreshable = (XRefreshable) UnoRuntime.queryInterface(XRefreshable.class, xNameAccessColumns);
                return true;
            }
            else
                return false;
        }


        /**
         * creates the table under the passed name
         * @param _tablename is made unique if necessary
         * @return true or false to indicate successful creation or not
         */
        public boolean createTable(String _tablename, String[] _fieldnames){
            String stablename = "";
            try {
                for (int i = 0; i < _fieldnames.length; i++){
                    if (xNameAccessColumns.hasByName(_fieldnames[i])){
                        XPropertySet xcurPropertySet = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xNameAccessColumns.getByName(_fieldnames[i]) );
                        xColumnDrop.dropByName(_fieldnames[i]);
                        xAppendField.appendByDescriptor(xcurPropertySet);
                    }
                }
                stablename = Desktop.getUniqueName(xTableNames, _tablename);
                xPropTableDataDescriptor.setPropertyValue("Name", stablename);
                xTableAppend.appendByDescriptor(xPropTableDataDescriptor);
                return true;
            } catch (Exception e) {
                e.printStackTrace(System.out);
                showMessageBox("ErrorBox", VclWindowPeerAttribute.OK, e.getLocalizedMessage());
                try {
                    xPropTableDataDescriptor.setPropertyValue("Name", "");
                    if ((xKeyDrop != null) && (xIndexAccessKeys != null)){
                        int icount = xIndexAccessKeys.getCount();
                        if (icount > 0){
                            for (int i = xIndexAccessKeys.getCount()-1; i >= 0; i--){
                                xKeyDrop.dropByIndex(i);
                            }
                        }
                    }
                    if (xTableDrop != null)
                        if (xTableNames.hasByName(stablename))
                            xTableDrop.dropByName(stablename);
                } catch (Exception e1) {
                    e1.printStackTrace(System.out);
                }
                return false;
            }
        }

        public boolean createTable(String _tablename, String[] _keycolumnnames, boolean _bAutoincrementation, String[] _fieldnames){
            if (createPrimaryKeys(_keycolumnnames, _bAutoincrementation))
                return createTable(_tablename, _fieldnames);
            return false;
        }


        public boolean modifyColumnName(String _soldname, String _snewname){
        try {
            if (xNameAccessColumns.hasByName(_soldname)){
                XPropertySet xPropertySet = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xNameAccessColumns.getByName(_soldname) );
                if (addColumn(_snewname, xPropertySet)){
                    xColumnDrop.dropByName(_soldname);
                    return true;
                }
                else
                    return false;
            }
            return true;
        } catch (Exception e) {
            e.printStackTrace(System.out);
            showMessageBox("ErrorBox", VclWindowPeerAttribute.OK, e.getLocalizedMessage());
            return false;
        }}


        public boolean modifyColumn(String _sname, XPropertySet _xColPropertySet){
            try {
                if (xNameAccessColumns.hasByName(_sname)){
                    xColumnDrop.dropByName(_sname);
                    addColumn(_sname, _xColPropertySet);
                    return true;
                }
            } catch (Exception e) {
                e.printStackTrace(System.out);
                showMessageBox("ErrorBox", VclWindowPeerAttribute.OK, e.getLocalizedMessage());
            }
            return false;
        }

        public XPropertySet getColumnbyName(String _sname){
        try {
            if (xNameAccessColumns.hasByName(_sname));
                return (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xNameAccessColumns.getByName(_sname));
        } catch (Exception e) {
            e.printStackTrace(System.out);
        }
        return null;
        }


        public void dropColumnbyName(String _sname){
        try {
            String[] snames = xNameAccessColumns.getElementNames();
            if (snames.length > 0){
                if (xNameAccessColumns.hasByName(_sname));
                    xColumnDrop.dropByName(_sname);
            }
        } catch (Exception e) {
            e.printStackTrace(System.out);
        }}


        public String[] getFieldnames(){
            String[] fieldnames = xNameAccessColumns.getElementNames();
            return fieldnames;
        }


        public XPropertySet assignPropertyValues(XPropertySet _xNewColPropertySet, String _sNewName){
        try {
            XPropertySet xRetPropertySet = xColumnDataDescriptorFactory.createDataDescriptor();
            Property[] aNewColProperties = _xNewColPropertySet.getPropertySetInfo().getProperties();
            for (int i = 0; i < aNewColProperties.length; i++){
                String sPropName = aNewColProperties[i].Name;
                Object oColValue = _xNewColPropertySet.getPropertyValue(sPropName);
                if (sPropName.equals("Type")){
                    int iType = AnyConverter.toInt(oColValue);
                    iType = convertDataType(iType);
                    xRetPropertySet.setPropertyValue("Type",new Integer(iType));
                }
                else if ((sPropName.equals("Name")) && (!_sNewName.equals(""))){
                    xRetPropertySet.setPropertyValue("Name", _sNewName);
                }
                else
                    xRetPropertySet.setPropertyValue(sPropName, oColValue);
            }
            return xRetPropertySet;
        } catch (Exception e) {
            e.printStackTrace(System.out);
            return null;
        }}


    //TODO it would be best to retrieve the properties via introspection from the dbfields
        public boolean addColumn(String _columnname, XPropertySet _xNewColPropertySet){
        try {
            if (!xNameAccessColumns.hasByName(_columnname)){
                if (_columnname.equals(""))
                    return false;
                else{
                    XPropertySet xColPropertySet = assignPropertyValues(_xNewColPropertySet, _columnname);
                    if (xColPropertySet != null){
                        xAppendField.appendByDescriptor(xColPropertySet);
                        return true;
                    }
                }
            }
        } catch (Exception e) {
            e.printStackTrace(System.out);
            showMessageBox("ErrorBox", VclWindowPeerAttribute.OK, e.getLocalizedMessage());
        }
        return false;
        }


        private Integer assignTypePropertyValue(Object _oPropertyValue){
        try {
            int itype = AnyConverter.toInt(_oPropertyValue);
            itype = super.convertDataType(itype);
            return new Integer(itype);
        } catch (Exception e) {
            e.printStackTrace(System.out);
            return new Integer(super.getLastConversionFallbackDataType());
        }}


        public boolean addColumn(PropertyValue[] _aNewPropertyValues){
        try {
            String sname = (String) Properties.getPropertyValue(_aNewPropertyValues, "Name");
            if (!xNameAccessColumns.hasByName(sname)){
                XPropertySet xColPropertySet = xColumnDataDescriptorFactory.createDataDescriptor();
                for (int i = 0; i < _aNewPropertyValues.length; i++){
                    if (_aNewPropertyValues[i].Name.equals("Type"))
                        xColPropertySet.setPropertyValue("Type", assignTypePropertyValue(_aNewPropertyValues[i].Value));
                    else
                        xColPropertySet.setPropertyValue(_aNewPropertyValues[i].Name, _aNewPropertyValues[i].Value);
                }
                xAppendField.appendByDescriptor(xColPropertySet);
                return true;
            }
        } catch (Exception e) {
            e.printStackTrace(System.out);
        }
        return false;
        }


        public boolean addPrimaryKeyColumn(String _columnname){
        try {
            if (!xNameAccessColumns.hasByName(_columnname)){
                XPropertySet xColPropertySet = xColumnDataDescriptorFactory.createDataDescriptor();
                xColPropertySet.setPropertyValue("Name", _columnname);
                xColPropertySet.setPropertyValue("Type", new Integer(super.convertDataType(com.sun.star.sdbc.DataType.INTEGER)));
                xAppendField.appendByDescriptor(xColPropertySet);
                return true;
            }
        } catch (Exception e) {
            e.printStackTrace(System.out);
            showMessageBox("ErrorBox", VclWindowPeerAttribute.OK, e.getLocalizedMessage());
        }
            return false;
        }


       public boolean createPrimaryKeys(String[] _columnnames, boolean _bAutoincrementation){
        try {
           XKeysSupplier xKeySupplier = (XKeysSupplier)UnoRuntime.queryInterface(XKeysSupplier.class, xPropTableDataDescriptor);
           xIndexAccessKeys = xKeySupplier.getKeys();
           XDataDescriptorFactory xKeyFac = (XDataDescriptorFactory)UnoRuntime.queryInterface(XDataDescriptorFactory.class,xIndexAccessKeys);
           xKeyDrop = (XDrop) UnoRuntime.queryInterface(XDrop.class, xIndexAccessKeys);
           XAppend xKeyAppend = (XAppend)UnoRuntime.queryInterface(XAppend.class, xKeyFac);
           XPropertySet xKey = xKeyFac.createDataDescriptor();
           xKey.setPropertyValue("Type", new Integer(KeyType.PRIMARY));
           XColumnsSupplier xKeyColumSup = (XColumnsSupplier)UnoRuntime.queryInterface(XColumnsSupplier.class, xKey);
           XDataDescriptorFactory xKeyColFac = (XDataDescriptorFactory)UnoRuntime.queryInterface(XDataDescriptorFactory.class,xKeyColumSup.getColumns());
           XAppend xKeyColAppend = (XAppend)UnoRuntime.queryInterface(XAppend.class, xKeyColFac);
           for (int i = 0; i < _columnnames.length; i++){
                  XPropertySet xKeyCol = xKeyColFac.createDataDescriptor();
                       if (!xNameAccessColumns.hasByName(_columnnames[i]))
                           addPrimaryKeyColumn(_columnnames[i]);
                   xKeyCol.setPropertyValue("Name", _columnnames[i]);
                xKeyCol.setPropertyValue("IsNullable", new Integer(com.sun.star.sdbc.ColumnValue.NO_NULLS));

                   if (supportsAutoIncrementation()){
                   if (xKeyCol.getPropertySetInfo().hasPropertyByName("IsAutoIncrement"))
                           xKeyCol.setPropertyValue("IsAutoIncrement", new Boolean(_bAutoincrementation));
                   }
                xKeyColAppend.appendByDescriptor(xKeyCol);
           }
           xKeyAppend.appendByDescriptor(xKey);
           return true;
        } catch (Exception e) {
            showMessageBox("ErrorBox", VclWindowPeerAttribute.OK, e.getLocalizedMessage());
            e.printStackTrace(System.out);
        }
            return false;
        }
    }   