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
import com.sun.star.container.ContainerEvent;
import com.sun.star.container.NoSuchElementException;
import com.sun.star.container.XContainer;
import com.sun.star.container.XContainerListener;
import com.sun.star.container.XHierarchicalNameAccess;
import com.sun.star.container.XIndexAccess;
import com.sun.star.container.XNameAccess;
import com.sun.star.lang.EventObject;
import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.lang.WrappedTargetException;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sdbc.ColumnValue;
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


public class TableDescriptor extends CommandMetaData  implements XContainerListener{
    XDataDescriptorFactory xTableDataDescriptorFactory;
    XPropertySet xPropTableDataDescriptor;
    XNameAccess xNameAccessColumns;
    XIndexAccess xIndexAccessKeys;
    XAppend xAppendColumns;
    public XDataDescriptorFactory xColumnDataDescriptorFactory;
    XContainer xTableContainer;
    XAppend xTableAppend;
    XDrop   xTableDrop;
    XDrop xColumnDrop;
    XDrop xKeyDrop;
    XRefreshable xRefreshable;
    private String[] sTableFilters = null;
    public Vector tempcolumncontainer;
    public XHierarchicalNameAccess xTableHierarchicalNameAccess;
    private CommandName ComposedTableName;

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
                xAppendColumns = (XAppend) UnoRuntime.queryInterface(XAppend.class, xNameAccessColumns);
                xRefreshable = (XRefreshable) UnoRuntime.queryInterface(XRefreshable.class, xNameAccessColumns);
                try {
                    createTypeInspector();
                    sTableFilters = (String[]) AnyConverter.toArray(xDataSourcePropertySet.getPropertyValue("TableFilter"));
                } catch (Exception e) {
                    e.printStackTrace(System.out);
                }
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
        public boolean createTable(String _catalogname, String _schemaname, String _tablename, String[] _fieldnames){
            try {
                for (int i = 0; i < _fieldnames.length; i++){
                    if (xNameAccessColumns.hasByName(_fieldnames[i])){
                        XPropertySet xcurPropertySet = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xNameAccessColumns.getByName(_fieldnames[i]) );
                        xColumnDrop.dropByName(_fieldnames[i]);
                        xAppendColumns.appendByDescriptor(xcurPropertySet);
                    }
                }
                assignTableProperty("Name", _tablename);
                assignTableProperty("CatalogName", _catalogname);
                assignTableProperty("SchemaName", _schemaname);
                xTableContainer = (XContainer) UnoRuntime.queryInterface(XContainer.class, xTableNames);
                xTableContainer.addContainerListener(this);
                xTableAppend.appendByDescriptor(xPropTableDataDescriptor);
                return true;
            } catch (Exception e) {
                e.printStackTrace(System.out);
                showMessageBox("ErrorBox", VclWindowPeerAttribute.OK, e.getMessage());
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
                        if (xTableNames.hasByName(_tablename))
                            xTableDrop.dropByName(_tablename);
                } catch (Exception e1) {
                    e1.printStackTrace(System.out);
                }
                return false;
            }
        }


        public boolean createTable(String _catalogname, String _schemaname, String _tablename, String[] _keycolumnnames, boolean _bAutoincrementation, String[] _fieldnames){
            if (createPrimaryKeys(_keycolumnnames, _bAutoincrementation))
                return createTable(_catalogname, _schemaname, _tablename, _fieldnames);
            return false;
        }


        private void assignTableProperty(String _spropname, String _svalue){
            if (_svalue != null){
                if (!_svalue.equals("")){
                    try {
                        xPropTableDataDescriptor.setPropertyValue(_spropname, _svalue);
                    } catch (Exception e) {
                        e.printStackTrace(System.out);
                    }
                }
            }
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
            if (!_sname.equals("")){
                if (xNameAccessColumns.hasByName(_sname));
                    return (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xNameAccessColumns.getByName(_sname));
            }
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


        public XPropertySet clonePropertySet(String _snewname, XPropertySet _xnewPropertySet){
            XPropertySet xRetPropertySet = xColumnDataDescriptorFactory.createDataDescriptor();
            try {
                if (xNameAccessColumns.hasByName(_snewname)){
                    Object oColumn = xNameAccessColumns.getByName(_snewname);
                    XPropertySet xPropertySet = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, oColumn);
                    Property[] aColProperties = xPropertySet.getPropertySetInfo().getProperties();
                    for (int i = 0; i < aColProperties.length; i++){
                        String sPropName = aColProperties[i].Name;
                        Object oColValue = _xnewPropertySet.getPropertyValue(sPropName);
                        xRetPropertySet.setPropertyValue(sPropName, oColValue);
                    }
                }
            } catch (Exception e) {
                e.printStackTrace(System.out);
            }
            return xRetPropertySet;
        }


        public boolean addColumn(PropertyValue[] _aNewPropertyValues){
        try {
            String sname = (String) Properties.getPropertyValue(_aNewPropertyValues, "Name");
            if (!xNameAccessColumns.hasByName(sname)){
                ColumnPropertySet oPropertySet = new ColumnPropertySet(oTypeInspector, xColumnDataDescriptorFactory.createDataDescriptor());
                oPropertySet.assignPropertyValues(_aNewPropertyValues, true);
                xAppendColumns.appendByDescriptor(oPropertySet.xPropertySet);
                return true;
            }
        } catch (Exception e) {
            e.printStackTrace(System.out);
        }
        return false;
        }


        public boolean addColumn(String _columnname, XPropertySet _xNewColPropertySet){
        try {
            if (!xNameAccessColumns.hasByName(_columnname)){
                if (_columnname.equals(""))
                    return false;
                else{
                    ColumnPropertySet oPropertySet = new ColumnPropertySet(oTypeInspector, xColumnDataDescriptorFactory.createDataDescriptor());
                    oPropertySet.assignNewPropertySet(_columnname, _xNewColPropertySet);
                    xAppendColumns.appendByDescriptor(oPropertySet.xPropertySet);
                    return true;
                }
            }
        } catch (Exception e) {
            e.printStackTrace(System.out);
            showMessageBox("ErrorBox", VclWindowPeerAttribute.OK, e.getLocalizedMessage());
        }
        return false;
        }



        public boolean addPrimaryKeyColumn(String _columnname){
        try {
            if (!xNameAccessColumns.hasByName(_columnname)){
                XPropertySet xColPropertySet = xColumnDataDescriptorFactory.createDataDescriptor();
                xColPropertySet.setPropertyValue("Name", _columnname);
                xColPropertySet.setPropertyValue("Type", new Integer(oTypeInspector.convertDataType(com.sun.star.sdbc.DataType.INTEGER)));
                xAppendColumns.appendByDescriptor(xColPropertySet);
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


       public String[] getNonBinaryFieldNames(){
            Vector NonBinaryFieldNameVector = new Vector();
            String[] scolnames = xNameAccessColumns.getElementNames();
            for (int i = 0; i < scolnames.length; i++){
                XPropertySet xColPropertySet = getColumnbyName(scolnames[i]);
                Property[] aProperties = xColPropertySet.getPropertySetInfo().getProperties();
                int itype;
                try {
                    itype = AnyConverter.toInt(xColPropertySet.getPropertyValue("Type"));
                    if (!isBinaryDataType(itype))
                        NonBinaryFieldNameVector.addElement(scolnames[i]);
                } catch (Exception e) {
                    e.printStackTrace(System.out);
                }
            }
            String[] sbinaryfieldnames = new String[NonBinaryFieldNameVector.size()];
            NonBinaryFieldNameVector.toArray(sbinaryfieldnames);
            return sbinaryfieldnames;
        }


           public String getComposedTableName(String _scatalogname, String _sschemaname, String _stablename){
            ComposedTableName = new CommandName(this, _scatalogname, _sschemaname, _stablename, false);
            return ComposedTableName.getComposedName();
           }


           public String getComposedTableName(){
               if (ComposedTableName != null)
                   return this.ComposedTableName.getComposedName();
               else
                   return null;
           }


               /* (non-Javadoc)
         * @see com.sun.star.container.XContainerListener#elementInserted(com.sun.star.container.ContainerEvent)
         */
        public void elementInserted(ContainerEvent arg0) {
        try {
            XPropertySet xTablePropertySet = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, arg0.Element);
            String stablename = AnyConverter.toString(xTablePropertySet.getPropertyValue("Name"));
            String sschemaname = AnyConverter.toString(xPropTableDataDescriptor.getPropertyValue("SchemaName"));
            String scatalogname = AnyConverter.toString(xPropTableDataDescriptor.getPropertyValue("CatalogName"));
            ComposedTableName = new CommandName(this, scatalogname, sschemaname, stablename, false);
            appendTableNameToFilter(ComposedTableName.getComposedName());
        } catch (Exception e) {
            e.printStackTrace(System.out);
        }}


        /* (non-Javadoc)
         * @see com.sun.star.container.XContainerListener#elementRemoved(com.sun.star.container.ContainerEvent)
         */
        public void elementRemoved(ContainerEvent arg0) {

        }

        /* (non-Javadoc)
         * @see com.sun.star.container.XContainerListener#elementReplaced(com.sun.star.container.ContainerEvent)
         */
        public void elementReplaced(ContainerEvent arg0) {

        }

        /* (non-Javadoc)
         * @see com.sun.star.lang.XEventListener#disposing(com.sun.star.lang.EventObject)
         */
        public void disposing(EventObject arg0) {

        }

           /**
            * @param _stablename
            * @return
            */
           public boolean appendTableNameToFilter(String _scomposedtablename){
               boolean bhastoinsert = true;
               for (int i = 0; i < sTableFilters.length; i++){
                   if (sTableFilters[i].compareTo("%") > -1){
                       if (sTableFilters[i].endsWith("." + _scomposedtablename))
                           bhastoinsert = false;
                       else if (sTableFilters[i].length() == 1)
                           bhastoinsert = false;
                   }
                   else if (sTableFilters[i].equals(_scomposedtablename))
                       bhastoinsert = false;
                   if (!bhastoinsert)
                       break;
               }
               if (bhastoinsert){
                   String[] sNewTableFilters = new String[sTableFilters.length + 1];
                System.arraycopy(sTableFilters, 0, sNewTableFilters, 0, sTableFilters.length);
                   sNewTableFilters[sTableFilters.length] = _scomposedtablename;
                   sTableFilters = sNewTableFilters;
                   try {
                    xDataSourcePropertySet.setPropertyValue("TableFilter", sTableFilters);
                } catch (Exception e) {
                    e.printStackTrace(System.out);
                    bhastoinsert = false;
                }
               }
               return bhastoinsert;
           }
}

