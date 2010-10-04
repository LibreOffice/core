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
package complex.tdoc;

import com.sun.star.beans.Property;
import com.sun.star.beans.XPropertySetInfo;
import com.sun.star.frame.XModel;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.XTextDocument;
import com.sun.star.ucb.Command;
import com.sun.star.ucb.XCommandProcessor;
import com.sun.star.ucb.XContent;
import com.sun.star.ucb.XContentIdentifier;
import com.sun.star.ucb.XContentIdentifierFactory;
import com.sun.star.ucb.XContentProvider;
import com.sun.star.uno.UnoRuntime;
import util.WriterTools;

import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import org.openoffice.test.OfficeConnection;
import static org.junit.Assert.*;
/**
 *
 */
public class CheckTransientDocumentsContent {
    // TODO: document doesn't exists
    private final String testDocuments[] = new String[]{"sForm.sxw"};//, "chinese.sxw", "Iterator.sxw"};
    private final int countDocs = testDocuments.length;
    private XMultiServiceFactory xMSF = null;
    private XTextDocument[] xTextDoc = null;

    public String[] getTestMethodNames() {
        return new String[] {"checkTransientDocumentsContent"};
    }

    @Before public void before() {
        xMSF = getMSF();
        xTextDoc = new XTextDocument[countDocs];
        System.out.println("Open some documents.");
        for (int i=0; i<countDocs; i++) {
            String fileName = TestDocument.getUrl(testDocuments[i]);
            xTextDoc[i] = WriterTools.loadTextDoc(xMSF, fileName);
            assertNotNull("Can't load document " + fileName, xTextDoc[i]);
        }
    }
    @After public void after() {
        System.out.println("Close all documents.");
        for (int i=0; i<countDocs; i++) {
            xTextDoc[i].dispose();
        }
    }

    /**
     * Check the content of one document
     */
    @Test public void checkTransientDocumentsContent() {
        try {
            // create the ucb
            XContentIdentifierFactory xContentIdentifierFactory =
                            UnoRuntime.queryInterface(XContentIdentifierFactory.class, xMSF.createInstance("com.sun.star.ucb.UniversalContentBroker"));
            XContentProvider xContentProvider =
                            UnoRuntime.queryInterface(XContentProvider.class, xContentIdentifierFactory);
            // create a content identifier from the ucb for tdoc
            XContentIdentifier xContentIdentifier =
                               xContentIdentifierFactory.createContentIdentifier("vnd.sun.star.tdoc:/1");
            // get content
            XContent xContent = xContentProvider.queryContent(xContentIdentifier);

            // actual test: commands to get some properties
            XCommandProcessor xCommandProcessor = UnoRuntime.queryInterface(XCommandProcessor.class, xContent);
            // build up the command
            Command command = new Command();
            command.Name = "getPropertySetInfo";
            command.Handle = -1;

            // execute the command
            Object result = xCommandProcessor.execute(command, 0, null);

            // check the result
            System.out.println("Result: "+ result.getClass().toString());
            XPropertySetInfo xPropertySetInfo = UnoRuntime.queryInterface(XPropertySetInfo.class, result);
            Property[] props = xPropertySetInfo.getProperties();
            boolean res = false;
            for(int i=0; i<props.length; i++) {
                String propName = props[i].Name;
                res |= propName.equals("DocumentModel");
                System.out.println("Found property: " + propName + "   type: " + props[i].Type.getTypeName());
            }
            assertNotNull("Did not find property 'DocumentModel' in the Property array.", res);

            // get on property
            command.Name = "getPropertyValues";
            command.Handle = -1;
            Property[] prop = new Property[1];
            prop[0] = new Property();
            prop[0].Name = "DocumentModel";
            prop[0].Handle = -1;
            command.Argument = prop;

            // execute the command
            result = xCommandProcessor.execute(command, 0, null);

            // check the result
            System.out.println("Result: "+ result.getClass().toString());

            XModel xModel = UnoRuntime.queryInterface(XModel.class, result);
            assertTrue("Did not get property 'DocumentModel'.", xModel == null);
        }
        catch (com.sun.star.uno.Exception e) {
            e.printStackTrace();
            fail("Could not create test objects.");
        }

    }

     private XMultiServiceFactory getMSF()
    {
        final XMultiServiceFactory xMSF1 = UnoRuntime.queryInterface(XMultiServiceFactory.class, connection.getComponentContext().getServiceManager());
        return xMSF1;
    }

    // setup and close connections
    @BeforeClass public static void setUpConnection() throws Exception {
        System.out.println("setUpConnection()");
        connection.setUp();
    }

    @AfterClass public static void tearDownConnection()
        throws InterruptedException, com.sun.star.uno.Exception
    {
        System.out.println("tearDownConnection()");
        connection.tearDown();
    }

    private static final OfficeConnection connection = new OfficeConnection();


}
