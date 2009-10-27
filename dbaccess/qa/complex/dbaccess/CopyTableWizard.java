/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: CopyTableWizard.java,v $
 * $Revision: 1.1.2.1 $
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

/** complex test case for Base's application UI
 */
public class CopyTableWizard extends CRMBasedTestCase
{

    private DatabaseApplication source;
    private DatabaseApplication dest;

    public CopyTableWizard()
    {
        super();
    }

    // --------------------------------------------------------------------------------------------------------
    public String[] getTestMethodNames()
    {
        return new String[]
                {
                    "copyTable", "copyTableDbase"
                };
    }

    // --------------------------------------------------------------------------------------------------------
    @Override
    public String getTestObjectName()
    {
        return getClass().getName();
    }
// --------------------------------------------------------------------------------------------------------
    // --------------------------------------------------------------------------------------------------------

    @Override
    public void after()
    {
        dest.store();
        super.after();
    }

    @Override
    public void before()
    {
        try
        {
            createTestCase();
            source = new DatabaseApplication(this.m_database.getDatabase());
            dest = new DatabaseApplication(new DbaseDatabase(getORB()));
        }
        catch (java.lang.Exception ex)
        {
            assure(false);
        }
    }
    // --------------------------------------------------------------------------------------------------------



    class CopyThread implements Runnable
    {

        final XCopyTableWizard copyWizard;

        public CopyThread(final XCopyTableWizard copyWizard)
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
            toolKit = getORB().createInstance("com.sun.star.awt.Toolkit");
        }
        catch (com.sun.star.uno.Exception e)
        {
            return null;
        }

        XExtendedToolkit tk = (XExtendedToolkit) UnoRuntime.queryInterface(XExtendedToolkit.class, toolKit);
        Object atw = tk.getActiveTopWindow();
        return (XWindow) UnoRuntime.queryInterface(XWindow.class, atw);
    }

    public void copyTable() throws Exception, IOException, java.lang.Exception
    {
        copyTable(source,source);
    }

    public void copyTableDbase() throws Exception, IOException, java.lang.Exception
    {
        copyTable(source,dest);
    }
    public void copyTable(final DatabaseApplication sourceDb,final DatabaseApplication destDb) throws Exception, IOException, java.lang.Exception
    {
        final XConnection destConnection = destDb.getDocumentUI().getActiveConnection();

        final XConnection sourceConnection = sourceDb.getDocumentUI().getActiveConnection();
        final XTablesSupplier suppTables = (XTablesSupplier) UnoRuntime.queryInterface(XTablesSupplier.class, sourceConnection);
        final XNameAccess tables = suppTables.getTables();

        final String[] names = tables.getElementNames();
        for (int i = 0; i < names.length; i++)
        {
            copyTable(names[i], sourceConnection, destConnection);
        }
    }

    public void assure(final String message)
    {
        assure(message, false);
    }

    private void copyTable(final String tableName, final XConnection sourceConnection, final XConnection destConnection) throws Exception, IOException, java.lang.Exception
    {

        final XInteractionHandler interAction = new CopyTableInterActionHandler(this);
        final XComponentContext context = getComponentContext();
        final XPropertySet sourceDescriptor = DataAccessDescriptorFactory.get(context).createDataAccessDescriptor();
        sourceDescriptor.setPropertyValue("CommandType", CommandType.TABLE);
        sourceDescriptor.setPropertyValue("Command", tableName);
        sourceDescriptor.setPropertyValue("ActiveConnection", sourceConnection);

        final XPropertySet destDescriptor = DataAccessDescriptorFactory.get(context).createDataAccessDescriptor();
        destDescriptor.setPropertyValue("ActiveConnection", destConnection);

        final XCopyTableWizard copyWizard = com.sun.star.sdb.application.CopyTableWizard.createWithInteractionHandler(context, sourceDescriptor, destDescriptor, interAction);
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
            final UITools uiTools = new UITools(getORB(), dialog);
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
                exception.printStackTrace();
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
