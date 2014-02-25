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

import com.sun.star.beans.PropertyState;
import com.sun.star.beans.PropertyValue;
import com.sun.star.container.XEnumeration;
import com.sun.star.frame.FrameSearchFlag;
import com.sun.star.frame.XComponentLoader;
import com.sun.star.frame.XController;
import com.sun.star.frame.XDispatch;
import com.sun.star.frame.XDispatchProvider;
import com.sun.star.frame.XFrame;
import com.sun.star.frame.XGlobalEventBroadcaster;
import com.sun.star.frame.XModel;
import com.sun.star.frame.theGlobalEventBroadcaster;
import com.sun.star.lang.XComponent;
import com.sun.star.sdb.CommandType;
import com.sun.star.uno.Exception;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.util.URL;
import com.sun.star.util.XURLTransformer;
import com.sun.star.view.XSelectionSupplier;
import java.io.IOException;


// ---------- junit imports -----------------
import org.junit.After;
// import org.junit.AfterClass;
import org.junit.Before;
// import org.junit.BeforeClass;
import org.junit.Test;
// import org.openoffice.test.OfficeConnection;
import static org.junit.Assert.*;



/** complex test case for Base's application UI
 */
public class Beamer extends TestCase
{

    private XModel docModel;

    public Beamer()
    {
        super();
    }


    @Before
    @Override
    public void before() throws Exception, java.lang.Exception
    {
        // load it into a frame
        final Object object = getMSF().createInstance("com.sun.star.frame.Desktop");
        final XComponentLoader xComponentLoader = UnoRuntime.queryInterface(XComponentLoader.class, object);
        final XComponent loadedComponent = xComponentLoader.loadComponentFromURL("private:factory/swriter", "_blank", 0, new PropertyValue[0]);
        // get the controller, which provides access to various UI operations
        docModel = UnoRuntime.queryInterface(XModel.class, loadedComponent);
    }


    @After
    @Override
    public void after()
    {
    }


    @Test
    public void testBeamer() throws Exception, IOException, java.lang.Exception
    {
        final XController controller = docModel.getCurrentController();
        final XFrame frame = controller.getFrame();
        final XDispatchProvider dispatchP = UnoRuntime.queryInterface(XDispatchProvider.class, frame);
        URL command = new URL();
        command.Complete = ".uno:ViewDataSourceBrowser";

        Object instance = getMSF().createInstance("com.sun.star.util.URLTransformer");
        XURLTransformer atrans = UnoRuntime.queryInterface(XURLTransformer.class, instance);
        com.sun.star.util.URL[] aURLA = new com.sun.star.util.URL[1];
        aURLA[0] = command;
        atrans.parseStrict(aURLA);
        command = aURLA[0];

        final XDispatch dispatch = dispatchP.queryDispatch(command, "_self", FrameSearchFlag.AUTO);
        assertNotNull(dispatch);
        dispatch.dispatch(command, new PropertyValue[0]);

        final PropertyValue[] props = new PropertyValue[]
        {
            new PropertyValue("DataSourceName", 0, "Bibliography", PropertyState.DIRECT_VALUE),
            new PropertyValue("CommandType", 0, Integer.valueOf(CommandType.TABLE), PropertyState.DIRECT_VALUE),
            new PropertyValue("Command", 0, "biblio", PropertyState.DIRECT_VALUE)
        };

        final XFrame beamer = frame.findFrame("_beamer", 0);
        assertNotNull(beamer);
        final XGlobalEventBroadcaster evtBc = theGlobalEventBroadcaster.get(
            getComponentContext());
        XEnumeration enumeration = evtBc.createEnumeration();
        int count = -1;
        while (enumeration.hasMoreElements())
        {
            enumeration.nextElement();
            ++count;
        }
        final XSelectionSupplier selSup = UnoRuntime.queryInterface(XSelectionSupplier.class, beamer.getController());
        selSup.select(props);
        final com.sun.star.util.XCloseable close = UnoRuntime.queryInterface(com.sun.star.util.XCloseable.class, frame);
        close.close(false);

        enumeration = evtBc.createEnumeration();
        int count2 = 0;
        while (enumeration.hasMoreElements())
        {
            enumeration.nextElement();
            ++count2;
        }

        assertTrue("count1 = " + count + " count2 = " + count2, count == count2);
    }
}
