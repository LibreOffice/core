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

import com.sun.star.beans.PropertyState;
import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XEnumeration;
import com.sun.star.container.XEnumerationAccess;
import com.sun.star.container.XNameAccess;
import com.sun.star.frame.FrameSearchFlag;
import com.sun.star.frame.XComponentLoader;
import com.sun.star.frame.XController;
import com.sun.star.frame.XDispatch;
import com.sun.star.frame.XDispatchProvider;
import com.sun.star.frame.XFrame;
import com.sun.star.frame.XModel;
import com.sun.star.frame.XStorable;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sdb.CommandType;
import com.sun.star.sdb.XDocumentDataSource;
import com.sun.star.sdb.XOfficeDatabaseDocument;
import com.sun.star.sdb.application.XDatabaseDocumentUI;
import com.sun.star.sdbcx.XTablesSupplier;
import com.sun.star.uno.Exception;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import com.sun.star.uno.XNamingService;
import com.sun.star.util.URL;
import com.sun.star.util.XCloseable;
import com.sun.star.util.XURLTransformer;
import com.sun.star.view.XSelectionSupplier;
import connectivity.tools.DataSource;
import connectivity.tools.HsqlColumnDescriptor;
import connectivity.tools.HsqlDatabase;
import connectivity.tools.HsqlTableDescriptor;
import helper.URLHelper;
import java.io.File;
import java.io.IOException;
import util.UITools;

/** complex test case for Base's application UI
 */
public class Beamer extends complexlib.ComplexTestCase
{

    private XModel docModel;

    public Beamer()
    {
        super();
    }

    // --------------------------------------------------------------------------------------------------------
    protected final XComponentContext getComponentContext()
    {
        XComponentContext context = null;
        try
        {
            final XPropertySet orbProps = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, getORB());
            context = (XComponentContext) UnoRuntime.queryInterface(XComponentContext.class,
                    orbProps.getPropertyValue("DefaultContext"));
        }
        catch (Exception ex)
        {
            failed("could not retrieve the ComponentContext");
        }
        return context;
    }
    // --------------------------------------------------------------------------------------------------------

    public String[] getTestMethodNames()
    {
        return new String[]
                {
                    "testBeamer"
                };
    }

    // --------------------------------------------------------------------------------------------------------
    public String getTestObjectName()
    {
        return getClass().getName();
    }

    // --------------------------------------------------------------------------------------------------------
    protected final XMultiServiceFactory getORB()
    {
        return (XMultiServiceFactory) param.getMSF();
    }

    // --------------------------------------------------------------------------------------------------------
    private void impl_closeDocument()
    {
    }

    // --------------------------------------------------------------------------------------------------------
    public void before() throws Exception, java.lang.Exception
    {
        // load it into a frame
        final Object object = getORB().createInstance("com.sun.star.frame.Desktop");
        final XComponentLoader xComponentLoader = (XComponentLoader) UnoRuntime.queryInterface(XComponentLoader.class, object);
        final XComponent loadedComponent = xComponentLoader.loadComponentFromURL("private:factory/swriter", "_blank", 0, new PropertyValue[0]);
        // get the controller, which provides access to various UI operations
        docModel = (XModel) UnoRuntime.queryInterface(XModel.class, loadedComponent);
    }

    // --------------------------------------------------------------------------------------------------------
    public void after()
    {
    }
    // --------------------------------------------------------------------------------------------------------

    public void testBeamer() throws Exception, IOException, java.lang.Exception
    {
        final XController controller = docModel.getCurrentController();
        final XFrame frame = controller.getFrame();
        final XDispatchProvider dispatchP = (XDispatchProvider) UnoRuntime.queryInterface(XDispatchProvider.class, frame);
        URL command = new URL();
        command.Complete = ".uno:ViewDataSourceBrowser";

        Object instance = getORB().createInstance("com.sun.star.util.URLTransformer");
        XURLTransformer atrans = (XURLTransformer) UnoRuntime.queryInterface(XURLTransformer.class, instance);
        com.sun.star.util.URL[] aURLA = new com.sun.star.util.URL[1];
        aURLA[0] = command;
        atrans.parseStrict(aURLA);
        command = aURLA[0];

        final XDispatch dispatch = dispatchP.queryDispatch(command, "_self", FrameSearchFlag.AUTO);
        assure(dispatch != null);
        dispatch.dispatch(command, new PropertyValue[0]);

        final PropertyValue[] props = new PropertyValue[]
        {
            new PropertyValue("DataSourceName", 0, "Bibliography", PropertyState.DIRECT_VALUE),
            new PropertyValue("CommandType", 0, Integer.valueOf(CommandType.TABLE), PropertyState.DIRECT_VALUE),
            new PropertyValue("Command", 0, "biblio", PropertyState.DIRECT_VALUE)
        };

        final XFrame beamer = frame.findFrame("_beamer", 0);
        assure(beamer != null);
        final XEnumerationAccess evtBc = (XEnumerationAccess) UnoRuntime.queryInterface(XEnumerationAccess.class, getORB().createInstance("com.sun.star.frame.GlobalEventBroadcaster"));
        XEnumeration enumeration = evtBc.createEnumeration();
        int count = -1;
        while (enumeration.hasMoreElements())
        {
            enumeration.nextElement();
            ++count;
        }
        final XSelectionSupplier selSup = (XSelectionSupplier)UnoRuntime.queryInterface(XSelectionSupplier.class, beamer.getController());
        selSup.select(props);
        final com.sun.star.util.XCloseable close = (com.sun.star.util.XCloseable)UnoRuntime.queryInterface(com.sun.star.util.XCloseable.class, frame);
        close.close(false);

        enumeration = evtBc.createEnumeration();
        int count2 = 0;
        while (enumeration.hasMoreElements())
        {
            enumeration.nextElement();
            ++count2;
        }

        assure("count1 = " + count + " count2 = " + count2, count == count2);
    }
}
