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
import com.sun.star.beans.PropertyValue;
import com.sun.star.document.XDocumentSubStorageSupplier;
import com.sun.star.embed.ElementModes;
import com.sun.star.embed.XStorage;
import com.sun.star.frame.XModel;
import com.sun.star.frame.XTransientDocumentsDocumentContentFactory;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sdbc.XResultSet;
import com.sun.star.sdbc.XRow;
import com.sun.star.text.XTextDocument;
import com.sun.star.ucb.Command;
import com.sun.star.ucb.ContentInfo;
import com.sun.star.ucb.OpenCommandArgument2;
import com.sun.star.ucb.OpenMode;
import com.sun.star.ucb.XCommandProcessor;
import com.sun.star.ucb.XContent;
import com.sun.star.ucb.XDynamicResultSet;
import com.sun.star.uno.UnoRuntime;
import util.WriterTools;
import util.utils;

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
public class CheckTransientDocumentsDocumentContent {
    // TODO: document doesn't exists
    private final String testDocuments = "sForm.sxw";
    private final String folderName = "TestFolder";
    private XMultiServiceFactory xMSF = null;
    private XTextDocument xTextDoc = null;

//    public String[] getTestMethodNames() {
//        return new String[]{"checkTransientDocumentsDocumentContent"};
//    }

    @Before public void before() {
        xMSF = getMSF();
        System.out.println("Open a document.");
        String fileName = TestDocument.getUrl(testDocuments);
        xTextDoc = WriterTools.loadTextDoc(xMSF, fileName);
        assertNotNull(xTextDoc);
    }
    @After public void after() {
        System.out.println("Close all documents.");
        xTextDoc.dispose();
    }

    /**
     * Check the provider of document content: open some documents
     * and look if they are accessible.
     */
    @Test public void checkTransientDocumentsDocumentContent() {
        try {
            // create a content provider
            Object o = xMSF.createInstance("com.sun.star.comp.ucb.TransientDocumentsDocumentContentFactory");

            XTransientDocumentsDocumentContentFactory xTransientDocumentsDocumentContentFactory =
                            UnoRuntime.queryInterface(XTransientDocumentsDocumentContentFactory.class, o);
            // get the model from the opened document
            XModel xModel = xTextDoc.getCurrentController().getModel();

            // a little additional check for 114733
            XDocumentSubStorageSupplier xDocumentSubStorageSupplier = UnoRuntime.queryInterface(XDocumentSubStorageSupplier.class, xModel);
            String[]names = xDocumentSubStorageSupplier.getDocumentSubStoragesNames();
            for (int i=0; i<names.length; i++) {
                System.out.println("SubStorage names " + i + ": " +names[i]);
            }
            XStorage xStorage = xDocumentSubStorageSupplier.getDocumentSubStorage(names[0], ElementModes.READWRITE);
            assertTrue("Could not get a storage from the XDocumentStorageSupplier.", xStorage != null);
            // get content
            XContent xContent = xTransientDocumentsDocumentContentFactory.createDocumentContent(xModel);
            // actual test: execute some commands
            XCommandProcessor xCommandProcessor = UnoRuntime.queryInterface(XCommandProcessor.class, xContent);

            // create the command and arguments
            Command command = new Command();
            OpenCommandArgument2 cmargs2 = new OpenCommandArgument2();
            Property[]props = new Property[1];
            props[0] = new Property();
            props[0].Name = "Title";
            props[0].Handle = -1;
            cmargs2.Mode = OpenMode.ALL;
            cmargs2.Properties = props;

            command.Name = "open";
            command.Argument = cmargs2;

            Object result = xCommandProcessor.execute(command, 0, null);
            XDynamicResultSet xDynamicResultSet = UnoRuntime.queryInterface(XDynamicResultSet.class, result);
            XResultSet xResultSet = xDynamicResultSet.getStaticResultSet();
            XRow xRow = UnoRuntime.queryInterface(XRow.class, xResultSet);
            // create the new folder 'folderName': first, check if it's already there
            while(xResultSet.next()) {
                String existingFolderName = xRow.getString(1);
                System.out.println("Found existing folder: '" + existingFolderName + "'");
                if (folderName.equals(existingFolderName)) {
                    fail("Cannot create a new folder: folder already exists: adapt test or choose a different document.");
                }
            }
            // create a folder
            System.out.println("Create new folder "+ folderName);
            ContentInfo contentInfo = new ContentInfo();
            contentInfo.Type = "application/vnd.sun.star.tdoc-folder";

            command.Name = "createNewContent";
            command.Argument = contentInfo;

            result = xCommandProcessor.execute(command, 0, null);
            XContent xNewFolder = UnoRuntime.queryInterface(XContent.class, result);

            XCommandProcessor xFolderCommandProcessor = UnoRuntime.queryInterface(XCommandProcessor.class, xNewFolder);
            System.out.println("Got the new folder: " + utils.getImplName(xNewFolder));

            // name the new folder
            PropertyValue[] titleProp = new PropertyValue[1];
            titleProp[0] = new PropertyValue();
            titleProp[0].Name = "Title";
            titleProp[0].Handle = -1;
            titleProp[0].Value = folderName;
            Command titleSetCommand = new Command();
            titleSetCommand.Name = "setPropertyValues";
            titleSetCommand.Argument = titleProp;
            xFolderCommandProcessor.execute(titleSetCommand, 0, null);

            // 2do: check all this stuff!
            // commit changes
/*            InsertCommandArgument insertArgs = new InsertCommandArgument();
            insertArgs.Data = null;
            insertArgs.ReplaceExisting = true;
            Command commitCommand = new Command();
            commitCommand.Name = "insert";
            commitCommand.Argument = insertArgs;
            xFolderCommandProcessor.execute(commitCommand, 0, null); */
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
