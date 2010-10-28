/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
package complex.dbaccess;

import com.sun.star.beans.PropertyValue;
import com.sun.star.frame.FrameSearchFlag;
import com.sun.star.frame.XComponentLoader;
import com.sun.star.frame.XModel;
import com.sun.star.frame.XStorable;
import com.sun.star.lang.XComponent;
import com.sun.star.sdb.XOfficeDatabaseDocument;
import com.sun.star.sdb.application.XDatabaseDocumentUI;
import com.sun.star.uno.Exception;
import com.sun.star.uno.UnoRuntime;
import connectivity.tools.DatabaseAccess;

/**
 *
 * @author oj93728
 */
public class DatabaseApplication
{

    private final XOfficeDatabaseDocument databaseDocument;
    private final XDatabaseDocumentUI documentUI;
    private final DatabaseAccess db;

    public DatabaseApplication(final DatabaseAccess _db) throws Exception
    {
        db = _db;
        databaseDocument = db.getDatabaseDocument();

        // load it into a frame
        final Object object = db.getORB().createInstance("com.sun.star.frame.Desktop");
        final XComponentLoader xComponentLoader = UnoRuntime.queryInterface(XComponentLoader.class, object);
        final XComponent loadedComponent = xComponentLoader.loadComponentFromURL(db.getDocumentURL(), "_blank", FrameSearchFlag.ALL, new PropertyValue[0]);

        // get the controller, which provides access to various UI operations
        final XModel docModel = UnoRuntime.queryInterface(XModel.class, loadedComponent);
        documentUI = UnoRuntime.queryInterface(XDatabaseDocumentUI.class, docModel.getCurrentController());
        documentUI.connect();
    }

    public XOfficeDatabaseDocument getDatabaseDocument()
    {
        return databaseDocument;
    }

    public XDatabaseDocumentUI getDocumentUI()
    {
        return documentUI;
    }

    public DatabaseAccess getDb()
    {
        return db;
    }

    public void store()
    {
        // store the doc in a new location
        try
        {
            final XStorable storeDoc = UnoRuntime.queryInterface(XStorable.class, databaseDocument);
            if (storeDoc != null)
            {
                storeDoc.store();
            }
        }
        catch (com.sun.star.io.IOException iOException)
        {
        }
    }
}
