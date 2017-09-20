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

package com.sun.star.sdbcx.comp.postgresql.util;

import java.util.Map;
import java.util.TreeMap;

import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.PropertyVetoException;
import com.sun.star.beans.UnknownPropertyException;
import com.sun.star.beans.XPropertySet;
import com.sun.star.beans.XPropertySetInfo;
import com.sun.star.container.ElementExistException;
import com.sun.star.container.XChild;
import com.sun.star.container.XIndexAccess;
import com.sun.star.container.XNameAccess;
import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.lang.IndexOutOfBoundsException;
import com.sun.star.lang.WrappedTargetException;
import com.sun.star.sdb.XOfficeDatabaseDocument;
import com.sun.star.sdbc.ColumnValue;
import com.sun.star.sdbc.DataType;
import com.sun.star.sdbc.KeyRule;
import com.sun.star.sdbc.SQLException;
import com.sun.star.sdbc.XConnection;
import com.sun.star.sdbc.XDataSource;
import com.sun.star.sdbc.XDatabaseMetaData;
import com.sun.star.sdbc.XResultSet;
import com.sun.star.sdbc.XResultSetMetaData;
import com.sun.star.sdbc.XResultSetMetaDataSupplier;
import com.sun.star.sdbc.XRow;
import com.sun.star.sdbc.XStatement;
import com.sun.star.sdbcx.KeyType;
import com.sun.star.sdbcx.XAppend;
import com.sun.star.sdbcx.XColumnsSupplier;
import com.sun.star.sdbcx.XKeysSupplier;
import com.sun.star.sdbcx.comp.postgresql.comphelper.CompHelper;
import com.sun.star.sdbcx.comp.postgresql.sdbcx.OColumnContainer.ExtraColumnInfo;
import com.sun.star.uno.Any;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.UnoRuntime;

public class DbTools {
    private static class NameComponentSupport {
        boolean useCatalogs;
        boolean useSchemas;

        NameComponentSupport() {
            useCatalogs = true;
            useSchemas = true;
        }

        NameComponentSupport(boolean useCatalogs, boolean useSchemas) {
            this.useCatalogs = useCatalogs;
            this.useSchemas = useSchemas;
        }
    }

    public static class NameComponents {
        private String catalog = "";
        private String schema = "";
        private String table = "";

        public NameComponents(String catalog, String schema, String table) {
            this.catalog = catalog;
            this.schema = schema;
            this.table = table;
        }

        public NameComponents() {
        }

        public String getCatalog() {
            return catalog;
        }

        public void setCatalog(String catalog) {
            this.catalog = catalog;
        }

        public String getSchema() {
            return schema;
        }

        public void setSchema(String schema) {
            this.schema = schema;
        }

        public String getTable() {
            return table;
        }

        public void setTable(String table) {
            this.table = table;
        }
    }

    private static NameComponentSupport getNameComponentSupport(XDatabaseMetaData metadata, ComposeRule composeRule) throws SQLException {
        switch (composeRule) {
        case InTableDefinitions:
            return new NameComponentSupport(
                    metadata.supportsCatalogsInTableDefinitions(), metadata.supportsSchemasInTableDefinitions());
        case InIndexDefinitions:
            return new NameComponentSupport(
                    metadata.supportsCatalogsInIndexDefinitions(), metadata.supportsSchemasInIndexDefinitions());
        case InDataManipulation:
            return new NameComponentSupport(
                    metadata.supportsCatalogsInDataManipulation(), metadata.supportsSchemasInDataManipulation());
        case InProcedureCalls:
            return new NameComponentSupport(
                    metadata.supportsCatalogsInProcedureCalls(), metadata.supportsSchemasInProcedureCalls());
        case InPrivilegeDefinitions:
            return new NameComponentSupport(
                    metadata.supportsCatalogsInPrivilegeDefinitions(), metadata.supportsSchemasInPrivilegeDefinitions());
        case Complete:
            return new NameComponentSupport(
                    true, true);
        default:
            throw new UnsupportedOperationException("Invalid/unknown enum value");
        }
    }

    /** compose a complete table name from it's up to three parts, regarding to the database meta data composing rules
     */
    public static String composeTableName(
            XDatabaseMetaData metadata, String catalog, String schema, String table, boolean quote, ComposeRule composeRule) throws SQLException {
        if (metadata == null) {
            return "";
        }
        String quoteString = metadata.getIdentifierQuoteString();
        NameComponentSupport nameComponentSupport = getNameComponentSupport(metadata, composeRule);

        StringBuilder composedName = new StringBuilder();

        String catalogSeparator = "";
        boolean catalogAtStart = true;
        if (!catalog.isEmpty() && nameComponentSupport.useCatalogs) {
            catalogSeparator = metadata.getCatalogSeparator();
            catalogAtStart = metadata.isCatalogAtStart();
            if (catalogAtStart && !catalogSeparator.isEmpty()) {
                composedName.append(quote ? quoteName(quoteString, catalog) : catalog);
                composedName.append(catalogSeparator);
            }
        }
        if (!schema.isEmpty() && nameComponentSupport.useSchemas) {
            composedName.append(quote ? quoteName(quoteString, schema) : schema);
            composedName.append('.');
        }
        composedName.append(quote ? quoteName(quoteString, table) : table);
        if (!catalog.isEmpty() && !catalogAtStart && !catalogSeparator.isEmpty() && nameComponentSupport.useCatalogs) {
            composedName.append(catalogSeparator);
            composedName.append(quote ? quoteName(quoteString, catalog) : catalog);
        }
        return composedName.toString();
    }

    public static String composeTableName(
            XDatabaseMetaData metadata, XPropertySet table, ComposeRule composeRule,
            boolean suppressCatalog, boolean suppressSchema, boolean shouldQuote) throws SQLException {
        NameComponents nameComponents = getTableNameComponents(table);
        return doComposeTableName(metadata,
                suppressCatalog ? "" : nameComponents.getCatalog(),
                suppressSchema ? "" : nameComponents.getSchema(),
                nameComponents.getTable(),
                shouldQuote, composeRule);
    }

    /** check if a specific property is enabled in the info sequence
     *  @deprecated
     *  Use getBooleanDataSourceSetting instead, which cares for the default of the property itself,
     *   instead of spreading this knowledge through all callers.
     */
    public static boolean isDataSourcePropertyEnabled(Object object, String property, boolean defaultValue) throws SQLException {
        try {
            boolean enabled = defaultValue;
            XPropertySet properties = UnoRuntime.queryInterface(XPropertySet.class, findDataSource(object));
            if (properties != null) {
                PropertyValue[] info = (PropertyValue[]) AnyConverter.toArray(properties.getPropertyValue("Info"));
                for (PropertyValue propertyValue : info) {
                    if (propertyValue.Name.equals(property)) {
                        enabled = AnyConverter.toBoolean(propertyValue.Value);
                        break;
                    }
                }
            }
            return enabled;
        } catch (IllegalArgumentException | WrappedTargetException | UnknownPropertyException exception) {
            throw new SQLException("Error", object, StandardSQLState.SQL_GENERAL_ERROR.text(), 0, exception);
        }
    }

    /** search the parent hierarchy for a data source.
     */
    public static XDataSource findDataSource(Object parent) {
        XOfficeDatabaseDocument databaseDocument = UnoRuntime.queryInterface(XOfficeDatabaseDocument.class, parent);
        XDataSource dataSource = null;
        if (databaseDocument != null) {
            dataSource = databaseDocument.getDataSource();
        }
        if (dataSource == null) {
            dataSource = UnoRuntime.queryInterface(XDataSource.class, parent);
        }
        if (dataSource == null) {
            XChild child = UnoRuntime.queryInterface(XChild.class, parent);
            if (child != null) {
                dataSource = findDataSource(child.getParent());
            }
        }
        return dataSource;
    }

    public static String doComposeTableName(XDatabaseMetaData metadata, String catalog, String schema, String table,
            boolean shouldQuote, ComposeRule composeRule) throws SQLException {
        Osl.ensure(!table.isEmpty(), "At least the table name should be non-empty");
        String quoteString = metadata.getIdentifierQuoteString();
        NameComponentSupport nameComponentSupport = getNameComponentSupport(metadata, composeRule);

        StringBuilder composedName = new StringBuilder();
        String catalogSeparator = "";
        boolean catalogAtStart = true;
        if (!catalog.isEmpty() && nameComponentSupport.useCatalogs) {
            catalogSeparator = metadata.getCatalogSeparator();
            catalogAtStart = metadata.isCatalogAtStart();

            if (catalogAtStart && !catalogSeparator.isEmpty()) {
                composedName.append(shouldQuote ? quoteName(quoteString, catalog) : catalog);
                composedName.append(catalogSeparator);
            }
        }

        if (!schema.isEmpty() && nameComponentSupport.useSchemas) {
            composedName.append(shouldQuote ? quoteName(quoteString, schema) : schema);
            composedName.append(".");
        }

        composedName.append(shouldQuote ? quoteName(quoteString, table) : table);

        if (!catalog.isEmpty() && !catalogAtStart && !catalogSeparator.isEmpty() && nameComponentSupport.useCatalogs) {
            composedName.append(catalogSeparator);
            composedName.append(shouldQuote ? quoteName(quoteString, catalog) : catalog);
        }
        return composedName.toString();
    }

    /** composes a table name for usage in a SELECT statement
     *
     * This includes quoting of the table as indicated by the connection's meta data, plus respecting
     * the settings "UseCatalogInSelect" and "UseSchemaInSelect", which might be present
     * in the data source which the connection belongs to.
     */
    public static String composeTableNameForSelect(XConnection connection, String catalog,
            String schema, String table) throws SQLException {
        boolean useCatalogInSelect = isDataSourcePropertyEnabled(connection, "UseCatalogInSelect", true);
        boolean useSchemaInSelect = isDataSourcePropertyEnabled(connection, "UseSchemaInSelect", true);
        return doComposeTableName(connection.getMetaData(), useCatalogInSelect ? catalog : "",
                useSchemaInSelect ? schema : "", table, true, ComposeRule.InDataManipulation);
    }

    /** composes a table name for usage in a SELECT statement
     *
     * This includes quoting of the table as indicated by the connection's meta data, plus respecting
     * the settings "UseCatalogInSelect" and "UseSchemaInSelect", which might be present
     * in the data source which the connection belongs to.
     */
    public static String composeTableNameForSelect(XConnection connection, XPropertySet table) throws SQLException {
        NameComponents nameComponents = getTableNameComponents(table);
        return composeTableNameForSelect(connection, nameComponents.getCatalog(), nameComponents.getSchema(), nameComponents.getTable());
    }

    private static NameComponents getTableNameComponents(XPropertySet table) throws SQLException {
        try {
            NameComponents nameComponents = new NameComponents();
            XPropertySetInfo propertySetInfo = table.getPropertySetInfo();
            if (propertySetInfo != null && propertySetInfo.hasPropertyByName(PropertyIds.NAME.name)) {
                if (propertySetInfo.hasPropertyByName(PropertyIds.CATALOGNAME.name)
                        && propertySetInfo.hasPropertyByName(PropertyIds.SCHEMANAME.name)) {
                    nameComponents.setCatalog(AnyConverter.toString(table.getPropertyValue(PropertyIds.CATALOGNAME.name)));
                    nameComponents.setSchema(AnyConverter.toString(table.getPropertyValue(PropertyIds.SCHEMANAME.name)));
                }
                nameComponents.setTable(AnyConverter.toString(table.getPropertyValue(PropertyIds.NAME.name)));
            } else {
                Osl.ensure(false, "this is not a table object");
            }
            return nameComponents;
        } catch (IllegalArgumentException | WrappedTargetException | UnknownPropertyException exception) {
            throw new SQLException("Error", Any.VOID, StandardSQLState.SQL_GENERAL_ERROR.text(), 0, exception);
        }
    }

    /** quote the given name with the given quote string.
     */
    public static String quoteName(String quote, String name) {
        if (!quote.isEmpty() && quote.codePointAt(0) != ' ') {
            return quote + name + quote;
        }
        return name;
    }

    /** quote the given table name (which may contain a catalog and a schema) according to the rules provided by the meta data
     */
    public static String quoteTableName(XDatabaseMetaData metadata, String name, ComposeRule composeRule) throws SQLException {
        NameComponents nameComponents = qualifiedNameComponents(metadata, name, composeRule);
        return doComposeTableName(metadata, nameComponents.getCatalog(), nameComponents.getSchema(), nameComponents.getTable(), true, composeRule);
    }

    /** split a fully qualified table name (including catalog and schema, if applicable) into its component parts.
     * @param  _rxConnMetaData     meta data describing the connection where you got the table name from
     * @param  _rQualifiedName     fully qualified table name
     * @param  _eComposeRule       where do you need the name for
     * @return the NameComponents object with the catalog, schema and table
     */
    public static NameComponents qualifiedNameComponents(XDatabaseMetaData _rxConnMetaData, String _rQualifiedName,
            ComposeRule _eComposeRule) throws SQLException {
        Osl.ensure(_rxConnMetaData, "QualifiedNameComponents : invalid meta data!");

        NameComponentSupport aNameComps = getNameComponentSupport( _rxConnMetaData, _eComposeRule );

        String sSeparator = _rxConnMetaData.getCatalogSeparator();
        NameComponents ret = new NameComponents();

        String sName = _rQualifiedName;
        // do we have catalogs ?
        if ( aNameComps.useCatalogs ) {
            if (_rxConnMetaData.isCatalogAtStart()) {
                // search for the catalog name at the beginning
                int nIndex = sName.indexOf(sSeparator);
                if (-1 != nIndex) {
                    ret.setCatalog(sName.substring(0, nIndex));
                    sName = sName.substring(nIndex + 1);
                }
            } else {
                // Katalogname am Ende
                int nIndex = sName.lastIndexOf(sSeparator);
                if (-1 != nIndex) {
                    ret.setCatalog(sName.substring(nIndex + 1));
                    sName = sName.substring(0, nIndex);
                }
            }
        }

        if ( aNameComps.useSchemas ) {
            int nIndex = sName.indexOf('.');
            //Osl.ensure(-1 != nIndex, "QualifiedNameComponents : no schema separator!");
            if ( nIndex != -1 ) {
                ret.setSchema(sName.substring(0, nIndex));
            }
            sName = sName.substring(nIndex + 1);
        }

        ret.setTable(sName);
        return ret;
    }

    /** creates a SQL CREATE TABLE statement
     *
     * @param  descriptor
     *    The descriptor of the new table.
     * @param  connection
     *    The connection.
     * @param  helper
     *    Allow to add special SQL constructs.
     * @param  createPattern
     *
     * @return
     *   The CREATE TABLE statement.
     */
    public static String createSqlCreateTableStatement(XPropertySet descriptor, XConnection connection,
            ISQLStatementHelper helper, String createPattern) throws SQLException {

        String sql = createStandardCreateStatement(descriptor, connection, helper, createPattern);
        final String keyStatement = createStandardKeyStatement(descriptor, connection);
        if (!keyStatement.isEmpty()) {
            sql += keyStatement;
        } else {
            sql += ')';
        }
        return sql;
    }

    /** creates the standard sql create table statement without the key part.
     * @param  descriptor
     *    The descriptor of the new table.
     * @param  connection
     *    The connection.
     * @param  helper
     *    Allow to add special SQL constructs.
     * @param  createPattern
     *
     */
    public static String createStandardCreateStatement(XPropertySet descriptor, XConnection connection,
            ISQLStatementHelper helper, String createPattern) throws SQLException {
        try {
            XDatabaseMetaData metadata = connection.getMetaData();
            String catalog = AnyConverter.toString(descriptor.getPropertyValue("CatalogName"));
            String schema = AnyConverter.toString(descriptor.getPropertyValue("SchemaName"));
            String table = AnyConverter.toString(descriptor.getPropertyValue("Name"));
            String composedName = composeTableName(metadata, catalog, schema, table, true, ComposeRule.InTableDefinitions);
            if (composedName.isEmpty()) {
                throw new SQLException(SharedResources.getInstance().getResourceString(Resources.STR_ERRORMSG_SEQUENCE), connection,
                        StandardSQLState.SQL_FUNCTION_SEQUENCE_ERROR.text(), 0, null);
            }

            XIndexAccess columns = null;
            XColumnsSupplier columnsSupplier = UnoRuntime.queryInterface(XColumnsSupplier.class, descriptor);
            if (columnsSupplier != null) {
                columns = UnoRuntime.queryInterface(XIndexAccess.class, columnsSupplier.getColumns());
            }
            if (columns == null || columns.getCount() <= 0) {
                throw new SQLException(SharedResources.getInstance().getResourceString(Resources.STR_ERRORMSG_SEQUENCE), connection,
                        StandardSQLState.SQL_FUNCTION_SEQUENCE_ERROR.text(), 0, null);
            }

            int columnCount = columns.getCount();
            StringBuilder columnText = new StringBuilder();
            String separator = "";
            for (int i = 0; i < columnCount; i++) {
                XPropertySet columnProperties;
                columnProperties = AnyConverter.toObject(XPropertySet.class, columns.getByIndex(i));
                if (columnProperties != null) {
                    columnText.append(separator);
                    separator = ",";
                    columnText.append(createStandardColumnPart(columnProperties, connection, helper, createPattern));
                }
            }

            return String.format("CREATE TABLE %s (%s", composedName, columnText.toString());
        } catch (IllegalArgumentException | WrappedTargetException | UnknownPropertyException | IndexOutOfBoundsException exception) {
            throw new SQLException("Error", Any.VOID, StandardSQLState.SQL_GENERAL_ERROR.text(), 0, exception);
        }
    }

    /** creates the standard sql statement for the column part of a create table statement.
     *  @param  columnProperties
     *      The descriptor of the column.
     *  @param  connection
     *      The connection.
     *  @param  helper
     *       Allow to add special SQL constructs.
     *  @param  createPattern
     *
     */
    public static String createStandardColumnPart(XPropertySet columnProperties, XConnection connection,
            ISQLStatementHelper helper, String createPattern) throws SQLException {
        try {
            XDatabaseMetaData metadata = connection.getMetaData();

            final String quoteString = metadata.getIdentifierQuoteString();
            final StringBuilder sql = new StringBuilder();
            sql.append(quoteName(quoteString, AnyConverter.toString(columnProperties.getPropertyValue("Name"))));
            sql.append(' ');

            String typename = AnyConverter.toString(columnProperties.getPropertyValue("TypeName"));
            int datatype = AnyConverter.toInt(columnProperties.getPropertyValue("Type"));
            int precision = AnyConverter.toInt(columnProperties.getPropertyValue("Precision"));
            int scale = AnyConverter.toInt(columnProperties.getPropertyValue("Scale"));
            boolean isAutoIncrement = AnyConverter.toBoolean(columnProperties.getPropertyValue("IsAutoIncrement"));

            // check if the user enter a specific string to create autoincrement values
            String autoIncrementValue = "";
            XPropertySetInfo columnPropertiesInfo = columnProperties.getPropertySetInfo();
            if (columnPropertiesInfo != null && columnPropertiesInfo.hasPropertyByName("AutoIncrementCreation")) {
                autoIncrementValue = AnyConverter.toString(columnProperties.getPropertyValue("AutoIncrementCreation"));
            }

            // look if we have to use precisions
            boolean useLiteral = false;
            String prefix = "";
            String postfix ="";
            String createParams = "";
            XResultSet results = null;
            try {
                results = metadata.getTypeInfo();
                if (results != null) {
                    XRow row = UnoRuntime.queryInterface(XRow.class, results);
                    while (results.next()) {
                        String typeName2Cmp = row.getString(1);
                        int nType = row.getShort(2);
                        prefix = row.getString(4);
                        postfix = row.getString(5);
                        createParams = row.getString(6);
                        // first identical type will be used if typename is empty
                        if (typename.isEmpty() && nType == datatype) {
                            typename = typeName2Cmp;
                        }
                        if (typename.equalsIgnoreCase(typeName2Cmp) && nType == datatype && !row.wasNull() && !createParams.isEmpty()) {
                            useLiteral = true;
                            break;
                        }
                    }
                }
            } finally {
                CompHelper.disposeComponent(results);
            }

            int index = 0;
            if (!autoIncrementValue.isEmpty() && (index = typename.indexOf(autoIncrementValue)) != -1) {
                typename = typename.substring(0, index);
            }

            if ((precision > 0 || scale > 0) && useLiteral) {
                int parenPos = typename.indexOf('(');
                if (parenPos == -1) {
                    sql.append(typename);
                    sql.append('(');
                } else {
                    sql.append(typename.substring(0, ++parenPos));
                }

                if (precision > 0 && datatype != DataType.TIMESTAMP) {
                    sql.append(precision);
                    if (scale > 0 || (!createPattern.isEmpty() && createParams.indexOf(createPattern) != -1)) {
                        sql.append(',');
                    }
                }
                if (scale > 0 || (!createPattern.isEmpty() && createParams.indexOf(createPattern) != -1) || datatype == DataType.TIMESTAMP) {
                    sql.append(scale);
                }
                if (parenPos == -1) {
                    sql.append(')');
                } else {
                    parenPos = typename.indexOf(')', parenPos);
                    sql.append(typename.substring(parenPos));
                }
            } else {
                sql.append(typename); // simply add the type name
            }

            String defaultValue = AnyConverter.toString(columnProperties.getPropertyValue("DefaultValue"));
            if (!defaultValue.isEmpty()) {
                sql.append(" DEFAULT ");
                sql.append(prefix);
                sql.append(defaultValue);
                sql.append(postfix);
            }

            if (AnyConverter.toInt(columnProperties.getPropertyValue("IsNullable")) == ColumnValue.NO_NULLS) {
                sql.append(" NOT NULL");
            }

            if (isAutoIncrement && !autoIncrementValue.isEmpty()) {
                sql.append(' ');
                sql.append(autoIncrementValue);
            }

            if (helper != null) {
                helper.addComment(columnProperties, sql);
            }

            return sql.toString();
        } catch (IllegalArgumentException | WrappedTargetException | UnknownPropertyException exception) {
            throw new SQLException("Error", Any.VOID, StandardSQLState.SQL_GENERAL_ERROR.text(), 0, exception);
        }
    }

    /** creates the standard sql statement for the key part of a create table statement.
     * @param  descriptor
     *      The descriptor of the new table.
     * @param  connection
     *      The connection.
     */
    public static String createStandardKeyStatement(XPropertySet descriptor, XConnection connection) throws SQLException {
        try {
            XDatabaseMetaData metadata = connection.getMetaData();
            StringBuilder sql = new StringBuilder();

            XKeysSupplier keysSupplier = UnoRuntime.queryInterface(XKeysSupplier.class, descriptor);
            XIndexAccess keys = keysSupplier.getKeys();
            if (keys != null) {
                boolean hasPrimaryKey = false;
                for (int i = 0; i < keys.getCount(); i++) {
                    XPropertySet columnProperties = AnyConverter.toObject(XPropertySet.class, keys.getByIndex(i));
                    if (columnProperties != null) {
                        int keyType = AnyConverter.toInt(columnProperties.getPropertyValue("Type"));
                        XColumnsSupplier columnsSupplier = UnoRuntime.queryInterface(XColumnsSupplier.class, columnProperties);
                        XIndexAccess columns = UnoRuntime.queryInterface(XIndexAccess.class, columnsSupplier.getColumns());
                        if (columns == null || columns.getCount() == 0) {
                            throw new SQLException(SharedResources.getInstance().getResourceString(Resources.STR_ERRORMSG_SEQUENCE), connection,
                                    StandardSQLState.SQL_FUNCTION_SEQUENCE_ERROR.text(), 0, null);
                        }
                        if (keyType == KeyType.PRIMARY) {
                            if (hasPrimaryKey) {
                                throw new SQLException(SharedResources.getInstance().getResourceString(Resources.STR_ERRORMSG_SEQUENCE), connection,
                                        StandardSQLState.SQL_FUNCTION_SEQUENCE_ERROR.text(), 0, null);
                            }
                            hasPrimaryKey = true;
                            sql.append(" PRIMARY KEY ");
                            sql.append(generateColumnNames(columns, metadata));
                        } else if (keyType == KeyType.UNIQUE) {
                            sql.append(" UNIQUE ");
                            sql.append(generateColumnNames(columns, metadata));
                        } else if (keyType == KeyType.FOREIGN) {
                            int deleteRule = AnyConverter.toInt(columnProperties.getPropertyValue("DeleteRule"));
                            sql.append(" FOREIGN KEY ");

                            String referencedTable = AnyConverter.toString(columnProperties.getPropertyValue("ReferencedTable"));
                            NameComponents nameComponents = qualifiedNameComponents(metadata, referencedTable, ComposeRule.InDataManipulation);
                            String composedName = composeTableName(metadata, nameComponents.getCatalog(), nameComponents.getSchema(), nameComponents.getTable(),
                                    true, ComposeRule.InTableDefinitions);
                            if (composedName.isEmpty()) {
                                throw new SQLException(SharedResources.getInstance().getResourceString(Resources.STR_ERRORMSG_SEQUENCE), connection,
                                        StandardSQLState.SQL_FUNCTION_SEQUENCE_ERROR.text(), 0, null);
                            }

                            sql.append(generateColumnNames(columns, metadata));

                            switch (deleteRule) {
                            case KeyRule.CASCADE:
                                sql.append(" ON DELETE CASCADE ");
                                break;
                            case KeyRule.RESTRICT:
                                sql.append(" ON DELETE RESTRICT ");
                                break;
                            case KeyRule.SET_NULL:
                                sql.append(" ON DELETE SET NULL ");
                                break;
                            case KeyRule.SET_DEFAULT:
                                sql.append(" ON DELETE SET DEFAULT ");
                                break;
                            }
                        }
                    }
                }
            }

            if (sql.length() > 0) {
                sql.append(')');
            }
            return sql.toString();
        } catch (IllegalArgumentException | WrappedTargetException | UnknownPropertyException | IndexOutOfBoundsException exception) {
            throw new SQLException("Error", Any.VOID, StandardSQLState.SQL_GENERAL_ERROR.text(), 0, exception);
        }
    }

    private static String generateColumnNames(XIndexAccess columns, XDatabaseMetaData metadata) throws
            SQLException, WrappedTargetException, UnknownPropertyException, IllegalArgumentException, IndexOutOfBoundsException {
        String quote = metadata.getIdentifierQuoteString();
        StringBuilder sql = new StringBuilder(" (");
        int columnCount = columns.getCount();
        String separator = "";
        for (int i = 0; i < columnCount; i++) {
            XPropertySet columnProperties = AnyConverter.toObject(XPropertySet.class, columns.getByIndex(i));
            if (columnProperties != null) {
                sql.append(separator);
                separator = ",";
                String columnName = AnyConverter.toString(columnProperties.getPropertyValue("Name"));
                sql.append(quoteName(quote, columnName));
            }
        }
        if (columnCount > 0) {
            sql.append(')');
        }
        return sql.toString();
    }

    /** collects the information about auto increment, currency and data type for the given column name.
     * The column must be quoted, * is also valid.
     * @param  connection
     *     The connection.
     * @param  composedName
     *    The quoted table name. ccc.sss.ttt
     * @param  columnName
     *    The name of the column, or *
     * @return
     *    The information about the column(s).
     */
    public static Map<String,ExtraColumnInfo> collectColumnInformation(XConnection connection, String composedName, String columnName) throws SQLException {
        String sql = String.format("SELECT %s FROM %s WHERE 0 = 1", columnName, composedName);
        XStatement statement = null;
        try {
            statement = connection.createStatement();
            XPropertySet statementProperties = UnoRuntime.queryInterface(XPropertySet.class, statement);
            statementProperties.setPropertyValue(PropertyIds.ESCAPEPROCESSING.name, false);
            XResultSet results = statement.executeQuery(sql);
            XResultSetMetaDataSupplier metadataSupplier = UnoRuntime.queryInterface(XResultSetMetaDataSupplier.class, results);
            XResultSetMetaData metadata = metadataSupplier.getMetaData();

            Map<String,ExtraColumnInfo> columns = new TreeMap<>();
            int count = metadata.getColumnCount();
            Osl.ensure(count > 0, "resultset has empty metadata");
            for (int i = 1; i <= count; i++) {
                String newColumnName = metadata.getColumnName(i);
                ExtraColumnInfo columnInfo = new ExtraColumnInfo();
                columnInfo.isAutoIncrement = metadata.isAutoIncrement(i);
                columnInfo.isCurrency = metadata.isCurrency(i);
                columnInfo.dataType = metadata.getColumnType(i);
                columns.put(newColumnName, columnInfo);
            }
            return columns;
        } catch (IllegalArgumentException | WrappedTargetException | UnknownPropertyException | PropertyVetoException exception) {
            throw new SQLException("Error", Any.VOID, StandardSQLState.SQL_GENERAL_ERROR.text(), 0, exception);
        } finally {
            CompHelper.disposeComponent(statement);
        }
    }

    /** returns the primary key columns of the table
     */
    public static XNameAccess getPrimaryKeyColumns(XPropertySet table) throws SQLException {
        try {
            XNameAccess keyColumns = null;
            XKeysSupplier keysSupplier = UnoRuntime.queryInterface(XKeysSupplier.class, table);
            if (keysSupplier != null) {
                XIndexAccess keys = keysSupplier.getKeys();
                if (keys != null) {
                    int count = keys.getCount();
                    for (int i = 0; i < count; i++) {
                        XPropertySet propertySet = UnoRuntime.queryInterface(XPropertySet.class, keys.getByIndex(i));
                        if (propertySet != null) {
                            int keyType = 0;
                            keyType = AnyConverter.toInt(propertySet.getPropertyValue(PropertyIds.TYPE.name));
                            if (keyType == KeyType.PRIMARY) {
                                XColumnsSupplier columnsSupplier = UnoRuntime.queryInterface(XColumnsSupplier.class, propertySet);
                                keyColumns = columnsSupplier.getColumns();
                                break;
                            }
                        }
                    }
                }
            }
            return keyColumns;
        } catch (IndexOutOfBoundsException | IllegalArgumentException | WrappedTargetException | UnknownPropertyException exception) {
            throw new SQLException("Error", Any.VOID, StandardSQLState.SQL_GENERAL_ERROR.text(), 0, exception);
        }
    }

    public static void cloneDescriptorColumns(XPropertySet source, XPropertySet destination) throws SQLException {
        XColumnsSupplier sourceColumnsSupplier = UnoRuntime.queryInterface(XColumnsSupplier.class, source);
        XIndexAccess sourceColumns = UnoRuntime.queryInterface(XIndexAccess.class, sourceColumnsSupplier.getColumns());

        XColumnsSupplier destinationColumnsSupplier = UnoRuntime.queryInterface(XColumnsSupplier.class, destination);
        XAppend destinationAppend = UnoRuntime.queryInterface(XAppend.class, destinationColumnsSupplier.getColumns());

        int count = sourceColumns.getCount();
        for (int i = 0; i < count; i++) {
            try {
                XPropertySet columnProperties = AnyConverter.toObject(XPropertySet.class, sourceColumns.getByIndex(i));
                destinationAppend.appendByDescriptor(columnProperties);
            } catch (WrappedTargetException | IndexOutOfBoundsException | IllegalArgumentException | ElementExistException exception) {
                throw new SQLException("Error", Any.VOID, StandardSQLState.SQL_GENERAL_ERROR.text(), 0, exception);
            }
        }
    }
}
