/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: RowSetEventListener.java,v $
 * $Revision: 1.4 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
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
