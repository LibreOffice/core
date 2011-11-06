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


package connectivity.tools;

import com.sun.star.frame.XModel;
import com.sun.star.io.IOException;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sdb.XOfficeDatabaseDocument;
import com.sun.star.sdbc.SQLException;
import connectivity.tools.sdb.Connection;

/**
 *
 * @author oj93728
 */
public interface DatabaseAccess
{
    Connection defaultConnection() throws SQLException;

    void executeSQL(final String statementString) throws SQLException;

    void store() throws IOException;

    void close();

    void closeAndDelete();

    XOfficeDatabaseDocument getDatabaseDocument();

    XModel getModel();

    String getDocumentURL();

    DataSource getDataSource();

    RowSet createRowSet(final int _commandType, final String _command);

    XMultiServiceFactory getORB();
}
