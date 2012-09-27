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

import com.sun.star.accessibility.XAccessible;
import com.sun.star.accessibility.XAccessibleContext;
import com.sun.star.awt.XExtendedToolkit;
import com.sun.star.awt.XWindow;
import com.sun.star.beans.Optional;
import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XNameAccess;
import com.sun.star.sdb.CommandType;
import com.sun.star.sdb.application.XCopyTableWizard;
import com.sun.star.sdb.DataAccessDescriptorFactory;
import com.sun.star.sdbc.XConnection;
import com.sun.star.sdbcx.XTablesSupplier;
import com.sun.star.task.XInteractionHandler;
import com.sun.star.uno.Exception;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import connectivity.tools.DbaseDatabase;
import java.io.IOException;
import util.UITools;

// ---------- junit imports -----------------
import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import static org.junit.Assert.*;
// ------------------------------------------

/** complex test case for Base's application UI
 */
public class CopyTableWizard extends CRMBasedTestCase
{

    private DatabaseApplication source;
    private DbaseDatabase destinationDB = null;
    private DatabaseApplication dest;

    public CopyTableWizard()
    {
        super();
    }

    // --------------------------------------------------------------------------------------------------------

    @After
    @Override
    public void after()
    {
        dest.store();
        if ( destinationDB != null )
            destinationDB.close();
        destinationDB = null;
        super.after();
    }

    @Before
    @Override
    public void before()
    {
        try
        {
            createTestCase();
            source = new DatabaseApplication(m_database.getDatabase());
            destinationDB = new DbaseDatabase( getMSF() );
            dest = new DatabaseApplication( destinationDB );
        }
        catch (java.lang.Exception ex)
        {
            fail("");
        }
    }

    // --------------------------------------------------------------------------------------------------------
    class CopyThread implements Runnable
    {

        final XCopyTableWizard copyWizard;

        CopyThread(final XCopyTableWizard copyWizard)
        {
            this.copyWizard = copyWizard;
        }

        public void run()
        {
            copyWizard.execute();
        }
    }

    private XWindow getActiveWindow()
    {
        Object toolKit = null;
        try
        {
            toolKit = getMSF().createInstance("com.sun.star.awt.Toolkit");
        }
        catch (com.sun.star.uno.Exception e)
        {
            return null;
        }

        XExtendedToolkit tk = UnoRuntime.queryInterface( XExtendedToolkit.class, toolKit );
        Object atw = tk.getActiveTopWindow();
        return UnoRuntime.queryInterface( XWindow.class, atw );
    }

    @Test
    public void copyTable() throws Exception, IOException, java.lang.Exception
    {
        copyTable(source,source);
    }

    @Test
    public void copyTableDbase() throws Exception, IOException, java.lang.Exception
    {
        copyTable(source,dest);
    }
    private void copyTable(final DatabaseApplication sourceDb,final DatabaseApplication destDb) throws Exception, IOException, java.lang.Exception
    {
        final XConnection destConnection = destDb.getDocumentUI().getActiveConnection();

        final XConnection sourceConnection = sourceDb.getDocumentUI().getActiveConnection();
        final XTablesSupplier suppTables = UnoRuntime.queryInterface(XTablesSupplier.class, sourceConnection);
        final XNameAccess tables = suppTables.getTables();

        final String[] names = tables.getElementNames();
        for (int i = 0; i < names.length; i++)
        {
            copyTable(names[i], sourceConnection, destConnection);
        }
    }

    private void copyTable(final String tableName, final XConnection sourceConnection, final XConnection destConnection) throws Exception, IOException, java.lang.Exception
    {

        final XInteractionHandler interAction = new CopyTableInterActionHandler();
        final XComponentContext context = getComponentContext();
        final XPropertySet sourceDescriptor = DataAccessDescriptorFactory.get(context).createDataAccessDescriptor();
        sourceDescriptor.setPropertyValue("CommandType", CommandType.TABLE);
        sourceDescriptor.setPropertyValue("Command", tableName);
        sourceDescriptor.setPropertyValue("ActiveConnection", sourceConnection);

        final XPropertySet destDescriptor = DataAccessDescriptorFactory.get(context).createDataAccessDescriptor();
        destDescriptor.setPropertyValue("ActiveConnection", destConnection);

        final XCopyTableWizard copyWizard = com.sun.star.sdb.application.CopyTableWizard.createWithInteractionHandler(
            context, sourceDescriptor, destDescriptor, interAction);
        copyWizard.setOperation((short) 0); // com.sun.star.sdb.application.CopyDefinitionAndData
        Optional<String> auto = new Optional<String>();

        auto.IsPresent = destConnection.getMetaData().supportsCoreSQLGrammar();
        if (auto.IsPresent)
        {
            auto.Value = "ID_test";
        }
        copyWizard.setCreatePrimaryKey(auto);
        Thread thread = new Thread(new CopyThread(copyWizard));
        thread.start();
        sleep();

        try
        {
            final XWindow dialog = getActiveWindow();
            final UITools uiTools = new UITools(getMSF(), dialog);
            final XAccessible root = uiTools.getRoot();
            final XAccessibleContext accContext = root.getAccessibleContext();
            final int count = accContext.getAccessibleChildCount();
            String buttonName = "Create";
            final XAccessibleContext childContext = accContext.getAccessibleChild(count - 3).getAccessibleContext();
            final String name = childContext.getAccessibleName();
            if (name != null && !"".equals(name))
            {
                buttonName = name;
            }
            try
            {
                uiTools.clickButton(buttonName);
            }
            catch (java.lang.Exception exception)
            {
                exception.printStackTrace( System.err );
            }
        }
        catch (com.sun.star.lang.IndexOutOfBoundsException indexOutOfBoundsException)
        {
        }
        sleep();

        thread.join();
    }

    private void sleep()
    {
        try
        {
            Thread.sleep(500);
        }
        catch (java.lang.InterruptedException e)
        {
        }
    }
    // --------------------------------------------------------------------------------------------------------
}
