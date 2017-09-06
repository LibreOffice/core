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

package com.sun.star.sdbcx.comp.postgresql;

import java.util.List;
import java.util.Map;

import com.sun.star.beans.XPropertySet;
import com.sun.star.container.ElementExistException;
import com.sun.star.container.NoSuchElementException;
import com.sun.star.lang.IndexOutOfBoundsException;
import com.sun.star.sdbc.SQLException;
import com.sun.star.sdbc.XConnection;
import com.sun.star.sdbcx.comp.postgresql.comphelper.CompHelper;
import com.sun.star.sdbcx.comp.postgresql.sdbcx.OColumnContainer;
import com.sun.star.sdbcx.comp.postgresql.sdbcx.OContainer;
import com.sun.star.sdbcx.comp.postgresql.sdbcx.OIndexContainer;
import com.sun.star.sdbcx.comp.postgresql.sdbcx.OKey;
import com.sun.star.sdbcx.comp.postgresql.sdbcx.OKeyContainer;
import com.sun.star.sdbcx.comp.postgresql.sdbcx.OTable;
import com.sun.star.sdbcx.comp.postgresql.sdbcx.SqlTableHelper;
import com.sun.star.sdbcx.comp.postgresql.sdbcx.SqlTableHelper.ColumnDescription;
import com.sun.star.sdbcx.comp.postgresql.sdbcx.descriptors.SdbcxTableDescriptor;

public class PostgresqlTable extends OTable {
    private PostgresqlTable(Object lock, XConnection connection, OContainer tables, String name,
            String catalogName, String schemaName, String description, String type) {
        super(lock, name, true, connection, tables);
        super.catalogName = catalogName;
        super.schemaName = schemaName;
        super.description = description;
        super.type = type;
    }

    public static PostgresqlTable create(XConnection connection, OContainer tables, String name,
            String catalogName, String schemaName, String description, String type) {
        Object lock = new Object();
        return new PostgresqlTable(lock, connection, tables, name, catalogName, schemaName, description, type);
    }

    @Override
    public XPropertySet createDataDescriptor() {
        SdbcxTableDescriptor descriptor = SdbcxTableDescriptor.create(true);
        synchronized (lock) {
            CompHelper.copyProperties(this, descriptor);
        }
        return descriptor;
    }

    @Override
    public void setName(String name) {
        // TODO Auto-generated method stub

    }

    @Override
    public void rename(String name) throws SQLException, ElementExistException {
        // TODO Auto-generated method stub

    }

    @Override
    public void alterColumnByIndex(int index, XPropertySet descriptor) throws SQLException, IndexOutOfBoundsException {
        // TODO Auto-generated method stub

    }

    @Override
    public void alterColumnByName(String name, XPropertySet descriptor) throws SQLException, NoSuchElementException {
        // TODO Auto-generated method stub

    }

    @Override
    protected OContainer refreshColumns() {
        try {
            List<ColumnDescription> columns = new SqlTableHelper().readColumns(getConnection().getMetaData(), catalogName, schemaName, getName());
            return new OColumnContainer(lock, isCaseSensitive(), columns, this, getConnection().getMetaData());
        } catch (ElementExistException elementExistException) {
            return null;
        } catch (SQLException sqlException) {
            return null;
        }
    }

    @Override
    protected OContainer refreshIndexes() {
        try {
            List<String> indexes = new SqlTableHelper().readIndexes(getConnection().getMetaData(), catalogName, schemaName, getName(), this);
            return new OIndexContainer(lock, indexes, isCaseSensitive(), this);
        } catch (ElementExistException elementExistException) {
            return null;
        } catch (SQLException sqlException) {
            return null;
        }
    }

    @Override
    protected OContainer refreshKeys() {
        try {
            Map<String, OKey> keys = new SqlTableHelper().readKeys(
                    getConnection().getMetaData(), catalogName, schemaName, getName(), isCaseSensitive(), this);
            return OKeyContainer.create(isCaseSensitive(), keys, this);
        } catch (ElementExistException elementExistException) {
            return null;
        } catch (SQLException sqlException) {
            return null;
        }
    }


}
