/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/

package com.sun.star.sdbcx.comp.postgresql.sdbcx;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.Map;

import com.sun.star.beans.PropertyVetoException;
import com.sun.star.beans.UnknownPropertyException;
import com.sun.star.beans.XPropertySet;
import com.sun.star.container.ElementExistException;
import com.sun.star.container.XIndexAccess;
import com.sun.star.container.XNamed;
import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.lang.IndexOutOfBoundsException;
import com.sun.star.lang.WrappedTargetException;
import com.sun.star.sdbc.KeyRule;
import com.sun.star.sdbc.SQLException;
import com.sun.star.sdbc.XConnection;
import com.sun.star.sdbc.XResultSet;
import com.sun.star.sdbc.XRow;
import com.sun.star.sdbc.XStatement;
import com.sun.star.sdbcx.KeyType;
import com.sun.star.sdbcx.XColumnsSupplier;
import com.sun.star.sdbcx.comp.postgresql.comphelper.CompHelper;
import com.sun.star.sdbcx.comp.postgresql.sdbcx.descriptors.SdbcxKeyDescriptor;
import com.sun.star.sdbcx.comp.postgresql.util.ComposeRule;
import com.sun.star.sdbcx.comp.postgresql.util.DbTools;
import com.sun.star.sdbcx.comp.postgresql.util.PropertyIds;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.UnoRuntime;

public class OKeyContainer extends OContainer {
    private OTable table;
    private Map<String,OKey> keys;

    protected OKeyContainer(Object lock, boolean isCaseSensitive, List<String> names, Map<String,OKey> keys, OTable table) throws ElementExistException {
        super(lock, isCaseSensitive, names);
        System.out.println("Keys.size()=" + keys.size());
        for (Map.Entry<String,OKey> entry : keys.entrySet()) {
            System.out.println(entry.getKey() + " => " + entry.getValue().referencedTable);
            XIndexAccess cols = UnoRuntime.queryInterface(XIndexAccess.class, entry.getValue().getColumns());
            try {
                System.out.println("" + cols.getCount() + " columns:");
                for (int i =0; i < cols.getCount(); i++) {
                    System.out.println("  " + AnyConverter.toObject(XNamed.class, cols.getByIndex(i)).getName());
                }
            } catch (WrappedTargetException wrappedTargetException) {
                wrappedTargetException.printStackTrace();
            } catch (IndexOutOfBoundsException indexOutOfBoundsException) {
                indexOutOfBoundsException.printStackTrace();
            } catch (IllegalArgumentException argumentException) {
                argumentException.printStackTrace();
            }
        }
        this.keys = keys;
        this.table = table;
    }

    public static OKeyContainer create(boolean isCaseSensitive, Map<String,OKey> keys, OTable table) throws ElementExistException {
        final Object lock = new Object();
        String[] names = new String[keys.size()];
        keys.keySet().toArray(names);
        return new OKeyContainer(lock, isCaseSensitive, Arrays.asList(names), keys, table);
    }

    @Override
    protected XPropertySet createDescriptor() {
        return SdbcxKeyDescriptor.create(isCaseSensitive());
    }

    @Override
    protected XPropertySet createObject(String name) throws SQLException {
        OKey ret = null;

        if (!name.isEmpty()) {
            ret = keys.get(name);
        }

        if (ret == null) { // we have a primary key with a system name
            // FIXME: so why was this exactly the same?
            ret = keys.get(name);
        }

        return ret;
    }

    @Override
    protected void impl_refresh() {
        //throw new NotImplementedException("");
    }

    @Override
    protected XPropertySet appendObject(String _rForName, XPropertySet descriptor) throws SQLException {
        XConnection connection = table.getConnection();
        if (connection == null) {
            return null;
        }
        try {
            int keyType = AnyConverter.toInt(descriptor.getPropertyValue(PropertyIds.TYPE.name));
            int updateRule = 0;
            int deleteRule = 0;
            String referencedName = "";
            if (keyType == KeyType.FOREIGN) {
                referencedName = AnyConverter.toString(descriptor.getPropertyValue(PropertyIds.UPDATERULE.name));
                updateRule = AnyConverter.toInt(descriptor.getPropertyValue(PropertyIds.UPDATERULE.name));
                deleteRule = AnyConverter.toInt(descriptor.getPropertyValue(PropertyIds.DELETERULE.name));
            }

            String quote = connection.getMetaData().getIdentifierQuoteString();
            String tableName = DbTools.composeTableName(connection.getMetaData(), table, ComposeRule.InTableDefinitions, false, false, true);

            String keyTypeString;
            if (keyType == KeyType.PRIMARY) {
                keyTypeString = "PRIMARY KEY";
            } else if (keyType == KeyType.FOREIGN) {
                keyTypeString = "FOREIGN KEY";
            } else {
                throw new SQLException();
            }

            StringBuilder columnsText = new StringBuilder();
            XColumnsSupplier columnsSupplier = UnoRuntime.queryInterface(XColumnsSupplier.class, descriptor);
            XIndexAccess columns = UnoRuntime.queryInterface(XIndexAccess.class, columnsSupplier.getColumns());
            String separator = "";
            for (int i = 0; i < columns.getCount(); i++) {
                columnsText.append(separator);
                separator = ",";
                XPropertySet columnProperties = AnyConverter.toObject(XPropertySet.class, columns.getByIndex(i));
                columnsText.append(DbTools.quoteName(quote, AnyConverter.toString(columnProperties.getPropertyValue(PropertyIds.NAME.name))));
            }

            String sql = String.format("ALTER TABLE %s ADD %s (%s)", tableName, keyTypeString, columnsText.toString());

            if (keyType == KeyType.FOREIGN) {
                String quotedTableName = DbTools.quoteTableName(connection.getMetaData(), referencedName, ComposeRule.InTableDefinitions);
                StringBuilder relatedColumns = new StringBuilder();
                separator = "";
                for (int i = 0; i < columns.getCount(); i++) {
                    relatedColumns.append(separator);
                    separator = ",";
                    XPropertySet columnProperties = AnyConverter.toObject(XPropertySet.class, columns.getByIndex(i));
                    relatedColumns.append(DbTools.quoteName(quote, AnyConverter.toString(columnProperties.getPropertyValue(PropertyIds.RELATEDCOLUMN.name))));
                }
                sql += String.format(" REFERENCES %s (%s)%s%s", quotedTableName, relatedColumns.toString(),
                        getKeyRuleString(true, updateRule), getKeyRuleString(false, deleteRule));
            }
            XStatement statement = null;
            try {
                statement = connection.createStatement();
                statement.execute(sql);
            } finally {
                CompHelper.disposeComponent(statement);
            }

            // find the name which the database gave the new key
            String newName = _rForName;
            try {
                String schema = AnyConverter.toString(this.table.getPropertyValue(PropertyIds.SCHEMANAME.name));
                String table = AnyConverter.toString(this.table.getPropertyValue(PropertyIds.TABLENAME.name));
                XResultSet results = null;
                try {
                    final int column;
                    if (keyType == KeyType.FOREIGN) {
                        results = connection.getMetaData().getImportedKeys(this.table.getPropertyValue(PropertyIds.CATALOGNAME.name), schema, table);
                        column = 12;
                    } else {
                        results = connection.getMetaData().getPrimaryKeys(this.table.getPropertyValue(PropertyIds.CATALOGNAME.name), schema, table);
                        column = 6;
                    }
                    if (results != null) {
                        XRow row = UnoRuntime.queryInterface(XRow.class, results);
                        while (results.next()) {
                            String name = row.getString(column);
                            if (!hasByName(name)) { // this name wasn't inserted yet so it must be the new one
                                descriptor.setPropertyValue(PropertyIds.NAME.name, name);
                                newName = name;
                                break;
                            }
                        }
                    }
                } finally {
                    CompHelper.disposeComponent(results);
                }
            } catch (SQLException sqlException) {
            }
            keys.put(newName, OKey.create(newName, isCaseSensitive(), referencedName, keyType, updateRule, deleteRule, new ArrayList<String>(), table));
            return createObject(newName);
        } catch (WrappedTargetException wrappedTargetException) {
        } catch (UnknownPropertyException unknownPropertyException) {
        } catch (IllegalArgumentException illegalArgumentException) {
        } catch (IndexOutOfBoundsException indexOutOfBoundsException) {
        } catch (PropertyVetoException propertyVetoException) {
        } catch (ElementExistException elementExistException) {
        }
        return null;
    }

    protected String getKeyRuleString(boolean isUpdate, int rule) {
        String keyRule = "";
        switch (rule) {
        case KeyRule.CASCADE:
            keyRule = isUpdate ? " ON UPDATE CASCADE " : " ON DELETE CASCADE ";
            break;
        case KeyRule.RESTRICT:
            keyRule = isUpdate ? " ON UPDATE RESTRICT " : " ON DELETE RESTRICT ";
            break;
        case KeyRule.SET_NULL:
            keyRule = isUpdate ? " ON UPDATE SET NULL " : " ON DELETE SET NULL ";
            break;
        case KeyRule.SET_DEFAULT:
            keyRule = isUpdate ? " ON UPDATE SET DEFAULT " : " ON DELETE SET DEFAULT ";
            break;
        }
        return keyRule;
    }

    @Override
    protected void dropObject(int index, String name) throws SQLException {
        XConnection connection = table.getConnection();
        if (connection == null) {
            return;
        }
        try {
            XPropertySet key = AnyConverter.toObject(XPropertySet.class, getObject(index));
            String tableName = DbTools.composeTableName(connection.getMetaData(), table,
                    ComposeRule.InTableDefinitions, false, false, true);
            final int keyType;
            if (key != null) {
                keyType = AnyConverter.toInt(key.getPropertyValue(PropertyIds.TYPE.name));
            } else {
                keyType = KeyType.PRIMARY;
            }
            final String sql;
            if (keyType == KeyType.PRIMARY) {
                sql = String.format("ALTER TABLE %s DROP PRIMARY KEY", tableName);
            } else {
                sql = String.format("ALTER TABLE %s %s %s", tableName, getDropForeignKey(),
                        DbTools.quoteName(connection.getMetaData().getIdentifierQuoteString(), name));
            }
            XStatement statement = null;
            try {
                statement = connection.createStatement();
                statement.execute(sql);
            } finally {
                CompHelper.disposeComponent(statement);
            }
        } catch (IllegalArgumentException illegalArgumentException) {

        } catch (WrappedTargetException wrappedTargetException) {
        } catch (UnknownPropertyException unknownPropertyException) {
        }
    }

    public String getDropForeignKey() {
        return "DROP CONSTRAINT";
    }
}
