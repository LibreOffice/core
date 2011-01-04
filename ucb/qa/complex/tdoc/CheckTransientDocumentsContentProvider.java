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

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.sdbc.XResultSet;
import com.sun.star.text.XTextDocument;
import com.sun.star.ucb.Command;
import com.sun.star.ucb.OpenCommandArgument2;
import com.sun.star.ucb.OpenMode;
import com.sun.star.ucb.XCommandProcessor;
import com.sun.star.ucb.XContent;
import com.sun.star.ucb.XContentAccess;
import com.sun.star.ucb.XContentIdentifier;
import com.sun.star.ucb.XContentIdentifierFactory;
import com.sun.star.ucb.XContentProvider;
import com.sun.star.ucb.XDynamicResultSet;
import com.sun.star.uno.UnoRuntime;
// import complexlib.ComplexTestCase;
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
public class CheckTransientDocumentsContentProvider {
    // TODO: document doesn't exists
    private final String testDocuments[] = new String[]{/*"sForm.sxw",*/ "chinese.sxw", "Iterator.sxw"};
    private final int countDocs = testDocuments.length;
    private XMultiServiceFactory xMSF = null;
    private XTextDocument[] xTextDoc = null;

    public String[] getTestMethodNames() {
        return new String[]{"checkTransientDocumentsContentProvider"};
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
     * Check the provider of document content: open some documents
     * and look if they are accessible.
     */
    @Test public void checkTransientDocumentsContentProvider() {
        try {
            // create a content provider
            Object o = xMSF.createInstance("com.sun.star.comp.ucb.TransientDocumentsContentProvider");
            XContentProvider xContentProvider =
                            UnoRuntime.queryInterface(XContentProvider.class, o);

            // create the ucb
            XContentIdentifierFactory xContentIdentifierFactory =
                            UnoRuntime.queryInterface(XContentIdentifierFactory.class, xMSF.createInstance("com.sun.star.ucb.UniversalContentBroker"));
            // create a content identifier from the ucb for tdoc
            XContentIdentifier xContentIdentifier =
                            xContentIdentifierFactory.createContentIdentifier("vnd.sun.star.tdoc:/");
            // get content
            XContent xContent = xContentProvider.queryContent(xContentIdentifier);

            // actual test: execute an "open" command with the content
            XCommandProcessor xCommandProcessor = UnoRuntime.queryInterface(XCommandProcessor.class, xContent);
            // build up the command
            Command command = new Command();
            OpenCommandArgument2 commandarg2 = new OpenCommandArgument2();
            commandarg2.Mode = OpenMode.ALL;
            command.Name = "open";
            command.Argument = commandarg2;

            // execute the command
            Object result = xCommandProcessor.execute(command, 0, null);

            // check the result
            System.out.println("Result: "+ result.getClass().toString());
            XDynamicResultSet xDynamicResultSet = UnoRuntime.queryInterface(XDynamicResultSet.class, result);

            // check bug of wrong returned service name.
            XServiceInfo xServiceInfo = UnoRuntime.queryInterface(XServiceInfo.class, xDynamicResultSet);
            String[] sNames = xServiceInfo.getSupportedServiceNames();
            String serviceName = sNames[0];
            if (sNames.length > 1)
            {
                fail("Implementation has been changed. Check this test!");
            }
            assertTrue("The service name '" + serviceName + "' is not valid.", !serviceName.equals("com.sun.star.ucb.DynamicContentResultSet"));

            XResultSet xResultSet = xDynamicResultSet.getStaticResultSet();
            XContentAccess xContentAccess = UnoRuntime.queryInterface(XContentAccess.class, xResultSet);
            // iterate over the result: three docs were opened, we should have at least three content identifier strings
            int countContentIdentifiers = 0;
            while(xResultSet.next()) {
                countContentIdentifiers++;
                String identifier = xContentAccess.queryContentIdentifierString();
                System.out.println("Identifier of row " + xResultSet.getRow() + ": " + identifier);
            }
            // some feeble test: if the amount >2, we're ok.
            // 2do: check better
            assertTrue("Did only find " + countContentIdentifiers + " open documents." +
                        " Should have been at least 3.", countContentIdentifiers>2);
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
