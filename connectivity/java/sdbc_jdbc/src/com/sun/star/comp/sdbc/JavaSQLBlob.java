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
package com.sun.star.comp.sdbc;

import java.io.InputStream;

import org.apache.openoffice.comp.sdbc.dbtools.comphelper.ResourceBasedEventLogger;
import org.apache.openoffice.comp.sdbc.dbtools.util.Resources;
import org.apache.openoffice.comp.sdbc.dbtools.util.SharedResources;
import org.apache.openoffice.comp.sdbc.dbtools.util.StandardSQLState;

import com.sun.star.io.XInputStream;
import com.sun.star.lib.uno.adapter.InputStreamToXInputStreamAdapter;
import com.sun.star.lib.uno.helper.ComponentBase;
import com.sun.star.logging.LogLevel;
import com.sun.star.sdbc.SQLException;
import com.sun.star.sdbc.XBlob;
import com.sun.star.uno.Any;

public class JavaSQLBlob extends ComponentBase implements XBlob {
    private final ResourceBasedEventLogger logger;
    private final java.sql.Blob jdbcBlob;

    public JavaSQLBlob(ResourceBasedEventLogger logger, java.sql.Blob jdbcBlob) {
        this.logger = logger;
        this.jdbcBlob = jdbcBlob;
    }

    @Override
    protected void postDisposing() {
        try {
            jdbcBlob.free();
        } catch (java.sql.SQLException jdbcSQLException) {
            logger.log(LogLevel.WARNING, jdbcSQLException);
        }
    }

    @Override
    public XInputStream getBinaryStream() throws SQLException {
        try {
            InputStream inputStream = jdbcBlob.getBinaryStream();
            if (inputStream != null) {
                return new InputStreamToXInputStreamAdapter(inputStream);
            } else {
                return null;
            }
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public byte[] getBytes(long pos, int length) throws SQLException {
        try {
            return jdbcBlob.getBytes(pos, length);
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public long length() throws SQLException {
        try {
            return jdbcBlob.length();
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public long position(byte[] pattern, long start) throws SQLException {
        try {
            return jdbcBlob.position(pattern, start);
        } catch (java.sql.SQLException jdbcSQLException) {
            throw Tools.toUnoException(this, jdbcSQLException);
        }
    }

    @Override
    public long positionOfBlob(XBlob arg0, long arg1) throws SQLException {
        // this was put here in CWS warnings01. The previous implementation was defective, as it did ignore
        // the pattern parameter. Since the effort for proper implementation is rather high - we would need
        // to translated patter into a byte[] -, we defer this functionality for the moment (hey, it was
        // unusable, anyway)
        // 2005-11-15 / #i57457# / frank.schoenheit@sun.com
        throw new SQLException(
                SharedResources.getInstance().getResourceStringWithSubstitution(
                        Resources.STR_UNSUPPORTED_FEATURE, "$featurename$", "XBlob::positionOfBlob"),
                this, StandardSQLState.SQL_FEATURE_NOT_IMPLEMENTED.name(), 0, Any.VOID);
    }
}
