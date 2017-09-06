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

package com.sun.star.sdbcx.comp.postgresql.sdbcx.descriptors;

import com.sun.star.beans.XPropertySet;
import com.sun.star.sdbc.SQLException;
import com.sun.star.sdbcx.comp.postgresql.sdbcx.OContainer;
import com.sun.star.sdbcx.comp.postgresql.util.Resources;
import com.sun.star.sdbcx.comp.postgresql.util.SharedResources;
import com.sun.star.sdbcx.comp.postgresql.util.StandardSQLState;

public abstract class SdbcxDescriptorContainer extends OContainer {
    public SdbcxDescriptorContainer(Object lock, boolean isCaseSensitive) {
        super(lock, isCaseSensitive);
    }

    @Override
    protected XPropertySet createObject(String name) throws SQLException {
        // This should never be called. DescriptorContainer always starts off empty,
        // and only grows as a result of appending.
        String error = SharedResources.getInstance().getResourceString(
                Resources.STR_ERRORMSG_SEQUENCE);
        throw new SQLException(error, this, StandardSQLState.SQL_FUNCTION_SEQUENCE_ERROR.text(), 0, null);
    }

    @Override
    protected void dropObject(int index, String name) throws SQLException {
    }

    @Override
    protected void impl_refresh() {
    }

    @Override
    protected XPropertySet appendObject(String _rForName, XPropertySet descriptor) throws SQLException {
        return cloneDescriptor(descriptor);
    }
}
