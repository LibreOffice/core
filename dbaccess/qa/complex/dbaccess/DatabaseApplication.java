/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
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
